// core.h

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <pthread.h>

// Windows
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <wtsapi32.h>
#include <iphlpapi.h>
#include <shlwapi.h>
#include <dsgetdc.h>
#include <process.h>
#include <psapi.h>
#include <tchar.h>
#include <sddl.h>
#include <lm.h>

#include <iostream>
#include <fstream>
#include <string>
#include <map>

#define MYBYTE UCHAR
#define MYWORD USHORT
#define MYDWORD UINT
#define MYWIDE ULONGLONG
#define NBSP 32

#if defined(_WIN64) || defined(_M_ALPHA)
#define MAX_NATURAL_ALIGNMENT sizeof(ULONGLONG)
#else
#define MAX_NATURAL_ALIGNMENT sizeof(DWORD)
#endif

#define MAX_NAME 256
#define MAX_SERVERS 125
#define MAX_CFGSIZE 510

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
#define MATCH(s,n) strcasecmp(section, s) == 0 && strcasecmp(name, n) == 0
#define LM(n,...) sprintf(lb.log, __VA_ARGS__), logMesg(lb.log, n);
#define LSM(n,s,...) sprintf(lb.log, "%s " s, ld.sn, ##__VA_ARGS__), logMesg(lb.log, n);

#define LOG_NONE 0
#define LOG_NOTICE 1
#define LOG_INFO 2
#define LOG_DEBUG 3

#define SERVICETOTAL 5
#define SERVICECOUNT MONITOR + FDNS + TUNNEL + DHCP + HTTP
#define SERVICENAMES { "MONITOR", "FDNS", "TUNNEL", "DHCP", "HTTP" }
#define SERVICETOSECS 20
#define SERVICETOMSECS 0
#define SERVICESTART(x) \
  ld.ir = &gd.running[x]; *ld.ir = true;\
  ld.ib = &net.busy[x];\
  ld.nr = &net.ready[x];\
  ld.fc = &net.failureCounts[x];\
  ld.sn = gd.serviceNames[x];\
  LSM(LOG_INFO, "starting");\
  _beginthread(init, 0, 0);\
  int i; fd_set readfds; timeval tv = { SERVICETOSECS, SERVICETOMSECS };\
  do {\
    *ld.ib = false;\
    if (!*ld.nr) { Sleep(1000); continue; }
#define SERVICEEND } while (*ld.ir); cleanup(1);

#define MONITOR_IDX 0
#define FDNS_IDX 1
#define TUNNEL_IDX 2
#define DHCP_IDX 3
#define HTTP_IDX 4

#define MONITOR_TIDX 0
#define FDNS_TIDX MONITOR_TIDX + MONITOR
#define TUNNEL_TIDX FDNS_TIDX + FDNS
#define DHCP_TIDX TUNNEL_TIDX + TUNNEL
#define HTTP_TIDX DHCP_TIDX + DHCP

typedef struct {
  char serviceName [_MAX_PATH + 1],
       displayName [_MAX_PATH + 1],
       * serviceNames [SERVICETOTAL],
       logFN [_MAX_PATH + 1],
       bpath [_MAX_PATH + 1], cpath [_MAX_PATH + 1],
       dpath [_MAX_PATH + 1], epath [_MAX_PATH + 1],
       ipath [_MAX_PATH + 1], lpath [_MAX_PATH + 1],
       tpath [_MAX_PATH + 1], lfname [_MAX_PATH + 1];
  bool running[SERVICETOTAL];
  pthread_t threads[SERVICECOUNT];
} GData;

typedef struct {
  HANDLE file;
  HANDLE log;
  OVERLAPPED dCol;
} GEvents;

typedef struct {
  const char* bin; // directory that contains executable
  const char* cfg; // directory that contains config
  const char* dir; // top level directory
  const char* exe; // executable
  const char* ini; // config file
  const char* log; // log directory
  char* lfn;       // log file
  const char* tmp; // temp directory
} GPaths;

typedef struct {

// services
  bool monitor;
  bool fdns;
  bool tunnel;
  bool dhcp;
  bool http;

// logging
  int logging;

// adapter
  const char* adptrdesc;
  const char* adptrdescf;
  const char* adptrset;
  const char* adptrmode;
  const char* adptrip;
  const char* adptrnm;
  bool adptrbo;

// monitor
  const char* monip;
  const char* monurl;
  const char* moncfgurl;

// fdns
  const char* fdnsip;

// tunnel
  const char* host;
  int rport;
  int lport;

// http
  const char* httpaddr;
  const char* httpclient;
  const char* htmltitle;
} GConfiguration;

typedef struct {
  bool verbose;
  bool service;
  bool exit;
  bool adapter;
} GSetting;

extern "C" GData gd;
extern "C" GEvents ge;
extern "C" GPaths path;
extern "C" GConfiguration config;
extern "C" GSetting gs;

void debug(int level, const char* xstr, void* mesg);
void showError(char* sname, DWORD errn);
void showSockError(char* sname, DWORD errn);
void logMesg(char* mesg, int level);
void startupMesg();
void __cdecl logThread(void* arg);
int main(int argc, char* argv[]);

namespace core {

typedef struct {
  char log[256];
  char tmp[512];
  char ext[512];
  time_t t;
} LocalBuffers;

typedef struct {
  SERVICE_STATUS serviceStatus;
  SERVICE_STATUS_HANDLE serviceStatusHandle;
  HANDLE stopServiceEvent;
} LocalData;

typedef struct {
  char* mesg;
  int level;
} LogData;

extern "C" LocalBuffers lb;
extern "C" LocalData ld;

void startThreads();
void stopThreads();
void __cdecl threadLoop(void* arg);
void WINAPI ServiceControlHandler(DWORD controlCode);
void WINAPI ServiceMain(DWORD, TCHAR* []);
void runService();
bool stopService(SC_HANDLE service);
void installService();
void uninstallService();

}
