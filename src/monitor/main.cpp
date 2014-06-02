#if MONITOR

/*
   monitor/main.cpp
   a recording system of how often we actually see the interface vs seeing the mac
   this tells us where any problem lies in communication
   save and report this information to an url
*/

#include "core.h"
#include "net.h"
#include "monitor.h"

bool monitor_running = false;

namespace monitor {

Monitor mon;
LocalBuffers lb;

void start() {
  pthread_create(&threads[MONITOR_TIDX], NULL, main, NULL);
}

void stop() {
  monitor_running = false;
}

int cleanup (int et) {
  if (et) {
    pthread_exit(NULL);
  } else return 0;
}

// this will be wrapped around our detectchange function instead
// since this is not foolproof
// problems with state after ethernet cord is dropped but before adapter finds that..
// event based will be way better

void *main(void *arg) {

  monitor_running = true;

  sprintf(lb.log, "Monitor starting\r\nLooking for adapter: %s", config.ifname);
  logMesg(lb.log, LOG_INFO);

  do {
    // record date of found
    if (getAdapterData()) {
      if (!adptr.ipset) {
        setAdptrIP();
        stopThreads();
      }
      startThreads();
    // record date of not found
    } else {
      mon.macfound = false;
      stopThreads();
    }
    detectChange();
  } while (monitor_running);

  cleanup(1);
}

}
#endif
