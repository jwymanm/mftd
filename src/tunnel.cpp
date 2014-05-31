#if TUNNEL

#include "core.h"
#include "tunnel.h"

bool tunnel_running = false;

namespace tunnel {

Sockets s;
LocalBuffers lb;

int buffer_size = 4096;

int cleanup(int et) {
  if (s.server) { shutdown(s.server, 2); closesocket(s.server); }
  if (s.client) { shutdown(s.client, 2); closesocket(s.client); }
  if (s.remote) { shutdown(s.remote, 2); closesocket(s.remote); }
  if (et) { tunnel_running = false; pthread_exit(NULL); } else return 0;
}

void* main(void* arg) {

  int optval;

  tunnel_running = true;

  if (build_server() == 1) cleanup(1);

  logMesg("Tunnel initializing..", LOG_INFO);

  do { if (wait_for_clients() == 0) if (build_tunnel() == 0) use_tunnel(); }
  while (tunnel_running);

  cleanup(1);
}

int build_server(void) {

  int optval;

  memset(&lb.sa, 0, sizeof(lb.sa));

  lb.sa.sin_family = AF_INET;
  lb.sa.sin_addr.s_addr = inet_addr(config.adptrip);
  lb.sa.sin_port = htons(config.lport);
  s.server = socket(AF_INET, SOCK_STREAM, 0);

  if (s.server < 0) {
    perror("build_server: socket()");
    printf("%d\r\n", WSAGetLastError());
    cleanup(1);
  }

  optval = 1;

  if (setsockopt(s.server, SOL_SOCKET, SO_REUSEADDR, (const char *) &optval, sizeof(optval)) < 0) {
    perror("build_server: setsockopt(SO_REUSEADDR)");
    printf("%d\r\n", WSAGetLastError());
    cleanup(1);
  }

  if (bind(s.server, (struct sockaddr *) &lb.sa, sizeof(lb.sa)) < 0) {
    perror("build_server: bind()");
    printf("%d\r\n", WSAGetLastError());
    cleanup(1);
  }

  if (listen(s.server, 1) < 0) {
    perror("build_server: listen()");
    printf("%d\r\n", WSAGetLastError());
    cleanup(1);
  }

  return 0;
}

int wait_for_clients(void) {

  int client_addr_size;
  client_addr_size = sizeof(struct sockaddr_in);
  s.client = accept(s.server, (struct sockaddr *) &lb.ca, &client_addr_size);

  if (s.client < 0) {
    if (errno != EINTR) { perror("wait_for_clients: accept()"); }
    return 1;
  }

  if (config.logging) {
    sprintf(lb.log, "Tunnel: request from %s", inet_ntoa(lb.ca.sin_addr));
    logMesg(lb.log, LOG_NOTICE);
  }

  return 0;
}


int build_tunnel(void) {

  lb.remote_host = gethostbyname(config.host);

  if (lb.remote_host == NULL) {
    perror("build_tunnel: gethostbyname()");
    return 1;
  }

  memset(&lb.ra, 0, sizeof(lb.ra));
  lb.ra.sin_family = AF_INET;
  lb.ra.sin_port = htons(config.rport);
  memcpy(&lb.ra.sin_addr.s_addr, lb.remote_host->h_addr, lb.remote_host->h_length);
  s.remote = socket(AF_INET, SOCK_STREAM, 0);

  if (s.remote < 0) {
    perror("build_tunnel: socket()");
    return 1;
  }

  if (connect(s.remote, (struct sockaddr *) &lb.ra, sizeof(lb.ra)) < 0) {
    perror("build_tunnel: connect()");
    return 1;
  }

  return 0;
}

int use_tunnel(void) {

  char buffer[buffer_size];
  fd_set io;

  do {

    FD_ZERO(&io);
    FD_SET(s.client, &io);
    FD_SET(s.remote, &io);

    memset(buffer, 0, sizeof(buffer));

    if (select(fd(), &io, NULL, NULL, NULL) < 0) {
      perror("use_tunnel: select()");
      break;
    }

    if (FD_ISSET(s.client, &io)) {

      int count = recv(s.client, buffer, sizeof(buffer), 0);

      if (count < 0) {
        perror("use_tunnel: recv(rc.client_socket)");
        closesocket(s.client);
        closesocket(s.remote);
        return 1;
      }

      if (count == 0) {
        closesocket(s.client);
        closesocket(s.remote);
        return 0;
      }

      send(s.remote, buffer, count, 0);

      if (config.logging) {
        printf("> %s > ", get_current_timestamp());
        fwrite(buffer, sizeof(char), count, stdout);
        fflush(stdout);
      }
    }

    if (FD_ISSET(s.remote, &io)) {

      int count = recv(s.remote, buffer, sizeof(buffer), 0);

      if (count < 0) {
        perror("use_tunnel: recv(rc.remote_socket)");
        closesocket(s.client);
        closesocket(s.remote);
        return 1;
      }

      if (count == 0) {
        closesocket(s.client);
        closesocket(s.remote);
        return 0;
      }

      send(s.client, buffer, count, 0);

      if (config.logging) {
        fwrite(buffer, sizeof(char), count, stdout);
        fflush(stdout);
      }
    }
  } while (tunnel_running);

  return 0;
}

int fd(void) {
	unsigned int fd = s.client;
	if (fd < s.remote) { fd = s.remote; }
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
