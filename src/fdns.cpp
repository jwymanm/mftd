#if FDNS

#include "core.h"
#include "net.h"
#include "fdns.h"

bool fdns_running = false;

namespace fdns {

Sockets s;
LocalBuffers lb;

void cleanup(int et) {
  if (s.server) { shutdown(s.server, 2); Sleep(1000); closesocket(s.server); }
  if (et) {
    fdns_running = false;
    logMesg("FDNS stopped", LOG_INFO);
    pthread_exit(NULL);
  } else return;
}

void *main(void *arg) {

  fdns_running = true;

  int len, flags, ip4[4], n;
  char *ip4str, *m = lb.msg;
  ip4str = strdup(config.fdnsip);

  logMesg("FDNS starting", LOG_INFO);

  for (int i=0; i < 4; i++) { ip4[i] = atoi(strsep(&ip4str, ".")); }

  s.server = socket(AF_INET, SOCK_DGRAM, 0);

  if (setsockopt(s.server, SOL_SOCKET, SO_REUSEADDR, "1", sizeof(int)) == -1) {
    net.failureCounts[FDNS_TIDX]++;
    cleanup(1);
  }

  if (s.server == INVALID_SOCKET) {
    sprintf(lb.log, "FDNS: cannot open socket, error %u", WSAGetLastError());
    logMesg(lb.log, LOG_DEBUG);
    net.failureCounts[FDNS_TIDX]++;
    cleanup(1);
  }

  memset(&lb.sa, 0, sizeof(lb.sa));
  memset(&lb.ca, 0, sizeof(lb.ca));

  lb.sa.sin_family = AF_INET;
  lb.sa.sin_addr.s_addr = inet_addr(config.adptrip);//htonl(INADDR_ANY);
  lb.sa.sin_port = htons(DNSPORT);

  if (bind(s.server, (struct sockaddr *) &lb.sa, sizeof(lb.sa)) == SOCKET_ERROR) {
    sprintf(lb.log, "FDNS: cannot bind dns port, error %u", WSAGetLastError());
    logMesg(lb.log, LOG_DEBUG);
    net.failureCounts[FDNS_TIDX]++;
    cleanup(1);
  }

  net.failureCounts[FDNS_TIDX] = 0;

  len = sizeof(lb.ca);
  flags = 0;

  do {
    n = recvfrom(s.server, m, DNSMSG_SIZE, flags, (struct sockaddr *) &lb.ca, &len);
    if (n < 0) continue; else logMesg("FDNS: client connected", LOG_INFO);
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
    m[n++]=ip4[0]; m[n++]=ip4[1]; m[n++]=ip4[2]; m[n++]=ip4[3]; // IP
    // Send the answer
    sendto(s.server, m, n, flags, (struct sockaddr *) &lb.ca, len);
  } while (fdns_running);

  cleanup(1);
}

}
#endif
