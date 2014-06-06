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
  monitor::cleanup(0);
}

int cleanup (int et) {
  if (et) {
    monitor_running = false;
    logMesg("Monitor stopped", LOG_INFO);
    pthread_exit(NULL);
  } else {
    logMesg("Monitor cleaned up", LOG_INFO);
    return 0;
  }
}

void watchDevice(void *args) {

  char buff[100];

  do {
    if (!net.ready) { Sleep(4000); continue; }
    if (!monitor_running) break;

    if (getMacAddress(mon.mac, config.monip) == NO_ERROR) {
      IFAddrToString(buff, mon.mac, sizeof(mon.mac));
      sprintf(lb.log, "Monitor: found attached device with mac: %s", buff);
      logMesg(lb.log, LOG_INFO);
      mon.macfound = true;
    } else {
      logMesg("Monitor: no attached device responding yet", LOG_INFO);
      mon.macfound = false;
    }
    Sleep(60000);
  } while (monitor_running);

  _endthread();
  return;
}

void *main(void *args) {

  monitor_running = true;

  net.failureCounts[MONITOR_IDX] = 0;

  logMesg("Monitor starting", LOG_INFO);

  _beginthread(watchDevice, 0, NULL);

  sprintf(lb.log, "Monitor: Looking for adapter with description \"%s\"", config.ifname);
  logMesg(lb.log, LOG_INFO);

  do {
    // record date of found
    if (getAdapterData()) {
      sprintf(lb.log, "Monitor: Adapter with description \"%s\" found", config.ifname);
      logMesg(lb.log, LOG_INFO);
      if (!adptr.ipset) {
        setAdptrIP(); stopThreads();
      }
      startThreads();
    // record date of not found
    } else {
      sprintf(lb.log, "Monitor: Waiting for adapter with description \"%s\" to be available", config.ifname);
      logMesg(lb.log, LOG_INFO);
      logMesg("Monitor: Stopping threads", LOG_INFO);
      stopThreads();
    }
    detectChange();
  } while (monitor_running);

  cleanup(1);
}

}
#endif
