// dhcp.cpp

#if DHCP

#include "core.h"
#include "util.h"
#include "net.h"
#include "dhcp.h"
#include "monitor.h"
#include "http.h"

namespace dhcp {

LocalBuffers lb;
LocalData ld;
NetworkData nd;
Configuration cfig;
Request dhcpr;
Request token;
dhcpMap dhcpCache;
lumpData lump;

char RANGESET[] = "RANGE_SET";
char GLOBALOPTIONS[] = "GLOBAL_OPTIONS";
const OPS opData[] = {
  { "SubnetMask", 1, 3 , 1},
  { "TimeOffset", 2, 4 , 1},
  { "Router", 3, 3 , 1},
  { "TimeServer", 4, 3 , 1},
  { "NameServer", 5, 3 , 1},
  { "DomainServer", 6, 3 , 1},
  { "LogServer", 7, 3 , 1},
  { "QuotesServer", 8, 3 , 1},
  { "LPRServer", 9, 3 , 1},
  { "ImpressServer", 10, 3 , 1},
  { "RLPServer", 11, 3, 1},
  { "Hostname", 12, 1, 1},
  { "BootFileSize", 13, 5 , 1},
  { "MeritDumpFile", 14, 1 , 1},
  { "DomainName", 15, 1 , 1},
  { "SwapServer", 16, 3 , 1},
  { "RootPath", 17, 1 , 1},
  { "ExtensionFile", 18, 1 , 1},
  { "ForwardOn/Off", 19, 7 , 1},
  { "SrcRteOn/Off", 20, 7 , 1},
  { "PolicyFilter", 21, 8 , 1},
  { "MaxDGAssembly", 22, 5 , 1},
  { "DefaultIPTTL", 23, 6 , 1},
  { "MTUTimeout", 24, 4 , 1},
  { "MTUPlateau", 25, 2 , 1},
  { "MTUInterface", 26, 5 , 1},
  { "MTUSubnet", 27, 7 , 1},
  { "BroadcastAddress", 28, 3 , 1},
  { "MaskDiscovery", 29, 7 , 1},
  { "MaskSupplier", 30, 7 , 1},
  { "RouterDiscovery", 31, 7 , 1},
  { "RouterRequest", 32, 3 , 1},
  { "StaticRoute", 33, 8 , 1},
  { "Trailers", 34, 7 , 1},
  { "ARPTimeout", 35, 4 , 1},
  { "Ethernet", 36, 7 , 1},
  { "DefaultTCPTTL", 37, 6 , 1},
  { "KeepaliveTime", 38, 4 , 1},
  { "KeepaliveData", 39, 7 , 1},
  { "NISDomain", 40, 1 , 1},
  { "NISServers", 41, 3 , 1},
  { "NTPServers", 42, 3 , 1},
  { "VendorSpecificInf", 43, 2 , 0},
  { "NETBIOSNameSrv", 44, 3 , 1},
  { "NETBIOSDistSrv", 45, 3 , 1},
  { "NETBIOSNodeType", 46, 6 , 1},
  { "NETBIOSScope", 47, 1 , 1},
  { "XWindowFont", 48, 1 , 1},
  { "XWindowManager", 49, 3 , 1},
  { "AddressRequest", 50, 3, 0},
  { "AddressTime", 51, 4 , 1},
  { "OverLoad", 52, 7, 0},
  { "DHCPMsgType", 53, 6, 0},
  { "DHCPServerId", 54, 3, 0},
  { "ParameterList", 55, 2 , 0},
  { "DHCPMessage", 56, 1, 0},
  { "DHCPMaxMsgSize", 57, 5, 0},
  { "RenewalTime", 58, 4 , 1},
  { "RebindingTime", 59, 4 , 1},
  { "ClassId", 60, 1, 0},
  { "ClientId", 61, 2, 0},
  { "NetWareIPDomain", 62, 1 , 1},
  { "NetWareIPOption", 63, 2 , 1},
  { "NISDomainName", 64, 1 , 1},
  { "NISServerAddr", 65, 3 , 1},
  { "TFTPServerName", 66, 1 , 1},
  { "BootFileOption", 67, 1 , 1},
  { "HomeAgentAddrs", 68, 3 , 1},
  { "SMTPServer", 69, 3 , 1},
  { "POP3Server", 70, 3 , 1},
  { "NNTPServer", 71, 3 , 1},
  { "WWWServer", 72, 3 , 1},
  { "FingerServer", 73, 3 , 1},
  { "IRCServer", 74, 3 , 1},
  { "StreetTalkServer", 75, 3 , 1},
  { "STDAServer", 76, 3 , 1},
  { "UserClass", 77, 1, 0},
  { "DirectoryAgent", 78, 1 , 1},
  { "ServiceScope", 79, 1 , 1},
  { "RapidCommit", 80, 2, 0},
  { "ClientFQDN", 81, 2, 0},
  { "RelayAgentInformation", 82, 3, 0},
  { "iSNS", 83, 1 , 1},
  { "NDSServers", 85, 3 , 1},
  { "NDSTreeName", 86, 1 , 1},
  { "NDSContext", 87, 1 , 1},
  { "LDAP", 95, 1 , 1},
  { "PCode", 100, 1 , 1},
  { "TCode", 101, 1 , 1},
  { "NetInfoAddress", 112, 3 , 1},
  { "NetInfoTag", 113, 1 , 1},
  { "URL", 114, 1 , 1},
  { "AutoConfig", 116, 7 , 1},
  { "NameServiceSearch", 117, 2 , 1},
  { "SubnetSelectionOption", 118, 3 , 1},
  { "DomainSearch", 119, 1 , 1},
  { "SIPServersDHCPOption", 120, 1 , 1},
  { "121", 121, 1 , 1},
  { "CCC", 122, 1 , 1},
  { "TFTPServerIPaddress", 128, 3 , 1},
  { "CallServerIPaddress", 129, 3 , 1},
  { "DiscriminationString", 130, 1 , 1},
  { "RemoteStatisticsServerIPAddress", 131, 3 , 1},
  { "HTTPProxyPhone", 135, 3 , 1},
  { "OPTION_CAPWAP_AC_V4", 138, 1 , 1},
  { "OPTIONIPv4_AddressMoS", 139, 1 , 1},
  { "OPTIONIPv4_FQDNMoS", 140, 1 , 1},
  { "SIPUAServiceDomains", 141, 1 , 1},
  { "OPTIONIPv4_AddressANDSF", 142, 1 , 1},
  { "IPTelephone", 176, 1 , 1},
  { "ConfigurationFile", 209, 1 , 1},
  { "PathPrefix", 210, 1 , 1},
  { "RebootTime", 211, 4 , 1},
  { "OPTION_6RD", 212, 1 , 1},
  { "OPTION_V4_ACCESS_DOMAIN", 213, 1 , 1},
  { "BootFileName", 253, 1 , 1},
  { "NextServer", 254, 3, 1}
};

void cleanup(int et) {

  int i;
  bool closed = false;

  for (i=0; i < MAX_SERVERS && nd.dhcpConn[i].loaded; i++)
    if (nd.dhcpConn[i].ready) {
      closesocket(nd.dhcpConn[i].sock);
      closed = true;
    }

  if (cfig.replication && cfig.dhcpReplConn.ready) {
    closesocket(cfig.dhcpReplConn.sock);
    closed = true;
  }

  if (closed) logMesg("DHCP cleared network connections", LOG_DEBUG);

  if (et) {
    gd.running[DHCP_IDX] = false;
    Sleep(1000);
    logMesg("DHCP stopped", LOG_INFO);
    pthread_exit(NULL);
  }
  return;
}

void stop() {
  if (config.dhcp && gd.running[DHCP_IDX]) {
    logMesg("Stopping DHCP", LOG_NOTICE);
    gd.running[DHCP_IDX] = false;
    cleanup(0);
  }
}

void start() {
  if (config.dhcp && !gd.running[DHCP_IDX]) {
    pthread_create(&gd.threads[DHCP_TIDX], NULL, main, NULL);
  }
}

MYWORD fUShort(void* raw) { return ntohs(*((MYWORD*)raw)); }
MYDWORD fULong(void* raw) { return ntohl(*((MYDWORD*)raw)); }
MYDWORD fIP(void* raw) { return(*((MYDWORD*)raw)); }
MYBYTE pUShort(void* raw, MYWORD data) { *((MYWORD*)raw) = htons(data); return sizeof(MYWORD); }
MYBYTE pULong(void* raw, MYDWORD data) { *((MYDWORD*)raw) = htonl(data); return sizeof(MYDWORD); }
MYBYTE pIP(void* raw, MYDWORD data) { *((MYDWORD*)raw) = data; return sizeof(MYDWORD); }

#if HTTP
bool buildSP(void* arg) {

  dhcpMap::iterator p;
  MYDWORD iip = 0;
  Data* dhcpEntry = NULL;

  http::Data* h = http::initDP("DHCP", arg, (135 * dhcpCache.size()) + (cfig.dhcpSize * 26));

  if (!h) return false;

  char tmp[512], ext[512];
  char* fp = h->res.dp;
  char* maxData = h->res.dp + h->res.memSize;

  fp += sprintf(fp, "<h3>DHCP</h3>\n");

  if (!nd.dhcpConn[0].ready) {
    fp += sprintf(fp, "<p>Waiting for interface</p>\n");
    h->res.bytes = fp - h->res.dp;
    return true;
  }

  fp += sprintf(fp, "<h4>Active Leases</h4>\n");
  fp += sprintf(fp, "<table border=\"0\" cellspacing=\"9\" width=\"800\">\n");

  if (cfig.dhcpRepl > h->req.t) {
    fp += sprintf(fp, "<tr><th>Mac Address</th><th>IP</th><th>Lease Expiry</th><th>Hostname (first 20 chars)</th><th>Server</th></tr>\n");
  } else {
    fp += sprintf(fp, "<tr><th>Mac Address</th><th>IP</th><th>Lease Expiry</th><th>Hostname (first 20 chars)</th></tr>\n");
  }

  for (p = dhcpCache.begin(); gd.running[DHCP_IDX] && p != dhcpCache.end() && fp < maxData; p++) {
    if ((dhcpEntry = p->second) && dhcpEntry->display && dhcpEntry->expiry >= h->req.t) {
      fp += sprintf(fp, "<tr bgcolor=\"#eee\">");
      fp += sprintf(fp, h->html.td200, dhcpEntry->mapname);
      fp += sprintf(fp, h->html.td200, IP2String(tmp, dhcpEntry->ip));

      if (dhcpEntry->expiry >= INT_MAX)
        fp += sprintf(fp, h->html.td200, "Infinity");
      else {
        tm* ttm = localtime(&dhcpEntry->expiry);
        strftime(tmp, sizeof(tmp), "%d-%b-%y %X", ttm);
        fp += sprintf(fp, h->html.td200, tmp);
      }

      if (dhcpEntry->hostname) {
        if (strlen(dhcpEntry->hostname) <= 20)
          fp += sprintf(fp, h->html.td200, dhcpEntry->hostname);
        else {
          strncpy(tmp, dhcpEntry->hostname, 20);
          tmp[20] = 0;
          fp += sprintf(fp, h->html.td200, tmp);
        }
      } else
        fp += sprintf(fp, h->html.td200, "&nbsp;");

      if (cfig.dhcpRepl > h->req.t) {

        if (dhcpEntry->local && cfig.replication == 1)
          fp += sprintf(fp, h->html.td200, "Primary");
        else if (dhcpEntry->local && cfig.replication == 2)
          fp += sprintf(fp, h->html.td200, "Secondary");
        else if (cfig.replication == 1)
          fp += sprintf(fp, h->html.td200, "Secondary");
        else
          fp += sprintf(fp, h->html.td200, "Primary");
      }

      fp += sprintf(fp, "</tr>\n");
    }
  }

  fp += sprintf(fp, "</table>\n<h4>Free Dynamic Leases</h4>\n<table border=\"0\" cellspacing=\"9\" width=\"800\">\n");
  fp += sprintf(fp, "<tr><td><b>DHCP Range</b></td><td align=\"right\"><b>Available Leases</b></td><td align=\"right\"><b>Free Leases</b></td></tr>\n");

  for (char rangeInd = 0; gd.running[DHCP_IDX] && rangeInd < cfig.rangeCount && fp < maxData; rangeInd++) {
    float ipused = 0;
    float ipfree = 0;
    int ind = 0;

    for (MYDWORD iip = cfig.dhcpRanges[rangeInd].rangeStart; iip <= cfig.dhcpRanges[rangeInd].rangeEnd; iip++, ind++) {
      if (cfig.dhcpRanges[rangeInd].expiry[ind] < h->req.t) ipfree++;
      else if (cfig.dhcpRanges[rangeInd].dhcpEntry[ind] && !(cfig.dhcpRanges[rangeInd].dhcpEntry[ind]->fixed)) ipused++;
    }

    IP2String(tmp, ntohl(cfig.dhcpRanges[rangeInd].rangeStart));
    IP2String(ext, ntohl(cfig.dhcpRanges[rangeInd].rangeEnd));
    fp += sprintf(fp, "<tr bgcolor=\"#eee\"><td>%s - %s</td><td align=\"right\">%5.0f</td><td align=\"right\">%5.0f</td></tr>\n", tmp, ext, (ipused + ipfree), ipfree);
  }

  fp += sprintf(fp, "</table>\n<h4>Free Static Leases</h4>\n<table border=\"0\" cellspacing=\"9\" width=\"800\">\n");
  fp += sprintf(fp, "<tr><th>Mac Address</th><th>IP</th><th>Mac Address</th><th>IP</th></tr>\n");
  MYBYTE colNum = 0;

  for (p = dhcpCache.begin(); gd.running[DHCP_IDX] && p != dhcpCache.end() && fp < maxData; p++) {
    if ((dhcpEntry = p->second) && dhcpEntry->fixed && dhcpEntry->expiry < h->req.t) {
      if (!colNum) { fp += sprintf(fp, "<tr bgcolor=\"#eee\">"); colNum = 1; }
      else if (colNum == 1) { colNum = 2; }
      else if (colNum == 2) { fp += sprintf(fp, "</tr>\n<tr bgcolor=\"#eee\">"); colNum = 1; }
      fp += sprintf(fp, h->html.td200, dhcpEntry->mapname);
      fp += sprintf(fp, h->html.td200, IP2String(tmp, dhcpEntry->ip));
    }
  }

  if (colNum) fp += sprintf(fp, "</tr>\n");

  fp += sprintf(fp, "</table>\n");
  h->res.bytes = fp - h->res.dp;
  return true;
}
#endif

bool checkRange(ClientSets* rangeData, char rangeInd) {

  if (!cfig.hasFilter) return true;

  MYBYTE rangeSetInd = cfig.dhcpRanges[rangeInd].rangeSetInd;
  RangeSets* rangeSet = &cfig.rangeSet[rangeSetInd];

  if((!rangeData->macFound && !rangeSet->macSize[0]) || (rangeData->macFound && rangeData->macArray[rangeSetInd]))
    if((!rangeData->vendFound && !rangeSet->vendClassSize[0]) || (rangeData->vendFound && rangeData->vendArray[rangeSetInd]))
      if((!rangeData->userFound && !rangeSet->userClassSize[0]) || (rangeData->userFound && rangeData->userArray[rangeSetInd]))
        if((!rangeData->subnetFound && !rangeSet->subnetIP[0]) || (rangeData->subnetFound && rangeData->subnetArray[rangeSetInd]))
          return true;

  return false;
}

MYDWORD resad(Request* req) {

  MYDWORD minRange = 0;
  MYDWORD maxRange = 0;

  if (req->dhcpp.header.bp_giaddr) {
    lockIP(req->dhcpp.header.bp_giaddr);
    lockIP(req->remote.sin_addr.s_addr);
  }

  req->dhcpEntry = findDHCPEntry(req->chaddr);

  if (req->dhcpEntry && req->dhcpEntry->fixed) {
    if (req->dhcpEntry->ip) {
      setTempLease(req->dhcpEntry);
      return req->dhcpEntry->ip;
    } else {
      sprintf(lb.log, "Static DHCP Host %s (%s) has No IP, DHCPDISCOVER ignored", req->chaddr, req->hostname);
      logMesg(lb.log, LOG_INFO);
      return 0;
    }
  }

  MYDWORD iipNew = 0;
  MYDWORD iipExp = 0;
  MYDWORD rangeStart = 0;
  MYDWORD rangeEnd = 0;
  char rangeInd = -1;
  bool rangeFound = false;
  ClientSets rangeData;
  memset(&rangeData, 0, sizeof(ClientSets));

  if (cfig.hasFilter) {
    for (MYBYTE rangeSetInd = 0; rangeSetInd < MAX_RANGE_SETS && cfig.rangeSet[rangeSetInd].active; rangeSetInd++) {
      RangeSets* rangeSet = &cfig.rangeSet[rangeSetInd];

      for (MYBYTE i = 0; i < MAX_RANGE_FILTERS && rangeSet->macSize[i]; i++) {

        if(memcmp(req->dhcpp.header.bp_chaddr, rangeSet->macStart[i], rangeSet->macSize[i]) >= 0 && memcmp(req->dhcpp.header.bp_chaddr, rangeSet->macEnd[i], rangeSet->macSize[i]) <= 0) {
          rangeData.macArray[rangeSetInd] = 1;
          rangeData.macFound = true;
          //printf("mac Found, rangeSetInd=%i\n", rangeSetInd);
          break;
        }
      }

      for (MYBYTE i = 0; i < MAX_RANGE_FILTERS && req->vendClass.size && rangeSet->vendClassSize[i]; i++) {
        if(rangeSet->vendClassSize[i] == req->vendClass.size && !memcmp(req->vendClass.value, rangeSet->vendClass[i], rangeSet->vendClassSize[i])) {
          rangeData.vendArray[rangeSetInd] = 1;
          rangeData.vendFound = true;
          break;
        }
      }

      for (MYBYTE i = 0; i < MAX_RANGE_FILTERS && req->userClass.size && rangeSet->userClassSize[i]; i++) {
        if(rangeSet->userClassSize[i] == req->userClass.size && !memcmp(req->userClass.value, rangeSet->userClass[i], rangeSet->userClassSize[i])) {
          rangeData.userArray[rangeSetInd] = 1;
          rangeData.userFound = true;
          break;
        }
      }

      for (MYBYTE i = 0; i < MAX_RANGE_FILTERS && req->subnetIP && rangeSet->subnetIP[i]; i++) {
        if(req->subnetIP == rangeSet->subnetIP[i]) {
          rangeData.subnetArray[rangeSetInd] = 1;
          rangeData.subnetFound = true;
          break;
        }
      }
    }
  }

  if (req->dhcpEntry) {
    req->dhcpEntry->rangeInd = getRangeInd(req->dhcpEntry->ip);

    if (req->dhcpEntry->rangeInd >= 0) {
      int ind = getIndex(req->dhcpEntry->rangeInd, req->dhcpEntry->ip);

      if (cfig.dhcpRanges[req->dhcpEntry->rangeInd].dhcpEntry[ind] == req->dhcpEntry && checkRange(&rangeData, req->dhcpEntry->rangeInd)) {
        MYBYTE rangeSetInd = cfig.dhcpRanges[req->dhcpEntry->rangeInd].rangeSetInd;

        if (!cfig.rangeSet[rangeSetInd].subnetIP[0]) {
          MYDWORD mask = cfig.dhcpRanges[req->dhcpEntry->rangeInd].mask;
          calcRangeLimits(req->subnetIP, mask, &minRange, &maxRange);

          if (htonl(req->dhcpEntry->ip) >= minRange && htonl(req->dhcpEntry->ip) <= maxRange) {
            setTempLease(req->dhcpEntry);
            return req->dhcpEntry->ip;
          }
        } else {
          setTempLease(req->dhcpEntry);
          return req->dhcpEntry->ip;
        }
      }
    }
  }

  if (!iipNew && req->reqIP) {
    char k = getRangeInd(req->reqIP);
    if (k >= 0) {
      if (checkRange(&rangeData, k)) {
        Range* range = &cfig.dhcpRanges[k];
        int ind = getIndex(k, req->reqIP);
        if (range->expiry[ind] <= ld.t) {
          if (!cfig.rangeSet[range->rangeSetInd].subnetIP[0]) {
            calcRangeLimits(req->subnetIP, range->mask, &minRange, &maxRange);
            MYDWORD iip = htonl(req->reqIP);
            if (iip >= minRange && iip <= maxRange) { iipNew = iip; rangeInd = k; }
          } else { MYDWORD iip = htonl(req->reqIP); iipNew = iip; rangeInd = k; }
        }
      }
    }
  }

  for (char k = 0; !iipNew && k < cfig.rangeCount; k++) {

    if (checkRange(&rangeData, k)) {

      Range* range = &cfig.dhcpRanges[k];
      rangeStart = range->rangeStart;
      rangeEnd = range->rangeEnd;

      if (!cfig.rangeSet[range->rangeSetInd].subnetIP[0]) {
        calcRangeLimits(req->subnetIP, range->mask, &minRange, &maxRange);
        if (rangeStart < minRange) rangeStart = minRange;
        if (rangeEnd > maxRange) rangeEnd = maxRange;
      }

      if (rangeStart <= rangeEnd) {
        rangeFound = true;
        if (cfig.replication == 2) {
          for (MYDWORD m = rangeEnd; m >= rangeStart; m--) {
            int ind = m - range->rangeStart;
            if (!range->expiry[ind]) {
              iipNew = m;
              rangeInd = k;
              break;
            } else if (!iipExp && range->expiry[ind] < ld.t) {
              iipExp = m;
              rangeInd = k;
            }
          }
        } else {
          for (MYDWORD m = rangeStart; m <= rangeEnd; m++) {
            int ind = m - range->rangeStart;
            if (!range->expiry[ind]) {
              iipNew = m;
              rangeInd = k;
              break;
            } else if (!iipExp && range->expiry[ind] < ld.t) {
              iipExp = m;
              rangeInd = k;
            }
          }
        }
      }
    }
  }

  if (!iipNew && iipExp) iipNew = iipExp;

  if (iipNew) {
    if (!req->dhcpEntry) {
      memset(&lump, 0, sizeof(lumpData));
      lump.mapname = req->chaddr;
      lump.hostname = req->hostname;
      req->dhcpEntry = createCache(&lump);
      if (!req->dhcpEntry) return 0;
      dhcpCache[req->dhcpEntry->mapname] = req->dhcpEntry;
    }

    req->dhcpEntry->ip = htonl(iipNew);
    req->dhcpEntry->rangeInd = rangeInd;
    setTempLease(req->dhcpEntry);
    return req->dhcpEntry->ip;
  }

  if (rangeFound) {
    if (req->dhcpp.header.bp_giaddr)
      sprintf(lb.log, "No free leases for DHCPDISCOVER for %s (%s) from RelayAgent %s", req->chaddr, req->hostname, IP2String(lb.tmp, req->dhcpp.header.bp_giaddr));
    else
      sprintf(lb.log, "No free leases for DHCPDISCOVER for %s (%s) from interface %s", req->chaddr, req->hostname, IP2String(lb.tmp, nd.dhcpConn[req->sockInd].server));
  } else {
    if (req->dhcpp.header.bp_giaddr)
      sprintf(lb.log, "No Matching DHCP Range for DHCPDISCOVER for %s (%s) from RelayAgent %s", req->chaddr, req->hostname, IP2String(lb.tmp, req->dhcpp.header.bp_giaddr));
    else
      sprintf(lb.log, "No Matching DHCP Range for DHCPDISCOVER for %s (%s) from interface %s", req->chaddr, req->hostname, IP2String(lb.tmp, nd.dhcpConn[req->sockInd].server));
  }
  logMesg(lb.log, LOG_INFO);
  return 0;
}

MYDWORD chad(Request* req) {
  req->dhcpEntry = findDHCPEntry(req->chaddr);
  if (req->dhcpEntry && req->dhcpEntry->ip) return req->dhcpEntry->ip;
  else return 0;
}

MYDWORD sdmess(Request* req) {

  if (req->req_type == DHCP_MESS_NONE) {
    req->dhcpp.header.bp_yiaddr = chad(req);

    if (!req->dhcpp.header.bp_yiaddr) {
      sprintf(lb.log, "No Static Entry found for BOOTPREQUEST from Host %s", req->chaddr);
      logMesg(lb.log, LOG_INFO);
      return 0;
    }
  } else if (req->req_type == DHCP_MESS_DECLINE) {
    if (req->dhcpp.header.bp_ciaddr && chad(req) == req->dhcpp.header.bp_ciaddr) {
      lockIP(req->dhcpp.header.bp_ciaddr);

      req->dhcpEntry->ip = 0;
      req->dhcpEntry->expiry = INT_MAX;
      req->dhcpEntry->display = false;
      req->dhcpEntry->local = false;

      sprintf(lb.log, "IP Address %s declined by Host %s (%s), locked", IP2String(lb.tmp, req->dhcpp.header.bp_ciaddr), req->chaddr, req->hostname);
      logMesg(lb.log, LOG_INFO);
    }
    return 0;
  } else if (req->req_type == DHCP_MESS_RELEASE) {
    if (req->dhcpp.header.bp_ciaddr && chad(req) == req->dhcpp.header.bp_ciaddr) {
      req->dhcpEntry->display = false;
      req->dhcpEntry->local = false;
      setLeaseExpiry(req->dhcpEntry, 0);
      _beginthread(updateStateFile, 0, (void*)req->dhcpEntry);
      sprintf(lb.log, "IP Address %s released by Host %s (%s)", IP2String(lb.tmp, req->dhcpp.header.bp_ciaddr), req->chaddr, req->hostname);
      logMesg(lb.log, LOG_INFO);
    }
    return 0;
  } else if (req->req_type == DHCP_MESS_INFORM) {
    if ((cfig.replication == 1 && req->remote.sin_addr.s_addr == cfig.zoneServers[1]) || (cfig.replication == 2 && req->remote.sin_addr.s_addr == cfig.zoneServers[0])) recvRepl(req);
    return 0;
  } else if (req->req_type == DHCP_MESS_DISCOVER && strcasecmp(req->hostname, net.hostname)) {
    req->dhcpp.header.bp_yiaddr = resad(req);
    if (!req->dhcpp.header.bp_yiaddr) return 0;
    req->resp_type = DHCP_MESS_OFFER;
  } else if (req->req_type == DHCP_MESS_REQUEST) {
    if (req->server) {
      if (req->server == nd.dhcpConn[req->sockInd].server) {
        if (req->reqIP && req->reqIP == chad(req) && req->dhcpEntry->expiry > ld.t) {
          req->resp_type = DHCP_MESS_ACK;
          req->dhcpp.header.bp_yiaddr = req->reqIP;
        } else if (req->dhcpp.header.bp_ciaddr && req->dhcpp.header.bp_ciaddr == chad(req) && req->dhcpEntry->expiry > ld.t) {
          req->resp_type = DHCP_MESS_ACK;
          req->dhcpp.header.bp_yiaddr = req->dhcpp.header.bp_ciaddr;
        } else {
          req->resp_type = DHCP_MESS_NAK;
          req->dhcpp.header.bp_yiaddr = 0;
          sprintf(lb.log, "DHCPREQUEST from Host %s (%s) without Discover, NAKed", req->chaddr, req->hostname);
          logMesg(lb.log, LOG_INFO);
	      }
      } else return 0;
    } else if (req->dhcpp.header.bp_ciaddr && req->dhcpp.header.bp_ciaddr == chad(req) && req->dhcpEntry->expiry > ld.t) {
      req->resp_type = DHCP_MESS_ACK;
      req->dhcpp.header.bp_yiaddr = req->dhcpp.header.bp_ciaddr;
    } else if (req->reqIP && req->reqIP == chad(req) && req->dhcpEntry->expiry > ld.t) {
      req->resp_type = DHCP_MESS_ACK;
      req->dhcpp.header.bp_yiaddr = req->reqIP;
    } else {
      req->resp_type = DHCP_MESS_NAK;
      req->dhcpp.header.bp_yiaddr = 0;

      sprintf(lb.log, "DHCPREQUEST from Host %s (%s) without Discover, NAKed", req->chaddr, req->hostname);
      logMesg(lb.log, LOG_INFO);
    }
  } else return 0;

  addOptions(req);
  int packSize = req->vp - (MYBYTE*)&req->dhcpp; packSize++;

  if (req->req_type == DHCP_MESS_NONE) packSize = req->messsize;

  if ((req->dhcpp.header.bp_giaddr || !req->remote.sin_addr.s_addr) && req->dhcpEntry && req->dhcpEntry->rangeInd >= 0) {
    MYBYTE rangeSetInd = cfig.dhcpRanges[req->dhcpEntry->rangeInd].rangeSetInd;
    req->targetIP = cfig.rangeSet[rangeSetInd].targetIP;
  }

  if (req->targetIP) {
    req->remote.sin_port = htons(IPPORT_DHCPS);
    req->remote.sin_addr.s_addr = req->targetIP;
  } else if (req->dhcpp.header.bp_giaddr) {
    req->remote.sin_port = htons(IPPORT_DHCPS);
    req->remote.sin_addr.s_addr = req->dhcpp.header.bp_giaddr;
  } else if (req->dhcpp.header.bp_broadcast || !req->remote.sin_addr.s_addr || req->reqIP) {
    req->remote.sin_port = htons(IPPORT_DHCPC);
    req->remote.sin_addr.s_addr = INADDR_BROADCAST;
  } else { req->remote.sin_port = htons(IPPORT_DHCPC); }

  req->dhcpp.header.bp_op = BOOTP_REPLY;
  errno = 0;

  if (req->req_type == DHCP_MESS_DISCOVER && !req->dhcpp.header.bp_giaddr) {
    req->bytes = sendto(nd.dhcpConn[req->sockInd].sock, req->raw, packSize, MSG_DONTROUTE, (sockaddr*)&req->remote, sizeof(req->remote));
  } else {
    req->bytes = sendto(nd.dhcpConn[req->sockInd].sock, req->raw, packSize, 0, (sockaddr*)&req->remote, sizeof(req->remote));
  }

  if (errno || req->bytes <= 0) return 0;

  return req->dhcpp.header.bp_yiaddr;
}

MYDWORD alad(Request* req) {

  if (req->dhcpEntry && (req->req_type == DHCP_MESS_NONE || req->resp_type == DHCP_MESS_ACK)) {
    MYDWORD hangTime = req->lease;

    if (req->rebind > req->lease) hangTime = req->rebind;

    req->dhcpEntry->display = true;
    req->dhcpEntry->local = true;
    setLeaseExpiry(req->dhcpEntry, hangTime);

    _beginthread(updateStateFile, 0, (void*)req->dhcpEntry);

    if (req->lease && req->reqIP) {
      sprintf(lb.log, "Host %s (%s) allotted %s for %u seconds", req->chaddr, req->hostname, IP2String(lb.tmp, req->dhcpp.header.bp_yiaddr), req->lease);
    } else if (req->req_type) {
      sprintf(lb.log, "Host %s (%s) renewed %s for %u seconds", req->chaddr, req->hostname, IP2String(lb.tmp, req->dhcpp.header.bp_yiaddr), req->lease);
    } else {
      sprintf(lb.log, "BOOTP Host %s (%s) allotted %s", req->chaddr, req->hostname, IP2String(lb.tmp, req->dhcpp.header.bp_yiaddr));
    }
    logMesg(lb.log, LOG_INFO);

    if (cfig.replication && cfig.dhcpRepl > ld.t) sendRepl(req);

    return req->dhcpEntry->ip;
  } else if (req->resp_type == DHCP_MESS_OFFER) {
    sprintf(lb.log, "Host %s (%s) offered %s", req->chaddr, req->hostname, IP2String(lb.tmp, req->dhcpp.header.bp_yiaddr));
    logMesg(lb.log, LOG_INFO);
  }

  return 0;
}

void addOptions(Request* req) {

  OptCode op;
  int i;

  if (req->req_type && req->resp_type) {
    op.opt_code = DHCP_OPTION_MESSAGETYPE;
    op.size = 1;
    op.value[0] = req->resp_type;
    pvdata(req, &op);
  }

  if (req->dhcpEntry && req->resp_type != DHCP_MESS_DECLINE && req->resp_type != DHCP_MESS_NAK) {
    strcpy(req->dhcpp.header.bp_sname, net.hostname);

    if (req->dhcpEntry->fixed) {
      MYBYTE* opPointer = req->dhcpEntry->options;

      if (opPointer) {
        MYBYTE requestedOnly = * opPointer;
        opPointer++;

        while (*opPointer && *opPointer != DHCP_OPTION_END) {
          op.opt_code = *opPointer;
          opPointer++;
          op.size = *opPointer;
          opPointer++;

          if (!requestedOnly || req->paramreqlist[*opPointer]) {
            memcpy(op.value, opPointer, op.size);
            pvdata(req, &op);
          }
          opPointer += op.size;
        }
      }
    }

    if (req->req_type && req->resp_type) {

      if (req->dhcpEntry->rangeInd >= 0) {
        MYBYTE* opPointer = cfig.dhcpRanges[req->dhcpEntry->rangeInd].options;

       if (opPointer) {
         MYBYTE requestedOnly = *opPointer;
         opPointer++;

         while (*opPointer && *opPointer != DHCP_OPTION_END) {
           op.opt_code = *opPointer;
           opPointer++;
           op.size = *opPointer;
           opPointer++;

           if (!requestedOnly || req->paramreqlist[*opPointer]) {
             memcpy(op.value, opPointer, op.size);
             pvdata(req, &op);
           }
           opPointer += op.size;
         }
        }
      }

      MYBYTE* opPointer = cfig.options;

      if (opPointer) {
        MYBYTE requestedOnly = *opPointer;

        opPointer++;
        while (*opPointer && *opPointer != DHCP_OPTION_END) {
          op.opt_code = *opPointer;
          opPointer++;
          op.size = *opPointer;
          opPointer++;

          if (!requestedOnly || req->paramreqlist[*opPointer]) {
            memcpy(op.value, opPointer, op.size);
            pvdata(req, &op);
          }
          opPointer += op.size;
        }
      }

      op.opt_code = DHCP_OPTION_SERVERID;
      op.size = 4;
      pIP(op.value, nd.dhcpConn[req->sockInd].server);
      pvdata(req, &op);

      if (!req->opAdded[DHCP_OPTION_IPADDRLEASE]) {
        op.opt_code = DHCP_OPTION_IPADDRLEASE;
        op.size = 4;
        pULong(op.value, cfig.lease);
        pvdata(req, &op);
      }

      if (!req->opAdded[DHCP_OPTION_NETMASK]) {
        op.opt_code = DHCP_OPTION_NETMASK;
        op.size = 4;

        if (req->dhcpEntry->rangeInd >= 0)
          pIP(op.value, cfig.dhcpRanges[req->dhcpEntry->rangeInd].mask);
        else
          pIP(op.value, cfig.mask);

        pvdata(req, &op);
      }

      if (!req->hostname[0])
        genHostName(req->hostname, req->dhcpp.header.bp_chaddr, req->dhcpp.header.bp_hlen);

      if (!req->dhcpEntry->hostname)
        req->dhcpEntry->hostname = cloneString(req->hostname);
      else if (strcasecmp(req->dhcpEntry->hostname, req->hostname)) {
        free(req->dhcpEntry->hostname);
        req->dhcpEntry->hostname = cloneString(req->hostname);
      }

      if (req->subnet.opt_code == DHCP_OPTION_SUBNETSELECTION)
        pvdata(req, &req->subnet);

      if (req->agentOption.opt_code == DHCP_OPTION_RELAYAGENTINFO)
        pvdata(req, &req->agentOption);
    }
  }

  *(req->vp) = DHCP_OPTION_END;
}

void pvdata(Request* req, OptCode* op) {

  if (!req->opAdded[op->opt_code] && ((req->vp - (MYBYTE*)&req->dhcpp) + op->size < req->messsize)) {

    if (op->opt_code == DHCP_OPTION_NEXTSERVER) req->dhcpp.header.bp_siaddr = fIP(op->value);
    else if (op->opt_code == DHCP_OPTION_BP_FILE) {
      if (op->size <= 128) memcpy(req->dhcpp.header.bp_file, op->value, op->size);
    } else if (op->size) {
      if (op->opt_code == DHCP_OPTION_IPADDRLEASE) {
        if (!req->lease || req->lease > fULong(op->value)) req->lease = fULong(op->value);
        if (req->lease >= INT_MAX) req->lease = UINT_MAX;
        pULong(op->value, req->lease);
      } else if (op->opt_code == DHCP_OPTION_REBINDINGTIME) req->rebind = fULong(op->value);
      else if (op->opt_code == DHCP_OPTION_HOSTNAME) {
        memcpy(req->hostname, op->value, op->size);
        req->hostname[op->size] = 0;
        req->hostname[63] = 0;
        if (char* ptr = strchr(req->hostname, '.')) *ptr = 0;
      }

      MYWORD tsize = op->size + 2;
      memcpy(req->vp, op, tsize);
      (req->vp) += tsize;
    }
    req->opAdded[op->opt_code] = true;
  }
}

void setTempLease(Data* dhcpEntry) {
  if (dhcpEntry && dhcpEntry->ip) {
    dhcpEntry->display = false;
    dhcpEntry->local = false;
    dhcpEntry->expiry = ld.t + 20;

    int ind = getIndex(dhcpEntry->rangeInd, dhcpEntry->ip);

    if (ind >= 0) {
      if (cfig.dhcpRanges[dhcpEntry->rangeInd].expiry[ind] != INT_MAX)
        cfig.dhcpRanges[dhcpEntry->rangeInd].expiry[ind] = dhcpEntry->expiry;

      cfig.dhcpRanges[dhcpEntry->rangeInd].dhcpEntry[ind] = dhcpEntry;
    }
  }
}

void setLeaseExpiry(Data* dhcpEntry, MYDWORD lease) {
  if (dhcpEntry && dhcpEntry->ip) {
    if (lease > (MYDWORD)(INT_MAX - ld.t)) dhcpEntry->expiry = INT_MAX;
    else dhcpEntry->expiry = ld.t + lease;

    int ind = getIndex(dhcpEntry->rangeInd, dhcpEntry->ip);

    if (ind >= 0) {
      if (cfig.dhcpRanges[dhcpEntry->rangeInd].expiry[ind] != INT_MAX)
        cfig.dhcpRanges[dhcpEntry->rangeInd].expiry[ind] = dhcpEntry->expiry;

      cfig.dhcpRanges[dhcpEntry->rangeInd].dhcpEntry[ind] = dhcpEntry;
    }
  }
}

void setLeaseExpiry(Data* dhcpEntry) {
  if (dhcpEntry && dhcpEntry->ip) {
    int ind = getIndex(dhcpEntry->rangeInd, dhcpEntry->ip);
    if (ind >= 0) {
      if (cfig.dhcpRanges[dhcpEntry->rangeInd].expiry[ind] != INT_MAX)
        cfig.dhcpRanges[dhcpEntry->rangeInd].expiry[ind] = dhcpEntry->expiry;
      cfig.dhcpRanges[dhcpEntry->rangeInd].dhcpEntry[ind] = dhcpEntry;
    }
  }
}

void lockIP(MYDWORD ip) {
  MYDWORD iip = htonl(ip);

  for (char rangeInd = 0; rangeInd < cfig.rangeCount; rangeInd++) {
    if (iip >= cfig.dhcpRanges[rangeInd].rangeStart && iip <= cfig.dhcpRanges[rangeInd].rangeEnd) {
      int ind = iip - cfig.dhcpRanges[rangeInd].rangeStart;

      if (cfig.dhcpRanges[rangeInd].expiry[ind] != INT_MAX)
         cfig.dhcpRanges[rangeInd].expiry[ind] = INT_MAX;

      break;
    }
  }
}

void holdIP(MYDWORD ip) {
  if (ip) {
    MYDWORD iip = htonl(ip);
    for (char rangeInd = 0; rangeInd < cfig.rangeCount; rangeInd++) {
      if (iip >= cfig.dhcpRanges[rangeInd].rangeStart && iip <= cfig.dhcpRanges[rangeInd].rangeEnd) {
        int ind = iip - cfig.dhcpRanges[rangeInd].rangeStart;
        if (cfig.dhcpRanges[rangeInd].expiry[ind] == 0) cfig.dhcpRanges[rangeInd].expiry[ind] = 1;
        break;
      }
    }
  }
}

void __cdecl sendToken(void* arg) {
  Sleep(1000 * 10);
  while (true) {
    errno = 0;
    sendto(cfig.dhcpReplConn.sock, token.raw, token.bytes, 0, (sockaddr*)&token.remote, sizeof(token.remote));
    Sleep(1000 * 300);
  }
  _endthread();
  return;
}

MYDWORD sendRepl(Request* req) {
  OptCode op;

  MYBYTE* opPointer = req->dhcpp.vend_data;

  while ((*opPointer) != DHCP_OPTION_END && opPointer < req->vp) {
    if ((*opPointer) == DHCP_OPTION_MESSAGETYPE) {
      *(opPointer + 2) = DHCP_MESS_INFORM;
      break;
    }
    opPointer = opPointer + *(opPointer + 1) + 2;
  }

  if (!req->opAdded[DHCP_OPTION_MESSAGETYPE]) {
    op.opt_code = DHCP_OPTION_MESSAGETYPE;
    op.size = 1;
    op.value[0] = DHCP_MESS_INFORM;
    pvdata(req, &op);
  }

  if (req->hostname[0] && !req->opAdded[DHCP_OPTION_HOSTNAME]) {
    op.opt_code = DHCP_OPTION_HOSTNAME;
    op.size = strlen(req->hostname);
    memcpy(op.value, req->hostname, op.size);
    pvdata(req, &op);
  }

  *(req->vp) = DHCP_OPTION_END;
  req->vp++;
  req->bytes = req->vp - (MYBYTE*)req->raw;
  req->dhcpp.header.bp_op = BOOTP_REQUEST;
  errno = 0;
  req->bytes = sendto(cfig.dhcpReplConn.sock, req->raw, req->bytes, 0, (sockaddr*)&token.remote, sizeof(token.remote));
  errno = WSAGetLastError();

  if (errno || req->bytes <= 0) {
    cfig.dhcpRepl = 0;
    if (cfig.replication == 1)
      sprintf(lb.log, "WSAError %u Sending DHCP Update to Secondary Server", errno);
    else
      sprintf(lb.log, "WSAError %u Sending DHCP Update to Primary Server", errno);
    logMesg(lb.log, LOG_NOTICE);
    return 0;
  } else {
    if (cfig.replication == 1)
      sprintf(lb.log, "DHCP Update for host %s (%s) sent to Secondary Server", req->dhcpEntry->mapname, IP2String(lb.tmp, req->dhcpEntry->ip));
    else
      sprintf(lb.log, "DHCP Update for host %s (%s) sent to Primary Server", req->dhcpEntry->mapname, IP2String(lb.tmp, req->dhcpEntry->ip));
    logMesg(lb.log, LOG_INFO);
  }

  return req->dhcpp.header.bp_yiaddr;
}

void recvRepl(Request* req) {

  cfig.dhcpRepl = ld.t + 600;

  MYDWORD ip = req->dhcpp.header.bp_yiaddr ? req->dhcpp.header.bp_yiaddr : req->dhcpp.header.bp_ciaddr;

  if (!ip || !req->dhcpp.header.bp_hlen) {
    if (cfig.replication == 1) {
      errno = 0;
      sendto(cfig.dhcpReplConn.sock, token.raw, token.bytes, 0, (sockaddr*)&token.remote, sizeof(token.remote));
    }
    return;
  }

  char rInd = getRangeInd(ip);

  if (rInd >= 0) {
    int ind  = getIndex(rInd, ip);
    req->dhcpEntry = cfig.dhcpRanges[rInd].dhcpEntry[ind];
    if (req->dhcpEntry && !req->dhcpEntry->fixed && strcasecmp(req->dhcpEntry->mapname, req->chaddr))
      req->dhcpEntry->expiry = 0;
  }

  req->dhcpEntry = findDHCPEntry(req->chaddr);

  if (req->dhcpEntry && req->dhcpEntry->ip != ip) {
    if (req->dhcpEntry->fixed) {
      if (cfig.replication == 1)
        sprintf(lb.log, "DHCP Update ignored for %s (%s) from Secondary Server", req->chaddr, IP2String(lb.tmp, ip));
      else
        sprintf(lb.log, "DHCP Update ignored for %s (%s) from Primary Server", req->chaddr, IP2String(lb.tmp, ip));
      logMesg(lb.log, LOG_NOTICE);
      return;
    } else if (req->dhcpEntry->rangeInd >= 0) {
      int ind = getIndex(req->dhcpEntry->rangeInd, req->dhcpEntry->ip);
      if (ind >= 0) cfig.dhcpRanges[req->dhcpEntry->rangeInd].dhcpEntry[ind] = 0;
    }
  }

  if (!req->dhcpEntry && rInd >= 0) {
    memset(&lump, 0, sizeof(lumpData));
    lump.mapname = req->chaddr;
    lump.hostname = req->hostname;
    req->dhcpEntry = createCache(&lump);
    if (req->dhcpEntry) dhcpCache[req->dhcpEntry->mapname] = req->dhcpEntry;
  }

  if (req->dhcpEntry) {
    req->dhcpEntry->ip = ip;
    req->dhcpEntry->rangeInd = rInd;
    req->dhcpEntry->display = true;
    req->dhcpEntry->local = false;
    MYDWORD hangTime = req->lease;

    if (req->rebind > req->lease) hangTime = req->rebind;
    setLeaseExpiry(req->dhcpEntry, hangTime);

    if (req->hostname[0]) {
      if (req->dhcpEntry->hostname && strcasecmp(req->dhcpEntry->hostname, req->hostname)) {
        free (req->dhcpEntry->hostname);
        req->dhcpEntry->hostname = cloneString(req->hostname);
      } else if (!req->dhcpEntry->hostname)
        req->dhcpEntry->hostname = cloneString(req->hostname);
    }

    _beginthread(updateStateFile, 0, (void*)req->dhcpEntry);
  } else {
    if (cfig.replication == 1)
      sprintf(lb.log, "DHCP Update ignored for %s (%s) from Secondary Server", req->chaddr, IP2String(lb.tmp, ip));
    else
      sprintf(lb.log, "DHCP Update ignored for %s (%s) from Primary Server", req->chaddr, IP2String(lb.tmp, ip));
    logMesg(lb.log, LOG_NOTICE);
    return;
  }

  if (cfig.replication == 1)
    sprintf(lb.log, "DHCP Update received for %s (%s) from Secondary Server", req->chaddr, IP2String(lb.tmp, ip));
  else
    sprintf(lb.log, "DHCP Update received for %s (%s) from Primary Server", req->chaddr, IP2String(lb.tmp, ip));
  logMesg(lb.log, LOG_INFO);
}

char getRangeInd(MYDWORD ip) {
  if (ip) {
    MYDWORD iip = htonl(ip);
    for (char k = 0; k < cfig.rangeCount; k++)
      if (iip >= cfig.dhcpRanges[k].rangeStart && iip <= cfig.dhcpRanges[k].rangeEnd) return k;
  }
  return -1;
}

int getIndex(char rangeInd, MYDWORD ip) {
  if (ip && rangeInd >= 0 && rangeInd < cfig.rangeCount) {
    MYDWORD iip = htonl(ip);
    if (iip >= cfig.dhcpRanges[rangeInd].rangeStart && iip <= cfig.dhcpRanges[rangeInd].rangeEnd)
      return (iip - cfig.dhcpRanges[rangeInd].rangeStart);
  }
  return -1;
}

void loadOptions(FILE* f, const char* sectionName, Opt* optionData) {
  optionData->ip = 0;
  optionData->mask = 0;
  MYBYTE maxInd = sizeof(opData) / sizeof(OPS);
  MYWORD buffsize = sizeof(Packet) - sizeof(Header);
  MYBYTE* dp = optionData->options;
  MYBYTE op_specified[256];

  memset(op_specified, 0, 256);
  *dp = 0;
  dp++;

  char raw[512];
  char name[512];
  char value[512];

  while (readSection(raw, f)) {

    MYBYTE* ddp = dp;
    MYBYTE hoption[256];
    MYBYTE valSize = sizeof(hoption) - 1;
    MYBYTE opTag = 0;
    MYBYTE opType = 0;
    MYBYTE valType = 0;
    bool tagFound = false;

    mySplit(name, value, raw, '=');

    if (!name[0]) {
      sprintf(lb.log, "Warning: section [%s] invalid option %s ignored", sectionName, raw);
      logMesg(lb.log, LOG_NOTICE);
      continue;
    }

    if (!strcasecmp(name, "DHCPRange")) {
      if (!strcasecmp(sectionName, RANGESET)) addDHCPRange(value);
      else {
        sprintf(lb.log, "Warning: section [%s] option %s not allowed in this section, option ignored", sectionName, raw);
        logMesg(lb.log, LOG_NOTICE);
      }
      continue;
    } else if (!strcasecmp(name, "IP")) {
      if (!strcasecmp(sectionName, GLOBALOPTIONS) || !strcasecmp(sectionName, RANGESET)) {
        sprintf(lb.log, "Warning: section [%s] option %s not allowed in this section, option ignored", sectionName, raw);
        logMesg(lb.log, LOG_NOTICE);
      } else if (!isIP(value) && strcasecmp(value, "0.0.0.0")) {
        sprintf(lb.log, "Warning: section [%s] option Invalid IP Addr %s option ignored", sectionName, value);
        logMesg(lb.log, LOG_NOTICE);
      } else
        optionData->ip = inet_addr(value);
      continue;
    } else if (!strcasecmp(name, "FilterMacRange")) {
      if (!strcasecmp(sectionName, RANGESET)) addMacRange(optionData->rangeSetInd, value);
      else {
        sprintf(lb.log, "Warning: section [%s] option %s not allowed in this section, option ignored", sectionName, raw);
        logMesg(lb.log, LOG_NOTICE);
      }
      continue;
    }

    if (!value[0]) valType = 9;
    else if (value[0] == '"' && value[strlen(value)-1] == '"') {
      valType = 2;
      value[0] = NBSP;
      value[strlen(value) - 1] = NBSP;
      myTrim(value, value);

      if (strlen(value) <= UCHAR_MAX) valSize = strlen(value);
      else {
        sprintf(lb.log, "Warning: section [%s] option %s value too big, option ignored", sectionName, raw);
        logMesg(lb.log, LOG_NOTICE);
        continue;
      }
    } else if (strchr(value, ':')) {
      valType = 2;
      valSize = sizeof(hoption) - 1;
      char* errorPos = getHexValue(hoption, value, &valSize);

      if (errorPos) {
        valType = 1;
        valSize = strlen(value);
      } else memcpy(value, hoption, valSize);
    } else if (isInt(value) && atol(value) > USHRT_MAX) valType = 4;
      else if (isInt(value) && atoi(value) > UCHAR_MAX) valType = 5;
      else if (isInt(value)) valType = 6;
      else if (strchr(value, '.') || strchr(value, ',')) {
        valType = 2;
        char buff[1024];
        int numbytes = myTokenize(buff, value, "/,.", true);

        if (numbytes > 255) {
          sprintf(lb.log, "Warning: section [%s] option %s, too many bytes, entry ignored", sectionName, raw);
          logMesg(lb.log, LOG_NOTICE);
          continue;
        } else {
          char* ptr = buff;
          valSize = 0;

          for (; *ptr; ptr = myGetToken(ptr, 1)) {
            if (isInt(ptr) && atoi(ptr) <= UCHAR_MAX) {
              hoption[valSize] = atoi(ptr);
              valSize++;
            } else break;
          }

          if (!(*ptr)) memcpy(value, hoption, valSize);
          else {
            valType = 1;
            valSize = strlen(value);
          }
        }
      } else {
        if (strlen(value) <= UCHAR_MAX) {
  	      valSize = strlen(value);
          valType = 1;
        }
      else {
        sprintf(lb.log, "Warning: section [%s] option %s value too long, option ignored", sectionName, raw);
        logMesg(lb.log, LOG_NOTICE);
        continue;
      }
    }

    if (!strcasecmp(name, "FilterVendorClass")) {
      if (!strcasecmp(sectionName, RANGESET)) addVendClass(optionData->rangeSetInd, value, valSize);
      else {
        sprintf(lb.log, "Warning: section [%s] option %s not allowed in this section, option ignored", sectionName, raw);
        logMesg(lb.log, LOG_NOTICE);
      }
      continue;
    } else if (!strcasecmp(name, "FilterUserClass")) {
      if (!strcasecmp(sectionName, RANGESET))
        addUserClass(optionData->rangeSetInd, value, valSize);
      else {
        sprintf(lb.log, "Warning: section [%s] option %s not allowed in this section, option ignored", sectionName, raw);
        logMesg(lb.log, LOG_NOTICE);
      }
      continue;
    } else if (!strcasecmp(name, "FilterSubnetSelection")) {
      if (valSize != 4) {
        sprintf(lb.log, "Warning: section [%s] invalid value %s, option ignored", sectionName, raw);
        logMesg(lb.log, LOG_NOTICE);
      } else if (!strcasecmp(sectionName, RANGESET)) {
        addServer(cfig.rangeSet[optionData->rangeSetInd].subnetIP, MAX_RANGE_FILTERS, fIP(value));
        cfig.hasFilter = 1;
      } else {
        sprintf(lb.log, "Warning: section [%s] option %s not allowed in this section, option ignored", sectionName, raw);
        logMesg(lb.log, LOG_NOTICE);
      }
      continue;
    } else if (!strcasecmp(name, "TargetRelayAgent")) {
      if (valSize != 4) {
        sprintf(lb.log, "Warning: section [%s] invalid value %s, option ignored", sectionName, raw);
        logMesg(lb.log, LOG_NOTICE);
      } else if (!strcasecmp(sectionName, RANGESET)) {
        cfig.rangeSet[optionData->rangeSetInd].targetIP = fIP(value);
      } else {
        sprintf(lb.log, "Warning: section [%s] option %s not allowed in this section, option ignored", sectionName, raw);
        logMesg(lb.log, LOG_NOTICE);
      }
      continue;
    }
    opTag = 0;

    if (isInt(name)) {
      if (atoi(name) < 1 || atoi(name) >= 254) {
        sprintf(lb.log, "Warning: section [%s] invalid option %s, ignored", sectionName, raw);
        logMesg(lb.log, LOG_NOTICE);
        continue;
      }
      opTag = atoi(name);
      opType = 0;
    }

    for (MYBYTE i = 0; i < maxInd; i++)
      if (!strcasecmp(name, opData[i].opName) || (opTag && opTag == opData[i].opTag)) {
        opTag = opData[i].opTag;
        opType = opData[i].opType;
        tagFound = true;
        break;
      }

    if (!opTag) {
      sprintf(lb.log, "Warning: section [%s] invalid option %s, ignored", sectionName, raw);
      logMesg(lb.log, LOG_NOTICE);
      continue;
    }

    if (!opType) opType = valType;

    if (op_specified[opTag]) {
      sprintf(lb.log, "Warning: section [%s] duplicate option %s, ignored", sectionName, raw);
      logMesg(lb.log, LOG_NOTICE);
      continue;
    }

    op_specified[opTag] = true;

    if (valType == 9) {
      if (buffsize > 2) {
        *dp = opTag; dp++; *dp = 0; dp++; buffsize -= 2;
      } else {
        sprintf(lb.log, "Warning: section [%s] option %s, no more space for options", sectionName, raw);
        logMesg(lb.log, LOG_NOTICE);
      }
      continue;
    }

    switch (opType) {
      case 1:
        {
          value[valSize] = 0;
          valSize++;
          if (valType != 1 && valType != 2) {
            sprintf(lb.log, "Warning: section [%s] option %s, need string value, option ignored", sectionName, raw);
            logMesg(lb.log, LOG_NOTICE);
          } else if (buffsize > valSize + 2) {
            *dp = opTag; dp++; *dp = valSize; dp++; memcpy(dp, value, valSize); dp += valSize; buffsize -= (valSize + 2);
          } else {
            sprintf(lb.log, "Warning: section [%s] option %s, no more space for options", sectionName, raw);
            logMesg(lb.log, LOG_NOTICE);
          }
        }
        break;
      case 3:
      case 8:
        {
          if (valType == 2) {
            if (opType == 3 && valSize % 4) {
              sprintf(lb.log, "Warning: section [%s] option %s, missing/extra bytes/octates in IP, option ignored", sectionName, raw);
              logMesg(lb.log, LOG_NOTICE);
              continue;
	          } else if (opType == 8 && valSize % 8) {
              sprintf(lb.log, "Warning: section [%s] option %s, some values not in IP/Mask form, option ignored", sectionName, raw);
              logMesg(lb.log, LOG_NOTICE);
              continue;
            }
            if (opTag == DHCP_OPTION_NETMASK) {
              if (valSize != 4 || !checkMask(fIP(value))) {
                sprintf(lb.log, "Warning: section [%s] Invalid subnetmask %s, option ignored", sectionName, raw);
                logMesg(lb.log, LOG_NOTICE);
                continue;
              } else optionData->mask = fIP(value);
     	      }
            if (buffsize > valSize + 2) {
              *dp = opTag; dp++; *dp = valSize; dp++; memcpy(dp, value, valSize); dp += valSize; buffsize -= (valSize + 2);
            } else {
	            sprintf(lb.log, "Warning: section [%s] option %s, no more space for options", sectionName, raw);
              logMesg(lb.log, LOG_NOTICE);
            }
          } else {
            sprintf(lb.log, "Warning: section [%s] option %s, Invalid value, should be one or more IP/4 Bytes", sectionName, raw);
            logMesg(lb.log, LOG_NOTICE);
          }
        }
        break;
      case 4:
        {
          MYDWORD j;

          if (valType == 2 && valSize == 4) j = fULong(value);
          else if (valType >= 4 && valType <= 6) j = atol(value);
          else {
            sprintf(lb.log, "Warning: section [%s] option %s, value should be integer between 0 & %u or 4 bytes, option ignored", sectionName, name, UINT_MAX);
            logMesg(lb.log, LOG_NOTICE);
            continue;
          }

          if (opTag == DHCP_OPTION_IPADDRLEASE) { if (j == 0) j = UINT_MAX; }

          if (buffsize > 6) {
            *dp = opTag; dp++; *dp = 4; dp++; dp += pULong(dp, j); buffsize -= 6;
          } else {
            sprintf(lb.log, "Warning: section [%s] option %s, no more space for options", sectionName, raw);
            logMesg(lb.log, LOG_NOTICE);
          }
        }
        break;
      case 5:
        {
          MYWORD j;

          if (valType == 2 && valSize == 2) j = fUShort(value);
          else if (valType == 5 || valType == 6) j = atol(value);
          else {
            sprintf(lb.log, "Warning: section [%s] option %s, value should be between 0 & %u or 2 bytes, option ignored", sectionName, name, USHRT_MAX);
            logMesg(lb.log, LOG_NOTICE);
            continue;
          }

          if (buffsize > 4) {
            *dp = opTag; dp++; *dp = 2; dp++; dp += pUShort(dp, j); buffsize -= 4;
          } else {
            sprintf(lb.log, "Warning: section [%s] option %s, no more space for options", sectionName, raw);
            logMesg(lb.log, LOG_NOTICE);
          }
        }
        break;
      case 6:
        {
          MYBYTE j;

          if (valType == 2 && valSize == 1) j = *value;
          else if (valType == 6) j = atol(value);
          else {
            sprintf(lb.log, "Warning: section [%s] option %s, value should be between 0 & %u or single byte, option ignored", sectionName, name, UCHAR_MAX);
            logMesg(lb.log, LOG_NOTICE);
            continue;
          }

          if (buffsize > 3) {
            *dp = opTag; dp++; *dp = 1; dp++; *dp = j; dp++; buffsize -= 3;
          } else {
            sprintf(lb.log, "Warning: section [%s] option %s, no more space for options", sectionName, raw);
            logMesg(lb.log, LOG_NOTICE);
          }
        }
        break;
      case 7:
        {
           MYBYTE j;
           if (valType == 2 && valSize == 1 && *value < 2) j = *value;
           else if (valType == 1 && (!strcasecmp(value, "yes") || !strcasecmp(value, "on") || !strcasecmp(value, "true"))) j = 1;
           else if (valType == 1 && (!strcasecmp(value, "no") || !strcasecmp(value, "off") || !strcasecmp(value, "false"))) j = 0;
      	   else if (valType == 6 && atoi(value) < 2) j = atoi(value);
           else {
             sprintf(lb.log, "Warning: section [%s] option %s, value should be yes/on/true/1 or no/off/false/0, option ignored", sectionName, raw);
             logMesg(lb.log, LOG_NOTICE);
             continue;
           }
           if (buffsize > 3) {
             *dp = opTag; dp++; *dp = 1; dp++; *dp = j; dp++; buffsize -= 3;
           } else {
             sprintf(lb.log, "Warning: section [%s] option %s, no more space for options", sectionName, raw);
             logMesg(lb.log, LOG_NOTICE);
          }
        }
        break;
      default:
        {
	  if (valType == 6) { valType = 2; valSize = 1; *value = atoi(value); }
          if (opType == 2 && valType != 2) {
	    sprintf(lb.log, "Warning: section [%s] option %s, value should be comma separated bytes or hex string, option ignored", sectionName, raw);
	    logMesg(lb.log, LOG_NOTICE);
	    continue;
	  } else if (buffsize > valSize + 2) {
	    *dp = opTag;
	    dp++;
	    *dp = valSize;
	    dp++;
	    memcpy(dp, value, valSize);
	    dp += valSize;
	    buffsize -= (valSize + 2);
	  } else {
	    sprintf(lb.log, "Warning: section [%s] option %s, no more space for options", sectionName, raw);
	    logMesg(lb.log, LOG_NOTICE);
          }
        }
        break;
    }
  }
  *dp = DHCP_OPTION_END; dp++;
  optionData->optionSize = (dp - optionData->options);
}

void lockOptions(FILE* f) {
  char raw[512];
  char name[512];
  char value[512];

  while (readSection(raw, f)) {
    mySplit(name, value, raw, '=');

    if (!name[0] || !value[0]) continue;

    int op_index;
    MYBYTE n = sizeof(opData) / sizeof(OPS);

    for (op_index = 0; op_index < n; op_index++)
      if (!strcasecmp(name, opData[op_index].opName) || (opData[op_index].opTag && atoi(name) == opData[op_index].opTag)) break;

    if (op_index >= n) continue;

    if (opData[op_index].opType == 3) {

      if (myTokenize(value, value, "/,.", true)) {
        char* ptr = value;
        char hoption[256];
        MYBYTE valueSize = 0;

        for (; *ptr; ptr = myGetToken(ptr, 1)) {
          if (valueSize >= UCHAR_MAX) break;
          else if (isInt(ptr) && atoi(ptr) <= UCHAR_MAX) {
            hoption[valueSize] = atoi(ptr);
            valueSize++;
          } else break;
        }

        if (*ptr) continue;
        if (valueSize % 4) continue;

        for (MYBYTE i = 0; i < valueSize; i += 4) {
          MYDWORD ip = *((MYDWORD*)&(hoption[i]));
          if (ip != INADDR_ANY && ip != INADDR_NONE) lockIP(ip);
        }
      }
    }
  }
}

void addDHCPRange(char* dp) {
  MYDWORD rs = 0;
  MYDWORD re = 0;
  char name[512];
  char value[512];
  mySplit(name, value, dp, '-');

  if (isIP(name) && isIP(value)) {
    rs = htonl(inet_addr(name));
    re = htonl(inet_addr(value));

    if (rs && re && rs <= re) {
      Range* range;
      MYBYTE m = 0;

      for (; m < MAX_DHCP_RANGES && cfig.dhcpRanges[m].rangeStart; m++) {
        range = &cfig.dhcpRanges[m];

        if ((rs >= range->rangeStart && rs <= range->rangeEnd) ||
            (re >= range->rangeStart && re <= range->rangeEnd) ||
            (range->rangeStart >= rs && range->rangeStart <= re) ||
            (range->rangeEnd >= rs && range->rangeEnd <= re)) {
          sprintf(lb.log, "Warning: DHCP Range %s overlaps with another range, ignored", dp);
          logMesg(lb.log, LOG_NOTICE);
          return;
        }
      }

      if (m < MAX_DHCP_RANGES) {
        cfig.dhcpSize += (re - rs + 1);
        range = &cfig.dhcpRanges[m];
        range->rangeStart = rs;
        range->rangeEnd = re;
        range->expiry = (time_t*)calloc((re - rs + 1), sizeof(time_t));
        range->dhcpEntry = (Data**)calloc((re - rs + 1), sizeof(Data*));

        if (!range->expiry || !range->dhcpEntry) {
          if (range->expiry) free(range->expiry);
          if (range->dhcpEntry) free(range->dhcpEntry);
          sprintf(lb.log, "DHCP Ranges Load, Memory Allocation Error");
          logMesg(lb.log, LOG_NOTICE);
          return;
        }
      }
    } else {
      sprintf(lb.log, "Section [%s] Invalid DHCP range %s in ini file, ignored", RANGESET, dp);
      logMesg(lb.log, LOG_NOTICE);
    }
  } else {
    sprintf(lb.log, "Section [%s] Invalid DHCP range %s in ini file, ignored", RANGESET, dp);
    logMesg(lb.log, LOG_NOTICE);
  }
}

void addVendClass(MYBYTE rangeSetInd, char* vendClass, MYBYTE vendClassSize) {
  RangeSets* rangeSet = &cfig.rangeSet[rangeSetInd];

  MYBYTE i = 0;

  for (; i <= MAX_RANGE_FILTERS && rangeSet->vendClassSize[i]; i++);

  if (i >= MAX_RANGE_FILTERS || !vendClassSize) return;

  rangeSet->vendClass[i] = (MYBYTE*)calloc(vendClassSize, 1);

  if(!rangeSet->vendClass[i]) {
    sprintf(lb.log, "Vendor Class Load, Memory Allocation Error");
    logMesg(lb.log, LOG_NOTICE);
  } else {
    cfig.hasFilter = true;
    rangeSet->vendClassSize[i] = vendClassSize;
    memcpy(rangeSet->vendClass[i], vendClass, vendClassSize);
  }
}

void addUserClass(MYBYTE rangeSetInd, char* userClass, MYBYTE userClassSize) {
  RangeSets* rangeSet = &cfig.rangeSet[rangeSetInd];

  MYBYTE i = 0;

  for (; i <= MAX_RANGE_FILTERS && rangeSet->userClassSize[i]; i++);

  if (i >= MAX_RANGE_FILTERS || !userClassSize) return;

  rangeSet->userClass[i] = (MYBYTE*)calloc(userClassSize, 1);

  if (!rangeSet->userClass[i]) {
    sprintf(lb.log, "Vendor Class Load, Memory Allocation Error");
    logMesg(lb.log, LOG_NOTICE);
  } else {
    cfig.hasFilter = true;
    rangeSet->userClassSize[i] = userClassSize;
    memcpy(rangeSet->userClass[i], userClass, userClassSize);
  }
}

void addMacRange(MYBYTE rangeSetInd, char* macRange) {
  if (macRange[0]) {
    RangeSets* rangeSet = &cfig.rangeSet[rangeSetInd];

    MYBYTE i = 0;

    for (; i <= MAX_RANGE_FILTERS && rangeSet->macSize[i]; i++);

    if (i >= MAX_RANGE_FILTERS) return;

    char name[256];
    char value[256];

    mySplit(name, value, macRange, '-');

    if(!name[0] || !value[0]) {
      sprintf(lb.log, "Section [%s], invalid Filter_Mac_Range %s, ignored", RANGESET, macRange);
      logMesg(lb.log, LOG_NOTICE);
    } else {
      MYBYTE macSize1 = 16;
      MYBYTE macSize2 = 16;
      MYBYTE* macStart = (MYBYTE*)calloc(1, macSize1);
      MYBYTE* macEnd = (MYBYTE*)calloc(1, macSize2);

      if (!macStart || !macEnd) {
        sprintf(lb.log, "DHCP Range Load, Memory Allocation Error");
        logMesg(lb.log, LOG_NOTICE);
      } else if (getHexValue(macStart, name, &macSize1) || getHexValue(macEnd, value, &macSize2)) {
        sprintf(lb.log, "Section [%s], Invalid character in Filter_Mac_Range %s", RANGESET, macRange);
        logMesg(lb.log, LOG_NOTICE);
        free(macStart);
        free(macEnd);
      } else if (memcmp(macStart, macEnd, 16) > 0) {
        sprintf(lb.log, "Section [%s], Invalid Filter_Mac_Range %s, (higher bound specified on left), ignored", RANGESET, macRange);
        logMesg(lb.log, LOG_NOTICE);
        free(macStart);
        free(macEnd);
      } else if (macSize1 != macSize2) {
        sprintf(lb.log, "Section [%s], Invalid Filter_Mac_Range %s, (start/end size mismatched), ignored", RANGESET, macRange);
        logMesg(lb.log, LOG_NOTICE);
        free(macStart);
        free(macEnd);
      } else {
        cfig.hasFilter = true;
        rangeSet->macSize[i] = macSize1;
        rangeSet->macStart[i] = macStart;
        rangeSet->macEnd[i] = macEnd;
      }
    }
  }
}

FILE* openSection(const char* sectionName, MYBYTE serial) {

  char section[128];
  sprintf(section, "[%s]", sectionName);
  toUpper(section);
  FILE* f = NULL;
  f = fopen(path.ini, "rt");

  if (f) {
    char buff[512];
    MYBYTE found = 0;

    while (fgets(buff, 511, f)) {
      toUpper(buff);
      myTrim(buff, buff);

      if (strstr(buff, section) == buff) {
        found++;

      	if (found == serial) {
	        MYDWORD fpos = ftell(f);

          if (fgets(buff, 511, f)) {
	          myTrim(buff, buff);

   	        if (buff[0] == '@') {
	            fclose(f);
	            f = NULL;
    	        buff[0] = NBSP;
	            myTrim(buff, buff);
	            if (strchr(buff, '\\') || strchr(buff, '/')) strcpy(lb.tmp, buff);
	            else sprintf(lb.tmp, "%s\\%s", path.cfg, buff);
	            f = fopen(lb.tmp, "rt");
   	          if (f) return f;
	            else {
                sprintf(lb.log, "Error: Section [%s], file %s not found", sectionName, lb.tmp);
                logMesg(lb.log, LOG_NOTICE);
                return NULL;
	            }
	          } else { fseek(f, fpos, SEEK_SET); return f; }
	        }
	      }
      }
    }
    fclose(f);
  }
  return NULL;
}

char* readSection(char* buff, FILE* f) {
  while (fgets(buff, 511, f)) {
    myTrim(buff, buff);
    if (*buff == '[') break;
    if ((*buff) >= '0' && (*buff) <= '9' || (*buff) >= 'A' && (*buff) <= 'Z' || (*buff) >= 'a' && (*buff) <= 'z' || ((*buff) && strchr("/\\?*", (*buff)))) return buff;
  }
  fclose(f);
  return NULL;
}

char* strqtype(char* buff, MYBYTE qtype) {
  switch (qtype) {
    case 1:
      strcpy(buff, "A"); break;
    case 2:
      strcpy(buff, "NS"); break;
    case 3:
      strcpy(buff, "MD"); break;
    case 4:
      strcpy(buff, "MF"); break;
    case 5:
      strcpy(buff, "CNAME"); break;
    case 6:
      strcpy(buff, "SOA"); break;
    case 7:
      strcpy(buff, "MB"); break;
    case 8:
      strcpy(buff, "MG"); break;
    case 9:
      strcpy(buff, "MR"); break;
    case 10:
      strcpy(buff, "NULL"); break;
    case 11:
      strcpy(buff, "WKS"); break;
    case 12:
      strcpy(buff, "PTR"); break;
    case 13:
      strcpy(buff, "HINFO"); break;
    case 14:
      strcpy(buff, "MINFO"); break;
    case 15:
      strcpy(buff, "MX"); break;
    case 16:
      strcpy(buff, "TXT"); break;
    case 28:
      strcpy(buff, "AAAA"); break;
    case 251:
      strcpy(buff, "IXFR"); break;
    case 252:
      strcpy(buff, "AXFR"); break;
    case 253:
      strcpy(buff, "MAILB"); break;
    case 254:
      strcpy(buff, "MAILA"); break;
    default:
      strcpy(buff, "ANY"); break;
  }
  return buff;
}

char* genHostName(char* target, MYBYTE* hex, MYBYTE bytes) {
  char* dp = target;
  if (bytes) dp += sprintf(target, "Host%02x", *hex);
  else *target = 0;
  for (MYBYTE i = 1; i < bytes; i++) dp += sprintf(dp, "%02x", *(hex + i));
  return target;
}

void __cdecl updateStateFile(void* arg) {

  Data* dhcpEntry = (Data*) arg;
  Client dhcpData;
  memset(&dhcpData, 0, sizeof(Client));
  dhcpData.bp_hlen = 16;
  getHexValue(dhcpData.bp_chaddr, dhcpEntry->mapname, &dhcpData.bp_hlen);
  dhcpData.ip = dhcpEntry->ip;
  dhcpData.expiry = dhcpEntry->expiry;
  dhcpData.local = dhcpEntry->local;

  if (dhcpEntry->hostname) strcpy(dhcpData.hostname, dhcpEntry->hostname);

  WaitForSingleObject(ge.file, INFINITE);

  if (dhcpEntry->dhcpInd) {
    dhcpData.dhcpInd = dhcpEntry->dhcpInd;
    FILE* f = fopen(lb.lea, "rb+");

    if (f) {
      if (fseek(f, (dhcpData.dhcpInd - 1)*sizeof(Client), SEEK_SET) >= 0)
        fwrite(&dhcpData, sizeof(Client), 1, f);
      fclose(f);
    }
  } else {
    cfig.dhcpInd++;
    dhcpEntry->dhcpInd = cfig.dhcpInd;
    dhcpData.dhcpInd = cfig.dhcpInd;
    FILE* f = fopen(lb.lea, "ab");

    if (f) {
      fwrite(&dhcpData, sizeof(Client), 1, f);
      fclose(f);
    }
  }

  SetEvent(ge.file);
  _endthread();
  return;
}

void calcRangeLimits(MYDWORD ip, MYDWORD mask, MYDWORD* rangeStart, MYDWORD* rangeEnd) {
  *rangeStart = htonl(ip & mask) + 1;
  *rangeEnd = htonl(ip | (~mask)) - 1;
}

Data* findDHCPEntry(char* key) {
  toLower(key);
  dhcpMap::iterator it = dhcpCache.find(key);
  if (it == dhcpCache.end()) return NULL;
  else return it->second;
}

MYWORD gdmess(Request* req, MYBYTE sockInd) {
  memset(req, 0, sizeof(Request));
  req->sockInd = sockInd;
  req->sockLen = sizeof(req->remote);
  errno = 0;

  req->bytes = recvfrom(nd.dhcpConn[req->sockInd].sock, req->raw, sizeof(req->raw), 0, (sockaddr*) &req->remote, &req->sockLen);

  errno = WSAGetLastError();

  if (errno || req->bytes <= 0 || req->dhcpp.header.bp_op != BOOTP_REQUEST) return 0;

  hex2String(req->chaddr, req->dhcpp.header.bp_chaddr, req->dhcpp.header.bp_hlen);

  OptCode* op;
  MYBYTE* raw = req->dhcpp.vend_data;
  MYBYTE* rawEnd = raw + (req->bytes - sizeof(Header));
  MYBYTE maxInd = sizeof(opData) / sizeof(OPS);

  for (; raw < rawEnd && *raw != DHCP_OPTION_END;) {
    op = (OptCode*)raw;

    switch (op->opt_code) {
      case DHCP_OPTION_PAD:
        raw++; continue;
      case DHCP_OPTION_PARAMREQLIST:
        for (int ix = 0; ix < op->size; ix++) req->paramreqlist[op->value[ix]] = 1;
        break;
      case DHCP_OPTION_MESSAGETYPE:
        req->req_type = op->value[0];
        break;
      case DHCP_OPTION_SERVERID:
        req->server = fIP(op->value);
        break;
      case DHCP_OPTION_IPADDRLEASE:
        req->lease = fULong(op->value);
        break;
      case DHCP_OPTION_MAXDHCPMSGSIZE:
        req->messsize = fUShort(op->value);
        break;
      case DHCP_OPTION_REQUESTEDIPADDR:
        req->reqIP = fIP(op->value);
        break;
      case DHCP_OPTION_HOSTNAME:
        {
          memcpy(req->hostname, op->value, op->size);
          req->hostname[op->size] = 0;
          req->hostname[63] = 0;
          if (char* ptr = strchr(req->hostname, '.')) *ptr = 0;
        }
        break;
      case DHCP_OPTION_VENDORCLASSID:
        memcpy(&req->vendClass, op, op->size + 2);
        break;
      case DHCP_OPTION_USERCLASS:
        memcpy(&req->userClass, op, op->size + 2);
        break;
      case DHCP_OPTION_RELAYAGENTINFO:
        memcpy(&req->agentOption, op, op->size + 2);
        break;
      case DHCP_OPTION_CLIENTID:
        memcpy(&req->clientId, op, op->size + 2);
        break;
      case DHCP_OPTION_SUBNETSELECTION:
        memcpy(&req->subnet, op, op->size + 2);
        req->subnetIP = fULong(op->value);
        break;
      case DHCP_OPTION_REBINDINGTIME:
        req->rebind = fULong(op->value);
        break;
    }
    raw += 2;
    raw += op->size;
  }

  if (!req->subnetIP) {
    if (req->dhcpp.header.bp_giaddr) req->subnetIP = req->dhcpp.header.bp_giaddr;
    else req->subnetIP = nd.dhcpConn[req->sockInd].server;
  }

  if (!req->messsize) {
    if (req->req_type == DHCP_MESS_NONE) req->messsize = req->bytes;
    else req->messsize = sizeof(Packet);
  }

  if ((req->req_type == 1 || req->req_type == 3) && config.logging == LOG_DEBUG) {
    Request* req1 = (Request*)calloc(1, sizeof(Request));
    memcpy(req1, req, sizeof(Request));
    _beginthread(logDebug, 0, req1);
  }

  if (req->req_type == DHCP_MESS_NONE) {
    if (req->dhcpp.header.bp_giaddr)
      sprintf(lb.log, "BOOTPREQUEST for %s (%s) from RelayAgent %s received", req->chaddr, req->hostname, IP2String(lb.tmp, req->dhcpp.header.bp_giaddr));
    else
      sprintf(lb.log, "BOOTPREQUEST for %s (%s) from interface %s received", req->chaddr, req->hostname, IP2String(lb.tmp, nd.dhcpConn[req->sockInd].server));
  } else if (req->req_type == DHCP_MESS_DISCOVER) {
    if (req->dhcpp.header.bp_giaddr)
      sprintf(lb.log, "DHCPDISCOVER for %s (%s) from RelayAgent %s received", req->chaddr, req->hostname, IP2String(lb.tmp, req->dhcpp.header.bp_giaddr));
    else
      sprintf(lb.log, "DHCPDISCOVER for %s (%s) from interface %s received", req->chaddr, req->hostname, IP2String(lb.tmp, nd.dhcpConn[req->sockInd].server));
  } else if (req->req_type == DHCP_MESS_REQUEST) {
    if (req->dhcpp.header.bp_giaddr)
      sprintf(lb.log, "DHCPREQUEST for %s (%s) from RelayAgent %s received", req->chaddr, req->hostname, IP2String(lb.tmp, req->dhcpp.header.bp_giaddr));
    else
      sprintf(lb.log, "DHCPREQUEST for %s (%s) from interface %s received", req->chaddr, req->hostname, IP2String(lb.tmp, nd.dhcpConn[req->sockInd].server));
  }
  logMesg(lb.log, LOG_INFO);

  req->vp = req->dhcpp.vend_data;
  memset(req->vp, 0, sizeof(Packet) - sizeof(Header));
  return 1;
}

void __cdecl logDebug(void* arg) {
  char localBuff[1024];
  char localExtBuff[256];
  Request* req = (Request*) arg;
  genHostName(localBuff, req->dhcpp.header.bp_chaddr, req->dhcpp.header.bp_hlen);
  sprintf(localExtBuff, lb.cli, localBuff);
  FILE* f = fopen(localExtBuff, "at");

  if (f) {
    tm* ttm = localtime(&ld.t);
    strftime(localExtBuff, sizeof(localExtBuff), "%d-%m-%y %X", ttm);

    char* s = localBuff;
    s += sprintf(s, localExtBuff);
    s += sprintf(s, " SourceMac=%s", req->chaddr);
    s += sprintf(s, " ClientIP=%s", IP2String(localExtBuff, req->dhcpp.header.bp_ciaddr));
    s += sprintf(s, " SourceIP=%s", IP2String(localExtBuff, req->remote.sin_addr.s_addr));
    s += sprintf(s, " RelayAgent=%s", IP2String(localExtBuff, req->dhcpp.header.bp_giaddr));
    fprintf(f, "%s\n", localBuff);

    OptCode* op;
    MYBYTE* raw = req->dhcpp.vend_data;
    MYBYTE* rawEnd = raw + (req->bytes - sizeof(Header));
    MYBYTE maxInd = sizeof(opData) / sizeof(OPS);

    for (; raw < rawEnd && * raw != DHCP_OPTION_END;) {
      op = (OptCode*)raw;

      BYTE opType = 2;
      char opName[40] = "Private";

      for (MYBYTE i = 0; i < maxInd; i++)
        if (op->opt_code == opData[i].opTag) {
          strcpy(opName, opData[i].opName);
          opType = opData[i].opType;
          break;
        }

      s = localBuff; s += sprintf(s, "\t%d\t%s\t", op->opt_code, opName);

      switch (opType) {
        case 1:
          memcpy(localExtBuff, op->value, op->size);
          localExtBuff[op->size] = 0;
          sprintf(s, "%s", localExtBuff);
          break;
        case 3:
          for (BYTE x = 4; x <= op->size; x += 4) {
            IP2String(localExtBuff, fIP(op->value));
            s += sprintf(s, "%s,", localExtBuff);
          }
          break;
        case 4:
          sprintf(s, "%u", fULong(op->value));
          break;
        case 5:
          sprintf(s, "%u", fUShort(op->value));
          break;
        case 6:
      	case 7:
          sprintf(s, "%u", op->value[0]);
          break;
        default:
      	  if (op->size == 1) sprintf(s, "%u", op->value[0]);
          else hex2String(s, op->value, op->size);
          break;
      }
      fprintf(f, "%s\n", localBuff);
      raw += 2;
      raw += op->size;
    }
    fclose(f);
  }
  free(req);
}

Data* createCache(lumpData* lump) {
  MYWORD dataSize = 1 + sizeof(Data) + strlen(lump->mapname);
  dataSize += lump->optionSize;
  Data* cache = (Data*)calloc(1, dataSize);
  if (!cache) return NULL;
  MYBYTE* dp = &cache->data;
  cache->mapname = (char*)dp;
  strcpy(cache->mapname, lump->mapname);
  dp += strlen(cache->mapname);
  dp++;
  if (lump->optionSize >= 5) {
    cache->options = dp;
    memcpy(cache->options, lump->options, lump->optionSize);
  }
  if (lump->hostname && lump->hostname[0])
    cache->hostname = cloneString(lump->hostname);
  return cache;
}

void loadDHCP() {

  Data* dhcpEntry = NULL;
  FILE* f = NULL;
  FILE* ff = NULL;
  char mapname[64];

  // GLOBALOPTIONS

  if (f = openSection(GLOBALOPTIONS, 1)) {
    Opt optionData;
    loadOptions(f, GLOBALOPTIONS, &optionData);
    cfig.options = (MYBYTE*)calloc(1, optionData.optionSize);
    memcpy(cfig.options, optionData.options, optionData.optionSize);
    cfig.mask = optionData.mask;
  }

  if (!cfig.mask) cfig.mask = inet_addr("255.255.255.0");

  for (MYBYTE i = 1; i <= MAX_RANGE_SETS ; i++) {
    if (f = openSection(RANGESET, i)) {
      MYBYTE m = cfig.rangeCount;
      Opt optionData;
      optionData.rangeSetInd = i - 1;
      loadOptions(f, RANGESET, &optionData);
      MYBYTE* options = NULL;
      cfig.rangeSet[optionData.rangeSetInd].active = true;

      if (optionData.optionSize > 3) {
        options = (MYBYTE*)calloc(1, optionData.optionSize);
        memcpy(options, optionData.options, optionData.optionSize);
      }

      for (; m < MAX_DHCP_RANGES && cfig.dhcpRanges[m].rangeStart; m++) {
        cfig.dhcpRanges[m].rangeSetInd = optionData.rangeSetInd;
        cfig.dhcpRanges[m].options = options;
        cfig.dhcpRanges[m].mask = optionData.mask;
      }
      cfig.rangeCount = m;
    } else break;
  }

  for (char rangeInd = 0; rangeInd < cfig.rangeCount; rangeInd++) {
    if (!cfig.dhcpRanges[rangeInd].mask) cfig.dhcpRanges[rangeInd].mask = cfig.mask;

    for (MYDWORD iip = cfig.dhcpRanges[rangeInd].rangeStart; iip <= cfig.dhcpRanges[rangeInd].rangeEnd; iip++) {
      MYDWORD ip = htonl(iip);

      if ((cfig.dhcpRanges[rangeInd].mask | (~ip)) == UINT_MAX || (cfig.dhcpRanges[rangeInd].mask | ip) == UINT_MAX)
        cfig.dhcpRanges[rangeInd].expiry[iip - cfig.dhcpRanges[rangeInd].rangeStart] = INT_MAX;
    }
  }

  if (f = openSection(GLOBALOPTIONS, 1)) lockOptions(f);

  for (MYBYTE i = 1; i <= MAX_RANGE_SETS ;i++) {
    if (f = openSection(RANGESET, i)) lockOptions(f);
    else break;
  }

  ff = fopen(path.ini, "rt");

  if (ff) {
    char sectionName[512];

    while (fgets(sectionName, 510, ff)) {

      if (*sectionName == '[') {
      	char* secend = strchr(sectionName, ']');
        if (secend) { *secend = 0; sectionName[0] = NBSP; myTrim(sectionName, sectionName); }
        else continue;
      } else continue;

      if (!strchr(sectionName, ':')) continue;

      MYBYTE hexValue[UCHAR_MAX];
      MYBYTE hexValueSize = sizeof(hexValue);
      Opt optionData;

      if (strlen(sectionName) <= 48 && !getHexValue(hexValue, sectionName, &hexValueSize)) {
        if (hexValueSize <= 16) {
          dhcpEntry = findDHCPEntry(hex2String(mapname, hexValue, hexValueSize));

          if (!dhcpEntry) {
            if (f = openSection(sectionName, 1)) loadOptions(f, sectionName, &optionData);
	          if (f = openSection(sectionName, 1)) lockOptions(f);

            dhcpMap::iterator p = dhcpCache.begin();

            for (; p != dhcpCache.end(); p++) {
              if (p->second && p->second->ip && p->second->ip == optionData.ip) break;
            }

            if (p == dhcpCache.end()) {
              memset(&lump, 0, sizeof(lumpData));
              lump.mapname = mapname;
              lump.optionSize = optionData.optionSize;
              lump.options = optionData.options;
              dhcpEntry = createCache(&lump);
              if (!dhcpEntry) return;
              dhcpEntry->ip = optionData.ip;
              dhcpEntry->rangeInd = getRangeInd(optionData.ip);
              dhcpEntry->fixed = 1;
              lockIP(optionData.ip);
              dhcpCache[dhcpEntry->mapname] = dhcpEntry;
       	    } else {
       	      sprintf(lb.log, "Static DHCP Host [%s] Duplicate IP Address %s, Entry ignored", sectionName, IP2String(lb.tmp, optionData.ip));
       	      logMesg(lb.log, LOG_NOTICE);
      	    }
          } else {
            sprintf(lb.log, "Duplicate Static DHCP Host [%s] ignored", sectionName);
            logMesg(lb.log, LOG_NOTICE);
       	  }
        } else {
          sprintf(lb.log, "Invalid Static DHCP Host MAC Addr size, ignored", sectionName);
          logMesg(lb.log, LOG_NOTICE);
        }
      } else {
        sprintf(lb.log, "Invalid Static DHCP Host MAC Addr [%s] ignored", sectionName);
        logMesg(lb.log, LOG_NOTICE);
      }
      if (!optionData.ip) {
        sprintf(lb.log, "Warning: No IP Address for DHCP Static Host %s specified", sectionName);
        logMesg(lb.log, LOG_NOTICE);
      }
    }

    fclose(ff);
  }

  ff = fopen(lb.lea, "rb");

  if (ff) {
    Client dhcpData;

    while (fread(&dhcpData, sizeof(Client), 1, ff)) {
      char rangeInd = -1;
      int ind = -1;

      if (dhcpData.bp_hlen <= 16 && !findServer(net.allServers, MAX_SERVERS, dhcpData.ip)) {
        hex2String(mapname, dhcpData.bp_chaddr, dhcpData.bp_hlen);

        dhcpMap::iterator p = dhcpCache.begin();

        for (; p != dhcpCache.end(); p++) {
          dhcpEntry = p->second;
          if (dhcpEntry && (!strcasecmp(mapname, dhcpEntry->mapname) || dhcpEntry->ip == dhcpData.ip)) break;
        }

        if (p != dhcpCache.end() && (strcasecmp(mapname, dhcpEntry->mapname) || dhcpEntry->ip != dhcpData.ip)) continue;

        dhcpEntry = findDHCPEntry(mapname);
        rangeInd = getRangeInd(dhcpData.ip);

        if (!dhcpEntry && rangeInd >= 0) {
          memset(&lump, 0, sizeof(lumpData));
          lump.mapname = mapname;
          dhcpEntry = createCache(&lump);
        }

        if (dhcpEntry) {
          dhcpCache[dhcpEntry->mapname] = dhcpEntry;
          dhcpEntry->ip = dhcpData.ip;
          dhcpEntry->rangeInd = rangeInd;
          dhcpEntry->expiry = dhcpData.expiry;
          dhcpEntry->local = dhcpData.local;
          dhcpEntry->display = true;

      	  if (dhcpData.hostname[0])
            dhcpEntry->hostname = cloneString(dhcpData.hostname);

      	  setLeaseExpiry(dhcpEntry);
        }
      }
    }

    fclose(ff);

    ff = fopen(lb.lea, "wb");
    cfig.dhcpInd = 0;

    if (ff) {
      dhcpMap::iterator p = dhcpCache.begin();
      for (; p != dhcpCache.end(); p++) {
        if ((dhcpEntry = p->second) && (dhcpEntry->expiry > ld.t || !dhcpEntry->fixed)) {
          memset(&dhcpData, 0, sizeof(Client));
          dhcpData.bp_hlen = 16;
          getHexValue(dhcpData.bp_chaddr, dhcpEntry->mapname, &dhcpData.bp_hlen);
          dhcpData.ip = dhcpEntry->ip;
          dhcpData.expiry = dhcpEntry->expiry;
          dhcpData.local = dhcpEntry->local;
    	    if (dhcpEntry->hostname) strcpy(dhcpData.hostname, dhcpEntry->hostname);
          cfig.dhcpInd++;
          dhcpData.dhcpInd = cfig.dhcpInd;
          dhcpEntry->dhcpInd = cfig.dhcpInd;
          fwrite(&dhcpData, sizeof(Client), 1, ff);
        }
      }
      fclose(ff);
    }
  }
}

void __cdecl init(void* arg) {

  FILE* f = NULL;
  char raw[512], name[512], value[512];
  int i, j, nRet;
  bool bindfailed;

  memset(&cfig, 0, sizeof(cfig));
  memset(&nd, 0, sizeof(nd));

  sprintf(lb.lea, "%s\\" NAME ".state", path.tmp);
  sprintf(lb.cli, "%s\\" NAME "-%%s.log", path.log);

  loadDHCP();

  cfig.lease = 36000;

  for (int i = 0; i < cfig.rangeCount; i++) {
    char* logPtr = lb.log;
    logPtr += sprintf(logPtr, "DHCP range ");
    logPtr += sprintf(logPtr, "%s", IP2String(lb.tmp, htonl(cfig.dhcpRanges[i].rangeStart)));
    logPtr += sprintf(logPtr, "-%s", IP2String(lb.tmp, htonl(cfig.dhcpRanges[i].rangeEnd)));
    logPtr += sprintf(logPtr, "/%s", IP2String(lb.tmp, cfig.dhcpRanges[i].mask));
    logMesg(lb.log, LOG_INFO);
  }

  if (f = openSection("REPLICATION_SERVERS", 1)) {
    while (readSection(raw, f)) {
      mySplit(name, value, raw, '=');
      if (name[0] && value[0]) {
        if (!isIP(name) && isIP(value)) {
	        if (!strcasecmp(name, "Primary")) cfig.zoneServers[0] = inet_addr(value);
	        else
            if (!strcasecmp(name, "Secondary")) cfig.zoneServers[1] = inet_addr(value);
	          else {
	            sprintf(lb.log, "Section [REPLICATION_SERVERS] Invalid Entry: %s ignored", raw);
	            logMesg(lb.log, LOG_NOTICE);
  	        }
        } else {
          sprintf(lb.log, "Section [REPLICATION_SERVERS] Invalid Entry: %s ignored", raw);
          logMesg(lb.log, LOG_NOTICE);
	      }
      } else {
        sprintf(lb.log, "Section [REPLICATION_SERVERS], Missing value, entry %s ignored", raw);
        logMesg(lb.log, LOG_NOTICE);
      }
    }
  }

  if (!cfig.zoneServers[0] && cfig.zoneServers[1]) {
    sprintf(lb.log, "Section [REPLICATION_SERVERS] Missing Primary Server");
    logMesg(lb.log, LOG_NOTICE);
  } else if (cfig.zoneServers[0] && !cfig.zoneServers[1]) {
    sprintf(lb.log, "Section [REPLICATION_SERVERS] Missing Secondary Server");
    logMesg(lb.log, LOG_NOTICE);
  } else if (cfig.zoneServers[0] && cfig.zoneServers[1]) {
    if (findServer(net.listenServers, MAX_SERVERS, cfig.zoneServers[0]) && findServer(net.listenServers, MAX_SERVERS, cfig.zoneServers[1])) {
      logMesg("Section [REPLICATION_SERVERS] Primary & Secondary should be different machines", LOG_NOTICE);
    } else if (findServer(net.listenServers, MAX_SERVERS, cfig.zoneServers[0])) cfig.replication = 1;
    else if (findServer(net.listenServers, MAX_SERVERS, cfig.zoneServers[1])) cfig.replication = 2;
    else logMesg("Section [REPLICATION_SERVERS] Server IP not found on this machine", LOG_NOTICE);
  }

  if (cfig.replication) {

    lockIP(cfig.zoneServers[0]);
    lockIP(cfig.zoneServers[1]);

    cfig.dhcpReplConn.sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (cfig.dhcpReplConn.sock == INVALID_SOCKET)
      logMesg("Failed to Create DHCP Replication Socket", LOG_NOTICE);
    else {
      if (cfig.replication == 1) cfig.dhcpReplConn.server = cfig.zoneServers[0];
      else cfig.dhcpReplConn.server = cfig.zoneServers[1];

      cfig.dhcpReplConn.addr.sin_family = AF_INET;
      cfig.dhcpReplConn.addr.sin_addr.s_addr = cfig.dhcpReplConn.server;
      cfig.dhcpReplConn.addr.sin_port = 0;

      int nRet = bind(cfig.dhcpReplConn.sock, (sockaddr*)&cfig.dhcpReplConn.addr, sizeof(struct sockaddr_in));

      if (nRet == SOCKET_ERROR) {
        cfig.dhcpReplConn.ready = false;
        logMesg("DHCP replication server bind failed", LOG_NOTICE);
      } else {
        cfig.dhcpReplConn.port = IPPORT_DHCPS;
        cfig.dhcpReplConn.loaded = true;
        cfig.dhcpReplConn.ready = true;

        OptCode op;
	      memset(&token, 0, sizeof(Request));
	      token.vp = token.dhcpp.vend_data;
	      token.messsize = sizeof(Packet);

	      token.dhcpp.header.bp_op = BOOTP_REQUEST;
	      token.dhcpp.header.bp_xid = ld.t;
	      token.dhcpp.header.bp_magic_num[0] = 99;
	      token.dhcpp.header.bp_magic_num[1] = 130;
	      token.dhcpp.header.bp_magic_num[2] = 83;
	      token.dhcpp.header.bp_magic_num[3] = 99;

	      op.opt_code = DHCP_OPTION_MESSAGETYPE;
	      op.size = 1;
	      op.value[0] = DHCP_MESS_INFORM;
	      pvdata(&token, &op);

	      token.vp[0] = DHCP_OPTION_END;
	      token.vp++;
	      token.bytes = token.vp - (MYBYTE*)token.raw;
	      token.remote.sin_port = htons(IPPORT_DHCPS);
	      token.remote.sin_family = AF_INET;

	      if (cfig.replication == 1) token.remote.sin_addr.s_addr = cfig.zoneServers[1];
	      else token.remote.sin_addr.s_addr = cfig.zoneServers[0];

	      if (cfig.replication == 2) _beginthread(sendToken, 0, 0);
      }
    }

    if (cfig.replication == 1)
      sprintf(lb.log, "Server name: %s (Primary)", net.hostname);
    else if (cfig.replication == 2)
      sprintf(lb.log, "Server name: %s (Secondary)", net.hostname);
  } else
    sprintf(lb.log, "DHCP standalone mode");

  logMesg(lb.log, LOG_INFO);

  do {

    cleanup(0);

    memset(&nd, 0, sizeof(nd));

    bindfailed = false;

    if (cfig.dhcpReplConn.ready && nd.maxFD < cfig.dhcpReplConn.sock)
      nd.maxFD = cfig.dhcpReplConn.sock;

    for (i=0,j=0; j < MAX_SERVERS && net.listenServers[j]; j++) {

      nd.dhcpConn[i].sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

      if (nd.dhcpConn[i].sock == INVALID_SOCKET) {
        sprintf(lb.log, "DHCP socket error %u", WSAGetLastError());
        logMesg(lb.log, LOG_NOTICE);
        bindfailed = true;
        continue;
      }

      nd.dhcpConn[i].addr.sin_family = AF_INET;
      nd.dhcpConn[i].addr.sin_addr.s_addr = net.listenServers[j];
      nd.dhcpConn[i].addr.sin_port = htons(IPPORT_DHCPS);
      nd.dhcpConn[i].broadCastVal = TRUE;
      nd.dhcpConn[i].broadCastSize = sizeof(nd.dhcpConn[i].broadCastVal);

      setsockopt(nd.dhcpConn[i].sock, SOL_SOCKET, SO_BROADCAST, (char*)(&nd.dhcpConn[i].broadCastVal), nd.dhcpConn[i].broadCastSize);
      nRet = bind(nd.dhcpConn[i].sock, (sockaddr*) &nd.dhcpConn[i].addr, sizeof(struct sockaddr_in));

      if (nRet == SOCKET_ERROR) {
        sprintf(lb.log, "DHCP error binding on %s UDP port 67", IP2String(lb.tmp, net.listenServers[j]));
        logMesg(lb.log, LOG_NOTICE);
        bindfailed = true;
        closesocket(nd.dhcpConn[i].sock);
        continue;
      }

      if (nd.maxFD < nd.dhcpConn[i].sock) nd.maxFD = nd.dhcpConn[i].sock;

      nd.dhcpConn[i].loaded = true;
      nd.dhcpConn[i].ready = true;
      nd.dhcpConn[i].server = net.listenServers[j];
      nd.dhcpConn[i].mask = net.listenMasks[j];
      nd.dhcpConn[i].port = IPPORT_DHCPS;

      i++;
    }

    nd.maxFD++;

    for (MYBYTE m = 0; m < MAX_SERVERS && net.allServers[m]; m++)
      lockIP(net.allServers[m]);

    if (bindfailed) net.failureCounts[DHCP_IDX]++;
    else net.failureCounts[DHCP_IDX] = 0;

    if (!nd.dhcpConn[0].ready) {
      logMesg("DHCP no interface ready, waiting...", LOG_NOTICE);
      continue;
    }

    for (i=0; i < MAX_SERVERS && net.listenServers[i]; i++) {
      for (j=0; j < MAX_SERVERS; j++) {
        if (nd.dhcpConn[j].server == net.listenServers[i]) {
          sprintf(lb.log, "DHCP listening on: %s", IP2String(lb.tmp, net.listenServers[i]));
          logMesg(lb.log, LOG_INFO);
          break;
        }
      }
    }

  } while (dCWait(DHCP_IDX));

  _endthread();
  return;
}

void* main(void* arg) {

  gd.running[DHCP_IDX] = true;

  logMesg("DHCP starting", LOG_INFO);

  _beginthread(init, 0, 0);

  int i;
  fd_set readfds;
  timeval tv = { 20, 0 };

  do {

    net.busy[DHCP_IDX] = false;

    if (!nd.dhcpConn[0].ready) { Sleep(1000); continue; }
    if (!net.ready[DHCP_IDX]) { Sleep(1000); continue; }
    if (net.refresh) { Sleep(1000); continue; }

    FD_ZERO(&readfds);

    for (i=0; i < MAX_SERVERS && nd.dhcpConn[i].ready; i++)
      FD_SET(nd.dhcpConn[i].sock, &readfds);

    if (cfig.dhcpReplConn.ready) FD_SET(cfig.dhcpReplConn.sock, &readfds);

    if (select(nd.maxFD, &readfds, NULL, NULL, &tv) != SOCKET_ERROR) {

      ld.t = time(NULL);

      if (net.ready[DHCP_IDX]) {

        net.busy[DHCP_IDX] = true;

        for (i=0; i < MAX_SERVERS && nd.dhcpConn[i].ready; i++)
          if (FD_ISSET(nd.dhcpConn[i].sock, &readfds) && gdmess(&dhcpr, i) && sdmess(&dhcpr)) alad(&dhcpr);

        if (cfig.dhcpReplConn.ready && FD_ISSET(cfig.dhcpReplConn.sock, &readfds)) {
          errno = 0;
          dhcpr.sockLen = sizeof(dhcpr.remote);
          dhcpr.bytes = recvfrom(cfig.dhcpReplConn.sock, dhcpr.raw, sizeof(dhcpr.raw), 0, (sockaddr*)&dhcpr.remote, &dhcpr.sockLen);
          errno = WSAGetLastError();
          if (errno || dhcpr.bytes <= 0) cfig.dhcpRepl = 0;
        }

      }

    } else ld.t = time(NULL);

  } while (gd.running[DHCP_IDX]);

  cleanup(1);
}

}
#endif
