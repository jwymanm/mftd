#include "core.h"
#include "tunnel.h"

#if TUNNEL

bool tunnel_running = false;

int buffer_size = 4096;

struct struct_rc rc;

int tunnel_cleanup(int sd, int rsd, int exitthread) {
  if (sd) { shutdown(sd, 2); Sleep(1000); closesocket(sd); }
  if (rsd) { shutdown(rsd, 2); Sleep(1000); closesocket(rsd); }
  if (exitthread) {
    WSACleanup();
    tunnel_running = false;
    pthread_exit(NULL);
  } else return 0;
}

void* tunnel(void* arg) {

  tunnel_running = true;
  int optval;
  WSADATA info;

  if (WSAStartup(MAKEWORD(1, 1), &info) != 0) {
    debug(0, "Tunnel: WSAStartup()", NULL);
    tunnel_running = false;
    pthread_exit(NULL);
  }

  if (build_server() == 1) { tunnel_cleanup(0,0,1); }

  debug(0, "Tunnel: Status: Running", NULL);

  while (tunnel_running)
  { if (wait_for_clients() == 0) if (build_tunnel() == 0) use_tunnel(); }

  tunnel_cleanup(rc.server_socket, rc.client_socket, 1);
}

int build_server(void) {

  int optval;

  memset(&rc.server_addr, 0, sizeof(rc.server_addr));

  rc.server_addr.sin_family = AF_INET;
  rc.server_addr.sin_addr.s_addr = inet_addr(config.adptrip);
  rc.server_addr.sin_port = htons(config.lport);
  rc.server_socket = socket(AF_INET, SOCK_STREAM, 0);

  if (rc.server_socket < 0) {
    perror("build_server: socket()");
    printf("%d\r\n", WSAGetLastError());
    tunnel_cleanup(0,0,1);
  }

  optval = 1;

  if (setsockopt(rc.server_socket, SOL_SOCKET, SO_REUSEADDR, (const char *) &optval, sizeof(optval)) < 0) {
    perror("build_server: setsockopt(SO_REUSEADDR)");
    printf("%d\r\n", WSAGetLastError());
    tunnel_cleanup(rc.server_socket,0,1);
  }

  if (bind(rc.server_socket, (struct sockaddr *) &rc.server_addr, sizeof(rc.server_addr)) < 0) {
    perror("build_server: bind()");
    printf("%d\r\n", WSAGetLastError());
    tunnel_cleanup(rc.server_socket,0,1);
  }

  if (listen(rc.server_socket, 1) < 0) {
    perror("build_server: listen()");
    printf("%d\r\n", WSAGetLastError());
    tunnel_cleanup(rc.server_socket,0,1);
  }

  return 0;
}

int wait_for_clients(void) {

  int client_addr_size;
  client_addr_size = sizeof(struct sockaddr_in);
  rc.client_socket = accept(rc.server_socket, (struct sockaddr *) &rc.client_addr, &client_addr_size);

  if (rc.client_socket < 0) {
    if (errno != EINTR) { perror("wait_for_clients: accept()"); }
    return 1;
  }

  if (config.logging) {
    //printf("Tunnel: %s: request from %s\n", get_current_timestamp(), inet_ntoa(rc.client_addr.sin_addr));
    debug(0, "Tunnel: request from ", (void *) inet_ntoa(rc.client_addr.sin_addr));
  }

  return 0;
}


int build_tunnel(void) {

  rc.remote_host = gethostbyname(config.host);

  if (rc.remote_host == NULL) {
    perror("build_tunnel: gethostbyname()");
    return 1;
  }

  memset(&rc.remote_addr, 0, sizeof(rc.remote_addr));
  rc.remote_addr.sin_family = AF_INET;
  rc.remote_addr.sin_port = htons(config.rport);
  memcpy(&rc.remote_addr.sin_addr.s_addr, rc.remote_host->h_addr, rc.remote_host->h_length);
  rc.remote_socket = socket(AF_INET, SOCK_STREAM, 0);

  if (rc.remote_socket < 0) {
    perror("build_tunnel: socket()");
    return 1;
  }

  if (connect(rc.remote_socket, (struct sockaddr *) &rc.remote_addr, sizeof(rc.remote_addr)) < 0) {
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
    FD_SET(rc.client_socket, &io);
    FD_SET(rc.remote_socket, &io);

    memset(buffer, 0, sizeof(buffer));

    if (select(fd(), &io, NULL, NULL, NULL) < 0) {
      perror("use_tunnel: select()");
      break;
    }

    if (FD_ISSET(rc.client_socket, &io)) {

      int count = recv(rc.client_socket, buffer, sizeof(buffer), 0);

// TODO Proper Cleanup
			if (count < 0) {
				perror("use_tunnel: recv(rc.client_socket)");
	//			tunnel_cleanup(rc.client_socket, rc.remote_socket, 0);
				closesocket(rc.client_socket);
				closesocket(rc.remote_socket);
	return 1;
      }

			if (count == 0) {
//				return tunnel_cleanup(rc.client_socket, rc.remote_socket, 0);
				closesocket(rc.client_socket);
				closesocket(rc.remote_socket);
				return 0;
      }

			send(rc.remote_socket, buffer, count, 0);

			if (config.logging) {
				printf("> %s > ", get_current_timestamp());
				fwrite(buffer, sizeof(char), count, stdout);
				fflush(stdout);
      }
    }

		if (FD_ISSET(rc.remote_socket, &io)) {

			int count = recv(rc.remote_socket, buffer, sizeof(buffer), 0);

			if (count < 0) {
				perror("use_tunnel: recv(rc.remote_socket)");
				//tunnel_cleanup(rc.client_socket, rc.remote_socket, 0);
				closesocket(rc.client_socket);
				closesocket(rc.remote_socket);
				return 1;
      }

			if (count == 0) {
				//return tunnel_cleanup(rc.client_socket, rc.remote_socket, 0);
				closesocket(rc.client_socket);
				closesocket(rc.remote_socket);
				return 0;
      }

			send(rc.client_socket, buffer, count, 0);

			if (config.logging) {
				fwrite(buffer, sizeof(char), count, stdout);
				fflush(stdout);
      }
    }
  } while (tunnel_running);

  return 0;
}

int fd(void) {
	unsigned int fd = rc.client_socket;
	if (fd < rc.remote_socket) { fd = rc.remote_socket; }
	return fd + 1;
}

char *get_current_timestamp(void)
{
	static char date_str[20];
	time_t date;
	time(&date);
	strftime(date_str, sizeof(date_str), "%Y-%m-%d %H:%M:%S", localtime(&date));
	return date_str;
}

#endif
