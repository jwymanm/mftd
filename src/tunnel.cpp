#if TUNNEL

#include "core.h"
#include "net.h"
#include "tunnel.h"

bool tunnel_running = false;

namespace tunnel {

Sockets s;
LocalBuffers lb;
NetworkData nd;

int cleanup(int et) {
  if (s.server) { shutdown(s.server, 2); closesocket(s.server); }
  if (s.client) { shutdown(s.client, 2); closesocket(s.client); }
  if (s.remote) { shutdown(s.remote, 2); closesocket(s.remote); }
  if (et) {
    tunnel_running = false;
    Sleep(1000);
    logMesg("Tunnel stopped", LOG_INFO);
    pthread_exit(NULL);
  } else {
    logMesg("Tunnel closed network connections", LOG_INFO);
    return 0;
  }
}

void* main(void* arg) {

  tunnel_running = true;

  logMesg("Tunnel starting", LOG_INFO);

  net.failureCounts[TUNNEL_IDX] = 0;

  while(!net.ready) Sleep(1000);

  build_server();

  do {
    if (!net.ready) { Sleep(1000); continue; }
    if (wait_for_clients()) if (build_tunnel()) use_tunnel();
  }
  while (tunnel_running);

  cleanup(1);
}

int build_server(void) {

  int optval = 1;

  memset(&nd.sa, 0, sizeof(nd.sa));

  nd.sa.sin_family = AF_INET;
  nd.sa.sin_addr.s_addr = inet_addr(config.adptrip);
  nd.sa.sin_port = htons(config.lport);
  s.server = socket(AF_INET, SOCK_STREAM, 0);

  if (s.server == INVALID_SOCKET) {
    sprintf(lb.log, "Tunnel: build_server socket error %d", WSAGetLastError());
    logMesg(lb.log, LOG_DEBUG);
    net.failureCounts[TUNNEL_IDX]++;
    cleanup(1);
  }

  if (setsockopt(s.server, SOL_SOCKET, SO_REUSEADDR, (const char *) &optval, sizeof(optval)) < 0) {
    sprintf(lb.log, "Tunnel: build_server setsockopt(SO_REUSEADDR) error %d", WSAGetLastError());
    logMesg(lb.log, LOG_DEBUG);
    net.failureCounts[TUNNEL_IDX]++;
    cleanup(1);
  }

  if (bind(s.server, (struct sockaddr *) &nd.sa, sizeof(nd.sa)) == SOCKET_ERROR) {
    sprintf(lb.log, "Tunnel: build_server: bind() socket error %d", WSAGetLastError());
    logMesg(lb.log, LOG_DEBUG);
    net.failureCounts[TUNNEL_IDX]++;
    cleanup(1);
  }

  if (listen(s.server, 1) == SOCKET_ERROR) {
    sprintf(lb.log, "Tunnel: build_server: listen() socket error %d", WSAGetLastError());
    logMesg(lb.log, LOG_DEBUG);
    net.failureCounts[TUNNEL_IDX]++;
    cleanup(1);
  }

  return 0;
}

int wait_for_clients(void) {

  int client_addr_size;
  client_addr_size = sizeof(struct sockaddr_in);
  s.client = accept(s.server, (struct sockaddr *) &nd.ca, &client_addr_size);

  if (!tunnel_running) return 0;

  if (s.client == INVALID_SOCKET) {
    logMesg("Tunnel: wait_for_clients accept() error", LOG_DEBUG);
    return 0;
  }

  if (config.logging) {
    sprintf(lb.log, "Tunnel: request from %s", inet_ntoa(nd.ca.sin_addr));
    logMesg(lb.log, LOG_NOTICE);
  }

  return 1;
}

int build_tunnel(void) {

  lb.remote_host = gethostbyname(config.host);

  if (lb.remote_host == NULL) {
    logMesg("Tunnel: build_tunnel: gethostbyname()", LOG_DEBUG);
    return 0;
  }

  memset(&nd.ra, 0, sizeof(nd.ra));
  nd.ra.sin_family = AF_INET;
  nd.ra.sin_port = htons(config.rport);
  memcpy(&nd.ra.sin_addr.s_addr, lb.remote_host->h_addr, lb.remote_host->h_length);
  s.remote = socket(AF_INET, SOCK_STREAM, 0);

  if (s.remote == INVALID_SOCKET) {
    logMesg("Tunnel: build_tunnel: socket()", LOG_DEBUG);
    return 0;
  }

  if (connect(s.remote, (struct sockaddr *) &nd.ra, sizeof(nd.ra)) == SOCKET_ERROR) {
    logMesg("Tunnel: build_tunnel: connect()", LOG_DEBUG);
    return 0;
  }

  return 1;
}

int use_tunnel(void) {

  do {

    FD_ZERO(&lb.io);
    FD_SET(s.client, &lb.io);
    FD_SET(s.remote, &lb.io);

    memset(lb.data, 0, sizeof(lb.data));

    if (select(fd(), &lb.io, NULL, NULL, NULL) == SOCKET_ERROR) {
      logMesg("Tunnel: use_tunnel: select()", LOG_DEBUG);
      break;
    }

    if (FD_ISSET(s.client, &lb.io)) {

      int count = recv(s.client, lb.data, sizeof(lb.data), 0);

      if (count == SOCKET_ERROR) {
        logMesg("Tunnel: use_tunnel: recv(s.client)", LOG_DEBUG);
        closesocket(s.client);
        closesocket(s.remote);
        return 1;
      }

      if (count == 0) {
        closesocket(s.client);
        closesocket(s.remote);
        return 0;
      }

      send(s.remote, lb.data, count, 0);

      if (config.logging == LOG_DEBUG) {
        printf("> %s > ", get_current_timestamp());
        fwrite(lb.data, sizeof(char), count, stdout);
        fflush(stdout);
      }
    }

    if (FD_ISSET(s.remote, &lb.io)) {

      int count = recv(s.remote, lb.data, sizeof(lb.data), 0);

      if (count == SOCKET_ERROR) {
        logMesg("Tunnel: use_tunnel: recv(rc.remote_socket)", LOG_DEBUG);
        closesocket(s.client);
        closesocket(s.remote);
        return 1;
      }

      if (count == 0) {
        closesocket(s.client);
        closesocket(s.remote);
        return 0;
      }

      send(s.client, lb.data, count, 0);

      if (config.logging) {
        fwrite(lb.data, sizeof(char), count, stdout);
        fflush(stdout);
      }
    }
  } while (tunnel_running);

  return 0;
}

int fd(void) {
	unsigned int fd = s.client;
	if (fd < s.remote) { fd = s.remote; }
  //printf("fds = c %d r %d\r\n", s.client, s.remote);
	return fd + 1;
}

char *get_current_timestamp(void) {
	static char date_str[20];
	time_t date;
	time(&date);
	strftime(date_str, sizeof(date_str), "%Y-%m-%d %H:%M:%S", localtime(&date));
	return date_str;
}

}
#endif
