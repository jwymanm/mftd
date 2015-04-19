// util.h

char* toLower(char* string);
char* toUpper(char* string);
char* hex2String(char* target, MYBYTE* hex, MYBYTE bytes);
char* cloneString(char* string);
char* strsep(char** stringp, const char* delim);
char* getHexValue(MYBYTE* target, char* src, MYBYTE* size);
char* getCurrentTimestamp();
void wpcopy(PCHAR dest, PWCHAR src);
bool wildcmp(char* string, char* wild);
bool isInt(char* str);
MYWORD myTokenize(char* target, char* src, const char* sep, bool whiteSep);
char* myGetToken(char* buff, MYBYTE index);
char* myTrim(char* target, char* src);
void mySplit(char* name, char* val, const char* src, char splitChar);
size_t getline(char** lineptr, size_t* n, FILE* stream);
