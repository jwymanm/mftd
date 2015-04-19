// monitor.h

#if MONITOR

#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3
#define PROCESSMATCH "explorer.exe"

namespace monitor {

typedef struct {
  char name[MAX_NAME];
  wchar_t wname[MAX_NAME];
  char fname[MAX_NAME];
  wchar_t wfname[MAX_NAME];
  char email[MAX_NAME];
  char sid[MAX_NAME];
} User;

typedef struct {
  char domain[MAX_NAME];
  wchar_t wdomain[MAX_NAME*2];
  char hostname[MAX_NAME];
} Computer;

typedef struct {
  char desc[MAX_NAME];
  bool found;
} Adapter;

typedef struct {
  BYTE mac[6];
  char ip[50];
  bool found;
} Device;

typedef struct {
  User user;
  Computer comp;
  Adapter adptr;
  Device dev;
} Monitor;

typedef struct {
  char log[256];
} LocalBuffers;

typedef struct {
  char* sn;
  bool* ir;
  bool* ib;
  bool* nr;
  int* fc;
} LocalData;

extern "C" Monitor mon;

// net.cpp

DWORD getDevMacAddress(unsigned char* mac, const char* ip);

// main.cpp
void cleanup(int et);
void stop();
void start();
bool buildSP(void* arg);
void __cdecl init(void* arg);
void* main(void* arg);

}
#endif
