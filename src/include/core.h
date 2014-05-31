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

/* all modules */

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
#define MATCH(s, n) strcasecmp(section, s) == 0 && strcasecmp(name, n) == 0

#define LOG_NONE 0
#define LOG_NOTICE 1
#define LOG_INFO 2
#define LOG_DEBUG 3

#define DEBUG_IP 1
#define DEBUG_ADPTR 2
#define DEBUG_SE 10

#define THREAD_TO 5000
#define THREADCOUNT MONITOR + FDNS + TUNNEL + DHCP

typedef struct {
  char serviceName [_MAX_PATH + 1],
       displayName [_MAX_PATH + 1],
       logFN [_MAX_PATH + 1],
       bpath [_MAX_PATH + 1 ], cpath [_MAX_PATH + 1 ],
       dpath [_MAX_PATH + 1 ], epath [_MAX_PATH + 1 ],
       ipath [_MAX_PATH + 1 ], lpath [_MAX_PATH + 1 ],
       tpath [_MAX_PATH + 1 ], lfname [_MAX_PATH + 1 ];
  WSADATA wsa;
} Buffers;

typedef struct {
  HANDLE file;
  HANDLE log;
} Events;

typedef struct {
  const char* bin; // directory that contains executable
  const char* cfg; // directory that contains config
  const char* dir; // top level directory
  const char* exe; // executable
  const char* ini; // config file
  const char* log; // log directory
  char* lfn; // log file
  const char* tmp; // temp directory
} Paths;

typedef struct {
  bool monitor;
  bool fdns;
  bool tunnel;
  bool dhcp;
  const char* ifname;
  const char* adptrip;
  const char* netmask;
  bool setstatic;
  const char* monip;
  const char* monurl;
  const char* fdnsip;
  const char* host;
  int rport;
  int lport;
  int logging;
  bool verbose;
} Configuration;

namespace core {
  typedef struct {
    char log[256];
    char tmp[512];
    char ext[512];
    time_t t;
  } LocalBuffers;
}

/* core exports */

extern "C" Buffers gb;
extern "C" Events ge;
extern "C" Paths path;
extern "C" Configuration config;
extern "C" pthread_t threads[];

char* cloneString(char *string);
char* strsep(char** stringp, const char* delim);
void debugl(const char *);
int debug(int cond, const char* xstr, void* data);
void __cdecl logThread(void *lpParam);
void logMesg(char *logBuff, int LogLevel);
void startThreads();
void stopThreads();
void runThreads();
int main(int argc, char* argv[]);
