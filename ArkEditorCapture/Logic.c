#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>

#include "Logic.h"
#include "Process.h"
#include "Dialog.h"
#include "Memory.h"
#include "InjectDll.h"
#include "Pipe.h"
#include "Reg.h"
#include "Image.h"
#include "SaveText.h"
#include "Utility.h"
#include "resource.h"

static CParam cParam;

UINT WINAPI 
WorkThread(
    LPVOID lpParam
);

int
ResizeToTileSize(
    long targetSize,
    long* width,
    long* height
);

HWND
GetUnrealHandle(
);

int 
LoadArkEditor(
    void* hDlg,
    unsigned long pID,
    char* dllPath
)
{
    char Buf[MAX_PATH];
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

        if (GetModuleFileName(NULL, Buf, MAX_PATH))
        {
            char* ptr = strrchr(Buf, '\\');

            if (ptr)
            {
                *ptr = 0;
                const size_t namelen = strlen(Buf) + strlen(DLLName) + 1;

                if (namelen <= MAX_PATH)
                {
                    sprintf_s(Buf, sizeof(Buf), "%s\\%s", Buf, DLLName);
                    attributes = GetFileAttributes(Buf);

                    if (INVALID_FILE_ATTRIBUTES != attributes ||
                        0 == (attributes & FILE_ATTRIBUTE_DIRECTORY))
                    {
                        dllPath = Buf;
                        SetDlgItemText(cParam.hDlg, IDC_EDIT_DLPATH, Buf);
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
QueryEngineVariable(
)
{
    if (NULL == cParam.hDlg)
        return;

    char Buf[50];

    switch (cParam.UEVersion)
    {
    case 4:
    {
        Vec3D vec;
        if (RequestMessage(MODE_GET_XYZ, sizeof(Vec3D), &vec))
        {
            SetDlgItemFloat(cParam.hDlg, IDC_EDIT_X1, vec.x);
            SetDlgItemFloat(cParam.hDlg, IDC_EDIT_Y1, vec.y);
        }
        break;
    }
    case 5:
    {
        Vec3DD vecd;
        if (RequestMessage(MODE_GET_XYZ, sizeof(Vec3DD), &vecd))
        {
            SetDlgItemFloat(cParam.hDlg, IDC_EDIT_X1, (float)vecd.x);
            SetDlgItemFloat(cParam.hDlg, IDC_EDIT_Y1, (float)vecd.y);
        }
        break;
    }
    }

    if (RequestMessage(MODE_GET_ZOOM, 4, Buf))
    {
        float fval;
        memcpy(&fval, Buf, sizeof(float));
        SetDlgItemFloat(cParam.hDlg, IDC_EDIT_Z1, fval);
    }

    if (RequestMessage(MODE_GET_WH, 8, Buf))
    {
        long lval;
        memcpy(&lval, Buf, sizeof(long));
        SetDlgItemInt(cParam.hDlg, IDC_WHI_W, lval, TRUE);

        memcpy(&lval, Buf + 4, sizeof(long));
        SetDlgItemInt(cParam.hDlg, IDC_WHI_H, lval, TRUE);
    }
}

int
OpenArkEditor(
    void* _hDlg
)
{
    char Buf[255];
    const HWND hDlg = _hDlg;

    const char* UE4prefix = "UE4Editor";
    const char* UE5prefix = "ShooterGameEditor";
    const char* UE5prefix2 = "UnrealEditor";

    GetDlgItemText(hDlg, IDC_EDIT_PN, Buf, sizeof(Buf));

    if (0 == strncmp(UE4prefix, Buf, sizeof(UE4prefix)))
        cParam.UEVersion = 4;
    else if (0 == strncmp(UE5prefix, Buf, sizeof(UE5prefix)))
        cParam.UEVersion = 5;
    else if (0 == strncmp(UE5prefix2, Buf, sizeof(UE5prefix2)))
        cParam.UEVersion = 5;
    else 
    {
        MessageBox(hDlg, "Unknown process name.", "Error", MB_ICONEXCLAMATION);
        return FALSE;
    }

    const unsigned long pID = GetpIDByName(Buf);

    if (0 == pID)
    {
        MessageBox(hDlg, "Process not found.", "Error", MB_ICONEXCLAMATION);
        return FALSE;
    }

    GetDlgItemText(hDlg, IDC_EDIT_DLPATH, Buf, sizeof(Buf));

    if (FALSE == LoadArkEditor(hDlg, pID, Buf))
        return FALSE;

    const char* pptr = UE4prefix;

    if (cParam.UEVersion == 4)
        pptr = UE4prefix;
    else if (cParam.UEVersion == 5)
        pptr = UE5prefix;
    
    const int sz = (int)strlen(pptr);

    Buf[0] = cParam.UEVersion;
    Buf[1] = (char)sz;
    memcpy(&Buf[2], pptr, sz);

    if (FALSE == SetMessage(MODE_SET_VER, 2 + sz, Buf))
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
    int quality,
    int mode,
    void* endcb
)
{
    if (0 < cParam.Status)
    {
        StopCapture();
        return TRUE;
    }

    DisableControls();

    cParam.Status = 1;
    cParam.quality = quality;
    cParam.currentMode = mode;

    // 새로운 스레드를 만들어 메인폼이 안 멈추게 한다.
    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, WorkThread, endcb, 0, NULL);

    if (NULL == hThread)
    {
        EnableControls();
        return FALSE;
    }

    CloseHandle(hThread);
    return TRUE;
}

void
DisableControls(
)
{
    EnableAllControls(cParam.hDlg, IDC_AAA_START_CTRL, IDC_ZZZ_END_CTRL, FALSE);
    KillTimer(cParam.hDlg, 1);
}

void
EnableControls(
)
{
    SetWindowText(GetDlgItem(cParam.hDlg, IDC_BUTTON_START), "Start Capture");
    SetTimer(cParam.hDlg, 1, 100, NULL);

    EnableAllControls(cParam.hDlg, IDC_AAA_START_CTRL, IDC_ZZZ_END_CTRL, TRUE);
    EnableWindow(GetDlgItem(cParam.hDlg, IDC_BUTTON_OPEN), FALSE);
}

void
SetButtonAction(
    int mode
)
{
    switch (mode)
    {
        case BUTTON_MODE_SETZERO:
        {
            // 뷰포트 카메라 위치를 0으로 설정하는 로직
            // Set viewport camera position to zero.
            if (cParam.UEVersion == 4)
            {
                Vec3D vec = { 0 };
                SetMessage(MODE_SET_XY, sizeof(Vec3D), &vec);
            }
            else if (cParam.UEVersion == 5)
            {
                Vec3DD vecd = { 0 };
                SetMessage(MODE_SET_XY, sizeof(Vec3DD), &vecd);
            }
            break;
        }
        case BUTTON_MODE_ORIZOOM:
        {
            // 뷰포트 카메라 Zoom Level을 메인폼에 설정된 값으로 변경해 주는 로직
            // Logic to change the viewport camera zoom level to the value set in the main form.
            float Zoom = GetDlgItemFloat(cParam.hDlg, IDC_EDIT_Z2);
            SetMessage(MODE_SET_ZOOM, 4, &Zoom);
            break;
        }
        case BUTTON_MODE_AUTOSIZE:
        {
            // 뷰포트의 카메라 위치에 따라 x, y 시작과 종료 좌표를 자동으로 계산해 주는 로직
            // Logic to automatically calculate the start and end coordinates (x, y) based on the viewport camera position.
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

            // 언리얼 엔진의 창 크기를 아주 조금 변경했다가 되돌려서 뷰포트 크기가 DLL에 의해 수집될 수 있도록 한다.
            // Modify the Unreal Engine window size slightly and then revert it so that the viewport size can be collected by the DLL.
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

            SetDlgItemFloat(cParam.hDlg, IDC_EDIT_X2, Coord.x);
            SetDlgItemFloat(cParam.hDlg, IDC_EDIT_Y2, Coord.y);

            Vec2D From = Coord;

            From.x = -From.x;
            From.y = -From.y;

            // 맵의 종료 좌표를 SAVETILESIZE 에 의해 얼마나 이동해야 하는지 계산하는 함수를 실행시켜 좌표를 구한다.
            // Execute a function to calculate how far the map's end coordinates need to be moved based on SAVETILESIZE, 
            // then determine the coordinates.
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

UINT WINAPI 
WorkThread(
    LPVOID lpParam
)
{
    const HANDLE hProcess = (HANDLE)cParam.hProcess;
    const HWND hDlg = (HWND)cParam.hDlg;

    const int mode = cParam.currentMode;

    char UEPath[MAX_PATH] = { 0 };
    char BMPPath[MAX_PATH] = { 0 };
    char SavePath[MAX_PATH] = { 0 };
    char Buf[MAX_PATH];

    if (LOGIC_MODE_PREVIEW != mode)
        StoreRegistryValue(hDlg);

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

    strcpy_s(BMPPath, sizeof(UEPath), UEPath);

    if (strlen(BMPPath) + 13 >= MAX_PATH) // 00001.bmp 
    {
        MessageBox(hDlg, "The file path in ARK DevKit is too long. Please check it.", "Error", MB_ICONEXCLAMATION);
        goto EXIT_THREAD;
    }

    if (cParam.UEVersion == 4)
        strcat_s(BMPPath, sizeof(BMPPath), "00001.bmp");
    else if (cParam.UEVersion == 5)
        // 언리얼 엔진 5에서 HighResShot 명령어로 내보낸 파일은 비트맵 형식이 아니다.
        // Files exported using the HighResShot command in Unreal Engine 5 are not in bitmap format.
        strcat_s(BMPPath, sizeof(BMPPath), "00000.png");

    if (LOGIC_MODE_PREVIEW != mode)
    {
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
    }
    else
    {
        GetModuleFileName(NULL, SavePath, MAX_PATH);
        char* ptr = strrchr(SavePath, '\\');

        if (ptr)
            *ptr = 0;

        strcat_s(SavePath, sizeof(SavePath), PREVIEW_PREFIX);
        CreateDirectory(SavePath, NULL);
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

    if (LOGIC_MODE_PREVIEW == mode)
    {
        strcpy_s(Extension, sizeof(Extension), "BMP");
        Z_to = Z_from = 0;
    }

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

    if (FALSE == ResizeToTileSize(SAVETILESIZE, &VpWidth, &VpHeight))
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

    DeleteFile(BMPPath);

    // 뷰포트 크기와 TileSize가 얼마나 차이 나는지 계산해서 HighResShot 의 값에 들어갈 수치를 구한다.
    // Calculate the difference between the viewport size and TileSize, then determine the scale value to 
    // use in HighResShot <scale>
    const float resValue = (((float)TileSize / VpWidth) + ((float)TileSize / VpHeight)) / 2;
    float Zoom = StartZ;

    cParam.Status = 2;

    if (LOGIC_MODE_PREVIEW != mode)
    {
        EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_START), TRUE);
        SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_START), "Stop");

        long long timeval = GetTickCount64();

        // 타일맵 변환에 쓰인 x, y 시작점과 종료 좌표를 파일로 기록해 나중에 확인할 수 있게 한다.
        // Save the x, y start and end coordinates used for tilemap conversion to a file for future reference.
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

        // DLL에 맵에 로드된 모든 엑터 ( 오브젝트 ) 의 위치를 구하는 명령을 한다.
        // Execute a command in the DLL to retrieve the positions of all actors ( objects ) loaded in the map.
        while (FALSE == SetMessage(MODE_GET_ACTORS, 0, NULL))
        {
            if (++WaitCount > 30)
                break;

            Sleep(100);
        }

        // 30보다 작으면 한 번은 성공했다는 상태이므로
        // If the value is less than 30, it means at least one attempt was successful
        if (WaitCount <= 30)
        {
            char Buf2[MAX_PATH];

            strcpy_s(Buf2, sizeof(Buf2), UEPath);
            strcat_s(Buf2, sizeof(Buf2), "AActors.txt");

            MoveFile(Buf2, Buf);
        }
    }

    QueryEngineVariable();

    Vec3D originalCoord = {
        GetDlgItemFloat(hDlg, IDC_EDIT_X1),
        GetDlgItemFloat(hDlg, IDC_EDIT_Y1),
        GetDlgItemFloat(hDlg, IDC_EDIT_Z1)
    };

    for (long ZoomLvl = 0; ZoomLvl <= Z_from; ++ZoomLvl, Zoom /= 2)
    {
        // 최소 요청 Zoom Level이 아닌 경우 넘긴다.
        // If it is not the minimum required Zoom Level, skip it.
        if (ZoomLvl < Z_to)
            continue;

        SetDlgItemInt(hDlg, IDC_WHI_I, ZoomLvl, TRUE);

        // Zoom Level에 매직 넘버 ( ZOOM = 1 일 때 x 또는 y 가 몇 움직이는가? ) 을 곱한다.
        // Multiply the Zoom Level by the magic number (how much x or y moves when ZOOM = 1).
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

        // Zoom level을 나타내는 폴더를 생성한다.
        // Create a folder named after the Zoom Level value.
        sprintf_s(z_string, sizeof(z_string), "%s\\%d", SavePath, ZoomLvl);
        CreateDirectory(z_string, NULL);

        // 맵을 어디까지 캡처할지 범위를 지정하는 boundary를 선언한다.
        // Declare a boundary to define the capture range of the map.
        long LimitX = (long)((Coord_From.x - Coord_To.x) / resInc.x + 0.5) - 1;
        long LimitY = (long)((Coord_From.y - Coord_To.y) / resInc.y + 0.5) - 1;

        // 맵이 타일 크기에 비해 작을 때 음수가 되는데 버그를 방지하기 위해 0보다 작을 때 0으로 만들어 준다.
        // If the map is smaller than the tile size, it may become negative. To prevent bugs, 
        // set it to 0 when it is less than 0.
        if (LimitX < 0)
            LimitX = 0;

        if (LimitY < 0)
            LimitY = 0;

        // 반복하면서 지정된 Zoom Level까지 캡처한다.
        // Capture repeatedly until the specified Zoom Level is reached.
        for (long IndexY = 0; IndexY <= LimitY; ++IndexY)
        {
            for (long IndexX = 0; IndexX <= LimitX; ++IndexX)
            {
                // 만약 메인폼에서 Stop 요청을 할 경우 스레드 종료 분기로 빠져나간다.
                // Simply terminate if a Stop request is made from the main form.
                if (2 != cParam.Status)
                    goto EXIT_THREAD;

                // 이름에 쓰일 x, y 좌표를 구한다. 0, 1, 2 ...
                // Set the x, y coordinates to be used in the name, such as 0, 1, 2...
                const long x = TileMul.x * IndexX;
                const long y = TileMul.y * IndexY;

                // 각각 x 폴더와 y 파일의 경로가 담긴 문자열을 집어넣는다.
                // Format a string to include the paths of the x folder and y file.
                sprintf_s(zx_string, sizeof(zx_string), "%s\\%d", z_string, x);
                sprintf_s(zxy_string, sizeof(zxy_string), "%s\\%d.%s", zx_string, y, Extension);

                // 파일이 없을 때 ( 파일이 있으면 캡처하지 않는다. )
                // When the file does not exist ( if a file exists, do not capture. )
                if (GetFileAttributes(zxy_string) == ((DWORD)-1))
                {
                    // 언리얼 엔진 뷰포트 카메라 위치 설정을 위한 x, y 좌표 설정
                    // Set the x and y coordinates for configuring the Unreal Engine viewport camera position.
                    const Vec2D rCoord = {
                        StartCoord.x + Increase.x * x,
                        StartCoord.y + Increase.y * y
                    };

                    // 몇 바이트 쓰였는지 저장하는 변수 ( 언리얼 4, 5 대응 )
                    // Variable to store the number of bytes used (compatible with Unreal Engine 4 and 5).
                    int writeBytes = 0;

                    if (cParam.UEVersion == 4)
                    { 
                        // 언리얼 엔진 4 는 FVector 구조체가 12바이트 ( float 3개 ) 이다.
                        // In Unreal Engine 4, the FVector structure is 12 bytes ( consisting of three floats ).
                        Vec5D vec = { 0 };

                        // 뷰포트 카메라 위치 설정을 위한 x, y, z, zoom 그리고 HighResShot 에 들어갈 인자값을 집어넣는다.
                        // Set the x, y, z, zoom, and HighResShot parameters for viewport camera position configuration.
                        vec.x = rCoord.x;
                        vec.y = rCoord.y;
                        vec.zoom = Zoom;
                        vec.resval = resValue;

                        memcpy(Buf, &vec, writeBytes = sizeof(Vec5D));
                    }
                    else if (cParam.UEVersion == 5)
                    {
                        // 언리얼 엔진 5 는 FVector 구조체가 24바이트 ( double 3개 ) 이다.
                        // In Unreal Engine 5, the FVector structure is 24 bytes ( consisting of three doubles ).
                        Vec5DD vecd = { 0 };

                        // 뷰포트 카메라 위치 설정을 위한 x, y, z, zoom 그리고 HighResShot 에 들어갈 인자값을 집어넣는다.
                        // Set the x, y, z, zoom, and HighResShot parameters for viewport camera position configuration.
                        vecd.x = (float)rCoord.x;
                        vecd.y = (float)rCoord.y;
                        vecd.zoom = Zoom;
                        vecd.resval = resValue;

                        memcpy(Buf, &vecd, writeBytes = sizeof(Vec5DD));
                    }
                    
                    // writeBytes 가 0이면 언리얼 엔진 버전이 4 또는 5가 아니라는 소리 ( 비정상 )
                    // If writeBytes is 0, it means the Unreal Engine version is neither 4 nor 5 (abnormal case)
                    if (0 == writeBytes)
                        goto EXIT_THREAD;

                    // 캡처를 위해 파이프 통신으로 DLL에 캡처 명령을 보낸다.
                    // Send a capture command to the DLL through pipe communication for capturing.
                    for (int i = 0; i <= 3000; ++i)
                    {
                        // 동기 함수이므로 캡처가 성공 또는 실패의 결괏값이 나와야 다음 로직으로 넘어간다.
                        // Since this is a synchronous function, the capture must return a success or failure result 
                        // before proceeding to the next logic.
                        SetMessage(MODE_SET_XYZZ_CAPTURE, writeBytes, Buf);

                        // 언리얼 엔진에서 내보낸 비트맵 파일이 있으면 반복문을 종료한다.
                        // Exit the loop if a bitmap file exported from Unreal Engine exists.
                        if (GetFileAttributes(BMPPath) != ((DWORD)-1))
                            break;

                        // 약간의 지연을 줘서 과도한 CPU 점유율을 방지한다.
                        // Add a short delay to reduce excessive CPU usage.
                        Sleep(10);
                    }

                    // 이름이 x 좌푯값인 폴더를 만든다.
                    // Create a folder named after the x coordinate.
                    if (0 == IndexY)
                        CreateDirectory(zx_string, NULL);

                    WaitCount = 0;

                    // 기본 타일 크기보다 큰 타일을 골랐을 때 몇 칸을 잘라야 하는지 알려주는 변수를 선언한다.
                    // Declare a variable to indicate how many tiles to cut when selecting a tile larger than the default size.
                    const long cutX = (long)((EndCoord.x - rCoord.x) / Increase.x + 0.5) - 1;
                    const long cutY = (long)((EndCoord.y - rCoord.y) / Increase.y + 0.5) - 1;

                    // 이미지 저장 로직을 실행한다.
                    // Execute the image-saving logic.
                    while (!SaveImageParts(ExtensionIndex,
                        cParam.quality,
                        SAVETILESIZE,
                        x, y,
                        cutX, cutY,
                        Extension, BMPPath, z_string))
                    {
                        if (++WaitCount > 10)
                        {
                            MessageBox(hDlg, "failed to convert image.", "Error", MB_ICONEXCLAMATION);
                            goto EXIT_THREAD;
                        }

                        Sleep(100);
                    }

                    // 언리얼에서 내보낸 비트맵 파일의 사용이 끝났으므로, 삭제한다. 
                    // ( 삭제를 안 하면 이름 인덱스가 카운터 되므로 버그가 일어난다. )
                    // Delete the bitmap file exported from Unreal Engine after it is no longer needed.
                    // ( If not deleted, the name index is counted, causing a bug. )
                    DeleteFile(BMPPath);

                    // 어디까지 좌표를 이동했는지 x, y 좌푯값을 갱신해 메인폼에 표시한다.
                    // Update and display the x and y coordinates on the main form to show movement.
                    QueryEngineVariable();
                }

            }
        }
    }

    // 목표 Zoom Level까지 캡처를 완료한 경우, 캡처 전 뷰포트 카메라 위치로 되돌아간다.
    // After completing the capture up to the target zoom level, revert to the viewport camera position before the capture.
    Vec5D vec = { 0 };
    Vec5DD vecd = { 0 };
    int writeBytes = 0;

    if (cParam.UEVersion == 4)
    {
        vec.x = originalCoord.x;
        vec.y = originalCoord.y;
        vec.zoom = originalCoord.z;

        memcpy(Buf, &vec, writeBytes = sizeof(Vec5D));
    }
    else if (cParam.UEVersion == 5)
    {
        vecd.x = originalCoord.x;
        vecd.y = originalCoord.y;
        vecd.zoom = originalCoord.z;

        memcpy(Buf, &vecd, writeBytes = sizeof(Vec5DD));
    }

    WaitCount = 0;

    while (FALSE == SetMessage(MODE_SET_XYZZ_CAPTURE, writeBytes, Buf))
    {
        if (++WaitCount > 30)
            break;

        Sleep(100);
    }

EXIT_THREAD:

    cParam.Status = 0;

    // 스레드가 종료되면, 메인폼의 컨트롤을 활성화한다.
    // Enable the controls on the main form when the thread terminates.
    EnableControls();

    // 스레드 종료할 때 호출해야 할 프로시저가 등록됐을 경우, 함수 포인터로 지정된 프로시저를 실행한다.
    // If a procedure is registered for thread termination, run the procedure specified by the function pointer.
    if (lpParam)
        ((void(*)())lpParam)();

    return 0;
}

void
StopCapture(
)
{
    cParam.Status = 1;
}

int
ResizeToTileSize(
    long targetSize,
    long* width,
    long* height
)
{
    // 원활한 캡처를 위해 언리얼 엔진의 뷰포트(Viewport) 크기를 물리적 ( 뷰포트 사이즈를 바로 조절 
    // 하는 게 아닌 시행착오를 통해 변경 ) 으로 변경하는 로직
    // Since the viewport size cannot be directly modified, it is adjusted indirectly.

    POINT sizeViewport = { 0 };
    POINT prevSIze = { 0 };
    POINT offset = { 0 };
    RECT currentSize = { 0 };
    int cnt = 10;
    int scnt = 10;
    int fail = 0;
    int WaitCount;

    // 안에 while 문이 있으므로 for 문으로 일단 무한루프 선언
    // Declare an infinite loop using a for loop since it contains a while loop inside.
    for (;;)
    {
        WaitCount = 0;
        Sleep(25 * (cnt < 1 ? 1 : cnt));

        // DLL에 언리얼 엔진의 현재 뷰포트 크기 값을 요청한다.
        // Request the current viewport size value of Unreal Engine from the DLL.
        while (!RequestMessage(MODE_GET_WH, sizeof(POINT), &sizeViewport))
        {
            if (++WaitCount > 10)
            {
                // 10번 이상 요청했는데도 모른다고 답할 때 메시지 박스를 띄운다.
                // Display a message box if the request is made more than 10 times and still returns an unknown response.
                MessageBox(cParam.hDlg,
                    "Failed to load the viewport rendering size in ARK DevKit.",
                    "Error", MB_ICONEXCLAMATION);
                return 0;
            }

            Sleep(100);
        }

        // 물리적으로 변경한 뷰포트 크기가 목푯값과 일치할 때 재차 확인해서 검증한다.
        // Verify again when the indirectly adjusted viewport size matches the target value.
        if (sizeViewport.x == targetSize &&
            sizeViewport.y == targetSize)
        {
            // 5번 확인 후 뷰포트 크기가 변동 없이 목푯값과 일치하면 최상위 반복문을 빠져나간다.
            // Exit the top-level loop if the viewport size remains unchanged and matches the target value after five checks.
            if (++scnt > 5)
                break;

            fail = 0;
            continue;
        }

        scnt = 0;
        // 언리얼 엔진 윈도우 핸들을 구한다.
        // Get the Unreal Engine window handle.
        HWND unrealhWnd = GetUnrealHandle();

        if (NULL == unrealhWnd)
        {
            // 언리얼 엔진 윈도우 핸들을 구할 수 없을 때 메시지 박스를 띄워 함수를 종료한다.
            // Show a message box and exit the function if the Unreal Engine window handle can't be gotten.
            MessageBox(cParam.hDlg,
                "Failed to get the window handle in ARK DevKit.",
                "Error", MB_ICONEXCLAMATION);
            return FALSE;
        }

        POINT resize;
        RECT rc;

        // 언리얼 엔진의 창 크기를 구해 rc에 값을 집어넣는다.
        // Get the Unreal Engine window size and store the value in rc.
        GetWindowRect(unrealhWnd, &rc);

        // SetWindowPos 요청에도 불구하고 언리얼 엔진의 윈도우 크기가 변동이 없을 때 fail 값을 증가시켜 정상이 아님을 감지한다.
        // Increase the fail value if the Unreal Engine window size doesn't change despite the SetWindowPos request.
        if (rc.left == currentSize.left &&
            rc.right == currentSize.right &&
            rc.top == currentSize.top &&
            rc.bottom == currentSize.bottom)
        {
            ++fail;
        }
        else
            fail = 0;

        currentSize = rc;

        /*
            SetWindowPos를 통해 언리얼 엔진의 창 크기가 조절될 때 증가 또는 감소의 상태가 뷰포트와 같은지 점검하는 플래그 선언

            SetWindowPos가 언리얼 엔진의 창 크기를 줄이고 싶은데 뷰포트 크기가 반대로 늘어날 경우, 서로 어긋나서 ( 지연이 생겨서 크기 변환 속도가 느림 )
            결국 목푯값에 도달하는 데 시간이 오래 걸리니까 방지하고자 플래그를 설정한다.

            Declare a flag to check if the viewport size changes correctly when adjusting the Unreal Engine window size with SetWindowPos.

            A flag is set to prevent delays when the Unreal Engine window size is reduced using SetWindowPos, but the viewport size increases instead. 
            This mismatch slows down the resizing process, making it take longer to reach the target value.
        */
        const int x_direction = ((targetSize - sizeViewport.x) < 0 && (offset.x < 0)) ||
            ((targetSize - sizeViewport.x) >= 0 && (offset.x >= 0));
        const int y_direction = ((targetSize - sizeViewport.y) < 0 && (offset.y < 0)) ||
            ((targetSize - sizeViewport.y) >= 0 && (offset.y >= 0));

        // 얼마나 창을 늘리고 줄여야 하는지 계산해서 offset에 수치를 입력한다.
        // Calculate how much to increase or decrease the window size and input the value into offset.
        offset.x = targetSize - sizeViewport.x;
        offset.y = targetSize - sizeViewport.y;

        // 수치를 반으로 나누어 부드럽게 변환을 가능하게 한다.
        // Divide the value in half to allow smoother transformation.
        if (offset.x / 2 >= 1)
            offset.x /= 2;

        if (offset.y / 2 >= 1)
            offset.y /= 2;

        // 위에서 구한 수치를 언리얼 엔진 창 크기에 더해서 실제 변환 수치를 계산한 값을 resize에 입력한다.
        // Add the calculated value to the Unreal Engine window size and store the final transformation value in resize.
        resize.x = rc.right - rc.left + offset.x;
        resize.y = rc.bottom - rc.top + offset.y;

        // 창 크기가 너무 작을 때 최솟값을 지정한다.
        // Set a minimum value when the window size is too small.
        if (resize.x < SAVETILESIZE)
            resize.x = SAVETILESIZE;

        if (resize.y < SAVETILESIZE)
            resize.y = SAVETILESIZE;

        // 언리얼 엔진 창 크기를 변경한다.
        // Set the Unreal Engine window size.
        SetWindowPos(unrealhWnd, NULL, 0, 0, resize.x, resize.y, SWP_NOMOVE | SWP_NOZORDER);

        // 언리얼 엔진 창 크기를 조절하는데 약간의 버벅거림이 있다면, 지연을 줘 변환 로직을 늦게 설정한다.
        // If there's slight stuttering when adjusting the Unreal Engine window size, add a delay to slow down the resizing process.
        if ((x_direction && (0 == offset.x || prevSIze.x != sizeViewport.x)) &&
            (y_direction && (0 == offset.y || prevSIze.y != sizeViewport.y)))
            --cnt;
        else
            ++cnt;

        // 창 크기 조절에 실패할 때 메시지 띄우고 종료한다.
        // Show a message and exit if resizing the window fails.
        if (fail >= 5 ||
            cnt >= 200 || cnt <= -200)
        {
            MessageBox(cParam.hDlg,
                "Failed to automatically adjust the viewport rendering size in ARK DevKit.",
                "Error", MB_ICONEXCLAMATION);
            return FALSE;
        }

        prevSIze = sizeViewport;
    }

    // 최종적으로 목푯값에 도달한 뷰포트 크기를 포인터에 넣고 종료한다.
    // Store the final viewport size that reached the target value into the pointer and exit.
    *width = sizeViewport.x;
    *height = sizeViewport.y;

    return TRUE;
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

void
FetchRegistryValue(
    void* _hDlg
)
{
    char Buf[255] = { 0 };
    const HWND hDlg = (HWND)_hDlg;

    ReadReg(REG_PATH, "IDC_EDIT_PN", "UE4Editor.exe", Buf);
    SetDlgItemText(hDlg, IDC_EDIT_PN, Buf);

    ReadReg(REG_PATH, "IDC_EDIT_DLPATH", "", Buf);
    SetDlgItemText(hDlg, IDC_EDIT_DLPATH, Buf);

    ReadReg(REG_PATH, "IDC_EDIT_SVPATH", "C:\\", Buf);
    SetDlgItemText(hDlg, IDC_EDIT_SVPATH, Buf);

    ReadReg(REG_PATH, "IDC_EDIT_X2", "-1000000", Buf);
    SetDlgItemText(hDlg, IDC_EDIT_X2, Buf);

    ReadReg(REG_PATH, "IDC_EDIT_X3", "1000000", Buf);
    SetDlgItemText(hDlg, IDC_EDIT_X3, Buf);

    ReadReg(REG_PATH, "IDC_EDIT_Y2", "-1000000", Buf);
    SetDlgItemText(hDlg, IDC_EDIT_Y2, Buf);

    ReadReg(REG_PATH, "IDC_EDIT_Y3", "1000000", Buf);
    SetDlgItemText(hDlg, IDC_EDIT_Y3, Buf);

    ReadReg(REG_PATH, "IDC_EDIT_Z2", "12800000", Buf);
    SetDlgItemText(hDlg, IDC_EDIT_Z2, Buf);

    ReadReg(REG_PATH, "IDC_EDIT_Z3", "0", Buf);
    SetDlgItemText(hDlg, IDC_EDIT_Z3, Buf);

    ReadReg(REG_PATH, "IDC_EDIT_Z4", "7", Buf);
    SetDlgItemText(hDlg, IDC_EDIT_Z4, Buf);

    SendDlgItemMessage(hDlg, IDC_COMBO_EXTIMG, CB_ADDSTRING, 0, (LPARAM)TEXT("BMP"));
    SendDlgItemMessage(hDlg, IDC_COMBO_EXTIMG, CB_ADDSTRING, 0, (LPARAM)TEXT("JPG"));
    SendDlgItemMessage(hDlg, IDC_COMBO_EXTIMG, CB_ADDSTRING, 0, (LPARAM)TEXT("GIF"));
    SendDlgItemMessage(hDlg, IDC_COMBO_EXTIMG, CB_ADDSTRING, 0, (LPARAM)TEXT("PNG"));
    SendDlgItemMessage(hDlg, IDC_COMBO_EXTIMG, CB_ADDSTRING, 0, (LPARAM)TEXT("WEBP"));

    ReadReg(REG_PATH, "IDC_COMBO_EXTIMG", "0", Buf);
    SendDlgItemMessage(hDlg, IDC_COMBO_EXTIMG, CB_SETCURSEL, atoi(Buf), 0);

    SendDlgItemMessage(hDlg, IDC_COMBO_TILESZ, CB_ADDSTRING, 0, (LPARAM)TEXT("256"));
    SendDlgItemMessage(hDlg, IDC_COMBO_TILESZ, CB_ADDSTRING, 0, (LPARAM)TEXT("512"));
    SendDlgItemMessage(hDlg, IDC_COMBO_TILESZ, CB_ADDSTRING, 0, (LPARAM)TEXT("1024"));
    SendDlgItemMessage(hDlg, IDC_COMBO_TILESZ, CB_ADDSTRING, 0, (LPARAM)TEXT("2048"));
    SendDlgItemMessage(hDlg, IDC_COMBO_TILESZ, CB_ADDSTRING, 0, (LPARAM)TEXT("4096"));
    SendDlgItemMessage(hDlg, IDC_COMBO_TILESZ, CB_ADDSTRING, 0, (LPARAM)TEXT("8192"));
    SendDlgItemMessage(hDlg, IDC_COMBO_TILESZ, CB_ADDSTRING, 0, (LPARAM)TEXT("16384"));

    ReadReg(REG_PATH, "IDC_COMBO_TILESZ", "0", Buf);
    SendDlgItemMessage(hDlg, IDC_COMBO_TILESZ, CB_SETCURSEL, atoi(Buf), 0);
}

void
StoreRegistryValue(
    void* _hDlg
)
{
    char Buf[255] = { 0 };
    const HWND hDlg = (HWND)_hDlg;

    GetDlgItemText(hDlg, IDC_EDIT_PN, Buf, sizeof(Buf));
    WriteReg(REG_PATH, "IDC_EDIT_PN", Buf);

    GetDlgItemText(hDlg, IDC_EDIT_DLPATH, Buf, sizeof(Buf));
    WriteReg(REG_PATH, "IDC_EDIT_DLPATH", Buf);

    GetDlgItemText(hDlg, IDC_EDIT_SVPATH, Buf, sizeof(Buf));
    WriteReg(REG_PATH, "IDC_EDIT_SVPATH", Buf);

    GetDlgItemText(hDlg, IDC_EDIT_X2, Buf, sizeof(Buf));
    WriteReg(REG_PATH, "IDC_EDIT_X2", Buf);

    GetDlgItemText(hDlg, IDC_EDIT_X3, Buf, sizeof(Buf));
    WriteReg(REG_PATH, "IDC_EDIT_X3", Buf);

    GetDlgItemText(hDlg, IDC_EDIT_Y2, Buf, sizeof(Buf));
    WriteReg(REG_PATH, "IDC_EDIT_Y2", Buf);

    GetDlgItemText(hDlg, IDC_EDIT_Y3, Buf, sizeof(Buf));
    WriteReg(REG_PATH, "IDC_EDIT_Y3", Buf);

    GetDlgItemText(hDlg, IDC_EDIT_Z2, Buf, sizeof(Buf));
    WriteReg(REG_PATH, "IDC_EDIT_Z2", Buf);

    GetDlgItemText(hDlg, IDC_EDIT_Z3, Buf, sizeof(Buf));
    WriteReg(REG_PATH, "IDC_EDIT_Z3", Buf);

    GetDlgItemText(hDlg, IDC_EDIT_Z4, Buf, sizeof(Buf));
    WriteReg(REG_PATH, "IDC_EDIT_Z4", Buf);

    int sel = (int)SendDlgItemMessage(hDlg, IDC_COMBO_EXTIMG, CB_GETCURSEL, 0, 0);

    Buf[0] = (char)sel + '0';
    Buf[1] = 0;

    WriteReg(REG_PATH, "IDC_COMBO_EXTIMG", Buf);

    sel = (int)SendDlgItemMessage(hDlg, IDC_COMBO_TILESZ, CB_GETCURSEL, 0, 0);

    Buf[0] = (char)sel + '0';
    Buf[1] = 0;

    WriteReg(REG_PATH, "IDC_COMBO_TILESZ", Buf);
}