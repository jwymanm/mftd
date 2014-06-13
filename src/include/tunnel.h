#if TUNNEL

#define BUFFER_SIZE 4096

namespace tunnel {

typedef struct {
  char data[BUFFER_SIZE];
  char log[256];
  char tmp[512];
  fd_set io;
} LocalBuffers;

typedef struct {
  bool transm;
  bool active;
  time_t t;
  char* sn;
  bool* ir;
  bool* ib;
  bool* nr;
  int* fc;
} LocalData;

typedef struct {
  ConnType tunConn[MAX_SERVERS];
  ConnType remConn;
  ConnType cliConn;
  hostent* remote_host;
  SOCKET maxFD;
} NetworkData;

void cleanup(int et);
void stop();
void start();
bool buildSP(void* arg);
int fd();
void remoteData();
void clientData();
int useTunnel();
int buildTunnel();
int handleClient(MYBYTE sndx);
void __cdecl init(void* arg);
void* main(void* arg);

}
#endif
