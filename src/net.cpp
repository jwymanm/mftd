/*
 *   Network helper and core routines
*/

#include "core.h"
#include "util.h"
#include "net.h"

Adapter adptr;
Network net;

using namespace core;

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

void IFAddr2String(char* buff, BYTE* phyaddr, DWORD len) {
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

      if (wcscmp(adptr.wdesc, pCA->Description) != 0) { pCA = pCA->Next; continue; }

      adptr.exist = true;
      adptr.idx4 = pCA->IfIndex;
      adptr.idx6 = pCA->Ipv6IfIndex;
      strcpy(adptr.name, pCA->AdapterName);
      wcscpy(adptr.wcname, pCA->FriendlyName);
      wpcopy(adptr.cname, adptr.wcname);
      // look for our ip address
      pUnicast = pCA->FirstUnicastAddress;
      if (adptr.mode != ADAPTER_MODE_DHCP && pUnicast != NULL) {
        for (i = 0; pUnicast != NULL; i++) {
          WSAAddressToStringA(pUnicast->Address.lpSockaddr, pUnicast->Address.iSockaddrLength, NULL, ipstr, &size);
          // TODO add netmask matcher also...
          //printf("ip: %s\r\n", ipstr);
          if (!strcmp(ipstr, config.adptrip)) adptr.ipset = true;
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
      logMesg("no addresses were found for the requested parameters", LOG_DEBUG);
    else showError(NULL, dwRetVal);
  }

  if (pA) FREE(pA);

  return adptr.exist;
}

int setAdptrIP() {

  char* sysstr = (char*) calloc(2, MAX_ADAPTER_NAME_LENGTH + 4);

  if (adptr.mode == ADAPTER_MODE_DHCP) {
    sprintf (sysstr, "netsh interface ip set address \"%s\" dhcp", adptr.cname);
    system(sysstr);
    logMesg("adapter configured to use dhcp only", LOG_INFO);
  } else if (adptr.mode == ADAPTER_MODE_DHCPPLUS) {
    ULONG NTEContext = 0;
    ULONG NTEInstance = 0;
    sprintf (sysstr, "netsh interface ip set address \"%s\" dhcp", adptr.cname);
    system(sysstr);
    AddIPAddress(inet_addr(config.adptrip), inet_addr(config.adptrip), adptr.idx4, &NTEContext, &NTEInstance);
    LM(LOG_INFO, "adapter set to dhcp with temporary IP %s mask %s", config.adptrip, config.adptrnm);
  } else if (adptr.mode == ADAPTER_MODE_STATIC) {
    sprintf(sysstr, "netsh interface ip set address \"%s\" static %s %s", adptr.cname, config.adptrip, config.adptrnm);
    system(sysstr);
    LM(LOG_INFO, "adapter IP statically set to: %s mask %s", config.adptrip, config.adptrnm);
  } else {
    logMesg("adapter mode not set", LOG_INFO);
  }
  free (sysstr);
}

void getServerIFs() {

  SOCKET sd = WSASocket(PF_INET, SOCK_DGRAM, 0, 0, 0, 0);
  if (sd == INVALID_SOCKET) return;

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
}

void setServerIFs() {

  bool adapterData = false;

  if (adptr.set && getAdapterData()) {
    if (!adptr.ipset) {
      setAdptrIP(); Sleep(2000);
    }
    adapterData = true;
  }

  getServerIFs();

  // TODO
  // possibly don't bail if bindonly is set and continue.. have a flag to do so in config?
  // like failbo or something

  if (adptr.set && adptr.mode != ADAPTER_MODE_DHCP && config.adptrbo) {
    if (adapterData) {
      LM(LOG_INFO, "bindonly set, attempting to use adapter interface ip only")
      net.listenServers[0] = inet_addr(config.adptrip);
      net.listenMasks[0] = inet_addr(config.adptrnm);
    } else {
      LM(LOG_NOTICE, "bindonly set but adapter interface is not available, bailing static interface selection")
    }
    return;
  } else {
    if (adptr.set && adptr.mode == ADAPTER_MODE_DHCP && config.adptrbo) {
      LM(LOG_NOTICE, "bindonly set but adapter is set to dhcp, ignoring bindonly")
    }
  }

  if (net.staticServers[0]) {
    LM(LOG_INFO, "using static interface ip address(es): ")
    int i=0;
    bool nomatch = true;
    for (; i < MAX_SERVERS && net.staticServers[i]; i++) {
      LM(LOG_INFO, "  %s", IP2String(lb.tmp, net.staticServers[i]));
      net.listenServers[i] = net.staticServers[i];
      net.listenMasks[i] = net.staticServers[i];
      if (adptr.set && adptr.mode == ADAPTER_MODE_STATIC &&
          net.staticServers[i] == inet_addr(config.adptrip)) nomatch = false;
    }
    if (adptr.set && adptr.mode == ADAPTER_MODE_STATIC && nomatch) {
      LM(LOG_NOTICE, "no match for adapter ip under static interfaces")
    }
  } else {
    if (adptr.set && adptr.mode != ADAPTER_MODE_DHCP) {
      LM(LOG_NOTICE, "no static interfaces found, falling back to adapter ip")
      net.listenServers[0] = inet_addr(config.adptrip);
      net.listenMasks[0] = inet_addr(config.adptrnm);
    } else {
      if (adptr.set) {
        LM(LOG_NOTICE, "no static interfaces found and adapter is set to dhcp");
      } else {
        LM(LOG_NOTICE, "no static interfaces found and no adapter is configured");
      }
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
    LM(LOG_INFO, "%s failureCount %d, sleeping %d msecs and retrying failed service", sname, fc, eventWait)
    Sleep(eventWait);
    net.ready[idx] = false;
    while (net.busy[idx]) Sleep(1000);
    if (!config.adptrbo || (adptr.set && config.adptrbo && getAdapterData()))
      return true;
    net.failureCounts[idx] = 0;
  }

  LM(LOG_INFO, "%s waiting for network changes", sname)

  while (!net.refresh) Sleep(1000);

  net.ready[idx] = false;

  while (net.busy[idx]) Sleep(1000);

  while (net.refresh) {
    LM(LOG_DEBUG, "%s waiting on detectChange", sname)
    Sleep(1000);
  }

  if (!gs.exit) {
    LM(LOG_INFO, "%s network event, service refreshing", sname)
  } else return false;

  return true;
}

bool detectChange() {

  // Future TODO possibly use some intel to decide to refresh
  // services if bindonly is set
  /*
  if (gs.adptr && config.bindonly) {
    bool adptrOk = getAdapterData();
    bool hasStatic = net.staticServers[0];
  }
  */

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

  LM(LOG_INFO, "waiting for network interface changes")

  if (WaitForSingleObject(ge.dCol.hEvent, UINT_MAX) == WAIT_OBJECT_0)
    WSACloseEvent(ge.dCol.hEvent);

  net.refresh = true;

  Sleep(2000);

  if (!gs.exit) {
    LM(LOG_NOTICE, "network event, refreshing")
    while (detectBusy()) {
      LM(LOG_DEBUG, "net.busy: MONITOR: %d FDNS: %d TUNNEL: %d DHCP: %d HTTP: %d",
        net.busy[MONITOR_IDX], net.busy[FDNS_IDX], net.busy[TUNNEL_IDX], net.busy[DHCP_IDX], net.busy[HTTP_IDX])
      Sleep(1000);
    }
  } else {
    net.refresh = false;
    return false;
  }

  getHostName(net.hostname);
  adapterInit();
  setServerIFs();
  return true;
}

void adapterCleanup() {
  free (adptr.wcname);
  free (adptr.cname);
  free (adptr.name);
  free (adptr.wdesc);
  free (adptr.desc);
}

void adapterInit() {

  int line, useLine;
  char buff[_MAX_PATH+1], * buffp;
  size_t bytes, nbytes;
  FILE* f,* f2;

  if (config.adptrdesc || config.adptrdescf) {

    if (!adptr.desc) {
      adptr.desc = (PCHAR) calloc(1, MAX_ADAPTER_NAME_LENGTH + 4);
      adptr.wdesc = (LPWSTR) calloc(sizeof(wchar_t), (MAX_ADAPTER_DESCRIPTION_LENGTH + 4));
      adptr.name = (PCHAR) calloc(1, MAX_ADAPTER_NAME_LENGTH + 4);
      adptr.cname = (PCHAR) calloc(1, MAX_ADAPTER_NAME_LENGTH + 4);
      adptr.wcname = (PWCHAR) calloc(sizeof(wchar_t), (MAX_ADAPTER_NAME_LENGTH + 4));
    } else {
      *adptr.desc = 0; *adptr.wdesc = 0; *adptr.name = 0;
      *adptr.cname = 0; *adptr.wcname = 0;
    }

    // TODO if using files just check timestamp instead of reloading

    if (!config.adptrdesc) {

      sprintf(buff, "%s\\%s", path.cfg, config.adptrdescf);
      f = fopen(buff, "r");
      if (f) {
        if (config.adptrset) {
          if (!isdigit(config.adptrset[0])) {
            sprintf (buff, "%s\\%s", path.cfg, config.adptrset);
            f2 = fopen(buff, "r");
            if (f2) {
              nbytes = ADAPTER_SETFILE_MAXLEN;
              buffp = (char*) calloc(1, nbytes);
              if (buffp) {
                bytes = getline(&buffp, &nbytes, f2);
                if (bytes == -1) {
                  LM(LOG_NOTICE, "getline error file %s, using first adapter", buff);
                  useLine = 0;
                } else {
                  if (isdigit(buffp[0])) {
                    useLine = atoi(buffp);
                    if (useLine) useLine--;
                  } else {
                    LM(LOG_NOTICE, "adapter set file %s invalid contents, using first adapter", buff);
                    useLine = 0;
                  }
                }
                free(buffp);
              } else useLine = 0;
              fclose(f2);
            } else {
              LM(LOG_NOTICE, "could not load adapter set file %s, using first adapter", config.adptrset);
              useLine = 0;
            }
          } else useLine = atoi(config.adptrset);
        } else useLine = 0;
        nbytes = MAX_ADAPTER_DESCRIPTION_LENGTH;
        buffp = adptr.desc;
        for (line=0; line <= useLine; line++) {
          bytes = getline(&buffp, &nbytes, f);
          if (bytes == -1) break;
        }
        if (adptr.desc[strlen(adptr.desc)-1] == '\n') adptr.desc[strlen(adptr.desc)-1] = 0;
        if (bytes == -1) {
          LM(LOG_NOTICE, "could not read line %d of adapter description file %s\\%s", useLine+1, path.cfg, config.adptrdescf);
          LM(LOG_NOTICE, "using last read line %d: %s", line, adptr.desc);
        }
        fclose(f);
      } else {
        LM(LOG_NOTICE, "could not load adapter description file %s, no adapter configured", buff);
        adptr.set = false;
        return;
      }
    } else sprintf(adptr.desc, "%s", config.adptrdesc);

    int ifdesclen = MultiByteToWideChar(CP_ACP, 0, adptr.desc, -1, adptr.wdesc, 0);
    if (ifdesclen > 0)
      MultiByteToWideChar(CP_ACP, 0, adptr.desc, -1, adptr.wdesc, ifdesclen);

    if (!strcasecmp(config.adptrmode, "static"))
      adptr.mode = ADAPTER_MODE_STATIC;
    else if (!strcasecmp(config.adptrmode, "dhcp"))
      adptr.mode = ADAPTER_MODE_DHCP;
    else if (!strcasecmp(config.adptrmode, "dhcp+"))
      adptr.mode = ADAPTER_MODE_DHCPPLUS;

    if (adptr.mode != ADAPTER_MODE_DHCP) {
      if (!config.adptrip) config.adptrip = ADAPTER_DEFAULT_IP;
      if (!config.adptrnm) config.adptrnm = ADAPTER_DEFAULT_NETMASK;
    }

    adptr.set = true;
    LM(LOG_INFO, "using adapter with description %s", adptr.desc);
  } else {
    LM(LOG_INFO, "no adapter configured");
    adptr.set = false;
  }

  return;
}

int netExit() {
  WSACleanup();
  adapterCleanup();
}

int netInit() {

  getHostName(net.hostname);
  LM(LOG_NOTICE, "hostname %s", net.hostname)

  MYWORD wVersionReq = MAKEWORD(1,1);
  WSAStartup(wVersionReq, &net.wsa);

  if (net.wsa.wVersion != wVersionReq) {
    LM(LOG_NOTICE, "WSAStartup error")
    showError(NULL, GetLastError());
  }

  adapterInit();
  setServerIFs();
  return 0;
}
