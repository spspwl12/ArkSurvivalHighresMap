#pragma once

#define DECLARE_HOOK(name, returnType, ...) typedef returnType(__fastcall * name ## _Func)(__VA_ARGS__); \
name ## _Func name ## _original; \
returnType __fastcall Hook_ ## name(__VA_ARGS__)

typedef struct FVector
{
	float x;
	float y;
	float z;
}FVector;

int
LoadDll(
	void* hModule
);

int
UnloadDll(
);
