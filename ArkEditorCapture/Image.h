#pragma once

#define IMAGE_TYPE_BMP      1
#define IMAGE_TYPE_JPG      2
#define IMAGE_TYPE_GIF      3
#define IMAGE_TYPE_PNG      4
#define IMAGE_TYPE_WEBP     5

#ifdef __cplusplus
extern "C" {
#endif
    int
    InitImage(
    );

    void
    CloseImage(
    );

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
    );

    int
    SaveWebP(
        const char* filename,
        unsigned char* bmpData,
        int width,
        int height,
        int quality
    );
#ifdef __cplusplus
}
#endif