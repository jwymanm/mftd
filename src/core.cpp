// core.cpp
// core data and routines

#include "core.h"
#include "util.h"
#include "ini.h"
#include "net.h"
#include "fdns.h"
#include "tunnel.h"
#include "dhcp.h"
#include "http.h"
#include "monitor.h"

// Globals
GData gd = {
  SERVICE_NAME, SERVICE_DISPLAY_NAME, SERVICENAMES,
  0, 0, 0, 0, 0, 0, 0, 0, 0
};
GEvents ge;
GPaths path = { gd.bpath, gd.cpath, gd.dpath, gd.epath, gd.ipath, gd.lpath, gd.lfname, gd.tpath };
GConfiguration config;
GSetting gs;

// Locals to core
namespace core {  LocalBuffers lb; LocalData ld; }
using namespace core;

// Functions

// Logging

void debug(int level, const char* xstr, void* mesg) {
  std::string str;
  if (xstr) str += xstr;
  switch (level) {
    case LOG_DEBUG:
      str += "DEBUG  "; break;
    case LOG_INFO:
      str += "INFO   "; break;
    case LOG_NOTICE:
      str += "NOTICE "; break;
    default:
      break;
  }
  if (mesg) str += static_cast<char *>(mesg);
  std::cout << str << std::endl;
  return;
}

void logMesg(char* mesg, int level) {
  if (level <= config.logging) {
    LogData* log = (LogData*) calloc(1, sizeof(LogData));
    if (!log) return;
    log->mesg = cloneString(mesg);
    log->level = level;
    _beginthread(logThread, 0, log);
  }
}

void showError(char* sname, DWORD errn) {
  LPTSTR lpMsgBuf;
  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL, (DWORD) errn, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPTSTR)&lpMsgBuf, 0, NULL);
  if (!sname)
    sprintf(lb.log, "%s", lpMsgBuf);
  else
    sprintf(lb.log, "%s %s", sname, lpMsgBuf);
  logMesg(lpMsgBuf, LOG_NOTICE);
}

void showSockError(char* sname, DWORD errn) {
  LM(LOG_NOTICE, "%s socket error %u", sname, errn);
  showError(sname, errn);
}

void startupMesg() {
  if (config.logging) {
    if (gs.service)
      logMesg(SERVICE_NAME " service starting", LOG_NOTICE);
    else
      logMesg(NAME " starting", LOG_NOTICE);
    const char* lmesg = "log level %s";
    if (config.logging == LOG_NOTICE)
      sprintf(lb.log, lmesg, "NOTICE");
    if (config.logging == LOG_INFO)
      sprintf(lb.log, lmesg, "INFO");
    if (config.logging == LOG_DEBUG)
      sprintf(lb.log, lmesg, "DEBUG");
    logMesg(lb.log, LOG_NOTICE);
  }
}

// Threads

void __cdecl logThread(void* arg) {

  LogData* log = (LogData*) arg;
  char* lfn = gd.logFN;

  WaitForSingleObject(ge.log, INFINITE);
  lb.t = time(NULL);
  tm* ttm = localtime(&lb.t);
  char buff[_MAX_PATH + 1];
  strftime(buff, sizeof(buff), path.lfn, ttm);

  // TODO: max # logs
  if (strcmp(lfn, buff)) {
    if (lfn[0]) {
      FILE* f = fopen(lfn, "at");
      if (f) {
        fprintf(f, "Logging Continued on file %s\n", buff);
        fclose(f);
      }
      strcpy(lfn, buff);
      f = fopen(lfn, "at");
      if (f) {
        fprintf(f, "%s\n\n", gd.displayName);
        fclose(f);
      }
    }
    strcpy(lfn, buff);
  }

  FILE* f = fopen(lfn, "at");

  if (gs.verbose) {
    strftime(buff, sizeof(buff), " %X ", ttm);
    debug(log->level, buff, (void* ) log->mesg);
  }

  if (f) {
    strftime(buff, sizeof(buff), "%d-%b-%y %X", ttm);
    fprintf(f, "[%s] %s\n", buff, log->mesg);
    fclose(f);
  } else config.logging = 0;

  free(log->mesg);
  free(log);
  SetEvent(ge.log);

  _endthread();
  return;
}

namespace core {

void startThreads() {
#if MONITOR
  monitor::start();
#endif
#if FDNS
  fdns::start();
#endif
#if TUNNEL
  tunnel::start();
#endif
#if DHCP
  dhcp::start();
#endif
#if HTTP
  http::start();
#endif
}

void stopThreads() {
#if MONITOR
  monitor::stop();
#endif
#if FDNS
  fdns::stop();
#endif
#if TUNNEL
  tunnel::stop();
#endif
#if DHCP
  dhcp::stop();
#endif
#if HTTP
  http::stop();
#endif
}

void __cdecl threadLoop(void* arg) {

  startThreads();

  while (detectChange()) Sleep(1000);

  if (gs.service) _endthread();
  return;
}

// service code

void WINAPI ServiceControlHandler(DWORD controlCode) {
  switch (controlCode) {
    case SERVICE_CONTROL_INTERROGATE:
      break;
    case SERVICE_CONTROL_SHUTDOWN:
    case SERVICE_CONTROL_STOP:
      ld.serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
      ld.serviceStatus.dwWaitHint = 20000;
      ld.serviceStatus.dwCheckPoint = 1;
      SetServiceStatus(ld.serviceStatusHandle, &ld.serviceStatus);
      SetEvent(ld.stopServiceEvent);
      return;
    case SERVICE_CONTROL_PAUSE:
      break;
    case SERVICE_CONTROL_CONTINUE:
      break;
    default:
      if (controlCode >= 128 && controlCode <= 255) break;
      else break;
  }
  SetServiceStatus(ld.serviceStatusHandle, &ld.serviceStatus);
}

void WINAPI ServiceMain(DWORD, TCHAR* []) {

  ld.serviceStatus.dwServiceType = SERVICE_WIN32;
  ld.serviceStatus.dwCurrentState = SERVICE_STOPPED;
  ld.serviceStatus.dwControlsAccepted = 0;
  ld.serviceStatus.dwWin32ExitCode = NO_ERROR;
  ld.serviceStatus.dwServiceSpecificExitCode = NO_ERROR;
  ld.serviceStatus.dwCheckPoint = 0;
  ld.serviceStatus.dwWaitHint = 0;
  ld.serviceStatusHandle = RegisterServiceCtrlHandler(gd.serviceName, ServiceControlHandler);

  if (ld.serviceStatusHandle) {
    ld.serviceStatus.dwCurrentState = SERVICE_START_PENDING;
    SetServiceStatus(ld.serviceStatusHandle, &ld.serviceStatus);
    ld.stopServiceEvent = CreateEvent(0, FALSE, FALSE, 0);
    ld.serviceStatus.dwControlsAccepted |= (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
    ld.serviceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(ld.serviceStatusHandle, &ld.serviceStatus);
    gs.service = true;
    startupMesg();
    netInit();
    _beginthread(threadLoop, 0, NULL);
    while (WaitForSingleObject(ld.stopServiceEvent, 0) == WAIT_TIMEOUT) Sleep(1000);
    logMesg(SERVICE_NAME " stopping", LOG_NOTICE);
    gs.exit = true;
    stopDC();
    Sleep(1000);
    stopThreads();
    ld.serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
    ld.serviceStatus.dwCheckPoint = 2;
    ld.serviceStatus.dwWaitHint = 1000;
    SetServiceStatus(ld.serviceStatusHandle, &ld.serviceStatus);
    netExit();
    logMesg(SERVICE_NAME " stopped", LOG_NOTICE);
    ld.serviceStatus.dwControlsAccepted &= ~(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
    ld.serviceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(ld.serviceStatusHandle, &ld.serviceStatus);
    CloseHandle(ld.stopServiceEvent);
    ld.stopServiceEvent = 0;
    return;
  }
}

void runService() {
  SERVICE_TABLE_ENTRY serviceTable[] = { {gd.serviceName, ServiceMain}, {0, 0} };
  StartServiceCtrlDispatcher(serviceTable);
}

bool stopService(SC_HANDLE service) {
  if (service) {
    QueryServiceStatus(service, &ld.serviceStatus);
    if (ld.serviceStatus.dwCurrentState != SERVICE_STOPPED) {
      ControlService(service, SERVICE_CONTROL_STOP, &ld.serviceStatus);
      printf("Stopping Service.");
      for (int i = 0; i < 100; i++) {
        QueryServiceStatus(service, &ld.serviceStatus);
        if (ld.serviceStatus.dwCurrentState == SERVICE_STOPPED) {
          printf("Stopped\r\n"); return true;
        } else { Sleep(500); printf("."); }
      }
      printf("Failed\r\n"); return false;
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
    } else { showError(NULL, GetLastError()); }
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
        else showError(NULL, GetLastError());
      } else printf("Failed to stop service %s\n", SERVICE_NAME);
      CloseServiceHandle(service);
    } else printf("Service %s not found\n", SERVICE_NAME);
    CloseServiceHandle(serviceControlManager);
  }
  return;
}

}

int main(int argc, char* argv[]) {

  OSVERSIONINFO osvi;
  osvi.dwOSVersionInfoSize = sizeof(osvi);
  bool result = GetVersionEx(&osvi);
  char *fileExt;

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
        printf("%s can not load configuration file: '" NAME ".ini' in . or %s\r\n", NAME, gd.cpath);
        exit(1);
      } else {
        sprintf(gd.lpath, "%s\\" LOGDIR, gd.dpath);
        sprintf(gd.tpath, "%s\\" TMPDIR, gd.dpath);
      }
    } else {
      strcpy(gd.lpath, gd.dpath);
      strcpy(gd.tpath, gd.dpath);
    }
  } else { exit(1); }

  if (result && osvi.dwPlatformId >= VER_PLATFORM_WIN32_NT) {
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
        gs.verbose = true; startupMesg(); netInit(); threadLoop(NULL);
      } else printf("Failed to stop service\n");
    } else { runService(); }
    } else if (argc == 1 || lstrcmpi(argv[1], TEXT("-v")) == 0) {
        gs.verbose = true; startupMesg(); netInit(); threadLoop(NULL);
  } else printf(NAME " requires Windows XP or newer, exiting\n");
  CloseHandle(ge.file);
  CloseHandle(ge.log);
  return 0;
}
