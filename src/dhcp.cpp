#include "core.h"
#include "dhcp.h"

#if DHCP

bool dhcp_running = false;
bool verbatim = false;

time_t t = time(NULL);

data1 network;
data2 cfig;
data9 dhcpr;
data9 token;
data71 lump;
dhcpMap dhcpCache;
//expiryMap dhcpAge;
char tempbuff[512];
char extbuff[512];
char logBuff[256];
char htmlTitle[256] = "";
char iniFile[_MAX_PATH];
char logFile[_MAX_PATH];
char leaFile[_MAX_PATH];
char htmFile[_MAX_PATH];
char lnkFile[_MAX_PATH];
char cliFile[_MAX_PATH];
char filePATH[_MAX_PATH];
char cfilePATH[_MAX_PATH];
timeval tv;
fd_set readfds;
fd_set writefds;
HANDLE fEvent;
HANDLE lEvent;

char NBSP = 32;
const char arpa[] = ".in-addr.arpa";
char RANGESET[] = "RANGE_SET";
char GLOBALOPTIONS[] = "GLOBAL_OPTIONS";
const char base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char send200[] = "HTTP/1.1 200 OK\r\nDate: %s\r\nLast-Modified: %s\r\nContent-Type: text/html\r\nConnection: Close\r\nContent-Length:         \r\n\r\n";
//const char send200[] = "HTTP/1.1 200 OK\r\nDate: %s\r\nLast-Modified: %s\r\nContent-Type: text/html\r\nConnection: Close\r\nTransfer-Encoding: chunked\r\n";
//const char send403[] = "HTTP/1.1 403 Forbidden\r\nDate: %s\r\nLast-Modified: %s\r\nContent-Type: text/html\r\nConnection: Close\r\n\r\n";
const char send403[] = "HTTP/1.1 403 Forbidden\r\n\r\n<h1>403 Forbidden</h1>";
const char send404[] = "HTTP/1.1 404 Not Found\r\n\r\n<h1>404 Not Found</h1>";
const char td200[] = "<td>%s</td>";
const char sVersion[] = SERVICE_DISPLAY_NAME;
const char htmlStart[] = "<html>\n<head>\n<title>%s</title><meta http-equiv=\"refresh\" content=\"60\">\n<meta http-equiv=\"cache-control\" content=\"no-cache\">\n</head>\n";
//const char bodyStart[] = "<body bgcolor=\"#fff\"><table width=\"800\"><tr><td align=\"center\"><font size=\"5\"><b>%s</b></font></b></b></td></tr><tr><td align=\"right\"><a target=\"_new\" href=\"http://dhcp-dns-server.sourceforge.net/\">http://dhcp-dns-server.sourceforge.net/</b></b></td></tr></table>";
const char bodyStart[] = "<body bgcolor=\"#fff\"><table width=640><tr><td align=\"center\"><font size=\"5\"><b>%s</b></font></td></tr><tr><td align=\"right\"></td></tr></table>";
//const char bodyStart[] = "<body bgcolor=\"#fff\"><table width=640><tr><td align=\"center\"><font size=\"5\"><b>%s</b></font></td></tr><tr><td align=\"center\"><font size=\"5\">%s</font></td></tr></table>";
const data4 opData[] = {
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

void closeConn() {
  if (network.httpConn.ready) closesocket(network.httpConn.sock);
  for (int i = 0; i < MAX_SERVERS && network.dhcpConn[i].loaded; i++)
    if (network.dhcpConn[i].ready) closesocket(network.dhcpConn[i].sock);
}

int dhcp_cleanup(int exitthread) {
  sprintf(logBuff, "Closing Network Connections...");
  logDHCPMess(logBuff, 1);
  closeConn();
  if (cfig.replication && cfig.dhcpReplConn.ready) closesocket(cfig.dhcpReplConn.sock);
  dhcp_running = false;
  WSACleanup();
  Sleep(1000);
  sprintf(logBuff, "DHCP stopped");
  logDHCPMess(logBuff, 1);
  if (exitthread) { 
    CloseHandle(fEvent);
    CloseHandle(lEvent);
    pthread_exit(NULL);
  }
  else return 0;
}

void* dhcp(void *arg) {
  //printf("%i\n",t);
  //printf("%i\n",sizeof(data7));
  //printf("%d\n",dnsCache[currentInd].max_size());

  dhcp_running = true;
  verbatim = true;

  if (_beginthread(init, 0, NULL) == 0) {
    sprintf(logBuff, "Thread Creation Failed");
    logDHCPMess(logBuff, 1);
    dhcp_cleanup(1);
  }

  tv.tv_sec = 20;
  tv.tv_usec = 0;

  do {

    network.busy = false;

    if (!network.dhcpConn[0].ready) { Sleep(1000); continue; }

    if (!network.ready) { Sleep(1000); continue; }

    FD_ZERO(&readfds);

    if (cfig.dhcpReplConn.ready)
      FD_SET(cfig.dhcpReplConn.sock, &readfds);

    if (network.httpConn.ready)
      FD_SET(network.httpConn.sock, &readfds);

    for (int i = 0; i < MAX_SERVERS && network.dhcpConn[i].ready; i++)
      FD_SET(network.dhcpConn[i].sock, &readfds);

    if (select(network.maxFD, &readfds, NULL, NULL, &tv)) {

      t = time(NULL);

      if (network.ready) {

        network.busy = true;

        if (network.httpConn.ready && FD_ISSET(network.httpConn.sock, &readfds)) {
          data19 *req = (data19*)calloc(1, sizeof(data19));

          if (req) {

            req->sockLen = sizeof(req->remote);
            req->sock = accept(network.httpConn.sock, (sockaddr*)&req->remote, &req->sockLen);

            if (req->sock == INVALID_SOCKET) {
              sprintf(logBuff, "Accept Failed, Error=%u\n", WSAGetLastError());
              logDHCPMess(logBuff, 1);
              free(req);
            } else procHTTP(req);

          } else {
            sprintf(logBuff, "Memory Error");
            logDHCPMess(logBuff, 1);
          }

        }

        for (int i = 0; i < MAX_SERVERS && network.dhcpConn[i].ready; i++) {
          if (FD_ISSET(network.dhcpConn[i].sock, &readfds) && gdmess(&dhcpr, i) && sdmess(&dhcpr)) alad(&dhcpr);
        }

        if (cfig.dhcpReplConn.ready && FD_ISSET(cfig.dhcpReplConn.sock, &readfds)) {
          errno = 0;
          dhcpr.sockLen = sizeof(dhcpr.remote);
          dhcpr.bytes = recvfrom(cfig.dhcpReplConn.sock, dhcpr.raw, sizeof(dhcpr.raw), 0, (sockaddr*)&dhcpr.remote, &dhcpr.sockLen);
          errno = WSAGetLastError();
          if (errno || dhcpr.bytes <= 0) cfig.dhcpRepl = 0;
        }

      }

    } else t = time(NULL);
    //checkSize();
    //printf("CacheInd=%d\n");
  } while (dhcp_running);

  dhcp_cleanup(1);
}

MYWORD fUShort(void *raw) {
  return ntohs(*((MYWORD*)raw));
}

MYDWORD fULong(void *raw) {
  return ntohl(*((MYDWORD*)raw));
}

MYDWORD fIP(void *raw) {
  return(*((MYDWORD*)raw));
}

MYBYTE pUShort(void *raw, MYWORD data) {
  *((MYWORD*)raw) = htons(data);
  return sizeof(MYWORD);
}

MYBYTE pULong(void *raw, MYDWORD data) {
  *((MYDWORD*)raw) = htonl(data);
  return sizeof(MYDWORD);
}

MYBYTE pIP(void *raw, MYDWORD data) {
  *((MYDWORD*)raw) = data;
  return sizeof(MYDWORD);
}

void procHTTP(data19 *req) {
  //debug("procHTTP");

  req->ling.l_onoff = 1; //0 = off (l_linger ignored), nonzero = on
  req->ling.l_linger = 30; //0 = discard data, nonzero = wait for data sent
  setsockopt(req->sock, SOL_SOCKET, SO_LINGER, (const char*)&req->ling, sizeof(req->ling));

  timeval tv1;
  fd_set readfds1;
  FD_ZERO(&readfds1);
  tv1.tv_sec = 1;
  tv1.tv_usec = 0;
  FD_SET(req->sock, &readfds1);

  if (!select((req->sock + 1), &readfds1, NULL, NULL, &tv1)) {
    sprintf(logBuff, "Client %s, HTTP Message Receive failed", IP2String(tempbuff, req->remote.sin_addr.s_addr));
    logDHCPMess(logBuff, 1);
    closesocket(req->sock);
    free(req);
    return;
  }

  errno = 0;
  char buffer[1024];
  req->bytes = recv(req->sock, buffer, sizeof(buffer), 0);
  errno = WSAGetLastError();

  if (errno || req->bytes <= 0) {
    sprintf(logBuff, "Client %s, HTTP Message Receive failed, WSAError %d", IP2String(tempbuff, req->remote.sin_addr.s_addr), errno);
    logDHCPMess(logBuff, 1);
    closesocket(req->sock);
    free(req);
    return;
  }
  else if (verbatim || cfig.dhcpLogLevel >= 2) {
    sprintf(logBuff, "Client %s, HTTP Request Received", IP2String(tempbuff, req->remote.sin_addr.s_addr));
    logDHCPMess(logBuff, 2);
    //printf("%s\n", buffer);
  }

  if (cfig.httpClients[0] && !findServer(cfig.httpClients, 8, req->remote.sin_addr.s_addr)) {
    if (verbatim || cfig.dhcpLogLevel >= 2) {
      sprintf(logBuff, "Client %s, HTTP Access Denied", IP2String(tempbuff, req->remote.sin_addr.s_addr));
      logDHCPMess(logBuff, 2);
    }

    req->dp = (char*)calloc(1, sizeof(send403));
    req->memSize = sizeof(send403);
    req->bytes = sprintf(req->dp, send403);
    _beginthread(sendHTTP, 0, (void*)req);
    return;
  }

  buffer[sizeof(buffer) - 1] = 0;
  char *fp = NULL;
  char *end = strchr(buffer, '\n');

  if (end && end > buffer && (*(end - 1) == '\r')) {
    *(end - 1) = 0;

    if (myTokenize(buffer, buffer, " ", true) > 1)
      fp = myGetToken(buffer, 1);
  }

  if (fp && !strcasecmp(fp, "/")) sendStatus(req);
//	else if (fp && !strcasecmp(fp, "/scopestatus"))
//		sendScopeStatus(req);
  else {
    if (fp && (verbatim || cfig.dhcpLogLevel >= 2)) {
      sprintf(logBuff, "Client %s, %s not found", IP2String(tempbuff, req->remote.sin_addr.s_addr), fp);
      logDHCPMess(logBuff, 2);
    } else if (verbatim || cfig.dhcpLogLevel >= 2) {
      sprintf(logBuff, "Client %s, Invalid http request", IP2String(tempbuff, req->remote.sin_addr.s_addr));
      logDHCPMess(logBuff, 2);
    }

    req->dp = (char*)calloc(1, sizeof(send404));
    req->bytes = sprintf(req->dp, send404);
    req->memSize = sizeof(send404);
    _beginthread(sendHTTP, 0, (void*)req);
    return;
  }
}

void sendStatus(data19 *req) {

  //debug("sendStatus");

  dhcpMap::iterator p;
  MYDWORD iip = 0;
  data7 *dhcpEntry = NULL;
  //data7 *cache = NULL;
  //printf("%d=%d\n", dhcpCache.size(), cfig.dhcpSize);
  req->memSize = 2048 + (135 * dhcpCache.size()) + (cfig.dhcpSize * 26);
  req->dp = (char*)calloc(1, req->memSize);

  if (!req->dp) {
    sprintf(logBuff, "Memory Error");
    logDHCPMess(logBuff, 1);
    closesocket(req->sock);
    free(req);
    return;
  }

  char *fp = req->dp;
  char *maxData = req->dp + (req->memSize - 512);
  tm *ttm = gmtime(&t);
  strftime(tempbuff, sizeof(tempbuff), "%a, %d %b %Y %H:%M:%S GMT", ttm);
  fp += sprintf(fp, send200, tempbuff, tempbuff);
  char *contentStart = fp;
  fp += sprintf(fp, htmlStart, htmlTitle);
  fp += sprintf(fp, bodyStart, sVersion);
  fp += sprintf(fp, "<table border=\"1\" cellpadding=\"1\" width=\"640\" bgcolor=\"#b8b8b8\">\n");

  if (cfig.dhcpRepl > t) {
    fp += sprintf(fp, "<tr><th colspan=\"5\"><font size=\"5\"><i>Active Leases</i></font></th></tr>\n");
    fp += sprintf(fp, "<tr><th>Mac Address</th><th>IP</th><th>Lease Expiry</th><th>Hostname (first 20 chars)</th><th>Server</th></tr>\n");
  } else {
    fp += sprintf(fp, "<tr><th colspan=\"4\"><font size=\"5\"><i>Active Leases</i></font></th></tr>\n");
    fp += sprintf(fp, "<tr><th>Mac Address</th><th>IP</th><th>Lease Expiry</th><th>Hostname (first 20 chars)</th></tr>\n");
  }

  for (p = dhcpCache.begin(); dhcp_running && p != dhcpCache.end() && fp < maxData; p++) {
    if ((dhcpEntry = p->second) && dhcpEntry->display && dhcpEntry->expiry >= t) {
      fp += sprintf(fp, "<tr>");
      fp += sprintf(fp, td200, dhcpEntry->mapname);
      fp += sprintf(fp, td200, IP2String(tempbuff, dhcpEntry->ip));

      if (dhcpEntry->expiry >= INT_MAX)
	fp += sprintf(fp, td200, "Infinity");
      else {
        tm *ttm = localtime(&dhcpEntry->expiry);
        strftime(tempbuff, sizeof(tempbuff), "%d-%b-%y %X", ttm);
        fp += sprintf(fp, td200, tempbuff);
      }

      if (dhcpEntry->hostname) {
        if (strlen(dhcpEntry->hostname) <= 20)
  	  fp += sprintf(fp, td200, dhcpEntry->hostname);
	else {
	  strncpy(tempbuff, dhcpEntry->hostname, 20);
	  tempbuff[20] = 0;
	  fp += sprintf(fp, td200, tempbuff);
	}
      } else
	fp += sprintf(fp, td200, "&nbsp;");

      if (cfig.dhcpRepl > t) {

        if (dhcpEntry->local && cfig.replication == 1)
	  fp += sprintf(fp, td200, "Primary");
	else if (dhcpEntry->local && cfig.replication == 2)
	  fp += sprintf(fp, td200, "Secondary");
	else if (cfig.replication == 1)
	  fp += sprintf(fp, td200, "Secondary");
	else
	  fp += sprintf(fp, td200, "Primary");
      }

      fp += sprintf(fp, "</tr>\n");
    }
  }

/*
	fp += sprintf(fp, "</table>\n<br>\n<table border=\"1\" width=\"640\" cellpadding=\"1\" bgcolor=\"#b8b8b8\">\n");
	fp += sprintf(fp, "<tr><th colspan=\"5\"><font size=\"5\"><i>Free Dynamic Leases</i></font></th></tr>\n");
	MYBYTE colNum = 0;

	for (char rangeInd = 0; dhcp_running && rangeInd < cfig.rangeCount && fp < maxData; rangeInd++)
	{
		for (MYDWORD ind = 0, iip = cfig.dhcpRanges[rangeInd].rangeStart; dhcp_running && iip <= cfig.dhcpRanges[rangeInd].rangeEnd; iip++, ind++)
		{
			if (cfig.dhcpRanges[rangeInd].expiry[ind] < t)
			{
				if (!colNum)
				{
					fp += sprintf(fp, "<tr>");
					colNum = 1;
				}
				else if (colNum < 5)
					colNum++;
				else
				{
					fp += sprintf(fp, "</tr>\n<tr>");
					colNum = 1;
				}

				fp += sprintf(fp, td200, IP2String(tempbuff, htonl(iip)));
			}
		}
	}

	if (colNum)
		fp += sprintf(fp, "</tr>\n");
*/
  fp += sprintf(fp, "</table>\n<br>\n<table border=\"1\" cellpadding=\"1\" width=\"640\" bgcolor=\"#b8b8b8\">\n");
  fp += sprintf(fp, "<tr><th colspan=\"4\"><font size=\"5\"><i>Free Dynamic Leases</i></font></th></tr>\n");
  fp += sprintf(fp, "<tr><td><b>DHCP Range</b></td><td align=\"right\"><b>Available Leases</b></td><td align=\"right\"><b>Free Leases</b></td></tr>\n");

  for (char rangeInd = 0; dhcp_running && rangeInd < cfig.rangeCount && fp < maxData; rangeInd++) {
    float ipused = 0;
    float ipfree = 0;
    int ind = 0;

    for (MYDWORD iip = cfig.dhcpRanges[rangeInd].rangeStart; iip <= cfig.dhcpRanges[rangeInd].rangeEnd; iip++, ind++) {
      if (cfig.dhcpRanges[rangeInd].expiry[ind] < t) ipfree++;
      else if (cfig.dhcpRanges[rangeInd].dhcpEntry[ind] && !(cfig.dhcpRanges[rangeInd].dhcpEntry[ind]->fixed)) ipused++;
    }

    IP2String(tempbuff, ntohl(cfig.dhcpRanges[rangeInd].rangeStart));
    IP2String(extbuff, ntohl(cfig.dhcpRanges[rangeInd].rangeEnd));
    fp += sprintf(fp, "<tr><td>%s - %s</td><td align=\"right\">%5.0f</td><td align=\"right\">%5.0f</td></tr>\n", tempbuff, extbuff, (ipused + ipfree), ipfree);
  }

	fp += sprintf(fp, "</table>\n<br>\n<table border=\"1\" width=\"640\" cellpadding=\"1\" bgcolor=\"#b8b8b8\">\n");
	fp += sprintf(fp, "<tr><th colspan=\"4\"><font size=\"5\"><i>Free Static Leases</i></font></th></tr>\n");
	fp += sprintf(fp, "<tr><th>Mac Address</th><th>IP</th><th>Mac Address</th><th>IP</th></tr>\n");
	MYBYTE colNum = 0;

	for (p = dhcpCache.begin(); dhcp_running && p != dhcpCache.end() && fp < maxData; p++)
	{
		if ((dhcpEntry = p->second) && dhcpEntry->fixed && dhcpEntry->expiry < t)
		{
			if (!colNum)
			{
				fp += sprintf(fp, "<tr>");
				colNum = 1;
			}
			else if (colNum == 1)
			{
				colNum = 2;
			}
			else if (colNum == 2)
			{
				fp += sprintf(fp, "</tr>\n<tr>");
				colNum = 1;
			}

			fp += sprintf(fp, td200, dhcpEntry->mapname);
			fp += sprintf(fp, td200, IP2String(tempbuff, dhcpEntry->ip));
		}
	}

	if (colNum)
		fp += sprintf(fp, "</tr>\n");

	fp += sprintf(fp, "</table>\n</body>\n</html>");
	MYBYTE x = sprintf(tempbuff, "%u", (fp - contentStart));
	memcpy((contentStart - 12), tempbuff, x);
	req->bytes = fp - req->dp;

	_beginthread(sendHTTP, 0, (void*)req);
	return;
}

void __cdecl sendHTTP(void *lpParam)
{
	data19 *req = (data19*)lpParam;

	//sprintf(logBuff, "sendHTTP memsize=%d bytes=%d", req->memSize, req->bytes);
	//debug(logBuff);

	char *dp = req->dp;
	timeval tv1;
	fd_set writefds1;
	int sent = 0;

	while (dhcp_running && req->bytes > 0)
	{
		tv1.tv_sec = 5;
		tv1.tv_usec = 0;
		FD_ZERO(&writefds1);
		FD_SET(req->sock, &writefds1);

		if (select((req->sock + 1), NULL, &writefds1, NULL, &tv1))
		{
			if (req->bytes > 1024)
				sent  = send(req->sock, dp, 1024, 0);
			else
				sent  = send(req->sock, dp, req->bytes, 0);

			errno = WSAGetLastError();

			if (errno || sent < 0)
				break;

			dp += sent;
			req->bytes -= sent;
		}
		else
			break;
	}

	closesocket(req->sock);
	free(req->dp);
	free(req);
	_endthread();
	return;
}

bool checkRange(data17 *rangeData, char rangeInd)
{
	//debug("checkRange");

	if (!cfig.hasFilter)
		return true;

	MYBYTE rangeSetInd = cfig.dhcpRanges[rangeInd].rangeSetInd;
	data14 *rangeSet = &cfig.rangeSet[rangeSetInd];
	//printf("checkRange entering, rangeInd=%i rangeSetInd=%i\n", rangeInd, rangeSetInd);
	//printf("checkRange entered, macFound=%i vendFound=%i userFound=%i\n", macFound, vendFound, userFound);

	if((!rangeData->macFound && !rangeSet->macSize[0]) || (rangeData->macFound && rangeData->macArray[rangeSetInd]))
		if((!rangeData->vendFound && !rangeSet->vendClassSize[0]) || (rangeData->vendFound && rangeData->vendArray[rangeSetInd]))
			if((!rangeData->userFound && !rangeSet->userClassSize[0]) || (rangeData->userFound && rangeData->userArray[rangeSetInd]))
				if((!rangeData->subnetFound && !rangeSet->subnetIP[0]) || (rangeData->subnetFound && rangeData->subnetArray[rangeSetInd]))
					return true;

	//printf("checkRange, returning false rangeInd=%i rangeSetInd=%i\n", rangeInd, rangeSetInd);
	return false;
}

MYDWORD resad(data9 *req)
{
	//debug("resad");

	MYDWORD minRange = 0;
	MYDWORD maxRange = 0;

	if (req->dhcpp.header.bp_giaddr)
	{
		lockIP(req->dhcpp.header.bp_giaddr);
		lockIP(req->remote.sin_addr.s_addr);
	}

	req->dhcpEntry = findDHCPEntry(req->chaddr);

	if (req->dhcpEntry && req->dhcpEntry->fixed)
	{
		if (req->dhcpEntry->ip)
		{
			setTempLease(req->dhcpEntry);
			return req->dhcpEntry->ip;
		}
		else
		{
			if (verbatim || cfig.dhcpLogLevel)
			{
				sprintf(logBuff, "Static DHCP Host %s (%s) has No IP, DHCPDISCOVER ignored", req->chaddr, req->hostname);
				logDHCPMess(logBuff, 1);
			}
			return 0;
		}
	}

	MYDWORD iipNew = 0;
	MYDWORD iipExp = 0;
	MYDWORD rangeStart = 0;
	MYDWORD rangeEnd = 0;
	char rangeInd = -1;
	bool rangeFound = false;
	data17 rangeData;
	memset(&rangeData, 0, sizeof(data17));

	if (cfig.hasFilter)
	{
		for (MYBYTE rangeSetInd = 0; rangeSetInd < MAX_RANGE_SETS && cfig.rangeSet[rangeSetInd].active; rangeSetInd++)
		{
			data14 *rangeSet = &cfig.rangeSet[rangeSetInd];

			for (MYBYTE i = 0; i < MAX_RANGE_FILTERS && rangeSet->macSize[i]; i++)
			{
				//printf("%s\n", hex2String(tempbuff, rangeSet->macStart[i], rangeSet->macSize[i]));
				//printf("%s\n", hex2String(tempbuff, rangeSet->macEnd[i], rangeSet->macSize[i]));

				if(memcmp(req->dhcpp.header.bp_chaddr, rangeSet->macStart[i], rangeSet->macSize[i]) >= 0 && memcmp(req->dhcpp.header.bp_chaddr, rangeSet->macEnd[i], rangeSet->macSize[i]) <= 0)
				{
					rangeData.macArray[rangeSetInd] = 1;
					rangeData.macFound = true;
					//printf("mac Found, rangeSetInd=%i\n", rangeSetInd);
					break;
				}
			}

			for (MYBYTE i = 0; i < MAX_RANGE_FILTERS && req->vendClass.size && rangeSet->vendClassSize[i]; i++)
			{
				if(rangeSet->vendClassSize[i] == req->vendClass.size && !memcmp(req->vendClass.value, rangeSet->vendClass[i], rangeSet->vendClassSize[i]))
				{
					rangeData.vendArray[rangeSetInd] = 1;
					rangeData.vendFound = true;
					//printf("vend Found, rangeSetInd=%i\n", rangeSetInd);
					break;
				}
			}

			for (MYBYTE i = 0; i < MAX_RANGE_FILTERS && req->userClass.size && rangeSet->userClassSize[i]; i++)
			{
				if(rangeSet->userClassSize[i] == req->userClass.size && !memcmp(req->userClass.value, rangeSet->userClass[i], rangeSet->userClassSize[i]))
				{
					rangeData.userArray[rangeSetInd] = 1;
					rangeData.userFound = true;
					//printf("user Found, rangeSetInd=%i\n", rangeSetInd);
					break;
				}
			}

			for (MYBYTE i = 0; i < MAX_RANGE_FILTERS && req->subnetIP && rangeSet->subnetIP[i]; i++)
			{
				if(req->subnetIP == rangeSet->subnetIP[i])
				{
					rangeData.subnetArray[rangeSetInd] = 1;
					rangeData.subnetFound = true;
					//printf("subnet Found, rangeSetInd=%i\n", rangeSetInd);
					break;
				}
			}
		}

	}

//	printArray("macArray", (char*)cfig.macArray);
//	printArray("vendArray", (char*)cfig.vendArray);
//	printArray("userArray", (char*)cfig.userArray);

	if (req->dhcpEntry)
	{
		req->dhcpEntry->rangeInd = getRangeInd(req->dhcpEntry->ip);

		if (req->dhcpEntry->rangeInd >= 0)
		{
			int ind = getIndex(req->dhcpEntry->rangeInd, req->dhcpEntry->ip);

			if (cfig.dhcpRanges[req->dhcpEntry->rangeInd].dhcpEntry[ind] == req->dhcpEntry && checkRange(&rangeData, req->dhcpEntry->rangeInd))
			{
				MYBYTE rangeSetInd = cfig.dhcpRanges[req->dhcpEntry->rangeInd].rangeSetInd;

				if (!cfig.rangeSet[rangeSetInd].subnetIP[0])
				{
					MYDWORD mask = cfig.dhcpRanges[req->dhcpEntry->rangeInd].mask;
					calcRangeLimits(req->subnetIP, mask, &minRange, &maxRange);

					if (htonl(req->dhcpEntry->ip) >= minRange && htonl(req->dhcpEntry->ip) <= maxRange)
					{
						setTempLease(req->dhcpEntry);
						return req->dhcpEntry->ip;
					}
				}
				else
				{
					setTempLease(req->dhcpEntry);
					return req->dhcpEntry->ip;
				}
			}
		}
	}

	if (!iipNew && req->reqIP)
	{
		char k = getRangeInd(req->reqIP);

		if (k >= 0)
		{
			if (checkRange(&rangeData, k))
			{
				data13 *range = &cfig.dhcpRanges[k];
				int ind = getIndex(k, req->reqIP);

				if (range->expiry[ind] <= t)
				{
					if (!cfig.rangeSet[range->rangeSetInd].subnetIP[0])
					{
						calcRangeLimits(req->subnetIP, range->mask, &minRange, &maxRange);
						MYDWORD iip = htonl(req->reqIP);

						if (iip >= minRange && iip <= maxRange)
						{
							iipNew = iip;
							rangeInd = k;
						}
					}
					else
					{
						MYDWORD iip = htonl(req->reqIP);
						iipNew = iip;
						rangeInd = k;
					}
				}
			}
		}
	}

	for (char k = 0; !iipNew && k < cfig.rangeCount; k++)
	{
		if (checkRange(&rangeData, k))
		{
			data13 *range = &cfig.dhcpRanges[k];
			rangeStart = range->rangeStart;
			rangeEnd = range->rangeEnd;

			if (!cfig.rangeSet[range->rangeSetInd].subnetIP[0])
			{
				calcRangeLimits(req->subnetIP, range->mask, &minRange, &maxRange);

				if (rangeStart < minRange)
					rangeStart = minRange;

				if (rangeEnd > maxRange)
					rangeEnd = maxRange;
			}

			if (rangeStart <= rangeEnd)
			{
				rangeFound = true;

				if (cfig.replication == 2)
				{
					for (MYDWORD m = rangeEnd; m >= rangeStart; m--)
					{
						int ind = m - range->rangeStart;

						if (!range->expiry[ind])
						{
							iipNew = m;
							rangeInd = k;
							break;
						}
						else if (!iipExp && range->expiry[ind] < t)
						{
							iipExp = m;
							rangeInd = k;
						}
					}
				}
				else
				{
					for (MYDWORD m = rangeStart; m <= rangeEnd; m++)
					{
						int ind = m - range->rangeStart;

						if (!range->expiry[ind])
						{
							iipNew = m;
							rangeInd = k;
							break;
						}
						else if (!iipExp && range->expiry[ind] < t)
						{
							iipExp = m;
							rangeInd = k;
						}
					}
				}
			}
		}
	}


	if (!iipNew && iipExp)
			iipNew = iipExp;

	if (iipNew)
	{
		if (!req->dhcpEntry)
		{
			memset(&lump, 0, sizeof(data71));
			lump.mapname = req->chaddr;
			lump.hostname = req->hostname;
			req->dhcpEntry = createCache(&lump);

			if (!req->dhcpEntry)
				return 0;

/*
			req->dhcpEntry = (data7*)calloc(1, sizeof(data7));

			if (!req->dhcpEntry)
			{
				sprintf(logBuff, "Memory Allocation Error");
				logDHCPMess(logBuff, 1);
				return 0;
			}

			req->dhcpEntry->mapname = cloneString(req->chaddr);

			if (!req->dhcpEntry->mapname)
			{
				sprintf(logBuff, "Memory Allocation Error");
				logDHCPMess(logBuff, 1);
				return 0;
			}
*/

			dhcpCache[req->dhcpEntry->mapname] = req->dhcpEntry;
		}

		req->dhcpEntry->ip = htonl(iipNew);
		req->dhcpEntry->rangeInd = rangeInd;
		setTempLease(req->dhcpEntry);
		return req->dhcpEntry->ip;
	}

	if (verbatim || cfig.dhcpLogLevel)
	{
		if (rangeFound)
		{
			if (req->dhcpp.header.bp_giaddr)
				sprintf(logBuff, "No free leases for DHCPDISCOVER for %s (%s) from RelayAgent %s", req->chaddr, req->hostname, IP2String(tempbuff, req->dhcpp.header.bp_giaddr));
			else
				sprintf(logBuff, "No free leases for DHCPDISCOVER for %s (%s) from interface %s", req->chaddr, req->hostname, IP2String(tempbuff, network.dhcpConn[req->sockInd].server));
		}
		else
		{
			if (req->dhcpp.header.bp_giaddr)
				sprintf(logBuff, "No Matching DHCP Range for DHCPDISCOVER for %s (%s) from RelayAgent %s", req->chaddr, req->hostname, IP2String(tempbuff, req->dhcpp.header.bp_giaddr));
			else
				sprintf(logBuff, "No Matching DHCP Range for DHCPDISCOVER for %s (%s) from interface %s", req->chaddr, req->hostname, IP2String(tempbuff, network.dhcpConn[req->sockInd].server));
		}
		logDHCPMess(logBuff, 1);
	}
	return 0;
}

MYDWORD chad(data9 *req)
{
	req->dhcpEntry = findDHCPEntry(req->chaddr);
	//printf("dhcpEntry=%d\n", req->dhcpEntry);

	if (req->dhcpEntry && req->dhcpEntry->ip)
		return req->dhcpEntry->ip;
	else
		return 0;
}

MYDWORD sdmess(data9 *req)
{
	//sprintf(logBuff, "sdmess, Request Type = %u",req->req_type);
	//debugl(logBuff);

	if (req->req_type == DHCP_MESS_NONE)
	{
		req->dhcpp.header.bp_yiaddr = chad(req);

		if (!req->dhcpp.header.bp_yiaddr)
		{
			if (verbatim || cfig.dhcpLogLevel)
			{
				sprintf(logBuff, "No Static Entry found for BOOTPREQUEST from Host %s", req->chaddr);
				logDHCPMess(logBuff, 1);
			}

			return 0;
		}
	}
	else if (req->req_type == DHCP_MESS_DECLINE)
	{
		if (req->dhcpp.header.bp_ciaddr && chad(req) == req->dhcpp.header.bp_ciaddr)
		{
			lockIP(req->dhcpp.header.bp_ciaddr);

			req->dhcpEntry->ip = 0;
			req->dhcpEntry->expiry = INT_MAX;
			req->dhcpEntry->display = false;
			req->dhcpEntry->local = false;

			if (verbatim || cfig.dhcpLogLevel)
			{
				sprintf(logBuff, "IP Address %s declined by Host %s (%s), locked", IP2String(tempbuff, req->dhcpp.header.bp_ciaddr), req->chaddr, req->hostname);
				logDHCPMess(logBuff, 1);
			}
		}

		return 0;
	}
	else if (req->req_type == DHCP_MESS_RELEASE)
	{
		if (req->dhcpp.header.bp_ciaddr && chad(req) == req->dhcpp.header.bp_ciaddr)
		{
			req->dhcpEntry->display = false;
			req->dhcpEntry->local = false;
			setLeaseExpiry(req->dhcpEntry, 0);
			_beginthread(updateStateFile, 0, (void*)req->dhcpEntry);

			if (verbatim || cfig.dhcpLogLevel)
			{
				sprintf(logBuff, "IP Address %s released by Host %s (%s)", IP2String(tempbuff, req->dhcpp.header.bp_ciaddr), req->chaddr, req->hostname);
				logDHCPMess(logBuff, 1);
			}
		}

		return 0;
	}
	else if (req->req_type == DHCP_MESS_INFORM)
	{
		//printf("repl0=%s\n", IP2String(tempbuff, cfig.zoneServers[0]));
		//printf("repl1=%s\n", IP2String(tempbuff, cfig.zoneServers[1]));
		//printf("IP=%s bytes=%u replication=%i\n", IP2String(tempbuff, req->remote.sin_addr.s_addr), req->bytes, cfig.replication);

		if ((cfig.replication == 1 && req->remote.sin_addr.s_addr == cfig.zoneServers[1]) || (cfig.replication == 2 && req->remote.sin_addr.s_addr == cfig.zoneServers[0]))
			recvRepl(req);

		return 0;
	}
	else if (req->req_type == DHCP_MESS_DISCOVER && strcasecmp(req->hostname, cfig.servername))
	{
		req->dhcpp.header.bp_yiaddr = resad(req);

		if (!req->dhcpp.header.bp_yiaddr)
			return 0;

		req->resp_type = DHCP_MESS_OFFER;
	}
	else if (req->req_type == DHCP_MESS_REQUEST)
	{
		//printf("DHCP_MESS_REQUEST: %s\n", IP2String(tempbuff, req->dhcpp.header.bp_ciaddr));

		if (req->server)
		{
			if (req->server == network.dhcpConn[req->sockInd].server)
			{
				if (req->reqIP && req->reqIP == chad(req) && req->dhcpEntry->expiry > t)
				{
					req->resp_type = DHCP_MESS_ACK;
					req->dhcpp.header.bp_yiaddr = req->reqIP;
				}
				else if (req->dhcpp.header.bp_ciaddr && req->dhcpp.header.bp_ciaddr == chad(req) && req->dhcpEntry->expiry > t)
				{
					req->resp_type = DHCP_MESS_ACK;
					req->dhcpp.header.bp_yiaddr = req->dhcpp.header.bp_ciaddr;
				}
				else
				{
					req->resp_type = DHCP_MESS_NAK;
					req->dhcpp.header.bp_yiaddr = 0;

					if (verbatim || cfig.dhcpLogLevel)
					{
						sprintf(logBuff, "DHCPREQUEST from Host %s (%s) without Discover, NAKed", req->chaddr, req->hostname);
						logDHCPMess(logBuff, 1);
					}
				}
			}
			else
				return 0;
		}
		else if (req->dhcpp.header.bp_ciaddr && req->dhcpp.header.bp_ciaddr == chad(req) && req->dhcpEntry->expiry > t)
		{
			req->resp_type = DHCP_MESS_ACK;
			req->dhcpp.header.bp_yiaddr = req->dhcpp.header.bp_ciaddr;
		}
		else if (req->reqIP && req->reqIP == chad(req) && req->dhcpEntry->expiry > t)
		{
			req->resp_type = DHCP_MESS_ACK;
			req->dhcpp.header.bp_yiaddr = req->reqIP;
		}
		else
		{
			req->resp_type = DHCP_MESS_NAK;
			req->dhcpp.header.bp_yiaddr = 0;

			if (verbatim || cfig.dhcpLogLevel)
			{
				sprintf(logBuff, "DHCPREQUEST from Host %s (%s) without Discover, NAKed", req->chaddr, req->hostname);
				logDHCPMess(logBuff, 1);
			}
		}
	}
	else
		return 0;

	addOptions(req);
	int packSize = req->vp - (MYBYTE*)&req->dhcpp;
	packSize++;

	if (req->req_type == DHCP_MESS_NONE)
		packSize = req->messsize;

	if ((req->dhcpp.header.bp_giaddr || !req->remote.sin_addr.s_addr) && req->dhcpEntry && req->dhcpEntry->rangeInd >= 0)
	{
		MYBYTE rangeSetInd = cfig.dhcpRanges[req->dhcpEntry->rangeInd].rangeSetInd;
		req->targetIP = cfig.rangeSet[rangeSetInd].targetIP;
	}

	if (req->targetIP)
	{
		req->remote.sin_port = htons(IPPORT_DHCPS);
		req->remote.sin_addr.s_addr = req->targetIP;
	}
	else if (req->dhcpp.header.bp_giaddr)
	{
		req->remote.sin_port = htons(IPPORT_DHCPS);
		req->remote.sin_addr.s_addr = req->dhcpp.header.bp_giaddr;
	}
	else if (req->dhcpp.header.bp_broadcast || !req->remote.sin_addr.s_addr || req->reqIP)
	{
		req->remote.sin_port = htons(IPPORT_DHCPC);
		req->remote.sin_addr.s_addr = INADDR_BROADCAST;
	}
	else
	{
		req->remote.sin_port = htons(IPPORT_DHCPC);
	}

	req->dhcpp.header.bp_op = BOOTP_REPLY;
	errno = 0;

	if (req->req_type == DHCP_MESS_DISCOVER && !req->dhcpp.header.bp_giaddr)
	{
		req->bytes = sendto(network.dhcpConn[req->sockInd].sock,
							req->raw,
							packSize,
							MSG_DONTROUTE,
							(sockaddr*)&req->remote,
							sizeof(req->remote));
	}
	else
	{
		req->bytes = sendto(network.dhcpConn[req->sockInd].sock,
							req->raw,
							packSize,
							0,
							(sockaddr*)&req->remote,
							sizeof(req->remote));
	}

	if (errno || req->bytes <= 0)
		return 0;

	//printf("goes=%s %i\n",IP2String(tempbuff, req->dhcpp.header.bp_yiaddr),req->sockInd);
	return req->dhcpp.header.bp_yiaddr;
}

MYDWORD alad(data9 *req)
{
	//debugl("alad");
	//printf("in alad hostname=%s\n", req->hostname);

	if (req->dhcpEntry && (req->req_type == DHCP_MESS_NONE || req->resp_type == DHCP_MESS_ACK))
	{
		MYDWORD hangTime = req->lease;

		if (req->rebind > req->lease)
			hangTime = req->rebind;

		req->dhcpEntry->display = true;
		req->dhcpEntry->local = true;
		setLeaseExpiry(req->dhcpEntry, hangTime);

		_beginthread(updateStateFile, 0, (void*)req->dhcpEntry);

		if (verbatim || cfig.dhcpLogLevel >= 1)
		{
			if (req->lease && req->reqIP)
			{
				sprintf(logBuff, "Host %s (%s) allotted %s for %u seconds", req->chaddr, req->hostname, IP2String(tempbuff, req->dhcpp.header.bp_yiaddr), req->lease);
			}
			else if (req->req_type)
			{
				sprintf(logBuff, "Host %s (%s) renewed %s for %u seconds", req->chaddr, req->hostname, IP2String(tempbuff, req->dhcpp.header.bp_yiaddr), req->lease);
			}
			else
			{
				sprintf(logBuff, "BOOTP Host %s (%s) allotted %s", req->chaddr, req->hostname, IP2String(tempbuff, req->dhcpp.header.bp_yiaddr));
			}
			logDHCPMess(logBuff, 1);
		}

		if (cfig.replication && cfig.dhcpRepl > t)
			sendRepl(req);

		return req->dhcpEntry->ip;
	}
	else if ((verbatim || cfig.dhcpLogLevel >= 2) && req->resp_type == DHCP_MESS_OFFER)
	{
		sprintf(logBuff, "Host %s (%s) offered %s", req->chaddr, req->hostname, IP2String(tempbuff, req->dhcpp.header.bp_yiaddr));
		logDHCPMess(logBuff, 2);
	}
	//printf("%u=out\n", req->resp_type);
	return 0;
}

void addOptions(data9 *req)
{
	//debug("addOptions");

	data3 op;
	int i;

	if (req->req_type && req->resp_type)
	{
		op.opt_code = DHCP_OPTION_MESSAGETYPE;
		op.size = 1;
		op.value[0] = req->resp_type;
		pvdata(req, &op);
	}

	if (req->dhcpEntry && req->resp_type != DHCP_MESS_DECLINE && req->resp_type != DHCP_MESS_NAK)
	{
		strcpy(req->dhcpp.header.bp_sname, cfig.servername);

		if (req->dhcpEntry->fixed)
		{
			//printf("%u,%u\n", req->dhcpEntry->options, *req->dhcpEntry->options);
			MYBYTE *opPointer = req->dhcpEntry->options;

			if (opPointer)
			{
				MYBYTE requestedOnly = *opPointer;
				opPointer++;

				while (*opPointer && *opPointer != DHCP_OPTION_END)
				{
					op.opt_code = *opPointer;
					opPointer++;
					op.size = *opPointer;
					opPointer++;

					if (!requestedOnly || req->paramreqlist[*opPointer])
					{
						memcpy(op.value, opPointer, op.size);
						pvdata(req, &op);
					}
					opPointer += op.size;
				}
			}
		}

		if (req->req_type && req->resp_type)
		{
			if (req->dhcpEntry->rangeInd >= 0)
			{
				MYBYTE *opPointer = cfig.dhcpRanges[req->dhcpEntry->rangeInd].options;
				//printf("Range=%i Pointer=%u\n", req->dhcpEntry->rangeInd,opPointer);

				if (opPointer)
				{
					MYBYTE requestedOnly = *opPointer;
					opPointer++;

					while (*opPointer && *opPointer != DHCP_OPTION_END)
					{
						op.opt_code = *opPointer;
						opPointer++;
						op.size = *opPointer;
						opPointer++;

						if (!requestedOnly || req->paramreqlist[*opPointer])
						{
							memcpy(op.value, opPointer, op.size);
							pvdata(req, &op);
						}
						opPointer += op.size;
					}
				}
			}

			MYBYTE *opPointer = cfig.options;

			if (opPointer)
			{
				MYBYTE requestedOnly = *opPointer;

				opPointer++;
				while (*opPointer && *opPointer != DHCP_OPTION_END)
				{
					op.opt_code = *opPointer;
					opPointer++;
					op.size = *opPointer;
					opPointer++;

					if (!requestedOnly || req->paramreqlist[*opPointer])
					{
						memcpy(op.value, opPointer, op.size);
						pvdata(req, &op);
					}
					opPointer += op.size;
				}
			}

			op.opt_code = DHCP_OPTION_SERVERID;
			op.size = 4;
			pIP(op.value, network.dhcpConn[req->sockInd].server);
			pvdata(req, &op);

			if (!req->opAdded[DHCP_OPTION_IPADDRLEASE])
			{
				op.opt_code = DHCP_OPTION_IPADDRLEASE;
				op.size = 4;
				pULong(op.value, cfig.lease);
				pvdata(req, &op);
			}

			if (!req->opAdded[DHCP_OPTION_NETMASK])
			{
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
			else if (strcasecmp(req->dhcpEntry->hostname, req->hostname))
			{
				free(req->dhcpEntry->hostname);
				req->dhcpEntry->hostname = cloneString(req->hostname);
			}

/*
			if (!req->opAdded[DHCP_OPTION_ROUTER])
			{
				op.opt_code = DHCP_OPTION_ROUTER;
				op.size = 4;
				pIP(op.value, network.dhcpConn[req->sockInd].server);
				pvdata(req, &op);
			}
*/
/*
			if (!req->opAdded[DHCP_OPTION_HOSTNAME])
			{
				op.opt_code = DHCP_OPTION_HOSTNAME;
				op.size = strlen(req->hostname);
				memcpy(op.value, req->hostname, op.size);
				pvdata(req, &op);
			}

			if (req->clientId.opt_code == DHCP_OPTION_CLIENTID)
				pvdata(req, &req->clientId);
*/
			if (req->subnet.opt_code == DHCP_OPTION_SUBNETSELECTION)
				pvdata(req, &req->subnet);

			if (req->agentOption.opt_code == DHCP_OPTION_RELAYAGENTINFO)
				pvdata(req, &req->agentOption);
		}
	}

	*(req->vp) = DHCP_OPTION_END;
}

void pvdata(data9 *req, data3 *op)
{
	//debugl("pvdata");

	if (!req->opAdded[op->opt_code] && ((req->vp - (MYBYTE*)&req->dhcpp) + op->size < req->messsize))
	{
		if (op->opt_code == DHCP_OPTION_NEXTSERVER)
			req->dhcpp.header.bp_siaddr = fIP(op->value);
		else if (op->opt_code == DHCP_OPTION_BP_FILE)
		{
			if (op->size <= 128)
				memcpy(req->dhcpp.header.bp_file, op->value, op->size);
		}
		else if(op->size)
		{
			if (op->opt_code == DHCP_OPTION_IPADDRLEASE)
			{
				if (!req->lease || req->lease > fULong(op->value))
					req->lease = fULong(op->value);

				if (req->lease >= INT_MAX)
					req->lease = UINT_MAX;

				pULong(op->value, req->lease);
			}
			else if (op->opt_code == DHCP_OPTION_REBINDINGTIME)
				req->rebind = fULong(op->value);
			else if (op->opt_code == DHCP_OPTION_HOSTNAME)
			{
				memcpy(req->hostname, op->value, op->size);
				req->hostname[op->size] = 0;
				req->hostname[63] = 0;

				if (char *ptr = strchr(req->hostname, '.'))
					*ptr = 0;
			}

			MYWORD tsize = op->size + 2;
			memcpy(req->vp, op, tsize);
			(req->vp) += tsize;
		}
		req->opAdded[op->opt_code] = true;
	}
}

void setTempLease(data7 *dhcpEntry)
{
	if (dhcpEntry && dhcpEntry->ip)
	{
		dhcpEntry->display = false;
		dhcpEntry->local = false;
		dhcpEntry->expiry = t + 20;

		int ind = getIndex(dhcpEntry->rangeInd, dhcpEntry->ip);

		if (ind >= 0)
		{
			if (cfig.dhcpRanges[dhcpEntry->rangeInd].expiry[ind] != INT_MAX)
				cfig.dhcpRanges[dhcpEntry->rangeInd].expiry[ind] = dhcpEntry->expiry;

			cfig.dhcpRanges[dhcpEntry->rangeInd].dhcpEntry[ind] = dhcpEntry;
		}
	}
}

void setLeaseExpiry(data7 *dhcpEntry, MYDWORD lease)
{
	//printf("%d=%d\n", t, lease);
	if (dhcpEntry && dhcpEntry->ip)
	{
		if (lease > (MYDWORD)(INT_MAX - t))
			dhcpEntry->expiry = INT_MAX;
		else
			dhcpEntry->expiry = t + lease;

		int ind = getIndex(dhcpEntry->rangeInd, dhcpEntry->ip);

		if (ind >= 0)
		{
			if (cfig.dhcpRanges[dhcpEntry->rangeInd].expiry[ind] != INT_MAX)
				cfig.dhcpRanges[dhcpEntry->rangeInd].expiry[ind] = dhcpEntry->expiry;

			cfig.dhcpRanges[dhcpEntry->rangeInd].dhcpEntry[ind] = dhcpEntry;
		}
	}
}

void setLeaseExpiry(data7 *dhcpEntry)
{
	if (dhcpEntry && dhcpEntry->ip)
	{
		int ind = getIndex(dhcpEntry->rangeInd, dhcpEntry->ip);

		if (ind >= 0)
		{
			if (cfig.dhcpRanges[dhcpEntry->rangeInd].expiry[ind] != INT_MAX)
				cfig.dhcpRanges[dhcpEntry->rangeInd].expiry[ind] = dhcpEntry->expiry;

			cfig.dhcpRanges[dhcpEntry->rangeInd].dhcpEntry[ind] = dhcpEntry;
		}
	}
}

void lockIP(MYDWORD ip)
{
	MYDWORD iip = htonl(ip);

	for (char rangeInd = 0; rangeInd < cfig.rangeCount; rangeInd++)
	{
		if (iip >= cfig.dhcpRanges[rangeInd].rangeStart && iip <= cfig.dhcpRanges[rangeInd].rangeEnd)
		{
			int ind = iip - cfig.dhcpRanges[rangeInd].rangeStart;

			if (cfig.dhcpRanges[rangeInd].expiry[ind] != INT_MAX)
				cfig.dhcpRanges[rangeInd].expiry[ind] = INT_MAX;

			break;
		}
	}
}

void holdIP(MYDWORD ip)
{
	if (ip)
	{
		MYDWORD iip = htonl(ip);

		for (char rangeInd = 0; rangeInd < cfig.rangeCount; rangeInd++)
		{
			if (iip >= cfig.dhcpRanges[rangeInd].rangeStart && iip <= cfig.dhcpRanges[rangeInd].rangeEnd)
			{
				int ind = iip - cfig.dhcpRanges[rangeInd].rangeStart;

				if (cfig.dhcpRanges[rangeInd].expiry[ind] == 0)
					cfig.dhcpRanges[rangeInd].expiry[ind] = 1;

				break;
			}
		}
	}
}

void __cdecl sendToken(void *lpParam)
{
	//debug("Send Token");
	Sleep(1000 * 10);

	while (true)
	{
		errno = 0;

		sendto(cfig.dhcpReplConn.sock,
				token.raw,
				token.bytes,
				0,
				(sockaddr*)&token.remote,
				sizeof(token.remote));

//		errno = WSAGetLastError();
//
//		if (!errno && verbatim || cfig.dhcpLogLevel >= 2)
//		{
//			sprintf(logBuff, "Token Sent");
//			logDHCPMess(logBuff, 2);
//		}

		Sleep(1000 * 300);
	}

	_endthread();
	return;
}


MYDWORD sendRepl(data9 *req)
{
	data3 op;

	MYBYTE *opPointer = req->dhcpp.vend_data;

	while ((*opPointer) != DHCP_OPTION_END && opPointer < req->vp)
	{
		if ((*opPointer) == DHCP_OPTION_MESSAGETYPE)
		{
			*(opPointer + 2) = DHCP_MESS_INFORM;
			break;
		}
		opPointer = opPointer + *(opPointer + 1) + 2;
	}

	if (!req->opAdded[DHCP_OPTION_MESSAGETYPE])
	{
		op.opt_code = DHCP_OPTION_MESSAGETYPE;
		op.size = 1;
		op.value[0] = DHCP_MESS_INFORM;
		pvdata(req, &op);
	}

	if (req->hostname[0] && !req->opAdded[DHCP_OPTION_HOSTNAME])
	{
		op.opt_code = DHCP_OPTION_HOSTNAME;
		op.size = strlen(req->hostname);
		memcpy(op.value, req->hostname, op.size);
		pvdata(req, &op);
	}

//	op.opt_code = DHCP_OPTION_SERIAL;
//	op.size = 4;
//	pULong(op.value, cfig.serial1);
//	pvdata(req, &op);

	*(req->vp) = DHCP_OPTION_END;
	req->vp++;
	req->bytes = req->vp - (MYBYTE*)req->raw;

	req->dhcpp.header.bp_op = BOOTP_REQUEST;
	errno = 0;

	req->bytes = sendto(cfig.dhcpReplConn.sock,
	                    req->raw,
	                    req->bytes,
	                    0,
						(sockaddr*)&token.remote,
						sizeof(token.remote));

	errno = WSAGetLastError();

	if (errno || req->bytes <= 0)
	{
		cfig.dhcpRepl = 0;

		if (verbatim || cfig.dhcpLogLevel >= 1)
		{
			if (cfig.replication == 1)
				sprintf(logBuff, "WSAError %u Sending DHCP Update to Secondary Server", errno);
			else
				sprintf(logBuff, "WSAError %u Sending DHCP Update to Primary Server", errno);

			logDHCPMess(logBuff, 1);
		}

		return 0;
	}
	else if (verbatim || cfig.dhcpLogLevel >= 2)
	{
		if (cfig.replication == 1)
			sprintf(logBuff, "DHCP Update for host %s (%s) sent to Secondary Server", req->dhcpEntry->mapname, IP2String(tempbuff, req->dhcpEntry->ip));
		else
			sprintf(logBuff, "DHCP Update for host %s (%s) sent to Primary Server", req->dhcpEntry->mapname, IP2String(tempbuff, req->dhcpEntry->ip));

		logDHCPMess(logBuff, 2);
	}

	return req->dhcpp.header.bp_yiaddr;
}

/*
MYDWORD sendRepl(data7 *dhcpEntry)
{
	data9 req;
	memset(&req, 0, sizeof(data9));
	req.vp = req.dhcpp.vend_data;
	req.messsize = sizeof(dhcp_packet);
	req.dhcpEntry = dhcpEntry;

	req.dhcpp.header.bp_op = BOOTP_REQUEST;
	req.dhcpp.header.bp_xid = t;
	req.dhcpp.header.bp_ciaddr = dhcpEntry->ip;
	req.dhcpp.header.bp_yiaddr = dhcpEntry->ip;
	req.dhcpp.header.bp_hlen = 16;
	getHexValue(req.dhcpp.header.bp_chaddr, req.dhcpEntry->mapname, &(req.dhcpp.header.bp_hlen));
	req.dhcpp.header.bp_magic_num[0] = 99;
	req.dhcpp.header.bp_magic_num[1] = 130;
	req.dhcpp.header.bp_magic_num[2] = 83;
	req.dhcpp.header.bp_magic_num[3] = 99;
	strcpy(req.hostname, dhcpEntry->hostname);

	return sendRepl(&req);
}
*/

void recvRepl(data9 *req)
{
	cfig.dhcpRepl = t + 600;

	MYDWORD ip = req->dhcpp.header.bp_yiaddr ? req->dhcpp.header.bp_yiaddr : req->dhcpp.header.bp_ciaddr;

	if (!ip || !req->dhcpp.header.bp_hlen)
	{
//		if (verbatim || cfig.dhcpLogLevel >= 2)
//		{
//			sprintf(logBuff, "Token Received");
//			logDHCPMess(logBuff, 2);
//		}

		if (cfig.replication == 1)
		{
			errno = 0;

			sendto(cfig.dhcpReplConn.sock,
					token.raw,
					token.bytes,
					0,
					(sockaddr*)&token.remote,
					sizeof(token.remote));

//			//errno = WSAGetLastError();
//
//			if (!errno && (verbatim || cfig.dhcpLogLevel >= 2))
//			{
//				sprintf(logBuff, "Token Responded");
//				logDHCPMess(logBuff, 2);
//			}
		}
		return;
	}

	char rInd = getRangeInd(ip);

	if (rInd >= 0)
	{
		int ind  = getIndex(rInd, ip);
		req->dhcpEntry = cfig.dhcpRanges[rInd].dhcpEntry[ind];

		if (req->dhcpEntry && !req->dhcpEntry->fixed && strcasecmp(req->dhcpEntry->mapname, req->chaddr))
			req->dhcpEntry->expiry = 0;
	}

	req->dhcpEntry = findDHCPEntry(req->chaddr);

	if (req->dhcpEntry && req->dhcpEntry->ip != ip)
	{
		if (req->dhcpEntry->fixed)
		{
			if (cfig.replication == 1)
				sprintf(logBuff, "DHCP Update ignored for %s (%s) from Secondary Server", req->chaddr, IP2String(tempbuff, ip));
			else
				sprintf(logBuff, "DHCP Update ignored for %s (%s) from Primary Server", req->chaddr, IP2String(tempbuff, ip));

			logDHCPMess(logBuff, 1);
			return;
		}
		else if (req->dhcpEntry->rangeInd >= 0)
		{
			int ind  = getIndex(req->dhcpEntry->rangeInd, req->dhcpEntry->ip);

			if (ind >= 0)
				cfig.dhcpRanges[req->dhcpEntry->rangeInd].dhcpEntry[ind] = 0;
		}
	}

	if (!req->dhcpEntry && rInd >= 0)
	{
		memset(&lump, 0, sizeof(data71));
		lump.mapname = req->chaddr;
		lump.hostname = req->hostname;
		req->dhcpEntry = createCache(&lump);

		if (req->dhcpEntry)
			dhcpCache[req->dhcpEntry->mapname] = req->dhcpEntry;
/*
		req->dhcpEntry = (data7*)calloc(1, sizeof(data7));

		if (!req->dhcpEntry)
		{
			sprintf(logBuff, "Memory Allocation Error");
			logDHCPMess(logBuff, 1);
			return;
		}

		req->dhcpEntry->mapname = cloneString(req->chaddr);

		if (!req->dhcpEntry->mapname)
		{
			sprintf(logBuff, "Memory Allocation Error");
			free(req->dhcpEntry);
			logDHCPMess(logBuff, 1);
			return;
		}
*/
	}

	if (req->dhcpEntry)
	{
		req->dhcpEntry->ip = ip;
		req->dhcpEntry->rangeInd = rInd;
		req->dhcpEntry->display = true;
		req->dhcpEntry->local = false;

		MYDWORD hangTime = req->lease;

		if (req->rebind > req->lease)
			hangTime = req->rebind;

		setLeaseExpiry(req->dhcpEntry, hangTime);

		if (req->hostname[0])
		{
			if (req->dhcpEntry->hostname && strcasecmp(req->dhcpEntry->hostname, req->hostname))
			{
				free (req->dhcpEntry->hostname);
				req->dhcpEntry->hostname = cloneString(req->hostname);
			}
			else if (!req->dhcpEntry->hostname)
				req->dhcpEntry->hostname = cloneString(req->hostname);
		}

		_beginthread(updateStateFile, 0, (void*)req->dhcpEntry);
	}
	else
	{
		if (cfig.replication == 1)
			sprintf(logBuff, "DHCP Update ignored for %s (%s) from Secondary Server", req->chaddr, IP2String(tempbuff, ip));
		else
			sprintf(logBuff, "DHCP Update ignored for %s (%s) from Primary Server", req->chaddr, IP2String(tempbuff, ip));

		logDHCPMess(logBuff, 1);
		return;
	}

	if (verbatim || cfig.dhcpLogLevel >= 2)
	{
		if (cfig.replication == 1)
			sprintf(logBuff, "DHCP Update received for %s (%s) from Secondary Server", req->chaddr, IP2String(tempbuff, ip));
		else
			sprintf(logBuff, "DHCP Update received for %s (%s) from Primary Server", req->chaddr, IP2String(tempbuff, ip));

		logDHCPMess(logBuff, 2);
	}
}

char getRangeInd(MYDWORD ip)
{
	if (ip)
	{
		MYDWORD iip = htonl(ip);

		for (char k = 0; k < cfig.rangeCount; k++)
			if (iip >= cfig.dhcpRanges[k].rangeStart && iip <= cfig.dhcpRanges[k].rangeEnd)
				return k;
	}
	return -1;
}

int getIndex(char rangeInd, MYDWORD ip)
{
	if (ip && rangeInd >= 0 && rangeInd < cfig.rangeCount)
	{
		MYDWORD iip = htonl(ip);
		if (iip >= cfig.dhcpRanges[rangeInd].rangeStart && iip <= cfig.dhcpRanges[rangeInd].rangeEnd)
			return (iip - cfig.dhcpRanges[rangeInd].rangeStart);
	}
	return -1;
}

void loadOptions(FILE *f, const char *sectionName, data20 *optionData) {
  optionData->ip = 0;
  optionData->mask = 0;
  MYBYTE maxInd = sizeof(opData) / sizeof(data4);
  MYWORD buffsize = sizeof(dhcp_packet) - sizeof(dhcp_header);
  MYBYTE *dp = optionData->options;
  MYBYTE op_specified[256];

  memset(op_specified, 0, 256);
  *dp = 0;
  dp++;

  char raw[512];
  char name[512];
  char value[512];

  while (readSection(raw, f)) {

    MYBYTE *ddp = dp;
    MYBYTE hoption[256];
    MYBYTE valSize = sizeof(hoption) - 1;
    MYBYTE opTag = 0;
    MYBYTE opType = 0;
    MYBYTE valType = 0;
    bool tagFound = false;

    mySplit(name, value, raw, '=');

    //printf("%s=%s\n", name, value);

    if (!name[0]) {
      sprintf(logBuff, "Warning: section [%s] invalid option %s ignored", sectionName, raw);
      logDHCPMess(logBuff, 1);
      continue;
    }

    if (!strcasecmp(name, "DHCPRange")) {
      if (!strcasecmp(sectionName, RANGESET)) addDHCPRange(value);
      else {
        sprintf(logBuff, "Warning: section [%s] option %s not allowed in this section, option ignored", sectionName, raw);
        logDHCPMess(logBuff, 1);
      }
      continue;
    } else if (!strcasecmp(name, "IP")) {
      if (!strcasecmp(sectionName, GLOBALOPTIONS) || !strcasecmp(sectionName, RANGESET)) {
        sprintf(logBuff, "Warning: section [%s] option %s not allowed in this section, option ignored", sectionName, raw);
        logDHCPMess(logBuff, 1);
      } else if (!isIP(value) && strcasecmp(value, "0.0.0.0")) {
        sprintf(logBuff, "Warning: section [%s] option Invalid IP Addr %s option ignored", sectionName, value);
	logDHCPMess(logBuff, 1);
      } else
        optionData->ip = inet_addr(value);
      continue;
    } else if (!strcasecmp(name, "FilterMacRange")) {
      if (!strcasecmp(sectionName, RANGESET)) addMacRange(optionData->rangeSetInd, value);
      else {
        sprintf(logBuff, "Warning: section [%s] option %s not allowed in this section, option ignored", sectionName, raw);
	logDHCPMess(logBuff, 1);
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
	sprintf(logBuff, "Warning: section [%s] option %s value too big, option ignored", sectionName, raw);
	logDHCPMess(logBuff, 1);
	continue;
      }
    } else if (strchr(value, ':')) {
      valType = 2;
      valSize = sizeof(hoption) - 1;
      char *errorPos = getHexValue(hoption, value, &valSize);

      if (errorPos) {
        valType = 1;
        valSize = strlen(value);
      } else memcpy(value, hoption, valSize);
    } else if (isInt(value) && atol(value) > USHRT_MAX) valType = 4;
      else if (isInt(value) && atoi(value) > UCHAR_MAX) valType = 5;
      else if (isInt(value)) valType = 6;
//    else if ((strchr(value, '.') && (opType == 2 || opType == 3 || opType == 8 || opType == 0)) || (!strchr(value, '.') && strchr(value, ',')))
      else if (strchr(value, '.') || strchr(value, ',')) {
	valType = 2;
	char buff[1024];
	int numbytes = myTokenize(buff, value, "/,.", true);

	if (numbytes > 255) {
  	  sprintf(logBuff, "Warning: section [%s] option %s, too many bytes, entry ignored", sectionName, raw);
	  logDHCPMess(logBuff, 1);
	  continue;
	} else {
	  char *ptr = buff;
	  valSize = 0;

	  for (; *ptr; ptr = myGetToken(ptr, 1)) {
 	    //printf("%s:", ptr);
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
	sprintf(logBuff, "Warning: section [%s] option %s value too long, option ignored", sectionName, raw);
	logDHCPMess(logBuff, 1);
	continue;
      }
    }

    if (!strcasecmp(name, "FilterVendorClass")) {
      if (!strcasecmp(sectionName, RANGESET)) addVendClass(optionData->rangeSetInd, value, valSize);
      else {
	sprintf(logBuff, "Warning: section [%s] option %s not allowed in this section, option ignored", sectionName, raw);
	logDHCPMess(logBuff, 1);
      }
      continue;
    } else if (!strcasecmp(name, "FilterUserClass")) {
      if (!strcasecmp(sectionName, RANGESET))
        addUserClass(optionData->rangeSetInd, value, valSize);
      else {
	sprintf(logBuff, "Warning: section [%s] option %s not allowed in this section, option ignored", sectionName, raw);
	logDHCPMess(logBuff, 1);
      }
      continue;
    } else if (!strcasecmp(name, "FilterSubnetSelection")) {
      if (valSize != 4) {
	sprintf(logBuff, "Warning: section [%s] invalid value %s, option ignored", sectionName, raw);
	logDHCPMess(logBuff, 1);
      } else if (!strcasecmp(sectionName, RANGESET)) {
        addServer(cfig.rangeSet[optionData->rangeSetInd].subnetIP, MAX_RANGE_FILTERS, fIP(value));
        cfig.hasFilter = 1;
      } else {
	sprintf(logBuff, "Warning: section [%s] option %s not allowed in this section, option ignored", sectionName, raw);
	logDHCPMess(logBuff, 1);
      }
      continue;
    } else if (!strcasecmp(name, "TargetRelayAgent")) {
      if (valSize != 4) {
	sprintf(logBuff, "Warning: section [%s] invalid value %s, option ignored", sectionName, raw);
	logDHCPMess(logBuff, 1);
      } else if (!strcasecmp(sectionName, RANGESET)) {
	cfig.rangeSet[optionData->rangeSetInd].targetIP = fIP(value);
	//printf("TARGET IP %s set RangeSetInd  %d\n", IP2String(tempbuff, cfig.rangeSet[optionData->rangeSetInd].targetIP), optionData->rangeSetInd);
      } else {
	sprintf(logBuff, "Warning: section [%s] option %s not allowed in this section, option ignored", sectionName, raw);
	logDHCPMess(logBuff, 1);
      }
      continue;
    }
    opTag = 0;

    if (isInt(name)) {
      if (atoi(name) < 1 || atoi(name) >= 254) {
        sprintf(logBuff, "Warning: section [%s] invalid option %s, ignored", sectionName, raw);
        logDHCPMess(logBuff, 1);
	continue;
      }
      opTag = atoi(name);
      opType = 0;
    }

    for (MYBYTE i = 0; i < maxInd; i++)
			if (!strcasecmp(name, opData[i].opName) || (opTag && opTag == opData[i].opTag))
			{
				opTag = opData[i].opTag;
				opType = opData[i].opType;
				tagFound = true;
				break;
			}

		if (!opTag)
		{
			sprintf(logBuff, "Warning: section [%s] invalid option %s, ignored", sectionName, raw);
			logDHCPMess(logBuff, 1);
			continue;
		}

		if (!opType)
			opType = valType;

		//sprintf(logBuff, "Tag %i ValType %i opType %i value=%s size=%u", opTag, valType, opType, value, valSize);
		//logDHCPMess(logBuff, 1);

		if (op_specified[opTag])
		{
			sprintf(logBuff, "Warning: section [%s] duplicate option %s, ignored", sectionName, raw);
			logDHCPMess(logBuff, 1);
			continue;
		}

		//printf("Option=%u opType=%u valueType=%u valSize=%u\n", opTag, opType, valType, valSize);

		op_specified[opTag] = true;

		if (valType == 9)
		{
			if (buffsize > 2)
			{
				*dp = opTag;
				dp++;
				*dp = 0;
				dp++;
				buffsize -= 2;
			}
			else
			{
				sprintf(logBuff, "Warning: section [%s] option %s, no more space for options", sectionName, raw);
				logDHCPMess(logBuff, 1);
			}
			continue;
		}

		switch (opType)
		{
			case 1:
			{
				value[valSize] = 0;
				valSize++;

				if (valType != 1 && valType != 2)
				{
					sprintf(logBuff, "Warning: section [%s] option %s, need string value, option ignored", sectionName, raw);
					logDHCPMess(logBuff, 1);
				}
//				else if (opTag == DHCP_OPTION_DOMAINNAME)
//				{
//					sprintf(logBuff, "Warning: section [%s] option %u should be under [DOMAIN_NAME], ignored", sectionName, opTag);
//					logDHCPMess(logBuff, 1);
//					continue;
//				}
				else if (buffsize > valSize + 2)
				{
					*dp = opTag;
					dp++;
					*dp = valSize;
					dp++;
					memcpy(dp, value, valSize);
					dp += valSize;
					buffsize -= (valSize + 2);
				}
				else
				{
					sprintf(logBuff, "Warning: section [%s] option %s, no more space for options", sectionName, raw);
					logDHCPMess(logBuff, 1);
				}
			}
			break;

			case 3:
			case 8:
			{
				if (valType == 2)
				{
					if (opType == 3 && valSize % 4)
					{
						sprintf(logBuff, "Warning: section [%s] option %s, missing/extra bytes/octates in IP, option ignored", sectionName, raw);
						logDHCPMess(logBuff, 1);
						continue;
					}
					else if (opType == 8 && valSize % 8)
					{
						sprintf(logBuff, "Warning: section [%s] option %s, some values not in IP/Mask form, option ignored", sectionName, raw);
						logDHCPMess(logBuff, 1);
						continue;
					}

					if (opTag == DHCP_OPTION_NETMASK)
					{
						if (valSize != 4 || !checkMask(fIP(value)))
						{
							sprintf(logBuff, "Warning: section [%s] Invalid subnetmask %s, option ignored", sectionName, raw);
							logDHCPMess(logBuff, 1);
							continue;
						}
						else
							optionData->mask = fIP(value);
					}

					if (buffsize > valSize + 2)
					{
						*dp = opTag;
						dp++;
						*dp = valSize;
						dp++;
						memcpy(dp, value, valSize);
						dp += valSize;
						buffsize -= (valSize + 2);
					}
					else
					{
						sprintf(logBuff, "Warning: section [%s] option %s, no more space for options", sectionName, raw);
						logDHCPMess(logBuff, 1);
					}
				}
				else
				{
					sprintf(logBuff, "Warning: section [%s] option %s, Invalid value, should be one or more IP/4 Bytes", sectionName, raw);
					logDHCPMess(logBuff, 1);
				}
			}
			break;

			case 4:
			{
				MYDWORD j;

				if (valType == 2 && valSize == 4)
					j = fULong(value);
				else if (valType >= 4 && valType <= 6)
					j = atol(value);
				else
				{
					sprintf(logBuff, "Warning: section [%s] option %s, value should be integer between 0 & %u or 4 bytes, option ignored", sectionName, name, UINT_MAX);
					logDHCPMess(logBuff, 1);
					continue;
				}

				if (opTag == DHCP_OPTION_IPADDRLEASE)
				{
					if (j == 0)
						j = UINT_MAX;
//
//					if (!strcasecmp(sectionName, GLOBALOPTIONS))
//					{
//						sprintf(logBuff, "Warning: section [%s] option %s not allowed in this section, please set it in [TIMINGS] section", sectionName, raw);
//						logDHCPMess(logBuff, 1);
//						continue;
//					}
//					else if (j > cfig.lease)
//					{
//						sprintf(logBuff, "Warning: section [%s] option %s value should be less then %u (max lease), ignored", sectionName, name, cfig.lease);
//						logDHCPMess(logBuff, 1);
//						continue;
//					}
				}

				if (buffsize > 6)
				{
					*dp = opTag;
					dp++;
					*dp = 4;
					dp++;
					dp += pULong(dp, j);
					buffsize -= 6;
					//printf("%s=%u=%u\n",opData[op_index].opName,opData[op_index].opType,htonl(j));
				}
				else
				{
					sprintf(logBuff, "Warning: section [%s] option %s, no more space for options", sectionName, raw);
					logDHCPMess(logBuff, 1);
				}
			}
			break;

			case 5:
			{
				MYWORD j;

				if (valType == 2 && valSize == 2)
					j = fUShort(value);
				else if (valType == 5 || valType == 6)
					j = atol(value);
				else
				{
					sprintf(logBuff, "Warning: section [%s] option %s, value should be between 0 & %u or 2 bytes, option ignored", sectionName, name, USHRT_MAX);
					logDHCPMess(logBuff, 1);
					continue;
				}

				if (buffsize > 4)
				{
					*dp = opTag;
					dp++;
					*dp = 2;
					dp++;
					dp += pUShort(dp, j);
					buffsize -= 4;
				}
				else
				{
					sprintf(logBuff, "Warning: section [%s] option %s, no more space for options", sectionName, raw);
					logDHCPMess(logBuff, 1);
				}
			}
			break;

			case 6:
			{
				MYBYTE j;

				if (valType == 2 && valSize == 1)
					j = *value;
				else if (valType == 6)
					j = atol(value);
				else
				{
					sprintf(logBuff, "Warning: section [%s] option %s, value should be between 0 & %u or single byte, option ignored", sectionName, name, UCHAR_MAX);
					logDHCPMess(logBuff, 1);
					continue;
				}

				if (buffsize > 3)
				{
					*dp = opTag;
					dp++;
					*dp = 1;
					dp++;
					*dp = j;
					dp++;
					buffsize -= 3;
				}
				else
				{
					sprintf(logBuff, "Warning: section [%s] option %s, no more space for options", sectionName, raw);
					logDHCPMess(logBuff, 1);
				}
			}
			break;

			case 7:
			{
				MYBYTE j;

				if (valType == 2 && valSize == 1 && *value < 2)
					j = *value;
				else if (valType == 1 && (!strcasecmp(value, "yes") || !strcasecmp(value, "on") || !strcasecmp(value, "true")))
					j = 1;
				else if (valType == 1 && (!strcasecmp(value, "no") || !strcasecmp(value, "off") || !strcasecmp(value, "false")))
					j = 0;
				else if (valType == 6 && atoi(value) < 2)
					j = atoi(value);
				else
				{
					sprintf(logBuff, "Warning: section [%s] option %s, value should be yes/on/true/1 or no/off/false/0, option ignored", sectionName, raw);
					logDHCPMess(logBuff, 1);
					continue;
				}

				if (buffsize > 3)
				{
					*dp = opTag;
					dp++;
					*dp = 1;
					dp++;
					*dp = j;
					dp++;
					buffsize -= 3;
				}
				else
				{
					sprintf(logBuff, "Warning: section [%s] option %s, no more space for options", sectionName, raw);
					logDHCPMess(logBuff, 1);
				}
			}
			break;

			default:
			{
				if (valType == 6)
				{
					valType = 2;
					valSize = 1;
					*value = atoi(value);
				}

				if (opType == 2 && valType != 2)
				{
					sprintf(logBuff, "Warning: section [%s] option %s, value should be comma separated bytes or hex string, option ignored", sectionName, raw);
					logDHCPMess(logBuff, 1);
					continue;
				}
				else if (buffsize > valSize + 2)
				{
					*dp = opTag;
					dp++;
					*dp = valSize;
					dp++;
					memcpy(dp, value, valSize);
					dp += valSize;
					buffsize -= (valSize + 2);
				}
				else
				{
					sprintf(logBuff, "Warning: section [%s] option %s, no more space for options", sectionName, raw);
					logDHCPMess(logBuff, 1);
				}
			}
			break;
		}

		//printf("%s Option=%u opType=%u valType=%u  valSize=%u\n", raw, opTag, opType, valType, valSize);
		//printf("%s %s\n", name, hex2String(tempbuff, ddp, valSize+2, ':'));
	}

	//printf("%s=%s\n", sectionName, optionData->vendClass);

	*dp = DHCP_OPTION_END;
	dp++;
	optionData->optionSize = (dp - optionData->options);
	//printf("section=%s buffersize = %u option size=%u\n", sectionName, buffsize, optionData->optionSize);
}

void lockOptions(FILE *f)
{
	char raw[512];
	char name[512];
	char value[512];

	while (readSection(raw, f))
	{
		mySplit(name, value, raw, '=');

		if (!name[0] || !value[0])
			continue;

		int op_index;
		MYBYTE n = sizeof(opData) / sizeof(data4);

		for (op_index = 0; op_index < n; op_index++)
			if (!strcasecmp(name, opData[op_index].opName) || (opData[op_index].opTag && atoi(name) == opData[op_index].opTag))
				break;

		if (op_index >= n)
			continue;

		if (opData[op_index].opType == 3)
		{
			if (myTokenize(value, value, "/,.", true))
			{
				char *ptr = value;
				char hoption[256];
				MYBYTE valueSize = 0;

				for (; *ptr; ptr = myGetToken(ptr, 1))
				{
					if (valueSize >= UCHAR_MAX)
						break;
					else if (isInt(ptr) && atoi(ptr) <= UCHAR_MAX)
					{
						hoption[valueSize] = atoi(ptr);
						valueSize++;
					}
					else
						break;
				}

				if (*ptr)
					continue;

				if (valueSize % 4)
					continue;

				for (MYBYTE i = 0; i < valueSize; i += 4)
				{
					MYDWORD ip = *((MYDWORD*)&(hoption[i]));

					if (ip != INADDR_ANY && ip != INADDR_NONE)
						lockIP(ip);
				}
			}
		}
	}
}

void addDHCPRange(char *dp)
{
	MYDWORD rs = 0;
	MYDWORD re = 0;
	char name[512];
	char value[512];
	mySplit(name, value, dp, '-');

	if (isIP(name) && isIP(value))
	{
		rs = htonl(inet_addr(name));
		re = htonl(inet_addr(value));

		if (rs && re && rs <= re)
		{
			data13 *range;
			MYBYTE m = 0;

			for (; m < MAX_DHCP_RANGES && cfig.dhcpRanges[m].rangeStart; m++)
			{
				range = &cfig.dhcpRanges[m];

				if ((rs >= range->rangeStart && rs <= range->rangeEnd)
						|| (re >= range->rangeStart && re <= range->rangeEnd)
						|| (range->rangeStart >= rs && range->rangeStart <= re)
						|| (range->rangeEnd >= rs && range->rangeEnd <= re))
				{
					sprintf(logBuff, "Warning: DHCP Range %s overlaps with another range, ignored", dp);
					logDHCPMess(logBuff, 1);
					return;
				}
			}

			if (m < MAX_DHCP_RANGES)
			{
				cfig.dhcpSize += (re - rs + 1);
				range = &cfig.dhcpRanges[m];
				range->rangeStart = rs;
				range->rangeEnd = re;
				range->expiry = (time_t*)calloc((re - rs + 1), sizeof(time_t));
				range->dhcpEntry = (data7**)calloc((re - rs + 1), sizeof(data7*));

				if (!range->expiry || !range->dhcpEntry)
				{
					if (range->expiry)
						free(range->expiry);

					if (range->dhcpEntry)
						free(range->dhcpEntry);

					sprintf(logBuff, "DHCP Ranges Load, Memory Allocation Error");
					logDHCPMess(logBuff, 1);
					return;
				}
			}
		}
		else
		{
			sprintf(logBuff, "Section [%s] Invalid DHCP range %s in ini file, ignored", RANGESET, dp);
			logDHCPMess(logBuff, 1);
		}
	}
	else
	{
		sprintf(logBuff, "Section [%s] Invalid DHCP range %s in ini file, ignored", RANGESET, dp);
		logDHCPMess(logBuff, 1);
	}
}

void addVendClass(MYBYTE rangeSetInd, char *vendClass, MYBYTE vendClassSize)
{
	data14 *rangeSet = &cfig.rangeSet[rangeSetInd];

	MYBYTE i = 0;

	for (; i <= MAX_RANGE_FILTERS && rangeSet->vendClassSize[i]; i++);

	if (i >= MAX_RANGE_FILTERS || !vendClassSize)
		return;

	rangeSet->vendClass[i] = (MYBYTE*)calloc(vendClassSize, 1);

	if(!rangeSet->vendClass[i])
	{
		sprintf(logBuff, "Vendor Class Load, Memory Allocation Error");
		logDHCPMess(logBuff, 1);
	}
	else
	{
		cfig.hasFilter = true;
		rangeSet->vendClassSize[i] = vendClassSize;
		memcpy(rangeSet->vendClass[i], vendClass, vendClassSize);
		//printf("Loaded Vendor Class %s Size=%i rangeSetInd=%i Ind=%i\n", rangeSet->vendClass[i], rangeSet->vendClassSize[i], rangeSetInd, i);
		//printf("Loaded Vendor Class %s Size=%i rangeSetInd=%i Ind=%i\n", hex2String(tempbuff, rangeSet->vendClass[i], rangeSet->vendClassSize[i], ':'), rangeSet->vendClassSize[i], rangeSetInd, i);
	}
}

void addUserClass(MYBYTE rangeSetInd, char *userClass, MYBYTE userClassSize)
{
	data14 *rangeSet = &cfig.rangeSet[rangeSetInd];

	MYBYTE i = 0;

	for (; i <= MAX_RANGE_FILTERS && rangeSet->userClassSize[i]; i++);

	if (i >= MAX_RANGE_FILTERS || !userClassSize)
		return;

	rangeSet->userClass[i] = (MYBYTE*)calloc(userClassSize, 1);

	if(!rangeSet->userClass[i])
	{
		sprintf(logBuff, "Vendor Class Load, Memory Allocation Error");
		logDHCPMess(logBuff, 1);
	}
	else
	{
		cfig.hasFilter = true;
		rangeSet->userClassSize[i] = userClassSize;
		memcpy(rangeSet->userClass[i], userClass, userClassSize);
		//printf("Loaded User Class %s Size=%i rangeSetInd=%i Ind=%i\n", hex2String(tempbuff, rangeSet->userClass[i], rangeSet->userClassSize[i], ':'), rangeSet->vendClassSize[i], rangeSetInd, i);
	}
}

void addMacRange(MYBYTE rangeSetInd, char *macRange)
{
	if (macRange[0])
	{
		data14 *rangeSet = &cfig.rangeSet[rangeSetInd];

		MYBYTE i = 0;

		for (; i <= MAX_RANGE_FILTERS && rangeSet->macSize[i]; i++);

		if (i >= MAX_RANGE_FILTERS)
			return;

		char name[256];
		char value[256];

		mySplit(name, value, macRange, '-');

		//printf("%s=%s\n", name, value);

		if(!name[0] || !value[0])
		{
			sprintf(logBuff, "Section [%s], invalid Filter_Mac_Range %s, ignored", RANGESET, macRange);
			logDHCPMess(logBuff, 1);
		}
		else
		{
			MYBYTE macSize1 = 16;
			MYBYTE macSize2 = 16;
			MYBYTE *macStart = (MYBYTE*)calloc(1, macSize1);
			MYBYTE *macEnd = (MYBYTE*)calloc(1, macSize2);

			if(!macStart || !macEnd)
			{
				sprintf(logBuff, "DHCP Range Load, Memory Allocation Error");
				logDHCPMess(logBuff, 1);
			}
			else if (getHexValue(macStart, name, &macSize1) || getHexValue(macEnd, value, &macSize2))
			{
				sprintf(logBuff, "Section [%s], Invalid character in Filter_Mac_Range %s", RANGESET, macRange);
				logDHCPMess(logBuff, 1);
				free(macStart);
				free(macEnd);
			}
			else if (memcmp(macStart, macEnd, 16) > 0)
			{
				sprintf(logBuff, "Section [%s], Invalid Filter_Mac_Range %s, (higher bound specified on left), ignored", RANGESET, macRange);
				logDHCPMess(logBuff, 1);
				free(macStart);
				free(macEnd);
			}
			else if (macSize1 != macSize2)
			{
				sprintf(logBuff, "Section [%s], Invalid Filter_Mac_Range %s, (start/end size mismatched), ignored", RANGESET, macRange);
				logDHCPMess(logBuff, 1);
				free(macStart);
				free(macEnd);
			}
			else
			{
				cfig.hasFilter = true;
				rangeSet->macSize[i] = macSize1;
				rangeSet->macStart[i] = macStart;
				rangeSet->macEnd[i] = macEnd;
				//printf("Mac Loaded, Size=%i Start=%s rangeSetInd=%i Ind=%i\n", rangeSet->macSize[i], hex2String(tempbuff, rangeSet->macStart[i], rangeSet->macSize[i]), rangeSetInd, i);
			}
		}
	}
}

void loadDHCP() {

  data7 *dhcpEntry = NULL;
  char mapname[64];
  FILE *f = NULL;
  FILE *ff = NULL;

  if (f = openSection(GLOBALOPTIONS, 1)) {
    data20 optionData;
    loadOptions(f, GLOBALOPTIONS, &optionData);
    cfig.options = (MYBYTE*)calloc(1, optionData.optionSize);
    memcpy(cfig.options, optionData.options, optionData.optionSize);
    cfig.mask = optionData.mask;
  }

  if (!cfig.mask)
    cfig.mask = inet_addr("255.255.255.0");

  for (MYBYTE i = 1; i <= MAX_RANGE_SETS ; i++) {
    if (f = openSection(RANGESET, i)) {
      MYBYTE m = cfig.rangeCount;
      data20 optionData;
      optionData.rangeSetInd = i - 1;
      loadOptions(f, RANGESET, &optionData);
			MYBYTE *options = NULL;
			cfig.rangeSet[optionData.rangeSetInd].active = true;

			if (optionData.optionSize > 3)
			{
				options = (MYBYTE*)calloc(1, optionData.optionSize);
				memcpy(options, optionData.options, optionData.optionSize);
			}

			for (; m < MAX_DHCP_RANGES && cfig.dhcpRanges[m].rangeStart; m++)
			{
				cfig.dhcpRanges[m].rangeSetInd = optionData.rangeSetInd;
				cfig.dhcpRanges[m].options = options;
				cfig.dhcpRanges[m].mask = optionData.mask;
			}
			cfig.rangeCount = m;
		}
		else
			break;
	}

	//printf("%s\n", IP2String(tempbuff, cfig.mask));

	for (char rangeInd = 0; rangeInd < cfig.rangeCount; rangeInd++) {
		if (!cfig.dhcpRanges[rangeInd].mask)
			cfig.dhcpRanges[rangeInd].mask = cfig.mask;

		for (MYDWORD iip = cfig.dhcpRanges[rangeInd].rangeStart; iip <= cfig.dhcpRanges[rangeInd].rangeEnd; iip++)
		{
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

	ff = fopen(iniFile, "rt");

	if (ff) {
		char sectionName[512];

		while (fgets(sectionName, 510, ff))
		{
			if (*sectionName == '[')
			{
				char *secend = strchr(sectionName, ']');

				if (secend)
				{
					*secend = 0;
					sectionName[0] = NBSP;
					myTrim(sectionName, sectionName);
				}
				else
					continue;
			}
			else
				continue;

			if (!strchr(sectionName, ':'))
				continue;

			//printf("%s\n", sectionName);

			MYBYTE hexValue[UCHAR_MAX];
			MYBYTE hexValueSize = sizeof(hexValue);
			data20 optionData;

			if (strlen(sectionName) <= 48 && !getHexValue(hexValue, sectionName, &hexValueSize))
			{
				if (hexValueSize <= 16)
				{
					dhcpEntry = findDHCPEntry(hex2String(mapname, hexValue, hexValueSize));

					if (!dhcpEntry)
					{
						if (f = openSection(sectionName, 1))
							loadOptions(f, sectionName, &optionData);
						if (f = openSection(sectionName, 1))
							lockOptions(f);

						dhcpMap::iterator p = dhcpCache.begin();

						for (; p != dhcpCache.end(); p++)
						{
							if (p->second && p->second->ip && p->second->ip == optionData.ip)
								break;
						}

						if (p == dhcpCache.end())
						{
							memset(&lump, 0, sizeof(data71));
							lump.mapname = mapname;
							lump.optionSize = optionData.optionSize;
							lump.options = optionData.options;
							dhcpEntry = createCache(&lump);

							if (!dhcpEntry)
								return;
/*
							dhcpEntry = (data7*)calloc(1, sizeof(data7));

							if (!dhcpEntry)
							{
								sprintf(logBuff, "Host Options Load, Memory Allocation Error");
								logDHCPMess(logBuff, 1);
								return;
							}

							dhcpEntry->mapname = cloneString(mapname);

							if (!dhcpEntry->mapname)
							{
								sprintf(logBuff, "Host Data Load, Memory Allocation Error");
								logDHCPMess(logBuff, 1);
								return;
							}
*/
							dhcpEntry->ip = optionData.ip;
							dhcpEntry->rangeInd = getRangeInd(optionData.ip);
							dhcpEntry->fixed = 1;
							lockIP(optionData.ip);
							dhcpCache[dhcpEntry->mapname] = dhcpEntry;
							//printf("%s=%s=%s size=%u %u\n", mapname, dhcpEntry->mapname, IP2String(tempbuff, optionData.ip), optionData.optionSize, dhcpEntry->options);
						}
						else
						{
							sprintf(logBuff, "Static DHCP Host [%s] Duplicate IP Address %s, Entry ignored", sectionName, IP2String(tempbuff, optionData.ip));
							logDHCPMess(logBuff, 1);
						}
					}
					else
					{
						sprintf(logBuff, "Duplicate Static DHCP Host [%s] ignored", sectionName);
						logDHCPMess(logBuff, 1);
					}
				}
				else
				{
					sprintf(logBuff, "Invalid Static DHCP Host MAC Addr size, ignored", sectionName);
					logDHCPMess(logBuff, 1);
				}
			}
			else
			{
				sprintf(logBuff, "Invalid Static DHCP Host MAC Addr [%s] ignored", sectionName);
				logDHCPMess(logBuff, 1);
			}

			if (!optionData.ip)
			{
				sprintf(logBuff, "Warning: No IP Address for DHCP Static Host %s specified", sectionName);
				logDHCPMess(logBuff, 1);
			}
		}

		fclose(ff);
	}

	ff = fopen(leaFile, "rb");

	if (ff)
	{
		data8 dhcpData;

		while (fread(&dhcpData, sizeof(data8), 1, ff))
		{
			char rangeInd = -1;
			int ind = -1;

			// TODO: grab mac here?
			//printf("Loading %s=%s\n", dhcpData.hostname, IP2String(tempbuff, dhcpData.ip));

			if (dhcpData.bp_hlen <= 16 && !findServer(network.allServers, MAX_SERVERS, dhcpData.ip))
			{
				hex2String(mapname, dhcpData.bp_chaddr, dhcpData.bp_hlen);

				dhcpMap::iterator p = dhcpCache.begin();

				for (; p != dhcpCache.end(); p++)
				{
					dhcpEntry = p->second;

					if (dhcpEntry && (!strcasecmp(mapname, dhcpEntry->mapname) || dhcpEntry->ip == dhcpData.ip))
						break;
				}

				if (p != dhcpCache.end() && (strcasecmp(mapname, dhcpEntry->mapname) || dhcpEntry->ip != dhcpData.ip))
					continue;

				dhcpEntry = findDHCPEntry(mapname);
				rangeInd = getRangeInd(dhcpData.ip);

				if(!dhcpEntry && rangeInd >= 0)
				{
					memset(&lump, 0, sizeof(data71));
					lump.mapname = mapname;
					dhcpEntry = createCache(&lump);
/*
					dhcpEntry = (data7*)calloc(1, sizeof(data7));

					if (!dhcpEntry)
					{
						sprintf(logBuff, "Loading Existing Leases, Memory Allocation Error");
						logDHCPMess(logBuff, 1);
						return;
					}

					dhcpEntry->mapname = cloneString(mapname);

					if (!dhcpEntry->mapname)
					{
						sprintf(logBuff, "Loading Existing Leases, Memory Allocation Error");
						free(dhcpEntry);
						logDHCPMess(logBuff, 1);
						return;
					}
*/
				}

				if (dhcpEntry)
				{
					dhcpCache[dhcpEntry->mapname] = dhcpEntry;
					dhcpEntry->ip = dhcpData.ip;
					dhcpEntry->rangeInd = rangeInd;
					dhcpEntry->expiry = dhcpData.expiry;
					dhcpEntry->local = dhcpData.local;
					dhcpEntry->display = true;

					if (dhcpData.hostname[0])
						dhcpEntry->hostname = cloneString(dhcpData.hostname);

					setLeaseExpiry(dhcpEntry);

					// TODO or grab mac here?
					//printf("Loaded %s=%s\n", dhcpData.hostname, IP2String(tempbuff, dhcpData.ip));
				}
			}
		}

		fclose(ff);

		ff = fopen(leaFile, "wb");
		cfig.dhcpInd = 0;

		if (ff)
		{
			dhcpMap::iterator p = dhcpCache.begin();

			for (; p != dhcpCache.end(); p++)
			{
				if ((dhcpEntry = p->second) && (dhcpEntry->expiry > t || !dhcpEntry->fixed))
				{
					memset(&dhcpData, 0, sizeof(data8));
					dhcpData.bp_hlen = 16;
					getHexValue(dhcpData.bp_chaddr, dhcpEntry->mapname, &dhcpData.bp_hlen);
					dhcpData.ip = dhcpEntry->ip;
					dhcpData.expiry = dhcpEntry->expiry;
					dhcpData.local = dhcpEntry->local;

					if (dhcpEntry->hostname)
						strcpy(dhcpData.hostname, dhcpEntry->hostname);

					cfig.dhcpInd++;
					dhcpData.dhcpInd = cfig.dhcpInd;
					dhcpEntry->dhcpInd = cfig.dhcpInd;
					fwrite(&dhcpData, sizeof(data8), 1, ff);
				}
			}
			fclose(ff);
		}
	}
}

FILE *openSection(const char *sectionName, MYBYTE serial) {

  char section[128];
  sprintf(section, "[%s]", sectionName);
  myUpper(section);
  FILE *f = NULL;
  f = fopen(iniFile, "rt");

  if (f) {
    //printf("opened %s=%d\n", tempbuff, f);
    char buff[512];
    MYBYTE found = 0;

    while (fgets(buff, 511, f)) {
      myUpper(buff);
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

	      if (strchr(buff, '\\') || strchr(buff, '/')) strcpy(tempbuff, buff);
	      else sprintf(tempbuff, "%s%s", filePATH, buff);

	      f = fopen(tempbuff, "rt");

   	      if (f) return f;
	      else {
		sprintf(logBuff, "Error: Section [%s], file %s not found", sectionName, tempbuff);
		logDHCPMess(logBuff, 1);
		return NULL;
	      }
	    } else {
	      fseek(f, fpos, SEEK_SET);
	      return f;
	    }
	  }
	}
      }
    }
    fclose(f);
  }
  return NULL;
}

char *readSection(char* buff, FILE *f) {
  while (fgets(buff, 511, f)) {
    myTrim(buff, buff);
    if (*buff == '[') break;
    if ((*buff) >= '0' && (*buff) <= '9' || (*buff) >= 'A' && (*buff) <= 'Z' || (*buff) >= 'a' && (*buff) <= 'z' || ((*buff) && strchr("/\\?*", (*buff)))) return buff;
  }

  fclose(f);
  return NULL;
}

char* myGetToken(char* buff, MYBYTE index) {
  while (*buff) {
    if (index) index--;
    else break;

    buff += strlen(buff) + 1;
  }
  return buff;
}

MYWORD myTokenize(char *target, char *source, const char *sep, bool whiteSep) {
  bool found = true;
  char *dp = target;
  MYWORD kount = 0;

  while (*source) {
    if (sep && sep[0] && strchr(sep, (*source))) {
      found = true;
      source++;
      continue;
    } else if (whiteSep && (*source) <= NBSP) {
      found = true;
      source++;
      continue;
    }
    if (found) {
      if (target != dp) { *dp = 0; dp++; }
      kount++;
    }
    found = false; *dp = *source; dp++; source++;
  }

  *dp = 0; dp++; *dp = 0;
  //printf("%s\n", target);
  return kount;
}

char* myTrim(char *target, char *source) {
  while ((*source) && (*source) <= NBSP) source++;
  int i = 0;
  for (; i < 511 && source[i]; i++) target[i] = source[i];
  target[i] = source[i];
  i--;
  for (; i >= 0 && target[i] <= NBSP; i--) target[i] = 0;
  return target;
}

void mySplit(char *name, char *value, char *source, char splitChar) {
  int i = 0;
  int j = 0;
  int k = 0;

  for (; source[i] && j <= 510 && source[i] != splitChar; i++, j++) {
    name[j] = source[i];
  }

  if (source[i]) {
    i++;
    for (; k <= 510 && source[i]; i++, k++) {
      value[k] = source[i];
    }
  }

  name[j] = 0;
  value[k] = 0;

  myTrim(name, name);
  myTrim(value, value);
  //printf("%s %s\n", name, value);
}


char *strqtype(char *buff, MYBYTE qtype)
{
	switch (qtype)
	{
		case 1:
			strcpy(buff, "A");
			break;
		case 2:
			strcpy(buff, "NS");
			break;
		case 3:
			strcpy(buff, "MD");
			break;
		case 4:
			strcpy(buff, "MF");
			break;
		case 5:
			strcpy(buff, "CNAME");
			break;
		case 6:
			strcpy(buff, "SOA");
			break;
		case 7:
			strcpy(buff, "MB");
			break;
		case 8:
			strcpy(buff, "MG");
			break;
		case 9:
			strcpy(buff, "MR");
			break;
		case 10:
			strcpy(buff, "NULL");
			break;
		case 11:
			strcpy(buff, "WKS");
			break;
		case 12:
			strcpy(buff, "PTR");
			break;
		case 13:
			strcpy(buff, "HINFO");
			break;
		case 14:
			strcpy(buff, "MINFO");
			break;
		case 15:
			strcpy(buff, "MX");
			break;
		case 16:
			strcpy(buff, "TXT");
			break;
		case 28:
			strcpy(buff, "AAAA");
			break;
		case 251:
			strcpy(buff, "IXFR");
			break;
		case 252:
			strcpy(buff, "AXFR");
			break;
		case 253:
			strcpy(buff, "MAILB");
			break;
		case 254:
			strcpy(buff, "MAILA");
			break;
		default:
			strcpy(buff, "ANY");
			break;
	}
	return buff;
}

MYDWORD getClassNetwork(MYDWORD ip)
{
	data15 data;
	data.ip = ip;
	data.octate[3] = 0;

	if (data.octate[0] < 192)
		data.octate[2] = 0;

	if (data.octate[0] < 128)
		data.octate[1] = 0;

	return data.ip;
}

char *IP2String(char *target, MYDWORD ip)
{
	data15 inaddr;
	inaddr.ip = ip;
	sprintf(target, "%u.%u.%u.%u", inaddr.octate[0], inaddr.octate[1], inaddr.octate[2], inaddr.octate[3]);
	//MYBYTE *octate = (MYBYTE*)&ip;
	//sprintf(target, "%u.%u.%u.%u", octate[0], octate[1], octate[2], octate[3]);
	return target;
}

MYDWORD *addServer(MYDWORD *array, MYBYTE maxServers, MYDWORD ip)
{
	for (MYBYTE i = 0; i < maxServers; i++)
	{
		if (array[i] == ip)
			return &(array[i]);
		else if (!array[i])
		{
			array[i] = ip;
			return &(array[i]);
		}
	}
	return NULL;
}

MYDWORD *findServer(MYDWORD *array, MYBYTE kount, MYDWORD ip)
{
	if (ip)
	{
		for (MYBYTE i = 0; i < kount && array[i]; i++)
		{
			if (array[i] == ip)
				return &(array[i]);
		}
	}
	return 0;
}

bool isInt(char *str)
{
	if (!str || !(*str))
		return false;

	for(; *str; str++)
		if (*str <  '0' || *str > '9')
			return false;

	return true;
}

bool isIP(char *str)
{
	if (!str || !(*str))
		return false;

	MYDWORD ip = inet_addr(str);
	int j = 0;

	for (; *str; str++)
	{
		if (*str == '.' && *(str + 1) != '.')
			j++;
		else if (*str < '0' || *str > '9')
			return false;
	}

	if (j == 3)
	 {
		if (ip == INADDR_NONE || ip == INADDR_ANY)
			return false;
		else
			return true;
	}
	else
		return false;
}

char *hex2String(char *target, MYBYTE *hex, MYBYTE bytes)
{
	char *dp = target;

	if (bytes)
		dp += sprintf(target, "%02x", *hex);
	else
		*target = 0;

	for (MYBYTE i = 1; i < bytes; i++)
			dp += sprintf(dp, ":%02x", *(hex + i));

	return target;
}

char *genHostName(char *target, MYBYTE *hex, MYBYTE bytes)
{
	char *dp = target;

	if (bytes)
		dp += sprintf(target, "Host%02x", *hex);
	else
		*target = 0;

	for (MYBYTE i = 1; i < bytes; i++)
			dp += sprintf(dp, "%02x", *(hex + i));

	return target;
}

char *IP62String(char *target, MYBYTE *source)
{
	char *dp = target;
	bool zerostarted = false;
	bool zeroended = false;

	for (MYBYTE i = 0; i < 16; i += 2, source += 2)
	{
		if (source[0])
		{
			if (zerostarted)
				zeroended = true;

			if (zerostarted && zeroended)
			{
				dp += sprintf(dp, "::");
				zerostarted = false;
			}
			else if (dp != target)
				dp += sprintf(dp, ":");

			dp += sprintf(dp, "%x", source[0]);
			dp += sprintf(dp, "%02x", source[1]);
		}
		else if (source[1])
		{
			if (zerostarted)
				zeroended = true;

			if (zerostarted && zeroended)
			{
				dp += sprintf(dp, "::");
				zerostarted = false;
			}
			else if (dp != target)
				dp += sprintf(dp, ":");

			dp += sprintf(dp, "%0x", source[1]);
		}
		else if (!zeroended)
			zerostarted = true;
	}

	return target;
}

char *getHexValue(MYBYTE *target, char *source, MYBYTE *size)
{
	if (*size)
		memset(target, 0, (*size));

	for ((*size) = 0; (*source) && (*size) < UCHAR_MAX; (*size)++, target++)
	{
		if ((*source) >= '0' && (*source) <= '9')
		{
			(*target) = (*source) - '0';
		}
		else if ((*source) >= 'a' && (*source) <= 'f')
		{
			(*target) = (*source) - 'a' + 10;
		}
		else if ((*source) >= 'A' && (*source) <= 'F')
		{
			(*target) = (*source) - 'A' + 10;
		}
		else
		{
			return source;
		}

		source++;

		if ((*source) >= '0' && (*source) <= '9')
		{
			(*target) *= 16;
			(*target) += (*source) - '0';
		}
		else if ((*source) >= 'a' && (*source) <= 'f')
		{
			(*target) *= 16;
			(*target) += (*source) - 'a' + 10;
		}
		else if ((*source) >= 'A' && (*source) <= 'F')
		{
			(*target) *= 16;
			(*target) += (*source) - 'A' + 10;
		}
		else if ((*source) == ':' || (*source) == '-')
		{
			source++;
			continue;
		}
		else if (*source)
		{
			return source;
		}
		else
		{
			continue;
		}

		source++;

		if ((*source) == ':' || (*source) == '-')
		{
			source++;
		}
		else if (*source)
			return source;
	}

	if (*source)
		return source;

	return NULL;
}

char *myUpper(char *string) {
  char diff = 'a' - 'A';
  MYWORD len = strlen(string);
  for (int i = 0; i < len; i++)
    if (string[i] >= 'a' && string[i] <= 'z') string[i] -= diff;
  return string;
}

char *myLower(char *string) {
  char diff = 'a' - 'A';
  MYWORD len = strlen(string);
  for (int i = 0; i < len; i++)
    if (string[i] >= 'A' && string[i] <= 'Z') string[i] += diff;
  return string;
}

bool wildcmp(char *string, char *wild) {
  // Written by Jack Handy - jakkhandy@hotmail.com
  // slightly modified
  char *cp = NULL;
  char *mp = NULL;

  while ((*string) && (*wild != '*')) {
    if ((*wild != *string) && (*wild != '?')) {
      return 0;
    }
    wild++;
    string++;
  }

  while (*string) {
    if (*wild == '*') {
      if (!*++wild) return 1;
      mp = wild;
      cp = string + 1;
    } else if ((*wild == *string) || (*wild == '?')) {
      wild++;
      string++;
    } else {
      wild = mp;
      string = cp++;
    }
  }

  while (*wild == '*') wild++;
  return !(*wild);
}

void __cdecl updateStateFile(void *lpParam) {

  data7 *dhcpEntry = (data7*)lpParam;
  data8 dhcpData;
  memset(&dhcpData, 0, sizeof(data8));
  dhcpData.bp_hlen = 16;
  getHexValue(dhcpData.bp_chaddr, dhcpEntry->mapname, &dhcpData.bp_hlen);
  dhcpData.ip = dhcpEntry->ip;
  dhcpData.expiry = dhcpEntry->expiry;
  dhcpData.local = dhcpEntry->local;

  if (dhcpEntry->hostname) strcpy(dhcpData.hostname, dhcpEntry->hostname);

  WaitForSingleObject(fEvent, INFINITE);

  if (dhcpEntry->dhcpInd) {
    dhcpData.dhcpInd = dhcpEntry->dhcpInd;
    FILE *f = fopen(leaFile, "rb+");

    if (f) {
      if (fseek(f, (dhcpData.dhcpInd - 1)*sizeof(data8), SEEK_SET) >= 0)
        fwrite(&dhcpData, sizeof(data8), 1, f);
      fclose(f);
    }
  } else {
    cfig.dhcpInd++;
    dhcpEntry->dhcpInd = cfig.dhcpInd;
    dhcpData.dhcpInd = cfig.dhcpInd;
    FILE *f = fopen(leaFile, "ab");

    if (f) {
      fwrite(&dhcpData, sizeof(data8), 1, f);
      fclose(f);
    }
  }

  SetEvent(fEvent);
  _endthread();
  return;
}

void calcRangeLimits(MYDWORD ip, MYDWORD mask, MYDWORD *rangeStart, MYDWORD *rangeEnd) {
  *rangeStart = htonl(ip & mask) + 1;
  *rangeEnd = htonl(ip | (~mask)) - 1;
}

data7 *findDHCPEntry(char *key) {
  //printf("finding %u=%s\n",ind,key);
  myLower(key);
  dhcpMap::iterator it = dhcpCache.find(key);

  if (it == dhcpCache.end()) return NULL;
  else return it->second;
}

bool checkMask(MYDWORD mask) {
  mask = htonl(mask);
  while (mask) {
    if (mask < (mask << 1)) return false;
    mask <<= 1;
  }
  return true;
}

MYDWORD calcMask(MYDWORD rangeStart, MYDWORD rangeEnd) {
  data15 ip1, ip2, mask;

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

char *cloneString(char *string) {
  if (!string) return NULL;
  char *s = (char*) calloc(1, strlen(string) + 1);
  if (s) { strcpy(s, string); }
  return s;
}

void __cdecl init(void *lpParam) {

  FILE *f = NULL;
  char raw[512];
  char name[512];
  char value[512];
  char *fileExt;

  memset(&cfig, 0, sizeof(cfig));
  memset(&network, 0, sizeof(network));

  GetModuleFileName(NULL, filePATH, _MAX_PATH);
  strcpy(cfilePATH, config.cfgfn);

  fileExt = strrchr(filePATH, '.'); *fileExt = 0;
  fileExt = strrchr(cfilePATH, '.'); *fileExt = 0;

  // store the following adjacent to config..

  sprintf(iniFile, "%s.ini", cfilePATH);
  sprintf(lnkFile, "%s.url", cfilePATH);
  sprintf(htmFile, "%s.htm", cfilePATH);
  sprintf(leaFile, "%s.state", cfilePATH);

  // if module path and config path are not the same 
  // put the state/log information into directories
  // under the parent otherwise put them under module path

  if (strcmp(filePATH, cfilePATH) != 0) {
    fileExt = strrchr(filePATH, '\\');
    *fileExt = 0;
    fileExt = strrchr(filePATH, '\\');
    *fileExt = 0;
    sprintf(logFile, "%s\\log\\%s-%%Y%%m%%d.log", filePATH, NAME);
    sprintf(cliFile, "%s\\log\\%%s.log", filePATH);
  } else {
    fileExt = strrchr(filePATH, '\\');
    *fileExt = 0;
    sprintf(logFile, "%s\\%s-%%Y%%m%%d.log", filePATH, NAME);
    sprintf(cliFile, "%s\\%%s.log", filePATH);
  }

  cfig.dhcpLogLevel = config.logging;

  sprintf(logBuff, "Status: Running");
  logDHCPMess(logBuff, 1);

  MYWORD wVersionRequested = MAKEWORD(1, 1);
  WSAStartup(wVersionRequested, &cfig.wsaData);

  if (cfig.wsaData.wVersion != wVersionRequested) {
    sprintf(logBuff, "WSAStartup Error");
    logDHCPMess(logBuff, 1);
  }

  loadDHCP();

  cfig.lease = 36000;

  for (int i = 0; i < cfig.rangeCount; i++) {
    char *logPtr = logBuff;
    logPtr += sprintf(logPtr, "DHCP Range: ");
    logPtr += sprintf(logPtr, "%s", IP2String(tempbuff, htonl(cfig.dhcpRanges[i].rangeStart)));
    logPtr += sprintf(logPtr, "-%s", IP2String(tempbuff, htonl(cfig.dhcpRanges[i].rangeEnd)));
    logPtr += sprintf(logPtr, "/%s", IP2String(tempbuff, cfig.dhcpRanges[i].mask));
    logDHCPMess(logBuff, 1);
  }

  FIXED_INFO *FixedInfo;
  IP_ADDR_STRING *pIPAddr;

  FixedInfo = (FIXED_INFO*) GlobalAlloc(GPTR, sizeof(FIXED_INFO));
  DWORD ulOutBufLen = sizeof(FIXED_INFO);

  if (ERROR_BUFFER_OVERFLOW == GetNetworkParams(FixedInfo, &ulOutBufLen)) {
    GlobalFree(FixedInfo);
    FixedInfo = (FIXED_INFO*)GlobalAlloc(GPTR, ulOutBufLen);
  }

  if (!GetNetworkParams(FixedInfo, &ulOutBufLen))
    strcpy(cfig.servername, FixedInfo->HostName);

  fEvent = CreateEvent(
    NULL,   // default security descriptor
    FALSE,  // ManualReset
    TRUE,   // Signalled
    TEXT("AchalOpenDHCPFileEvent"));  // object name

  if (fEvent == NULL) {
    printf("CreateEvent error: %d\n", GetLastError());
    exit(-1);
  } else if ( GetLastError() == ERROR_ALREADY_EXISTS ) {
    sprintf(logBuff, "CreateEvent opened an existing Event\nServer May already be Running");
    logDHCPMess(logBuff, 0);
    exit(-1);
  }
  //SetEvent(fEvent);

  lEvent = CreateEvent(
    NULL,                  // default security descriptor
    FALSE,                 // ManualReset
    TRUE,                  // Signalled
    TEXT("AchalOpenDHCPLogEvent"));  // object name

  if (lEvent == NULL) {
    printf("CreateEvent error: %d\n", GetLastError());
    exit(-1);
  } else if ( GetLastError() == ERROR_ALREADY_EXISTS ) {
    sprintf(logBuff, "CreateEvent opened an existing Event\nServer May already be Running");
    logDHCPMess(logBuff, 0);
    exit(-1);
  }

  getInterfaces(&network);

  if (f = openSection("REPLICATION_SERVERS", 1)) {
    while (readSection(raw, f)) {

      mySplit(name, value, raw, '=');

      if (name[0] && value[0]) {

        if (!isIP(name) && isIP(value)) {

	  if (!strcasecmp(name, "Primary"))
	    cfig.zoneServers[0] = inet_addr(value);
	  else if (!strcasecmp(name, "Secondary"))
	    cfig.zoneServers[1] = inet_addr(value);
	  else {
	    sprintf(logBuff, "Section [REPLICATION_SERVERS] Invalid Entry: %s ignored", raw);
	    logDHCPMess(logBuff, 1);
  	  }
	} else {
	  sprintf(logBuff, "Section [REPLICATION_SERVERS] Invalid Entry: %s ignored", raw);
	  logDHCPMess(logBuff, 1);
	}
      } else {
	sprintf(logBuff, "Section [REPLICATION_SERVERS], Missing value, entry %s ignored", raw);
	logDHCPMess(logBuff, 1);
      }

    }
  }

  if (!cfig.zoneServers[0] && cfig.zoneServers[1]) {
    sprintf(logBuff, "Section [REPLICATION_SERVERS] Missing Primary Server");
    logDHCPMess(logBuff, 1);
  } else if (cfig.zoneServers[0] && !cfig.zoneServers[1]) {
    sprintf(logBuff, "Section [REPLICATION_SERVERS] Missing Secondary Server");
    logDHCPMess(logBuff, 1);
  } else if (cfig.zoneServers[0] && cfig.zoneServers[1]) {

    if (findServer(network.staticServers, MAX_SERVERS, cfig.zoneServers[0]) && findServer(network.staticServers, MAX_SERVERS, cfig.zoneServers[1])) {
      sprintf(logBuff, "Section [REPLICATION_SERVERS] Primary & Secondary should be Different Boxes");
      logDHCPMess(logBuff, 1);
    } else if (findServer(network.staticServers, MAX_SERVERS, cfig.zoneServers[0])) cfig.replication = 1;
    else if (findServer(network.staticServers, MAX_SERVERS, cfig.zoneServers[1])) cfig.replication = 2;
    else {
      sprintf(logBuff, "Section [REPLICATION_SERVERS] No Server IP not found on this Machine");
      logDHCPMess(logBuff, 1);
    }
  }

  if (cfig.replication) {
    lockIP(cfig.zoneServers[0]);
    lockIP(cfig.zoneServers[1]);

    cfig.dhcpReplConn.sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (cfig.dhcpReplConn.sock == INVALID_SOCKET) {
      sprintf(logBuff, "Failed to Create DHCP Replication Socket");
			logDHCPMess(logBuff, 1);
		}
		else
		{
			//printf("Socket %u\n", cfig.dhcpReplConn.sock);

			if (cfig.replication == 1)
				cfig.dhcpReplConn.server = cfig.zoneServers[0];
			else
				cfig.dhcpReplConn.server = cfig.zoneServers[1];

			cfig.dhcpReplConn.addr.sin_family = AF_INET;
			cfig.dhcpReplConn.addr.sin_addr.s_addr = cfig.dhcpReplConn.server;
			cfig.dhcpReplConn.addr.sin_port = 0;

			int nRet = bind(cfig.dhcpReplConn.sock, (sockaddr*)&cfig.dhcpReplConn.addr, sizeof(struct sockaddr_in));

			if (nRet == SOCKET_ERROR)
			{
				cfig.dhcpReplConn.ready = false;
				sprintf(logBuff, "DHCP Replication Server, Bind Failed");
				logDHCPMess(logBuff, 1);
			}
			else
			{
				cfig.dhcpReplConn.port = IPPORT_DHCPS;
				cfig.dhcpReplConn.loaded = true;
				cfig.dhcpReplConn.ready = true;

				data3 op;
				memset(&token, 0, sizeof(data9));
				token.vp = token.dhcpp.vend_data;
				token.messsize = sizeof(dhcp_packet);

				token.dhcpp.header.bp_op = BOOTP_REQUEST;
				token.dhcpp.header.bp_xid = t;
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

				if (cfig.replication == 1)
					token.remote.sin_addr.s_addr = cfig.zoneServers[1];
				else
					token.remote.sin_addr.s_addr = cfig.zoneServers[0];

				if (cfig.replication == 2)
					_beginthread(sendToken, 0, 0);
			}
		}
	}

	if (cfig.replication == 1)
		sprintf(logBuff, "Server Name: %s (Primary)", cfig.servername);
	else if (cfig.replication == 2)
		sprintf(logBuff, "Server Name: %s (Secondary)", cfig.servername);
	else
		sprintf(logBuff, "Server Name: %s", cfig.servername);

	logDHCPMess(logBuff, 1);

// TODO: change this.. since we know already what interface we want to listen on and setup to be static already

	sprintf(logBuff, "Detecting Static Interfaces..");
	logDHCPMess(logBuff, 1);

	do {
	  closeConn();
	  getInterfaces(&network);

	  if (cfig.dhcpReplConn.ready && network.maxFD < cfig.dhcpReplConn.sock)
	    network.maxFD = cfig.dhcpReplConn.sock;

		bool ifSpecified = false;
		bool bindfailed = false;

		if (f = openSection("LISTEN_ON", 1))
		{
			//char *name = myGetToken(iniStr, 0)
			int i = 0;

			while (readSection(raw, f))
			{
				if(i < MAX_SERVERS)
				{
					ifSpecified = true;
					MYDWORD addr = inet_addr(raw);

					if (isIP(raw))
					{
						for (MYBYTE m = 0; ; m++)
						{
							if (m >= MAX_SERVERS || !network.staticServers[m])
							{
								if (findServer(network.allServers, MAX_SERVERS, addr))
								{
									sprintf(logBuff, "Warning: Section [LISTEN_ON], Interface %s is not Static, ignored", raw);
									logDHCPMess(logBuff, 1);
								}
								else
								{
									bindfailed = true;
									sprintf(logBuff, "Warning: Section [LISTEN_ON], Interface %s not available, ignored", raw);
									logDHCPMess(logBuff, 1);
								}
								break;
							}
							else if (network.staticServers[m] == addr)
							{
								for (MYBYTE n = 0; n < MAX_SERVERS; n++)
								{
									if (network.listenServers[n] == addr)
										break;
									else if (!network.listenServers[n])
									{
										network.listenServers[n] = network.staticServers[m];
										network.listenMasks[n] = network.staticMasks[m];
										break;
									}
								}
								break;
							}
						}
					}
					else
					{
						sprintf(logBuff, "Warning: Section [LISTEN_ON], Invalid Interface Address %s, ignored", raw);
						logDHCPMess(logBuff, 1);
					}
				}
			}
		}

		if (!ifSpecified)
		{
			MYBYTE k = 0;

			for (MYBYTE m = 0; m < MAX_SERVERS && network.allServers[m]; m++)
			{
				for (MYBYTE n = 0; n < MAX_SERVERS; n++)
				{
					if (network.allServers[m] == network.staticServers[n])
					{
						network.listenServers[k] = network.staticServers[n];
						network.listenMasks[k] = network.staticMasks[n];
						k++;
						break;
					}
					else if (!network.staticServers[n])
					{
						sprintf(logBuff, "Warning: Interface %s is not Static, not used", IP2String(tempbuff, network.allServers[m]));
						logDHCPMess(logBuff, 2);
						break;
					}
				}
			}
		}

		int i = 0;

		for (int j = 0; j < MAX_SERVERS && network.listenServers[j]; j++)
		{
			network.dhcpConn[i].sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

			if (network.dhcpConn[i].sock == INVALID_SOCKET)
			{
				bindfailed = true;
				sprintf(logBuff, "Failed to Create Socket");
				logDHCPMess(logBuff, 1);
				continue;
			}

			//printf("Socket %u\n", network.dhcpConn[i].sock);

			network.dhcpConn[i].addr.sin_family = AF_INET;
			network.dhcpConn[i].addr.sin_addr.s_addr = network.listenServers[j];
			network.dhcpConn[i].addr.sin_port = htons(IPPORT_DHCPS);

			network.dhcpConn[i].broadCastVal = TRUE;
			network.dhcpConn[i].broadCastSize = sizeof(network.dhcpConn[i].broadCastVal);
			setsockopt(network.dhcpConn[i].sock, SOL_SOCKET, SO_BROADCAST, (char*)(&network.dhcpConn[i].broadCastVal), network.dhcpConn[i].broadCastSize);

			int nRet = bind(network.dhcpConn[i].sock,
							(sockaddr*)&network.dhcpConn[i].addr,
							sizeof(struct sockaddr_in)
						   );

			if (nRet == SOCKET_ERROR)
			{
				bindfailed = true;
				closesocket(network.dhcpConn[i].sock);
				sprintf(logBuff, "Warning: %s UDP Port 67 already in use", IP2String(tempbuff, network.listenServers[j]));
				logDHCPMess(logBuff, 1);
				continue;
			}

			network.dhcpConn[i].loaded = true;
			network.dhcpConn[i].ready = true;

			if (network.maxFD < network.dhcpConn[i].sock)
				network.maxFD = network.dhcpConn[i].sock;

			network.dhcpConn[i].server = network.listenServers[j];
			network.dhcpConn[i].mask = network.listenMasks[j];
			network.dhcpConn[i].port = IPPORT_DHCPS;

			i++;
		}

    //TODO config.http port / hostname
		network.httpConn.port = 6789;
		network.httpConn.server = inet_addr("127.0.0.1");
		network.httpConn.loaded = true;

		if (f = openSection("HTTP_INTERFACE", 1))
		{
			while (readSection(raw, f))
			{
				mySplit(name, value, raw, '=');

				if (!strcasecmp(name, "HTTPServer"))
				{
					mySplit(name, value, value, ':');

					if (isIP(name))
					{
						network.httpConn.loaded = true;
						network.httpConn.server = inet_addr(name);
					}
					else
					{
						network.httpConn.loaded = false;
						sprintf(logBuff, "Warning: Section [HTTP_INTERFACE], Invalid IP Address %s, ignored", name);
						logDHCPMess(logBuff, 1);
					}

					if (value[0])
					{
						if (atoi(value))
							network.httpConn.port = atoi(value);
						else
						{
							network.httpConn.loaded = false;
							sprintf(logBuff, "Warning: Section [HTTP_INTERFACE], Invalid port %s, ignored", value);
							logDHCPMess(logBuff, 1);
						}
					}

					if (network.httpConn.server != inet_addr("127.0.0.1") && !findServer(network.allServers, MAX_SERVERS, network.httpConn.server))
					{
						bindfailed = true;
						network.httpConn.loaded = false;
						sprintf(logBuff, "Warning: Section [HTTP_INTERFACE], %s not available, ignored", raw);
						logDHCPMess(logBuff, 1);
					}
				}
				else if (!strcasecmp(name, "HTTPClient"))
				{
					if (isIP(value))
						addServer(cfig.httpClients, 8, inet_addr(value));
					else
					{
						sprintf(logBuff, "Warning: Section [HTTP_INTERFACE], invalid client IP %s, ignored", raw);
						logDHCPMess(logBuff, 1);
					}
				}
				else if (!strcasecmp(name, "HTTPTitle"))
				{
					strncpy(htmlTitle, value, 255);
					htmlTitle[255] = 0;
				}
				else
				{
					sprintf(logBuff, "Warning: Section [HTTP_INTERFACE], invalid entry %s, ignored", raw);
					logDHCPMess(logBuff, 1);
				}
			}
		}

		if (htmlTitle[0] == 0)
			sprintf(htmlTitle, "Open DHCP on %s", cfig.servername);

		network.httpConn.sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (network.httpConn.sock == INVALID_SOCKET) {
			bindfailed = true;
			sprintf(logBuff, "Failed to Create Socket");
			logDHCPMess(logBuff, 1);
		} else {
			//printf("Socket %u\n", network.httpConn.sock);

			network.httpConn.addr.sin_family = AF_INET;
			network.httpConn.addr.sin_addr.s_addr = network.httpConn.server;
			network.httpConn.addr.sin_port = htons(network.httpConn.port);

			int nRet = bind(network.httpConn.sock,
							(sockaddr*)&network.httpConn.addr,
							sizeof(struct sockaddr_in));

			if (nRet == SOCKET_ERROR) {
				bindfailed = true;
				sprintf(logBuff, "Http Interface %s TCP Port %u not available", IP2String(tempbuff, network.httpConn.server), network.httpConn.port);
				logDHCPMess(logBuff, 1);
				closesocket(network.httpConn.sock);
			} else {
				nRet = listen(network.httpConn.sock, SOMAXCONN);

				if (nRet == SOCKET_ERROR) {
					bindfailed = true;
					sprintf(logBuff, "%s TCP Port %u Error on Listen", IP2String(tempbuff, network.httpConn.server), network.httpConn.port);
					logDHCPMess(logBuff, 1);
					closesocket(network.httpConn.sock);
				} else {
					network.httpConn.loaded = true;
					network.httpConn.ready = true;

					if (network.httpConn.sock > network.maxFD)
						network.maxFD = network.httpConn.sock;
				}
			}
		}

		network.maxFD++;

		for (MYBYTE m = 0; m < MAX_SERVERS && network.allServers[m]; m++)
			lockIP(network.allServers[m]);

		if (bindfailed)
			cfig.failureCount++;
		else
			cfig.failureCount = 0;


		//printf("%i %i %i\n", network.dhcpConn[0].ready, network.dnsUdpConn[0].ready, network.dnsTcpConn[0].ready);

		if (!network.dhcpConn[0].ready) {
			sprintf(logBuff, "No Static Interface ready, Waiting...");
			logDHCPMess(logBuff, 1);
			continue;
		}

		if (network.httpConn.ready) {
			sprintf(logBuff, "Lease Status URL: http://%s:%u", IP2String(tempbuff, network.httpConn.server), network.httpConn.port);
			logDHCPMess(logBuff, 1);
			FILE *f = fopen(htmFile, "wt");

			if (f) {
				fprintf(f, "<html><head><meta http-equiv=\"refresh\" content=\"0;url=http://%s:%u\"</head></html>", IP2String(tempbuff, network.httpConn.server), network.httpConn.port);
				fclose(f);
			}
		} else {
			FILE *f = fopen(htmFile, "wt");

			if (f) {
				fprintf(f, "<html><body><h2>DHCP/HTTP Service is not running</h2></body></html>");
				fclose(f);
			}
		}

		for (int i = 0; i < MAX_SERVERS && network.staticServers[i]; i++) {
			for (MYBYTE j = 0; j < MAX_SERVERS; j++) {
				if (network.dhcpConn[j].server == network.staticServers[i]) {
					sprintf(logBuff, "Listening On: %s", IP2String(tempbuff, network.staticServers[i]));
					logDHCPMess(logBuff, 1);
					break;
				}
			}
		}

	} while (detectChange() && dhcp_running);

	_endthread();
	return;
}

bool detectChange() {

  //sprintf(logBuff, "Calling detectChange()");
  //logDHCPMess(logBuff, 1);

  network.ready = true;

  if (cfig.failureCount) {
    MYDWORD eventWait = (MYDWORD)(10000 * pow(2, cfig.failureCount));
    Sleep(eventWait);
    sprintf(logBuff, "Retrying failed Listening Interfaces..");
    logDHCPMess(logBuff, 1);
    network.ready = false;

    while (network.busy) Sleep(1000);

    return true;
  }

  OVERLAPPED overlap;
  MYDWORD ret;
  HANDLE hand = NULL;
  overlap.hEvent = WSACreateEvent();

  ret = NotifyAddrChange(&hand, &overlap);

  if (ret != NO_ERROR) {
    if (WSAGetLastError() != WSA_IO_PENDING) {
      WSACloseEvent(overlap.hEvent);
      Sleep(1000);
      return true;
    }
  }

  if (WaitForSingleObject(overlap.hEvent, UINT_MAX) == WAIT_OBJECT_0)
    WSACloseEvent(overlap.hEvent);

  network.ready = false;

  while (network.busy) Sleep(1000);

  sprintf(logBuff, "Network changed, re-detecting Static Interfaces..");
  logDHCPMess(logBuff, 1);

  return true;
}

void getInterfaces(data1 *network) {

  memset(network, 0, sizeof(data1));

  SOCKET sd = WSASocket(PF_INET, SOCK_DGRAM, 0, 0, 0, 0);

  if (sd == INVALID_SOCKET) return;

  INTERFACE_INFO InterfaceList[MAX_SERVERS];
  unsigned long nBytesReturned;

  if (WSAIoctl(sd, SIO_GET_INTERFACE_LIST, 0, 0, &InterfaceList,
    sizeof(InterfaceList), &nBytesReturned, 0, 0) == SOCKET_ERROR) return;

  int nNumInterfaces = nBytesReturned / sizeof(INTERFACE_INFO);

  for (int i = 0; i < nNumInterfaces; ++i) {
    sockaddr_in *pAddress = (sockaddr_in*)&(InterfaceList[i].iiAddress);
    u_long nFlags = InterfaceList[i].iiFlags;
    //if (!((nFlags & IFF_POINTTOPOINT)))
    if (!((nFlags & IFF_POINTTOPOINT) || (nFlags & IFF_LOOPBACK))) {
      //printf("%s\n", IP2String(tempbuff, pAddress->sin_addr.S_un.S_addr));
      addServer(network->allServers, MAX_SERVERS, pAddress->sin_addr.s_addr);
    }
  }

  closesocket(sd);

  network->staticServers[0] = inet_addr(config.adptrip);
  network->staticMasks[0] = inet_addr(config.netmask);

  DWORD ulOutBufLen = sizeof(IP_ADAPTER_INFO);

/*
  PIP_ADAPTER_INFO pAdapterInfo;
  PIP_ADAPTER_INFO pAdapter;

  pAdapterInfo = (IP_ADAPTER_INFO*) calloc(1, sizeof(IP_ADAPTER_INFO));

  if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
    free(pAdapterInfo);
    pAdapterInfo = (IP_ADAPTER_INFO*)calloc(1, ulOutBufLen);
  }

  printf("here!");

  if ((GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
    pAdapter = pAdapterInfo;
    while (pAdapter) {
      if (!strcmp(pAdapter->Description, config.ifname)) {
        IP_ADDR_STRING *sList = &pAdapter->IpAddressList;
        while (sList) {
          DWORD iaddr = inet_addr(sList->IpAddress.String);
    	  if (iaddr) {
  	    for (MYBYTE k = 0; k < MAX_SERVERS; k++) {
  		printf("%d %d\r\n", k, iaddr);
	      if (network->staticServers[k] == iaddr) break;
	      else if (!network->staticServers[k]) {
	        network->staticServers[k] = iaddr;
	        network->staticMasks[k] = inet_addr(sList->IpMask.String);
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
*/

  FIXED_INFO *FixedInfo;
  IP_ADDR_STRING *pIPAddr;

  FixedInfo = (FIXED_INFO*)GlobalAlloc(GPTR, sizeof(FIXED_INFO));
  ulOutBufLen = sizeof(FIXED_INFO);

  if (ERROR_BUFFER_OVERFLOW == GetNetworkParams(FixedInfo, &ulOutBufLen)) {
    GlobalFree(FixedInfo);
    FixedInfo = (FIXED_INFO*)GlobalAlloc(GPTR, ulOutBufLen);
  }

  if (!GetNetworkParams(FixedInfo, &ulOutBufLen)) {
    if (!cfig.servername[0]) strcpy(cfig.servername, FixedInfo->HostName);
      //printf("d=%u=%s", strlen(FixedInfo->DomainName), FixedInfo->DomainName);
      GlobalFree(FixedInfo);
  }

  return;
}

MYWORD gdmess(data9 *req, MYBYTE sockInd) {
	//debugl("gdmess");
	memset(req, 0, sizeof(data9));
	req->sockInd = sockInd;
	req->sockLen = sizeof(req->remote);
	errno = 0;

	req->bytes = recvfrom(network.dhcpConn[req->sockInd].sock,
	                      req->raw,
	                      sizeof(req->raw),
	                      0,
	                      (sockaddr*)&req->remote,
	                      &req->sockLen);

	//printf("IP=%s bytes=%u\n", IP2String(tempbuff,req->remote.sin_addr.s_addr), req->bytes);

	errno = WSAGetLastError();

	//printf("errno=%u\n", errno);

	if (errno || req->bytes <= 0 || req->dhcpp.header.bp_op != BOOTP_REQUEST)
		return 0;

	hex2String(req->chaddr, req->dhcpp.header.bp_chaddr, req->dhcpp.header.bp_hlen);

	data3 *op;
	MYBYTE *raw = req->dhcpp.vend_data;
	MYBYTE *rawEnd = raw + (req->bytes - sizeof(dhcp_header));
	MYBYTE maxInd = sizeof(opData) / sizeof(data4);

	for (; raw < rawEnd && *raw != DHCP_OPTION_END;)
	{
		op = (data3*)raw;

		switch (op->opt_code)
		{

			case DHCP_OPTION_PAD:
				raw++;
				continue;

			case DHCP_OPTION_PARAMREQLIST:
				for (int ix = 0; ix < op->size; ix++)
					req->paramreqlist[op->value[ix]] = 1;
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

					if (char *ptr = strchr(req->hostname, '.'))
						*ptr = 0;
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

	if (!req->subnetIP)
	{
		if (req->dhcpp.header.bp_giaddr)
			req->subnetIP = req->dhcpp.header.bp_giaddr;
		else
			req->subnetIP = network.dhcpConn[req->sockInd].server;
	}

	if (!req->messsize)
	{
		if (req->req_type == DHCP_MESS_NONE)
			req->messsize = req->bytes;
		else
			req->messsize = sizeof(dhcp_packet);
	}

	if ((req->req_type == 1 || req->req_type == 3) && cfig.dhcpLogLevel == 3)
	{
		data9 *req1 = (data9*)calloc(1, sizeof(data9));
		memcpy(req1, req, sizeof(data9));
		_beginthread(logDebug, 0, req1);
	}

	if (cfig.dhcpLogLevel >= 2 || verbatim)
	{
		if (req->req_type == DHCP_MESS_NONE)
		{
			if (req->dhcpp.header.bp_giaddr)
			  sprintf(logBuff, "BOOTPREQUEST for %s (%s) from RelayAgent %s received", req->chaddr, req->hostname, IP2String(tempbuff, req->dhcpp.header.bp_giaddr));
			else
			  sprintf(logBuff, "BOOTPREQUEST for %s (%s) from interface %s received", req->chaddr, req->hostname, IP2String(tempbuff, network.dhcpConn[req->sockInd].server));

			logDHCPMess(logBuff, 2);
		}
		else if (req->req_type == DHCP_MESS_DISCOVER)
		{
			if (req->dhcpp.header.bp_giaddr)
		  	  sprintf(logBuff, "DHCPDISCOVER for %s (%s) from RelayAgent %s received", req->chaddr, req->hostname, IP2String(tempbuff, req->dhcpp.header.bp_giaddr));
			else
			  sprintf(logBuff, "DHCPDISCOVER for %s (%s) from interface %s received", req->chaddr, req->hostname, IP2String(tempbuff, network.dhcpConn[req->sockInd].server));

			logDHCPMess(logBuff, 2);
		}
		else if (req->req_type == DHCP_MESS_REQUEST)
		{
			if (req->dhcpp.header.bp_giaddr)
	 		  sprintf(logBuff, "DHCPREQUEST for %s (%s) from RelayAgent %s received", req->chaddr, req->hostname, IP2String(tempbuff, req->dhcpp.header.bp_giaddr));
			else
			  sprintf(logBuff, "DHCPREQUEST for %s (%s) from interface %s received", req->chaddr, req->hostname, IP2String(tempbuff, network.dhcpConn[req->sockInd].server));

			logDHCPMess(logBuff, 2);
		}
	}

	req->vp = req->dhcpp.vend_data;
	memset(req->vp, 0, sizeof(dhcp_packet) - sizeof(dhcp_header));
	//printf("end bytes=%u\n", req->bytes);
	return 1;
}

void debugl(const char *mess)
{
	char t[254];
	strcpy(t, mess);
	logDHCPMess(t, 1);
}

void __cdecl logThread(void *lpParam)
{
	char *mess = (char*)lpParam;

	WaitForSingleObject(lEvent, INFINITE);
	tm *ttm = localtime(&t);
	char buffer[_MAX_PATH];
	strftime(buffer, sizeof(buffer), logFile, ttm);

	if (strcmp(cfig.logFileName, buffer)) {
		if (cfig.logFileName[0]) {
		  FILE *f = fopen(cfig.logFileName, "at");

		  if (f) {
		    fprintf(f, "Logging Continued on file %s\n", buffer);
		    fclose(f);
		  }

		  strcpy(cfig.logFileName, buffer);
		  f = fopen(cfig.logFileName, "at");

		  if (f) {
		    fprintf(f, "%s\n\n", sVersion);
		    fclose(f);
		  }
		}

		strcpy(cfig.logFileName, buffer);
		WritePrivateProfileString("InternetShortcut","URL", buffer, lnkFile);
		WritePrivateProfileString("InternetShortcut","IconIndex", "0", lnkFile);
		WritePrivateProfileString("InternetShortcut","IconFile", buffer, lnkFile);
	}

	FILE *f = fopen(cfig.logFileName, "at");

	if (f) {
		strftime(buffer, sizeof(buffer), "%d-%b-%y %X", ttm);
		fprintf(f, "[%s] %s\n", buffer, mess);
		fclose(f);
	} else {
		cfig.dhcpLogLevel = 0;
	}

	free(mess);
	SetEvent(lEvent);

	_endthread();
	return;
}

void __cdecl logDebug(void *lpParam)
{
	char localBuff[1024];
	char localExtBuff[256];
	data9 *req = (data9*)lpParam;
	genHostName(localBuff, req->dhcpp.header.bp_chaddr, req->dhcpp.header.bp_hlen);
	sprintf(localExtBuff, cliFile, localBuff);
	FILE *f = fopen(localExtBuff, "at");

	if (f)
	{
		tm *ttm = localtime(&t);
		strftime(localExtBuff, sizeof(localExtBuff), "%d-%m-%y %X", ttm);

		char *s = localBuff;
		s += sprintf(s, localExtBuff);
		s += sprintf(s, " SourceMac=%s", req->chaddr);
		s += sprintf(s, " ClientIP=%s", IP2String(localExtBuff, req->dhcpp.header.bp_ciaddr));
		s += sprintf(s, " SourceIP=%s", IP2String(localExtBuff, req->remote.sin_addr.s_addr));
		s += sprintf(s, " RelayAgent=%s", IP2String(localExtBuff, req->dhcpp.header.bp_giaddr));
		fprintf(f, "%s\n", localBuff);

		data3 *op;
		MYBYTE *raw = req->dhcpp.vend_data;
		MYBYTE *rawEnd = raw + (req->bytes - sizeof(dhcp_header));
		MYBYTE maxInd = sizeof(opData) / sizeof(data4);

		for (; raw < rawEnd && *raw != DHCP_OPTION_END;)
		{
			op = (data3*)raw;

			BYTE opType = 2;
			char opName[40] = "Private";

			for (MYBYTE i = 0; i < maxInd; i++)
				if (op->opt_code == opData[i].opTag)
				{
					strcpy(opName, opData[i].opName);
					opType = opData[i].opType;
					break;
				}

			s = localBuff;
			s += sprintf(s, "\t%d\t%s\t", op->opt_code, opName);
			//printf("OpCode=%u,OpLen=%u,OpType=%u\n", op->opt_code, op->size, opType);

			switch (opType)
			{
				case 1:
					memcpy(localExtBuff, op->value, op->size);
					localExtBuff[op->size] = 0;
					sprintf(s, "%s", localExtBuff);
					break;
				case 3:
					for (BYTE x = 4; x <= op->size; x += 4)
					{
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
					if (op->size == 1)
						sprintf(s, "%u", op->value[0]);
					else
						hex2String(s, op->value, op->size);
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

void logDHCPMess(char *logBuff, MYBYTE logLevel) {
  if (verbatim)
    debug(0, "DHCP: ", (void* ) logBuff);

  if (logLevel <= cfig.dhcpLogLevel) {
    char *mess = cloneString(logBuff);
    _beginthread(logThread, 0, mess);
  }
}

data7 *createCache(data71 *lump) {
  MYWORD dataSize = 1 + sizeof(data7) + strlen(lump->mapname);
  dataSize += lump->optionSize;
  data7 *cache = (data7*)calloc(1, dataSize);

  if (!cache) return NULL;

  MYBYTE *dp = &cache->data;
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

#endif
