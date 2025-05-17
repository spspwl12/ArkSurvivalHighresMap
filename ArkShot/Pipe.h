#pragma once

#define PIPE_NAME                   "\\\\.\\pipe\\ArkShotPipe8a78d7dc\\connect"
#define PIPE_BUF_SIZE				1024

int
StartPipeComm(
	void* cb
);

int
StopPipeComm(
);

void 
SetDisconnectedPipeFunc(
	void* cb
);

int
SendPipeMessage(
	int size, 
	char* buf
);