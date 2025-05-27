#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "Dialog.h"

float 
GetDlgItemFloat(
    void* hDlg, 
    int nIDDlgItem
) 
{
    char buf[50];
    GetDlgItemText((HWND)hDlg, nIDDlgItem, buf, sizeof(buf));

    buf[49] = 0;

    return strtof(buf, NULL);
}

void 
SetDlgItemFloat(
    void* hDlg, 
    int nIDDlgItem,
    float value
) 
{
    char buf[50];

    sprintf_s(buf, sizeof(buf), "%.2f", value);
    buf[49] = 0;

    SetDlgItemText((HWND)hDlg, nIDDlgItem, buf);
}

void
EnableAllControls(
    void* hDlg,
    int startID,
    int endID,
    int bEnable
)
{
    for (int i = startID; i < endID; ++i)
        EnableWindow(GetDlgItem((HWND)hDlg, i), bEnable);
}

void
CopyTextDlgItem(
    void* hDlg,
    int nSrcDlgItem,
    int nDstDlgItem
)
{
    char buf[255] = { 0 };

    GetDlgItemText(hDlg, nSrcDlgItem, buf, sizeof(buf));
    if (0 >= strlen(buf))
        return;
    SetDlgItemText(hDlg, nDstDlgItem, buf);
}