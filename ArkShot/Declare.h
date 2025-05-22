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

//#define MODE_CAPTURE_RELEASE	8

#define ERROR_RESULT_PIPE		255

typedef struct Vector2D {
    float x;
    float y;
} Vec2D;

typedef struct FVector {
    float x;
    float y;
    float z;
} FVector;

typedef struct FVectorD {
    double x;
    double y;
    double z;
} FVectorD;
