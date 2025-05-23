#pragma once

#include "../ArkShot/Declare.h"

Vec2D
GetEndCoord(
    Vec2D CoordTo,
    Vec2D CoordFrom,
    const float multiplier
);

int
BrowseFolder(
    void* hWndOwner,
    const char*	DialogTitle,
    char* ResultPath
);

int
BrowseFile(
    void* hWnd,
    char* Path,
    const char* Filter
);

int
IsNumeric(
    const char* str
);