#include "readline.h"

#undef RtlMoveMemory
__declspec(dllimport) void __stdcall RtlMoveMemory(void *dst, const void *src, size_t len);

READLINE* rl_new(_In_ const HANDLE handle, _In_ const DWORD buffersize)
{
	READLINE* rl = (READLINE*)HeapAlloc(GetProcessHeap(), 0, sizeof(READLINE));

	rl->handle = handle;
	rl->codepage = CP_ACP;
	rl->convertToCodepage = TRUE;
	rl->firstRead = TRUE;

	rl->bufSize = buffersize;
	rl->bufLen = 0;

	rl->readBuffer = (char*)HeapAlloc(GetProcessHeap(), 0, rl->bufSize);
	rl->lineBuffer = NULL;
	
	rl->readPos = rl->readBuffer;

	return rl;
}
void rl_delete(_Inout_ READLINE* rl)
{
	HeapFree(GetProcessHeap(), 0, rl->readBuffer);
	if (rl->lineBuffer != NULL)
	{
		HeapFree(GetProcessHeap(), 0, rl->lineBuffer);
	}
	HeapFree(GetProcessHeap(), 0, rl);
}

/*
#ifdef _DEBUG
void* memchr(const void *s, int c, size_t n)
{
	const unsigned char*  p = (const unsigned char*)s;
	const unsigned char*  end = p + n;
	for (;;) {
		if (p >= end || p[0] == c) break; p++;
		if (p >= end || p[0] == c) break; p++;
		if (p >= end || p[0] == c) break; p++;
		if (p >= end || p[0] == c) break; p++;
	}
	if (p >= end)
		return NULL;
	else
		return (void*)p;
}
#endif 
*/
static BOOL isEOF(_In_ const READLINE* rl)
{
	return rl->readPos > (rl->readBuffer + rl->bufLen - 1);
}
static int fillBuffer(READLINE* rl, _Out_ DWORD* bytesRead)
{
	int rc;

	DWORD bytesToRead = rl->bufSize - rl->bufLen;

	BOOL ok = ReadFile(
		rl->handle
		, rl->readBuffer + rl->bufLen
		, bytesToRead
		, bytesRead
		, NULL);

	if (!ok)
	{
		rc = GetLastError();
		//Log::Instance()->win32errfunc(L"ReadFile", L"fillBuffer()");
	}
	else
	{
		rl->bufLen += *bytesRead;
		rc = 0;
	}

	return rc;
}
static WCHAR* replaceNewlineWithZeroUTF16(_Inout_ WCHAR* lastchar)
{
	if (*lastchar == L'\n')
	{
		if (*(lastchar - 1) == L'\r')
		{
			--lastchar;		// set to	 \r
		}
		*lastchar = L'\0';
		--lastchar;			// set to last character
	}
	else
	{
		*(lastchar + 1) = L'\0';
	}

	return lastchar;
}
static SIZE_T calcMultibyteLenWithoutNewline(_In_ const char* firstChar, _In_ const char* lastChar)
{
	SIZE_T len;

	if (*lastChar == '\n')
	{
		if (*(lastChar - 1) == '\r')
		{
			len = (lastChar - 1) - firstChar;
		}
		else
		{
			len = (lastChar - 0) - firstChar;
		}
	}
	else
	{
		    len = (lastChar + 1) - firstChar;
	}

	return len;
}
static DWORD reportLineFromTo(_Inout_ READLINE* rl, _In_ char* lastChar, _Out_ LPWSTR* line, _Out_ DWORD* cchLen)
{
	DWORD rc = 0;

	if (rl->convertToCodepage)
	{
		int widecharsWritten;
		*line = rl->lineBuffer;

		SIZE_T cbMultiByte = calcMultibyteLenWithoutNewline(rl->readPos, lastChar);

		if (cbMultiByte == 0)
		{
			widecharsWritten = 0;
		}
		else
		{
			if ((widecharsWritten = MultiByteToWideChar(
				rl->codepage					// CodePage 
				, 0								// dwFlags
				, rl->readPos					// lpMultiByteStr
				, cbMultiByte					// cbMultiByte 
				, rl->lineBuffer				// lpWideCharStr
				, rl->bufSize					// cchWideChar 
			)) == 0)
			{
				rc = GetLastError();
				*cchLen = 0;
				//Log::Instance()->win32err(L"MultiByteToWideChar", L"reportLineFromTo()");
			}
		}
		if (rc == 0)
		{
			rl->lineBuffer[widecharsWritten] = L'\0';
			*cchLen = widecharsWritten;
		}
	}
	else
	{
		*line = (WCHAR*)(rl->readPos);
		WCHAR* lastCharW = (WCHAR*)(lastChar);
		lastCharW = replaceNewlineWithZeroUTF16(lastCharW);
		*cchLen = lastCharW - *line + 1;
	}

	return rc;
}
static void MoveRemainingDataToBeginOfBuffer(_Inout_ READLINE * rl)
{
	size_t remainingLen = rl->bufLen - (rl->readPos - rl->readBuffer);
	RtlMoveMemory(rl->readBuffer, rl->readPos, remainingLen);
	rl->readPos = rl->readBuffer;
	rl->bufLen = remainingLen;
}
static DWORD handleNoNewLine(_Inout_ READLINE* rl, _Out_ BOOL* searchNewline, _Out_ LPWSTR* line, _Out_ DWORD* cchLen)
{
	DWORD rc = 0;

	if (rl->bufLen < rl->bufSize)
	{
		// buffer is not full and we have no \n --> must be the last line WITHOUT \n 
		char *lastChar = rl->readBuffer + rl->bufLen - rl->charSize;
		rc = reportLineFromTo(rl, lastChar, line, cchLen);
		rl->readPos = rl->readBuffer + rl->bufLen;			// set readPos > len to signal the end
		*searchNewline = FALSE;
	}
	else if (rl->bufLen == rl->bufSize)
	{
		if (rl->readPos > rl->readBuffer)   
		{
			MoveRemainingDataToBeginOfBuffer(rl);
			DWORD bytesRead;
			rc = fillBuffer(rl, &bytesRead);
			*searchNewline = TRUE;
		}
		else
		{
			// no newline char AND buffer is full
			*line = NULL;
			*cchLen = 0;
			rc = ERROR_INSUFFICIENT_BUFFER;
			*searchNewline = FALSE;
		}
	}
	else
	{
		// bufLen > bufSize ==> BAD!
		rc = 998;
	}

	return rc;
}
static DWORD reportLine(_Inout_ READLINE* rl, _Out_ LPWSTR* line, _Out_ DWORD* cchLen)
{
	DWORD rc = 0;
	*cchLen = 0;
	*line = NULL;
	BOOL searchNewLine = TRUE;

	while (rc == 0 && searchNewLine)
	{
		size_t charsToSeach = (rl->readBuffer + rl->bufLen) - rl->readPos;
		char* newLineChar = memchr(rl->readPos, '\n', charsToSeach);

		if (newLineChar == NULL)
		{
			rc = handleNoNewLine(rl, &searchNewLine, line, cchLen);
		}
		else
		{
			rc = reportLineFromTo(rl, newLineChar, line, cchLen);
			rl->readPos = newLineChar + rl->charSize;
			if ( isEOF(rl) )
			{
				rl->readPos = rl->readBuffer;
				rl->bufLen = 0;
			}
			searchNewLine = FALSE;
		}
	} 

	return rc;
}
static void tryDetectBOM(_In_ const unsigned char* buf, _In_ DWORD bufLen, _Inout_ UINT* codepage, _Out_ BYTE* lenBOM, _Out_ BOOL* UTF16found)
{
	*lenBOM = 0;

	if (bufLen >= 2)
	{
		// UTF16 LE ... 0xFF 0xFE
		if (   buf[0] == 0xFF 
			&& buf[1] == 0xFE)
		{
			*lenBOM = 2;
			*UTF16found = TRUE;
		}
		else if (bufLen >= 3)
		{
			// UTF8 ... 0xEF,0xBB,0xBF
			if (buf[0] == 0xEF
				&& buf[1] == 0xBB
				&& buf[2] == 0xBF)
			{
				*codepage = CP_UTF8;
				*lenBOM = 3;
			}
		}
	}
}
static void handleFirstRead(_Inout_ READLINE* rl, _Out_ BYTE* lenBOM)
{
	BOOL UTF16found = FALSE;

	tryDetectBOM((unsigned char*)rl->readBuffer, rl->bufLen, &rl->codepage, lenBOM, &UTF16found);
	rl->readPos += *lenBOM;

	if (UTF16found)
	{
		rl->convertToCodepage = FALSE;
		rl->charSize = 2;
	}
	else
	{
		rl->charSize = 1;
		// create a buffer for line conversions
		rl->lineBuffer = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, (SIZE_T)rl->bufSize * 2);
	}
}
DWORD rl_readline(_Inout_ READLINE* rl, _Out_ LPWSTR* line, _Out_ DWORD* cchLen)
{
	DWORD rc = 0;

	if (rl->bufLen > 0 && isEOF(rl) )
	{
		// we enter ReadLine and the readPointer is past the bufLen.
		// means there is no more data.
		// the last call to readLine should return NULL for line
		*line = NULL;
		*cchLen = 0;
	}
	else
	{
		if (rl->bufLen == 0)
		{
			DWORD bytesRead;
			rc = fillBuffer(rl, &bytesRead);
			if (rc == 0)
			{
				if (rl->firstRead)
				{
					rl->firstRead = FALSE;
					BYTE lenBOM;
					handleFirstRead(rl, &lenBOM);
					if (bytesRead == lenBOM)
					{
						bytesRead = 0;	// set *line to NULL, ccLen to 0
						rl->bufLen = 0; // to skip reportLine()
					}
				}

				if (bytesRead == 0)
				{
					// the previous fillBuffer filled the buffer exactly with the last bytes of the file
					// so the last call gives us 0 bytesRead
					*line = NULL;
					*cchLen = 0;
				}
			}
		}


		if (rc == 0 && rl->bufLen > 0)
		{
			rc = reportLine(rl, line, cchLen);
		}
	}

	return rc;
}