#include "Pipe.h"
#include <Windows.h>
#include <process.h>

static HANDLE hPipe;
static BOOL bStart;

char StartPipeComm()
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

char StopPipeComm()
{
    bStart = FALSE;

    if (hPipe)
        CloseHandle(hPipe);

    hPipe = 0;

    return TRUE;
}

char RecvPipeMessage(int size, char* buf)
{
    DWORD bytesRead;
    return ReadFile(hPipe, buf, size, &bytesRead, NULL) && bytesRead != 0;
}

char SendPipeMessage(int size, char* buf)
{
    DWORD bytesRead;
    return WriteFile(hPipe, buf, size, &bytesRead, NULL) && bytesRead != 0;
}