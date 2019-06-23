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
	DWORD		_bytes_in_read_buffer;
	//DWORD		_read_buf_len;

	bool		_eof;

	LPWSTR		_conv_buffer;
	DWORD		_conv_start_idx;
	DWORD		_conv_buf_len;

	DWORD fill_read_buffer();
	DWORD convert(_In_ int byteBufStartIdx, _In_ int convStartIdx);
	DWORD first_read_and_convert();
	bool find_next_line(_Out_ LPWSTR& line, _Out_ DWORD & cchLen);
};