#if TUNNEL

#define TUNNEL_TIDX FDNS + MONITOR

extern "C" bool tunnel_running;

namespace tunnel {

typedef struct {
  unsigned int server;
  unsigned int client;
  unsigned int remote;
} Sockets;

typedef struct {
  struct sockaddr_in sa;
  struct sockaddr_in ca;
  struct sockaddr_in ra;
  struct hostent *remote_host;
  char log[256];
} LocalBuffers;

void* main(void* arg);
int cleanup(int sd, int rsd, int exitthread);
int build_server(void);
int wait_for_clients(void);
void handle_client(void);
void handle_tunnel(void);
int build_tunnel(void);
int use_tunnel(void);
int fd(void);
char *get_current_timestamp(void);

}
#endif
