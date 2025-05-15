#include <stdio.h>

char 
saveTextToFile(
    const char* filename,
    const char* text,
    const int len
)
{
    FILE* fp = NULL;
    errno_t err = fopen_s(&fp, filename, "w");

    if (err != 0 || fp == NULL)
        return 0;

    fwrite(text, sizeof(char), len, fp);
    fclose(fp);

    return 1;
}