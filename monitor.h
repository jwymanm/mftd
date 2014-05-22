

#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3
#define MON_TO 5000

void *monAdptr(void *arg);
int getAdptrAddr();
int getAdptrInfo();
int setAdptrIP();
int lkupIF();

/*
typedef union _NET_LUID {
  ULONG64 Value;
  struct {
    ULONG64 Reserved  :24;
    ULONG64 NetLuidIndex  :24;
    ULONG64 IfType  :16;
  } Info;
} NET_LUID, *PNET_LUID;
*/

