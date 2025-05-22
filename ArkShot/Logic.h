#pragma once

#define         REQUEST_CAPTURE     1
#define         REQUEST_GETACTORS   2

#define DECLARE_HOOK(name, returnType, ...) typedef returnType(__fastcall * name ## _Func)(__VA_ARGS__); \
name ## _Func name ## _original; \
returnType __fastcall Hook_ ## name(__VA_ARGS__)

#define EXCEPTION_MACRO(src, ex)  __try{src}__except (EXCEPTION_EXECUTE_HANDLER){ex}

typedef struct TArray
{
	void* AllocatorInstance;
	signed int  ArrayNum;
	signed int  ArrayMax;
}TArray;

int
LoadDll(
	void* hModule
);

int
UnloadDll(
);
