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

#define THREADCOUNT MONITOR + FDNS + TUNNEL + DHCP + HTTP

#define MONITOR_IDX 0
#define FDNS_IDX 1
#define TUNNEL_IDX 2
#define DHCP_IDX 3
#define HTTP_IDX 4

#define LOG_NONE 0
#define LOG_NOTICE 1
#define LOG_INFO 2
#define LOG_DEBUG 3

extern "C" bool running;

typedef struct {
  char serviceName [_MAX_PATH + 1],
       displayName [_MAX_PATH + 1],
       logFN [_MAX_PATH + 1],
       bpath [_MAX_PATH + 1], cpath [_MAX_PATH + 1],
       dpath [_MAX_PATH + 1], epath [_MAX_PATH + 1],
       ipath [_MAX_PATH + 1], lpath [_MAX_PATH + 1],
       tpath [_MAX_PATH + 1], lfname [_MAX_PATH + 1];
  WSADATA wsa;
} Data;

typedef struct {
  HANDLE file;
  HANDLE log;
  HANDLE net;
  OVERLAPPED dCol;
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
  bool http;
  int logging;
  const char* ifname;
  const char* adptrip;
  const char* netmask;
  bool setstatic;
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
  bool verbose;
  bool isService;
  bool isExiting;
} Configuration;

namespace core {

typedef struct {
  char log[256];
  char tmp[512];
  char ext[512];
  time_t t;
} LocalBuffers;

extern "C" LocalBuffers lb;

}

/* core exports */

extern "C" Data gd;
extern "C" Events ge;
extern "C" Paths path;
extern "C" Configuration config;
extern "C" pthread_t threads[];

char* toLower(char* string);
char* toUpper(char* string);
char* hex2String(char* target, MYBYTE* hex, MYBYTE bytes);
char* cloneString(char* string);
char* strsep(char** stringp, const char* delim);
char* getHexValue(MYBYTE* target, char* src, MYBYTE* size);
void wpcopy(PCHAR dest, PWCHAR src);
bool wildcmp(char* string, char* wild);
bool isInt(char* str);
MYWORD myTokenize(char* target, char* src, const char* sep, bool whiteSep);
char* myGetToken(char* buff, MYBYTE index);
char* myTrim(char* target, char* src);
void mySplit(char* name, char* val, const char* src, char splitChar);
int debug(int level, const char* xstr, void* data);
void showError(DWORD enumber);
void logMesg(char* logBuff, int LogLevel);
void __cdecl logThread(void* args);
void startThreads();
void stopThreads();
void __cdecl threadLoop(void* args);
void runService();
void WINAPI ServiceControlHandler(DWORD controlCode);
void WINAPI ServiceMain(DWORD, TCHAR* []);
bool stopService(SC_HANDLE service);
void installService();
int main(int argc, char* argv[]);
