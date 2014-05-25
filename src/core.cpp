/*
 core data and routines
*/

#include "core.h"
#include "ini.h"
#include "net.h"
#include "monitor.h"
#include "fdns.h"
#include "tunnel.h"
#include "dhcp.h"

//#include <boost/thread/thread.hpp>

char serviceName[] = SERVICE_NAME;
char displayName[] = SERVICE_DISPLAY_NAME;

SERVICE_STATUS serviceStatus;
SERVICE_STATUS_HANDLE serviceStatusHandle = 0;

HANDLE stopServiceEvent = 0;

pthread_t threads[THREADCOUNT];

configuration config;

/* helpers */

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
    string str = "DEBUG: ";
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
    cout << endl << str << endl;
  }
}

void startThreads() {
#if FDNS
  if (!fdns_running) {
    pthread_create (&threads[FDNS_TIDX], NULL, fdns, NULL);
  }
#endif
#if TUNNEL
  if (!tunnel_running) {
    pthread_create (&threads[TUNNEL_TIDX], NULL, tunnel, NULL);
  }
#endif
#if DHCP
  if (!dhcp_running) {
    pthread_create (&threads[DHCP_TIDX], NULL, dhcp, NULL);
  } 
#endif
}

void stopThreads() {
#if FDNS
  if (fdns_running) {
    debug(0, "Stopping fdns", NULL);
    fdns_running = false;
    Sleep(2000);
  }
#endif
#if TUNNEL
  if (tunnel_running) {
    debug(0, "Stopping tunnel", NULL);
    tunnel_running = false;
    Sleep(2000);
  }
#endif
#if DHCP
  if (dhcp_running) {
    debug(0, "Stopping dhcp", NULL);
    dhcp_running = false;
    Sleep(2000);
  }
#endif
}

void runThreads() {
#if MONITOR
  startMonitor();
  while (1) { monitorLoop(); };
#else
  // if not monitoring then just attempt to set adapter ip if/once it is available and then run threads
  // mostly useful for running to provide simple services on an as-needed basis TODO: add DHCP only flag?
  if (getAdaptrInfo() && !adptr_exist) {
    adptr_exist = true;
    Sleep(5000);
    if (strcmp(adptr_ip, config.adptrip) != 0) {
      setAdptrIP();
      Sleep(3000);
    }
    startThreads();
  }
#endif
}

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
  serviceStatusHandle = RegisterServiceCtrlHandler(serviceName, ServiceControlHandler);

  if (serviceStatusHandle) {
    serviceStatus.dwCurrentState = SERVICE_START_PENDING;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);

    stopServiceEvent = CreateEvent(0, FALSE, FALSE, 0);
    serviceStatus.dwControlsAccepted |= (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
    serviceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);

#if MONITOR
    startMonitor();
    do { monitorLoop(); }
#else 
    do { runThreads(); Sleep(10000); }
#endif
    while (WaitForSingleObject(stopServiceEvent, 0) == WAIT_TIMEOUT);

    stopThreads();

#if MONITOR
    stopMonitor();
#endif

    serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
    serviceStatus.dwCheckPoint = 2;
    serviceStatus.dwWaitHint = 1000;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);

    Sleep(2000); 

    WSACleanup();

    serviceStatus.dwControlsAccepted &= ~(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
    serviceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);

    CloseHandle(stopServiceEvent);
    stopServiceEvent = 0;
    return;
  }
}

void runService() {
  SERVICE_TABLE_ENTRY serviceTable[] = { {serviceName, ServiceMain}, {0, 0} };
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
  SC_HANDLE serviceControlManager =
    OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE | SERVICE_START);

  if (serviceControlManager) {
    TCHAR path[ _MAX_PATH + 1 ];
    if (GetModuleFileName(0, path, sizeof(path) / sizeof(path[0])) > 0) {
      SC_HANDLE service =
        CreateService(serviceControlManager, serviceName, displayName,
	SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
	SERVICE_AUTO_START, SERVICE_ERROR_IGNORE, path, 0, 0, 0, 0, 0);
      if (service) {
        printf("Successfully installed.. !\n");
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
    SC_HANDLE service =
      OpenService(serviceControlManager, serviceName, SERVICE_QUERY_STATUS | SERVICE_STOP | DELETE);
    if (service) {
      if (stopService(service)) {
        if (DeleteService(service)) printf("Successfully Removed !\n");
        else showError(GetLastError());
      } else printf("Failed to Stop Service..\n");
      CloseServiceHandle(service);
    } else printf("Service Not Found..\n");
    CloseServiceHandle(serviceControlManager);
  }
  return;
}

int main(int argc, char* argv[]) {

  OSVERSIONINFO osvi;
  osvi.dwOSVersionInfoSize = sizeof(osvi);
  bool result = GetVersionEx(&osvi);
  TCHAR path [ _MAX_PATH + 1 ] = "", rpath [_MAX_PATH + 1 ] = "";
  char *fileExt;

  if (GetModuleFileName(0, path, sizeof(path) / sizeof(path[0])) > 0) {
    PathRemoveFileSpec(path);
    strcat(rpath, path);
    strcat(path, "\\" NAME ".ini");
    fileExt  = strrchr(rpath, '\\');
    *fileExt = 0;
    strcat(rpath, "\\" CFGDIR "\\" NAME ".ini");
    // look for config file in same directory as binary or ..\CFGDIR
    if (ini_parse(path, ini_handler, &config) < 0) {
      if (ini_parse(rpath, ini_handler, &config) < 0) {
        printf("Can't load configuration file: '" NAME ".ini'\r\nSearch paths: \r\n\t%s\r\n\t%s", path, rpath); exit(1);
      } else config.cfgfn = rpath;
    } else config.cfgfn = path;
  } else { exit(1); }

  if (result && osvi.dwPlatformId >= VER_PLATFORM_WIN32_NT) {

    if (argc > 1 && lstrcmpi(argv[1], TEXT("-i")) == 0) {
      installService();
    } else if (argc > 1 && lstrcmpi(argv[1], TEXT("-u")) == 0) {
      uninstallService();
    } else if (argc > 1 && lstrcmpi(argv[1], TEXT("-v")) == 0) {
      SC_HANDLE serviceControlManager = OpenSCManager(0, 0, SC_MANAGER_CONNECT);
      bool serviceStopped = true;

      if (serviceControlManager) {
        SC_HANDLE service = OpenService(serviceControlManager, serviceName, SERVICE_QUERY_STATUS | SERVICE_STOP);

	if (service) {
   	  serviceStopped = stopService(service);
	  CloseServiceHandle(service);
	}

	CloseServiceHandle(serviceControlManager);
      }

      if (serviceStopped) {
        verbatim = true; runThreads();
      } else printf("Failed to Stop Service\n");
    } else { runService(); }
    } else if (argc == 1 || lstrcmpi(argv[1], TEXT("-v")) == 0) {
      verbatim = true; runThreads();
  } else printf("This option is not available on Windows95/98/ME\n");
  return 0; 
}
