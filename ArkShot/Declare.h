#pragma once

#define TRUE                    1
#define FALSE                   0

#define MODE_GET_XYZ			0
#define MODE_GET_ZOOM			1
#define MODE_GET_WH				2
#define MODE_GET_PATH			3
#define MODE_GET_ACTORS			4
#define MODE_SET_XY				5
#define MODE_SET_ZOOM			6
#define MODE_SET_VER			7
#define MODE_SET_XYZZ_CAPTURE	8

#define ERROR_RESULT_PIPE		255

typedef struct TArray
{
    void* AllocatorInstance;
    signed int  ArrayNum;
    signed int  ArrayMax;
}TArray;

typedef struct Vector2D {
    float x;
    float y;
} Vec2D;

typedef struct Vector3D {
    float x;
    float y;
    float z;
} Vec3D;

typedef struct Vector4D {
    float x;
    float y;
    float z;
    float zoom;
} Vec4D;

typedef struct Vector5D {
    float x;
    float y;
    float z;
    float zoom;
    float resval;
} Vec5D;

typedef struct Vector2DD {
    double x;
    double y;
} Vec2DD;

typedef struct Vector3DD {
    double x;
    double y;
    double z;
} Vec3DD;

typedef struct Vector4DD {
    double x;
    double y;
    double z;
    float zoom;
} Vec4DD;

typedef struct Vector5DD {
    double x;
    double y;
    double z;
    float zoom;
    float resval;
} Vec5DD;