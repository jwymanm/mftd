#if MONITOR

#define MONITOR_TIDX 0

#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3
#define MON_TO THREAD_TO

extern "C" bool monitor_running;

void startMonitor();
void stopMonitor();

int monitor_cleanup (int exitthread);

void *monitor(void *arg);
void monitorLoop();

#endif
