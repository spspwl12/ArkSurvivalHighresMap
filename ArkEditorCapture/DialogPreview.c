#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "resource.h"
#include "Logic.h"
#include "Image.h"

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
    static int bLoaded;

    switch (message)
    {
        case WM_INITDIALOG:
        {
            int quality = (int)lParam;
            hWnd = hDlg;

            bLoaded = FALSE;
            StartCapture(quality, LOGIC_MODE_PREVIEW, finishCapture);

            return (INT_PTR)TRUE;
        }
        case WM_USER + 0x10000:
        {
            // SendMessage 또는 PostMessage API로 실행되는 것을 막는다.
            // Block execution triggered by the SendMessage or PostMessage API.
            if (NULL != hDlg)
                return (INT_PTR)FALSE;

            bLoaded = TRUE;

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
                MessageBox(hDlg, "Failed to initialize.", "Error", MB_ICONEXCLAMATION);
                return (INT_PTR)FALSE;
            }

            // 임시로 비트맵 공간을 만들어서 미리보기에 표시할 사진을 그린 다음 WM_PAINT 메시지 요청될 때마다 표시하도록 한다.
            // Create a temporary bitmap space, draw the preview image onto it, and update the display whenever a WM_PAINT message is requested.
            if (NULL == (hBitmap = CreateCompatibleBitmap(hDC, 8192, 8192)))
            {
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
                HWND_TOPMOST,
                rc.left + rc.right / 2 - xy.x / 2,
                rc.top + rc.bottom / 2 - xy.y / 2,
                xy.x,
                xy.y,
                SWP_NOACTIVATE);

            DeleteDC(hdcMem);

            SetTimer(hWnd, 1, 10, NULL);

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

            if (FALSE == bLoaded)
                return (INT_PTR)FALSE;

            hBitmap = NULL;
            memset(&BitmapSize, 0, sizeof(POINT));
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
    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_PREVIEW), hDlg, PreviewDialogProc, val);
    return 0;
}

void
finishCapture(
)
{
    PreviewDialogProc(0, WM_USER + 0x10000, 0, 0);
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