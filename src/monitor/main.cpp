// monitor/main.cpp
// a recording system of how often we actually see the interface vs seeing the mac
// this tells us where any problem lies in communication
// save and report this information to an url

#if MONITOR

#include "core.h"
#include "net.h"
#include "monitor.h"

namespace monitor {

Monitor mon;
LocalBuffers lb;
LocalData ld;

void cleanup (int et) {

  if (et) {
    gd.running[MONITOR_IDX] = false;
    logMesg("Monitor stopped", LOG_INFO);
    pthread_exit(NULL);
  } else {
    logMesg("Monitor cleaned up", LOG_INFO);
    return;
  }
}

void stop() {
  if (config.monitor && gd.running[MONITOR_IDX]) {
    gd.running[MONITOR_IDX] = false;
    cleanup(0);
  }
}

void start() {
  if (config.monitor && !gd.running[MONITOR_IDX])
    pthread_create(&gd.threads[MONITOR_TIDX], NULL, main, NULL);
}

//todo: http

void __cdecl init(void* arg) {

  char buff[100];

  do {

    if (getMacAddress(mon.mac, config.monip) == NO_ERROR) {
      IFAddr2String(buff, mon.mac, sizeof(mon.mac));
      sprintf(lb.log, "MONITOR found attached device with mac: %s", buff);
      logMesg(lb.log, LOG_INFO);
      mon.macfound = true;
    } else {
      logMesg("MONITOR no attached device responding yet", LOG_INFO);
      mon.macfound = false;
    }

    Sleep(60000);

  } while (gd.running[MONITOR_IDX]);

  _endthread();
  return;
}

void* main(void* arg) {

  gd.running[MONITOR_IDX] = true;

  logMesg("MONITOR starting", LOG_INFO);

  _beginthread(init, 0, NULL);

  do {

    net.busy[MONITOR_IDX] = false;

    if (!net.ready[MONITOR_IDX]) { Sleep(1000); continue; }

    // record date of found
    if (getAdapterData()) {
      sprintf(lb.log, "MONITOR adapter with description \"%s\" found", config.ifname);
      logMesg(lb.log, LOG_INFO);
      if (!adptr.ipset) {
        setAdptrIP(); core::stopThreads();
      }
      core::startThreads();
    // record date of not found
    } else {
      sprintf(lb.log, "MONITOR waiting for adapter with description \"%s\" to be available", config.ifname);
      logMesg(lb.log, LOG_INFO);
    }

  } while (gd.running[MONITOR_IDX]);

  cleanup(1);
}

}
#endif
