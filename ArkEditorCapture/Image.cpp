#include "Image.h"
#include <windows.h>
#include <gdiplus.h>
#include <stdio.h>

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;
static ULONG_PTR gdiplusToken;

int
InitImage(
)
{
    GdiplusStartupInput gdiplusStartupInput;

    if (Ok != GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL))
    {
        gdiplusToken = 0;
        return FALSE;
    }

    return TRUE;
}

void
CloseImage(
)
{
    if (gdiplusToken)
        GdiplusShutdown(gdiplusToken);

    gdiplusToken = 0;
}

int
ConvertImage(
    int type,
    const char* srcPath,
    const char* dstPath
)
{
    if (0 == gdiplusToken && FALSE == InitImage())
        return FALSE;

    /*
        IMAGE/BMP  : {557CF400-1A04-11D3-9A73-0000F81EF32E}
        IMAGE/JPEG : {557CF401-1A04-11D3-9A73-0000F81EF32E}
        IMAGE/GIF  : {557CF402-1A04-11D3-9A73-0000F81EF32E}
        IMAGE/TIFF : {557CF405-1A04-11D3-9A73-0000F81EF32E}
        IMAGE/PNG  : {557CF406-1A04-11D3-9A73-0000F81EF32E}
    */

    HRESULT hr;
    const wchar_t* uuid;

    switch (type)
    {
    case 1: // BMP
        uuid = L"{557CF400-1A04-11D3-9A73-0000F81EF32E}";
        break;
    case 2: // JPG
        uuid = L"{557CF401-1A04-11D3-9A73-0000F81EF32E}";
        break;
    case 3: // GIF
        uuid = L"{557CF402-1A04-11D3-9A73-0000F81EF32E}";
        break;
    case 4: // TIF
        uuid = L"{557CF405-1A04-11D3-9A73-0000F81EF32E}";
        break;
    case 5: // PNG
        uuid = L"{557CF406-1A04-11D3-9A73-0000F81EF32E}";
        break;
    default:
        return FALSE;
    }

    CLSID encoderClsid;

    hr = CLSIDFromString(uuid, &encoderClsid);

    if (FALSE == SUCCEEDED(hr))
        return FALSE;

    Bitmap* bmp = nullptr;
    wchar_t* buf1 = nullptr;
    wchar_t* buf2 = nullptr;

    int rst = 0;

    if (nullptr == (buf1 = new wchar_t[MAX_PATH]))
        goto ERROR_RESULT;

    if (0 == MultiByteToWideChar(CP_UTF8, 0, srcPath, -1, buf1, MAX_PATH))
        goto ERROR_RESULT;

    if (nullptr == (buf2 = new wchar_t[MAX_PATH]))
        goto ERROR_RESULT;

    if (0 == MultiByteToWideChar(CP_UTF8, 0, dstPath, -1, buf2, MAX_PATH))
        goto ERROR_RESULT;

    if (nullptr == (bmp = new Bitmap(buf1)))
        goto ERROR_RESULT;

    if (Ok != bmp->GetLastStatus())
        goto ERROR_RESULT;

    if (Ok != bmp->Save(buf2, &encoderClsid, NULL))
        goto ERROR_RESULT;

    rst = 1;

ERROR_RESULT:

    if (nullptr != buf1)
        delete[] buf1;

    if (nullptr != buf2)
        delete[] buf2;

    if (nullptr != bmp)
        delete bmp;

    return 1 == rst;
}

int 
SaveImageParts(
    int type,
    int cutSizeX,
    int cutSizeY,
    int startX,
    int startY,
    int maxX,
    int maxY,
    const char* extension,
    const char* srcPath,
    const char* dstPath
) 
{
    if (0 == gdiplusToken && FALSE == InitImage())
        return FALSE;

    /*
        IMAGE/BMP  : {557CF400-1A04-11D3-9A73-0000F81EF32E}
        IMAGE/JPEG : {557CF401-1A04-11D3-9A73-0000F81EF32E}
        IMAGE/GIF  : {557CF402-1A04-11D3-9A73-0000F81EF32E}
        IMAGE/TIFF : {557CF405-1A04-11D3-9A73-0000F81EF32E}
        IMAGE/PNG  : {557CF406-1A04-11D3-9A73-0000F81EF32E}
    */

    HRESULT hr;
    const wchar_t* uuid;

    switch (type)
    {
    case 1: // BMP
        uuid = L"{557CF400-1A04-11D3-9A73-0000F81EF32E}";
        break;
    case 2: // JPG
        uuid = L"{557CF401-1A04-11D3-9A73-0000F81EF32E}";
        break;
    case 3: // GIF
        uuid = L"{557CF402-1A04-11D3-9A73-0000F81EF32E}";
        break;
    case 4: // TIF
        uuid = L"{557CF405-1A04-11D3-9A73-0000F81EF32E}";
        break;
    case 5: // PNG
        uuid = L"{557CF406-1A04-11D3-9A73-0000F81EF32E}";
        break;
    default:
        return FALSE;
    }

    CLSID encoderClsid;

    hr = CLSIDFromString(uuid, &encoderClsid);

    if (FALSE == SUCCEEDED(hr))
        return FALSE;

    Image* image = nullptr;
    wchar_t* buf1 = nullptr;
    wchar_t* buf2 = nullptr;
    Bitmap* parts = nullptr;
    Graphics* graphics = nullptr;

    int rst = 0;

    char zx_string[MAX_PATH];
    char zxy_string[MAX_PATH];

    if (nullptr == (buf1 = new wchar_t[MAX_PATH]))
        goto ERROR_RESULT;

    if (0 == MultiByteToWideChar(CP_UTF8, 0, srcPath, -1, buf1, MAX_PATH))
        goto ERROR_RESULT;

    if (nullptr == (buf2 = new wchar_t[MAX_PATH]))
        goto ERROR_RESULT;

    if (nullptr == (image = new Image(buf1)))
        goto ERROR_RESULT;

    {
        int width = image->GetWidth();
        int height = image->GetHeight();

        int cntX = width / cutSizeX;
        int cntY = height / cutSizeY;

        if (NULL == (parts = new Bitmap(cutSizeX, cutSizeY, PixelFormat32bppARGB)))
            goto ERROR_RESULT;

        if (NULL == (graphics = new Graphics(parts)))
            goto ERROR_RESULT;

        for (int x = 0; x < cntX; ++x)
        {
            if (maxX < x)
                continue;

            sprintf_s(zx_string, sizeof(zx_string), "%s\\%d", dstPath, startX + x);
            CreateDirectory(zx_string, NULL);

            for (int y = 0; y < cntY; ++y)
            {
                if (maxY < y)
                    continue;

                sprintf_s(zxy_string, sizeof(zxy_string), "%s\\%d.%s", zx_string, startY + y, extension);

                if (GetFileAttributes(zxy_string) != ((DWORD)-1))
                    continue;

                graphics->DrawImage(
                    image,
                    0, 0,
                    cutSizeX * x,
                    cutSizeY * y,
                    cutSizeX,
                    cutSizeY, UnitPixel);

                if (0 == MultiByteToWideChar(CP_UTF8, 0, zxy_string, -1, buf2, MAX_PATH))
                    goto ERROR_RESULT;

                if (Ok != parts->Save(buf2, &encoderClsid, NULL))
                    goto ERROR_RESULT;
            }
        }
    }

    rst = 1;

ERROR_RESULT:

    if (nullptr != graphics)
        delete graphics;

    if (nullptr != parts)
        delete parts;

    if (nullptr != buf1)
        delete[] buf1;

    if (nullptr != buf2)
        delete[] buf2;

    if (nullptr != image)
        delete image;

    return 1 == rst;
}