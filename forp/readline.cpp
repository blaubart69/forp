#include "pch.h"

READLINE* rl_new(const HANDLE handle, const DWORD buffersize)
{
	READLINE* rl = (READLINE*)HeapAlloc(GetProcessHeap(), 0, sizeof(READLINE));
	rl->handle = handle;
	rl->codepage = CP_UTF8;
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

char* findNewLineChar(char* startPos, char *endPos)
{
	char* c = startPos;

	bool found = false;
	while (c <= endPos)
	{
		if (*c == '\n')
		{
			found = true;
			break;
		}
		++c;
	}

	if (!found)
	{
		c = NULL;
	}

	return c;
}
int fillBuffer(READLINE* rl, DWORD offset, DWORD* bytesRead)
{
	int rc;

	/*
	if (rl->writePos + 1 >= rl->bufSize)
	{
		Log::Instance()->err(L"buffer full (fillBuffer) writePos [%ud] bufSize [%ud]", rl->writePos, rl->bufSize);
		return ERROR_INSUFFICIENT_BUFFER;
	}
	*/

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
		Log::Instance()->win32errfunc(L"ReadFile", L"fillBuffer()");
	}
	else
	{
		rl->bufLen = offset + *bytesRead;
		rc = 0;
	}

	return rc;
}

WCHAR* processUTF16end(WCHAR* lastchar)
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
int calcMultibyteLenWithoutNewline(const char* firstChar, const char* lastChar)
{
	int len;

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
DWORD reportLineFromTo(READLINE* rl, char* lastChar, LPWSTR* line, DWORD* cchLen)
{
	DWORD rc = 0;

	if (rl->codepage == 0) // UTF-16 LE
	{
		*line = (WCHAR*)(rl->readPos);
		WCHAR* lastCharW = (WCHAR*)(lastChar);
		lastCharW = processUTF16end(lastCharW);
		*cchLen = lastCharW - *line + 1;
	}
	else
	{
		int cbMultiByte = calcMultibyteLenWithoutNewline(rl->readPos, lastChar);

		int widecharsWritten;
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
			Log::Instance()->win32err(L"MultiByteToWideChar", L"reportLineFromTo()");
		}
		else
		{
			rl->lineBuffer[widecharsWritten] = L'\0';
			*line = rl->lineBuffer;
			*cchLen = widecharsWritten;
		}
	}

	return rc;
}
DWORD reportLine(READLINE* rl, LPWSTR* line, DWORD* cchLen)
{
	DWORD rc = 0;

	while (rc == 0)
	{
		char* newLineChar = findNewLineChar(rl->readPos, rl->readBuffer + rl->bufLen);
		if (newLineChar == NULL)
		{
			if (rl->bufLen == rl->bufSize)
			{
				if (rl->readPos == (rl->readBuffer + rl->bomLength))   // readPos is at first character. skipping BOM
				{
					// buffer is full but no newline char
					*line = NULL;
					*cchLen = 0;
					rc = ERROR_INSUFFICIENT_BUFFER;
					break;
				}
				else
				{
					size_t remainingLen = rl->bufSize - (rl->readPos - rl->readBuffer);
					RtlMoveMemory(rl->readBuffer, rl->readPos, remainingLen);
					rl->readPos = 0;
					rl->bufLen = remainingLen;
					DWORD bytesRead;
					rc = fillBuffer(rl, rl->bufLen, &bytesRead);
				}
			}
			else if (rl->bufLen < rl->bufSize)
			{
				// buffer is not full and we have no \n
				// --> must be the last line WITHOUT \n 
				int charSize = rl->codepage == 0 ? 2 : 1;
				char *lastChar = rl->readBuffer + rl->bufLen - charSize;
				rc = reportLineFromTo(rl, lastChar, line, cchLen);
				// set readPos > len to signal the end
				rl->readPos = rl->readBuffer + rl->bufLen;
				break;
			}
		}
		else
		{
			rc = reportLineFromTo(rl, newLineChar, line, cchLen);

			rl->readPos = newLineChar + ( rl->codepage == 0 ? 2 : 1 );
			if (rl->readPos > (rl->readBuffer + rl->bufSize - 1 ))		// NEW readPos is past bufSize
			{
				rl->readPos = 0;
				rl->bufLen = 0;
			}
			break;
		}
	}

	return rc;
}
void tryDetectBOM(const unsigned char* buf, DWORD bufLen, UINT* codepage, BYTE* lenBOM)
{
	BOOL found = FALSE;
	*lenBOM = 0;
	*codepage = CP_UTF8;

	if (bufLen >= 2)
	{
		// UTF16 LE ... 0xFF 0xFE
		if (   buf[0] == 0xFF 
			&& buf[1] == 0xFE)
		{
			found = TRUE;
			*codepage = 0;
			*lenBOM = 2;
		}
		else if (bufLen >= 3)
		{
			// UTF8 ... 0xEF,0xBB,0xBF
			if (buf[0] == 0xEF
				&& buf[1] == 0xBB
				&& buf[2] == 0xBF)
			{
				found = TRUE;
				*codepage = CP_UTF8;
				*lenBOM = 3;
			}
		}
	}
}
void handleFirstRead(READLINE* rl, DWORD firstBytesRead)
{
	tryDetectBOM((unsigned char*)rl->readBuffer, firstBytesRead, &rl->codepage, &rl->bomLength);
	rl->readPos += rl->bomLength;

	if (rl->codepage != 0)
	{
		rl->lineBuffer = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, rl->bufSize * 2);
	}
}
DWORD rl_readline(READLINE* rl, LPWSTR* line, DWORD* cchLen)
{
	DWORD rc = 0;

	if (rl->bufLen > 0 && (rl->readPos > (rl->readBuffer + rl->bufLen - 1)) )
	{
		// we enter ReadLine and the readPointer is past the bufLen.
		// means there is no more data.
		// the last call to readLine should return NULL for line
		*line = NULL;
		*cchLen = 0;
	}
	else
	{
		DWORD bytesRead;
		if (rl->bufLen == 0)
		{
			rc = fillBuffer(rl, rl->bufLen, &bytesRead);
			if (rc == 0)
			{
				if (rl->firstRead)
				{
					rl->firstRead = FALSE;
					handleFirstRead(rl, bytesRead);
				}
			}
		}

		if (rc == 0)
		{
			if (bytesRead == 0)
			{
				// the previous fullBuffer filled the buffer exactly with the last bytes of the file
				// so the last call gives us 0 bytesRead
				*line = NULL;
				*cchLen = 0;
			}
			else
			{
				rc = reportLine(rl, line, cchLen);
			}
		}
	}

	return rc;
}


