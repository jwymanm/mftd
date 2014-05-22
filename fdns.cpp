
#include "core.h"
#include "fdns.h"

bool fdns_running = false;
bool fdns_docleanup = false;
int fdns_sd;

void fdns_cleanup(int sd, int exitthread) {
  if (sd) { shutdown(sd, 2); closesocket(sd); }
  fdns_running = false;
  if (exitthread) pthread_exit(NULL);
  else return;
}

void *fdns(void *arg) {

  fdns_running = true;
  int wsaerr, rc, len, flags, ip4[4];
  char msg[DNSMSG_SIZE];
  char *ip4str;
  struct sockaddr_in addr, server;
  WSADATA wsaData;

  ip4str = strdup(config.dnsipaddr);
  for (int i=0; i < 4; i++) { ip4[i] = atoi(strsep(&ip4str, ".")); }

  wsaerr = WSAStartup(MAKEWORD(1, 1), &wsaData);

  if (wsaerr != 0) {
    debug(0, "Winsock dll was not found!\n", NULL);
    fdns_cleanup(0,1);
  } else {
    debug(0, "FDNS Status: ", (void *) wsaData.szSystemStatus);
  }

  fdns_sd = socket(AF_INET, SOCK_DGRAM, 0);

  if (setsockopt(fdns_sd, SOL_SOCKET, SO_REUSEADDR, "1", sizeof(int)) == -1) {
    fdns_cleanup(fdns_sd,1);
  }

  if(fdns_sd == INVALID_SOCKET) {
    wsaerr = WSAGetLastError();
    debug(DEBUG_SE, "cannot open socket, error: ", &wsaerr);
    fdns_cleanup(fdns_sd,1);
  }

	memset(&server, 0, sizeof(server));
	memset(&addr, 0, sizeof(addr));

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(config.ipaddr);//htonl(INADDR_ANY);
  server.sin_port = htons(DNSPORT);

  rc = bind(fdns_sd, (struct sockaddr *) &server,sizeof(server));

  if (rc < 0) {
    wsaerr = WSAGetLastError();
    debug(DEBUG_SE, "FDNS: cannot bind dns port, error: ", &wsaerr);
    fdns_cleanup(fdns_sd,1);
  }

  len = sizeof(addr);
  flags = 0;

  while (1) {
    int n = recvfrom(fdns_sd, msg, DNSMSG_SIZE, flags, (struct sockaddr *) &addr, &len);
    if (n < 0) { continue; }
    // Same Id
    msg[2]=0x81;msg[3]=0x80; // Change Opcode and flags
    msg[6]=0;msg[7]=1; // One answer
    msg[8]=0;msg[9]=0; // NSCOUNT
    msg[10]=0;msg[11]=0; // ARCOUNT
    // Keep request in message and add answer
    msg[n++]=0xC0;msg[n++]=0x0C; // Offset to the domain name
    msg[n++]=0x00;msg[n++]=0x01; // Type 1
    msg[n++]=0x00;msg[n++]=0x01; // Class 1
    msg[n++]=0x00;msg[n++]=0x00;msg[n++]=0x00;msg[n++]=0x3c; // TTL
    msg[n++]=0x00;msg[n++]=0x04; // Size --> 4
    msg[n++]=ip4[0];msg[n++]=ip4[1];msg[n++]=ip4[2];msg[n++]=ip4[3]; // IP
    // Send the answer
    sendto(fdns_sd,msg,n,flags,(struct sockaddr *)&addr,len);
 }
 fdns_cleanup(fdns_sd,1);
}
