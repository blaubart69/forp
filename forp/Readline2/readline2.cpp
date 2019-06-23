#include "readline2.h"
//
// ----------------------------------------------------------------------------
//
void tryDetectBOM(
	_In_	const unsigned char* buf,
	_In_	DWORD bufLen,
	_Inout_ UINT* codepage,
	_Out_	BYTE* lenBOM,
	_Out_	BOOL* UTF16LEfound)
{
	*lenBOM = 0;
	*UTF16LEfound = false;

	if (bufLen >= 2)
	{
		// UTF16 LE ... 0xFF 0xFE
		if (buf[0] == 0xFF
			&& buf[1] == 0xFE)
		{
			*lenBOM = 2;
			*UTF16LEfound = TRUE;
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
	_eof = false;
	_bytes_in_read_buffer = 0;
	_conv_buf_len = 0;
	_conv_start_idx = 0;
}
readline2::~readline2()
{
	delete[] _read_buffer;
	delete[] _conv_buffer;
}
DWORD readline2::fill_read_buffer()
{
	DWORD rc;
	DWORD bytesToRead = _bufsize - _bytes_in_read_buffer;
	DWORD bytesRead;

	if ( ReadFile(
		_fp
		, _read_buffer + _bytes_in_read_buffer
		, bytesToRead
		, &bytesRead
		, NULL))
	{
		rc = 0;
		_bytes_in_read_buffer += bytesRead;
		if (bytesRead == 0)
		{
			_eof = true;
		}
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
	if ( (rc = fill_read_buffer()) != 0 ) // startIdx == 0
	{
		return rc;
	}

	_codepage = CP_ACP;
	BYTE lenBOM;
	BOOL isUTF16;
	tryDetectBOM((unsigned char*)_read_buffer, _bytes_in_read_buffer, &_codepage, &lenBOM, &isUTF16);

	if (lenBOM == _bytes_in_read_buffer)
	{
		_eof = true;
		// TODO
		// 2019-06-23 Beanie
		//	not really. we would have to call ReadFile() another time to see if we get 0 BytesRead
	}
	else
	{
		if (!isUTF16)
		{
			rc = convert(lenBOM, _conv_start_idx);
		}
	}

	return rc;
}
DWORD readline2::convert(_In_ int readBufStartIdx, _In_ int convBufStartIdx)
{
	DWORD rc;
	int widecharsWritten = 0;
	DWORD bytesNotConverted = 0;

	while (bytesNotConverted < 4)
	{
		int bytesToConvert = _bytes_in_read_buffer - readBufStartIdx - bytesNotConverted;

		if (bytesToConvert == 0)
		{
			rc = 0;
			break;
		}

		if ((widecharsWritten = MultiByteToWideChar(
			_codepage							// CodePage 
			, MB_ERR_INVALID_CHARS				// dwFlags
			, _read_buffer + readBufStartIdx	// lpMultiByteStr
			, bytesToConvert					// cbMultiByte 
			, _conv_buffer + convBufStartIdx	// lpWideCharStr
			, _bufsize							// cchWideChar 
		)) == 0)     
		{
			rc = GetLastError();
			if (rc == ERROR_NO_UNICODE_TRANSLATION)
			{
				bytesNotConverted += 1;
			}
			else
			{
				break;
			}
		}
		else
		{
			rc = 0;

			_conv_buf_len			+= widecharsWritten;
			_bytes_in_read_buffer	= _bytes_in_read_buffer - (readBufStartIdx + bytesToConvert);

			if (_bytes_in_read_buffer > 0)
			{
				RtlMoveMemory(
					_read_buffer													//	dest
					, &(_read_buffer[_bytes_in_read_buffer - bytesNotConverted])	// source
					, bytesNotConverted);
			}

			break;
		}
	}

	return rc;
}
bool readline2::find_next_line(_Out_ LPWSTR & line, _Out_ DWORD & cchLen)
{
	DWORD rc = 0;

	line = _conv_buffer + _conv_start_idx;
	cchLen = 0;

	bool newLineFound = false;
	DWORD idx = _conv_start_idx;

	while ( idx < _conv_buf_len )
	{
		if (_conv_buffer[idx] == L'\n')
		{
			newLineFound = true;
			break;
		}
		++idx;
	}

	if (newLineFound)
	{
		int idxToSetZero = idx;
		if (idx > 0 && _conv_buffer[idx - 1] == L'\r')
		{
			--idxToSetZero;
		}
		_conv_buffer[idxToSetZero] = L'\0';

		cchLen = idxToSetZero - _conv_start_idx;
		_conv_start_idx = idx + 1;
	}
	else
	{
		// have we reached EOF?
		rc = fill_read_buffer();
		if (rc == 0)
		{

		}
	}

	return newLineFound;
}

DWORD readline2::next(_Out_ LPWSTR & line, _Out_ DWORD & cchLen)
{
	DWORD rc = 0;

	line = nullptr;
	cchLen = 0;

	if (_firstRead)
	{
		_firstRead = false;
		rc = first_read_and_convert();
		if (rc != 0)
		{
			return rc;
		}
		if (_eof)
		{
			return 0;
		}
	}
	
	if (_conv_start_idx >= _conv_buf_len)	// no more data in CONV buffer
	{
		if (_bytes_in_read_buffer == 0) 
		{
			return 0;
		}
		else
		{
			if (!_eof)
			{
				rc = fill_read_buffer();
			}

			if (rc == 0)
			{
				rc = convert(
					  0	//readBufStartIdx
					, 0	// convBufStartIdx
				);
			}
		}
	}

	while (true)
	{
		if (rc != 0)
		{
			break;
		}

		if (find_next_line(line, cchLen))
		{
			break;
		}
		else
		{
			if (_conv_start_idx == 0)
			{
				rc = ERROR_INSUFFICIENT_BUFFER;
				break;
			}
			//
			// fill read buffer - see if we reached EOF
			//
			if (!_eof)
			{
				rc = fill_read_buffer();
			}
			//
			// shift conv buffer down
			//
			DWORD convBytesToMove = ( _conv_buf_len - _conv_start_idx ) * 2;
			RtlMoveMemory(
				_conv_buffer,						// dest
				_conv_buffer  + _conv_start_idx,	// src
				convBytesToMove);

			_conv_start_idx = convBytesToMove;
			//
			// convert read buffer
			//
			if (rc == 0)
			{
				rc = convert(0, _conv_start_idx);
			}
		}
	}

	return rc;
}
