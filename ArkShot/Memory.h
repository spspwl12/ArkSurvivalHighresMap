#pragma once

char
ReadMemByte(
	void* hdl,
	unsigned long long addr,
	char* buf
);

char
ReadMemBuf(
	void* hdl,
	unsigned long long addr,
	char* buf,
	int size
);

unsigned long long
ReadMemPtr(
	void* hdl,
	unsigned long long addr
);

char
WriteMemByte(
	void* hdl,
	unsigned long long addr,
	char val
);

char
WriteMemLong(
	void* hdl,
	unsigned long long addr,
	long val
);

char
WriteMemBuf(
	void* hdl,
	unsigned long long addr,
	char* buf,
	int size
);