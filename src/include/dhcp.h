// dhcp.h

#if DHCP

#define MAX_DHCP_RANGES 125
#define MAX_RANGE_SETS 32
#define MAX_RANGE_FILTERS 32

#define BOOTP_REQUEST  1
#define BOOTP_REPLY    2

#define DHCP_MESS_NONE       0
#define DHCP_MESS_DISCOVER   1
#define DHCP_MESS_OFFER      2
#define DHCP_MESS_REQUEST	 3
#define DHCP_MESS_DECLINE	 4
#define DHCP_MESS_ACK		 5
#define DHCP_MESS_NAK		 6
#define DHCP_MESS_RELEASE    7
#define DHCP_MESS_INFORM	 8

#define DHCP_OPTION_PAD				0
#define DHCP_OPTION_NETMASK          		1
#define DHCP_OPTION_TIMEOFFSET       		2
#define DHCP_OPTION_ROUTER           		3
#define DHCP_OPTION_TIMESERVER       		4
#define DHCP_OPTION_NAMESERVER       		5
#define DHCP_OPTION_DNS              		6
#define DHCP_OPTION_LOGSERVER        		7
#define DHCP_OPTION_COOKIESERVER     		8
#define DHCP_OPTION_LPRSERVER        		9
#define DHCP_OPTION_IMPRESSSERVER    		10
#define DHCP_OPTION_RESLOCSERVER     		11
#define DHCP_OPTION_HOSTNAME         		12
#define DHCP_OPTION_BOOTFILESIZE     		13
#define DHCP_OPTION_MERITDUMP        		14
#define DHCP_OPTION_DOMAINNAME       		15
#define DHCP_OPTION_SWAPSERVER       		16
#define DHCP_OPTION_ROOTPATH         		17
#define DHCP_OPTION_EXTSPATH         		18
#define DHCP_OPTION_IPFORWARD        		19
#define DHCP_OPTION_NONLOCALSR       		20
#define DHCP_OPTION_POLICYFILTER     		21
#define DHCP_OPTION_MAXREASSEMBLE    		22
#define DHCP_OPTION_IPTTL            		23
#define DHCP_OPTION_PATHMTUAGING     		24
#define DHCP_OPTION_PATHMTUPLATEAU   		25
#define DHCP_OPTION_INTERFACEMTU     		26
#define DHCP_OPTION_SUBNETSLOCAL     		27
#define DHCP_OPTION_BCASTADDRESS     		28
#define DHCP_OPTION_MASKDISCOVERY    		29
#define DHCP_OPTION_MASKSUPPLIER     		30
#define DHCP_OPTION_ROUTERDISCOVERY  		31
#define DHCP_OPTION_ROUTERSOLIC      		32
#define DHCP_OPTION_STATICROUTE      		33
#define DHCP_OPTION_TRAILERENCAPS    		34
#define DHCP_OPTION_ARPTIMEOUT       		35
#define DHCP_OPTION_ETHERNETENCAPS   		36
#define DHCP_OPTION_TCPTTL           		37
#define DHCP_OPTION_TCPKEEPALIVEINT  		38
#define DHCP_OPTION_TCPKEEPALIVEGRBG 		39
#define DHCP_OPTION_NISDOMAIN        		40
#define DHCP_OPTION_NISSERVERS       		41
#define DHCP_OPTION_NTPSERVERS       		42
#define DHCP_OPTION_VENDORSPECIFIC   		43
#define DHCP_OPTION_NETBIOSNAMESERV  		44
#define DHCP_OPTION_NETBIOSDGDIST    		45
#define DHCP_OPTION_NETBIOSNODETYPE  		46
#define DHCP_OPTION_NETBIOSSCOPE     		47
#define DHCP_OPTION_X11FONTS         		48
#define DHCP_OPTION_X11DISPLAYMNGR   		49
#define DHCP_OPTION_REQUESTEDIPADDR  		50
#define DHCP_OPTION_IPADDRLEASE      		51
#define DHCP_OPTION_OVERLOAD         		52
#define DHCP_OPTION_MESSAGETYPE      		53
#define DHCP_OPTION_SERVERID         		54
#define DHCP_OPTION_PARAMREQLIST     		55
#define DHCP_OPTION_MESSAGE          		56
#define DHCP_OPTION_MAXDHCPMSGSIZE   		57
#define DHCP_OPTION_RENEWALTIME      		58
#define DHCP_OPTION_REBINDINGTIME    		59
#define DHCP_OPTION_VENDORCLASSID    		60
#define DHCP_OPTION_CLIENTID         		61
#define DHCP_OPTION_NETWARE_IPDOMAIN            62
#define DHCP_OPTION_NETWARE_IPOPTION            63
#define DHCP_OPTION_NISPLUSDOMAIN    		64
#define DHCP_OPTION_NISPLUSSERVERS   		65
#define DHCP_OPTION_TFTPSERVER       		66
#define DHCP_OPTION_BOOTFILE         		67
#define DHCP_OPTION_MOBILEIPHOME     		68
#define DHCP_OPTION_SMTPSERVER       		69
#define DHCP_OPTION_POP3SERVER       		70
#define DHCP_OPTION_NNTPSERVER       		71
#define DHCP_OPTION_WWWSERVER        		72
#define DHCP_OPTION_FINGERSERVER     		73
#define DHCP_OPTION_IRCSERVER        		74
#define DHCP_OPTION_STSERVER         		75
#define DHCP_OPTION_STDASERVER       		76
#define DHCP_OPTION_USERCLASS        		77
#define DHCP_OPTION_SLPDIRAGENT      		78
#define DHCP_OPTION_SLPDIRSCOPE      		79
#define DHCP_OPTION_CLIENTFQDN       		81
#define DHCP_OPTION_RELAYAGENTINFO     		82
#define DHCP_OPTION_I_SNS     			83
#define DHCP_OPTION_NDSSERVERS       		85
#define DHCP_OPTION_NDSTREENAME      		86
#define DHCP_OPTION_NDSCONTEXT		 	87
#define DHCP_OPTION_AUTHENTICATION		90
#define DHCP_OPTION_CLIENTSYSTEM		93
#define DHCP_OPTION_CLIENTNDI			94
#define DHCP_OPTION_LDAP			95
#define DHCP_OPTION_UUID_GUID			97
#define DHCP_OPTION_USER_AUTH			98
#define DHCP_OPTION_P_CODE			100
#define DHCP_OPTION_T_CODE			101
#define DHCP_OPTION_NETINFOADDRESS	        112
#define DHCP_OPTION_NETINFOTAG			113
#define DHCP_OPTION_URL				114
#define DHCP_OPTION_AUTO_CONFIG			116
#define DHCP_OPTION_NAMESERVICESEARCH		117
#define DHCP_OPTION_SUBNETSELECTION		118
#define DHCP_OPTION_DOMAINSEARCH		119
#define DHCP_OPTION_SIPSERVERSDHCP		120
#define DHCP_OPTION_CLASSLESSSTATICROUTE	121
#define DHCP_OPTION_CCC				122
#define DHCP_OPTION_GEOCONF			123
#define DHCP_OPTION_V_IVENDORCLASS		124
#define DHCP_OPTION_V_IVENDOR_SPECIFIC		125
#define DHCP_OPTION_TFPTSERVERIPADDRESS		128
#define DHCP_OPTION_CALLSERVERIPADDRESS		129
#define DHCP_OPTION_DISCRIMINATIONSTRING	130
#define DHCP_OPTION_REMOTESTATISTICSSERVER	131
#define DHCP_OPTION_802_1PVLANID		132
#define DHCP_OPTION_802_1QL2PRIORITY		133
#define DHCP_OPTION_DIFFSERVCODEPOINT		134
#define DHCP_OPTION_HTTPPROXYFORPHONE_SPEC	135
#define DHCP_OPTION_SERIAL			252
#define DHCP_OPTION_BP_FILE			253
#define DHCP_OPTION_NEXTSERVER			254
#define DHCP_OPTION_END				255

#define IPPORT_DHCPS   67
#define IPPORT_DHCPC   68

#define VM_STANFORD  0x5354414EUL
#define VM_RFC1048   0x63825363UL

extern bool dhcp_running;

namespace dhcp {

typedef struct {
  char *mapname;
  time_t expiry;
  union {
    struct {
      MYBYTE reserved;
      MYBYTE dataType;
      MYBYTE sockInd;
      MYBYTE dnsIndex;
    };
    struct {
      unsigned fixed: 1;
      unsigned local: 1;
      unsigned display: 1;
      unsigned reserved1: 5;
      char rangeInd;
      MYWORD dhcpInd;
    };
  };
  union {
   int bytes;
   MYDWORD ip;
  };
  union {
    SOCKADDR_IN *addr;
    MYBYTE *options;
  };
  union {
    MYBYTE *response;
    char *hostname;
    char *query;
  };
  MYBYTE data;
} Data;

typedef struct {
  char* mapname;
  MYBYTE* response;
  char* hostname;
  char* query;
  SOCKADDR_IN* addr;
  MYBYTE* options;
  MYWORD optionSize;
  int bytes;
  MYBYTE dataType;
} lumpData;

typedef std::map<std::string, Data*> dhcpMap;
typedef std::multimap<time_t, Data*> expiryMap;

typedef struct {
  MYBYTE bp_op;
  MYBYTE bp_htype;
  MYBYTE bp_hlen;
  MYBYTE bp_hops;
  MYDWORD bp_xid;
  struct {
    unsigned bp_secs:16;
    unsigned bp_spare:7;
    unsigned bp_broadcast:1;
    unsigned bp_spare1:8;
  };
  MYDWORD bp_ciaddr;
  MYDWORD bp_yiaddr;
  MYDWORD bp_siaddr;
  MYDWORD bp_giaddr;
  MYBYTE bp_chaddr[16];
  char bp_sname[64];
  MYBYTE bp_file[128];
  MYBYTE bp_magic_num[4];
} Header;

typedef struct {
  Header header;
  MYBYTE vend_data[1024 - sizeof(Header)];
} Packet;

typedef struct {
  MYBYTE rangeSetInd;
  MYDWORD rangeStart;
  MYDWORD rangeEnd;
  MYDWORD mask;
  MYBYTE* options;
  time_t* expiry;
  Data** dhcpEntry;
} Range;

typedef struct {
  MYWORD dhcpInd;
  MYBYTE bp_hlen;
  MYBYTE local;
  MYDWORD source;
  MYDWORD ip;
  time_t expiry;
  MYBYTE bp_chaddr[16];
  char hostname[64];
} Client;

typedef struct {
  MYBYTE active;
  MYBYTE* macStart[MAX_RANGE_FILTERS];
  MYBYTE* macEnd[MAX_RANGE_FILTERS];
  MYBYTE macSize[MAX_RANGE_FILTERS];
  MYBYTE* vendClass[MAX_RANGE_FILTERS];
  MYBYTE vendClassSize[MAX_RANGE_FILTERS];
  MYBYTE* userClass[MAX_RANGE_FILTERS];
  MYBYTE userClassSize[MAX_RANGE_FILTERS];
  MYDWORD subnetIP[MAX_RANGE_FILTERS];
  MYDWORD targetIP;
} RangeSets;

typedef struct {
  MYBYTE macArray[MAX_RANGE_SETS];
  MYBYTE vendArray[MAX_RANGE_SETS];
  MYBYTE userArray[MAX_RANGE_SETS];
  MYBYTE subnetArray[MAX_RANGE_SETS];
  bool macFound;
  bool vendFound;
  bool userFound;
  bool subnetFound;
} ClientSets;

typedef struct {
  MYBYTE options[sizeof(Packet)];
  MYWORD optionSize;
  MYDWORD ip;
  MYDWORD mask;
  MYBYTE rangeSetInd;
} Opt;

typedef struct {
  MYBYTE opt_code;
  MYBYTE size;
  MYBYTE value[256];
} OptCode;

typedef struct {
  MYDWORD lease;
  union {
    char raw[sizeof(Packet)];
    Packet dhcpp;
  };
  char hostname[256];
  char chaddr[64];
  MYDWORD server;
  MYDWORD reqIP;
  int bytes;
  SOCKADDR_IN remote;
  socklen_t sockLen;
  MYWORD messsize;
  MYBYTE* vp;
  Data* dhcpEntry;
  OptCode agentOption;
  OptCode clientId;
  OptCode subnet;
  OptCode vendClass;
  OptCode userClass;
  MYDWORD subnetIP;
  MYDWORD targetIP;
  MYDWORD rebind;
  MYBYTE paramreqlist[256];
  MYBYTE opAdded[256];
  MYBYTE req_type;
  MYBYTE resp_type;
  MYBYTE sockInd;
} Request;

typedef struct {
  SOCKET sock;
  SOCKADDR_IN addr;
  MYDWORD server;
  MYWORD port;
  MYDWORD mask;
  int broadCastVal;
  int broadCastSize;
  int reUseVal;
  int reUseSize;
  int donotRouteVal;
  int donotRouteSize;
  int pktinfo;
  bool loaded;
  bool ready;
} DHCPConnType;

typedef struct {
  char opName[40];
  MYBYTE opTag;
  MYBYTE opType;
  bool configurable;
} OPS;

typedef struct {
  MYDWORD zoneServers[2];
  ConnType dhcpReplConn;
  MYDWORD mask;
  MYDWORD lease;
  Range dhcpRanges[MAX_DHCP_RANGES];
  RangeSets rangeSet[MAX_RANGE_SETS];
  MYDWORD rangeStart;
  MYDWORD rangeEnd;
  MYBYTE* options;
  MYDWORD dhcpSize;
  MYDWORD serial;
  MYWORD dhcpInd;
  MYBYTE replication;
  time_t dhcpRepl;
  MYBYTE rangeCount;
  bool hasFilter;
} Configuration;

typedef struct {
  char log[256];
  char tmp[512];
  char ext[512];
  char lea[_MAX_PATH];
  char cli[_MAX_PATH];
} LocalBuffers;

typedef struct {
  time_t t;
} LocalData;

typedef struct {
  DHCPConnType dhcpConn[MAX_SERVERS];
  SOCKET maxFD;
} NetworkData;

//Function Prototypes
bool buildSP(void* lpParam);
char* genHostName(char* target, MYBYTE* hex, MYBYTE bytes);
FILE* openSection(const char* sectionName, MYBYTE index);
char* readSection(char* raw, FILE* f);
bool getSection(const char* sectionName, char* buffer, MYBYTE index, char* fileName);
char* strqtype(char* buff, MYBYTE qtype);
char getRangeInd(MYDWORD ip);
int MyRecvMess(char* buffer, MYWORD buffsize, SOCKET m_Socket, bool* broadcast, SOCKADDR_IN* remote, socklen_t sockLen);
MYBYTE pIP(void* raw, MYDWORD data);
MYBYTE pULong(void* raw, MYDWORD data);
MYBYTE pUShort(void* raw, MYWORD data);
MYWORD fUShort(void* raw);
MYDWORD alad(Request* req);
MYDWORD chad(Request* req);
MYDWORD fIP(void* raw);
MYDWORD fULong(void* raw);
MYDWORD resad(Request* req);
MYDWORD sendRepl(Request* req);
MYDWORD sdmess(Request* req);
MYDWORD updateDHCP(Request* req);
MYWORD gdmess(Request* req, MYBYTE sockInd);
Data* findDHCPEntry(char* entry);
Data* createCache(lumpData* lump);
bool checkRange(ClientSets* rangeData, char rangeInd);
void loadOptions(FILE* f, const char* sectionName, Opt* optionData);
int getIndex(char rangeInd, MYDWORD ip);
void addDHCPRange(char* dp);
void addMacRange(MYBYTE rangeSetInd, char* macRange);
void addOptions(Request* req);
void addUserClass(MYBYTE rangeSetInd, char* userClass, MYBYTE userClassSize);
void addVendClass(MYBYTE rangeSetInd, char* vendClass, MYBYTE vendClassSize);
void calcRangeLimits(MYDWORD ip, MYDWORD mask, MYDWORD* rangeStart, MYDWORD* rangeEnd);
void checkSize();
void loadDHCP();
void logDebug(void* lpParam);
void lockIP(MYDWORD ip);
void pvdata(Request* req, OptCode* op);
void releaseLease(void* lpParam);
void recvRepl(Request* req);
void setTempLease(Data*);
void setLeaseExpiry(Data*);
void setLeaseExpiry(Data*, MYDWORD);
void sendToken(void* lpParam);
void updateStateFile(void*);
void cleanup(int et);
void __cdecl init(void *lpParam);
void* main(void* arg);

}
#endif
