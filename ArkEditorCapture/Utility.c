#include <Shlobj.h>
#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#include "Utility.h"

Vec2D
GetEndCoord(
    Vec2D CoordTo,
    Vec2D CoordFrom,
    const float multiplier
)
{
    if (CoordFrom.x < CoordTo.x)
        CoordFrom.x = CoordTo.x + 1;

    if (CoordFrom.y < CoordTo.y)
        CoordFrom.y = CoordTo.y + 1;

    Vec2D CurrentCoord = CoordTo;

    for (; CurrentCoord.x <= CoordFrom.x; CurrentCoord.x += multiplier)
    {
        if ((long)CurrentCoord.x == (long)CoordFrom.x)
            break;
    }

    for (; CurrentCoord.y <= CoordFrom.y; CurrentCoord.y += multiplier)
    {
        if ((long)CurrentCoord.y == (long)CoordFrom.y)
            break;
    }

    if (CurrentCoord.x > CoordFrom.x)
        CoordFrom.x = CurrentCoord.x;

    if (CurrentCoord.y > CoordFrom.y)
        CoordFrom.y = CurrentCoord.y;

    return CoordFrom;
}

INT CALLBACK
BrowseCallbackProc(
    HWND        hWnd,
    UINT        uMsg,
    LPARAM      lParam,
    LPARAM      lpData
)
{
    if (uMsg == BFFM_INITIALIZED)
        SendMessage(hWnd, BFFM_SETSELECTION, TRUE, lpData);

    return 0;
}

int
BrowseFolder(
    void* hWndOwner,
    const char* DialogTitle,
    char* ResultPath
)
{
    LPITEMIDLIST	pidlBrowse;
    BROWSEINFO		brInfo = { 0 };

    DWORD attributes = GetFileAttributes(ResultPath);

    if (INVALID_FILE_ATTRIBUTES != attributes &&
        0 == (attributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        char* ptr = strrchr(ResultPath, '\\');

        if (ptr)
            *ptr = 0;
    }

    brInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT | BIF_VALIDATE;
    brInfo.hwndOwner = (HWND)hWndOwner;
    brInfo.pszDisplayName = ResultPath;
    brInfo.lpszTitle = DialogTitle;
    brInfo.lpfn = BrowseCallbackProc;
    brInfo.lParam = (LPARAM)ResultPath;

    if (pidlBrowse = SHBrowseForFolder(&brInfo))
    {
        SHGetPathFromIDList(pidlBrowse, ResultPath);
        return TRUE;
    }

    return FALSE;
}

int
BrowseFile(
    void* hWnd,
    char* Path,
    const char* Filter
)
{
    OPENFILENAME ofn = { 0 };
    char Buffer[MAX_PATH] = { 0 };
    char* Part;

    memcpy(Buffer, Path, sizeof(Buffer));

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = (HWND)hWnd;
    ofn.lpstrFile = Buffer;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    ofn.lpstrFilter = Filter;

    const int bRst = GetOpenFileName(&ofn);
    GetFullPathName(Buffer, MAX_PATH, Path, &Part);

    return bRst;
}

int 
IsNumeric(
    const char* str
) 
{
    while (*str) 
    {
        if (!isdigit(*str))
            return 0;

        str++;
    }

    return 1;
}