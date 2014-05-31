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
Buffers gb;
Events ge;
Paths path;
Configuration config;

SERVICE_STATUS serviceStatus;
SERVICE_STATUS_HANDLE serviceStatusHandle = 0;
HANDLE stopServiceEvent = 0;

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

char* strsep(char** stringp, const char* delim) {
  char *p;
  if (!stringp) return(NULL);
  p=*stringp;
  while (**stringp && !strchr(delim,**stringp)) (*stringp)++;
  if (**stringp) { **stringp='\0'; (*stringp)++; }
  else *stringp=NULL;
  return(p);
}

int debug(int cond, const char* xstr, void* data) {
  if ((config.logging) != 0) {
    std::string str = "DEBUG: ";
    if (xstr) str += xstr;
    switch (cond) {
      case DEBUG_IP:
        str = str + "IP Address: " + static_cast<char *>(data);
        break;
      case DEBUG_SE:
        str += "WSocket Error: ";
        str += *(int*) data;
        break;
      default :
        if (data) str += static_cast<char *>(data);
        break;
    }
    std::cout << std::endl << str << std::endl;
  }
}

void debugl(const char *mess) {
  char t[254];
  strcpy(t, mess);
  logMesg(t, 1);
}

void logMesg(char* logBuff, int logLevel) {
  if (config.verbose) debug(0, NAME ": ", (void* ) logBuff);
  if (logLevel <= config.logging) {
    char *mess = cloneString(logBuff);
    _beginthread(logThread, 0, mess);
  }
}

/* threads */

void __cdecl logThread(void *lpParam) {
  char* mess = (char*) lpParam;
  char* lfn = gb.logFN;

  WaitForSingleObject(ge.log, INFINITE);
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
        fprintf(f, "%s\n\n", gb.displayName);
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
  }
#endif
#if TUNNEL
  if (config.tunnel && !tunnel_running) {
    pthread_create (&threads[TUNNEL_TIDX], NULL, tunnel::main, NULL);
  }
#endif
#if DHCP
  if (config.dhcp && !dhcp_running) {
    pthread_create (&threads[DHCP_TIDX], NULL, dhcp::main, NULL);
  }
#endif
}

void stopThreads() {
#if FDNS
  if (config.fdns && fdns_running) {
    logMesg("Stopping FDNS", LOG_INFO);
    fdns_running = false;
    fdns::cleanup(0);
    Sleep(1000);
  }
#endif
#if TUNNEL
  if (config.tunnel && tunnel_running) {
    logMesg("Stopping Tunnel", LOG_INFO);
    tunnel_running = false;
    Sleep(1000);
  }
#endif
#if DHCP
  if (config.dhcp && dhcp_running) {
    logMesg("Stopping DHCP", LOG_INFO);
    dhcp_running = false;
    Sleep(1000);
  }
#endif
  Sleep(2000);
}

void runThreads() {
#if MONITOR
  if (config.monitor) {
    if (!monitor_running) {
      monitor::start();
      Sleep(2000);
    }
    while (monitor_running) { monitor::doLoop(); };
  } else {
#endif
    if (getAdapterData()) {
      if (!adptr.ipset) {
        logMesg("Setting adapter ip..", LOG_INFO);
        setAdptrIP();
      } else startThreads();
    } else {
      sprintf(lb.log, "Looking for adapter: %s", config.ifname);
      logMesg(lb.log, LOG_DEBUG);
      stopThreads();
    }
#if MONITOR
  }
#endif
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
  serviceStatusHandle = RegisterServiceCtrlHandler(gb.serviceName, ServiceControlHandler);

  if (serviceStatusHandle) {
    serviceStatus.dwCurrentState = SERVICE_START_PENDING;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);

    stopServiceEvent = CreateEvent(0, FALSE, FALSE, 0);
    serviceStatus.dwControlsAccepted |= (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
    serviceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);

    netInit();

#if MONITOR
    if (config.monitor) {
      monitor::start(); do { monitor::doLoop(); } while (WaitForSingleObject(stopServiceEvent, 0) == WAIT_TIMEOUT);
    } else {
#else
      do { runThreads(); Sleep(THREAD_TO); } while (WaitForSingleObject(stopServiceEvent, 0) == WAIT_TIMEOUT);
#endif
#if MONITOR
    }
#endif

    stopThreads();

#if MONITOR
    if (config.monitor) monitor::stop();
#endif

    serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
    serviceStatus.dwCheckPoint = 2;
    serviceStatus.dwWaitHint = 1000;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);

    Sleep(2000);

    netExit();

    serviceStatus.dwControlsAccepted &= ~(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
    serviceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);

    CloseHandle(stopServiceEvent);
    stopServiceEvent = 0;
    return;
  }
}

void runService() {
  SERVICE_TABLE_ENTRY serviceTable[] = { {gb.serviceName, ServiceMain}, {0, 0} };
  StartServiceCtrlDispatcher(serviceTable);
}

void showError(UINT enumber) {
  LPTSTR lpMsgBuf;
  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL, enumber, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPTSTR)&lpMsgBuf, 0, NULL);
    printf("%s\n", lpMsgBuf);
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
          printf("Stopped\n");
          return true;
        } else { Sleep(500); printf("."); }
      }
      printf("Failed\n");
      return false;
    }
  }
  return true;
}

void installService() {
  SC_HANDLE serviceControlManager = OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE | SERVICE_START);

  if (serviceControlManager) {
    TCHAR path[ _MAX_PATH + 1 ];
    if (GetModuleFileName(0, path, sizeof(path) / sizeof(path[0])) > 0) {
      SC_HANDLE service =
        CreateService(serviceControlManager, gb.serviceName, gb.displayName,
        SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_IGNORE, path, 0, 0, 0, 0, 0);
      if (service) {
        printf("Successfully installed %s as a service\n", SERVICE_NAME);
        //StartService(service, 0, NULL);
        CloseServiceHandle(service);
      } else { showError(GetLastError()); }
    }
    CloseServiceHandle(serviceControlManager);
  }
}

void uninstallService() {
  SC_HANDLE serviceControlManager = OpenSCManager(0, 0, SC_MANAGER_CONNECT);

  if (serviceControlManager) {
    SC_HANDLE service = OpenService(serviceControlManager, gb.serviceName, SERVICE_QUERY_STATUS | SERVICE_STOP | DELETE);
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

  strcpy(gb.serviceName, SERVICE_NAME);
  strcpy(gb.displayName, SERVICE_DISPLAY_NAME);

  lb.t = time(NULL);

  if (GetModuleFileName(0, gb.epath, sizeof(gb.bpath) / sizeof(gb.bpath[0])) > 0) {
    strcpy(gb.bpath, gb.epath);
    fileExt = strrchr(gb.bpath, '\\'); *fileExt = 0;
    strcpy(gb.cpath, gb.bpath);
    strcpy(gb.dpath, gb.bpath);
    strcpy(gb.ipath, gb.bpath);
    strcat(gb.ipath, "\\" NAME ".ini");
    // look for config file in same directory as binary or ..\CFGDIR
    if (ini_parse(gb.ipath, ini_handler, &config) < 0) {
      fileExt = strrchr(gb.dpath, '\\'); *fileExt = 0;
      sprintf(gb.cpath, "%s\\" CFGDIR, gb.dpath);
      strcpy(gb.ipath, gb.cpath);
      strcat(gb.ipath, "\\" NAME ".ini");
      if (ini_parse(gb.ipath, ini_handler, &config) < 0) {
        printf("%s can not load configuration file: '" NAME ".ini' in . or %s\r\n", gb.cpath);
        exit(1);
      } else {
        sprintf(gb.lpath, "%s\\" LOGDIR, gb.dpath);
        sprintf(gb.tpath, "%s\\" TMPDIR, gb.dpath);
      }
    } else {
      strcpy(gb.lpath, gb.dpath); strcpy(gb.tpath, gb.dpath);
    }
  } else { exit(1); }

  if (result && osvi.dwPlatformId >= VER_PLATFORM_WIN32_NT) {

    path.bin = gb.bpath; path.cfg = gb.cpath; path.dir = gb.dpath;
    path.exe = gb.epath; path.ini = gb.ipath; path.log = gb.lpath;
    path.tmp = gb.tpath; path.lfn = gb.lfname;

    sprintf(path.lfn, "%s\\" NAME "-%%Y%%m%%d.log", path.log);

    // CE() = default security descriptor, ManualReset, Signalled, object name
    if ((ge.file = CreateEvent(NULL, FALSE, TRUE, TEXT(NAME "FileEvent"))) == NULL)
      printf(NAME ": createEvent error %d\n", GetLastError());
    else if (GetLastError() == ERROR_ALREADY_EXISTS) {
      logMesg("CreateEvent opened an existing event\nServer may already be running", LOG_INFO);
      exit(1);
    }

    if ((ge.log = CreateEvent(NULL, FALSE, TRUE, TEXT(NAME "LogEvent"))) == NULL)
      printf(NAME ": createEvent error %d\n", GetLastError());
    else if (GetLastError() == ERROR_ALREADY_EXISTS) {
      logMesg("CreateEvent opened an existing Event\nServer May already be Running", LOG_INFO);
      exit(1);
    }

    if (argc > 1 && lstrcmpi(argv[1], TEXT("-i")) == 0) {
      installService();
    } else if (argc > 1 && lstrcmpi(argv[1], TEXT("-u")) == 0) {
      uninstallService();
    } else if (argc > 1 && lstrcmpi(argv[1], TEXT("-v")) == 0) {
      SC_HANDLE serviceControlManager = OpenSCManager(0, 0, SC_MANAGER_CONNECT);
      bool serviceStopped = true;
      if (serviceControlManager) {
        SC_HANDLE service = OpenService(serviceControlManager, gb.serviceName, SERVICE_QUERY_STATUS | SERVICE_STOP);
	      if (service) {
   	      serviceStopped = stopService(service);
	        CloseServiceHandle(service);
	      }
	      CloseServiceHandle(serviceControlManager);
      }
      if (serviceStopped) {
        config.verbose = true;
        logMesg("Starting " NAME "..", LOG_NOTICE);
        netInit();
        while (1) { runThreads(); Sleep(THREAD_TO); }
      } else printf("Failed to Stop Service\n");
    } else { runService(); }
    } else if (argc == 1 || lstrcmpi(argv[1], TEXT("-v")) == 0) {
        config.verbose = true;
        logMesg("Starting " NAME "..", LOG_NOTICE);
        netInit();
        while (1) { runThreads(); Sleep(THREAD_TO); }
  } else printf(NAME " requires Windows XP or newer, exiting\n");

  CloseHandle(ge.file);
  CloseHandle(ge.log);
  return 0;
}
