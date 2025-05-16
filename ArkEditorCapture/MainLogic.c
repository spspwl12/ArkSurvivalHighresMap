#include <stdlib.h>
#include <Shlobj.h>
#include "resource.h"
#include "Reg.h"
#include "MainLogic.h"

void
LoadReg(
    HWND        hDlg
)
{
    char Buf[255] = { 0 };

    ReadReg("ArkEditorCapture", "IDC_EDIT_PN", "UE4Editor.exe", Buf);
    SetDlgItemText(hDlg, IDC_EDIT_PN, Buf);

    ReadReg("ArkEditorCapture", "IDC_EDIT_DLPATH", "", Buf);
    SetDlgItemText(hDlg, IDC_EDIT_DLPATH, Buf);

    ReadReg("ArkEditorCapture", "IDC_EDIT_SVPATH", "C:\\", Buf);
    SetDlgItemText(hDlg, IDC_EDIT_SVPATH, Buf);

    ReadReg("ArkEditorCapture", "IDC_EDIT_X2", "-1000000", Buf);
    SetDlgItemText(hDlg, IDC_EDIT_X2, Buf);

    ReadReg("ArkEditorCapture", "IDC_EDIT_X3", "1000000", Buf);
    SetDlgItemText(hDlg, IDC_EDIT_X3, Buf);

    ReadReg("ArkEditorCapture", "IDC_EDIT_Y2", "-1000000", Buf);
    SetDlgItemText(hDlg, IDC_EDIT_Y2, Buf);

    ReadReg("ArkEditorCapture", "IDC_EDIT_Y3", "1000000", Buf);
    SetDlgItemText(hDlg, IDC_EDIT_Y3, Buf);

    ReadReg("ArkEditorCapture", "IDC_EDIT_Z2", "12800000", Buf);
    SetDlgItemText(hDlg, IDC_EDIT_Z2, Buf);

    ReadReg("ArkEditorCapture", "IDC_EDIT_Z3", "0", Buf);
    SetDlgItemText(hDlg, IDC_EDIT_Z3, Buf);

    ReadReg("ArkEditorCapture", "IDC_EDIT_Z4", "7", Buf);
    SetDlgItemText(hDlg, IDC_EDIT_Z4, Buf);

    SendDlgItemMessage(hDlg, IDC_COMBO_EXTIMG, CB_ADDSTRING, 0, (LPARAM)TEXT("BMP"));
    SendDlgItemMessage(hDlg, IDC_COMBO_EXTIMG, CB_ADDSTRING, 0, (LPARAM)TEXT("JPG"));
    SendDlgItemMessage(hDlg, IDC_COMBO_EXTIMG, CB_ADDSTRING, 0, (LPARAM)TEXT("GIF"));
    SendDlgItemMessage(hDlg, IDC_COMBO_EXTIMG, CB_ADDSTRING, 0, (LPARAM)TEXT("PNG"));
    SendDlgItemMessage(hDlg, IDC_COMBO_EXTIMG, CB_ADDSTRING, 0, (LPARAM)TEXT("WEBP"));

    ReadReg("ArkEditorCapture", "IDC_COMBO_EXTIMG", "0", Buf);
    SendDlgItemMessage(hDlg, IDC_COMBO_EXTIMG, CB_SETCURSEL, atoi(Buf), 0);

    SendDlgItemMessage(hDlg, IDC_COMBO_TILESZ, CB_ADDSTRING, 0, (LPARAM)TEXT("256"));
    SendDlgItemMessage(hDlg, IDC_COMBO_TILESZ, CB_ADDSTRING, 0, (LPARAM)TEXT("512"));
    SendDlgItemMessage(hDlg, IDC_COMBO_TILESZ, CB_ADDSTRING, 0, (LPARAM)TEXT("1024"));
    SendDlgItemMessage(hDlg, IDC_COMBO_TILESZ, CB_ADDSTRING, 0, (LPARAM)TEXT("2048"));
    SendDlgItemMessage(hDlg, IDC_COMBO_TILESZ, CB_ADDSTRING, 0, (LPARAM)TEXT("4096"));
    SendDlgItemMessage(hDlg, IDC_COMBO_TILESZ, CB_ADDSTRING, 0, (LPARAM)TEXT("8192"));
    SendDlgItemMessage(hDlg, IDC_COMBO_TILESZ, CB_ADDSTRING, 0, (LPARAM)TEXT("16384"));

    ReadReg("ArkEditorCapture", "IDC_COMBO_TILESZ", "0", Buf);
    SendDlgItemMessage(hDlg, IDC_COMBO_TILESZ, CB_SETCURSEL, atoi(Buf), 0);
}

void
SaveReg(
    HWND        hDlg
)
{
    char Buf[255] = { 0 };

    GetDlgItemText(hDlg, IDC_EDIT_PN, Buf, sizeof(Buf));
    WriteReg("ArkEditorCapture", "IDC_EDIT_PN", Buf);

    GetDlgItemText(hDlg, IDC_EDIT_DLPATH, Buf, sizeof(Buf));
    WriteReg("ArkEditorCapture", "IDC_EDIT_DLPATH", Buf);

    GetDlgItemText(hDlg, IDC_EDIT_SVPATH, Buf, sizeof(Buf));
    WriteReg("ArkEditorCapture", "IDC_EDIT_SVPATH", Buf);

    GetDlgItemText(hDlg, IDC_EDIT_X2, Buf, sizeof(Buf));
    WriteReg("ArkEditorCapture", "IDC_EDIT_X2", Buf);

    GetDlgItemText(hDlg, IDC_EDIT_X3, Buf, sizeof(Buf));
    WriteReg("ArkEditorCapture", "IDC_EDIT_X3", Buf);

    GetDlgItemText(hDlg, IDC_EDIT_Y2, Buf, sizeof(Buf));
    WriteReg("ArkEditorCapture", "IDC_EDIT_Y2", Buf);

    GetDlgItemText(hDlg, IDC_EDIT_Y3, Buf, sizeof(Buf));
    WriteReg("ArkEditorCapture", "IDC_EDIT_Y3", Buf);

    GetDlgItemText(hDlg, IDC_EDIT_Z2, Buf, sizeof(Buf));
    WriteReg("ArkEditorCapture", "IDC_EDIT_Z2", Buf);

    GetDlgItemText(hDlg, IDC_EDIT_Z3, Buf, sizeof(Buf));
    WriteReg("ArkEditorCapture", "IDC_EDIT_Z3", Buf);

    GetDlgItemText(hDlg, IDC_EDIT_Z4, Buf, sizeof(Buf));
    WriteReg("ArkEditorCapture", "IDC_EDIT_Z4", Buf);

    int sel = (int)SendDlgItemMessage(hDlg, IDC_COMBO_EXTIMG, CB_GETCURSEL, 0, 0);

    Buf[0] = (char)sel + '0';
    Buf[1] = 0;

    WriteReg("ArkEditorCapture", "IDC_COMBO_EXTIMG", Buf);

    sel = (int)SendDlgItemMessage(hDlg, IDC_COMBO_TILESZ, CB_GETCURSEL, 0, 0);

    Buf[0] = (char)sel + '0';
    Buf[1] = 0;

    WriteReg("ArkEditorCapture", "IDC_COMBO_TILESZ", Buf);
}

void
DisableAllControls(
    HWND        hDlg
)
{
    for (int i = IDC_AAA_START_CTRL; i < IDC_zzz_END_CTRL; ++i)
        EnableWindow(GetDlgItem(hDlg, i), FALSE);

    KillTimer(hDlg, 1);
}

void
EnableValidControls(
    HWND        hDlg
)
{
    for (int i = IDC_AAA_START_CTRL; i < IDC_zzz_END_CTRL; ++i)
        EnableWindow(GetDlgItem(hDlg, i), TRUE);

    EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_OPEN), FALSE);

    SetTimer(hDlg, 1, 100, NULL);
    SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_START), "Start");
}

INT CALLBACK
BrowseCallbackProc(
    HWND        hWnd,
    UINT        uMsg,
    LPARAM      lParam,
    LPARAM      lpData
)
{
    if (uMsg == BFFM_INITIALIZED)
        SendMessage(hWnd, BFFM_SETSELECTION, TRUE, lpData);

    return 0;
}

int
BrowseFolder(
	HWND		hWndOwner,
	LPCSTR		DialogTitle,
	LPSTR		ResultPath
)
{
	LPITEMIDLIST	pidlBrowse;
	BROWSEINFO		brInfo = { 0 };

    DWORD attributes = GetFileAttributes(ResultPath);

    if (INVALID_FILE_ATTRIBUTES != attributes &&
        0 == (attributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        char* ptr = strrchr(ResultPath, '\\');

        if (ptr)
            *ptr = 0;
    }

	brInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT | BIF_VALIDATE;
	brInfo.hwndOwner = hWndOwner;
	brInfo.pszDisplayName = ResultPath;
	brInfo.lpszTitle = DialogTitle;
	brInfo.lpfn = BrowseCallbackProc;
	brInfo.lParam = (LPARAM)ResultPath;

	if (pidlBrowse = SHBrowseForFolder(&brInfo))
	{
		SHGetPathFromIDList(pidlBrowse, ResultPath);
		return TRUE;
	}

	return FALSE;
}

int 
BrowseFile(
    HWND        hWnd, 
    LPSTR       Path,
    LPCSTR      Filter
)
{
    OPENFILENAME ofn = { 0 };
    char Buffer[MAX_PATH] = { 0 };
    char* Part;

    memcpy(Buffer, Path, sizeof(Buffer));

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = Buffer;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    ofn.lpstrFilter = Filter;

    const int bRst = GetOpenFileName(&ofn);
    GetFullPathName(Buffer, MAX_PATH, Path, &Part);

    return bRst;
}