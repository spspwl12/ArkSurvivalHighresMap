#include <Windows.h>
#include "Process.h"

char
InjectDll(
    void* hdl,
    const char* dllPath
)
{
    size_t pathLen = strlen(dllPath);

    if (0 >= pathLen)
        return 0;

    HMODULE hModule = GetModuleHandle("kernel32.dll");

    if (NULL == hModule)
        return 0;

    void* pFunction = GetProcAddress(hModule, "LoadLibraryA");

    if (NULL == pFunction)
        return 0;

    void* pMem = VirtualAllocEx((HANDLE)hdl, NULL, pathLen, MEM_COMMIT, PAGE_READWRITE);

    if (NULL == pMem)
        return 0;

    DWORD rst = 0;
    size_t bytes;
    HANDLE hThread = 0;

    if (FALSE == WriteProcessMemory((HANDLE)hdl, pMem, dllPath, pathLen, &bytes))
        goto FREE_ALLOC;

    if (NULL == (hThread = CreateRemoteThread((HANDLE)hdl, NULL, 0, 
        pFunction, pMem, 0, NULL)))
        goto FREE_ALLOC;

    WaitForSingleObject(hThread, INFINITE);

    if (FALSE == GetExitCodeThread(hThread, &rst))
        goto FREE_ALLOC;

    rst = 1;

FREE_ALLOC:

    if (hThread)
        CloseHandle(hThread);

    VirtualFreeEx((HANDLE)hdl, pMem, 0, MEM_RELEASE);
    return (char)rst;
}

char
UnloadDll(
    void* hdl,
    const char* dllPath
)
{
    unsigned long pID = GetProcessId(hdl);

    if (0 == pID)
        return 0;

    char* dllName = strrchr(dllPath, '\\');

    if (NULL == dllName)
        dllName = (char*)dllPath;
    else
        dllName++;

    HMODULE hModule = GetModuleBaseAddress(pID, dllName, NULL);

    if (NULL == hModule)
        return 0;

    HMODULE hModule2 = GetModuleHandle("kernel32.dll");

    if (NULL == hModule2)
        return 0;

    void* pFunction = GetProcAddress(hModule, "FreeLibrary");

    if (NULL == pFunction)
        return 0;

    HANDLE hThread;

    if (NULL == (hThread = CreateRemoteThread((HANDLE)hdl, NULL, 0,
        pFunction, hModule, 0, NULL)))
        return 0;

    WaitForSingleObject(hThread, INFINITE);

    return 1;
}