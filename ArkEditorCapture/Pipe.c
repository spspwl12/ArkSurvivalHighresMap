#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#include <process.h>

#include "Pipe.h"

static HANDLE hPipe;
static BOOL bStart;

char
StartPipeComm(
)
{
    if (INVALID_HANDLE_VALUE == (hPipe = CreateFileA(
        PIPE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        0, NULL, 
        OPEN_EXISTING, 
        0, NULL
    )))
    {
        hPipe = 0;
        return FALSE;
    }

    bStart = TRUE;

    return TRUE;
}

char 
StopPipeComm(
)
{
    bStart = FALSE;

    if (hPipe)
        CloseHandle(hPipe);

    hPipe = 0;

    return TRUE;
}

char
RecvPipeMessage(
    int size, 
    char* buf
)
{
    DWORD bytesRead;
    return ReadFile(hPipe, buf, size, &bytesRead, NULL) && bytesRead != 0;
}

char
SendPipeMessage(
    int size, 
    char* buf
)
{
    DWORD bytesRead;
    return WriteFile(hPipe, buf, size, &bytesRead, NULL) && bytesRead != 0;
}

int
SetMessage(
    char mode,
    int size,
    void* buf
)
{
    if (size > PIPE_BUF_SIZE)
        return FALSE;

    char _buf[PIPE_BUF_SIZE] = { 0 };

    _buf[0] = mode;

    if (size > 0)
        memcpy(_buf + 1, buf, sizeof(char) * size);

    if (FALSE == SendPipeMessage(1 + size, _buf))
        return FALSE;

    if (FALSE == RecvPipeMessage(1, _buf) || _buf[0] != mode)
        return FALSE;

    return TRUE;
}

int
RequestMessage(
    char mode,
    int size,
    void* buf
)
{
    if (size > PIPE_BUF_SIZE)
        return FALSE;

    if (FALSE == SendPipeMessage(1, &mode))
        return FALSE;

    char _buf[PIPE_BUF_SIZE] = { 0 };

    if (FALSE == RecvPipeMessage(1 + size, _buf))
        return FALSE;

    if (mode != (unsigned char)_buf[0])
        return FALSE;

    memcpy(buf, _buf + 1, sizeof(char) * size);

    return TRUE;
}