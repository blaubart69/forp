#pragma once
void tryDetectBOM(
	_In_	const unsigned char* buf,
	_In_	DWORD bufLen,
	_Inout_ UINT* codepage,
	_Out_	BYTE* lenBOM,
	_Out_	BOOL* UTF16LEfound);

