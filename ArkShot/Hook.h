#pragma once

char
HookFunction(
    void* pTarget,
    void* pDetour,
    void** ppOriginal
);

char
UnHookFunction(
    void* pTarget
);