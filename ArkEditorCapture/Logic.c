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
    void* buf
);

int 
RequestMessage(
    char mode, 
    int size, 
    void* buf
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

HWND
GetUnrealHandle(
);

Vec2D
GetEndCoord(
    Vec2D CoordTo,
    Vec2D CoordFrom,
    const float multiplier
);

int
ResizeToTileSize(
    long* width,
    long* height
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

    char buf[30];
    FVector vec;
    FVectorD vecd;
    float fval;
    long lval;

    switch (cParam.UEVersion)
    {
        case 4:
            if (RequestMessage(MODE_GET_XYZ, sizeof(FVector), &vec))
            {
                SetDlgItemFloat(cParam.hDlg, IDC_EDIT_X1, vec.x);
                SetDlgItemFloat(cParam.hDlg, IDC_EDIT_Y1, vec.y);
            }
            break;
        case 5:
            if (RequestMessage(MODE_GET_XYZ, sizeof(FVectorD), &vecd))
            {
                SetDlgItemFloat(cParam.hDlg, IDC_EDIT_X1, (float)vecd.x);
                SetDlgItemFloat(cParam.hDlg, IDC_EDIT_Y1, (float)vecd.y);
            }
            break;
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

    const char* UE4prefix = "UE4Editor";
    const char* UE5prefix = "ShooterGameEditor";
    const char* UE5prefix2 = "UnrealEditor";

    GetDlgItemText(hDlg, IDC_EDIT_PN, buf, sizeof(buf));

    if (0 == strncmp(UE4prefix, buf, sizeof(UE4prefix)))
        cParam.UEVersion = 4;
    else if (0 == strncmp(UE5prefix, buf, sizeof(UE5prefix)))
        cParam.UEVersion = 5;
    else if (0 == strncmp(UE5prefix2, buf, sizeof(UE5prefix2)))
        cParam.UEVersion = 5;
    else 
    {
        MessageBox(hDlg, "Unknown process name.", "Error", MB_ICONEXCLAMATION);
        return FALSE;
    }

    const unsigned long pID = GetpIDByName(buf);

    if (0 == pID)
    {
        MessageBox(hDlg, "Process not found.", "Error", MB_ICONEXCLAMATION);
        return FALSE;
    }

    GetDlgItemText(hDlg, IDC_EDIT_DLPATH, buf, sizeof(buf));

    if (FALSE == LoadArkEditor(hDlg, pID, buf))
        return FALSE;

    const char* pptr = UE4prefix;

    if (cParam.UEVersion == 4)
        pptr = UE4prefix;
    else if (cParam.UEVersion == 5)
        pptr = UE5prefix;
    
    const int sz = (int)strlen(pptr);

    buf[0] = cParam.UEVersion;
    buf[1] = (char)sz;
    memcpy(&buf[2], pptr, sz);

    if (FALSE == SetMessage(MODE_SET_VER, 2 + sz, buf))
    {
        MessageBox(hDlg, "Failed to load the DLL.", "Error", MB_ICONEXCLAMATION);
        return FALSE;
    }

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
            SetMessage(MODE_SET_ZOOM, 4, &Zoom);
            break;
        }
        case 1:
        {
            if (cParam.UEVersion == 4)
            {
                FVector vec = { 0 };
                SetMessage(MODE_SET_XY, sizeof(FVector), &vec);
            }
            else if (cParam.UEVersion == 5)
            {
                FVectorD vecd = { 0 };
                SetMessage(MODE_SET_XY, sizeof(FVectorD), &vecd);
            }
            break;
        }
        case 2:
        {
            Vec2D Coord = {
                GetDlgItemFloat(cParam.hDlg, IDC_EDIT_X1),
                GetDlgItemFloat(cParam.hDlg, IDC_EDIT_Y1)
            };

            HWND unrealhWnd = GetUnrealHandle();

            if (NULL == unrealhWnd)
            {
                MessageBox(cParam.hDlg, "Failed to get the window handle in ARK DevKit.", "Error", MB_ICONEXCLAMATION);
                break;
            }

            POINT xy = { 
                GetDlgItemInt(cParam.hDlg, IDC_WHI_W, NULL, FALSE),
                GetDlgItemInt(cParam.hDlg, IDC_WHI_H, NULL, FALSE)
            };
            int retry = 0;

            while (++retry < 10 && (xy.x == 0 || xy.y == 0))
            { 
                RECT rc;

                GetWindowRect(unrealhWnd, &rc);

                SetWindowPos(unrealhWnd, NULL, 0, 0, 
                    rc.right - rc.left - 1, 
                    rc.bottom - rc.top - 1, SWP_NOMOVE | SWP_NOZORDER);

                Sleep(100);

                SetWindowPos(unrealhWnd, NULL, 0, 0,
                    rc.right - rc.left,
                    rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);

                RequestMessage(MODE_GET_WH, 8, &xy);
            }

            float Zoom = GetDlgItemFloat(cParam.hDlg, IDC_EDIT_Z1);
            Zoom *= ATOMIC_FN;

            float StartZ = GetDlgItemFloat(cParam.hDlg, IDC_EDIT_Z2);
            StartZ *= ATOMIC_FN;

            float MulZ = SAVETILESIZE * StartZ;

            long vpWhf = xy.x / 2;
            long vpHhf = xy.y / 2;

            Coord.x += (float)-vpWhf * Zoom;
            Coord.y += (float)-vpHhf * Zoom;

            Coord.x = (float)(long)((Coord.x + 500) / 1000) * 1000;
            Coord.y = (float)(long)((Coord.y + 500) / 1000) * 1000;

            SetDlgItemFloat(cParam.hDlg, IDC_EDIT_X2, Coord.x);
            SetDlgItemFloat(cParam.hDlg, IDC_EDIT_Y2, Coord.y);

            Vec2D From = Coord;

            From.x = -From.x;
            From.y = -From.y;

            From.x += ((long)From.x % (long)MulZ);
            From.y += ((long)From.y % (long)MulZ);

            From.x = (float)(long)((From.x + 500) / 1000) * 1000;
            From.y = (float)(long)((From.y + 500) / 1000) * 1000;

            const Vec2D GetCoord = GetEndCoord(
                Coord,
                From,
                MulZ);

            if (GetCoord.x > Coord.x)
                Coord.x = GetCoord.x;

            if (GetCoord.y > Coord.y)
                Coord.y = GetCoord.y;

            SetDlgItemFloat(cParam.hDlg, IDC_EDIT_X3, Coord.x);
            SetDlgItemFloat(cParam.hDlg, IDC_EDIT_Y3, Coord.y);
          
            break;
        }
    }
}

int 
SetMessage(
    char mode, 
    int size,
    void* buf
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
    void* buf
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

    char UEPath[MAX_PATH] = { 0 };
    char BitmapPath[MAX_PATH] = { 0 };
    char SavePath[MAX_PATH] = { 0 };
    char Buf[MAX_PATH];

    if (FALSE == RequestMessage(MODE_GET_PATH, MAX_PATH, UEPath))
    {
        MessageBox(hDlg, "Failed to get the ARK DevKit path.", "Error", MB_ICONEXCLAMATION);
        goto EXIT_THREAD;
    }

    char* filename = strrchr(UEPath, '\\');

    if (NULL == filename)
    {
        MessageBox(hDlg, "Invalid ARK DevKit path.", "Error", MB_ICONEXCLAMATION);
        goto EXIT_THREAD;
    }

    *(filename + 1) = 0;

    strcpy_s(BitmapPath, sizeof(UEPath), UEPath);

    if (strlen(BitmapPath) + 13 >= MAX_PATH) // 00001.bmp 
    {
        MessageBox(hDlg, "The file path in ARK DevKit is too long. Please check it.", "Error", MB_ICONEXCLAMATION);
        goto EXIT_THREAD;
    }

    if (cParam.UEVersion == 4)
        strcat_s(BitmapPath, sizeof(BitmapPath), "00001.bmp");
    else if (cParam.UEVersion == 5)
        strcat_s(BitmapPath, sizeof(BitmapPath), "00000.png");

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

    if (FALSE == ResizeToTileSize(&VpWidth, &VpHeight))
        goto EXIT_THREAD;
    
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

    const Vec2D GetCoord = GetEndCoord(
        Coord_To,
        Coord_From,
        SAVETILESIZE * StartZ * ATOMIC_FN);

    if (GetCoord.x > Coord_From.x)
    {
        SetDlgItemFloat(hDlg, IDC_EDIT_X3, (float)GetCoord.x);
        Coord_From.x = GetCoord.x;
    }

    if (GetCoord.y > Coord_From.y)
    {
        SetDlgItemFloat(hDlg, IDC_EDIT_Y3, (float)GetCoord.y);
        Coord_From.y = GetCoord.y;
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

    strcpy_s(Buf, sizeof(Buf), SavePath);
    strcat_s(Buf, sizeof(Buf), "\\AActors.txt");

    DeleteFile(Buf);

    WaitCount = 0;

    while (FALSE == SetMessage(MODE_GET_ACTORS, 0, NULL))
    {
        if (++WaitCount > 30)
            break;

        Sleep(100);
    }

    if (WaitCount <= 10)
    {
        char Buf2[MAX_PATH];

        strcpy_s(Buf2, sizeof(Buf2), UEPath);
        strcat_s(Buf2, sizeof(Buf2), "AActors.txt");

        MoveFile(Buf2, Buf);
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
                    int writeBytes = 0;

                    const Vec2D rCoord = {
                        StartCoord.x + Increase.x * x,
                        StartCoord.y + Increase.y * y
                    };

                    if (cParam.UEVersion == 4)
                    { 
                        FVector vec;

                        vec.x = rCoord.x;
                        vec.y = rCoord.y;
                        vec.z = 0;

                        memcpy(Buf + 0, &vec, sizeof(FVector));
                        writeBytes += sizeof(FVector);
                    }
                    else if (cParam.UEVersion == 5)
                    {
                        FVectorD vecd;

                        vecd.x = (float)rCoord.x;
                        vecd.y = (float)rCoord.y;
                        vecd.z = 0;

                        memcpy(Buf + 0, &vecd, sizeof(FVectorD));
                        writeBytes += sizeof(FVectorD);
                    }
                    
                    if (0 == writeBytes)
                        goto EXIT_THREAD;

                    fval = (float)Zoom;
                    memcpy(Buf + writeBytes, &fval, 4);
                    writeBytes += sizeof(float);

                    fval = (float)resValue;
                    memcpy(Buf + writeBytes, &fval, 4);
                    writeBytes += sizeof(float);

                    // synchronize
                    for (int i = 0; i <= 3000; ++i)
                    {
                        SetMessage(MODE_SET_XYZZ_CAPTURE, writeBytes, Buf);

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
                        SAVETILESIZE, SAVETILESIZE,
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

HWND
GetUnrealHandle(
)
{
    unsigned long pID = GetProcessId(cParam.hProcess);

    if (0 == pID)
        return NULL;

    return FindWindowsByPID(pID);
}

Vec2D
GetEndCoord(
    Vec2D CoordTo,
    Vec2D CoordFrom,
    const float multiplier
)
{
    if (CoordFrom.x == 0 || CoordFrom.x < CoordTo.x)
        CoordFrom.x = CoordTo.x + 1;

    if (CoordFrom.y == 0 || CoordFrom.y < CoordTo.y)
        CoordFrom.y = CoordTo.y + 1;

    Vec2D CurrentCoord = CoordTo;

    for (; CurrentCoord.x <= CoordFrom.x; CurrentCoord.x += multiplier)
    {
        if ((long)CurrentCoord.x == (long)CoordFrom.x)
            break;
    }

    for (; CurrentCoord.y <= CoordFrom.y; CurrentCoord.y += multiplier)
    {
        if ((long)CurrentCoord.y == (long)CoordFrom.y)
            break;
    }

    if (CurrentCoord.x > CoordFrom.x)
        CoordFrom.x = CurrentCoord.x;

    if (CurrentCoord.y > CoordFrom.y)
        CoordFrom.y = CurrentCoord.y;

    return CoordFrom;
}

int
ResizeToTileSize(
    long* width,
    long* height
)
{
    // auto sizing window

    POINT sizeViewport = { 0 };
    POINT prevSIze = { 0 };
    int cnt = 10;
    int scnt = 0;
    int WaitCount;

    for (;;)
    {
        WaitCount = 0;

        while (!RequestMessage(MODE_GET_WH, sizeof(POINT), &sizeViewport))
        {
            if (++WaitCount > 10)
            {
                MessageBox(cParam.hDlg, 
                    "Failed to load the viewport rendering size in ARK DevKit.", 
                    "Error", MB_ICONEXCLAMATION);
                return 0;
            }

            Sleep(100);
        }

        if (sizeViewport.x == SAVETILESIZE &&
            sizeViewport.y == SAVETILESIZE)
        {
            if (++scnt > 5)
                break;

            Sleep(50 * (cnt < 1 ? 1 : cnt));
            continue;
        }

        scnt = 0;
        HWND unrealhWnd = GetUnrealHandle();

        if (NULL == unrealhWnd)
        {
            MessageBox(cParam.hDlg,
                "Failed to get the window handle in ARK DevKit.", 
                "Error", MB_ICONEXCLAMATION);
            return FALSE;
        }

        RECT rc;
        POINT offset;
        POINT resize;

        GetWindowRect(unrealhWnd, &rc);

        offset.x = (SAVETILESIZE - sizeViewport.x);
        offset.y = (SAVETILESIZE - sizeViewport.y);

        if (offset.x / 2 >= 1)
            offset.x /= 2;

        if (offset.y / 2 >= 1)
            offset.y /= 2;

        resize.x = rc.right - rc.left + offset.x;
        resize.y = rc.bottom - rc.top + offset.y;

        if (resize.x < 50)
            resize.x = 50;

        if (resize.y < 50)
            resize.y = 50;

        SetWindowPos(unrealhWnd, NULL, 0, 0, resize.x, resize.y, SWP_NOMOVE | SWP_NOZORDER);

        if ((0 == offset.x || prevSIze.x != sizeViewport.x) &&
            (0 == offset.y || prevSIze.y != sizeViewport.y))
            --cnt;
        else
            ++cnt;

        if (cnt >= 100 || cnt <= -100)
        {
            MessageBox(cParam.hDlg,
                "Failed to automatically adjust the viewport rendering size in ARK DevKit.",
                "Error", MB_ICONEXCLAMATION);
            return FALSE;
        }

        prevSIze = sizeViewport;

        Sleep(50 * (cnt < 1 ? 1 : cnt));
    }

    *width = sizeViewport.x;
    *height = sizeViewport.y;

    return TRUE;
}