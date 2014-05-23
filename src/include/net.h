
extern "C" bool adptr_exist;
extern "C" bool adptr_ipset;
extern "C" char adptr_ip[255];
extern DWORD adptr_idx;

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

