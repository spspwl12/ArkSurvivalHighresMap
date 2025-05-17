#include "Pipe.h"
#include <Windows.h>
#include <process.h>

static HANDLE hPipe;
static BOOL bStart;
static void* cbEndPipe;

UINT WINAPI
PipeThread(
    LPVOID cb
)
{
    char buf[PIPE_BUF_SIZE];
    DWORD bytesRead;
    BOOL fConnected = FALSE;

    while (bStart == 1)
    {
        if (FALSE == fConnected)
        {
            fConnected = ConnectNamedPipe(hPipe, NULL) ?
                TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

            if (FALSE == fConnected)
                goto FINALLY;

            if (FALSE == bStart)
                goto FINALLY;
        }

        // 클라이언트와 연결이 끊긴 경우, 대기 중인 ReadFile 이 실패하여 스레드 종료된다.
        // If the connection with the client is lost, the pending ReadFile operation fails, and the thread terminates.
        if (FALSE == ReadFile(hPipe, buf, sizeof(buf), &bytesRead, NULL))
        {
            fConnected = FALSE;
            goto FINALLY;
        }

        ((void(*)(int, char*))cb)(bytesRead, buf);
    }

FINALLY:

    FlushFileBuffers(hPipe);
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);

    hPipe = 0;
    bStart = FALSE;
  
    ((void(*)())cbEndPipe)();
    return FALSE;
}

int
StartPipeComm(
    void* cb
)
{
    if (INVALID_HANDLE_VALUE == (hPipe = CreateNamedPipeA(
        PIPE_NAME,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES, 
        PIPE_BUF_SIZE, PIPE_BUF_SIZE, 0, NULL
    )))
    {
        hPipe = 0;
        return FALSE;
    }

    bStart = TRUE;

    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, PipeThread, cb, 0, NULL);

    if (NULL != hThread)
        CloseHandle(hThread);

    return TRUE;
}

int
StopPipeComm(
)
{
    bStart = FALSE;

    if (hPipe)
    {
        // 대기 중인 PipeThread를 일깨워 종료되도록 유도합니다.
        // Wake up the waiting PipeThread to prompt its termination.

        HANDLE h = CreateFileA(
            PIPE_NAME,
            GENERIC_READ | GENERIC_WRITE,
            0, NULL,
            OPEN_EXISTING,
            0, NULL
        );

        if (INVALID_HANDLE_VALUE != h)
            CloseHandle(h);
    }

    return TRUE;
}

void 
SetDisconnectedPipeFunc(
    void* cb
)
{
    cbEndPipe = cb;
}

int
SendPipeMessage(
    int size, 
    char* buf
)
{
    DWORD bytesRead;
    return WriteFile(hPipe, buf, size, &bytesRead, NULL) && bytesRead != 0;
}