// monitor/main.cpp
// a recording system of how often we actually see the interface vs seeing the mac
// this tells us where any problem lies in communication
// save and report this information to an url

#if MONITOR

#include "core.h"
#include "net.h"
#include "monitor.h"
#include "http.h"

namespace monitor {

Monitor mon;
LocalBuffers lb;
LocalData ld;

void cleanup (int et) {

  if (et) {
    *ld.ir = false;
    Sleep(1000);
    LSM(LOG_NOTICE, "stopped")
    pthread_exit(NULL);
  } else {
    return;
  }
}

void stop() {
  if (config.monitor && gd.running[MONITOR_IDX]) {
    *ld.ir = false;
    LSM(LOG_NOTICE, "stopping")
    cleanup(0);
    Sleep(1000);
  }
}

void start() {
  if (config.monitor && !gd.running[MONITOR_IDX]) {
    pthread_create(&gd.threads[MONITOR_TIDX], NULL, main, NULL);
    Sleep(1000);
  }
}

#if HTTP
bool buildSP(void* arg) {

  HTTP_SPHEAD(0)

  fp += sprintf(fp, "<p>Waiting</p>\n<br/>\n");

  HTTP_SPFOOT

}
#endif

void __cdecl init(void* arg) {

  char buff[100];

  do {

    if (getMacAddress(mon.mac, config.monip) == NO_ERROR) {
      IFAddr2String(buff, mon.mac, sizeof(mon.mac));
      LSM(LOG_INFO, "found attached device with mac %s", buff)
      mon.macfound = true;
    } else {
      LSM(LOG_INFO, "no attached device responding yet")
      mon.macfound = false;
    }

    Sleep(60000);

  } while (*ld.ir);

  _endthread();
  return;
}

void* main(void* arg) {

  SERVICESTART(MONITOR_IDX)

    // record date of found
    if (getAdapterData()) {
      LSM(LOG_INFO, "adapter with description \"%s\" found", config.ifname)
      //core::startThreads();
    // record date of not found
    } else {
      LSM(LOG_INFO, "waiting for adapter with description \"%s\" to be available", config.ifname)
    }

  SERVICEEND

}

}
#endif
