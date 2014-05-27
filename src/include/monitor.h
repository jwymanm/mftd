#if MONITOR

#define MONITOR_TIDX 0

#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3
#define MON_TO THREAD_TO * 2

typedef struct {
  int macfound;
  unsigned char mac[6];
} Monitor;

extern "C" bool monitor_running;
extern "C" Monitor mon;

void startMonitor();
void stopMonitor();
int monitor_cleanup (int exitthread);
void *monitor(void *arg);
void monitorLoop();
DWORD getMacAddress(unsigned char* mac, const char* ip);

#endif
