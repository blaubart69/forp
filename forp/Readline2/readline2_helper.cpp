#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used 
#define STRICT
#include <windows.h>

void tryDetectBOM(
	_In_ const unsigned char* buf,
	_In_ DWORD bufLen,
	_Inout_ UINT* codepage,
	_Out_ BYTE* lenBOM,
	_Out_ BOOL* UTF16found)
{
	*lenBOM = 0;

	if (bufLen >= 2)
	{
		// UTF16 LE ... 0xFF 0xFE
		if (buf[0] == 0xFF
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