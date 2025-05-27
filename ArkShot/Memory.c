#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>

int
ReadMemByte(
    void* hdl,
    unsigned long long addr,
    char* buf
)
{
    size_t bytes;
    char val;

    if (NULL == hdl)
        return FALSE;

    if (0 == addr)
        return FALSE;

    if (FALSE == ReadProcessMemory((HANDLE)hdl, (void*)addr, &val, sizeof(char), &bytes))
        return FALSE;

    if (sizeof(char) != bytes)
        return FALSE;

    *buf = val;

    return TRUE;
}

int
ReadMemBuf(
    void* hdl,
    unsigned long long addr,
    char* buf,
    int size
)
{
    size_t bytes;

    if (NULL == hdl)
        return FALSE;

    if (0 == addr)
        return FALSE;

    if (FALSE == ReadProcessMemory((HANDLE)hdl, (void*)addr, buf, size, &bytes))
        return FALSE;

    if (size != bytes)
        return FALSE;

    return TRUE;
}

unsigned long long
ReadMemPtr(
    void* hdl,
    unsigned long long addr
)
{
    size_t bytes;
    unsigned long long val;

    if (NULL == hdl)
        return (unsigned long long)0;

    if (0 == addr)
        return (unsigned long long)0;

    if (FALSE == ReadProcessMemory((HANDLE)hdl, (void*)addr, &val, sizeof(unsigned long long), &bytes))
        return (unsigned long long)0;

    if (sizeof(unsigned long long) != bytes)
        return (unsigned long long)0;

    return val;
}

int
WriteMemByte(
    void* hdl,
    unsigned long long addr,
    char val
)
{
    size_t bytes;

    if (NULL == hdl)
        return FALSE;

    if (0 == addr)
        return FALSE;

    if (FALSE == WriteProcessMemory((HANDLE)hdl, (void*)addr, &val, sizeof(char), &bytes))
        return FALSE;

    if (sizeof(char) != bytes)
        return FALSE;

    return TRUE;
}

int
WriteMemLong(
    void* hdl,
    unsigned long long addr,
    long val
)
{
    size_t bytes;

    if (NULL == hdl)
        return FALSE;

    if (0 == addr)
        return FALSE;

    if (FALSE == WriteProcessMemory((HANDLE)hdl, (void*)addr, &val, sizeof(long), &bytes))
        return FALSE;

    if (sizeof(long) != bytes)
        return FALSE;

    return TRUE;
}

int
WriteMemBuf(
    void* hdl,
    unsigned long long addr,
    char* buf,
    int size
)
{
    size_t bytes;

    if (NULL == hdl)
        return FALSE;

    if (0 == addr)
        return FALSE;

    if (FALSE == WriteProcessMemory((HANDLE)hdl, (void*)addr, buf, size, &bytes))
        return FALSE;

    if (size != bytes)
        return FALSE;

    return TRUE;
}