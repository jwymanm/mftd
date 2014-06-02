#if MONITOR

#define MONITOR_TIDX 0

#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3
#define MON_TO THREAD_TO * 2

extern "C" bool monitor_running;

namespace monitor {

typedef struct {
  int macfound;
  unsigned char mac[6];
} Monitor;

typedef struct {
  char log[256];
} LocalBuffers;

extern "C" Monitor mon;

void start();
void stop();
int cleanup (int et);
void *main(void *arg);
DWORD getMacAddress(unsigned char* mac, const char* ip);

}
#endif
