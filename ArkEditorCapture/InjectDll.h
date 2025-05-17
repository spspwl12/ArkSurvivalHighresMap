#pragma once

int
InjectDll(
    void* hdl,
    const char* dllPath
);

int
UnloadDll(
    void* hdl,
    const char* dllPath
);