// tunnel.cpp

// TODO: allow multiple remote destinations
// possibly also multiple local ports

#if TUNNEL

#include "core.h"
#include "util.h"
#include "net.h"
#include "tunnel.h"
#include "monitor.h"
#include "http.h"

namespace tunnel {

LocalBuffers lb;
LocalData ld;
NetworkData nd;

void cleanup(int et) {

  int i;
  bool closed = false;

  for (i=0; i < MAX_SERVERS && nd.tunConn[i].loaded; i++)
    if (nd.tunConn[i].ready) {
      closesocket(nd.tunConn[i].sock);
      closed = true;
    }

  if (closed) logMesg("TUNNEL closed network connections", LOG_INFO);

  if (et) {
    gd.running[TUNNEL_IDX] = false;
    Sleep(1000);
    logMesg("Tunnel stopped", LOG_INFO);
    pthread_exit(NULL);
  } else return;
}

void stop() {
  if (config.tunnel && gd.running[TUNNEL_IDX]) {
    logMesg("Stopping TUNNEL", LOG_NOTICE);
    gd.running[TUNNEL_IDX] = false;
    cleanup(0);
  }
}

void start() {
  if (config.tunnel && !gd.running[TUNNEL_IDX]) {
    pthread_create(&gd.threads[TUNNEL_TIDX], NULL, main, NULL);
  }
}

#if HTTP
bool buildSP(void* arg) {

  http::Data* h = http::initDP("Tunnel", arg, 0);

  if (!h) return false;

  int i=0, j=0;
  char* fp = h->res.dp;
  char* maxData = h->res.dp + h->res.memSize;

  fp += sprintf(fp, "<h3>Tunnel</h3>\n");

  if (!nd.tunConn[0].ready) {
    fp += sprintf(fp, "<p>Waiting for interface</p>\n");
    h->res.bytes = fp - h->res.dp;
    return true;
  }

  fp += sprintf(fp, "<table border=\"0\" cellspacing=\"9\" width=\"800\"><tr><th>Local Host/Port</th><th>Remote Host/Port</th></tr>");

  for (; i < MAX_SERVERS && net.listenServers[i]; i++)
    for (; j < MAX_SERVERS; j++)
      if (nd.tunConn[j].server == net.listenServers[i]) {
        fp += sprintf(fp, "<tr><td>%s / %d</td>", IP2String(lb.tmp, nd.tunConn[j].server), config.lport);
        fp += sprintf(fp, "<td>%s / %d</td></tr>", config.host, config.rport);
      }
  fp += sprintf(fp, "</table>\n<br/>\n<p>\n");

  // TODO split active off per server connection..
  if (ld.active)
    fp += sprintf(fp, "Active with client %s\n", inet_ntoa(nd.cliConn.addr.sin_addr));
  else
    fp += sprintf(fp, "Waiting for client\n");

  fp += sprintf(fp, "</p>\n");

  h->res.bytes = fp - h->res.dp;
  return true;
}
#endif

int fd() {
	unsigned int cfd = nd.cliConn.sock;
	if (cfd < nd.remConn.sock) cfd = nd.remConn.sock;
	return cfd + 1;
  printf("fds = c %d r %d\r\n", cfd, nd.remConn.sock);
}

void remoteData() {

  int cnt = recv(nd.remConn.sock, lb.data, sizeof(lb.data), 0);

  if (cnt == SOCKET_ERROR) {
    logMesg("TUNNEL remoteData socket error", LOG_DEBUG);
    closesocket(nd.cliConn.sock);
    closesocket(nd.remConn.sock);
  }

  if (cnt == 0) {
    closesocket(nd.cliConn.sock);
    closesocket(nd.remConn.sock);
  }

  if (cnt == SOCKET_ERROR || cnt == 0) {
    ld.transm = false;
    ld.active = false;
  }

  send(nd.cliConn.sock, lb.data, cnt, 0);

  if (gs.verbose && config.logging == LOG_DEBUG) {
    printf("TUNNEL transmitting data from remote:\r\n");
    fwrite(lb.data, sizeof(char), cnt, stdout);
    fflush(stdout);
  }

}

void clientData() {

  int cnt = recv(nd.cliConn.sock, lb.data, sizeof(lb.data), 0);


  if (cnt == SOCKET_ERROR) {
    logMesg("TUNNEL clientData socket error", LOG_DEBUG);
    closesocket(nd.cliConn.sock);
    closesocket(nd.remConn.sock);
  }

  if (cnt == 0) {
    closesocket(nd.cliConn.sock);
    closesocket(nd.remConn.sock);
  }

  if (cnt == SOCKET_ERROR || cnt == 0) {
    ld.active = false;
    ld.transm = false;
  }

  send(nd.remConn.sock, lb.data, cnt, 0);

  if (gs.verbose && config.logging == LOG_DEBUG) {
    printf("TUNNEL transmitting data from client:\r\n");
    fwrite(lb.data, sizeof(char), cnt, stdout);
    fflush(stdout);
  }

}

int useTunnel() {

  int nRet;
  fd_set io;

  ld.transm = true;

  do {

    FD_ZERO(&io);
    FD_SET(nd.cliConn.sock, &io);
    FD_SET(nd.remConn.sock, &io);

    memset(lb.data, 0, sizeof(lb.data));

    if (select(fd(), &io, NULL, NULL, NULL) == SOCKET_ERROR) {
      if (WSAGetLastError() == WSAENOTSOCK)
        sprintf(lb.log,"TUNNEL client/remote closed connection");
      else
        sprintf(lb.log,"TUNNEL useTunnel select error: %u", WSAGetLastError());
      logMesg(lb.log, LOG_NOTICE);
      break;
    }

    if (FD_ISSET(nd.cliConn.sock, &io)) clientData();
    if (FD_ISSET(nd.remConn.sock, &io)) remoteData();

  } while (ld.transm);

  return 0;
}

int buildTunnel() {

  memset(&nd.remConn, 0, sizeof(ConnType));

  nd.remote_host = gethostbyname(config.host);

  if (nd.remote_host == NULL) {
    sprintf(lb.log, "TUNNEL could not resolve host %s", config.host);
    logMesg(lb.log, LOG_NOTICE);
    return 0;
  }

  nd.remConn.addr.sin_family = AF_INET;
  nd.remConn.addr.sin_port = htons(config.rport);
  memcpy(&nd.remConn.addr.sin_addr.s_addr, nd.remote_host->h_addr, nd.remote_host->h_length);
  nd.remConn.sock = socket(AF_INET, SOCK_STREAM, 0);

  if (nd.remConn.sock == INVALID_SOCKET) {
    sprintf(lb.log, "TUNNEL buildTunnel socket error %u", WSAGetLastError());
    logMesg(lb.log, LOG_NOTICE);
    ld.active = false;
    return 0;
  }

  if (connect(nd.remConn.sock, (sockaddr*) &nd.remConn.addr, sizeof(nd.remConn.addr)) == SOCKET_ERROR) {
    sprintf(lb.log, "TUNNEL buildTunnel connect error %u", WSAGetLastError());
    logMesg(lb.log, LOG_NOTICE);
    ld.active = false;
    return 0;
  }

  return 1;
}

int handleClient(MYBYTE sndx) {

  int sockLen = sizeof(nd.cliConn.addr);

  ld.active = true;

  nd.cliConn.sock =
    accept(nd.tunConn[sndx].sock, (sockaddr*) &nd.cliConn.addr, &sockLen);

  if (nd.cliConn.sock == INVALID_SOCKET) {
    sprintf(lb.log, "TUNNEL accept error %u", WSAGetLastError());
    logMesg(lb.log, LOG_NOTICE);
    return 0;
  }

  sprintf(lb.log, "TUNNEL request from %s", inet_ntoa(nd.cliConn.addr.sin_addr));
  logMesg(lb.log, LOG_INFO);

  return 1;
}


void __cdecl init(void* arg) {

  bool bindfailed;
  int i, j, nRet;

  if (!config.lport) config.lport = 80;
  if (!config.rport) config.rport = 80;
  if (!config.host) config.host = "localhost";

  do {

    cleanup(0);

    memset(&nd, 0, sizeof(nd));

    bindfailed = false;
    ld.active = false;

    for (i=0, j=0; j < MAX_SERVERS && net.listenServers[j]; j++) {

      nd.tunConn[i].sock = socket(AF_INET, SOCK_STREAM, 0);

      if (nd.tunConn[i].sock == INVALID_SOCKET) {
        sprintf(lb.log, "TUNNEL socket error %u", WSAGetLastError());
        logMesg(lb.log, LOG_NOTICE);
        bindfailed = true;
        continue;
      }

      nd.tunConn[i].addr.sin_family = AF_INET;
      nd.tunConn[i].addr.sin_addr.s_addr = net.listenServers[j];
      nd.tunConn[i].addr.sin_port = htons(config.lport);

      setsockopt(nd.tunConn[i].sock, SOL_SOCKET, SO_REUSEADDR, "1", sizeof(int));

      nRet = bind(nd.tunConn[i].sock, (sockaddr*) &nd.tunConn[i].addr, sizeof(nd.tunConn[i].addr));

      if (nRet == SOCKET_ERROR) {
        sprintf(lb.log, "TUNNEL cannot bind local port, error %u", WSAGetLastError());
        logMesg(lb.log, LOG_NOTICE);
        bindfailed = true;
        closesocket(nd.tunConn[i].sock);
        continue;
      }

      if (listen(nd.tunConn[i].sock, 1) == SOCKET_ERROR) {
        sprintf(lb.log, "TUNNEL listen error %u", WSAGetLastError());
        logMesg(lb.log, LOG_NOTICE);
        bindfailed = true;
        closesocket(nd.tunConn[i].sock);
        continue;
      }

      if (nd.maxFD < nd.tunConn[i].sock) nd.maxFD = nd.tunConn[i].sock;

      nd.tunConn[i].loaded = true;
      nd.tunConn[i].ready = true;
      nd.tunConn[i].server = net.listenServers[j];
      nd.tunConn[i].mask = net.listenMasks[j];
      nd.tunConn[i].port = config.lport;

      i++;
    }

    nd.maxFD++;

    if (bindfailed) net.failureCounts[TUNNEL_IDX]++;
    else net.failureCounts[TUNNEL_IDX] = 0;

    if (!nd.tunConn[0].ready) {
      logMesg("TUNNEL no interface ready, waiting...", LOG_NOTICE);
      continue;
    }

    for (i=0; i < MAX_SERVERS && net.listenServers[i]; i++) {
      for (j=0; j < MAX_SERVERS; j++) {
        if (nd.tunConn[j].server == net.listenServers[i]) {
          sprintf(lb.log, "TUNNEL listening on: %s", IP2String(lb.tmp, net.listenServers[i]));
          logMesg(lb.log, LOG_INFO);
          break;
        }
      }
    }

  } while (dCWait(TUNNEL_IDX));

  _endthread();
  return;
}

void* main(void* arg) {

  gd.running[TUNNEL_IDX] = true;

  logMesg("TUNNEL starting", LOG_INFO);

  _beginthread(init, 0, 0);

  int i;
  fd_set readfds;
  timeval tv = { 20, 0 };

  do {

    net.busy[TUNNEL_IDX] = false;

    if (!nd.tunConn[0].ready) { Sleep(1000); continue; }
    if (!net.ready[TUNNEL_IDX]) { Sleep(1000); continue; }
    if (net.refresh) { Sleep(1000); continue; }

    FD_ZERO(&readfds);

    for (i=0; i < MAX_SERVERS && nd.tunConn[i].ready; i++)
      FD_SET(nd.tunConn[i].sock, &readfds);

    if (select(nd.maxFD, &readfds, NULL, NULL, &tv) != SOCKET_ERROR) {

      ld.t = time(NULL);

      if (net.ready[TUNNEL_IDX]) {

        net.busy[TUNNEL_IDX] = true;

        for (i=0; i < MAX_SERVERS && nd.tunConn[i].ready; i++)

          if (FD_ISSET(nd.tunConn[i].sock, &readfds)) {
            while(ld.active) Sleep(1000);
            if (handleClient(i)) if (buildTunnel()) useTunnel();
          }

      }

    } else ld.t = time(NULL);

  } while (gd.running[TUNNEL_IDX]);

  cleanup(1);
}

}
#endif
