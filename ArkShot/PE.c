#include <Windows.h>

void*
GetFunction(
    void* hModule,
    const char* name
)
{
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule;
    PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((DWORD_PTR)hModule + pDosHeader->e_lfanew);

    DWORD exportDirRVA = pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    PIMAGE_EXPORT_DIRECTORY pExportDir = (PIMAGE_EXPORT_DIRECTORY)((DWORD_PTR)hModule + exportDirRVA);

    DWORD* pNames = (DWORD*)((DWORD_PTR)hModule + pExportDir->AddressOfNames);
    DWORD* pFunctions = (DWORD*)((DWORD_PTR)hModule + pExportDir->AddressOfFunctions);

    for (DWORD i = 0; i < pExportDir->NumberOfNames; i++)
        if (NULL != strstr((const char*)((DWORD_PTR)hModule + pNames[i]), name))
            return (void*)((DWORD_PTR)hModule + pFunctions[i]);

    return NULL;
}