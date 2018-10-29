#pragma once

#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used 
#define STRICT
#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _readline
{
	HANDLE handle;
	UINT codepage;

	DWORD bufSize;
	DWORD bufLen;

	char* readBuffer;
	char* readPos;
	
	BOOL firstRead;
	WCHAR* lineBuffer;
	BYTE bomLength;

} READLINE;

READLINE* rl_new(const HANDLE handle, const DWORD buffersize);
void      rl_delete(READLINE* rl);
DWORD     rl_readline(READLINE * rl, LPWSTR * line, DWORD* cchLen);

static void MoveRemainingDataToBeginOfBuffer(READLINE * rl);

#ifdef __cplusplus
}
#endif
