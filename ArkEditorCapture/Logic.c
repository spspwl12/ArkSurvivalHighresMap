#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include <process.h>

#include "Logic.h"
#include "Process.h"
#include "Dialog.h"
#include "resource.h"
#include "Memory.h"
#include "InjectDll.h"
#include "Pipe.h"
#include "Image.h"
#include "SaveText.h"
#include "../ArkShot/Declare.h"

static CParam cParam;

int 
SetMessage(
    char mode, 
    int size, 
    char* buf
);

int 
RequestMessage(
    char mode, 
    int size, 
    char* buf
);

void
EnableValidControls(
    HWND hDlg
);

UINT WINAPI 
WorkThread(
    LPVOID lpParam
);

INT_PTR CALLBACK
QualityDialogProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
);

int 
LoadArkEditor(
    void* hDlg,
    unsigned long pID,
    char* dllPath
)
{
    char buf[MAX_PATH];
#ifdef  _DEBUG
    const char* DLLName = "ArkShotd.dll";
#else
    const char* DLLName = "ArkShot.dll";
#endif

    cParam.hDlg = (HWND)hDlg;

    DWORD attributes = GetFileAttributes(dllPath);

    if (INVALID_FILE_ATTRIBUTES == attributes ||
        0 != (attributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        int bfd = FALSE;

        if (GetModuleFileName(NULL, buf, MAX_PATH))
        {
            char* ptr = strrchr(buf, '\\');

            if (ptr)
            {
                *ptr = 0;
                const size_t namelen = strlen(buf) + strlen(DLLName) + 1;

                if (namelen <= MAX_PATH)
                {
                    sprintf_s(buf, sizeof(buf), "%s\\%s", buf, DLLName);
                    attributes = GetFileAttributes(buf);

                    if (INVALID_FILE_ATTRIBUTES != attributes ||
                        0 == (attributes & FILE_ATTRIBUTE_DIRECTORY))
                    {
                        dllPath = buf;
                        SetDlgItemText(cParam.hDlg, IDC_EDIT_DLPATH, buf);
                        bfd = TRUE;
                    }
                }
            }
        }

        if (FALSE == bfd)
        {
            MessageBox(hDlg, "Invalid Dll Path.", "Error", MB_ICONEXCLAMATION);
            return FALSE;
        }
    }

    if (NULL == (cParam.hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID)))
    {
        MessageBox(hDlg, "Failed to open the process.", "Error", MB_ICONEXCLAMATION);
        return FALSE;
    }

    if (FALSE == InjectDll(cParam.hProcess, dllPath))
    {
        MessageBox(hDlg, "Failed to inject the DLL.", "Error", MB_ICONEXCLAMATION);
        CloseHandle(cParam.hProcess);
        return FALSE;
    }

    if (FALSE == StartPipeComm())
        return FALSE;

    return TRUE;
}

void
LoadValue(
)
{
    if (NULL == cParam.hDlg)
        return;

    char buf[20];
    float fval;
    long lval;

    if (RequestMessage(MODE_GET_XYZ, 12, buf))
    {
        memcpy(&fval, buf, sizeof(float));
        SetDlgItemFloat(cParam.hDlg, IDC_EDIT_X1, fval);

        memcpy(&fval, buf + 4, sizeof(float));
        SetDlgItemFloat(cParam.hDlg, IDC_EDIT_Y1, fval);
    }

    if (RequestMessage(MODE_GET_ZOOM, 4, buf))
    {
        memcpy(&fval, buf, sizeof(float));
        SetDlgItemFloat(cParam.hDlg, IDC_EDIT_Z1, fval);
    }

    if (RequestMessage(MODE_GET_WH, 8, buf))
    {
        memcpy(&lval, buf, sizeof(long));
        SetDlgItemInt(cParam.hDlg, IDC_WHI_W, lval, TRUE);

        memcpy(&lval, buf + 4, sizeof(long));
        SetDlgItemInt(cParam.hDlg, IDC_WHI_H, lval, TRUE);
    }
}

int
OpenProcessProc(
    HWND hDlg
)
{
    char buf[255];

    GetDlgItemText(hDlg, IDC_EDIT_PN, buf, sizeof(buf));
    const unsigned long pID = GetpIDByName(buf);

    if (0 == pID)
    {
        MessageBox(hDlg, "Process not found.", "Error", MB_ICONEXCLAMATION);
        return FALSE;
    }

    GetDlgItemText(hDlg, IDC_EDIT_DLPATH, buf, sizeof(buf));

    if (FALSE == LoadArkEditor(hDlg, pID, buf))
        return FALSE;

    EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_OPEN), FALSE);
    SetTimer(hDlg, 1, 100, NULL);

    return TRUE;
}

void 
CloseArkEditor(
)
{
    CloseHandle(cParam.hProcess);
    StopPipeComm();
}

int 
StartCapture(
    int quality
)
{
    if (0 < cParam.Status)
    {
        StopCapture();
        return TRUE;
    }

    cParam.Status = 1;
    cParam.quality = quality;

    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, WorkThread, NULL, 0, NULL);

    if (NULL != hThread)
    {
        CloseHandle(hThread);
        return TRUE;
    }

    return FALSE;
}

void
SetButtonAction(
    int mode
)
{
    switch (mode)
    {
        case 0:
        {
            float Zoom = GetDlgItemFloat(cParam.hDlg, IDC_EDIT_Z2);
            SetMessage(MODE_SET_ZOOM, 4, (char*)&Zoom);
            break;
        }
        case 1:
        {
            Vec2D Coord = {
                GetDlgItemFloat(cParam.hDlg, IDC_EDIT_X1),
                GetDlgItemFloat(cParam.hDlg, IDC_EDIT_Y1)
            };

            double StartZ = GetDlgItemFloat(cParam.hDlg, IDC_EDIT_Z1);

            StartZ *= ATOMIC_FN * 256;

            SetDlgItemFloat(cParam.hDlg, IDC_EDIT_X2, (float)-StartZ + Coord.x);
            SetDlgItemFloat(cParam.hDlg, IDC_EDIT_Y2, (float)-StartZ + Coord.y);

            SetDlgItemFloat(cParam.hDlg, IDC_EDIT_X3, (float)StartZ + Coord.x);
            SetDlgItemFloat(cParam.hDlg, IDC_EDIT_Y3, (float)StartZ + Coord.y);
            break;
        }
        case 2:
        {
            Vec2D Coord_To = {
                GetDlgItemFloat(cParam.hDlg, IDC_EDIT_X2),
                GetDlgItemFloat(cParam.hDlg, IDC_EDIT_Y2)
            };

            Vec2D Coord_From = {
                GetDlgItemFloat(cParam.hDlg, IDC_EDIT_X3),
                GetDlgItemFloat(cParam.hDlg, IDC_EDIT_Y3)
            };

            Coord_To.x = (Coord_To.x + Coord_From.x) / 2;
            Coord_To.y = (Coord_To.y + Coord_From.y) / 2;

            char buf[10] = { 0 };
            float fval;

            fval = (float)Coord_To.x;
            memcpy(buf + 0, &fval, 4);

            fval = (float)Coord_To.y;
            memcpy(buf + 4, &fval, 4);

            SetMessage(MODE_SET_XY, 8, buf);
            break;
        }
        case 3:
        {
            char buf[10] = { 0 };
            SetMessage(MODE_SET_XY, 8, buf);
            break;
        }
    }
}

int 
SetMessage(
    char mode, 
    int size,
    char* buf
)
{
    if (size > PIPE_BUF_SIZE)
        return FALSE;

    char _buf[PIPE_BUF_SIZE] = { 0 };

    _buf[0] = mode;

    if (size > 0)
        memcpy(_buf + 1, buf, sizeof(char) * size);

    if (FALSE == SendPipeMessage(1 + size, _buf))
        return FALSE;

    if (FALSE == RecvPipeMessage(1, _buf) || _buf[0] != mode)
        return FALSE;

    return TRUE;
}

int 
RequestMessage(
    char mode, 
    int size, 
    char* buf
)
{
    if (size > PIPE_BUF_SIZE)
        return FALSE;

    if (FALSE == SendPipeMessage(1, &mode))
        return FALSE;

    char _buf[PIPE_BUF_SIZE] = { 0 };

    if (FALSE == RecvPipeMessage(1 + size, _buf))
        return FALSE;

    if (mode != (unsigned char)_buf[0])
        return FALSE;

    memcpy(buf, _buf + 1, sizeof(char) * size);

    return TRUE;
}

UINT WINAPI 
WorkThread(
    LPVOID lpParam
)
{
    HANDLE hProcess = (HANDLE)cParam.hProcess;
    HWND hDlg = (HWND)cParam.hDlg;

    char UE4Path[MAX_PATH] = { 0 };
    char BitmapPath[MAX_PATH] = { 0 };
    char SavePath[MAX_PATH] = { 0 };
    char Buf[MAX_PATH];

    if (FALSE == RequestMessage(MODE_GET_PATH, MAX_PATH, UE4Path))
    {
        MessageBox(hDlg, "Failed to get the ARK DevKit path.", "Error", MB_ICONEXCLAMATION);
        goto EXIT_THREAD;
    }

    char* filename = strrchr(UE4Path, '\\');

    if (NULL == filename)
    {
        MessageBox(hDlg, "Invalid ARK DevKit path.", "Error", MB_ICONEXCLAMATION);
        goto EXIT_THREAD;
    }

    *(filename + 1) = 0;

    strcpy_s(BitmapPath, sizeof(UE4Path), UE4Path);

    if (strlen(BitmapPath) + 13 >= MAX_PATH) // 00001.bmp 
    {
        MessageBox(hDlg, "The file path in ARK DevKit is too long. Please check it.", "Error", MB_ICONEXCLAMATION);
        goto EXIT_THREAD;
    }

    strcat_s(BitmapPath, sizeof(BitmapPath), "00001.bmp");

    GetDlgItemText(hDlg, IDC_EDIT_SVPATH, SavePath, sizeof(SavePath));

    if (strlen(SavePath) + 13 >= MAX_PATH) // \AActors.txt
    {
        MessageBox(hDlg, "The save path is too long. Please check it.", "Error", MB_ICONEXCLAMATION);
        goto EXIT_THREAD;
    }

    DWORD attributes = GetFileAttributes(SavePath);

    if (INVALID_FILE_ATTRIBUTES == attributes ||
        0 == (attributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        MessageBox(hDlg, "Invalid Save Path.", "Error", MB_ICONEXCLAMATION);
        goto EXIT_THREAD;
    }

    Vec2D Coord_To = {
        GetDlgItemFloat(hDlg, IDC_EDIT_X2),
        GetDlgItemFloat(hDlg, IDC_EDIT_Y2)
    };

    Vec2D Coord_From = {
        GetDlgItemFloat(hDlg, IDC_EDIT_X3),
        GetDlgItemFloat(hDlg, IDC_EDIT_Y3)
    };

    if (Coord_To.x > Coord_From.x)
    {
        MessageBox(hDlg, "xto cannot be greater than xfrom.", "Error", MB_ICONEXCLAMATION);
        goto EXIT_THREAD;
    }

    if (Coord_To.y > Coord_From.y)
    {
        MessageBox(hDlg, "yto cannot be greater than yfrom.", "Error", MB_ICONEXCLAMATION);
        goto EXIT_THREAD;
    }

    long Z_to = GetDlgItemInt(hDlg, IDC_EDIT_Z3, NULL, TRUE);
    long Z_from = GetDlgItemInt(hDlg, IDC_EDIT_Z4, NULL, TRUE);

    if (Z_to > Z_from)
    {
        MessageBox(hDlg, "zto cannot be greater than zfrom.", "Error", MB_ICONEXCLAMATION);
        goto EXIT_THREAD;
    }

    float StartZ = GetDlgItemFloat(hDlg, IDC_EDIT_Z2);

    long VpWidth;
    long VpHeight;

    char Extension[10] = { 0 };
    int ExtensionIndex = 0;
    int TileSize = 0;
    int WaitCount = 0;

    int index = (int)SendDlgItemMessage(hDlg, IDC_COMBO_TILESZ, CB_GETCURSEL, 0, 0);
    SendDlgItemMessage(hDlg, IDC_COMBO_TILESZ, CB_GETLBTEXT, index, (LPARAM)Extension);

    TileSize = atoi(Extension);

    index = (int)SendDlgItemMessage(hDlg, IDC_COMBO_EXTIMG, CB_GETCURSEL, 0, 0);
    SendDlgItemMessage(hDlg, IDC_COMBO_EXTIMG, CB_GETLBTEXT, index, (LPARAM)Extension);

    if (0 == strcmp(Extension, "BMP"))
        ExtensionIndex = IMAGE_TYPE_BMP;
    else if (0 == strcmp(Extension, "JPG"))
        ExtensionIndex = IMAGE_TYPE_JPG;
    else if (0 == strcmp(Extension, "GIF"))
        ExtensionIndex = IMAGE_TYPE_GIF;
    else if (0 == strcmp(Extension, "PNG"))
        ExtensionIndex = IMAGE_TYPE_PNG;
    else if (0 == strcmp(Extension, "WEBP"))
        ExtensionIndex = IMAGE_TYPE_WEBP;
    else
    {
        MessageBox(hDlg, "unknown extension.", "Error", MB_ICONEXCLAMATION);
        goto EXIT_THREAD;
    }

    if (TileSize < 256 || TileSize > 16384)
    {
        MessageBox(hDlg, "The tile size is out of range.", "Error", MB_ICONEXCLAMATION);
        goto EXIT_THREAD;
    }

    {
        POINT xy = { 0 };
        int cnt = 10;

        for (;;)
        {
            while (!RequestMessage(MODE_GET_WH, 8, Buf))
            {
                if (++WaitCount > 10)
                {
                    MessageBox(hDlg, "Failed to load the viewport rendering size in ARK DevKit.", "Error", MB_ICONEXCLAMATION);
                    goto EXIT_THREAD;
                }

                Sleep(100);
            }

            memcpy(&VpWidth, Buf, sizeof(long));
            memcpy(&VpHeight, Buf + 4, sizeof(long));

            if (xy.x != VpWidth ||
                xy.y != VpHeight)
            {
                if (cnt > 0)
                    --cnt;
            }
            else
                ++cnt;

            if (cnt >= 100)
            {
                MessageBox(hDlg, "Failed to automatically adjust the viewport rendering size in ARK DevKit.", "Error", MB_ICONEXCLAMATION);
                goto EXIT_THREAD;
            }

            memcpy(&xy, Buf, sizeof(POINT));

            // auto sizing window
            POINT windowSize = { 256, 256 };

            if (VpWidth == windowSize.x && VpHeight == windowSize.y)
                break;

            unsigned long pID = GetProcessId(hProcess);
            HWND unrealhWnd = FindWindowsByPID(pID);

            if (NULL == unrealhWnd)
            {
                MessageBox(hDlg, "Failed to get the window handle in ARK DevKit.", "Error", MB_ICONEXCLAMATION);
                goto EXIT_THREAD;
            }

            RECT rc;
            POINT offset = windowSize;

            GetClientRect(unrealhWnd, &rc);

            offset.x = (windowSize.x - VpWidth);
            offset.y = (windowSize.y - VpHeight);

            offset.x = rc.right - rc.left + offset.x;
            offset.y = rc.bottom - rc.top + offset.y;

            if (offset.x < 50)
                offset.x = 50;

            if (offset.y < 50)
                offset.y = 50;

            SetWindowPos(unrealhWnd, NULL, 0, 0, offset.x, offset.y, SWP_NOMOVE | SWP_NOZORDER);

            Sleep(50 * (cnt + 1));
        }
    }

    int oTileSz = 256;

    const Vec2D offset = {
        (VpWidth * StartZ * ATOMIC_FN) / 2,
        (VpHeight * StartZ * ATOMIC_FN) / 2
    };

    const POINT TileMul = {
        TileSize / VpWidth,
        TileSize / VpWidth
    };

    const Vec2D tileCalc = {
        (TileMul.x - 1) * offset.x,
        (TileMul.y - 1) * offset.y
    };

    {
        Vec2D EndCoord = Coord_From;

        if (EndCoord.x == 0 || EndCoord.x < Coord_To.x)
            EndCoord.x = Coord_To.x + 1;

        if (EndCoord.y == 0 || EndCoord.y < Coord_To.y)
            EndCoord.y = Coord_To.y + 1;

        const float extTile = oTileSz * StartZ * ATOMIC_FN;

        Vec2D CurrentCoord = Coord_To;

        for (; CurrentCoord.x <= EndCoord.x; CurrentCoord.x += extTile)
        {
            if ((long)CurrentCoord.x == (long)EndCoord.x)
                break;
        }

        for (; CurrentCoord.y <= EndCoord.y; CurrentCoord.y += extTile)
        {
            if ((long)CurrentCoord.y == (long)EndCoord.y)
                break;
        }

        if (CurrentCoord.x > EndCoord.x)
        {
            SetDlgItemFloat(hDlg, IDC_EDIT_X3, (float)CurrentCoord.x);
            Coord_From.x = CurrentCoord.x;
        }

        if (CurrentCoord.y > EndCoord.y)
        {
            SetDlgItemFloat(hDlg, IDC_EDIT_Y3, (float)CurrentCoord.y);
            Coord_From.y = CurrentCoord.y;
        }
    }

    char z_string[MAX_PATH];
    char zx_string[MAX_PATH];
    char zxy_string[MAX_PATH];

    DeleteFile(BitmapPath);

    const float resValue = (((float)TileSize / VpWidth) + ((float)TileSize / VpHeight)) / 2;
    float Zoom = StartZ;

    cParam.Status = 2;

    EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_START), TRUE);
    SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_START), "Stop");

    long long timeval = GetTickCount64();

    sprintf_s(z_string, sizeof(z_string), "%s\\info_%lld.txt", SavePath, timeval);
    sprintf_s(zx_string, sizeof(zx_string),
        "Map Info: \n\n"
        "left: \t%g\tright: \t%g\n"
        "top: \t%g\tbottom: \t%g\n\n"
        "Zoom Min / Max: \t%d / %d\n"
        "Zoom Value: \t%.2f\n\n"
        "Width: \t%d\nHeight: \t%d\n\n"
        "Image Codec: \t%s\nQuality: \t%d",
        Coord_To.x, Coord_From.x,
        Coord_To.y, Coord_From.y,
        Z_to, Z_from,
        StartZ, 
        VpWidth, VpHeight,
        Extension, cParam.quality);

    saveTextToFile(z_string, zx_string, (int)strlen(zx_string));

    {
        char a = 0;

        strcpy_s(Buf, sizeof(Buf), SavePath);
        strcat_s(Buf, sizeof(Buf), "\\AActors.txt");

        DeleteFile(Buf);

        if (TRUE == SetMessage(MODE_GET_ACTORS, 1, &a))
        {
            char Buf2[MAX_PATH];

            strcpy_s(Buf2, sizeof(Buf2), UE4Path);
            strcat_s(Buf2, sizeof(Buf2), "AActors.txt");

            MoveFile(Buf2, Buf);
        }
    }

    for (long ZoomLvl = 0; ZoomLvl <= Z_from; ++ZoomLvl, Zoom /= 2)
    {
        if (ZoomLvl < Z_to)
            continue;

        SetDlgItemInt(hDlg, IDC_WHI_I, ZoomLvl, TRUE);

        const float ZoomTile = Zoom * ATOMIC_FN;

        const Vec2D Increase = {
            VpWidth * ZoomTile,
            VpHeight * ZoomTile
        };

        const Vec2D resInc = {
            VpWidth * resValue * ZoomTile,
            VpHeight * resValue * ZoomTile
        };

        const float ZoomDiv = Zoom / StartZ;

        const Vec2D zoomCalc = {
            offset.x * ZoomDiv + tileCalc.x * ZoomDiv,
            offset.y * ZoomDiv + tileCalc.y * ZoomDiv
        };

        const Vec2D StartCoord = {
            Coord_To.x + zoomCalc.x,
            Coord_To.y + zoomCalc.y
        };

        const Vec2D EndCoord = {
            Coord_From.x + zoomCalc.x,
            Coord_From.y + zoomCalc.y
        };

        sprintf_s(z_string, sizeof(z_string), "%s\\%d", SavePath, ZoomLvl);
        CreateDirectory(z_string, NULL);

        long LimitX = (long)((Coord_From.x - Coord_To.x) / resInc.x + 0.5) - 1;
        long LimitY = (long)((Coord_From.y - Coord_To.y) / resInc.y + 0.5) - 1;

        if (LimitX < 0)
            LimitX = 0;

        if (LimitY < 0)
            LimitY = 0;

        for (long IndexY = 0; IndexY <= LimitY; ++IndexY)
        {
            for (long IndexX = 0; IndexX <= LimitX; ++IndexX)
            {
                if (2 != cParam.Status)
                    goto EXIT_THREAD;

                const long x = TileMul.x * IndexX;
                const long y = TileMul.y * IndexY;

                sprintf_s(zx_string, sizeof(zx_string), "%s\\%d", z_string, x);
                sprintf_s(zxy_string, sizeof(zxy_string), "%s\\%d.%s", zx_string, y, Extension);

                if (GetFileAttributes(zxy_string) == ((DWORD)-1))
                {
                    float fval;

                    const Vec2D rCoord = {
                        StartCoord.x + Increase.x * x,
                        StartCoord.y + Increase.y * y
                    };

                    fval = (float)rCoord.x;
                    memcpy(Buf + 0, &fval, 4);
                    fval = (float)rCoord.y;
                    memcpy(Buf + 4, &fval, 4);
                    fval = (float)0;
                    memcpy(Buf + 8, &fval, 4);
                    fval = (float)Zoom;
                    memcpy(Buf + 12, &fval, 4);
                    fval = (float)resValue;
                    memcpy(Buf + 16, &fval, 4);

                    // synchronize
                    for (int i = 0; i <= 3000; ++i)
                    {
                        SetMessage(MODE_SET_XYZZ_CAPTURE, 20, Buf);

                        if (GetFileAttributes(BitmapPath) != ((DWORD)-1))
                            break;

                        Sleep(10);
                    }

                    if (0 == IndexY)
                        CreateDirectory(zx_string, NULL);

                    WaitCount = 0;

                    const long cutX = (long)((EndCoord.x - rCoord.x) / Increase.x + 0.5) - 1;
                    const long cutY = (long)((EndCoord.y - rCoord.y) / Increase.y + 0.5) - 1;

                    while (!SaveImageParts(ExtensionIndex,
                        cParam.quality,
                        oTileSz, oTileSz,
                        x, y,
                        cutX, cutY,
                        Extension, BitmapPath, z_string))
                    {
                        if (++WaitCount > 10)
                        {
                            MessageBox(hDlg, "failed to convert image.", "Error", MB_ICONEXCLAMATION);
                            goto EXIT_THREAD;
                        }

                        Sleep(100);
                    }

                    DeleteFile(BitmapPath);
                    LoadValue();
                }

            }
        }
    }

EXIT_THREAD:

    cParam.Status = 0;
    EnableValidControls(hDlg);

    return 0;
}

void
StopCapture(
)
{
    cParam.Status = 1;
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
            MAKEINTRESOURCE(IDD_QUALITY), hDlg, QualityDialogProc, val);
    }

    return -1;
}