#if HTTP

#define HTTP_SECSIZE 2048
#define HTTP_ISLINGER 1
#define HTTP_LOFFT    12

extern "C" bool http_running;

namespace http {

typedef struct {
  char send200[256];
  char send403[256];
  char send404[256];
} Codes;

typedef struct {
  Codes code;
  int memSize;
  int bytes;
  char* dp;
} Response;

typedef struct {
  SOCKET sock;
  SOCKADDR_IN remote;
  socklen_t sockLen;
  linger ling;
  int bytes;
  time_t t;
} Request;

typedef struct {
  char htmlTitle[256];
  char htmlStart[256];
  char bodyStart[256];
  char td200[256];
  char bodyEnd[256];
  char htmlEnd[256];
} HTML;

typedef struct {
  Request req;
  Response res;
  HTML html;
} Data;

typedef struct {
  char log[256];
  char tmp[512];
  char http[1024];
  char htm[_MAX_PATH];
} LocalBuffers;

typedef struct {
  ConnType httpConn;
  MYDWORD httpClients[8];
  SOCKET maxFD;
  bool ready;
  bool busy;
} NetworkData;

Data* initDP(const char* name, void* lpParam, int memSize);
void buildHP(Data* h);
void procHTTP(Data* h);
void sendHTTP(void* lpParam);
void cleanup(int et);
void __cdecl init(void *lpParam);
void* main(void* arg);

}
#endif
