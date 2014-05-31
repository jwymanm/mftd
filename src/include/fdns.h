#if FDNS

#define FDNS_TIDX MONITOR
#define DNSPORT 53
#define DNSMSG_SIZE 512

extern "C" bool fdns_running;

namespace fdns {

typedef struct {
  unsigned int server;
  unsigned int listen;
} Sockets;

typedef struct {
  struct sockaddr_in sa;
  struct sockaddr_in ca;
  char msg[DNSMSG_SIZE];
} LocalBuffers;

//extern "C" Sockets s;

void cleanup(int et);
void* main(void* arg);

}

#endif
