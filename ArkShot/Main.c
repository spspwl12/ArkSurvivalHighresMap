#include <Windows.h>
#include "Logic.h"

BOOL APIENTRY DllMain(HMODULE   hModule,
                      DWORD     ul_reason_for_call,                
                      LPVOID    lpReserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
            if (FALSE == LoadDll(hModule))
                UnloadDll();

            return TRUE;
        }
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}