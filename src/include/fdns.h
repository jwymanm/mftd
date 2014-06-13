// fdns.h

#if FDNS

#define FDNSPORT 53
#define FDNSMSG_SIZE 512

namespace fdns {

typedef struct {
  int sockLen;
  int flags;
  int bytes;
  SOCKADDR_IN remote;
} Request;

typedef struct {
  char msg[FDNSMSG_SIZE];
  char tmp[512];
  char log[256];
} LocalBuffers;

typedef struct {
  time_t t;
  int ip4[4];
  char ip4str[20];
  char* sn;
  bool* ir;
  bool* ib;
  bool* nr;
  int* fc;
} LocalData;

typedef struct {
  ConnType fdnsConn[MAX_SERVERS];
  Request req;
  SOCKET maxFD;
} NetworkData;

void cleanup(int et);
void stop();
void start();
bool buildSP(void* arg);
void sendResponse(MYBYTE sndx);
void __cdecl init(void* arg);
void* main(void* arg);

}
#endif
