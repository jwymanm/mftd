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
  int failureCounts[4];
} Network;

extern "C" Adapter adptr;
extern "C" Network net;

int netInit();
int netExit();
int setAdptrIP();
bool isIP(char *str);
void IFAddrToString(char* buff, BYTE* phyaddr, DWORD len);
void getHostName(char *hn);
bool getAdapterData();
bool detectChange();
