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

extern "C" Adapter adptr;

int netInit();
int netExit();
int setAdptrIP();
void storeA();
void printIFAddr();
bool getAdapterData();
