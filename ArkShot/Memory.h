#pragma once

int
ReadMemByte(
	void* hdl,
	unsigned long long addr,
	char* buf
);

int
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

int
WriteMemByte(
	void* hdl,
	unsigned long long addr,
	char val
);

int
WriteMemLong(
	void* hdl,
	unsigned long long addr,
	long val
);

int
WriteMemBuf(
	void* hdl,
	unsigned long long addr,
	char* buf,
	int size
);