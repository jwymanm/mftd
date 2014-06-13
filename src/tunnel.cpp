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

  if (closed) {
    LSM(LOG_INFO, "closed network connections")
  }

  if (et) {
    *ld.ir = false;
    Sleep(1000);
    LSM(LOG_NOTICE, "stopped")
    pthread_exit(NULL);
  } else return;
}

void stop() {
  if (config.tunnel && gd.running[TUNNEL_IDX]) {
    *ld.ir = false;
    LSM(LOG_NOTICE, "stopping")
    cleanup(0);
    Sleep(1000);
  }
}

void start() {
  if (config.tunnel && !gd.running[TUNNEL_IDX]) {
    pthread_create(&gd.threads[TUNNEL_TIDX], NULL, main, NULL);
    Sleep(1000);
  }
}

#if HTTP
bool buildSP(void* arg) {

  HTTP_SPHEAD(0)

  int i=0, j=0;

  if (!nd.tunConn[0].ready) {
    fp += sprintf(fp, "<p>Waiting for interface</p>\n");
    HTTP_SPFOOT
  }

  fp += sprintf(fp, "<table border=\"0\" cellspacing=\"9\" width=\"800\"><tr><th>Local Host/Port</th><th>Remote Host/Port</th></tr>");

  for (; i < MAX_SERVERS && net.listenServers[i]; i++)
    for (; j < MAX_SERVERS; j++)
      if (nd.tunConn[j].server == net.listenServers[i]) {
        fp += sprintf(fp, "<tr><td>%s / %d</td>", IP2String(lb.tmp, nd.tunConn[j].server), config.lport);
        fp += sprintf(fp, "<td>%s / %d</td></tr>", config.host, config.rport);
      }
  fp += sprintf(fp, "</table>\n<br/>\n<p>");

  // TODO split active off per server connection..
  if (ld.active)
    fp += sprintf(fp, "Active with client %s", inet_ntoa(nd.cliConn.addr.sin_addr));
  else
    fp += sprintf(fp, "Waiting for client");

  fp += sprintf(fp, "</p>\n<br/>\n");

  HTTP_SPFOOT
}
#endif

int fd() {
	unsigned int cfd = nd.cliConn.sock;
	if (cfd < nd.remConn.sock) cfd = nd.remConn.sock;
	return cfd + 1;
  //printf("fds = c %d r %d\r\n", cfd, nd.remConn.sock);
}

void remoteData() {

  int cnt = recv(nd.remConn.sock, lb.data, sizeof(lb.data), 0);

  if (cnt == SOCKET_ERROR) {
    LSM(LOG_DEBUG, "remoteData socket error %u", WSAGetLastError());
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
    printf("%s transmitting data from remote:\r\n", ld.sn);
    fwrite(lb.data, sizeof(char), cnt, stdout);
    fflush(stdout);
  }

}

void clientData() {

  int cnt = recv(nd.cliConn.sock, lb.data, sizeof(lb.data), 0);


  if (cnt == SOCKET_ERROR) {
    LSM(LOG_DEBUG, "clientData socket error %u", WSAGetLastError());
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
    printf("%s transmitting data from client:\r\n", ld.sn);
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
        sprintf(lb.log, "client/remote closed connection");
      else
        sprintf(lb.log, "useTunnel select error: %u", WSAGetLastError());
      LSM(LOG_NOTICE, "%s", lb.log)
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
    LSM(LOG_NOTICE, "could not resolve host %s", config.host)
    ld.active = false;
    return 0;
  }

  nd.remConn.addr.sin_family = AF_INET;
  nd.remConn.addr.sin_port = htons(config.rport);
  memcpy(&nd.remConn.addr.sin_addr.s_addr, nd.remote_host->h_addr, nd.remote_host->h_length);
  nd.remConn.sock = socket(AF_INET, SOCK_STREAM, 0);

  if (nd.remConn.sock == INVALID_SOCKET) {
    showSockError(ld.sn, GetLastError());
    ld.active = false;
    return 0;
  }

  if (connect(nd.remConn.sock, (sockaddr*) &nd.remConn.addr, sizeof(nd.remConn.addr)) == SOCKET_ERROR) {
    LSM(LOG_NOTICE, "buildTunnel connect error %u", WSAGetLastError())
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
    showSockError(ld.sn, GetLastError());
    return 0;
  }

  LSM(LOG_INFO, "request from %s", inet_ntoa(nd.cliConn.addr.sin_addr))
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
        //todo show server interface responsible
        showSockError(ld.sn, GetLastError());
        bindfailed = true;
        continue;
      }

      nd.tunConn[i].addr.sin_family = AF_INET;
      nd.tunConn[i].addr.sin_addr.s_addr = net.listenServers[j];
      nd.tunConn[i].addr.sin_port = htons(config.lport);

      setsockopt(nd.tunConn[i].sock, SOL_SOCKET, SO_REUSEADDR, "1", sizeof(int));

      nRet = bind(nd.tunConn[i].sock, (sockaddr*) &nd.tunConn[i].addr, sizeof(nd.tunConn[i].addr));

      if (nRet == SOCKET_ERROR) {
        showSockError(ld.sn, GetLastError());
        bindfailed = true;
        closesocket(nd.tunConn[i].sock);
        continue;
      }

      if (listen(nd.tunConn[i].sock, 1) == SOCKET_ERROR) {
        showSockError(ld.sn, GetLastError());
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

    if (bindfailed)  (*ld.fc)++;
    else *ld.fc = 0;

    if (!nd.tunConn[0].ready) {
      LSM(LOG_NOTICE, "no interface ready, waiting")
      continue;
    }

    for (i=0; i < MAX_SERVERS && net.listenServers[i]; i++) {
      for (j=0; j < MAX_SERVERS; j++) {
        if (nd.tunConn[j].server == net.listenServers[i]) {
          LSM(LOG_INFO, "listening on %s", IP2String(lb.tmp, net.listenServers[i]))
          break;
        }
      }
    }

  } while (dCWait(TUNNEL_IDX));

  _endthread();
  return;
}

void* main(void* arg) {

  SERVICESTART(TUNNEL_IDX)

    if (!nd.tunConn[0].ready) { Sleep(1000); continue; }
    if (net.refresh) { Sleep(1000); continue; }

    FD_ZERO(&readfds);

    for (i=0; i < MAX_SERVERS && nd.tunConn[i].ready; i++)
      FD_SET(nd.tunConn[i].sock, &readfds);

    if (select(nd.maxFD, &readfds, NULL, NULL, &tv) != SOCKET_ERROR) {

      ld.t = time(NULL);

      if (*ld.nr) {

        *ld.ib = true;

        for (i=0; i < MAX_SERVERS && nd.tunConn[i].ready; i++)

          if (FD_ISSET(nd.tunConn[i].sock, &readfds)) {
            while(ld.active) Sleep(1000);
            if (handleClient(i)) if (buildTunnel()) useTunnel();
          }

      }

    } else ld.t = time(NULL);

  SERVICEEND
}

}
#endif
