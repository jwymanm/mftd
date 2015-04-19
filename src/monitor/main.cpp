// monitor/main.cpp
// a recording system of how often we actually see the interface vs seeing the mac
// this tells us where any problem lies in communication
// save and report this information to an url

#if MONITOR

#include "core.h"
#include "util.h"
#include "net.h"
#include "monitor.h"
#include "http.h"

namespace monitor {

Monitor mon;
LocalBuffers lb;
LocalData ld;

void cleanup (int et) {

  if (et) {
    *ld.ir = false;
    Sleep(1000);
    LSM(LOG_NOTICE, "stopped")
    pthread_exit(NULL);
  } else return;
}

void stop() {
  if (config.monitor && gd.running[MONITOR_IDX]) {
    *ld.ir = false;
    LSM(LOG_NOTICE, "stopping")
    cleanup(0);
    Sleep(1000);
  }
}

void start() {
  if (config.monitor && !gd.running[MONITOR_IDX]) {
    pthread_create(&gd.threads[MONITOR_TIDX], NULL, main, NULL);
    Sleep(1000);
  }
}

#if HTTP
bool buildSP(void* arg) {

  HTTP_SPHEAD(0)

  char buff[100];

  if (*ld.ir) {

    if (mon.comp.domain[0]) {

      fp += sprintf(fp, "<h4>Computer information</h4>");
      fp += sprintf(fp, "<p>\n");
      fp += sprintf(fp, "Domain name: %s", mon.comp.domain);
      fp += sprintf(fp, "</p>\n");

    }

    if (mon.user.name[0]) {

      fp += sprintf(fp, "<h4>User information</h4>");
      fp += sprintf(fp, "<p>\n");
      fp += sprintf(fp, "User name: %s<br/>\n", mon.user.name);

      if (mon.user.fname[0])
        fp += sprintf(fp, "Full name: %s<br/>\n", mon.user.fname);

      if (mon.user.email[0])
        fp += sprintf(fp, "E-mail: %s<br/>\n", mon.user.email);

      if (mon.user.sid[0])
        fp += sprintf(fp, "Sid: %s<br/>\n", mon.user.sid);

      fp += sprintf(fp, "</p>\n");
    }

    fp += sprintf(fp, "<h4>Adapter information</h4>");


    fp += sprintf(fp, "<p>Adapter description: %s</p>", adptr.desc);

    if (config.monip) {

      fp += sprintf(fp, "<h4>Monitored Device</h4>");

      if (mon.dev.found) {
        IFAddr2String(buff, mon.dev.mac, sizeof(mon.dev.mac));
        fp += sprintf(fp, "<p>Device found</p>\n<p>Mac: %s<br/>IP: %s", buff, config.monip);
      } else fp += sprintf(fp, "<p>Device unreachable</p>\n");
    }

  } else {
    fp += sprintf(fp, "<p>Not running</p>\n<br/>\n");
  }

  HTTP_SPFOOT

}
#endif

void getUserFullname() {

  PDOMAIN_CONTROLLER_INFO pdci;
  NET_API_STATUS nStatus;
  LPUSER_INFO_2 pBuf = NULL;

  int i = 0, j = 0, namelen = 0;

  namelen = MultiByteToWideChar(CP_ACP, 0, mon.user.name, -1, mon.user.wname, 0);
  if (namelen > 0)
    MultiByteToWideChar(CP_ACP, 0, mon.user.name, -1, mon.user.wname, namelen);

  DsGetDcName(NULL, mon.comp.domain, NULL, NULL, 0, &pdci);

  if (pdci->DomainControllerName) {

  namelen = MultiByteToWideChar(CP_ACP, 0, pdci->DomainControllerName, -1, mon.comp.wdomain, 0);
  if (namelen > 0)
    MultiByteToWideChar(CP_ACP, 0, pdci->DomainControllerName, -1, mon.comp.wdomain, namelen);

  nStatus = NetUserGetInfo(mon.comp.wdomain, mon.user.wname, 2, (LPBYTE*) &pBuf);

  if (pBuf) NetApiBufferFree(pBuf);

  } else {
    nStatus = NetUserGetInfo(NULL, mon.user.wname, 2, (LPBYTE*) &pBuf);
    if (pBuf) NetApiBufferFree(pBuf);
  }

  if (nStatus == NERR_Success) {
    if (pBuf) {
      wcscpy(mon.user.wfname, pBuf->usri2_full_name);
      wpcopy(mon.user.fname, mon.user.wfname);
      NetApiBufferFree(pBuf);
    }
  } else {
    LSM(LOG_DEBUG, "NetUserGetinfo failed with error %d", nStatus)
  }
  return;
}

void getLogonFromToken (HANDLE hToken) {

  SID_NAME_USE sidType;
  PTOKEN_USER ptu = NULL;
  DWORD dwSize = MAX_NAME, dwLength = 0, dwResult = 0;
  LPTSTR sidString;

  if (!GetTokenInformation(hToken, TokenUser, (LPVOID) ptu, 0, &dwLength)) {
    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
      ptu = (PTOKEN_USER) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwLength);
      if (ptu != NULL) {
        if (GetTokenInformation(hToken, TokenUser, (LPVOID) ptu, dwLength, &dwLength)) {
          if (!LookupAccountSid(NULL, ptu->User.Sid, mon.user.name, &dwSize, mon.comp.domain, &dwSize, &sidType)) {
            dwResult = GetLastError();
            if (dwResult == ERROR_NONE_MAPPED) mon.user.name[0] = '\0';
            else { LSM(LOG_DEBUG, "LookupAccountSid error %u", dwResult) }
          } else {
            if (ConvertSidToStringSid(ptu->User.Sid, &sidString)) {
              sprintf(mon.user.sid, "%s", sidString);
              LocalFree(sidString);
            }
          }
        }
      }
    }
    if (ptu != NULL) HeapFree(GetProcessHeap(), 0, (LPVOID) ptu);
  }
  return;
}

void getLogonInfo() {

  DWORD aProcesses[1024];
  DWORD dwSessID = 0, dwcbSzUserName = 0, cbNeeded = 0, cProcesses = 0;
  TCHAR szProcessName[MAX_PATH];
  HANDLE hProcess = NULL, hToken = NULL;
  HMODULE hMod;
  UINT i;

  if (EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
    cProcesses = cbNeeded / sizeof(DWORD);
    for (i=0; i < cProcesses; i++) {
      if (aProcesses[i] != 0) {
        szProcessName[0] = '\0';
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);
        if (hProcess == NULL) continue;
        if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
          GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName)/sizeof(TCHAR));
          if (!strcasecmp(szProcessName, PROCESSMATCH)) {
            if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) {
              CloseHandle(hProcess);
              continue;
            }
            if (hToken != NULL) { getLogonFromToken(hToken); CloseHandle(hToken); }
            CloseHandle(hProcess);
          }
        }
      }
    }
  }
  return;
}

void __cdecl init(void* arg) {

  char buff[100];

  do {

    if (getDevMacAddress(mon.dev.mac, config.monip) == NO_ERROR) {
      IFAddr2String(buff, mon.dev.mac, sizeof(mon.dev.mac));
      mon.dev.found = true;
      LSM(LOG_INFO, "found attached device with mac %s", buff)
    } else {
      mon.dev.found = false;
      LSM(LOG_INFO, "no attached device responding yet")
    }

    //getLogonInfo();
    //getUserFullname();

    /*
    if (ld.ui.name)
      printf("%s", ld.ui.name);
    if (ld.ui.fname)
      printf("%s", ld.ui.fname);
    if (ld.ci.domain)
      printf("%s", ld.ci.domain);
    */

    Sleep(60000);

  } while (*ld.ir);

  _endthread();
  return;
}

void* main(void* arg) {

  SERVICESTART(MONITOR_IDX)

    // record date of found
    if (getAdapterData()) {
      LSM(LOG_INFO, "adapter with description \"%s\" found", adptr.desc)
    // record date of not found
    } else {
      LSM(LOG_INFO, "waiting for adapter with description \"%s\" to be available", adptr.desc)
    }

  SERVICEEND

}

}
#endif
