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
  char msg[DNSMSG_SIZE];
  char log[256];
} LocalBuffers;

typedef struct {
  struct sockaddr_in sa;
  struct sockaddr_in ca;
} NetworkData;

void cleanup(int et);
void* main(void* arg);

}
#endif
