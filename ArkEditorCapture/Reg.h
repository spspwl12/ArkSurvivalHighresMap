#pragma once

#define REG_PATH    "ArkEditorCapture"

int 
ReadReg(
	const char* path, 
	const char* key, 
	const char* defaultVal,
	char* buf
);

int 
WriteReg(
	const char* path,
	const char* key, 
	char* value
);

int
LoadVal(
	const char* name,
	const char* initVal
);
int
SaveVal(
	const char* name,
	int val
);