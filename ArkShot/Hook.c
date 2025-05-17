#include "..\libMinHook\MinHook.h"

#if defined _DEBUG
#pragma comment(lib, "..\\Build\\libMinHook.x64d.lib")
#else
#pragma comment(lib, "..\\Build\\libMinHook.x64.lib")
#endif

static int HookCount;

int
HookFunction(
    void* pTarget,
    void* pDetour,
    void** ppOriginal
)
{
    // Init Hooking Module
    if (1 > HookCount)
    {
        if (MH_OK != MH_Initialize())
            return FALSE;

        HookCount = 1;
    }

    // Hook Function
    if (MH_OK != MH_CreateHook(pTarget, pDetour, ppOriginal))
        return FALSE;

    if (MH_OK != MH_EnableHook(pTarget))
        return FALSE;

    ++HookCount;

    return TRUE;
}

int
UnHookFunction(
    void* pTarget
)
{
    if (HookCount > 1)
    {
        if (MH_OK != MH_DisableHook(pTarget))
            return FALSE;

        if (MH_OK != MH_RemoveHook(pTarget))
            return FALSE;

        --HookCount;
    }

    if (1 < HookCount)
        return TRUE;

    if (MH_OK != MH_Uninitialize())
        return FALSE;

    HookCount = 0;

    return TRUE;
}