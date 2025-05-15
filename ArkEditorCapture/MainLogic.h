#pragma once
#include <windows.h>

INT_PTR CALLBACK
DlgProc(
    HWND        hDlg,
    UINT        message,
    WPARAM      wParam,
    LPARAM      lParam
);

void
LoadReg(
    HWND        hDlg
);

void
SaveReg(
    HWND        hDlg
);

void
DisableAllControls(
    HWND        hDlg
);

void
EnableValidControls(
    HWND        hDlg
);

int
BrowseFolder(
    HWND		hWndOwner,
    LPCSTR		DialogTitle,
    LPSTR		ResultPath
);

int
BrowseFile(
    HWND        hWnd,
    LPSTR       Path,
    LPCSTR      Filter
);