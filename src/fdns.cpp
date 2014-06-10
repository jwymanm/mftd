// fdns.cpp

#if FDNS

#include "core.h"
#include "util.h"
#include "net.h"
#include "fdns.h"
#include "monitor.h"
#include "http.h"

bool fdns_running = false;

namespace fdns {

LocalBuffers lb;
LocalData ld;
NetworkData nd;

void sendResponse(MYBYTE sndx) {

  char* m = lb.msg;
  int n,* i = ld.ip4;
  Request* r = &nd.req;

  memset(r, 0, sizeof(Request));

  r->sockLen = sizeof(r->remote);
  r->flags = 0;
  r->bytes = recvfrom(nd.fdnsConn[sndx].sock, m, DNSMSG_SIZE, r->flags, (sockaddr*) &r->remote, &r->sockLen);

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

void cleanup(int et) {
  int i; bool closed = false;
  for (i=0; i < MAX_SERVERS && nd.fdnsConn[i].loaded; i++)
    if (nd.fdnsConn[i].ready) {
      closesocket(nd.fdnsConn[i].sock);
      closed = true;
    }
  if (closed) logMesg("FDNS cleared network connections", LOG_DEBUG);
  if (et) {
    fdns_running = false;
    Sleep(1000);
    logMesg("FDNS stopped", LOG_INFO);
    pthread_exit(NULL);
  }
  return;
}

void __cdecl init(void* arg) {

  bool bindfailed;
  int i, j, nRet;

  ld.ip4str = strdup(config.fdnsip);

  for (int i=0; i < 4; i++)
    ld.ip4[i] = atoi(strsep(&ld.ip4str, "."));

  do {

    cleanup(0);

    memset(&nd, 0, sizeof(nd));

    bindfailed = false;

    for (i=0, j=0; j < MAX_SERVERS && net.listenServers[j]; j++) {

      nd.fdnsConn[i].sock = socket(AF_INET, SOCK_DGRAM, 0);

      if (nd.fdnsConn[i].sock == INVALID_SOCKET) {
        sprintf(lb.log, "FDNS socket error %u", WSAGetLastError());
        logMesg(lb.log, LOG_NOTICE);
        bindfailed = true;
        continue;
      }

      nd.fdnsConn[i].addr.sin_family = AF_INET;
      nd.fdnsConn[i].addr.sin_addr.s_addr = net.listenServers[j];
      nd.fdnsConn[i].addr.sin_port = htons(DNSPORT);

      setsockopt(nd.fdnsConn[i].sock, SOL_SOCKET, SO_REUSEADDR, "1", sizeof(int));
      nRet = bind(nd.fdnsConn[i].sock, (sockaddr*) &nd.fdnsConn[i].addr, sizeof(struct sockaddr_in));

      if (nRet == SOCKET_ERROR) {
        sprintf(lb.log, "FDNS: cannot bind dns port, error %u", WSAGetLastError());
        logMesg(lb.log, LOG_NOTICE);
        bindfailed = true;
        closesocket(nd.fdnsConn[i].sock);
        continue;
      }

      if (nd.maxFD < nd.fdnsConn[i].sock) nd.maxFD = nd.fdnsConn[i].sock;

      nd.fdnsConn[i].loaded = true;
      nd.fdnsConn[i].ready = true;
      nd.fdnsConn[i].server = net.listenServers[j];
      nd.fdnsConn[i].mask = net.listenMasks[j];
      nd.fdnsConn[i].port = DNSPORT;

      i++;
    }

    nd.maxFD++;

    if (bindfailed) net.failureCounts[FDNS_IDX]++;
    else net.failureCounts[FDNS_IDX] = 0;

    if (!nd.fdnsConn[0].ready) {
      logMesg("FDNS no interface ready, waiting...", LOG_NOTICE);
      continue;
    }

    for (i=0; i < MAX_SERVERS && net.listenServers[i]; i++) {
      for (j=0; j < MAX_SERVERS; j++) {
        if (nd.fdnsConn[j].server == net.listenServers[i]) {
          sprintf(lb.log, "FDNS listening on: %s", IP2String(lb.tmp, net.listenServers[i]));
          logMesg(lb.log, LOG_INFO);
          break;
        }
      }
    }

  } while (dCWait(FDNS_IDX));

  _endthread();
  return;
}

void* main(void *arg) {

  fdns_running = true;

  logMesg("FDNS starting", LOG_INFO);

  _beginthread(init, 0, 0);

  int i;
  fd_set readfds;
  timeval tv = { 20, 0 };

  do {

    net.busy[FDNS_IDX] = false;

    if (!nd.fdnsConn[0].ready) { Sleep(1000); continue; }
    if (!net.ready[FDNS_IDX]) { Sleep(1000); continue; }
    if (net.refresh) { Sleep(1000); continue; }

    FD_ZERO(&readfds);

    for (i=0; i < MAX_SERVERS && nd.fdnsConn[i].ready; i++)
      FD_SET(nd.fdnsConn[i].sock, &readfds);

    if (select(nd.maxFD, &readfds, NULL, NULL, &tv) != SOCKET_ERROR) {

      ld.t = time(NULL);

      if (net.ready[FDNS_IDX]) {

        net.busy[FDNS_IDX] = true;

        for (i=0; i < MAX_SERVERS && nd.fdnsConn[i].ready; i++)
          if (FD_ISSET(nd.fdnsConn[i].sock, &readfds)) sendResponse(i);

      }

    } else ld.t = time(NULL);

  } while (fdns_running);

  free(ld.ip4str);

  cleanup(1);
}

}
#endif
