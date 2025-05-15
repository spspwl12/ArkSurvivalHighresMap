#include <Windows.h>

char
ReadMemByte(
    void* hdl,
    unsigned long long addr,
    char* buf
)
{
    size_t bytes;
    char val;

    if (NULL == hdl)
        return 0;

    if (0 == addr)
        return 0;

    if (FALSE == ReadProcessMemory((HANDLE)hdl, (void*)addr, &val, sizeof(char), &bytes))
        return 0;

    if (sizeof(char) != bytes)
        return 0;

    *buf = val;

    return 1;
}

char
ReadMemBuf(
    void* hdl,
    unsigned long long addr,
    char* buf,
    int size
)
{
    size_t bytes;

    if (NULL == hdl)
        return 0;

    if (0 == addr)
        return 0;

    if (FALSE == ReadProcessMemory((HANDLE)hdl, (void*)addr, buf, size, &bytes))
        return 0;

    if (size != bytes)
        return 0;

    return 1;
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
        return 0;

    if (0 == addr)
        return 0;

    if (FALSE == ReadProcessMemory((HANDLE)hdl, (void*)addr, &val, sizeof(unsigned long long), &bytes))
        return 0;

    if (sizeof(unsigned long long) != bytes)
        return 0;

    return val;
}

char
WriteMemByte(
    void* hdl,
    unsigned long long addr,
    char val
)
{
    size_t bytes;

    if (NULL == hdl)
        return 0;

    if (0 == addr)
        return 0;

    if (FALSE == WriteProcessMemory((HANDLE)hdl, (void*)addr, &val, sizeof(char), &bytes))
        return 0;

    if (sizeof(char) != bytes)
        return 0;

    return 1;
}

char 
WriteMemLong(
    void* hdl,
    unsigned long long addr,
    long val
)
{
    size_t bytes;

    if (NULL == hdl)
        return 0;

    if (0 == addr)
        return 0;

    if (FALSE == WriteProcessMemory((HANDLE)hdl, (void*)addr, &val, sizeof(long), &bytes))
        return 0;

    if (sizeof(long) != bytes)
        return 0;

    return 1;
}

char
WriteMemBuf(
    void* hdl,
    unsigned long long addr,
    char* buf,
    int size
)
{
    size_t bytes;

    if (NULL == hdl)
        return 0;

    if (0 == addr)
        return 0;

    if (FALSE == WriteProcessMemory((HANDLE)hdl, (void*)addr, buf, size, &bytes))
        return 0;

    if (size != bytes)
        return 0;

    return 1;
}