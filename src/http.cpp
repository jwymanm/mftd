// http.cpp

#if HTTP

#include "core.h"
#include "util.h"
#include "net.h"
#include "fdns.h"
#include "tunnel.h"
#include "dhcp.h"
#include "monitor.h"
#include "http.h"

namespace http {

LocalBuffers lb;
NetworkData nd;

Codes resp = {
  "HTTP/1.1 200 OK\r\nDate: %s\r\nLast-Modified: %s\r\nContent-Type: text/html\r\nConnection: Close\r\nContent-Length:         \r\n\r\n",
  "HTTP/1.1 403 Forbidden\r\n\r\n<h1>403 Forbidden</h1>",
  "HTTP/1.1 404 Not Found\r\n\r\n<h1>404 Not Found</h1>"
};

HTML html = {
  "",
  "<!HTML>\n<html>\n<head>\n<style>\nbody{color:#333;font-family:verdana;font-size:9pt}th,td{font-size:8pt;text-align:left}</style>\n<title>%s</title>\n<meta http-equiv=\"refresh\" content=\"60\">\n<meta http-equiv=\"cache-control\" content=\"no-cache\">\n</head>\n",
  "<body>\n<h1>%s</h1>",
  "<td>%s</td>",
  "\n</body>",
  "\n</html>"
};

void cleanup(int et) {

  if (nd.httpConn.ready) {
    closesocket(nd.httpConn.sock);
    logMesg("HTTP cleared network connections", LOG_DEBUG);
  }

  if (et) {
    gd.running[HTTP_IDX] = false;
    Sleep(1000);
    logMesg("HTTP stopped", LOG_INFO);
    pthread_exit(NULL);
  }

  return;
}

void stop() {
  if (config.http && gd.running[HTTP_IDX]) {
    logMesg("Stopping HTTP", LOG_NOTICE);
    gd.running[HTTP_IDX] = false;
    cleanup(0);
  }
}

void start() {
  if (config.http && !gd.running[HTTP_IDX]) {
    pthread_create(&gd.threads[HTTP_TIDX], NULL, main, NULL);
  }
}

Data* initDP(const char* name, void* arg, int memSize) {

  Data* h = (Data*) arg;

  h->res.memSize = HTTP_SECSIZE + memSize;
  h->res.dp = (char*) calloc(1, h->res.memSize);

  if (!h->res.dp) {
    sprintf(lb.log, "%s Memory Error in initDP", name);
    logMesg(lb.log, LOG_NOTICE);
    return 0;
  }

  return h;
}

void buildHP(Data* h) {

  h->res.code = resp;
  h->html = html;

  char respBuff[sizeof(Response)];
  char* rB = respBuff;
  char* dp[SERVICECOUNT-1];
  int i = 0, j = 0, sizes[SERVICECOUNT-1];

  tm* ttm = localtime(&h->req.t);
  strftime(lb.tmp, sizeof(lb.tmp), "%a, %d %b %Y %H:%M:%S", ttm);

  rB += sprintf(rB, h->res.code.send200, lb.tmp, lb.tmp);

  char* contentStart = rB;

  rB += sprintf(rB, h->html.htmlStart, h->html.htmlTitle);
  rB += sprintf(rB, h->html.bodyStart, gd.displayName);
  rB += sprintf(rB, "\n<h2>Services running</h2>\n<h3>");

#if MONITOR
  if (config.monitor) {
    rB += sprintf(rB, "Monitor ");
//    if (monitor::htmlStatus((void*)req, &rH))
  //    dp[j++] = h->res.dp; sizes[i] = h->res.bytes;
  }
#endif
#if FDNS
  if (config.fdns) {
    rB += sprintf(rB, "FDNS ");
//    if (fdns::htmlStatus((void*)req, &rH))
//      dp[j++] = req->dp; sizes[i] = req->bytes;
  }
#endif
#if TUNNEL
  if (config.tunnel) {
    rB += sprintf(rB, "Tunnel ");
    if (tunnel::buildSP((void*)h)) {
      dp[j] = h->res.dp; sizes[j] = h->res.bytes; j++;
    }
  }
#endif
#if DHCP
  if (config.dhcp) {
    rB += sprintf(rB, "DHCP");
    if (dhcp::buildSP((void*)h)) {
      dp[j] = h->res.dp; sizes[j] = h->res.bytes; j++;
    }
  }
#endif

  rB += sprintf(rB, "</h3><h2>Service config/status</h2> \n");

  h->res.memSize = rB - respBuff;

  for (;i<j;i++) h->res.memSize += sizes[i];

  h->res.memSize += strlen(h->html.bodyEnd) + strlen(h->html.htmlEnd) + 2;

  h->res.dp = (char*)calloc(1, h->res.memSize);

  if (!h->res.dp) {
    logMesg("HTTP Memory Error in buildHP", LOG_NOTICE);
    closesocket(h->req.sock);
    free(h);
    return;
  }

  memcpy(h->res.dp, respBuff, rB - respBuff);

  contentStart = h->res.dp + (contentStart - respBuff);

  rB = h->res.dp + (rB - respBuff);

  for (i=0;i<j;i++)
    if (sizes[i]) {
      memcpy(rB, dp[i], sizes[i]);
      rB += sizes[i];
      free(dp[i]);
      dp[i]=NULL;
    }

  rB += sprintf(rB, h->html.bodyEnd);
  rB += sprintf(rB, h->html.htmlEnd);

  MYBYTE x = sprintf(lb.tmp, "%u", (rB - contentStart));
  memcpy((contentStart - HTTP_LOFFT), lb.tmp, x);
  h->res.bytes = rB - h->res.dp;
  return;
}

void __cdecl sendHTTP(void* arg) {
  Data* h = (Data*) arg;
  char* dp = h->res.dp;
  timeval tv;
  fd_set writefds;
  int sent = 0;
  while (gd.running[HTTP_IDX] && h->res.bytes > 0) {
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    FD_ZERO(&writefds);
    FD_SET(h->req.sock, &writefds);
    if (select((h->req.sock + 1), NULL, &writefds, NULL, &tv)) {
      if (h->res.bytes > 1024) sent = send(h->req.sock, dp, 1024, 0);
      else sent = send(h->req.sock, dp, h->res.bytes, 0);
      errno = WSAGetLastError();
      if (errno || sent < 0) break;
      dp += sent;
      h->res.bytes -= sent;
    } else break;
  }
  closesocket(h->req.sock);
  free(h->res.dp);
  free(h);
  _endthread();
  return;
}

void procHTTP(Data* h) {

  h->req.ling.l_onoff = HTTP_ISLINGER;
  h->req.ling.l_linger = 30; //0 = discard data, nonzero = wait for data sent
  setsockopt(h->req.sock, SOL_SOCKET, SO_LINGER, (const char*)&h->req.ling, sizeof(h->req.ling));

  fd_set rfds;

  timeval tv;
  tv.tv_sec = 2;
  tv.tv_usec = 0;

  FD_ZERO(&rfds);
  FD_SET(h->req.sock, &rfds);

  if (!select((h->req.sock + 1), &rfds, NULL, NULL, &tv)) {
    sprintf(lb.log, "Client %s, HTTP Message Receive failed", IP2String(lb.tmp, h->req.remote.sin_addr.s_addr));
    logMesg(lb.log, LOG_INFO);
    closesocket(h->req.sock);
    free(h);
    return;
  }

  errno = 0;
  h->req.bytes = recv(h->req.sock, lb.http, sizeof(lb.http), 0);
  errno = WSAGetLastError();

  if (errno || h->req.bytes <= 0) {
    sprintf(lb.log, "Client %s, HTTP Message Receive failed, WSAError %d", IP2String(lb.tmp, h->req.remote.sin_addr.s_addr), errno);
    logMesg(lb.log, LOG_INFO);
    closesocket(h->req.sock);
    free(h);
    return;
  } else {
    sprintf(lb.log, "Client %s, HTTP Request Received", IP2String(lb.tmp, h->req.remote.sin_addr.s_addr));
    logMesg(lb.log, LOG_INFO);
  }

  if (nd.httpClients[0] && !findServer(nd.httpClients, 8, h->req.remote.sin_addr.s_addr)) {
    sprintf(lb.log, "Client %s, HTTP Access Denied", IP2String(lb.tmp, h->req.remote.sin_addr.s_addr));
    logMesg(lb.log, LOG_INFO);
    h->res.dp = resp.send403;//(char*)calloc(1, sizeof(hd.send403));
    h->res.memSize = sizeof(resp.send403);
    h->res.bytes = strlen(resp.send403);//sprintf(req->dp, hd.send403);
    _beginthread(sendHTTP, 0, (void*)h);
    return;
  }

  lb.http[sizeof(lb.http) - 1] = 0;
  char* fp = NULL;
  char* end = strchr(lb.http, '\n');

  if (end && end > lb.http && (*(end - 1) == '\r')) {
    *(end - 1) = 0;
    if (myTokenize(lb.http, lb.http, " ", true) > 1)
      fp = myGetToken(lb.http, 1);
  }

  if (fp && !strcasecmp(fp, "/")) {
    buildHP(h);
    _beginthread(sendHTTP, 0, (void*)h);
  } else {
    if (fp) {
      sprintf(lb.log, "Client %s, %s not found", IP2String(lb.tmp, h->req.remote.sin_addr.s_addr), fp);
      logMesg(lb.log, LOG_INFO);
    } else {
      sprintf(lb.log, "Client %s, Invalid http request", IP2String(lb.tmp, h->req.remote.sin_addr.s_addr));
      logMesg(lb.log, LOG_INFO);
    }
    h->res.dp = resp.send404; //(char*)calloc(1, sizeof(send404));
    h->res.bytes = strlen(resp.send404);//sprintf(req->dp, hd.send404);
    h->res.memSize = sizeof(resp.send404);
    _beginthread(sendHTTP, 0, (void*)h);
  }
  return;
}

void __cdecl init(void* arg) {

  FILE* f;
  char name[MAXCFGSIZE+2], value[MAXCFGSIZE+2];
  bool bindfailed;

  sprintf(lb.htm, "%s\\" NAME ".htm", path.tmp);

  do {

    cleanup(0);

    memset(&nd, 0, sizeof(nd));

    bindfailed = false;

    nd.httpConn.port = 6789;
    nd.httpConn.server = inet_addr("127.0.0.1");
    nd.httpConn.loaded = true;

    if (config.httpaddr) {
      mySplit(name, value, config.httpaddr, ':');
      if (isIP(name)) {
        nd.httpConn.loaded = true;
        nd.httpConn.server = inet_addr(name);
      } else {
        nd.httpConn.loaded = false;
        sprintf(lb.log, "Warning: Section [HTTP], Invalid IP address %s, ignored", name);
        logMesg(lb.log, LOG_NOTICE);
      }
      if (value[0]) {
        if (atoi(value)) nd.httpConn.port = atoi(value);
  	    else {
          nd.httpConn.loaded = false;
          sprintf(lb.log, "Warning: Section [HTTP], Invalid port %s, ignored", value);
	        logMesg(lb.log, LOG_NOTICE);
 	      }
      }
      if (nd.httpConn.server != inet_addr("127.0.0.1") && !findServer(net.allServers, MAX_SERVERS, nd.httpConn.server)) {
	      nd.httpConn.loaded = false;
	      sprintf(lb.log, "Warning: Section [HTTP], %s not available, ignored", name);
	      logMesg(lb.log, LOG_NOTICE);
        bindfailed = true;
      }
    }

    if (config.httpclient) {
      if (isIP(config.httpclient)) addServer(nd.httpClients, 8, inet_addr(config.httpclient));
	    else {
        sprintf(lb.log, "Warning: Section [HTTP], invalid client IP %s, ignored", config.httpclient);
        logMesg(lb.log, LOG_NOTICE);
      }
    }

    if (config.htmltitle)
      strncpy(html.htmlTitle, config.htmltitle, strlen(config.htmltitle));
    else
      sprintf(html.htmlTitle, NAME " on %s", net.hostname);

    nd.httpConn.sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (nd.httpConn.sock == INVALID_SOCKET) {
      sprintf(lb.log, "HTTP socket error %u", WSAGetLastError());
      logMesg(lb.log, LOG_NOTICE);
      bindfailed = true;
    } else {

      nd.httpConn.addr.sin_family = AF_INET;
      nd.httpConn.addr.sin_addr.s_addr = nd.httpConn.server;
      nd.httpConn.addr.sin_port = htons(nd.httpConn.port);

      int nRet = bind(nd.httpConn.sock, (sockaddr*)&nd.httpConn.addr, sizeof(sockaddr_in));

      if (nRet == SOCKET_ERROR) {
        sprintf(lb.log, "HTTP interface %s TCP port %u not available", IP2String(lb.tmp, nd.httpConn.server), nd.httpConn.port);
        logMesg(lb.log, LOG_NOTICE);
        bindfailed = true;
        closesocket(nd.httpConn.sock);
      } else {
        nRet = listen(nd.httpConn.sock, SOMAXCONN);

        if (nRet == SOCKET_ERROR) {
          sprintf(lb.log, "HTTP %s TCP port %u error on listen", IP2String(lb.tmp, nd.httpConn.server), nd.httpConn.port);
          logMesg(lb.log, LOG_NOTICE);
          bindfailed = true;
          closesocket(nd.httpConn.sock);
        } else {
          nd.httpConn.loaded = true;
          nd.httpConn.ready = true;
          if (nd.httpConn.sock > nd.maxFD) nd.maxFD = nd.httpConn.sock;
        }
      }
    }

    nd.maxFD++;

    if (bindfailed) net.failureCounts[HTTP_IDX]++;
    else net.failureCounts[HTTP_IDX] = 0;

    if (nd.httpConn.ready) {
      sprintf(lb.log, "HTTP service status: http://%s:%u", IP2String(lb.tmp, nd.httpConn.server), nd.httpConn.port);
      logMesg(lb.log, LOG_INFO);
      FILE* f = fopen(lb.htm, "wt");
      if (f) {
        fprintf(f, "<!html><html><head><meta http-equiv=\"refresh\" content=\"0;url=http://%s:%u\"</head></html>", IP2String(lb.tmp, nd.httpConn.server), nd.httpConn.port);
        fclose(f);
      }
    } else {
      FILE* f = fopen(lb.htm, "wt");
      if (f) {
        fprintf(f, "<!html><html><body><h2>HTTP Service is not running</h2></body></html>");
        fclose(f);
      }
    }

  } while (dCWait(HTTP_IDX));

  _endthread();
  return;
}

void* main(void* arg) {

  gd.running[HTTP_IDX] = true;

  logMesg("HTTP starting", LOG_INFO);

  _beginthread(init, 0, 0);

  fd_set readfds;
  timeval tv = { 20, 0 };

  do {

    net.busy[HTTP_IDX] = false;

    if (!nd.httpConn.ready) { Sleep(1000); continue; }
    if (!net.ready[HTTP_IDX]) { Sleep(1000); continue; }
    if (net.refresh) { Sleep(1000); continue; }

    FD_ZERO(&readfds);

    if (nd.httpConn.ready) FD_SET(nd.httpConn.sock, &readfds);

    if (select(nd.maxFD, &readfds, NULL, NULL, &tv) != SOCKET_ERROR) {
      if (net.ready[HTTP_IDX]) {
        net.busy[HTTP_IDX] = true;
        if (nd.httpConn.ready && FD_ISSET(nd.httpConn.sock, &readfds)) {
          Data* h = (Data*) calloc(1, sizeof(Data));
          if (h) {
            h->req.t = time(NULL);
            h->req.sockLen = sizeof(h->req.remote);
            h->req.sock = accept(nd.httpConn.sock, (sockaddr*)&h->req.remote, &h->req.sockLen);
            if (h->req.sock == INVALID_SOCKET) {
              free(h);
              if (!net.ready[HTTP_IDX]) break;
              sprintf(lb.log, "HTTP accept failed, error %u\n", WSAGetLastError());
              logMesg(lb.log, LOG_NOTICE);
            } else procHTTP(h);
          } else logMesg("HTTP memory error", LOG_NOTICE);
        }
      }
    }

  } while (gd.running[HTTP_IDX]);

  cleanup(1);

}

}
#endif
