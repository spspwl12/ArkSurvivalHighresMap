#include "Image.h"
#include "../libwebp/webp/encode.h"
#include <windows.h>
#include <gdiplus.h>
#include <stdio.h>
#include <stdint.h>

#pragma comment(lib, "gdiplus.lib")

#ifdef _DEBUG
#pragma comment(lib, "..\\Build\\libwebpd.lib")
#pragma comment(lib, "..\\Build\\libjpegd.lib")
#else
#pragma comment(lib, "..\\Build\\libwebp.lib")
#pragma comment(lib, "..\\Build\\libjpeg.lib")
#endif

using namespace Gdiplus;
static ULONG_PTR gdiplusToken;

int
ProcessCustomExtension(
    int type,
    Bitmap* bmp,
    int width,
    int height,
    const char* savePath,
    int quality
);

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

CLSID
GetClsid(
    int type
)
{
    HRESULT hr;
    const wchar_t* uuid;
    CLSID clsid = { 0 };

    /*
    IMAGE/BMP  : {557CF400-1A04-11D3-9A73-0000F81EF32E}
    IMAGE/JPEG : {557CF401-1A04-11D3-9A73-0000F81EF32E}
    IMAGE/GIF  : {557CF402-1A04-11D3-9A73-0000F81EF32E}
    IMAGE/TIFF : {557CF405-1A04-11D3-9A73-0000F81EF32E}
    IMAGE/PNG  : {557CF406-1A04-11D3-9A73-0000F81EF32E}
    */

    switch (type)
    {
    case IMAGE_TYPE_BMP: // BMP
        uuid = L"{557CF400-1A04-11D3-9A73-0000F81EF32E}";
        break;
    case IMAGE_TYPE_JPG: // JPG
        clsid.Data1 = 1;
        return clsid;
    case IMAGE_TYPE_GIF: // GIF
        uuid = L"{557CF402-1A04-11D3-9A73-0000F81EF32E}";
        break;
    case IMAGE_TYPE_PNG: // PNG
        uuid = L"{557CF406-1A04-11D3-9A73-0000F81EF32E}";
        break;
    case IMAGE_TYPE_WEBP: // WEBP
        clsid.Data1 = 1;
        return clsid;
    default:
        return clsid;
    }

    CLSID encoderClsid;

    hr = CLSIDFromString(uuid, &encoderClsid);

    if (FALSE == SUCCEEDED(hr))
        return clsid;

    return encoderClsid;
}

void
ConvertBGRtoRGB(
    unsigned char* data,
    int width,
    int height
)
{
    for (int i = 0, j = width * height; i < j; i++) {
        unsigned char temp = data[i * 3];
        data[i * 3] = data[i * 3 + 2];
        data[i * 3 + 2] = temp;
    }
}

unsigned char* 
GetBitmapData(
    Bitmap* bmp, 
    int width, 
    int height
) 
{
    unsigned char* buffer = (unsigned char*)malloc(width * height * 3);

    if (NULL == buffer)
        return 0;

    BitmapData bitmapData;
    Rect rect(0, 0, width, height);

    if (Ok != bmp->LockBits(&rect, ImageLockModeRead, PixelFormat24bppRGB, &bitmapData))
    {
        free(buffer);
        return 0;
    }

    memcpy(buffer, bitmapData.Scan0, width * height * 3);

    if (Ok != bmp->UnlockBits(&bitmapData))
    {
        free(buffer);
        return 0;
    }

    ConvertBGRtoRGB(buffer, width, height);

    return buffer;
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

    CLSID encoderClsid = GetClsid(type);

    if (0 == encoderClsid.Data1)
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

        if (NULL == (parts = new Bitmap(cutSizeX, cutSizeY, PixelFormat24bppRGB)))
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

                if (type == IMAGE_TYPE_JPG ||
                    type == IMAGE_TYPE_WEBP)
                { 
                    if (FALSE == ProcessCustomExtension(type,
                        parts, cutSizeX, cutSizeY, zxy_string, 100))
                        goto ERROR_RESULT;    
                }
                else if (Ok != parts->Save(buf2, &encoderClsid, NULL))
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

#include "../libjpeg/jpeglib.h"

int 
SaveJPG(
    const char* filename, 
    unsigned char* bmpData, 
    int width, 
    int height, 
    int quality
)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    FILE* file;
    errno_t err = fopen_s(&file, filename, "wb");

    if (err != 0 || file == NULL)
        return 0;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, file);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    if (quality < 100)
        jpeg_set_quality(&cinfo, quality, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    JSAMPROW row_pointer;

    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer = &bmpData[cinfo.next_scanline * width * 3];
        jpeg_write_scanlines(&cinfo, &row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    fclose(file);

    return 1;
}

int 
SaveWebP(
    const char* filename, 
    unsigned char* bmpData, 
    int width, 
    int height,
    int quality
) 
{
    FILE* file;
    errno_t err = fopen_s(&file, filename, "wb");

    if (err != 0 || file == NULL)
        return 0;

    uint8_t* output;
    size_t outputSize;
    
    if (quality >= 100)
        outputSize = WebPEncodeLosslessRGB(bmpData, width, height, width * 3, &output);
    else
        outputSize = WebPEncodeRGB(bmpData, width, height, width * 3, (float)quality, &output);

    if (!outputSize) 
        return 0;

    fwrite(output, outputSize, 1, file);
    fclose(file);

    free(output);

    return 1;
}

int
ProcessCustomExtension(
    int type,
    Bitmap* bmp,
    int width,
    int height,
    const char* savePath,
    int quality
)
{
    unsigned char* data = GetBitmapData(bmp, width, height);

    if (NULL == data)
        return 0;

    int rst = 0;

    switch (type)
    {
        case IMAGE_TYPE_JPG:
        {
            if (0 == SaveJPG(savePath, data, width, height, quality))
                goto ERROR_RESULT;

            break;
        }
        case IMAGE_TYPE_WEBP:
        {
            if (0 == SaveWebP(savePath, data, width, height, quality))
                goto ERROR_RESULT;

            break;
        }
    }

    rst = 1;

ERROR_RESULT:

    free(data);

    return rst;
}