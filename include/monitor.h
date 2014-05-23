#if MONITOR

#define MONITOR_TIDX 0

#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3
#define MON_TO 5000

void startMonitor();
void stopMonitor();

void *monitor(void *arg);
void monitorLoop();

#endif
