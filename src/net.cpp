/*
 *   Network helper and core routines
*/

#include "core.h"
#include "util.h"
#include "net.h"

Adapter adptr;
Network net;

using namespace core;

void IFAddrToString(char* buff, BYTE* phyaddr, DWORD len) {
  char* bptr = buff;
  for (int i = 0; i < len; i++) {
    if (i == (len - 1))
      sprintf(bptr, "%.2X", phyaddr[i]);
    else {
      sprintf(bptr, "%.2X-", phyaddr[i]);
      bptr+=3;
    }
  }
}

bool isIP(const char* str) {
  if (!str || !(*str)) return false;
  MYDWORD ip = inet_addr(str); int j = 0;
  for (; *str; str++) {
    if (*str == '.' && *(str + 1) != '.') j++;
    else if (*str < '0' || *str > '9') return false;
  }
  if (j == 3) {
    if (ip == INADDR_NONE || ip == INADDR_ANY) return false;
    else return true;
  } else return false;
}

char* IP2String(char* target, MYDWORD ip) {
  NET4Address inaddr;
  inaddr.ip = ip;
  sprintf(target, "%u.%u.%u.%u", inaddr.octate[0], inaddr.octate[1], inaddr.octate[2], inaddr.octate[3]);
  return target;
}

char* IP62String(char* target, MYBYTE* src) {
  char *dp = target;
  bool zerostarted = false;
  bool zeroended = false;
  for (MYBYTE i = 0; i < 16; i += 2, src += 2) {
    if (src[0]) {
      if (zerostarted) zeroended = true;
      if (zerostarted && zeroended) {
        dp += sprintf(dp, "::");
        zerostarted = false;
      } else if (dp != target) dp += sprintf(dp, ":");
      dp += sprintf(dp, "%x", src[0]);
      dp += sprintf(dp, "%02x", src[1]);
    } else if (src[1]) {
      if (zerostarted) zeroended = true;
      if (zerostarted && zeroended) {
        dp += sprintf(dp, "::");
        zerostarted = false;
      } else if (dp != target) dp += sprintf(dp, ":");
      dp += sprintf(dp, "%0x", src[1]);
    } else if (!zeroended) zerostarted = true;
  }
  return target;
}

bool checkMask(MYDWORD mask) {
  mask = htonl(mask);
  while (mask) { if (mask < (mask << 1)) return false; mask <<= 1; }
  return true;
}

MYDWORD calcMask(MYDWORD rangeStart, MYDWORD rangeEnd) {
  NET4Address ip1, ip2, mask;

  ip1.ip = htonl(rangeStart);
  ip2.ip = htonl(rangeEnd);

  for (MYBYTE i = 0; i < 4; i++) {
    mask.octate[i] = ip1.octate[i] ^ ip2.octate[i];

    if (i && mask.octate[i - 1] < 255)
      mask.octate[i] = 0;
    else if (mask.octate[i] == 0)
      mask.octate[i] = 255;
    else if (mask.octate[i] < 2)
      mask.octate[i] = 254;
    else if (mask.octate[i] < 4)
      mask.octate[i] = 252;
    else if (mask.octate[i] < 8)
      mask.octate[i] = 248;
    else if (mask.octate[i] < 16)
      mask.octate[i] = 240;
    else if (mask.octate[i] < 32)
      mask.octate[i] = 224;
    else if (mask.octate[i] < 64)
      mask.octate[i] = 192;
    else if (mask.octate[i] < 128)
      mask.octate[i] = 128;
    else
      mask.octate[i] = 0;
  }
  return mask.ip;
}

MYDWORD getClassNetwork(MYDWORD ip) {
  NET4Address data;
  data.ip = ip;
  data.octate[3] = 0;
  if (data.octate[0] < 192) data.octate[2] = 0;
  if (data.octate[0] < 128) data.octate[1] = 0;
  return data.ip;
}

void getHostName(char *hn) {
  FIXED_INFO *FixedInfo;
  IP_ADDR_STRING *pIPAddr;
  DWORD ulOutBufLen = sizeof(FIXED_INFO);
  FixedInfo = (FIXED_INFO*) GlobalAlloc(GPTR, sizeof(FIXED_INFO));
  if (ERROR_BUFFER_OVERFLOW == GetNetworkParams(FixedInfo, &ulOutBufLen)) {
    GlobalFree(FixedInfo);
    FixedInfo = (FIXED_INFO*)GlobalAlloc(GPTR, ulOutBufLen);
  }
  if (!GetNetworkParams(FixedInfo, &ulOutBufLen)) {
    strcpy(hn, FixedInfo->HostName);
    GlobalFree(FixedInfo);
  }
}

bool getAdapterData()  {
  DWORD dwSize = 0;
  DWORD dwRetVal = 0;
  unsigned int i = 0;
  char ipstr[256];
  ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
  ULONG family = AF_UNSPEC;  // default to unspecified address family (both) (or AF_INET / AF_INET6)
  ULONG outBufLen = WORKING_BUFFER_SIZE;
  ULONG Iterations = 0;
  ULONG size = 256;
  LPVOID lpMsgBuf = NULL;
  PIP_ADAPTER_ADDRESSES pA = NULL;
  PIP_ADAPTER_ADDRESSES pCA = NULL;
  PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
  PIP_ADAPTER_ANYCAST_ADDRESS pAnycast = NULL;
  PIP_ADAPTER_MULTICAST_ADDRESS pMulticast = NULL;
  IP_ADAPTER_DNS_SERVER_ADDRESS *pDnServer = NULL;
  IP_ADAPTER_PREFIX *pPrefix = NULL;
  adptr.exist = false;
  adptr.ipset = false;
  do {
    pA = (IP_ADAPTER_ADDRESSES *) MALLOC(outBufLen);
    if (pA == NULL) return false;
    dwRetVal = GetAdaptersAddresses(family, flags, NULL, pA, &outBufLen);
    if (dwRetVal == ERROR_BUFFER_OVERFLOW) { FREE(pA); pA = NULL; }
    else break;
    Iterations++;
  } while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < MAX_TRIES));
  if (dwRetVal == NO_ERROR) {
    pCA = pA;
    while (pCA) {
      if (wcscmp(adptr.desc, pCA->Description) != 0) { pCA = pCA->Next; continue; }
      adptr.exist = true;
      adptr.idx4 = pCA->IfIndex;
      adptr.idx6 = pCA->Ipv6IfIndex;
      strcpy(adptr.name, pCA->AdapterName);
      wcscpy(adptr.wfname, pCA->FriendlyName);
      wpcopy(adptr.fname, adptr.wfname);
      // look for our ip address
      pUnicast = pCA->FirstUnicastAddress;
      if (pUnicast != NULL) {
        for (i = 0; pUnicast != NULL; i++) {
          WSAAddressToStringA(pUnicast->Address.lpSockaddr, pUnicast->Address.iSockaddrLength, NULL, ipstr, &size);
          // TODO add netmask matcher also...
          if (strcmp(ipstr, config.adptrip) == 0) adptr.ipset = true;
          pUnicast = pUnicast->Next;
        }
      }
      if (pCA->PhysicalAddressLength != 0) {
        adptr.phyaddrlen = pCA->PhysicalAddressLength;
        memcpy(adptr.phyaddr, pCA->PhysicalAddress, adptr.phyaddrlen);
      }
      break;
    }
  } else {
    if (dwRetVal == ERROR_NO_DATA)
      logMesg("Net: No addresses were found for the requested parameters", LOG_DEBUG);
    else showError(dwRetVal);
  }
  if (pA) FREE(pA);
  return adptr.exist;
}

int setAdptrIP() {
  ULONG NTEContext = 0;
  ULONG NTEInstance = 0;
  char* sysstr = (char*) calloc(2, MAX_ADAPTER_NAME_LENGTH + 4);
  if (config.setstatic) {
    sprintf(sysstr, "netsh interface ip set address \"%s\" static %s %s", adptr.fname, config.adptrip, config.netmask);
    system(sysstr);
    sprintf(lb.log, "Net: Adapter IP statically set to: %s", config.adptrip);
    logMesg(lb.log, LOG_INFO);
  } else {
    sprintf (sysstr, "netsh interface ip set address \"%s\" dhcp", adptr.fname);
    system(sysstr);
    AddIPAddress(inet_addr(config.adptrip), inet_addr(config.netmask), adptr.idx4, &NTEContext, &NTEInstance);
    sprintf(lb.log, "Net: Added IP %s to adapter", config.adptrip);
    logMesg(lb.log, LOG_INFO);
  }
  free (sysstr);
}

MYDWORD* addServer(MYDWORD* array, MYBYTE maxServers, MYDWORD ip) {
  for (MYBYTE i = 0; i < maxServers; i++) {
    if (array[i] == ip) return &(array[i]);
    else if (!array[i]) { array[i] = ip; return &(array[i]); }
  }
  return NULL;
}

MYDWORD* findServer(MYDWORD* array, MYBYTE cnt, MYDWORD ip) {
  if (ip) {
    for (MYBYTE i = 0; i < cnt && array[i]; i++) {
      if (array[i] == ip) return &(array[i]);
    }
  }
  return 0;
}

void setServerIFs() {

  SOCKET sd = WSASocket(PF_INET, SOCK_DGRAM, 0, 0, 0, 0);

  if (sd == INVALID_SOCKET) return;

  if (getAdapterData())
    if (!adptr.ipset) {
      setAdptrIP();
      Sleep(2000);
    }

  INTERFACE_INFO InterfaceList[MAX_SERVERS];
  unsigned long nBytesReturned;

  if (WSAIoctl(sd, SIO_GET_INTERFACE_LIST, 0, 0, &InterfaceList,
    sizeof(InterfaceList), &nBytesReturned, 0, 0) == SOCKET_ERROR) return;

  int nNumInterfaces = nBytesReturned / sizeof(INTERFACE_INFO);

  for (int i = 0; i < nNumInterfaces; ++i) {
    sockaddr_in* pAddress = (sockaddr_in*)&(InterfaceList[i].iiAddress);
    u_long nFlags = InterfaceList[i].iiFlags;
    if (!((nFlags & IFF_POINTTOPOINT) || (nFlags & IFF_LOOPBACK))) {
      addServer(net.allServers, MAX_SERVERS, pAddress->sin_addr.s_addr);
    }
  }

  closesocket(sd);

  PIP_ADAPTER_INFO pAdapterInfo;
  PIP_ADAPTER_INFO pAdapter;

  pAdapterInfo = (IP_ADAPTER_INFO*) calloc(1, sizeof(IP_ADAPTER_INFO));
  DWORD ulOutBufLen = sizeof(IP_ADAPTER_INFO);

  if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
    free(pAdapterInfo);
    pAdapterInfo = (IP_ADAPTER_INFO*)calloc(1, ulOutBufLen);
  }

  if ((GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
    pAdapter = pAdapterInfo;
    while (pAdapter) {
      if (!pAdapter->DhcpEnabled) {
        IP_ADDR_STRING *sList = &pAdapter->IpAddressList;
        while (sList) {
          DWORD iaddr = inet_addr(sList->IpAddress.String);
          if (iaddr) {
            for (MYBYTE k = 0; k < MAX_SERVERS; k++) {
              if (net.staticServers[k] == iaddr) break;
              else if (!net.staticServers[k]) {
                net.staticServers[k] = iaddr;
                net.staticMasks[k] = inet_addr(sList->IpMask.String);
                break;
              }
            }
          }
          sList = sList->Next;
        }
      }
      pAdapter = pAdapter->Next;
    }
    free(pAdapterInfo);
  }

  if (config.bindonly) {
    if (getAdapterData()) {
      logMesg("bindonly set, using adapter interface ip only", LOG_INFO);
      net.listenServers[0] = inet_addr(config.adptrip);
      net.listenMasks[0] = inet_addr(config.netmask);
    } else {
      logMesg("bindonly set but adapter interface is not available", LOG_NOTICE);
    }
    return;
  }

  if (net.staticServers[0]) {
    logMesg("using static interface ip address(es): ", LOG_INFO);
    int i=0;
    bool nomatch = true;
    for (; i < sizeof(net.staticServers)/sizeof(*net.staticServers), net.staticServers[i]; i++) {
      sprintf(lb.log, "  %s", IP2String(lb.tmp, net.staticServers[i]));
      logMesg(lb.log, LOG_INFO);
      net.listenServers[i] = net.staticServers[i];
      net.listenMasks[i] = net.staticServers[i];
      if (net.staticServers[i] == inet_addr(config.adptrip)) nomatch = false;
    }
    if (config.adptrip && nomatch) {
      logMesg("no match for adapter ip under static interfaces", LOG_NOTICE);
    }
  } else {
    if (config.adptrip) {
      logMesg("no static interfaces found, attempting to use adapter ip", LOG_NOTICE);
      net.listenServers[0] = inet_addr(config.adptrip);
      net.listenMasks[0] = inet_addr(config.netmask);
    } else {
      logMesg("no static interfaces found and no adapter ip", LOG_NOTICE);
    }
  }

  return;
}

int detectFailure() {
  return
    net.failureCounts[MONITOR_IDX] +
    net.failureCounts[FDNS_IDX] +
    net.failureCounts[TUNNEL_IDX] +
    net.failureCounts[DHCP_IDX] +
    net.failureCounts[HTTP_IDX];
}

bool detectBusy() {
  if (net.busy[MONITOR_IDX] +
      net.busy[FDNS_IDX] +
      net.busy[TUNNEL_IDX] +
      net.busy[DHCP_IDX] +
      net.busy[HTTP_IDX])
    return true;
  else
    return false;
}

void stopDC() {

  if (ge.dCol.hEvent != NULL)
    CancelIPChangeNotify(&ge.dCol);

  return;
}

// detectChange waiting function
bool dCWait(int idx) {

  int fc = net.failureCounts[idx];
  char* sname = gd.serviceNames[idx];
  DWORD eventWait;

  net.ready[idx] = true;

  if (fc) {
    // wait up to max 40 tries and then reset fc
    if (fc >= 40) fc = net.failureCounts[idx] = 0;
    eventWait = 2000 * fc;
    sprintf(lb.log, "detectChange sleeping %d msecs and retrying failed service %s", eventWait, sname);
    logMesg(lb.log, LOG_INFO);
    sprintf(lb.log, "serviceName: %s, failureCount: %d\r\n", sname, fc);
    logMesg(lb.log, LOG_DEBUG);
    Sleep(eventWait);
    net.ready[idx] = false;
    while (net.busy[idx]) Sleep(1000);
    if (!config.bindonly || (config.bindonly && getAdapterData()))
      return true;
    net.failureCounts[idx] = 0;
  }

  sprintf(lb.log, "Service %s waiting for network changes", sname);
  logMesg(lb.log, LOG_INFO);

  while (!net.refresh) Sleep(1000);

  net.ready[idx] = false;

  while (net.busy[idx]) Sleep(1000);

  while (net.refresh) {
    sprintf(lb.log, "%s waiting on detectChange", sname);
    logMesg(lb.log, LOG_DEBUG);
    Sleep(1000);
  }

  if (!config.isExiting) {
    sprintf(lb.log, "Network event, service %s refreshing", sname);
    logMesg(lb.log, LOG_INFO);
  } else return false;

  return true;
}

bool detectChange() {

  bool adptrOk = getAdapterData();
  bool hasStatic = net.staticServers[0];

  net.refresh = false;

  HANDLE hand = NULL;
  ge.dCol.hEvent = WSACreateEvent();

  if (NotifyAddrChange(&hand, &ge.dCol) != NO_ERROR) {
    if (WSAGetLastError() != WSA_IO_PENDING) {
      WSACloseEvent(ge.dCol.hEvent);
      Sleep(1000);
      return true;
    }
  }

  logMesg("Waiting for network interface changes", LOG_INFO);

  if (WaitForSingleObject(ge.dCol.hEvent, UINT_MAX) == WAIT_OBJECT_0)
    WSACloseEvent(ge.dCol.hEvent);

  net.refresh = true;

  Sleep(2000);

  if (!config.isExiting) {
    logMesg("Network event, refreshing", LOG_NOTICE);
    while (detectBusy()) {
      sprintf(lb.log, "net.busy: MONITOR: %d FDNS: %d TUNNEL: %d DHCP: %d HTTP: %d",
        net.busy[0], net.busy[1], net.busy[2], net.busy[3], net.busy[4], net.busy[5]);
      logMesg(lb.log, LOG_DEBUG);
      Sleep(1000);
    }
  } else {
    net.refresh = false;
    return false;
  }

  setServerIFs();

  getHostName(net.hostname);

  return true;
}

int netExit() {
  WSACleanup();
  free (adptr.wfname);
  free (adptr.fname);
  free (adptr.name);
  free (adptr.desc);
}

int netInit() {

  getHostName(net.hostname);
  sprintf(lb.log, "hostname %s", net.hostname);
  logMesg(lb.log, LOG_NOTICE);

  adptr.desc = (LPWSTR) calloc(sizeof(wchar_t), (MAX_ADAPTER_DESCRIPTION_LENGTH + 4));
  adptr.name = (PCHAR) calloc(1, MAX_ADAPTER_NAME_LENGTH + 4);
  adptr.fname = (PCHAR) calloc(1, MAX_ADAPTER_NAME_LENGTH + 4);
  adptr.wfname = (PWCHAR) calloc(sizeof(wchar_t), (MAX_ADAPTER_NAME_LENGTH + 4));

  int ifdesclen = MultiByteToWideChar(CP_ACP, 0, config.ifname, -1, adptr.desc, 0);
  if (ifdesclen > 0)
    MultiByteToWideChar(CP_ACP, 0, config.ifname, -1, adptr.desc, ifdesclen);

  MYWORD wVersionReq = MAKEWORD(1,1);
  WSAStartup(wVersionReq, &net.wsa);
  if (net.wsa.wVersion != wVersionReq)
    logMesg("WSAStartup error", LOG_INFO);

  if (config.adptrip)
    if (!config.netmask) config.netmask = "255.255.255.0";

  setServerIFs();

  return 0;
}
