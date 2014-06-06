#if MONITOR

#define MONITOR_TIDX MONITOR

#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3

extern "C" bool monitor_running;

namespace monitor {

typedef struct {
  int macfound;
  BYTE mac[6];
} Monitor;

typedef struct {
  char log[256];
} LocalBuffers;

extern "C" Monitor mon;

// net.cpp

DWORD getMacAddress(unsigned char* mac, const char* ip);

// main.cpp

void start();
void stop();
int cleanup (int et);
void __cdecl watchDevice(void *args);
void *main(void *args);

}
#endif
