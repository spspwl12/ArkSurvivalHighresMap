#include "Image.h"
#include "../libwebp/webp/encode.h"
#include <windows.h>
#include <gdiplus.h>
#include <stdio.h>
#include <stdint.h>
#include <process.h>
#include <string>

#pragma comment(lib, "gdiplus.lib")

#ifdef _DEBUG
#pragma comment(lib, "..\\Build\\libwebpd.lib")
#pragma comment(lib, "..\\Build\\libjpegd.lib")
#else
#pragma comment(lib, "..\\Build\\libwebp.lib")
#pragma comment(lib, "..\\Build\\libjpeg.lib")
#endif

using namespace std;
using namespace Gdiplus;
static ULONG_PTR gdiplusToken;

struct converterThreadData
{
    HANDLE handle;
    Bitmap* bitmap;
    int type;
    int quality;
    int width;
    int height;
    int x;
    int y;
    string savePath;
    int result;

    converterThreadData() : handle(0), bitmap(0), type(0), quality(0), 
        width(0), height(0), x(0), y(0), result(0) {}

    void setData(int type, int quality, int width, 
        int height, int x, int y, const char* savePath)
    {
        this->handle = 0;
        this->bitmap = 0;
        this->result = 0;
        this->type = type;
        this->quality = quality;
        this->width = width;
        this->height = height;
        this->x = x;
        this->y = y;
        this->savePath = savePath;
    }
};

bool
ProcessImageSaving(
    int type,
    Bitmap* bmp,
    int width,
    int height,
    string savePath,
    int quality
);

bool
ProcessSaveImageParts(
    Image* img,
    converterThreadData* datas,
    int index,
    bool bSingle
);

bool
WaitThreads(
    int maxThreads,
    converterThreadData* datas
);

UINT WINAPI
_converterThread(
    LPVOID lpParam
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
    case IMAGE_TYPE_GIF: // GIF
        uuid = L"{557CF402-1A04-11D3-9A73-0000F81EF32E}";
        break;
    case IMAGE_TYPE_PNG: // PNG
        uuid = L"{557CF406-1A04-11D3-9A73-0000F81EF32E}";
        break;
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
    int quality,
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

    Image* image = nullptr;
    wchar_t* wszPath = nullptr;
    converterThreadData* thData = nullptr;
    char zx_string[MAX_PATH];
    char zxy_string[MAX_PATH];
    int rst = FALSE;
    int width, height, cntX, cntY, count;
    int coreCount;

    if (nullptr == (wszPath = new wchar_t[MAX_PATH]))
        goto ERROR_RESULT;

    if (0 == MultiByteToWideChar(CP_UTF8, 0, srcPath, -1, wszPath, MAX_PATH))
        goto ERROR_RESULT;

    if (nullptr == (image = new Image(wszPath)))
        goto ERROR_RESULT;

    width = image->GetWidth();
    height = image->GetHeight();

    cntX = width / cutSizeX;
    cntY = height / cutSizeY;

    count = cntX * cntY;
    coreCount = 0;

    SYSTEM_INFO sysInfo;

    GetSystemInfo(&sysInfo);
    coreCount = (int)sysInfo.dwNumberOfProcessors;

    if (count <= 1)
        coreCount = 1;

    if (nullptr == (thData = new converterThreadData[coreCount]))
        goto ERROR_RESULT;

    for (int i = 0, j = 0, x, y; i < count; ++i)
    {
        x = i % cntX;
        y = i / cntX;

        if (x <= maxX)
        {
            sprintf_s(zx_string, sizeof(zx_string), "%s\\%d", dstPath, startX + x);
            CreateDirectory(zx_string, NULL);

            if (y <= maxY)
            {
                sprintf_s(zxy_string, sizeof(zxy_string), "%s\\%d.%s", zx_string, startY + y, extension);

                if (GetFileAttributes(zxy_string) == ((DWORD)-1))
                {
                    thData[j].setData(type, quality, cutSizeX, cutSizeY, x, y, zxy_string);

                    if (false == ProcessSaveImageParts(image, thData, j, count == 1))
                        goto ERROR_RESULT;

                    j++;
                }
            }
        }

        if (j > 0 && count > 1 && (coreCount <= j || count <= i + 1))
        {
            if (false == WaitThreads(coreCount, thData))
                goto ERROR_RESULT;

            j = 0;
        }
    }

    rst = TRUE;

ERROR_RESULT:

    if (nullptr != thData)
    {
        for (int i = 0; i < coreCount; ++i)
        {
            if (NULL != thData[i].handle)
                CloseHandle(thData[i].handle);

            if (NULL != thData[i].bitmap)
                delete thData[i].bitmap;
        }

        delete[] thData;
    }

    if (nullptr != wszPath)
        delete[] wszPath;

    if (nullptr != image)
        delete image;

    return TRUE == rst;
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

bool
ProcessImageSaving(
    int type,
    Bitmap* bmp,
    int width,
    int height,
    string savePath,
    int quality
)
{
    unsigned char* rgbData = 0;
    bool rst = false;

    switch (type)
    {
        case IMAGE_TYPE_BMP:
        case IMAGE_TYPE_PNG:
        case IMAGE_TYPE_GIF:
        {
            CLSID encoderClsid = GetClsid(type);

            if (0 == encoderClsid.Data1)
                return FALSE;

            wchar_t* wszPath;

            if (nullptr == (wszPath = new wchar_t[MAX_PATH]))
                return FALSE;

            if (0 == MultiByteToWideChar(CP_UTF8, 0, savePath.c_str(), -1, wszPath, MAX_PATH))
            {
                delete[] wszPath;
                return FALSE;
            }

            if (Ok != bmp->Save(wszPath, &encoderClsid, NULL))
            {
                delete[] wszPath;
                return FALSE;
            }

            delete[] wszPath;
            break;
        }
        case IMAGE_TYPE_JPG:
        {
            if (NULL == (rgbData = GetBitmapData(bmp, width, height)))
                return FALSE;

            if (0 == SaveJPG(savePath.c_str(), rgbData, width, height, quality))
                goto ERROR_RESULT;

            break;
        }
        case IMAGE_TYPE_WEBP:
        {
            if (NULL == (rgbData = GetBitmapData(bmp, width, height)))
                return FALSE;

            if (0 == SaveWebP(savePath.c_str(), rgbData, width, height, quality))
                goto ERROR_RESULT;

            break;
        }
    }

    rst = true;

ERROR_RESULT:

    if (NULL != rgbData)
        free(rgbData);

    return rst;
}

bool 
ProcessSaveImageParts(
    Image* img,
    converterThreadData* datas,
    int index,
    bool bSingle
)
{
    Bitmap* parts = nullptr;
    Graphics* graphics = nullptr;
    bool rst = false;

    if (nullptr == (parts = new Bitmap(datas->width, datas->height, PixelFormat24bppRGB)))
        goto ERROR_RESULT;

    if (nullptr == (graphics = new Graphics(parts)))
        goto ERROR_RESULT;

    if (bSingle)
    {
        if (Ok != graphics->DrawImage(
            img,
            0, 0,
            0, 0,
            datas->width, datas->height, UnitPixel))
            goto ERROR_RESULT;

        if (false == ProcessImageSaving(
            datas->type,
            parts, 
            datas->width, datas->height,
            datas->savePath,
            datas->quality))
            goto ERROR_RESULT;
    }
    else
    {
        converterThreadData& data = datas[index];

        if (Ok != graphics->DrawImage(
            img,
            0, 0,
            data.width * data.x,
            data.height * data.y,
            data.width, data.height, UnitPixel))
            goto ERROR_RESULT;

        data.bitmap = parts;

        if (NULL == (data.handle = (HANDLE)_beginthreadex(NULL, 0, _converterThread, &data, 0, NULL)))
            goto ERROR_RESULT;
    }

    rst = true;

ERROR_RESULT:

    if (nullptr != graphics)
        delete graphics;

    if (bSingle && nullptr != parts)
        delete parts;

    return rst;
}

bool 
WaitThreads(
    int maxThreads,
    converterThreadData* datas
)
{
    HANDLE* h = new HANDLE[maxThreads];

    if (nullptr == h)
        return false;

    bool rst = true;
    int max = maxThreads;

    for (int i = 0; i < maxThreads; ++i)
    {
        if (NULL == (h[i] = datas[i].handle))
        {
            max = i;
            break;
        }
    }

    if (max <= 0)
    {
        delete[] h;
        return false;
    }

    WaitForMultipleObjects(max, h, TRUE, INFINITE);

    for (int i = 0; i < max; ++i)
    {
        CloseHandle(h[i]);
        datas[i].handle = 0;

        if (nullptr != datas[i].bitmap)
        {
            delete datas[i].bitmap;
            datas[i].bitmap = nullptr;
        }

        if (0 == datas[i].result)
            rst = false;
    }

    delete[] h;
    return rst;
}

UINT WINAPI
_converterThread(
    LPVOID lpParam
)
{
    converterThreadData* data = (converterThreadData*)lpParam;

    data->result = ProcessImageSaving(
        data->type,
        data->bitmap,
        data->width,
        data->height,
        data->savePath,
        data->quality);

    return FALSE;
}