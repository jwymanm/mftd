/*
 *   Network helper and core routines
*/

#include "core.h"
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

void setServers() {

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

  net.listenServers[0] = inet_addr(config.adptrip);
  net.listenMasks[0] = inet_addr(config.netmask);

  return;
}

bool detectChange() {

  logMesg("Waiting for network changes", LOG_INFO);

  int nfctot =
    net.failureCounts[MONITOR_IDX] +
    net.failureCounts[FDNS_IDX] +
    net.failureCounts[TUNNEL_IDX] +
    net.failureCounts[DHCP_IDX];
   // net.failureCounts[HTTP_IDX];

  if (nfctot) {
    DWORD eventWait = (DWORD) (1000 * pow(2, nfctot));
    sprintf(lb.log, "detectChange sleeping %d msecs and retrying failed threads", eventWait);
    logMesg(lb.log, LOG_INFO);
    sprintf(lb.log, "failureCounts: MONITOR: %d FDNS: %d TUNNEL: %d DHCP: %d HTTP: \r\n",
      net.failureCounts[MONITOR_IDX], net.failureCounts[FDNS_IDX], net.failureCounts[TUNNEL_IDX], net.failureCounts[DHCP_IDX]);
    //net.failureCounts[HTTP_IDX]);
    logMesg(lb.log, LOG_DEBUG);
    Sleep(eventWait);
    net.ready = false;
    return true;
  }

  if (getAdapterData()) net.ready = true;

  ge.net = NULL;
  ge.dCol.hEvent = WSACreateEvent();

  if (NotifyAddrChange(&ge.net, &ge.dCol) != NO_ERROR) {
    if (WSAGetLastError() != WSA_IO_PENDING) {
      WSACloseEvent(ge.dCol.hEvent);
      Sleep(1000);
      return true;
    }
  }

  // change to infinite?
  if (WaitForSingleObject(ge.dCol.hEvent, UINT_MAX) == WAIT_OBJECT_0)
    WSACloseEvent(ge.dCol.hEvent);

  net.ready = false;

  if (!config.isExiting)
    logMesg("Network event, waiting to refresh services", LOG_NOTICE);
  else return false;

  /*  Wait 8 seconds for network to finish changing */
  Sleep(8000);
  getHostName(net.hostname); // just in case hostname changed
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
  sprintf(lb.log, "Using hostname %s", net.hostname);
  logMesg(lb.log, LOG_NOTICE);
  adptr.desc = (LPWSTR) calloc(sizeof(wchar_t), (MAX_ADAPTER_DESCRIPTION_LENGTH + 4));
  adptr.name = (PCHAR) calloc(1, MAX_ADAPTER_NAME_LENGTH + 4);
  adptr.fname = (PCHAR) calloc(1, MAX_ADAPTER_NAME_LENGTH + 4);
  adptr.wfname = (PWCHAR) calloc(sizeof(wchar_t), (MAX_ADAPTER_NAME_LENGTH + 4));
  int ifdesclen = MultiByteToWideChar(CP_ACP, 0, config.ifname, -1, adptr.desc, 0);
  if (ifdesclen > 0)
    MultiByteToWideChar(CP_ACP, 0, config.ifname, -1, adptr.desc, ifdesclen);
  MYWORD wVersionReq = MAKEWORD(1, 1);
  WSAStartup(wVersionReq, &gd.wsa);
  if (gd.wsa.wVersion != wVersionReq)
    logMesg("WSAStartup error", LOG_INFO);
  return 0;
}

#if 0
// Left here for example code for now

char  adptr_ip[255];
char  adptr_name[MAX_ADAPTER_NAME_LENGTH + 4];
DWORD adptr_idx;

int getAdptrInfo() {

  PIP_ADAPTER_INFO pAdapterInfo;
  PIP_ADAPTER_INFO pAdapter = NULL;
  DWORD dwRetVal = 0;
  UINT i;

  ULONG ulOutBufLen = sizeof (IP_ADAPTER_INFO);
  pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(sizeof (IP_ADAPTER_INFO));

  if (pAdapterInfo == NULL) { return false; }

  if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
    FREE(pAdapterInfo);
    pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(ulOutBufLen);
    if (pAdapterInfo == NULL) { return false; }
  }

  if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
    pAdapter = pAdapterInfo;
    while (pAdapter) {
      if (!strcmp(pAdapter->Description, config.ifname)) {
        memcpy (adptr_ip, pAdapter->IpAddressList.IpAddress.String, strlen(pAdapter->IpAddressList.IpAddress.String) + 1);
        adptr_idx = pAdapter->Index;
        strcpy(adptr_name, pAdapter->AdapterName);
        if (pAdapterInfo) FREE(pAdapterInfo);
        return true;
      } else { pAdapter = pAdapter->Next; }
    }
  }

  if (pAdapterInfo) FREE(pAdapterInfo);
  return false;
}

int getAdptrAddr()  {

  DWORD dwSize = 0;
  DWORD dwRetVal = 0;
  unsigned int i = 0;
  ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
  // default to unspecified address family (both)
  ULONG family = AF_UNSPEC;
  LPVOID lpMsgBuf = NULL;
  PIP_ADAPTER_ADDRESSES pAddresses = NULL;
  ULONG outBufLen = 0;
  ULONG Iterations = 0;
  PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
  PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
  PIP_ADAPTER_ANYCAST_ADDRESS pAnycast = NULL;
  PIP_ADAPTER_MULTICAST_ADDRESS pMulticast = NULL;
  IP_ADAPTER_DNS_SERVER_ADDRESS *pDnServer = NULL;
  IP_ADAPTER_PREFIX *pPrefix = NULL;

  //family = AF_INET;
  //family = AF_INET6;

  outBufLen = WORKING_BUFFER_SIZE;

  do {

    pAddresses = (IP_ADAPTER_ADDRESSES *) MALLOC(outBufLen);
    if (pAddresses == NULL) return 0;

    dwRetVal = GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);

    if (dwRetVal == ERROR_BUFFER_OVERFLOW) { FREE(pAddresses); pAddresses = NULL; }
    else break;

    Iterations++;

  } while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < MAX_TRIES));

  if (dwRetVal == NO_ERROR) {

    pCurrAddresses = pAddresses;

    while (pCurrAddresses) {

      printf("\tLength of the IP_ADAPTER_ADDRESS struct: %ld\n", pCurrAddresses->Length);
      printf("\tIfIndex (IPv4 interface): %u\n", pCurrAddresses->IfIndex);
      printf("\tAdapter name: %s\n", pCurrAddresses->AdapterName);

      pUnicast = pCurrAddresses->FirstUnicastAddress;
      if (pUnicast != NULL) {
        for (i = 0; pUnicast != NULL; i++) pUnicast = pUnicast->Next;
        printf("\tNumber of Unicast Addresses: %d\n", i);
      } else printf("\tNo Unicast Addresses\n");

      pAnycast = pCurrAddresses->FirstAnycastAddress;
      if (pAnycast) {
        for (i = 0; pAnycast != NULL; i++) pAnycast = pAnycast->Next;
        printf("\tNumber of Anycast Addresses: %d\n", i);
      } else printf("\tNo Anycast Addresses\n");

      pMulticast = pCurrAddresses->FirstMulticastAddress;
      if (pMulticast) {
        for (i = 0; pMulticast != NULL; i++) pMulticast = pMulticast->Next;
        printf("\tNumber of Multicast Addresses: %d\n", i);
      } else printf("\tNo Multicast Addresses\n");

      pDnServer = pCurrAddresses->FirstDnsServerAddress;
      if (pDnServer) {
        for (i = 0; pDnServer != NULL; i++) pDnServer = pDnServer->Next;
        printf("\tNumber of DNS Server Addresses: %d\n", i);
      } else printf("\tNo DNS Server Addresses\n");

      printf("\tDNS Suffix: %wS\n", pCurrAddresses->DnsSuffix);
      printf("\tDescription: %wS\n", pCurrAddresses->Description);
      printf("\tFriendly name: %wS\n", pCurrAddresses->FriendlyName);

      if (pCurrAddresses->PhysicalAddressLength != 0) {
        printf("\tPhysical address: ");
        for (i = 0; i < (int) pCurrAddresses->PhysicalAddressLength; i++) {
          if (i == (pCurrAddresses->PhysicalAddressLength - 1))
            printf("%.2X\n", (int) pCurrAddresses->PhysicalAddress[i]);
          else printf("%.2X-", (int) pCurrAddresses->PhysicalAddress[i]);
        }
      }
      printf("\tFlags: %ld\n", pCurrAddresses->Flags);
      printf("\tMtu: %lu\n", pCurrAddresses->Mtu);
      printf("\tIfType: %ld\n", pCurrAddresses->IfType);
      printf("\tOperStatus: %ld\n", pCurrAddresses->OperStatus);
      printf("\tIpv6IfIndex (IPv6 interface): %u\n", pCurrAddresses->Ipv6IfIndex);
      printf("\tZoneIndices (hex): ");
      for (i = 0; i < 16; i++) printf("%lx ", pCurrAddresses->ZoneIndices[i]); printf("\n");

      // printf("\tTransmit link speed: %I64u\n", pCurrAddresses->TransmitLinkSpeed);
      //printf("\tReceive link speed: %I64u\n", pCurrAddresses->ReceiveLinkSpeed);

      pPrefix = pCurrAddresses->FirstPrefix;
      if (pPrefix) {
        for (i = 0; pPrefix != NULL; i++) pPrefix = pPrefix->Next;
        printf("\tNumber of IP Adapter Prefix entries: %d\n", i);
      } else printf("\tNumber of IP Adapter Prefix entries: 0\n");

      printf("\n");

      pCurrAddresses = pCurrAddresses->Next;
    }
  } else {
    if (dwRetVal == ERROR_NO_DATA)
      logMesg("Net: No addresses were found for the requested parameters", LOG_NOTICE);
    else
      showError(dwRetVal);
    }
  }

  if (pAddresses) FREE(pAddresses);
  return 0;
}

int lkupIF() {
   // Declare and initialize variables.

    DWORD dwSize = 0;
    DWORD dwRetVal = 0;

    unsigned int i, j;

    /* variables used for GetIfTable and GetIfEntry */
    MIB_IFTABLE *pIfTable;
    MIB_IFROW *pIfRow;

    // Allocate memory for our pointers.
    pIfTable = (MIB_IFTABLE *) MALLOC(sizeof (MIB_IFTABLE));
    if (pIfTable == NULL) {
        printf("Error allocating memory needed to call GetIfTable\n");
        return 1;
    }
    // Make an initial call to GetIfTable to get the
    // necessary size into dwSize
    dwSize = sizeof (MIB_IFTABLE);
    if (GetIfTable(pIfTable, &dwSize, FALSE) == ERROR_INSUFFICIENT_BUFFER) {
        FREE(pIfTable);
        pIfTable = (MIB_IFTABLE *) MALLOC(dwSize);
        if (pIfTable == NULL) {
            printf("Error allocating memory needed to call GetIfTable\n");
            return 1;
        }
    }
    // Make a second call to GetIfTable to get the actual
    // data we want.
    if ((dwRetVal = GetIfTable(pIfTable, &dwSize, FALSE)) == NO_ERROR) {
        printf("\tNum Entries: %ld\n\n", pIfTable->dwNumEntries);
        for (i = 0; i < pIfTable->dwNumEntries; i++) {
            pIfRow = (MIB_IFROW *) & pIfTable->table[i];
            printf("\tIndex[%d]:\t %ld\n", i, pIfRow->dwIndex);
            printf("\tInterfaceName[%d]:\t %ws", i, pIfRow->wszName);
            printf("\n");
            printf("\tDescription[%d]:\t ", i);
            for (j = 0; j < pIfRow->dwDescrLen; j++)
                printf("%c", pIfRow->bDescr[j]);
            printf("\n");
            printf("\tType[%d]:\t ", i);
            switch (pIfRow->dwType) {
            case IF_TYPE_OTHER:
                printf("Other\n");
                break;
            case IF_TYPE_ETHERNET_CSMACD:
                printf("Ethernet\n");
                break;
            case IF_TYPE_ISO88025_TOKENRING:
                printf("Token Ring\n");
                break;
            case IF_TYPE_PPP:
                printf("PPP\n");
                break;
            case IF_TYPE_SOFTWARE_LOOPBACK:
                printf("Software Lookback\n");
                break;
            case IF_TYPE_ATM:
                printf("ATM\n");
                break;
            case IF_TYPE_IEEE80211:
                printf("IEEE 802.11 Wireless\n");
                break;
            case IF_TYPE_TUNNEL:
                printf("Tunnel type encapsulation\n");
                break;
            case IF_TYPE_IEEE1394:
                printf("IEEE 1394 Firewire\n");
                break;
            default:
                printf("Unknown type %ld\n", pIfRow->dwType);
                break;
            }
            printf("\tMtu[%d]:\t\t %ld\n", i, pIfRow->dwMtu);
            printf("\tSpeed[%d]:\t %ld\n", i, pIfRow->dwSpeed);
            printf("\tPhysical Addr:\t ");
            if (pIfRow->dwPhysAddrLen == 0)
                printf("\n");
            for (j = 0; j < pIfRow->dwPhysAddrLen; j++) {
                if (j == (pIfRow->dwPhysAddrLen - 1))
                    printf("%.2X\n", (int) pIfRow->bPhysAddr[j]);
                else
                    printf("%.2X-", (int) pIfRow->bPhysAddr[j]);
            }
            printf("\tAdmin Status[%d]:\t %ld\n", i, pIfRow->dwAdminStatus);
            printf("\tOper Status[%d]:\t ", i);
            switch (pIfRow->dwOperStatus) {
            case IF_OPER_STATUS_NON_OPERATIONAL:
                printf("Non Operational\n");
                break;
            case IF_OPER_STATUS_UNREACHABLE:
                printf("Unreachable\n");
                break;
            case IF_OPER_STATUS_DISCONNECTED:
                printf("Disconnected\n");
                break;
            case IF_OPER_STATUS_CONNECTING:
                printf("Connecting\n");
                break;
            case IF_OPER_STATUS_CONNECTED:
                printf("Connected\n");
                break;
            case IF_OPER_STATUS_OPERATIONAL:
                printf("Operational\n");
                break;
            default:
                printf("Unknown status %ld\n", pIfRow->dwAdminStatus);
                break;
            }
            printf("\n");
        }
    } else {
        printf("GetIfTable failed with error: \n", dwRetVal);
        if (pIfTable != NULL) {
            FREE(pIfTable);
            pIfTable = NULL;
        }
        return 1;
        // Here you can use FormatMessage to find out why
        // it failed.
    }
    if (pIfTable != NULL) {
        FREE(pIfTable);
        pIfTable = NULL;
    }
    return 0;
}

#endif
