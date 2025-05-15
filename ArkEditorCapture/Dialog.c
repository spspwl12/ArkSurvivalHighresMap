#include "Dialog.h"
#include <stdio.h>

float 
GetDlgItemFloat(
    HWND hDlg, 
    int nIDDlgItem
) 
{

    char buf[50];
    GetDlgItemText(hDlg, nIDDlgItem, buf, sizeof(buf));

    buf[49] = 0;

    return strtof(buf, NULL);
}

void 
SetDlgItemFloat(
    HWND hDlg, 
    int nIDDlgItem,
    float value
) 
{

    char buf[50];

    sprintf_s(buf, sizeof(buf), "%.2f", value);
    buf[49] = 0;

    SetDlgItemText(hDlg, nIDDlgItem, buf);
}

void
CopyTextDlgItem(
    HWND hDlg,
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