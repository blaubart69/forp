#pragma once

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

#undef RtlMoveMemory
extern "C" __declspec(dllimport) void __stdcall RtlMoveMemory(void *dst, const void *src, size_t len);


READLINE* rl_new(const HANDLE handle, const DWORD buffersize);
void      rl_delete(READLINE* rl);
void reportUTF16Line(LPWSTR * line, READLINE * rl, char * lastChar, DWORD * cchLen);
DWORD     rl_readline(READLINE * rl, LPWSTR * line, DWORD* cchLen);
