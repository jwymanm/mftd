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

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
#define MATCH(s, n) strcasecmp(section, s) == 0 && strcasecmp(name, n) == 0

#define DEBUG_IP 1
#define DEBUG_ADPTR 2
#define DEBUG_SE 10

#define THREAD_TO 10000
#define THREADCOUNT MONITOR + FDNS + TUNNEL + DHCP

using namespace std;

/* all modules */
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
  const char* cfgfn;
} configuration;

/* core */
extern "C" char serviceName[];
extern "C" char displayName[];
extern "C" configuration config;
extern "C" pthread_t threads[];
void startThreads();
void stopThreads();
void runThreads();
char* strsep(char** stringp, const char* delim);
int debug(int cond, const char* xstr, void* data);
int main(int argc, char* argv[]);
