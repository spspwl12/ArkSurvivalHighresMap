#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#include <tlhelp32.h>

unsigned long 
GetpIDByName(
    const char* name
) 
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot == INVALID_HANDLE_VALUE) 
        return 0;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe)) {
        do {
            if (strcmp(pe.szExeFile, name) == 0) {
                CloseHandle(hSnapshot);
                return pe.th32ProcessID;
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
    return 0;
}

void* 
GetModuleBaseAddress(
    unsigned long pID,
    const char* moduleName,
    char* exportPath
)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pID);

    if (hSnapshot == INVALID_HANDLE_VALUE) 
        return (void*)0;

    char* name = strrchr(moduleName, '\\');

    if (NULL == name)
        name = (char*)moduleName;
    else
        name++;

    MODULEENTRY32 me;
    me.dwSize = sizeof(MODULEENTRY32);

    if (Module32First(hSnapshot, &me)) {
        do {
            if (strcmp(me.szModule, name) == 0) {
                CloseHandle(hSnapshot);

                if (exportPath)
                    memcpy(exportPath, me.szExePath, sizeof(me.szExePath));

                return (void*)me.modBaseAddr;
            }
        } while (Module32Next(hSnapshot, &me));
    }

    CloseHandle(hSnapshot);
    return (void*)0;
}

BOOL CALLBACK 
EnumWindowsProc(
    HWND hWnd, 
    LPARAM lParam
) 
{
    DWORD pID;
    GetWindowThreadProcessId(hWnd, &pID);

    if (pID == ((ULONG_PTR*)lParam)[0] &&
        IsWindowVisible(hWnd))
    {
        ((ULONG_PTR*)lParam)[1] = (ULONG_PTR)hWnd;
        return FALSE;
    }

    return TRUE;
}

void* 
FindWindowsByPID(
    unsigned long pID
) 
{
    if (0 == pID)
        return 0;

    ULONG_PTR param[] = {
        pID,
        0x0
    };

    EnumWindows(EnumWindowsProc, (LPARAM)param);

    return (void*)param[1];
}