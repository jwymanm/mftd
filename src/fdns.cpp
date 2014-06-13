// fdns.cpp

#if FDNS

#include "core.h"
#include "util.h"
#include "net.h"
#include "fdns.h"
#include "monitor.h"
#include "http.h"

namespace fdns {

LocalBuffers lb;
LocalData ld;
NetworkData nd;

void cleanup(int et) {

  int i;
  bool closed = false;

  for (i=0; i < MAX_SERVERS && nd.fdnsConn[i].loaded; i++)
    if (nd.fdnsConn[i].ready) {
      closesocket(nd.fdnsConn[i].sock);
      closed = true;
    }

  if (closed) {
    LSM(LOG_DEBUG, "cleared network connections")
  }

  if (et) {
    *ld.ir = false;
    Sleep(1000);
    LSM(LOG_NOTICE, "stopped")
    pthread_exit(NULL);
  }
  return;
}

void stop() {
  if (config.fdns && gd.running[FDNS_IDX]) {
    *ld.ir = false;
    LSM(LOG_NOTICE, "stopping")
    cleanup(0);
    Sleep(1000);
  }
}

void start() {
  if (config.fdns && !gd.running[FDNS_IDX]) {
    pthread_create(&gd.threads[FDNS_TIDX], NULL, main, NULL);
    Sleep(1000);
  }
}

#if HTTP
// HTTP Service status page
bool buildSP(void* arg) {

  HTTP_SPHEAD(0)

  int i=0,j=0;

  if (!nd.fdnsConn[0].ready) {
    fp += sprintf(fp, "<p>Waiting for interface</p>\n");
    HTTP_SPFOOT
  }

  fp += sprintf(fp, "<table border=\"0\" cellspacing=\"9\" width=\"800\"><tr><th>Local Host/Port</th></tr>");

  for (; i < MAX_SERVERS && net.listenServers[i]; i++)
    for (; j < MAX_SERVERS; j++)
      if (nd.fdnsConn[j].server == net.listenServers[i])
        fp += sprintf(fp, "<tr><td>%s / %d</td></tr>", IP2String(lb.tmp, nd.fdnsConn[j].server), FDNSPORT);

  fp += sprintf(fp, "</table>\n<br/>\n<p>Response ip: %s</p>\n<br/>\n", config.fdnsip);

  HTTP_SPFOOT
}
#endif

void sendResponse(MYBYTE sndx) {

  char* m = lb.msg;
  int n,* i = ld.ip4;
  Request* r = &nd.req;

  memset(r, 0, sizeof(Request));

  r->sockLen = sizeof(r->remote);
  r->flags = 0;
  r->bytes = recvfrom(nd.fdnsConn[sndx].sock, m, FDNSMSG_SIZE, r->flags, (sockaddr*) &r->remote, &r->sockLen);

  if (r->bytes < 0) return;

  n = r->bytes;

  // Same Id
  m[2]=0x81; m[3]=0x80; // Change Opcode and flags
  m[6]=0; m[7]=1; // One answer
  m[8]=0; m[9]=0; // NSCOUNT
  m[10]=0; m[11]=0; // ARCOUNT
  // Keep request in message and add answer
  m[n++]=0xC0; m[n++]=0x0C; // Offset to the domain name
  m[n++]=0x00; m[n++]=0x01; // Type 1
  m[n++]=0x00; m[n++]=0x01; // Class 1
  m[n++]=0x00; m[n++]=0x00; m[n++]=0x00; m[n++]=0x3c; // TTL
  m[n++]=0x00; m[n++]=0x04; // Size --> 4
  m[n++]=i[0]; m[n++]=i[1]; m[n++]=i[2]; m[n++]=i[3]; // IP

  sendto(nd.fdnsConn[sndx].sock, m, n, r->flags, (sockaddr*) &r->remote, r->sockLen);

  return;
}

void __cdecl init(void* arg) {

  bool bindfailed;
  int i, j, nRet;
  char* ip4str = ld.ip4str;

  if (isIP(config.fdnsip)) {
    LSM(LOG_INFO, "responding with ip address %s", config.fdnsip)
    strcpy(ld.ip4str, config.fdnsip);
  } else {
    LSM(LOG_NOTICE, "no/invalid ip address set, responding with 127.0.0.1")
    strcpy(ld.ip4str, "127.0.0.1");
  }

  for (int i=0; i < 4; i++)
    ld.ip4[i] = atoi(strsep(&ip4str, "."));

  do {

    cleanup(0);

    memset(&nd, 0, sizeof(nd));

    bindfailed = false;

    for (i=0, j=0; j < MAX_SERVERS && net.listenServers[j]; j++) {

      nd.fdnsConn[i].sock = socket(AF_INET, SOCK_DGRAM, 0);

      if (nd.fdnsConn[i].sock == INVALID_SOCKET) {
        showSockError(ld.sn, GetLastError());
        bindfailed = true;
        continue;
      }

      nd.fdnsConn[i].addr.sin_family = AF_INET;
      nd.fdnsConn[i].addr.sin_addr.s_addr = net.listenServers[j];
      nd.fdnsConn[i].addr.sin_port = htons(FDNSPORT);

      setsockopt(nd.fdnsConn[i].sock, SOL_SOCKET, SO_REUSEADDR, "1", sizeof(int));
      nRet = bind(nd.fdnsConn[i].sock, (sockaddr*) &nd.fdnsConn[i].addr, sizeof(struct sockaddr_in));

      if (nRet == SOCKET_ERROR) {
        LSM(LOG_NOTICE, "cannot bind dns port, error %u", WSAGetLastError())
        bindfailed = true;
        closesocket(nd.fdnsConn[i].sock);
        continue;
      }

      if (nd.maxFD < nd.fdnsConn[i].sock) nd.maxFD = nd.fdnsConn[i].sock;

      nd.fdnsConn[i].loaded = true;
      nd.fdnsConn[i].ready = true;
      nd.fdnsConn[i].server = net.listenServers[j];
      nd.fdnsConn[i].mask = net.listenMasks[j];
      nd.fdnsConn[i].port = FDNSPORT;

      i++;
    }

    nd.maxFD++;

    if (bindfailed) (*ld.fc)++;
    else *ld.fc = 0;

    if (!nd.fdnsConn[0].ready) {
      LSM(LOG_NOTICE, "no interface ready, waiting...")
      continue;
    }

    for (i=0; i < MAX_SERVERS && net.listenServers[i]; i++) {
      for (j=0; j < MAX_SERVERS; j++) {
        if (nd.fdnsConn[j].server == net.listenServers[i]) {
          LSM(LOG_INFO, "listening on %s", IP2String(lb.tmp, net.listenServers[i]));
          break;
        }
      }
    }

  } while (dCWait(FDNS_IDX));

  _endthread();
  return;
}

void* main(void* arg) {

  SERVICESTART(FDNS_IDX)

    if (!nd.fdnsConn[0].ready) { Sleep(1000); continue; }
    if (net.refresh) { Sleep(1000); continue; }

    FD_ZERO(&readfds);

    for (i=0; i < MAX_SERVERS && nd.fdnsConn[i].ready; i++)
      FD_SET(nd.fdnsConn[i].sock, &readfds);

    if (select(nd.maxFD, &readfds, NULL, NULL, &tv) != SOCKET_ERROR) {

      ld.t = time(NULL);

      if (*ld.nr) {

        *ld.ib = true;

        for (i=0; i < MAX_SERVERS && nd.fdnsConn[i].ready; i++)
          if (FD_ISSET(nd.fdnsConn[i].sock, &readfds)) sendResponse(i);

      }

    } else ld.t = time(NULL);

  SERVICEEND

}

}
#endif
