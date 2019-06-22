#pragma once

#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used 
#define STRICT
#include <windows.h>

class readline2
{
public:
	readline2(const HANDLE fp, const int bufsize);
	virtual ~readline2();

	DWORD next(_Out_ LPWSTR& line, _Out_ DWORD & cchLen);

private:
	const	HANDLE	_fp;
	const	int		_bufsize;

	UINT		_codepage;
	BOOL		_firstRead;

	char*		_read_buffer;
	LPWSTR		_conv_buffer;

	int			_conv_start_idx;
	
	size_t		_read_buf_len;
	size_t		_conv_buf_len;

	bool		_eof;

	DWORD fill_read_buffer(int startIdx);
	DWORD conv_buffer_to_wchar(_In_ int startIdx, _Out_ int* bytesConverted);
	DWORD first_read_and_convert();
	DWORD convert_and_read(const int startIdx);
	bool report_next_line(_Out_ LPWSTR& line, _Out_ DWORD & cchLen);
};