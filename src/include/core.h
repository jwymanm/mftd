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
#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

#define DEBUG_IP 1
#define DEBUG_ADPTR 2
#define DEBUG_SE 10

#define THREADCOUNT MONITOR + FDNS + TUNNEL + DHCP

using namespace std;

/* all modules */
typedef struct {
  const char* ifname;
  const char* adptrip;
  const char* netmask;
  const char* monip;
  const char* dnsipaddr;
  const char* host;
  const char* cfgfn;
  int rport;
  int lport;
  int logging;
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
