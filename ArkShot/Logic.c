#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#include <process.h>
#include <wchar.h>
#include <stdio.h>

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
static void     (*FEditorViewportClient_Invalidate)(void* _this, char bInvalidateChildViews, char bInvalidateHitProxies);
static void*    (*FSceneViewport_ResizeViewport)(void* _this);
static void*    (*AActor_StaticClass)(void);
static void*    (*AActor_GetActorLocation)(void* _this, void* vec);
static void*    (*AActor_GetActorLabel)(void* _this);
static void*    (*UGameplayStatics_GetAllActorsOfClass)(void* WorldContextObject, void* ActorClass, void* OutActors);
static void     (*FScreenshotRequest_CreateViewportScreenShotFilename)(wchar_t** InOutFilename);
static void     (*FDebug_EnsureFailed)(const char* Expr, const char* File, int Line, const wchar_t* Msg);
static void     (*FDebug_AssertFailed)(const char* Expr, const char* File, int Line, const wchar_t* Msg);

static char     cOriginalCode[10];

static HANDLE   hProcess;
static HMODULE  hModule;
static HANDLE   hEvent;
static int      RequestSign;
static int      RequestType;
static float    CaptureRes;
static POINT    ViewportSize;
static int      UEVersion;

DECLARE_HOOK(UUnrealEdEngine_Tick, void, void*, float, char);
DECLARE_HOOK(FSceneViewport_ResizeViewport, void, void*, int, int, int);

void
Commnuication(
    int size,
    char* buf
);

UINT WINAPI
UnloadThread(
    LPVOID lpvParam
);

int
ReuqestGetActors(
);

int
InitFunctions(
    int ver,
    const char* prefix
);

int
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
        {
            CloseHandle(hEvent);
            return FALSE;
        }

        SetDisconnectedPipeFunc((void*)UnloadDll);
    }

    if (NULL == (hProcess = GetCurrentProcess()))
    {
        StopPipeComm();
        return FALSE;
    }

    return TRUE;
}

int
UnloadDll(
)
{
    if (NULL != hEvent)
    {
        SetEvent(hEvent);
        CloseHandle(hEvent);
    }

    // 캡처 로직이 대기상태인지 확인 후 후킹 했던 함수를 원상복구 시킵니다.
    // Check if the capture logic is in a waiting state, then restore the hooked function to its original state.
    while (0 != RequestSign)
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

    if (FDebug_AssertFailed)
        WriteMemByte(hProcess, (ULONG_PTR)FDebug_AssertFailed, *(cOriginalCode + 2));

    // 자기 자신을 언로드 합니다.
    // Revert the injection and unload myself.
    _beginthreadex(NULL, 0, UnloadThread,
        (PVOID)hModule, 0, NULL);

    return TRUE;
}

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

            int readSize = 0;

            if (UEVersion == 4)
                readSize = sizeof(Vec3D);
            else if (UEVersion == 5)
                readSize = sizeof(Vec3DD);
            
            if (0 == readSize)
                goto ERROR_RESULT;

            memcpy(ptr, pVec, readSize);

            SendPipeMessage(1 + readSize, buf);
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
        case MODE_GET_ACTORS:
        {
            RequestSign = 1;
            RequestType = REQUEST_GETACTORS;

            // 끝날 때까지 대기 ( 동기 )
            // Wait synchronously until it completes.
            WaitForSingleObject(hEvent, INFINITE);
            ResetEvent(hEvent);
            SendPipeMessage(1, buf);
            return;
        }
        case MODE_SET_XY:
        {
            int readSize = 0;

            if (UEVersion == 4)
                readSize = sizeof(Vec3D);
            else if (UEVersion == 5)
                readSize = sizeof(Vec3DD);

            if (readSize != size - 1)
                goto ERROR_RESULT;

            FEditorViewportClient_SetViewLocation(FLEVC_GCLEVC, ptr);
            FEditorViewportClient_Invalidate(FLEVC_GCLEVC, 0, 0);

            SendPipeMessage(1, buf);
            return;
        }
        case MODE_SET_ZOOM:
        {
            float fval;

            memcpy(&fval, ptr, 4);

            FEditorViewportClient_SetOrthoZoom(FLEVC_GCLEVC, fval);
            FEditorViewportClient_Invalidate(FLEVC_GCLEVC, 0, 0);

            SendPipeMessage(1, buf);
            return;
        }
        case MODE_SET_VER:
        {
            UEVersion = ptr[0];
            char size = ptr[1];
            char* prefix = &ptr[2];

            prefix[size] = 0;

            if (FALSE == InitFunctions(UEVersion, prefix))
            {
                StopPipeComm();
                goto ERROR_RESULT;
            }

            SendPipeMessage(1, buf);
            return;
        }
        case MODE_SET_XYZZ_CAPTURE:
        {
            int readSize = 0;
            Vec5D vec;
            Vec5DD vecd;

            if (UEVersion == 4)
                memcpy(&vec, ptr, readSize = sizeof(Vec5D));
            else if (UEVersion == 5)
                memcpy(&vecd, ptr, readSize = sizeof(Vec5DD));

            if (readSize != size - 1)
                goto ERROR_RESULT;

            FEditorViewportClient_SetViewLocation(FLEVC_GCLEVC, ptr);

            if (UEVersion == 4)
            {
                FEditorViewportClient_SetOrthoZoom(FLEVC_GCLEVC, vec.zoom);
                CaptureRes = vec.resval;
            }
            else if (UEVersion == 5)
            {
                FEditorViewportClient_SetOrthoZoom(FLEVC_GCLEVC, vecd.zoom);
                CaptureRes = vecd.resval;
            }

            FEditorViewportClient_Invalidate(FLEVC_GCLEVC, 0, 0);

            if (0 == CaptureRes)
            {
                SendPipeMessage(1, buf);
                return;
            }

            if (0 != RequestSign)
                return;

            RequestSign = 1;
            RequestType = REQUEST_CAPTURE;

            // 이미지 캡처가 끝날 때까지 대기 ( 동기 )
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
    // 후킹을 통해 동기적 로직을 실행해 이미지가 잘못 출력되는 것을 방지하는 과정입니다.
    // It is the process of executing synchronous logic through hooking to prevent images from being displayed incorrectly.

    wchar_t* wsz = NULL;

    if (RequestSign == 1)
    {
        switch (RequestType)
        {
        case REQUEST_CAPTURE:
        {
            if (NULL != GScreenshotBitmapIndex)
                WriteMemLong(hProcess, (ULONG_PTR)GScreenshotBitmapIndex, 0);

            if (NULL != (wsz = (wchar_t*)malloc(sizeof(wchar_t) * 50)))
            {
                swprintf_s(wsz, 50, L"HighResShot %g", CaptureRes);
                UUnrealEdEngine_Exec(GEngine, GWorld, wsz);
            }

            break;
        }
        case REQUEST_GETACTORS:
        {
            ReuqestGetActors();
            break;
        }
        }

    }

    // 첫 사이클은 HighResShot 명령어를 실행해 두 번째 사이클에 이미지를 출력
    // The first cycle executes the HighResShot command, and the second cycle outputs the image.
    UUnrealEdEngine_Tick_original(_this, DeltaSeconds, bIdleMode);

    switch (RequestSign)
    {
    case 1:
        if (NULL != wsz)
            free(wsz);
        RequestSign++;
        return;
    case 2:
        // 두 번째 사이클에 이미지 출력이 완료되어 (UUnrealEdEngine_Tick_original 안에 이미지 저장 로직이 들어가 있음) 대기 상태 해제.
        // The second cycle completes the image output, and the waiting state is released (the image saving logic is included in UUnrealEdEngine_Tick_original).
        SetEvent(hEvent);
        RequestSign = 0;
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
    // 렌더링 영역이 256 x 256 크기인지 확인하기 위해 후킹한 로직입니다.
    // This is the hooked logic to verify that the rendering area is 256 x 256 in size.

    FSceneViewport_ResizeViewport_original(_this, NewSizeX, NewSizeY, NewWindowMode);

    ViewportSize.x = NewSizeX;
    ViewportSize.y = NewSizeY;
}

UINT WINAPI
UnloadThread(
    LPVOID lpvParam
)
{
    Sleep(100);

    FreeLibrary((HMODULE)lpvParam);
    return FALSE;
}

int
ReuqestGetActors(
)
{
    if (NULL == AActor_StaticClass ||
        NULL == UGameplayStatics_GetAllActorsOfClass ||
        NULL == AActor_GetActorLocation ||
        NULL == AActor_GetActorLabel)
        return 0;

    void* AActorStaticClass;

    EXCEPTION_MACRO(
        AActorStaticClass = AActor_StaticClass();,
        return 0;
    );

    if (NULL == AActorStaticClass)
        return 0;

    TArray Array = { 0 };

    EXCEPTION_MACRO(
        UGameplayStatics_GetAllActorsOfClass(GWorld, AActorStaticClass, &Array); ,
        return 0;
    );

    if (Array.ArrayNum <= 0)
        return 0;

    FILE* fp;
    errno_t err = _wfopen_s(&fp, L"AActors.txt", L"w");

    if (err != 0 || fp == NULL)
        return 0;

    for (int i = 0; i < Array.ArrayNum; ++i)
    {
        void* AActor;

        if (0 == (AActor = (void*)ReadMemPtr(hProcess,
            (unsigned long long)((ULONG_PTR*)Array.AllocatorInstance + i))))
            continue;

        if (NULL == AActor)
            continue;

        Vec3D vec;

        if (UEVersion == 4)
        {
            EXCEPTION_MACRO(
                AActor_GetActorLocation(AActor, &vec);,
                goto ERROR_RESULT;
            );
        }
        else if (UEVersion == 5)
        {
            Vec3DD vecd;

            EXCEPTION_MACRO(
                AActor_GetActorLocation(AActor, &vecd); ,
                goto ERROR_RESULT;
            );

            vec.x = (float)vecd.x;
            vec.y = (float)vecd.y;
            vec.z = (float)vecd.z;
        }

        void* FStr;

        EXCEPTION_MACRO(
            FStr = AActor_GetActorLabel(AActor); ,
            goto ERROR_RESULT;
        );

        if (NULL == FStr)
            continue;

        wchar_t* name;

        if (0 == (name = (void*)ReadMemPtr(hProcess, (ULONG_PTR)FStr)))
            continue;

        fwprintf_s(fp, L"%s,%.5f,%.5f,%.5f\n", name, vec.x, vec.y, vec.z);
    }

ERROR_RESULT:

    fclose(fp);
    return 1;
}

int
InitFunctions(
    int ver,
    const char* prefix
)
{
    char Buf[255];

    // ----------------------ShooterGameEditor-Engine.dll----------------------
    {
        sprintf_s(Buf, sizeof(Buf), "%s-Engine.dll", prefix);
        DWORD_PTR DLL = (DWORD_PTR)GetModuleHandle(Buf);

        if (0 == DLL)
            return FALSE;

        // ?GEngine@@3PEAVUEngine@@EA
        GEngine = GetFunction((HMODULE)DLL, "GEngine@");

        if (0 == (GEngine = (void*)ReadMemPtr(hProcess, (ULONG_PTR)GEngine)))
            return FALSE;

        // ?GWorld@@3VUWorldProxy@@A
        GWorld = GetFunction((HMODULE)DLL, "GWorld@");

        if (0 == (GWorld = (void*)ReadMemPtr(hProcess, (ULONG_PTR)GWorld)))
            return FALSE;

        // ?CreateViewportScreenShotFilename@FScreenshotRequest@@SAXAEAVFString@@@Z
        FScreenshotRequest_CreateViewportScreenShotFilename =
            GetFunction((HMODULE)DLL, "CreateViewportScreenShotFilename@");

        // ?StaticClass@AActor@@SAPEAVUClass@@XZ
        AActor_StaticClass = GetFunction((HMODULE)DLL, "StaticClass@AActor");

        // ?GetAllActorsOfClass@UGameplayStatics@@SAXPEAVUObject@@V?$TSubclassOf@VAActor@@@@AEAV?$TArray@PEAVAActor@@VFDefaultAllocator@@@@@Z
        UGameplayStatics_GetAllActorsOfClass = GetFunction((HMODULE)DLL, "GetAllActorsOfClass@");

        // ?GetActorLocation@AActor@@QEBA?AVFVector@@XZ
        AActor_GetActorLocation = GetFunction((HMODULE)DLL, "GetActorLocation@AActor");

        // ?GetActorLabel@AActor@@QEBAAEBVFString@@XZ
        AActor_GetActorLabel = GetFunction((HMODULE)DLL, "GetActorLabel@");

        // ?ResizeViewport@FSceneViewport@@EEAAXIIW4Type@EWindowMode@@HH@Z
        FSceneViewport_ResizeViewport = GetFunction((HMODULE)DLL, "ResizeViewport@");

        if (0 == FSceneViewport_ResizeViewport)
            return FALSE;
    }

    // ----------------------ShooterGameEditor-Core.dll----------------------
    {
        sprintf_s(Buf, sizeof(Buf), "%s-Core.dll", prefix);
        DWORD_PTR DLL = (DWORD_PTR)GetModuleHandle(Buf);

        if (0 == DLL)
            return FALSE;

        // ?GScreenshotBitmapIndex@@3HA
        GScreenshotBitmapIndex = GetFunction((HMODULE)DLL, "GScreenshotBitmapIndex@");

        // ?EnsureFailed@FDebug@@SAXPEBD0HPEB_W@Z
        FDebug_EnsureFailed = GetFunction((HMODULE)DLL, "EnsureFailed@FDebug");

        // ?AssertFailed@FDebug@@SAXPEBD0HPEB_WZZ
        FDebug_AssertFailed = GetFunction((HMODULE)DLL, "AssertFailed@FDebug");
    }

    // ----------------------ShooterGameEditor-UnrealEd.dll----------------------
    {
        sprintf_s(Buf, sizeof(Buf), "%s-UnrealEd.dll", prefix);
        DWORD_PTR DLL = (DWORD_PTR)GetModuleHandle(Buf);

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
        FEditorViewportClient_Invalidate = GetFunction((HMODULE)DLL, "Invalidate@FEditorViewportClient");

        if (0 == FEditorViewportClient_Invalidate)
            return FALSE;

        // ?Tick@UUnrealEdEngine@@UEAAXM_N@Z
        UUnrealEdEngine_Tick = GetFunction((HMODULE)DLL, "Tick@UUnrealEdEngine");

        if (0 == UUnrealEdEngine_Tick)
            return FALSE;
    }

    // ?? 왜 0x38 을 더해야 하는지 모르겠습니다.
    // I don't understand why 0x38 needs to be added.'

    if (ver == 4)
        GEngine = (char*)GEngine + 0x38;
    else if (ver == 5)
        GEngine = (char*)GEngine + 0x30;

    // HOOK Function
    if (FALSE == HookFunction(FSceneViewport_ResizeViewport,
        (void*)&Hook_FSceneViewport_ResizeViewport,
        (void*)&FSceneViewport_ResizeViewport_original))
        return FALSE;

    if (FALSE == HookFunction(UUnrealEdEngine_Tick,
        (void*)&Hook_UUnrealEdEngine_Tick,
        (void*)&UUnrealEdEngine_Tick_original))
        return FALSE;

    // 어셈블리어 코드 nop (0xC3) 을 삽입해 로직이 실행되는 것을을 막습니다.
    // Insert the assembly code NOP (0xC3) to prevent the logic from executing.
    if (ReadMemByte(hProcess, (ULONG_PTR)FScreenshotRequest_CreateViewportScreenShotFilename, cOriginalCode + 0))
        // CreateViewportScreenShotFilename 를 막는 이유 : 이미지 출력 경로가 의도한 대로 하기 위해서입니다.
        // The reason for blocking CreateViewportScreenShotFilename: To ensure that the image output path follows the intended direction.
        WriteMemByte(hProcess, (ULONG_PTR)FScreenshotRequest_CreateViewportScreenShotFilename, 0xC3);

    if (ReadMemByte(hProcess, (ULONG_PTR)FDebug_EnsureFailed, cOriginalCode + 1))
        // EnsureFailed 를 막는 이유 : IsInGameThread 결과가 false이기 때문에 외부 프로그램이 Exec 커맨드를 쓰면 ADK에서 오류가 발생해 종료되기 때문입니다.
        // The reason for blocking EnsureFailed: Since the result of IsInGameThread is false, if an external program executes an Exec command, 
        // ADK will encounter an error and terminate.
        WriteMemByte(hProcess, (ULONG_PTR)FDebug_EnsureFailed, 0xC3);

    if (ReadMemByte(hProcess, (ULONG_PTR)FDebug_AssertFailed, cOriginalCode + 2))
        // AssertFailed 를 막는 이유 : IsInGameThread 결과가 false이기 때문에 외부 프로그램이 Exec 커맨드를 쓰면 ADK에서 오류가 발생해 종료되기 때문입니다.
        // The reason for blocking AssertFailed: Since the result of IsInGameThread is false, if an external program executes an Exec command, 
        // ADK will encounter an error and terminate.
        WriteMemByte(hProcess, (ULONG_PTR)FDebug_AssertFailed, 0xC3);

    return TRUE;
}