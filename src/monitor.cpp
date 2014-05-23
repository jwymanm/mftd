
#include "core.h"
#include "net.h"
#include "monitor.h"

void startMonitor() {
  pthread_create(&threads[MONITOR_TIDX], NULL, monitor, NULL);
}

void stopMonitor() {
  pthread_kill(threads[MONITOR_TIDX], SIGINT);
}

void *monitor(void *arg) {

  debug(0, "Looking for adapter: ", (void *) config.ifname);

  while(1) {
    if (getAdptrInfo()) {
      if (!adptr_exist) {
	adptr_exist = true;
	adptr_ipset = false;
      }
    } else { adptr_ipset = false; adptr_exist = false; }
    Sleep(MON_TO);
  }

}

void monitorLoop() {

  Sleep(MON_TO*2);

  if (adptr_exist) {
    if (strcmp(adptr_ip, config.adptrip) != 0) {
      debug(0, "Setting ip..", NULL);
      setAdptrIP();
    } else {
      if (!adptr_ipset) {
        adptr_ipset = true;
        debug(DEBUG_IP, "IP set to: ", (void *)config.adptrip);
        startThreads();
      } else {
        debug(0, "Looking for mac on ip: ", (void *) config.monip);
      //  getMacAddress(, config.monip);
      }
    }
  } else {
    stopThreads();
    debug(0, "Waiting on adapter", NULL);
  }

}
