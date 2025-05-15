#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <stdlib.h>
#include <Shlobj.h>
#include "Logic.h"
#include "Dialog.h"
#include "Process.h"
#include "resource.h"
#include "Reg.h"
#include "MainLogic.h"

int APIENTRY 
WinMain(
    _In_        HINSTANCE hInstance,
    _In_opt_    HINSTANCE hPrevInstance,
    _In_        LPSTR     lpCmdLine,
    _In_        int       nCmdShow
)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_EDITOR), NULL, DlgProc);

    return (int)0;
}

INT_PTR CALLBACK
DlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    UNREFERENCED_PARAMETER(lParam);

    static int bEnable;

    switch (message)
    {
        case WM_INITDIALOG:
        {
            LoadReg(hDlg);
            bEnable = 0;

            return (INT_PTR)TRUE;
        }
        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDC_BUTTON_OPEN:
                {
                    char buf[255];

                    GetDlgItemText(hDlg, IDC_EDIT_PN, buf, sizeof(buf));
                    const unsigned long pID = GetpIDByName(buf);

                    if (0 == pID)
                    {
                        MessageBox(hDlg, "Process not found.", "Error", MB_ICONEXCLAMATION);
                        return (INT_PTR)FALSE;
                    }

                    GetDlgItemText(hDlg, IDC_EDIT_DLPATH, buf, sizeof(buf));

                    if (FALSE == LoadArkEditor(hDlg, pID, buf))
                        return (INT_PTR)FALSE;
  
                    EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_OPEN), FALSE);
                    SetTimer(hDlg, 1, 100, NULL);
                    bEnable = 1;

                    return (INT_PTR)TRUE;
                }
                case IDC_BUTTON_BROWSEDL:
                {
                    char path[MAX_PATH];

                    GetDlgItemText(hDlg, IDC_EDIT_DLPATH, path, sizeof(path));

                    if (BrowseFile(hDlg, path, "DLL (*.dll)\0*.dll\0"))
                        SendDlgItemMessage(hDlg, IDC_EDIT_DLPATH, WM_SETTEXT, (WPARAM)MAX_PATH, (LPARAM)path);

                    break;
                }
                case IDC_BUTTON_BROWSESV:
                {
                    char path[MAX_PATH];

                    GetDlgItemText(hDlg, IDC_EDIT_SVPATH, path, sizeof(path));

                    if (BrowseFolder(hDlg, "Please select a folder.", path))
                        SendDlgItemMessage(hDlg, IDC_EDIT_SVPATH, WM_SETTEXT, (WPARAM)MAX_PATH, (LPARAM)path);

                    break;
                }
                case IDC_BUTTON_START:
                {
                    if (!bEnable &&
                        !DlgProc(hDlg, WM_COMMAND, IDC_BUTTON_OPEN, 0))
                        return (INT_PTR)FALSE;

                    SaveReg(hDlg);

                    if (StartCapture())
                        DisableAllControls(hDlg);
                    
                    break;
                }
                case IDC_BUTTON_ZORI:
                {
                    if (!bEnable &&
                        !DlgProc(hDlg, WM_COMMAND, IDC_BUTTON_OPEN, 0))
                        return (INT_PTR)FALSE;

                    SetButtonAction(0);
                    break;
                }
                case IDC_BUTTON_ZHALF:
                {
                    if (!bEnable &&
                        !DlgProc(hDlg, WM_COMMAND, IDC_BUTTON_OPEN, 0))
                        return (INT_PTR)FALSE;

                    SetButtonAction(1);
                    break;
                }
                case IDC_BUTTON_CENTER:
                {
                    if (!bEnable &&
                        !DlgProc(hDlg, WM_COMMAND, IDC_BUTTON_OPEN, 0))
                        return (INT_PTR)FALSE;

                    SetButtonAction(2);
                    break;
                }
                case IDC_BUTTON_ZEROXY:
                {
                    if (!bEnable &&
                        !DlgProc(hDlg, WM_COMMAND, IDC_BUTTON_OPEN, 0))
                        return (INT_PTR)FALSE;

                    SetButtonAction(3);
                    break;
                }
                case IDC_BUTTON_SETXYTO:
                {
                    CopyTextDlgItem(hDlg, IDC_EDIT_X1, IDC_EDIT_X2);
                    CopyTextDlgItem(hDlg, IDC_EDIT_Y1, IDC_EDIT_Y2);
                    break;
                }
                case IDC_BUTTON_SETXYFR:
                {
                    CopyTextDlgItem(hDlg, IDC_EDIT_X1, IDC_EDIT_X3);
                    CopyTextDlgItem(hDlg, IDC_EDIT_Y1, IDC_EDIT_Y3);
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case WM_TIMER:
        {
            LoadValue();
            break;
        }
        case WM_CLOSE:
        {
            SaveReg(hDlg);
            CloseArkEditor();
            EndDialog(hDlg, 1);

            return (INT_PTR)TRUE;
        }
    }

    return (INT_PTR)FALSE;
}