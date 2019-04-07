#include "readline2.h"
//
// ----------------------------------------------------------------------------
//
void tryDetectBOM(
	_In_ const unsigned char* buf,
	_In_ DWORD bufLen,
	_Inout_ UINT* codepage,
	_Out_ BYTE* lenBOM,
	_Out_ BOOL* UTF16found);

#undef RtlMoveMemory
extern "C" __declspec(dllimport) void __stdcall RtlMoveMemory(void *dst, const void *src, size_t len);
//
// ----------------------------------------------------------------------------
//
readline2::readline2(const HANDLE fp, const int bufsize)
	: _fp(fp), _bufsize(bufsize)
{
	_read_buffer = new char[bufsize];
	_conv_buffer = new WCHAR[bufsize];

	_firstRead = true;
	
}
readline2::~readline2()
{
	delete[] _read_buffer;
	delete[] _conv_buffer;
}
DWORD readline2::fill_read_buffer(_In_ int startIdx)
{
	DWORD bytesToRead = _bufsize - startIdx;
	DWORD bytesRead;

	BOOL ok = ReadFile(
		_fp
		, _read_buffer + startIdx
		, bytesToRead
		, &bytesRead
		, NULL);

	int rc;
	if (ok)
	{
		_read_buf_len = startIdx + bytesRead;
		rc = 0;
	}
	else
	{
		rc = GetLastError();
	}

	return rc;
}
DWORD readline2::first_read_and_convert()
{
	DWORD rc;
	rc = fill_read_buffer(0);
	if (rc != 0)
	{
		return rc;
	}

	_codepage = CP_ACP;
	BYTE lenBOM;
	BOOL isUTF16;
	tryDetectBOM((unsigned char*)_read_buffer, _read_buf_len, &_codepage, &lenBOM, &isUTF16);

	if (!isUTF16)
	{
		rc = convert_and_read_again(lenBOM);
	}

	return rc;
}
DWORD readline2::conv_buffer_to_wchar(_In_ int startIdx, _Out_ int* bytesConverted)
{
	DWORD rc;
	int widecharsWritten = 0;
	int retry = 0;
	_conv_start_idx = 0;
	_conv_buf_len = 0;
	*bytesConverted = 0;

	while (retry < 4)
	{
		int byteToConvert = _read_buf_len - startIdx - retry;

		if (byteToConvert == 0)
		{
			rc = 0;
			break;
		}

		if ((widecharsWritten = MultiByteToWideChar(
			_codepage							// CodePage 
			, MB_ERR_INVALID_CHARS				// dwFlags
			, _read_buffer + startIdx			// lpMultiByteStr
			, byteToConvert						// cbMultiByte 
			, _conv_buffer						// lpWideCharStr
			, _bufsize							// cchWideChar 
		)) == 0)     
		{
			rc = GetLastError();
			if (rc == ERROR_NO_UNICODE_TRANSLATION)
			{
				retry += 1;
			}
			else
			{
				break;
			}
		}
		else
		{
			rc = 0;
			_conv_buf_len = widecharsWritten;
			*bytesConverted = byteToConvert;
			break;
		}
	}

	return rc;
}
DWORD readline2::convert_and_read_again(_In_ const int startIdx)
{
	DWORD rc;

	int bytesConverted;
	rc = conv_buffer_to_wchar(startIdx, &bytesConverted);
	if (rc != 0)
	{
		return rc;
	}

	size_t bytesRemainingInReadBuffer = _read_buf_len - (startIdx + bytesConverted);
	if (bytesRemainingInReadBuffer > 0)
	{
		RtlMoveMemory(
			_read_buffer													//	dest
			, &(_read_buffer[_read_buf_len - bytesRemainingInReadBuffer])	// source
			, bytesRemainingInReadBuffer);
	}

	rc = fill_read_buffer(bytesRemainingInReadBuffer);

	return rc;
}
BOOL readline2::eof()
{
	return
			_conv_start_idx > _conv_buf_len
		&&	_read_buf_len == 0;
}
void readline2::report_next_line(_Out_ LPWSTR & line, _Out_ DWORD & cchLen)
{
	line = _conv_buffer + _conv_start_idx;
	cchLen = 0;

	int idx = _conv_start_idx;
	while ( idx < _conv_buf_len && _conv_buffer[idx] != L'\n' )
	{
		++idx;
	}

	if (_conv_buffer[idx] == L'\n')
	{
		if (idx > 0 && _conv_buffer[idx - 1] == L'\r')
		{
			_conv_buffer[idx - 1] = L'\0';
			cchLen = idx - _conv_start_idx - 1;
		}
		else
		{
			_conv_buffer[idx] = L'\0';
			cchLen = idx - _conv_start_idx;
		}
	}
	else
	{
		// TODO: might be 1 behind _conv_buffer
		_conv_buffer[idx] = L'\0';			
		cchLen = idx - _conv_start_idx;
	}

	_conv_start_idx = idx + 1;
}
DWORD readline2::next(_Out_ LPWSTR & line, _Out_ DWORD & cchLen)
{
	DWORD rc = 0;

	if (_firstRead)
	{
		_firstRead = false;
		rc = first_read_and_convert();
		if (rc != 0)
		{
			return rc;
		}
	}

	if (_conv_start_idx < _conv_buf_len)
	{
		report_next_line(line, cchLen);
	}
	else if (_read_buf_len == 0)
	{
		line = nullptr;
		cchLen = 0;
	}
	else
	{
		rc = convert_and_read_again(0);
		if (rc == 0)
		{
			report_next_line(line, cchLen);
		}
	}

	return rc;
}
