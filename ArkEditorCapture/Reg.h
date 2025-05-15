#ifndef REG_H
#define REG_H

int ReadReg(const char* path, const char* key, const char* defaultVal, char* buf);
int WriteReg(const char* path, const char* key, char* value);
#endif
