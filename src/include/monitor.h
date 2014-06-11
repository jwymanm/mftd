// monitor.h

#if MONITOR

#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3

namespace monitor {

typedef struct {
  int macfound;
  BYTE mac[6];
} Monitor;

typedef struct {
  char log[256];
} LocalBuffers;

typedef struct {
} LocalData;

extern "C" Monitor mon;
//extern "C" LocalBuffers lb;
//extern "C" LocalData ld;

// net.cpp

DWORD getMacAddress(unsigned char* mac, const char* ip);

// main.cpp
void cleanup(int et);
void stop();
void start();
void __cdecl init(void* arg);
void* main(void* arg);

}
#endif
