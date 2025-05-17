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
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_EDITOR), NULL, DlgProc, (LPARAM)hInstance);

    return (int)FALSE;
}

INT_PTR CALLBACK
DlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    static int bEnable;
    static HINSTANCE hInst;
    static int iQuality;

    switch (message)
    {
        case WM_INITDIALOG:
        {
            LoadReg(hDlg);
            bEnable = FALSE;

            hInst = (HINSTANCE)lParam;
            iQuality = LoadVal("IDC_EDIT_QULITY", "100");

            if (iQuality < 0)
                iQuality = 0;

            if (iQuality > 100)
                iQuality = 100;

            return (INT_PTR)TRUE;
        }
        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDC_BUTTON_OPEN:
                {
                    if (FALSE == OpenProcessProc(hDlg))
                        return (INT_PTR)FALSE;

                    bEnable = TRUE;
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
                    if (FALSE == bEnable &&
                        FALSE == DlgProc(hDlg, WM_COMMAND, IDC_BUTTON_OPEN, 0))
                        return (INT_PTR)FALSE;

                    SaveReg(hDlg);

                    if (StartCapture(iQuality))
                        DisableAllControls(hDlg);
                    
                    break;
                }
                case IDC_BUTTON_ZORI:
                {
                    if (FALSE == bEnable &&
                        FALSE == DlgProc(hDlg, WM_COMMAND, IDC_BUTTON_OPEN, 0))
                        return (INT_PTR)FALSE;

                    SetButtonAction(0);
                    break;
                }
                case IDC_BUTTON_ZHALF:
                {
                    if (FALSE == bEnable &&
                        FALSE == DlgProc(hDlg, WM_COMMAND, IDC_BUTTON_OPEN, 0))
                        return (INT_PTR)FALSE;

                    SetButtonAction(1);
                    break;
                }
                case IDC_BUTTON_CENTER:
                {
                    if (FALSE == bEnable &&
                        FALSE == DlgProc(hDlg, WM_COMMAND, IDC_BUTTON_OPEN, 0))
                        return (INT_PTR)FALSE;

                    SetButtonAction(2);
                    break;
                }
                case IDC_BUTTON_ZEROXY:
                {
                    if (FALSE == bEnable &&
                        FALSE == DlgProc(hDlg, WM_COMMAND, IDC_BUTTON_OPEN, 0))
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
                case IDC_COMBO_EXTIMG:
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        int newVal;

                        if (-1 != (newVal = GetImageQuality(hInst, hDlg, iQuality)))
                            SaveVal("IDC_EDIT_QULITY", iQuality = newVal);
                    }

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