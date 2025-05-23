#pragma once

float 
GetDlgItemFloat(
    void* hDlg,
    int nIDDlgItem
);

void 
SetDlgItemFloat(
    void* hDlg,
    int nIDDlgItem,
    float value
);

void
EnableAllControls(
    void* hDlg,
    int startID,
    int endID,
    int bEnable
);