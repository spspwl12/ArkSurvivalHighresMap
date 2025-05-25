#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "resource.h"
#include "Logic.h"
#include "Image.h"
#include "Dialog.h"

static HINSTANCE hInstance;

int
ListFilesRecursively(
    HDC hdc,
    const char* basePath,
    POINT* maxSize
);

void
finishCapture(
);

INT_PTR CALLBACK
DlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
);

INT_PTR CALLBACK
RemoconDialogProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
);

INT_PTR CALLBACK
PreviewDialogProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    static HBITMAP hBitmap;
    static POINT BitmapSize;
    static HWND hWnd;
    static HWND hRmhWnd;
    static int State;
    static int quality;

    switch (message)
    {
        case WM_INITDIALOG:
        {
            State = SET_BIT(0, 0, TRUE);
            quality = (int)lParam;
            hWnd = hDlg;
            hRmhWnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_REMOTE), NULL, RemoconDialogProc);
        }
        case WM_RELOAD_PREVIEW:
        {
            if (FALSE == GET_BIT(State, 0))
                return (INT_PTR)FALSE;

            BitmapSize.x = BitmapSize.y = 0;
            State = SET_BIT(State, 0, FALSE);

            EnableAllControls(hRmhWnd, IDC_AAA_START_CTRL, IDC_ZZZ_END_CTRL, FALSE);
            StartCapture(quality, LOGIC_MODE_PREVIEW, finishCapture);
            return (INT_PTR)TRUE;
        }
        case WM_FIN_PREVIEW:
        {
            // SendMessage 또는 PostMessage API로 실행되는 것을 막는다.
            // Block execution triggered by the SendMessage or PostMessage API.
            if (NULL != hDlg)
            {
                State = SET_BIT(State, 0, TRUE);
                return (INT_PTR)FALSE;
            }

            char searchPath[MAX_PATH];
            GetModuleFileName(NULL, searchPath, MAX_PATH);
            char* ptr;

            if (NULL != (ptr = strrchr(searchPath, '\\')))
                *ptr = 0;

            sprintf_s(searchPath, sizeof(searchPath), "%s\\%s\\0", searchPath, PREVIEW_PREFIX);

            HDC hDC = NULL, hdcMem = NULL;

            if (NULL == (hDC = GetDC(0)) ||
                NULL == (hdcMem = CreateCompatibleDC(hDC)))
            {
                State = SET_BIT(State, 0, TRUE);
                MessageBox(hDlg, "Failed to initialize.", "Error", MB_ICONEXCLAMATION);
                return (INT_PTR)FALSE;
            }

            if (hBitmap)
                DeleteObject(hBitmap);

            // 임시로 비트맵 공간을 만들어서 미리보기에 표시할 사진을 그린 다음 WM_PAINT 메시지 요청될 때마다 표시하도록 한다.
            // Create a temporary bitmap space, draw the preview image onto it, and update the display whenever a WM_PAINT message is requested.
            if (NULL == (hBitmap = CreateCompatibleBitmap(hDC, 8192, 8192)))
            {
                State = SET_BIT(State, 0, TRUE);
                DeleteDC(hdcMem);
                MessageBox(hDlg, "Failed to initialize.", "Error", MB_ICONEXCLAMATION);
                return (INT_PTR)FALSE;
            }

            SelectObject(hdcMem, hBitmap);

            // WorkThread에 의해 캡처된 이미지를 불러와 HDC에 그린다.
            // Load the image captured by the WorkThread and render it onto the HDC.
            ListFilesRecursively(hdcMem, searchPath, &BitmapSize);

            // 사용이 끝난 이미지는 삭제한다.
            // Erase unused images upon completion.
            RemoveDirectory(searchPath);

            // 상위 폴더도 삭제한다.
            // Remove the parent directory.
            if (NULL != (ptr = strrchr(searchPath, '\\')))
            {
                *ptr = 0;
                RemoveDirectory(searchPath);
            }

            RECT rc, rc2;

            GetClientRect(hWnd, &rc2);
            GetWindowRect(hWnd, &rc);

            ++BitmapSize.x;
            ++BitmapSize.y;

            rc.right -= rc.left;
            rc.bottom -= rc.top;

            POINT xy = {
                BitmapSize.x * SAVETILESIZE + rc.right - rc2.right,
                BitmapSize.y * SAVETILESIZE + rc.bottom - rc2.bottom
            };

            SetWindowPos(hWnd, 
                NULL,
                rc.left + rc.right / 2 - xy.x / 2,
                rc.top + rc.bottom / 2 - xy.y / 2,
                xy.x,
                xy.y,
                SWP_NOACTIVATE | SWP_NOZORDER);

            if (FALSE == GET_BIT(State, 1))
            {
                GetWindowRect(hRmhWnd, &rc2);

                SetWindowPos(hRmhWnd,
                    NULL,
                    rc.left + rc.right / 2 - xy.x / 2 + xy.x,
                    rc.top + rc.bottom / 2 - (rc2.bottom - rc2.top) / 2,
                    0, 0,
                    SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);

                ShowWindow(hRmhWnd, SW_SHOW);
            }

            DeleteDC(hdcMem);

            SetTimer(hWnd, 1, 1, NULL);

            State = SET_BIT(State, 0, TRUE);
            State = SET_BIT(State, 1, TRUE);
            EnableAllControls(hRmhWnd, IDC_AAA_START_CTRL, IDC_ZZZ_END_CTRL, TRUE);

            return (INT_PTR)TRUE;
        }
        case WM_TIMER:
        {
            KillTimer(hDlg, 1);
            InvalidateRgn(hWnd, NULL, TRUE);
            break;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hDlg, &ps);

            HDC hdcMem = CreateCompatibleDC(hdc);
            SelectObject(hdcMem, hBitmap);

            BitBlt(hdc, 0, 0, 
                BitmapSize.x * SAVETILESIZE, 
                BitmapSize.y * SAVETILESIZE,
                hdcMem, 0, 0, SRCCOPY);

            DeleteDC(hdcMem);
            EndPaint(hDlg, &ps);

            break;
        }
        case WM_LBUTTONDOWN:
        case WM_CLOSE:
        case WM_DESTROY:
        {
            if (hBitmap)
                DeleteObject(hBitmap);

            if (FALSE == State)
                return (INT_PTR)FALSE;

            hWnd = NULL;
            hBitmap = NULL;
            memset(&BitmapSize, 0, sizeof(POINT));
            EndDialog(hRmhWnd, 0);
            EndDialog(hDlg, 0);
            return (INT_PTR)TRUE;
        }
    }

    return (INT_PTR)FALSE;
}

void
RemoteMainfrm(
    HWND hDlg,
    HWND hWnd,
    int nIDDlgitem,
    int nIdDlgitem2,
    int offset
)
{
    if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO_MOVE))
    {
        float a = GetDlgItemFloat(hWnd, nIDDlgitem);
        SetDlgItemFloat(hWnd, nIDDlgitem, a + (float)offset);

        a = GetDlgItemFloat(hWnd, nIdDlgitem2);
        SetDlgItemFloat(hWnd, nIdDlgitem2, a + (float)offset);
    }
    else if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO_SIZE))
    {
        float a = GetDlgItemFloat(hWnd, nIdDlgitem2);
        float b = GetDlgItemFloat(hWnd, IDC_EDIT_Z2);

        SetDlgItemFloat(hWnd, nIdDlgitem2, a + (float)((offset < 0 ? 1 : -1) * SAVETILESIZE * ATOMIC_FN * b));
    }

    PreviewDialogProc(0, WM_RELOAD_PREVIEW, 0, 0);
}

INT_PTR CALLBACK
RemoconDialogProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    static HWND hMainWnd;
    static int val;
    const int moveSize = 5000;

    switch (message)
    {
    case WM_INITDIALOG:
    {
        hMainWnd = (HWND)DlgProc(0, WM_GET_DLGHWND, 0, 0);
        SendDlgItemMessage(hDlg, IDC_SLIDER_STEPS, TBM_SETRANGE, TRUE, MAKELONG(1, 20));
        val = 1;
        SetDlgItemInt(hDlg, IDC_STATIC_STEPS, val * moveSize, FALSE);
        SendDlgItemMessage(hDlg, IDC_RADIO_MOVE, BM_CLICK, 0, 0);
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDC_BUTTON_LEFT:
        {
            RemoteMainfrm(hDlg, hMainWnd, IDC_EDIT_X2, IDC_EDIT_X3, val * moveSize);
            return (INT_PTR)TRUE;
        }
        case IDC_BUTTON_RIGHT:
        {
            RemoteMainfrm(hDlg, hMainWnd, IDC_EDIT_X2, IDC_EDIT_X3, -val * moveSize);
            return (INT_PTR)TRUE;
        }
        case IDC_BUTTON_UP:
        {
            RemoteMainfrm(hDlg, hMainWnd, IDC_EDIT_Y2, IDC_EDIT_Y3, val * moveSize);
            return (INT_PTR)TRUE;
        }
        case IDC_BUTTON_DOWN:
        {
            RemoteMainfrm(hDlg, hMainWnd, IDC_EDIT_Y2, IDC_EDIT_Y3, -val * moveSize);
            return (INT_PTR)TRUE;
        }
        case IDC_BUTTON_TOP_LEFT:
        {
            RemoteMainfrm(hDlg, hMainWnd, IDC_EDIT_X2, IDC_EDIT_X3, val * moveSize);
            RemoteMainfrm(hDlg, hMainWnd, IDC_EDIT_Y2, IDC_EDIT_Y3, val * moveSize);
            return (INT_PTR)TRUE;
        }
        case IDC_BUTTON_TOP_RIGHT:
        {
            RemoteMainfrm(hDlg, hMainWnd, IDC_EDIT_X2, IDC_EDIT_X3, -val * moveSize);
            RemoteMainfrm(hDlg, hMainWnd, IDC_EDIT_Y2, IDC_EDIT_Y3, val * moveSize);
            return (INT_PTR)TRUE;
        }
        case IDC_BUTTON_BOTTOM_LEFT:
        {
            RemoteMainfrm(hDlg, hMainWnd, IDC_EDIT_X2, IDC_EDIT_X3, val * moveSize);
            RemoteMainfrm(hDlg, hMainWnd, IDC_EDIT_Y2, IDC_EDIT_Y3, -val * moveSize);
            return (INT_PTR)TRUE;
        }
        case IDC_BUTTON_BOTTOM_RIGHT:
        {
            RemoteMainfrm(hDlg, hMainWnd, IDC_EDIT_X2, IDC_EDIT_X3, -val * moveSize);
            RemoteMainfrm(hDlg, hMainWnd, IDC_EDIT_Y2, IDC_EDIT_Y3, -val * moveSize);
            return (INT_PTR)TRUE;
        }
        }
        break;
    }
    case WM_HSCROLL:
    {
        val = (int)SendDlgItemMessage(hDlg, IDC_SLIDER_STEPS, TBM_GETPOS, 0, 0);
        SetDlgItemInt(hDlg, IDC_STATIC_STEPS, val * moveSize, FALSE);

        break;
    }
    case WM_CLOSE:
    case WM_DESTROY:
    {
        EndDialog(hDlg, 0);
        return (INT_PTR)TRUE;
    }
    }

    return (INT_PTR)FALSE;
}

int
OpenPreviewProc(
    HINSTANCE hInst,
    HWND hDlg,
    int val
)
{
    hInstance = hInst;
    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_PREVIEW), hDlg, PreviewDialogProc, val);
    return 0;
}

void
finishCapture(
)
{
    PreviewDialogProc(0, WM_FIN_PREVIEW, 0, 0);
}

int
ListFilesRecursively(
    HDC hdc,
    const char* basePath,
    POINT* maxSize
)
{
    char searchPath[MAX_PATH];
    WIN32_FIND_DATA findData;
    HANDLE hFind;

    sprintf_s(searchPath, sizeof(searchPath), "%s\\*", basePath);
    hFind = FindFirstFile(searchPath, &findData);

    if (hFind == INVALID_HANDLE_VALUE) 
        return 0;

    do
    {
        if (findData.cFileName[0] != '.' &&
            (findData.cFileName[0] != '.' || 
            findData.cFileName[1] != '.'))
        {
            sprintf_s(searchPath, sizeof(searchPath), "%s\\%s", basePath, findData.cFileName);

            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                ListFilesRecursively(hdc, searchPath, maxSize);
                RemoveDirectory(searchPath);
                continue;
            }

            char *x_ptr, *y_ptr;

            if (NULL != (y_ptr = strrchr(searchPath, '\\')))
            {
                *y_ptr = 0;

                if (NULL != (x_ptr = strrchr(searchPath, '\\')))
                {
                    *y_ptr = '\\';

                    int x = -1, y = -1;

                    sscanf_s(x_ptr, "\\%d\\%d.", &x, &y);

                    if (maxSize->x < x)
                        maxSize->x = x;

                    if (maxSize->y < y)
                        maxSize->y = y;

                    if (x >= 0 && y >= 0)
                        DrawImagehDC(hdc, searchPath, x * SAVETILESIZE, y * SAVETILESIZE);

                    DeleteFile(searchPath);
                }
            }
        }
    } while (FindNextFile(hFind, &findData));

    FindClose(hFind);

    return 1;
}
