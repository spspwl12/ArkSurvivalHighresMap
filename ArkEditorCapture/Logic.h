#pragma once

#define ATOMIC_FN       1.3333333333333E-04f
#define CORRECTION      0.0000001f

typedef struct StartCaptureParameter {
    void* hProcess;
    void* funcAddr;
    void* hDlg;
    int Status;
}CParam;

typedef struct Vector2D {
    float x;
    float y;
} Vec2D;

char
LoadArkEditor(
    void* hDlg,
    unsigned long pID,
    char* dllPath
);

void
CloseArkEditor(
);

void
LoadValue(
);

char 
StartCapture(
);

void
StopCapture(
);

void
SetButtonAction(
    int mode
);