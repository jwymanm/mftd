/*
 * core data and routines
 */

#include "core.h"
#include "ini.h"
#include "net.h"
#include "monitor.h"
#include "fdns.h"
#include "tunnel.h"
#include "dhcp.h"

// Globals
Data gd;
Events ge;
Paths path;
Configuration config;

SERVICE_STATUS serviceStatus;
SERVICE_STATUS_HANDLE serviceStatusHandle = 0;
HANDLE stopServiceEvent = 0;

bool running = true;
pthread_t threads[THREADCOUNT];

namespace core { LocalBuffers lb; }
using namespace core;

/* helpers */

char *cloneString(char *string) {
  if (!string) return NULL;
  char *s = (char*) calloc(1, strlen(string) + 1);
  if (s) { strcpy(s, string); }
  return s;
}

char *myUpper(char *string) {
  char diff = 'a' - 'A';
  MYWORD len = strlen(string);
  for (int i = 0; i < len; i++)
    if (string[i] >= 'a' && string[i] <= 'z') string[i] -= diff;
  return string;
}

char *myLower(char *string) {
  char diff = 'a' - 'A';
  WORD len = strlen(string);
  for (int i = 0; i < len; i++)
    if (string[i] >= 'A' && string[i] <= 'Z') string[i] += diff;
  return string;
}

char* strsep(char** stringp, const char* delim) {
  char *p;
  if (!stringp) return(NULL);
  p=*stringp;
  while (**stringp && !strchr(delim,**stringp)) (*stringp)++;
  if (**stringp) { **stringp='\0'; (*stringp)++; }
  else *stringp=NULL;
  return(p);
}

bool wildcmp(char *string, char *wild) {
  // Written by Jack Handy - jakkhandy@hotmail.com
  // slightly modified
  char *cp = NULL; char *mp = NULL;
  while ((*string) && (*wild != '*')) {
    if ((*wild != *string) && (*wild != '?')) { return 0; }
    wild++; string++;
  }
  while (*string) {
    if (*wild == '*') { if (!*++wild) return 1; mp = wild; cp = string + 1; }
    else if ((*wild == *string) || (*wild == '?')) { wild++; string++; }
    else { wild = mp; string = cp++; }
  }
  while (*wild == '*') wild++;
  return !(*wild);
}

// copies wchar into pchar
void wpcopy(PCHAR dest, PWCHAR src) {
  int srclen = wcslen(src);
  LPSTR destbuf = (LPSTR) calloc(1, srclen);
  WideCharToMultiByte(CP_ACP, 0, src, srclen, destbuf, srclen, NULL, NULL);
  strncpy(dest, destbuf, srclen);
  free(destbuf);
}

bool isInt(char *str) {
  if (!str || !(*str)) return false;
  for(; *str; str++) if (*str <  '0' || *str > '9') return false;
  return true;
}

char *hex2String(char *target, MYBYTE *hex, MYBYTE bytes) {
  char *dp = target;
  if (bytes) dp += sprintf(target, "%02x", *hex);
  else *target = 0;
  for (MYBYTE i = 1; i < bytes; i++) dp += sprintf(dp, ":%02x", *(hex + i));
  return target;
}

char *getHexValue(MYBYTE *target, char *source, MYBYTE *size) {
  if (*size) memset(target, 0, (*size));
  for ((*size) = 0; (*source) && (*size) < UCHAR_MAX; (*size)++, target++) {
    if ((*source) >= '0' && (*source) <= '9') { (*target) = (*source) - '0'; }
    else if ((*source) >= 'a' && (*source) <= 'f') { (*target) = (*source) - 'a' + 10; }
    else if ((*source) >= 'A' && (*source) <= 'F') { (*target) = (*source) - 'A' + 10; }
    else { return source; }
    source++;
    if ((*source) >= '0' && (*source) <= '9') { (*target) *= 16; (*target) += (*source) - '0'; }
    else if ((*source) >= 'a' && (*source) <= 'f') { (*target) *= 16; (*target) += (*source) - 'a' + 10; }
    else if ((*source) >= 'A' && (*source) <= 'F') { (*target) *= 16; (*target) += (*source) - 'A' + 10; }
    else if ((*source) == ':' || (*source) == '-') { source++; continue; }
    else if (*source) { return source; }
    else { continue; }
    source++;
    if ((*source) == ':' || (*source) == '-') { source++; }
    else if (*source) return source;
  }
  if (*source) return source;
  return NULL;
}

/* logging */

int debug(int level, const char* xstr, void* data) {
  std::string str = "";
  switch (level) {
    case LOG_DEBUG:
      str += "DEBUG: "; break;
    case LOG_INFO:
      str += "INFO: "; break;
    case LOG_NOTICE:
      str += "NOTICE: "; break;
    default:
      break;
  }
  if (xstr) str += xstr;
  if (data) str += static_cast<char *>(data);
  std::cout << str << std::endl;
}

void showError(DWORD enumber) {
  LPTSTR lpMsgBuf;
  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL, enumber, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPTSTR)&lpMsgBuf, 0, NULL);
  logMesg(lpMsgBuf, LOG_NOTICE);
}

void logMesg(char* logBuff, int logLevel) {
  if (config.verbose) {
    debug(logLevel, NULL, (void* ) logBuff);
  }
  if (logLevel <= config.logging) {
    char *mess = cloneString(logBuff);
    _beginthread(logThread, 0, mess);
  }
}

/* threads */

void __cdecl logThread(void *args) {
  char* mess = (char*) args;
  char* lfn = gd.logFN;

  WaitForSingleObject(ge.log, INFINITE);
  lb.t = time(NULL);
  tm *ttm = localtime(&lb.t);
  char buffer[_MAX_PATH + 1];
  strftime(buffer, sizeof(buffer), path.lfn, ttm);

  if (strcmp(lfn, buffer)) {
    if (lfn[0]) {
      FILE *f = fopen(lfn, "at");
      if (f) {
        fprintf(f, "Logging Continued on file %s\n", buffer);
        fclose(f);
      }
      strcpy(lfn, buffer);
      f = fopen(lfn, "at");
      if (f) {
        fprintf(f, "%s\n\n", gd.displayName);
        fclose(f);
      }
    }
    strcpy(lfn, buffer);
  }

  FILE *f = fopen(lfn, "at");

  if (f) {
    strftime(buffer, sizeof(buffer), "%d-%b-%y %X", ttm);
    fprintf(f, "[%s] %s\n", buffer, mess);
    fclose(f);
  } else config.logging = 0;

  free(mess);
  SetEvent(ge.log);

  _endthread();
  return;
}

void startThreads() {
#if FDNS
  if (config.fdns && !fdns_running) {
    pthread_create (&threads[FDNS_TIDX], NULL, fdns::main, NULL);
    Sleep(2000);
  }
#endif
#if TUNNEL
  if (config.tunnel && !tunnel_running) {
    pthread_create (&threads[TUNNEL_TIDX], NULL, tunnel::main, NULL);
    Sleep(2000);
  }
#endif
#if DHCP
  if (config.dhcp && !dhcp_running) {
    pthread_create (&threads[DHCP_TIDX], NULL, dhcp::main, NULL);
    Sleep(2000);
  }
#endif
}

void stopThreads() {
#if FDNS
  if (config.fdns && fdns_running) {
    logMesg("Stopping FDNS", LOG_INFO);
    fdns_running = false;
    fdns::cleanup(0);
    Sleep(2000);
  }
#endif
#if TUNNEL
  if (config.tunnel && tunnel_running) {
    logMesg("Stopping Tunnel", LOG_INFO);
    tunnel_running = false;
    tunnel::cleanup(0);
    Sleep(2000);
  }
#endif
#if DHCP
  if (config.dhcp && dhcp_running) {
    logMesg("Stopping DHCP", LOG_INFO);
    dhcp_running = false;
    Sleep(2000);
  }
#endif
}

void __cdecl threadLoop(void *args) {

#if MONITOR
  if (config.monitor) {
    monitor::start();
    while (running) Sleep(1000);
  } else {
#endif
  sprintf(lb.log, "Looking for adapter with description \"%s\"", config.ifname);
  logMesg(lb.log, LOG_INFO);
  do {
    if (getAdapterData()) {
      sprintf(lb.log, "Adapter with description \"%s\" found", config.ifname);
      logMesg(lb.log, LOG_INFO);
      if (!adptr.ipset) { setAdptrIP(); stopThreads(); }
      logMesg("Starting threads", LOG_INFO);
      startThreads();
    } else {
      sprintf(lb.log, "Waiting for adapter with description \"%s\" to be available", config.ifname);
      logMesg(lb.log, LOG_INFO);
      logMesg("Stopping threads", LOG_INFO);
      stopThreads();
    }
    detectChange();
  } while (running);
#if MONITOR
  }
#endif
  _endthread();
  return;
}

/* service code */

void WINAPI ServiceControlHandler(DWORD controlCode) {

  switch (controlCode) {
    case SERVICE_CONTROL_INTERROGATE:
      break;
    case SERVICE_CONTROL_SHUTDOWN:
    case SERVICE_CONTROL_STOP:
      serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
      serviceStatus.dwWaitHint = 20000;
      serviceStatus.dwCheckPoint = 1;
      SetServiceStatus(serviceStatusHandle, &serviceStatus);
      SetEvent(stopServiceEvent);
      return;
    case SERVICE_CONTROL_PAUSE:
      break;
    case SERVICE_CONTROL_CONTINUE:
      break;
    default:
      if (controlCode >= 128 && controlCode <= 255) break;
      else break;
  }

  SetServiceStatus(serviceStatusHandle, &serviceStatus);
}

void WINAPI ServiceMain(DWORD /*argc*/, TCHAR* /*argv*/[]) {

  serviceStatus.dwServiceType = SERVICE_WIN32;
  serviceStatus.dwCurrentState = SERVICE_STOPPED;
  serviceStatus.dwControlsAccepted = 0;
  serviceStatus.dwWin32ExitCode = NO_ERROR;
  serviceStatus.dwServiceSpecificExitCode = NO_ERROR;
  serviceStatus.dwCheckPoint = 0;
  serviceStatus.dwWaitHint = 0;
  serviceStatusHandle = RegisterServiceCtrlHandler(gd.serviceName, ServiceControlHandler);

  if (serviceStatusHandle) {
    serviceStatus.dwCurrentState = SERVICE_START_PENDING;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);
    stopServiceEvent = CreateEvent(0, FALSE, FALSE, 0);
    serviceStatus.dwControlsAccepted |= (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
    serviceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);
    logMesg(SERVICE_NAME " started", LOG_NOTICE);
    config.isService = true;
    netInit();
#if MONITOR
    if (config.monitor) monitor::start();
    else
#endif
    _beginthread(threadLoop, 0, NULL);
    while (WaitForSingleObject(stopServiceEvent, 0) == WAIT_TIMEOUT) Sleep(1000);
    logMesg(SERVICE_NAME " stopping", LOG_NOTICE);
    config.isExiting = true;
    running = false;
#if MONITOR
    if (config.monitor) monitor::stop();
#endif
    CancelIPChangeNotify(&ge.dCol);
    Sleep(2000);
    stopThreads();
    serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
    serviceStatus.dwCheckPoint = 2;
    serviceStatus.dwWaitHint = 1000;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);
    netExit();
    logMesg(SERVICE_NAME " stopped", LOG_NOTICE);
    serviceStatus.dwControlsAccepted &= ~(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
    serviceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);
    CloseHandle(stopServiceEvent);
    stopServiceEvent = 0;
    return;
  }
}

void runService() {
  SERVICE_TABLE_ENTRY serviceTable[] = { {gd.serviceName, ServiceMain}, {0, 0} };
  StartServiceCtrlDispatcher(serviceTable);
}

bool stopService(SC_HANDLE service) {
  if (service) {
    SERVICE_STATUS serviceStatus;
    QueryServiceStatus(service, &serviceStatus);
    if (serviceStatus.dwCurrentState != SERVICE_STOPPED) {
      ControlService(service, SERVICE_CONTROL_STOP, &serviceStatus);
      printf("Stopping Service.");
      for (int i = 0; i < 100; i++) {
        QueryServiceStatus(service, &serviceStatus);
        if (serviceStatus.dwCurrentState == SERVICE_STOPPED) {
          printf("Stopped\r\n"); return true;
        } else { Sleep(500); printf("."); }
      }
      printf("Failed\n"); return false;
    }
  }
  return true;
}

void installService() {
  SC_HANDLE serviceControlManager = OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE | SERVICE_START);
  if (serviceControlManager) {
    SC_HANDLE service =
      CreateService(serviceControlManager, gd.serviceName, gd.displayName,
      SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_IGNORE, path.exe, 0, 0, 0, 0, 0);
    if (service) {
      printf("Successfully installed %s as a service\n", SERVICE_NAME);
      //StartService(service, 0, NULL);
      CloseServiceHandle(service);
    } else { showError(GetLastError()); }
    CloseServiceHandle(serviceControlManager);
  }
}

void uninstallService() {
  SC_HANDLE serviceControlManager = OpenSCManager(0, 0, SC_MANAGER_CONNECT);
  if (serviceControlManager) {
    SC_HANDLE service = OpenService(serviceControlManager, gd.serviceName, SERVICE_QUERY_STATUS | SERVICE_STOP | DELETE);
    if (service) {
      if (stopService(service)) {
        if (DeleteService(service)) printf("Successfully removed %s from services\n", SERVICE_NAME);
        else showError(GetLastError());
      } else printf("Failed to stop service %s\n", SERVICE_NAME);
      CloseServiceHandle(service);
    } else printf("Service %s not found\n", SERVICE_NAME);
    CloseServiceHandle(serviceControlManager);
  }
  return;
}

/* main execution */

int main(int argc, char* argv[]) {

  OSVERSIONINFO osvi;
  osvi.dwOSVersionInfoSize = sizeof(osvi);
  bool result = GetVersionEx(&osvi);
  char *fileExt;

  strcpy(gd.serviceName, SERVICE_NAME);
  strcpy(gd.displayName, SERVICE_DISPLAY_NAME);

  lb.t = time(NULL);

  if (GetModuleFileName(0, gd.epath, sizeof(gd.bpath) / sizeof(gd.bpath[0])) > 0) {
    strcpy(gd.bpath, gd.epath);
    fileExt = strrchr(gd.bpath, '\\'); *fileExt = 0;
    strcpy(gd.cpath, gd.bpath);
    strcpy(gd.dpath, gd.bpath);
    strcpy(gd.ipath, gd.bpath);
    strcat(gd.ipath, "\\" NAME ".ini");
    // look for config file in same directory as binary or ..\CFGDIR
    if (ini_parse(gd.ipath, ini_handler, &config) < 0) {
      fileExt = strrchr(gd.dpath, '\\'); *fileExt = 0;
      sprintf(gd.cpath, "%s\\" CFGDIR, gd.dpath);
      strcpy(gd.ipath, gd.cpath);
      strcat(gd.ipath, "\\" NAME ".ini");
      if (ini_parse(gd.ipath, ini_handler, &config) < 0) {
        printf("%s can not load configuration file: '" NAME ".ini' in . or %s\r\n", gd.cpath);
        exit(1);
      } else {
        sprintf(gd.lpath, "%s\\" LOGDIR, gd.dpath);
        sprintf(gd.tpath, "%s\\" TMPDIR, gd.dpath);
      }
    } else {
      strcpy(gd.lpath, gd.dpath); strcpy(gd.tpath, gd.dpath);
    }
  } else { exit(1); }

  if (result && osvi.dwPlatformId >= VER_PLATFORM_WIN32_NT) {
    path.bin = gd.bpath; path.cfg = gd.cpath; path.dir = gd.dpath;
    path.exe = gd.epath; path.ini = gd.ipath; path.log = gd.lpath;
    path.tmp = gd.tpath; path.lfn = gd.lfname;
    sprintf(path.lfn, "%s\\" NAME "-%%Y%%m%%d.log", path.log);
    // CE() = default security descriptor, ManualReset, Signalled, object name
    // might want to change these to allow for running multiple instances
    if ((ge.file = CreateEvent(NULL, FALSE, TRUE, TEXT(NAME "FileEvent"))) == NULL)
      printf(NAME ": createEvent error %d\n", GetLastError());
    else if (GetLastError() == ERROR_ALREADY_EXISTS) {
      logMesg("CreateEvent opened an existing event\r\nServer may already be running", LOG_INFO);
      exit(1);
    }
    if ((ge.log = CreateEvent(NULL, FALSE, TRUE, TEXT(NAME "LogEvent"))) == NULL)
      printf(NAME ": createEvent error %d\n", GetLastError());
    else if (GetLastError() == ERROR_ALREADY_EXISTS) {
      logMesg("CreateEvent opened an existing Event\r\nServer May already be Running", LOG_INFO);
      exit(1);
    }
    // TODO: add arguments for each individual service to do one off runs
    if (argc > 1 && lstrcmpi(argv[1], TEXT("-i")) == 0) {
      installService();
    } else if (argc > 1 && lstrcmpi(argv[1], TEXT("-u")) == 0) {
      uninstallService();
    } else if (argc > 1 && lstrcmpi(argv[1], TEXT("-v")) == 0) {
      SC_HANDLE serviceControlManager = OpenSCManager(0, 0, SC_MANAGER_CONNECT);
      bool serviceStopped = true;
      if (serviceControlManager) {
        SC_HANDLE service = OpenService(serviceControlManager, gd.serviceName, SERVICE_QUERY_STATUS | SERVICE_STOP);
	      if (service) {
   	      serviceStopped = stopService(service);
	        CloseServiceHandle(service);
	      }
	      CloseServiceHandle(serviceControlManager);
      }
      if (serviceStopped) {
        config.verbose = true;
        logMesg(NAME " starting", LOG_NOTICE);
        netInit();
        threadLoop(NULL);
      } else printf("Failed to stop service\n");
    } else { runService(); }
    } else if (argc == 1 || lstrcmpi(argv[1], TEXT("-v")) == 0) {
        config.verbose = true;
        logMesg(NAME " starting", LOG_NOTICE);
        netInit();
        threadLoop(NULL);
  } else printf(NAME " requires Windows XP or newer, exiting\n");
  CloseHandle(ge.file);
  CloseHandle(ge.log);
  return 0;
}
