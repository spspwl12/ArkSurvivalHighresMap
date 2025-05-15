#pragma once

char
InjectDll(
    void* hdl,
    const char* dllPath
);

char
UnloadDll(
    void* hdl,
    const char* dllPath
);