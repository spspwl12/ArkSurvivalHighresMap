#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#include <ole2.h> 
#include <gdiplus.h>
#include <stdio.h>
#include <stdint.h>
#include <process.h>
#include <string>

#include "Image.h"
#include "../libwebp/webp/encode.h"

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

    // JPG는 GDI+ 에서 지원하지만, 분석 결과 압축 효율이 서드파티 JPG 모듈보다 안 좋으므로 외부 라이브러리로 대체한다.
    // GDI+ supports JPG, but since its compression efficiency is lower than that of third-party JPG modules, 
    // I’ve decided to replace it with an external library.

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
        // 나머지 포맷은 별도로 처리한다.
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
    // 출처 : Copilot 
    // FF0000 -> 0000FF, 123456 -> 563412 이렇게 바꿔주는 함수
    // A function that rearranges FF0000 to 0000FF and 123456 to 563412.

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
    // 출처 : Copilot 
    // Gdiplus::Bitmap 클래스에 있는 데이터를 RGB 데이터로 변환하는 함수
    // A function that extracts RGB data from a Gdiplus::Bitmap object.

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
    int cutSize,
    int startX,
    int startY,
    int maxX,
    int maxY,
    const char* extension,
    const char* srcPath,
    const char* dstPath
) 
{
    // BMP, PNG 파일을 설정한 포맷에 맞게 변환해 주는 함수
    // A function that transforms BMP and PNG files into a designated format.

    if (0 == gdiplusToken && FALSE == InitImage())
        return FALSE;

    converterThreadData* thData = nullptr;
    Image* image = nullptr;
    wchar_t* wszPath = nullptr;
    char zx_string[MAX_PATH];
    char zxy_string[MAX_PATH];
    int rst = FALSE;
    int width, height, cntX, cntY, count;
    int coreCount;

    // GDI+ 함수에 들어가는 문자열은 Unicode라서 ansi 기반인 프로그램에서는 전부 다 변환을 해줘야 한다.
    // GDI+ functions use Unicode strings, so conversion is required in ANSI-based programs.
    if (nullptr == (wszPath = new wchar_t[MAX_PATH]))
        goto ERROR_RESULT;

    if (0 == MultiByteToWideChar(CP_UTF8, 0, srcPath, -1, wszPath, MAX_PATH))
        goto ERROR_RESULT;

    if (nullptr == (image = new Image(wszPath)))
        goto ERROR_RESULT;

    width = image->GetWidth();
    height = image->GetHeight();

    cntX = width / cutSize;
    cntY = height / cutSize;

    count = cntX * cntY;
    coreCount = 0;

    SYSTEM_INFO sysInfo;

    // 시스템 정보를 불러와 컴퓨터의 스레드 개수를 구한다.
    // Fetch system information to obtain the computer's thread count.
    GetSystemInfo(&sysInfo);
    coreCount = (int)sysInfo.dwNumberOfProcessors;

    // 잘라야 할 이미지 개수가 1개 이하면 멀티 스레드의 의미가 없어지니까 단일 스레드로 처리하도록 유도한다.
    // If there is only one image or none to be cropped, multi-threading loses its purpose, 
    // so the process is optimized for single-thread execution.
    if (count <= 1)
        coreCount = 1;

    // 각각의 스레드에 보낼 정보를 할당한다.
    // Distribute the necessary data to individual threads.
    if (nullptr == (thData = new converterThreadData[coreCount]))
        goto ERROR_RESULT;

    for (int i = 0, j = 0, x, y; i < count; ++i)
    {
        // 멀티 스레드 구조에 맞게 이중 포문을 쓰지 않는다.
        // Avoid using nested loops to align with the multi-threaded structure.
        x = i % cntX;
        y = i / cntX;

        if (x <= maxX)
        {
            sprintf_s(zx_string, sizeof(zx_string), "%s\\%d", dstPath, startX + x);
            CreateDirectory(zx_string, NULL);

            if (y <= maxY)
            {
                sprintf_s(zxy_string, sizeof(zxy_string), "%s\\%d.%s", zx_string, startY + y, extension);

                // 파일이 없을 때 ( 파일이 있으면 변환하지 않는다. )
                // When the file is missing ( it is not converted if the file exists )
                if (GetFileAttributes(zxy_string) == ((DWORD)-1))
                {
                    thData[j].setData(type, quality, cutSize, cutSize, x, y, zxy_string);

                    // 변환하는 함수를 호출한다.
                    // Call the function that performs image conversion.
                    if (false == ProcessSaveImageParts(image, thData, j, count == 1))
                        goto ERROR_RESULT;

                    j++;
                }
            }
        }

        // 멀티 스레드를 사용 중이라면, 스레드가 끝날 때까지 기다린다.
        // If using multiple threads, wait until all threads have finished
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
            // 3개의 이미지 포맷은 압축률이 실제 사용에 지장을 일으킬 정도는 아니라고 판단해, 그대로 처리한다.
            // Since the compression rate of the three image formats is not disruptive to practical use, 
            // they are handled without modification.

            CLSID encoderClsid = GetClsid(type);

            if (0 == encoderClsid.Data1)
                return FALSE;

            wchar_t* wszPath;

            // GDI+ 함수에 들어가는 문자열은 Unicode라서 ansi 기반인 프로그램에서는 전부 다 변환을 해줘야 한다.
            // GDI+ functions use Unicode strings, so conversion is required in ANSI-based programs.
            if (nullptr == (wszPath = new wchar_t[MAX_PATH]))
                return FALSE;

            if (0 == MultiByteToWideChar(CP_UTF8, 0, savePath.c_str(), -1, wszPath, MAX_PATH))
            {
                delete[] wszPath;
                return FALSE;
            }

            // GDI+ 모듈을 이용해 이미지 저장을 한다.
            // Save images using the GDI+ module.
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
            // 별도의 외부 라이브러리를 이용해 JPG 변환을 위해, RGB 데이터를 준비한다.
            // Prepare RGB data for JPG conversion using a separate external library.
            if (NULL == (rgbData = GetBitmapData(bmp, width, height)))
                return FALSE;

            if (0 == SaveJPG(savePath.c_str(), rgbData, width, height, quality))
                goto ERROR_RESULT;

            break;
        }
        case IMAGE_TYPE_WEBP:
        {
            // 별도의 외부 라이브러리를 이용해 WEBP 변환을 위해, RGB 데이터를 준비한다.
            // Prepare RGB data for WEBP conversion using a separate external library.
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

    // 어차피 모든 스레드의 width, height의 수치는 똑같으니까 첫 번째 배열의 값을 불러와도 무방하다.
    // Since the width and height values are the same for all threads, it is fine to retrieve the values from the first array.
    if (nullptr == (parts = new Bitmap(datas->width, datas->height, PixelFormat24bppRGB)))
        goto ERROR_RESULT;

    if (nullptr == (graphics = new Graphics(parts)))
        goto ERROR_RESULT;

    if (bSingle)
    {
        // 단일 스레드일 경우 이미지를 한 개만 처리한다.
        // If using a single thread, only one image is processed.
        if (Ok != graphics->DrawImage(
            img,
            0, 0,
            0, 0,
            datas->width, datas->height, UnitPixel))
            goto ERROR_RESULT;

        // 실제 이미지를 처리하는 함수를 호출해, 이미지를 저장한다.
        // Call the function that processes the actual image and save it.
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

        // 스레드를 CPU 스레드 수만큼 만들어서 효율적으로 변환하도록 한다.
        // Spawn threads based on the CPU's thread count for efficient processing.
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

int
DrawImagehDC(
    void* hdc,
    const char* path,
    int x,
    int y
)
{
    Graphics graphics((HDC)hdc);
    Status status = graphics.GetLastStatus();

    if (Ok != status)
        return FALSE;

    wchar_t* wszPath;

    if (nullptr == (wszPath = new wchar_t[MAX_PATH]))
        return FALSE;

    if (0 == MultiByteToWideChar(CP_UTF8, 0, path, -1, wszPath, MAX_PATH))
    {
        delete[] wszPath;
        return FALSE;
    }

    Image* img = new Image(wszPath);

    if (nullptr == img)
    {
        delete[] wszPath;
        return FALSE;
    }

    if (Ok != graphics.DrawImage(img, x, y))
    {
        delete img;
        delete[] wszPath;
        return FALSE;
    }

    delete img;
    delete[] wszPath;

    return TRUE;
}