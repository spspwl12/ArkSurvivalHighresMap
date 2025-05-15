#include <Windows.h>
#include <process.h>
#include <wchar.h>

#include "PE.h"
#include "Pipe.h"
#include "Declare.h"
#include "Memory.h"
#include "Logic.h"
#include "Hook.h"

static void*    GEngine;
static void*    GWorld;
static void*    FLEVC_GCLEVC; // FLevelEditorViewportClient_GCurrentLevelEditingViewportClient
static void*    GScreenshotBitmapIndex;

static void     (*UUnrealEdEngine_Tick)(void* _this, float DeltaSeconds, char bIdleMode);
static char     (*UUnrealEdEngine_Exec)(void* _this, void* uWorld, wchar_t* Command);
static void*    (*FEditorViewportClient_GetViewLocation)(void* _this);
static void     (*FEditorViewportClient_SetViewLocation)(void* _this, void* Position);
static float    (*FEditorViewportClient_GetOrthoZoom)(void* _this);
static void     (*FEditorViewportClient_SetOrthoZoom)(void* _this, float InOrthoZoom);
static void     (*FEditorViewportClient_Invalidate)(void* _this);
static void*    (*FSceneViewport_ResizeViewport)(void* _this);
static void     (*FScreenshotRequest_CreateViewportScreenShotFilename)(wchar_t** InOutFilename);
static void     (*FDebug_EnsureFailed)(const char* Expr, const char* File, int Line, const wchar_t* Msg);

static char     cOriginalCode[10];

static HANDLE   hProcess;
static HMODULE  hModule;
static HANDLE   hEvent;
static char     CaptureSign;
static float    CaptureRes;
static POINT    ViewportSize;

DECLARE_HOOK(UUnrealEdEngine_Tick, void, void*, float, char);
DECLARE_HOOK(FSceneViewport_ResizeViewport, void, void*, int, int, int);

UINT WINAPI
UnloadThread(
    LPVOID lpvParam
);

void 
Commnuication(
    int size, 
    char* buf
)
{
    char* ptr = buf + 1;

    switch (*buf)
    {
        case MODE_GET_XYZ:
        {
            const void* pVec = FEditorViewportClient_GetViewLocation(FLEVC_GCLEVC);

            if (NULL == pVec)
                goto ERROR_RESULT;

            memcpy(ptr, pVec, 12);

            SendPipeMessage(1 + 12, buf);
            return;
        }
        case MODE_GET_ZOOM:
        {
            const float fval = FEditorViewportClient_GetOrthoZoom(FLEVC_GCLEVC);

            memcpy(ptr, &fval, 4);

            SendPipeMessage(1 + 4, buf);
            return;
        }
        case MODE_GET_WH:
        {
            memcpy(ptr, &ViewportSize, sizeof(POINT));
            SendPipeMessage(1 + 8, buf);
            return;
        }
        case MODE_GET_PATH:
        {
            const DWORD dwSize = GetModuleFileName(NULL, ptr, MAX_PATH);

            if (0 >= dwSize)
                goto ERROR_RESULT;

            SendPipeMessage(1 + dwSize, buf);
            return;
        }
        case MODE_SET_XY:
        {
            FEditorViewportClient_SetViewLocation(FLEVC_GCLEVC, ptr);
            FEditorViewportClient_Invalidate(FLEVC_GCLEVC);

            SendPipeMessage(1, buf);
            return;
        }
        case MODE_SET_ZOOM:
        {
            float fval;

            memcpy(&fval, ptr, 4);

            FEditorViewportClient_SetOrthoZoom(FLEVC_GCLEVC, fval);
            FEditorViewportClient_Invalidate(FLEVC_GCLEVC);

            SendPipeMessage(1, buf);
            return;
        }
        case MODE_SET_XYZZ_CAPTURE:
        {
            float fval;

            memcpy(&fval, ptr + 12, 4);

            FEditorViewportClient_SetViewLocation(FLEVC_GCLEVC, ptr);
            FEditorViewportClient_SetOrthoZoom(FLEVC_GCLEVC, fval);

            FEditorViewportClient_Invalidate(FLEVC_GCLEVC);

            // no return;
        }
        case MODE_CAPTURE:
        {
            if (0 != CaptureSign)
                return;

            memcpy(&CaptureRes, ptr + 16, 4);
            CaptureSign = 1;

            // �̹��� ĸó�� ���� ������ ��� ( ���� )
            // Wait synchronously until the image capture is complete.
            WaitForSingleObject(hEvent, INFINITE);
            ResetEvent(hEvent);
            SendPipeMessage(1, buf);
            return;
        }
    }

ERROR_RESULT:

    *buf = ERROR_RESULT_PIPE;
    SendPipeMessage(1, buf);
}

void
Hook_UUnrealEdEngine_Tick(
    void* _this, 
    float DeltaSeconds,
    char bIdleMode
)
{
    // ��ŷ�� ���� ������ ������ ������ �̹����� �߸� ��µǴ� ���� �����ϴ� �����Դϴ�.
    // It is the process of executing synchronous logic through hooking to prevent images from being displayed incorrectly.

    wchar_t* wsz = NULL;

    if (CaptureSign == 1 && 
        TRUE == WriteMemLong(hProcess, (ULONG_PTR)GScreenshotBitmapIndex, 0) &&
        NULL != (wsz = (wchar_t*)malloc(sizeof(wchar_t) * 30)))
    {
        swprintf_s(wsz, 30, L"HighResShot %g", CaptureRes);
        UUnrealEdEngine_Exec(GEngine, GWorld, wsz);
    }

    // ù ����Ŭ�� HighResShot ��ɾ ������ �� ��° ����Ŭ�� �̹����� ���
    // The first cycle executes the HighResShot command, and the second cycle outputs the image.
    UUnrealEdEngine_Tick_original(_this, DeltaSeconds, bIdleMode);

    switch (CaptureSign)
    {
    case 1:
        if (NULL != wsz)
            free(wsz);
        CaptureSign++;
        return;
    case 2:
        // �� ��° ����Ŭ�� �̹��� ����� �Ϸ�Ǿ� (UUnrealEdEngine_Tick_original �ȿ� �̹��� ���� ������ �� ����) ��� ���� ����.
        // The second cycle completes the image output, and the waiting state is released (the image saving logic is included in UUnrealEdEngine_Tick_original).
        SetEvent(hEvent);
        CaptureSign = 0;
        return;
    }
}

void
Hook_FSceneViewport_ResizeViewport(
    void* _this,
    int NewSizeX,
    int NewSizeY,
    int NewWindowMode
)
{
    // ������ ������ 256 x 256 ũ������ Ȯ���ϱ� ���� ��ŷ�� �����Դϴ�.
    // This is the hooked logic to verify that the rendering area is 256 x 256 in size.

    FSceneViewport_ResizeViewport_original(_this, NewSizeX, NewSizeY, NewWindowMode);

    ViewportSize.x = NewSizeX;
    ViewportSize.y = NewSizeY;
}

char 
LoadDll(
    void* _hModule
)
{
    hModule = _hModule;

    // Init hEvent
    {
        if (NULL == (hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)))
            return FALSE;
    }

    // Create Pipe
    {
        if (FALSE == StartPipeComm(Commnuication))
            return FALSE;

        SetDisconnectedPipeFunc((void*)UnloadDll);
    }

    if (NULL == (hProcess = GetCurrentProcess()))
        return FALSE;

    // ----------------------UE4Editor-Engine.dll----------------------
    {
        DWORD_PTR DLL = (DWORD_PTR)GetModuleHandle("UE4Editor-Engine.dll");

        if (0 == DLL)
            return FALSE;

        // ?GEngine@@3PEAVUEngine@@EA
        GEngine = GetFunction((HMODULE)DLL, "GEngine@");

        if (0 == (GEngine = (void*)ReadMemPtr(hProcess, (ULONG_PTR)GEngine)))
            return FALSE;

        // ?? �� 0x38 �� ���ؾ� �ϴ��� �𸣰ڽ��ϴ�.
        // I don't understand why 0x38 needs to be added.
        GEngine = (char*)GEngine + 0x38;

        // ?GWorld@@3VUWorldProxy@@A
        GWorld = GetFunction((HMODULE)DLL, "GWorld@");

        if (0 == (GWorld = (void*)ReadMemPtr(hProcess, (ULONG_PTR)GWorld)))
            return FALSE;

        // ?CreateViewportScreenShotFilename@FScreenshotRequest@@SAXAEAVFString@@@Z
        if (FScreenshotRequest_CreateViewportScreenShotFilename =
            GetFunction((HMODULE)DLL, "CreateViewportScreenShotFilename@"))
        {
            // CreateViewportScreenShotFilename �� ���� ���� : �̹��� ��� ��ΰ� �ǵ��� ��� �ϱ� ���ؼ��Դϴ�.
            // The reason for blocking CreateViewportScreenShotFilename: To ensure that the image output path follows the intended direction.

            if (ReadMemByte(hProcess, (ULONG_PTR)FScreenshotRequest_CreateViewportScreenShotFilename, cOriginalCode + 0))
                // ������� �ڵ� nop (0xC3) �� ������ CreateViewportScreenShotFilename ������ ����Ǵ� ������ �����ϴ�.
                // Insert the assembly code NOP (0xC3) to prevent the CreateViewportScreenShotFilename logic from executing.
                WriteMemByte(hProcess, (ULONG_PTR)FScreenshotRequest_CreateViewportScreenShotFilename, 0xC3);
        }

        // ?ResizeViewport@FSceneViewport@@EEAAXIIW4Type@EWindowMode@@HH@Z
        FSceneViewport_ResizeViewport = GetFunction((HMODULE)DLL, "ResizeViewport@");

        if (0 == FSceneViewport_ResizeViewport)
            return FALSE;

        // Hook Function
        if (FALSE == HookFunction(FSceneViewport_ResizeViewport, 
            (void*)&Hook_FSceneViewport_ResizeViewport,
            (void*)&FSceneViewport_ResizeViewport_original))
            return FALSE;
    }

    // ----------------------UE4Editor-Core.dll----------------------
    {
        DWORD_PTR DLL = (DWORD_PTR)GetModuleHandle("UE4Editor-Core.dll");

        if (0 == DLL)
            return FALSE;

        // ?GScreenshotBitmapIndex@@3HA
        GScreenshotBitmapIndex = GetFunction((HMODULE)DLL, "GScreenshotBitmapIndex@");

        if (0 == GScreenshotBitmapIndex)
            return FALSE;

        // ?EnsureFailed@FDebug@@SAXPEBD0HPEB_W@Z
        if (FDebug_EnsureFailed = GetFunction((HMODULE)DLL, "EnsureFailed@FDebug"))
        {
            // EnsureFailed �� ���� ���� : IsInGameThread ����� false�̱� ������ �ܺ� ���α׷��� Exec Ŀ�ǵ带 ���� ADK �� ������ �߻��� ����Ǳ� �����Դϴ�.
            // The reason for blocking EnsureFailed: Since the result of IsInGameThread is false, if an external program executes an Exec command, 
            // ADK will encounter an error and terminate.

            if (ReadMemByte(hProcess, (ULONG_PTR)FDebug_EnsureFailed, cOriginalCode + 1))
                // ������� �ڵ� nop (0xC3) �� ������ EnsureFailed ������ ����Ǵ� ������ �����ϴ�.
                // Insert the assembly code NOP (0xC3) to prevent the EnsureFailed logic from executing.
                WriteMemByte(hProcess, (ULONG_PTR)FDebug_EnsureFailed, 0xC3);
        }
    }

    // ----------------------UE4Editor-UnrealEd.dll----------------------
    {
        DWORD_PTR DLL = (DWORD_PTR)GetModuleHandle("UE4Editor-UnrealEd.dll");

        if (0 == DLL)
            return FALSE;

        // ?Exec@UUnrealEdEngine@@UEAA_NPEAVUWorld@@PEB_WAEAVFOutputDevice@@@Z
        UUnrealEdEngine_Exec = GetFunction((HMODULE)DLL, "Exec@UUnrealEdEngine");

        if (0 == UUnrealEdEngine_Exec)
            return FALSE;

        // ?GCurrentLevelEditingViewportClient@@3PEAVFLevelEditorViewportClient@@EA
        FLEVC_GCLEVC = 
            GetFunction((HMODULE)DLL, "GCurrentLevelEditingViewportClient@");

        if (0 == (FLEVC_GCLEVC = 
            (void*)ReadMemPtr(hProcess, (ULONG_PTR)FLEVC_GCLEVC)))
            return FALSE;

        // ?GetViewLocation@FEditorViewportClient@@QEBAAEBVFVector@@XZ
        FEditorViewportClient_GetViewLocation = GetFunction((HMODULE)DLL, "GetViewLocation@");

        if (0 == FEditorViewportClient_GetViewLocation)
            return FALSE;

        // ?SetViewLocation@FEditorViewportClient@@QEAAXAEBVFVector@@@Z
        FEditorViewportClient_SetViewLocation = GetFunction((HMODULE)DLL, "SetViewLocation@");

        if (0 == FEditorViewportClient_SetViewLocation)
            return FALSE;

        // ?GetOrthoZoom@FEditorViewportClient@@QEBAMXZ
        FEditorViewportClient_GetOrthoZoom = GetFunction((HMODULE)DLL, "GetOrthoZoom@");

        if (0 == FEditorViewportClient_GetOrthoZoom)
            return FALSE;

        // ?SetOrthoZoom@FEditorViewportClient@@QEAAXM@Z
        FEditorViewportClient_SetOrthoZoom = GetFunction((HMODULE)DLL, "SetOrthoZoom@");

        if (0 == FEditorViewportClient_SetOrthoZoom)
            return FALSE;

        // ?Invalidate@FEditorViewportClient@@QEAAX_N0@Z
        FEditorViewportClient_Invalidate = GetFunction((HMODULE)DLL, "Invalidate@");

        if (0 == FEditorViewportClient_Invalidate)
            return FALSE;

        // ?Tick@UUnrealEdEngine@@UEAAXM_N@Z
        UUnrealEdEngine_Tick = GetFunction((HMODULE)DLL, "Tick@UUnrealEdEngine");

        if (0 == UUnrealEdEngine_Tick)
            return FALSE;

        // Hook Function
        if (FALSE == HookFunction(UUnrealEdEngine_Tick, 
            (void*)&Hook_UUnrealEdEngine_Tick,
            (void*)&UUnrealEdEngine_Tick_original))
            return FALSE;
    }

    return TRUE;
}

char
UnloadDll(
)
{
    if (NULL != hEvent)
    {
        SetEvent(hEvent);
        CloseHandle(hEvent);
    }

    // ĸó ������ ���������� Ȯ�� �� ��ŷ �ߴ� �Լ��� ���󺹱� ��ŵ�ϴ�.
    // Check if the capture logic is in a waiting state, then restore the hooked function to its original state.
    while (0 != CaptureSign)
        Sleep(100);

    if (UUnrealEdEngine_Tick_original)
        UnHookFunction(UUnrealEdEngine_Tick);

    if (FSceneViewport_ResizeViewport_original)
        UnHookFunction(FSceneViewport_ResizeViewport);

    StopPipeComm();

    if (FScreenshotRequest_CreateViewportScreenShotFilename)
        WriteMemByte(hProcess, (ULONG_PTR)FScreenshotRequest_CreateViewportScreenShotFilename, *(cOriginalCode + 0));

    if (FDebug_EnsureFailed)
        WriteMemByte(hProcess, (ULONG_PTR)FDebug_EnsureFailed, *(cOriginalCode + 1));

    // �ڱ� �ڽ��� ��ε� �մϴ�.
    // Revert the injection and unload myself.
    _beginthreadex(NULL, 0, UnloadThread,
        (PVOID)hModule, 0, NULL);

    return TRUE;
}

UINT WINAPI
UnloadThread(
    LPVOID lpvParam
)
{
    Sleep(100);

    FreeLibrary((HMODULE)lpvParam);
    return 0;
}