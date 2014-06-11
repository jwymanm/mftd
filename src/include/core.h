// core.h

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>
#include <tchar.h>
#include <ws2tcpip.h>
#include <limits.h>
#include <iphlpapi.h>
#include <process.h>
#include <math.h>
#include <signal.h>
#include <windows.h>
#include <pthread.h>
#include <shlwapi.h>

#include <iostream>
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

#define MAXCFGSIZE 510

#define MAX_SERVERS 125

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
#define MATCH(s, n) strcasecmp(section, s) == 0 && strcasecmp(name, n) == 0

#define LOG_NONE 0
#define LOG_NOTICE 1
#define LOG_INFO 2
#define LOG_DEBUG 3

#define SERVICETOTAL 5
#define SERVICECOUNT MONITOR + FDNS + TUNNEL + DHCP + HTTP
#define SERVICENAMES { "MONITOR", "FDNS", "TUNNEL", "DHCP", "HTTP" }

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
  bool monitor;
  bool fdns;
  bool tunnel;
  bool dhcp;
  bool http;
  int logging;
  const char* ifname;
  const char* adptrip;
  const char* netmask;
  bool bindonly;
  const char* monip;
  const char* monurl;
  const char* moncfgurl;
  const char* fdnsip;
  const char* host;
  int rport;
  int lport;
  const char* httpaddr;
  const char* httpclient;
  const char* htmltitle;
} GConfiguration;

typedef struct {
  bool verbose;
  bool service;
  bool exit;
  bool adptr;
  bool adptrdhcp;
} GSetting;

extern "C" GData gd;
extern "C" GEvents ge;
extern "C" GPaths path;
extern "C" GConfiguration config;
extern "C" GSetting gs;

void debug(int level, const char* xstr, void* mesg);
void showError(DWORD enumber);
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
  char *mesg;
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
