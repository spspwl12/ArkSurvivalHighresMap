﻿#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "Reg.h"

int
ReadReg(
	const char* path,
	const char* key,
	const char* defaultVal, 
	char* buf
)
{
	HKEY    hKey;
	unsigned long   data_size = 255;
	unsigned long	data_type;

	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, path, 0, KEY_QUERY_VALUE, &hKey))
	{
		if (ERROR_SUCCESS == RegQueryValueEx(hKey, key, 0, &data_type, (unsigned char*)buf, (unsigned long*)&data_size))
		{
			RegCloseKey(hKey);
			return 1;
		}

		RegCloseKey(hKey);
	}

	size_t len = strlen(defaultVal);
	memcpy(buf, defaultVal, len);
	buf[len] = 0;

	return 0;
}

int 
WriteReg(
	const char* path, 
	const char* key, 
	char* value
)
{
	HKEY    hKey;
	unsigned long	dwDesc;

	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER, path, 0, KEY_WRITE, &hKey))
		if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CURRENT_USER, path, 0, (char*)"", 0, KEY_READ | KEY_WRITE, 0, &hKey, &dwDesc))
			return 0;


	RegSetValueEx(hKey, key, 0, REG_SZ, (unsigned char*)value, ((unsigned long)strlen((char*)value) + 1) * sizeof(char));

	RegCloseKey(hKey);
	return 1;
}

int
LoadVal(
	const char* name,
	const char* initVal
)
{
	char Buf[255] = { 0 };

	ReadReg(REG_PATH, name, initVal, Buf);
	Buf[10] = 0;

	return atoi(Buf);
}

int
SaveVal(
	const char* name,
	int val
)
{
	char Buf[255] = { 0 };

	_itoa_s(val, Buf, sizeof(Buf), 10);

	return WriteReg(REG_PATH, name, Buf);
}