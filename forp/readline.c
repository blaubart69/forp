#include "readline.h"

#undef RtlMoveMemory
__declspec(dllimport) void __stdcall RtlMoveMemory(void *dst, const void *src, size_t len);

READLINE* rl_new(const HANDLE handle, const DWORD buffersize)
{
	READLINE* rl = (READLINE*)HeapAlloc(GetProcessHeap(), 0, sizeof(READLINE));

	rl->handle = handle;
	rl->codepage = CP_ACP;
	rl->convertToCodepage = TRUE;
	rl->firstRead = TRUE;

	rl->bufSize = buffersize;
	rl->bufLen = 0;
	rl->bomLength = 0;

	rl->readBuffer = (char*)HeapAlloc(GetProcessHeap(), 0, rl->bufSize);
	rl->lineBuffer = NULL;
	
	rl->readPos = rl->readBuffer;

	return rl;
}
void rl_delete(READLINE* rl)
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
static BOOL isEOF(const READLINE* rl)
{
	return rl->readPos > (rl->readBuffer + rl->bufLen - 1);
}
static int fillBuffer(READLINE* rl, DWORD offset, DWORD* bytesRead)
{
	int rc;

	DWORD bytesToRead = rl->bufSize - offset;

	BOOL ok = ReadFile(
		rl->handle
		, rl->readBuffer + offset
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
		rl->bufLen = offset + *bytesRead;
		rc = 0;
	}

	return rc;
}

static WCHAR* replaceNewlineWithZeroUTF16(WCHAR* lastchar)
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
static SIZE_T calcMultibyteLenWithoutNewline(const char* firstChar, const char* lastChar)
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
static DWORD reportLineFromTo(READLINE* rl, char* lastChar, LPWSTR* line, DWORD* cchLen)
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
static void MoveRemainingDataToBeginOfBuffer(READLINE * rl)
{
	size_t remainingLen = rl->bufSize - (rl->readPos - rl->readBuffer);
	RtlMoveMemory(rl->readBuffer, rl->readPos, remainingLen);
	rl->readPos = rl->readBuffer;
	rl->bufLen = remainingLen;
}
static DWORD reportLine(READLINE* rl, LPWSTR* line, DWORD* cchLen)
{
	DWORD rc = 0;
	BYTE  charSize = rl->convertToCodepage ? 1 : 2;

	while (rc == 0)
	{
		//char* newLineChar = findNewLineChar(rl->readPos, rl->readBuffer + rl->bufLen);
		size_t charsToSeach = (rl->readBuffer + rl->bufLen) - rl->readPos;
		char* newLineChar = memchr(rl->readPos, '\n', charsToSeach);

		if (newLineChar == NULL)
		{
			if (rl->bufLen >= rl->bufSize)
			{
				if (rl->readPos == (rl->readBuffer + rl->bomLength))   // readPos is at first character. skipping BOM
				{
					// no newline char AND buffer is full
					*line = NULL;
					*cchLen = 0;
					rc = ERROR_INSUFFICIENT_BUFFER;
					break;
				}
				else
				{
					MoveRemainingDataToBeginOfBuffer(rl);
					DWORD bytesRead;
					rc = fillBuffer(rl, rl->bufLen, &bytesRead);
				}
			}
			else //if (rl->bufLen < rl->bufSize)
			{
				// buffer is not full and we have no \n --> must be the last line WITHOUT \n 
				char *lastChar = rl->readBuffer + rl->bufLen - charSize;
				rc = reportLineFromTo(rl, lastChar, line, cchLen);
				rl->readPos = rl->readBuffer + rl->bufLen;			// set readPos > len to signal the end
				break;
			}
		}
		else
		{
			rc = reportLineFromTo(rl, newLineChar, line, cchLen);

			rl->readPos = newLineChar + charSize;
			if (rl->readPos > (rl->readBuffer + rl->bufSize - 1 ))		// NEW readPos is past bufSize
			{
				rl->readPos = rl->readBuffer;
				rl->bufLen = 0;			// set both to 0 to read data into the buffer on the next call
			}
			break;
		}
	} 

	return rc;
}
static void tryDetectBOM(const unsigned char* buf, DWORD bufLen, UINT* codepage, BYTE* lenBOM, BOOL* UTF16found)
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
static void handleFirstRead(READLINE* rl, DWORD firstBytesRead)
{
	BOOL UTF16found = FALSE;

	tryDetectBOM((unsigned char*)rl->readBuffer, firstBytesRead, &rl->codepage, &rl->bomLength, &UTF16found);
	rl->readPos += rl->bomLength;

	if (UTF16found)
	{
		rl->convertToCodepage = FALSE;
	}
	else
	{
		// create a buffer for line conversions
		rl->lineBuffer = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, (SIZE_T)rl->bufSize * 2);
	}
}

DWORD rl_readline(READLINE* rl, LPWSTR* line, DWORD* cchLen)
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
			rc = fillBuffer(rl, rl->bufLen, &bytesRead);
			if (rc == 0)
			{
				if (rl->firstRead)
				{
					rl->firstRead = FALSE;
					handleFirstRead(rl, bytesRead);
					if (bytesRead == rl->bomLength)
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