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
  const char* ipaddr;
  const char* netmask;
  const char* dnsipaddr;
  const char* host;
  int rport;
  int lport;
  const char* logging;
} configuration;


/* core */
extern "C" char serviceName[];
extern "C" char displayName[];
extern "C" configuration config;
char* strsep(char** stringp, const char* delim);
int debug(int cond, const char* xstr, void* data);
int main(int argc, char* argv[]);

/* monitor module */
extern "C" bool adptr_exist;
extern "C" bool adptr_ipset;
extern "C" char adptr_ip[255];
extern DWORD adptr_idx;

/* fdns module */
extern "C" bool fdns_running;
extern "C" int fdns_sd;

/* tunnel module */
extern "C" bool tunnel_alive;
extern "C" struct struct_rc rc;

/* dhcp module */
extern bool verbatim;
extern bool dhcp_running;
void* dhcp(void* arg);

