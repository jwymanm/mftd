#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3

typedef struct {
  bool exist;
  bool ipset;
  PCHAR name;
  PCHAR fname;
  PWCHAR wfname;
  LPWSTR desc;
  BYTE phyaddr[MAX_ADAPTER_ADDRESS_LENGTH];
  DWORD phyaddrlen;
  DWORD idx4;
  DWORD idx6;
} Adapter;

typedef struct {
  bool ready;
  bool busy;
  char hostname[128];
  int failureCounts[5];
  MYDWORD allServers[MAX_SERVERS];
  MYDWORD listenServers[MAX_SERVERS];
  MYDWORD listenMasks[MAX_SERVERS];
} Network;

struct NET4Address {
  union {
   unsigned ip:32;
   MYBYTE octate[4];
  };
};

struct ConnType {
  SOCKET sock;
  SOCKADDR_IN addr;
  MYDWORD server;
  MYWORD port;
  bool loaded;
  bool ready;
};

extern "C" Adapter adptr;
extern "C" Network net;

void IFAddrToString(char* buff, BYTE* phyaddr, DWORD len);
bool isIP(const char *str);
char *IP2String(char *target, MYDWORD ip);
char *IP62String(char *target, MYBYTE *source);
bool checkMask(MYDWORD mask);
MYDWORD calcMask(MYDWORD rangeStart, MYDWORD rangeEnd);
MYDWORD getClassNetwork(MYDWORD ip);
void getHostName(char *hn);
bool getAdapterData();
int setAdptrIP();
bool detectChange();
MYDWORD *findServer(MYDWORD* array, MYBYTE cnt, MYDWORD ip);
MYDWORD *addServer(MYDWORD* array, MYBYTE maxServers, MYDWORD ip);
void setServers();
int netInit();
int netExit();
