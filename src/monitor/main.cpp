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
bool foundmac = false;
unsigned char monmac[6];

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

      foundmac = false;

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
      if (!foundmac) {
        debug(0, "Monitor: Looking for mac on ip: ", (void *) config.monip);
        const char *testip = "192.168.200.2";
        getMacAddress(monmac, testip);
        int i=0, j=0; for (; i < sizeof(monmac); i++) { if (monmac[i] != 0x00) j++; }
        if (j) foundmac = true;
        else debug(0,"\r\nMonitor: unknown/zeroed mac address for ip ", (void *) config.monip);
      } else {
        printf("\r\nDEBUG: Monitor: %s has mac address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\r\n",
          config.monip,monmac[0],monmac[1],monmac[2],monmac[3],monmac[4],monmac[5]);
      }
    }
  } else {
    stopThreads();
    debug(0, "Waiting on adapter", NULL);
  }

}
