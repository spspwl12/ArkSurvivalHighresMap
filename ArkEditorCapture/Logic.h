#pragma once

#define ATOMIC_FN               1.3333333333333E-04F
#define SAVETILESIZE            256

// MODE
#define LOGIC_MODE_CAPTURE      1
#define LOGIC_MODE_PREVIEW      2

#define PREVIEW_PREFIX          "\\.__preview"

typedef struct StartCaptureParameter {
    void* hProcess;
    void* funcAddr;
    void* hDlg;
    int Status;
    int quality;
    int currentMode;
    int UEVersion;
}CParam;

int
OpenArkEditor(
    void* _hDlg
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
QueryEngineVariable(
);

int 
StartCapture(
    int quality,
    int mode,
    void* endcb
);

void
DisableControls(
);

void
EnableControls(
);

void
StopCapture(
);

void
SetButtonAction(
    int mode
);

void
FetchRegistryValue(
    void* hDlg
);

void
StoreRegistryValue(
    void* hDlg
);