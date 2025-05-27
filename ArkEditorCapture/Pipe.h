#pragma once

#define PIPE_NAME                   "\\\\.\\pipe\\ArkShotPipe8a78d7dc\\connect"
#define PIPE_BUF_SIZE				1024

char 
StartPipeComm(
);

char 
StopPipeComm(
);

char
RecvPipeMessage(
    int size, 
    char* buf
);

char 
SendPipeMessage(
    int size, 
    char* buf
);

int
SetMessage(
    char mode,
    int size,
    void* buf
);

int
RequestMessage(
    char mode,
    int size,
    void* buf
);