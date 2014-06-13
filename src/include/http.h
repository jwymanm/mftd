#if HTTP

#define HTTP_SECSIZE 2048
#define HTTP_ISLINGER 1
#define HTTP_LOFFT    12

#define HTTP_SPHEAD(x) \
  http::Data* h = http::initDP(ld.sn, arg, x); \
  if (!h) return false; \
  char* fp = h->res.dp; \
  char* maxData = h->res.dp + h->res.memSize; \
  fp += sprintf(fp, "<h3>%s</h3>\n", ld.sn);
#define HTTP_SPFOOT \
  h->res.bytes = fp - h->res.dp; \
  return true;

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
  char htmlStart[256];
  char htmlHead[5000];
  char htmlTitle[256];
  char htmlStyle[4096];
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
  char* sn;
  bool* ir;
  bool* ib;
  bool* nr;
  int* fc;
} LocalData;

typedef struct {
  ConnType httpConn;
  MYDWORD httpClients[8];
  SOCKET maxFD;
  bool ready;
  bool busy;
} NetworkData;

void cleanup(int et);
void stop();
void start();
bool buildSP(void* arg);
Data* initDP(const char* sname, void* arg, int memSize);
void buildHP(Data* h);
void procHTTP(Data* h);
void sendHTTP(void* arg);
void __cdecl init(void* arg);
void* main(void* arg);

}
#endif
