#pragma once

#define ATOMIC_FN       1.3333333333333E-04f
#define CORRECTION      0.0000001f

typedef struct StartCaptureParameter {
    void* hProcess;
    void* funcAddr;
    void* hDlg;
    int Status;
    int quality;
}CParam;

typedef struct Vector2D {
    float x;
    float y;
} Vec2D;

int
OpenProcessProc(
    HWND hDlg
);

int
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

int 
StartCapture(
    int quality
);

void
StopCapture(
);

void
SetButtonAction(
    int mode
);

int
GetImageQuality(
    HINSTANCE hInst,
    HWND hDlg,
    int val
);