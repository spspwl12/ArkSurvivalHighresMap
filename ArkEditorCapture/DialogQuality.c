#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#include <commctrl.h>
#include "resource.h"

INT_PTR CALLBACK
DialogQualityProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    static int val;

    switch (message)
    {
        case WM_INITDIALOG:
        {
            val = (int)lParam;

            if (val > 100)
                val = 100;

            if (val < 0)
                val = 0;

            SendDlgItemMessage(hDlg, IDC_EDIT_QULITY, EM_LIMITTEXT, (WPARAM)3, 0);

        	SendDlgItemMessage(hDlg, IDC_SLIDER_QULITY, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 100));
			SendDlgItemMessage(hDlg, IDC_SLIDER_QULITY, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)val);
            SendDlgItemMessage(hDlg, IDC_SLIDER_QULITY, TBM_SETTHUMBLENGTH, (WPARAM)20, 0);

            SetDlgItemInt(hDlg, IDC_EDIT_QULITY, val, FALSE);

            return (INT_PTR)TRUE;
        }
        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDOK:
                {
                    EndDialog(hDlg, val);
                    return (INT_PTR)TRUE;
                }
                case IDCANCEL:
                {
                    EndDialog(hDlg, -1);
                    return (INT_PTR)TRUE;
                }
                case IDC_EDIT_QULITY:
                {
                    if (HIWORD(wParam) == EN_CHANGE)
                    {
                        int changed = 0;

                        val = GetDlgItemInt(hDlg, IDC_EDIT_QULITY, NULL, FALSE);

                        if (changed = (val > 100))
                            val = 100;
                        else if (changed = (val < 0))
                            val = 0;

                        SendDlgItemMessage(hDlg, IDC_SLIDER_QULITY, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)val);

                        if (changed)
                            SetDlgItemInt(hDlg, IDC_EDIT_QULITY, val, FALSE);
                    }
                }
                default:
                    break;
                }
            break;
        }
        case WM_HSCROLL:
        {
            val = (int)SendDlgItemMessage(hDlg, IDC_SLIDER_QULITY, TBM_GETPOS, 0, 0);
            SetDlgItemInt(hDlg, IDC_EDIT_QULITY, val, FALSE);

            break;
        }
        case WM_CLOSE:
        {
            EndDialog(hDlg, -1);
            return (INT_PTR)TRUE;
        }
    }

    return (INT_PTR)FALSE;
}

int
GetImageQuality(
    HINSTANCE hInst,
    HWND hDlg,
    int val
)
{
    const int index = (int)SendDlgItemMessage(hDlg,
        IDC_COMBO_EXTIMG, CB_GETCURSEL, 0, 0);

    char Buf[255] = { 0 };
    SendDlgItemMessage(hDlg, IDC_COMBO_EXTIMG, CB_GETLBTEXT, index, (LPARAM)Buf);

    if (0 == strcmp(Buf, "JPG") ||
        0 == strcmp(Buf, "WEBP"))
    {
        return (int)DialogBoxParam(hInst,
            MAKEINTRESOURCE(IDD_QUALITY), hDlg, DialogQualityProc, val);
    }

    return -1;
}