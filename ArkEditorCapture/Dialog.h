#pragma once

#include <windows.h>

float 
GetDlgItemFloat(
    HWND hDlg,
    int nIDDlgItem
);

void 
SetDlgItemFloat(
    HWND hDlg,
    int nIDDlgItem,
    float value
);

void
CopyTextDlgItem(
    HWND hDlg,
    int nSrcDlgItem,
    int nDstDlgItem
);