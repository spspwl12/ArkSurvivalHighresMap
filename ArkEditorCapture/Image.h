#pragma once

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
    ConvertImage(
        int type,
        const char* srcPath,
        const char* dstPath
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
#ifdef __cplusplus
}
#endif