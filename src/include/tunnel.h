#if TUNNEL

#define TUNNEL_TIDX FDNS + MONITOR
#define BUFFER_SIZE 4096

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
  char data[BUFFER_SIZE];
  char log[256];
  fd_set io;
} LocalBuffers;

void* main(void* arg);
int cleanup(int et);
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
