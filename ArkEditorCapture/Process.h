#pragma once

unsigned long 
GetpIDByName(
    const char* processName
);

void* GetModuleBaseAddress(
    unsigned long pID,
    const char* moduleName,
    char* exportPath
);

void*
FindWindowsByPID(
    unsigned long pID
);