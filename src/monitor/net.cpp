/*

   monitor/net.cpp

   network functions for monitor module

*/


#include "core.h"
#include "net.h"
#include "monitor.h"

DWORD getMacAddress(unsigned char *mac , const char *ip) {

  IPAddr srcip = 0;
  ULONG macAddr[2];
  ULONG phyAddrLen = 6;  /* default to length of six bytes */
  int i;
  struct in_addr destip;

  destip.s_addr = inet_addr(ip);
  //memset(&mac, 0xff, sizeof (mac));

  DWORD ret = SendARP((IPAddr) destip.S_un.S_addr, srcip, macAddr, &phyAddrLen);

  if (phyAddrLen) {
    BYTE *bMacAddr = (BYTE *) & macAddr;
    for (i = 0; i < (int) phyAddrLen; i++) { mac[i] = (char)bMacAddr[i]; }
  }

  return ret;
}

