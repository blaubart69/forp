#include "pch.h"

READLINE* rl_new(const HANDLE handle, const DWORD buffersize)
{
	READLINE* rl = (READLINE*)HeapAlloc(GetProcessHeap(), 0, sizeof(READLINE));
	rl->handle = handle;
	rl->bufSize = buffersize;
	rl->codepage = CP_UTF8;
	rl->firstRead = TRUE;
	rl->readBuffer = (char*)HeapAlloc(GetProcessHeap(), 0, rl->bufSize);
	rl->lineBuffer = NULL;
	rl->readPos = rl->writePos = rl->readBuffer;

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
void tryDetectBOM(const char* buf, DWORD bufLen, UINT* codepage, DWORD* lenBOM)
{
	BOOL found = FALSE;
	*lenBOM = 0;
	*codepage = CP_UTF8;

	if (bufLen >= 2)
	{
		// UTF16 LE ... 0xFF 0xFE
		if (buf[0] == 0xFF
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
bool hasDataInBuffer(READLINE* rl)
{
	return
		rl->readPos < rl->writePos;
}
char* findNewLineChar(READLINE* rl)
{
	char* c			= rl->readPos;
	char* bufferEnd = rl->readBuffer + rl->bufSize;

	while (c <= bufferEnd)
	{
		if (*c == '\n')
		{
			break;
		}
		++c;
	}

	return c;
}
int fillBuffer(READLINE* rl, DWORD* bytesRead)
{
	int rc;

	/*
	if (rl->writePos + 1 >= rl->bufSize)
	{
		Log::Instance()->err(L"buffer full (fillBuffer) writePos [%ud] bufSize [%ud]", rl->writePos, rl->bufSize);
		return ERROR_INSUFFICIENT_BUFFER;
	}
	*/

	DWORD bytesToRead = ( rl->readBuffer + rl->bufSize ) - rl->writePos;

	BOOL ok = ReadFile(
		rl->handle
		, rl->writePos
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
		rc = 0;
		rl->writePos += *bytesRead;
	}

	return rc;
}
DWORD reportLineFromTo(READLINE* rl, char* newLineChar, LPWSTR* line, DWORD* cchLen)
{
	DWORD rc = 0;

	if (rl->codepage == 0) // UTF-16 LE
	{
		*line = (WCHAR*)(rl->readPos);

		WCHAR* newLineW = (WCHAR*)(newLineChar - 1);
		--newLineW;				// back to	 \r
		*newLineW = L'\0';		// overwrite \r
		--newLineW;				// back to   last char
		*cchLen = (newLineW - *line) / 2;
	}
	else
	{
		int cbMultiByte = (newLineChar - 1) - rl->readPos;

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

	if (rl->readPos == rl->writePos)
	{
		// report "no more lines"
		*line = NULL;
		*cchLen = 0;
	}
	else
	{
		char* newLineChar = findNewLineChar(rl);
		if (newLineChar == NULL)
		{
			if (rl->readPos == 0)
			{
				rc = ERROR_INSUFFICIENT_BUFFER;
			}
			else
			{
				// load more data ???
			}
		}
		else
		{
			rc = reportLineFromTo(rl, newLineChar, line, cchLen);
		}
	}

	return rc;
}
/*
 * ERROR_INSUFFICIENT_BUFFER
 */
DWORD rl_readline(READLINE* rl, LPWSTR* line, DWORD* cchLen)
{
	DWORD rc = 0;

	if (hasDataInBuffer(rl))
	{
		rc = reportLine(rl, line, cchLen);
	}
	else
	{
		DWORD bytesRead;
		rc = fillBuffer(rl, &bytesRead);
		if (rc == 0)
		{
			if (rl->firstRead)
			{
				rl->firstRead = FALSE;
				DWORD lenBOM;
				tryDetectBOM(rl->readBuffer, bytesRead, &rl->codepage, &lenBOM);
				rl->readPos += lenBOM;

				if (rl->codepage != 0)
				{
					rl->lineBuffer = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, rl->bufSize * 2);
				}
			}

			reportLine(rl, line, cchLen);
		}
	}

	return rc;
}


