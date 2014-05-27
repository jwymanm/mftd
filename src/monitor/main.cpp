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

Monitor mon;

void startMonitor() {
  pthread_create(&threads[MONITOR_TIDX], NULL, monitor, NULL);
}

void stopMonitor() {
  monitor_running = false;
}

int monitor_cleanup (int exitthread) {
  if (exitthread) {
    WSACleanup();
    pthread_exit(NULL);
  } else return 0;
}

void *monitor(void *arg) {

  monitor_running = true;

  WSADATA wsaData;

  if (WSAStartup(MAKEWORD(1,1), &wsaData) != 0) {
     debug(0, "Monitor: WSAError", NULL);
     monitor_running = false;
     pthread_exit(NULL);
  }

  debug(0, "Looking for adapter: ", (void *) config.ifname);

  do {

    // record date of found
    if (getAdapterData()) {
     

    // record date of not found
    } else {

      mon.macfound = false;

    }

    Sleep(MON_TO);

  } while (monitor_running);

  monitor_cleanup(1);

}

void monitorLoop() {

  Sleep(MON_TO*2);

  if (adptr.exist) {
    if (!adptr.ipset) {
      debug(0, "Monitor: Setting adapter ip..", NULL);
      setAdptrIP();
    } else {
      startThreads();
      if (!mon.macfound) {
  	int macerr;
        debug(0, "Monitor: Looking for mac on ip: ", (void *) config.monip);
        if ((macerr=getMacAddress(mon.mac, config.monip)) == NO_ERROR) {
          mon.macfound = true;
        } else { 
          debug(0,"\r\nMonitor: error obtaining mac from ", (void *) config.monip);
          debug(0,"Monitor: error code ", (void *) macerr);
        }
      } else {
        printf("\r\nDEBUG: Monitor: %s has mac address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\r\n",
          config.monip,mon.mac[0],mon.mac[1],mon.mac[2],mon.mac[3],mon.mac[4],mon.mac[5]);
      }
    }
  } else {
    stopThreads();
    debug(0, "Waiting on adapter", NULL);
  }

}
