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

extern "C" Monitor mon;

int cleanup (int et);
void start();
void stop();
void *main(void *arg);
void doLoop();
DWORD getMacAddress(unsigned char* mac, const char* ip);

}

#endif
