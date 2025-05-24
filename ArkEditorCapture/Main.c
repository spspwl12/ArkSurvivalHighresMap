#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>

#include "Logic.h"
#include "DialogQuality.h"
#include "DialogPreview.h"
#include "resource.h"
#include "Utility.h"
#include "Reg.h"

INT_PTR CALLBACK
DlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
);

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
            FetchRegistryValue(hDlg);
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
                    if (FALSE == OpenArkEditor(hDlg))
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
                case IDC_BUTTON_PREVIEW:
                {
                    if (FALSE == bEnable &&
                        FALSE == DlgProc(hDlg, WM_COMMAND, IDC_BUTTON_OPEN, 0))
                        return (INT_PTR)FALSE;

                    OpenPreviewProc(hInst, hDlg, iQuality);
                    break;
                }
                case IDC_BUTTON_START:
                {
                    if (FALSE == bEnable &&
                        FALSE == DlgProc(hDlg, WM_COMMAND, IDC_BUTTON_OPEN, 0))
                        return (INT_PTR)FALSE;

                    StartCapture(iQuality, LOGIC_MODE_CAPTURE, NULL);
                    break;
                }
                case IDC_BUTTON_ZEROXY:
                {
                    if (FALSE == bEnable &&
                        FALSE == DlgProc(hDlg, WM_COMMAND, IDC_BUTTON_OPEN, 0))
                        return (INT_PTR)FALSE;

                    SetButtonAction(BUTTON_MODE_SETZERO);
                    break;
                }
                case IDC_BUTTON_ZORI:
                {
                    if (FALSE == bEnable &&
                        FALSE == DlgProc(hDlg, WM_COMMAND, IDC_BUTTON_OPEN, 0))
                        return (INT_PTR)FALSE;

                    SetButtonAction(BUTTON_MODE_ORIZOOM);
                    break;
                }
                case IDC_BUTTON_AUTOSET:
                {
                    if (FALSE == bEnable &&
                        FALSE == DlgProc(hDlg, WM_COMMAND, IDC_BUTTON_OPEN, 0))
                        return (INT_PTR)FALSE;

                    SetButtonAction(BUTTON_MODE_AUTOSIZE);
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
            QueryEngineVariable();
            break;
        }
        case WM_CLOSE:
        {
            StoreRegistryValue(hDlg);
            CloseArkEditor();
            EndDialog(hDlg, 1);

            return (INT_PTR)TRUE;
        }
    }

    return (INT_PTR)FALSE;
}