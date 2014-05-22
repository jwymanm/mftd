#ifndef TCPTUNNEL_H
#define TCPTUNNEL_H

int tunnel_cleanup(int sd, int rsd, int exitthread);
void* tunnel(void* arg);
int build_server(void);
int wait_for_clients(void);
void handle_client(void);
void handle_tunnel(void);
int build_tunnel(void);
int use_tunnel(void);
int fd(void);
char *get_current_timestamp(void);

struct struct_rc {
	unsigned int server_socket;
	unsigned int client_socket;
	unsigned int remote_socket;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	struct sockaddr_in remote_addr;
	struct hostent *remote_host;
};

#endif
