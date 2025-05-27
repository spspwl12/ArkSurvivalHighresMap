#pragma once

int
HookFunction(
    void* pTarget,
    void* pDetour,
    void** ppOriginal
);

int
UnHookFunction(
    void* pTarget
);