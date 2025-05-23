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
    int quality,
    int cutSize,
    int startX,
    int startY,
    int maxX,
    int maxY,
    const char* extension,
    const char* srcPath,
    const char* dstPath
);

int
DrawImagehDC(
    void* hdc,
    const char* path,
    int x,
    int y
);
#ifdef __cplusplus
}
#endif