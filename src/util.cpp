// util.cpp
// helpful routines

#include "core.h"
#include "util.h"

char* cloneString(char* string) {
  if (!string) return NULL;
  char *s = (char*) calloc(1, strlen(string) + 1);
  if (s) { strcpy(s, string); }
  return s;
}

char* toLower(char* string) {
  char diff = 'a' - 'A';
  MYWORD len = strlen(string);
  for (int i = 0; i < len; i++)
    if (string[i] >= 'A' && string[i] <= 'Z') string[i] += diff;
  return string;
}

char* toUpper(char* string) {
  char diff = 'a' - 'A';
  MYWORD len = strlen(string);
  for (int i = 0; i < len; i++)
    if (string[i] >= 'a' && string[i] <= 'z') string[i] -= diff;
  return string;
}

char* strsep(char** stringp, const char* delim) {
  char *p;
  if (!stringp) return(NULL);
  p=*stringp;
  while (**stringp && !strchr(delim,**stringp)) (*stringp)++;
  if (**stringp) { **stringp='\0'; (*stringp)++; }
  else *stringp=NULL;
  return(p);
}

bool wildcmp(char* string, char* wild) {
  // Written by Jack Handy - jakkhandy@hotmail.com
  // slightly modified
  char *cp = NULL; char *mp = NULL;
  while ((*string) && (*wild != '*')) {
    if ((*wild != *string) && (*wild != '?')) { return 0; }
    wild++; string++;
  }
  while (*string) {
    if (*wild == '*') { if (!*++wild) return 1; mp = wild; cp = string + 1; }
    else if ((*wild == *string) || (*wild == '?')) { wild++; string++; }
    else { wild = mp; string = cp++; }
  }
  while (*wild == '*') wild++;
  return !(*wild);
}

// copies wchar into pchar
void wpcopy(PCHAR dest, PWCHAR src) {
  int srclen = wcslen(src);
  LPSTR destbuf = (LPSTR) calloc(1, srclen);
  WideCharToMultiByte(CP_ACP, 0, src, srclen, destbuf, srclen, NULL, NULL);
  strncpy(dest, destbuf, srclen);
  free(destbuf);
}

bool isInt(char* str) {
  if (!str || !(*str)) return false;
  for(; *str; str++) if (*str <  '0' || *str > '9') return false;
  return true;
}

char* hex2String(char* target, MYBYTE* hex, MYBYTE bytes) {
  char *dp = target;
  if (bytes) dp += sprintf(target, "%02x", *hex);
  else *target = 0;
  for (MYBYTE i = 1; i < bytes; i++) dp += sprintf(dp, ":%02x", *(hex + i));
  return target;
}

char* getHexValue(MYBYTE* target, char* src, MYBYTE* size) {
  if (*size) memset(target, 0, (*size));
  for ((*size) = 0; (*src) && (*size) < UCHAR_MAX; (*size)++, target++) {
    if ((*src) >= '0' && (*src) <= '9') { (*target) = (*src) - '0'; }
    else if ((*src) >= 'a' && (*src) <= 'f') { (*target) = (*src) - 'a' + 10; }
    else if ((*src) >= 'A' && (*src) <= 'F') { (*target) = (*src) - 'A' + 10; }
    else { return src; }
    src++;
    if ((*src) >= '0' && (*src) <= '9') { (*target) *= 16; (*target) += (*src) - '0'; }
    else if ((*src) >= 'a' && (*src) <= 'f') { (*target) *= 16; (*target) += (*src) - 'a' + 10; }
    else if ((*src) >= 'A' && (*src) <= 'F') { (*target) *= 16; (*target) += (*src) - 'A' + 10; }
    else if ((*src) == ':' || (*src) == '-') { src++; continue; }
    else if (*src) { return src; }
    else { continue; }
    src++;
    if ((*src) == ':' || (*src) == '-') { src++; }
    else if (*src) return src;
  }
  if (*src) return src;
  return NULL;
}

char* getCurrentTimestamp() {
	static char dateStr[20];
	time_t date;
	time(&date);
	strftime(dateStr, sizeof(dateStr), "%Y-%m-%d %H:%M:%S", localtime(&date));
	return dateStr;
}

char* myGetToken(char* buff, MYBYTE index) {
  while (*buff) {
    if (index) index--; else break;
    buff += strlen(buff) + 1;
  }
  return buff;
}

MYWORD myTokenize(char* target, char* src, const char* sep, bool whiteSep) {
  bool found = true;
  char* dp = target;
  MYWORD cnt = 0;
  while (*src) {
    if (sep && sep[0] && strchr(sep, (*src))) {
      found = true; src++;
      continue;
    } else if (whiteSep && (*src) <= NBSP) {
      found = true; src++;
      continue;
    }
    if (found) {
      if (target != dp) { *dp = 0; dp++; }
      cnt++;
    }
    found = false; *dp = *src; dp++; src++;
  }
  *dp = 0; dp++; *dp = 0;
  return cnt;
}

char* myTrim(char* target, char* src) {
  while ((*src) && (*src) <= NBSP) src++;
  int i = 0;
  for (; i < MAXCFGSIZE+1 && src[i]; i++) target[i] = src[i];
  target[i] = src[i];
  i--;
  for (; i >= 0 && target[i] <= NBSP; i--) target[i] = 0;
  return target;
}

void mySplit(char* name, char* val, const char* src, char splitChar) {
  int i = 0, j = 0, k = 0;
  for (; src[i] && j <= MAXCFGSIZE && src[i] != splitChar; i++, j++) name[j] = src[i];
  if (src[i]) { i++; for (; k <= MAXCFGSIZE && src[i]; i++, k++) val[k] = src[i]; }
  name[j] = 0; val[k] = 0; myTrim(name, name); myTrim(val, val);
}
