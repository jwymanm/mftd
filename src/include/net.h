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
  char hostname[128];
  bool refresh;
  bool busy[SERVICETOTAL];
  bool ready[SERVICETOTAL];
  int failureCounts[SERVICETOTAL];
  MYDWORD allServers[MAX_SERVERS];
  MYDWORD listenServers[MAX_SERVERS];
  MYDWORD listenMasks[MAX_SERVERS];
  MYDWORD staticServers[MAX_SERVERS];
  MYDWORD staticMasks[MAX_SERVERS];
  WSADATA wsa;
} Network;

typedef struct {
  SOCKET sock;
  SOCKADDR_IN addr;
  MYDWORD server;
  MYWORD port;
  MYDWORD mask;
  bool loaded;
  bool ready;
} ConnType;

typedef struct {
  union {
    unsigned ip:32;
    MYBYTE octate[4];
  };
} NET4Address;

extern "C" Adapter adptr;
extern "C" Network net;

bool isIP(const char* str);
void IFAddr2String(char* buff, BYTE* phyaddr, DWORD len);
bool isIP(const char* str);
char* IP2String(char* target, MYDWORD ip);
char* IP62String(char* target, MYBYTE* source);
bool checkMask(MYDWORD mask);
MYDWORD calcMask(MYDWORD rangeStart, MYDWORD rangeEnd);
MYDWORD* findServer(MYDWORD* array, MYBYTE cnt, MYDWORD ip);
MYDWORD* addServer(MYDWORD* array, MYBYTE maxServers, MYDWORD ip);
MYDWORD getClassNetwork(MYDWORD ip);
void getHostName(char *hn);
bool getAdapterData();
int setAdptrIP();
void getServerIFs();
void setServerIFs();
void stopDC();
bool dCWait(int idx);
bool detectChange();
int netExit();
int netInit();
