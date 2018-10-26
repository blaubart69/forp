#pragma once

typedef struct _readline
{
	HANDLE handle;
	UINT codepage;
	DWORD bufSize;
	char* readBuffer;
	char* readPos;
	char* writePos;
	BOOL firstRead;
	WCHAR* lineBuffer;

} READLINE;

READLINE* rl_new(const HANDLE handle, const DWORD buffersize);
void rl_delete(READLINE* rl);
DWORD rl_readline(READLINE * rl, LPWSTR * line, DWORD* cchLen);
