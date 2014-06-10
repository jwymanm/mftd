// fdns.h

#if FDNS

#define DNSPORT 53
#define DNSMSG_SIZE 512

extern "C" bool fdns_running;

namespace fdns {

typedef struct {
  int sockLen;
  int flags;
  int bytes;
  SOCKADDR_IN remote;
} Request;

typedef struct {
  char msg[DNSMSG_SIZE];
  char tmp[512];
  char log[256];
} LocalBuffers;

typedef struct {
  time_t t;
  char* ip4str;
  int ip4[4];
} LocalData;

typedef struct {
  ConnType fdnsConn[MAX_SERVERS];
  Request req;
  SOCKET maxFD;
} NetworkData;

void sendResponse(MYBYTE sndx);
void cleanup(int et);
void __cdecl init(void* arg);
void* main(void* arg);

}
#endif
