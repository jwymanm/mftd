#if FDNS

#define FDNS_TIDX MONITOR
#define DNSPORT 53
#define DNSMSG_SIZE 512

extern "C" bool fdns_running;
extern "C" int fdns_sd;

void fdns_cleanup(int sd, int exitthread);
void* fdns(void* arg);

#endif
