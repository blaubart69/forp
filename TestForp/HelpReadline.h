#pragma once

class HelpTempFile
{

private:
	HANDLE hFile;
	WCHAR tempFilename[MAX_PATH];

public:

	HelpTempFile()
	{
		GetTempFileNameW(
			L"c:\\temp\\rl"
			, L"RL_"
			, 42
			, this->tempFilename
		);

		hFile =
			CreateFile(
				this->tempFilename
				, GENERIC_WRITE
				, FILE_SHARE_READ
				, NULL
				, CREATE_ALWAYS
				, FILE_ATTRIBUTE_TEMPORARY
				, NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			int rc = GetLastError();
			throw new std::exception("CreateFile", rc);
		}
	}
	~HelpTempFile()
	{
		CloseHandle(this->hFile);
	}

	void WriteContentW(LPCWSTR stuff)
	{
		DWORD written;
		WriteFile(
			this->hFile
			, stuff
			, lstrlen(stuff)
			, &written
			, NULL
		);
	}
	void WriteContentA(LPCSTR stuff)
	{
		DWORD written;
		WriteFile(
			this->hFile
			, stuff
			, lstrlenA(stuff)
			, &written
			, NULL
		);
	}
	void WriteUTF8BOM()
	{
		// UTF8 ... 0xEF,0xBB,0xBF
		char BOM[3] = { 0xEF,0xBB,0xBF };
		WriteBuff(BOM, 3);
	}
	void WriteUTF16LEBOM()
	{
		char BOM[2] = { 0xFF, 0xFE };
		WriteBuff(BOM, 2);
	}
	void WriteBuff(char* buf, int cbLen)
	{
		DWORD written;
		WriteFile(
			this->hFile
			, buf
			, cbLen
			, &written
			, NULL
		);
	}

	HANDLE OpenTempfile()
	{
		HANDLE handle =
			CreateFileW(
				this->tempFilename
				, GENERIC_READ
				, FILE_SHARE_WRITE
				, NULL
				, OPEN_EXISTING
				, FILE_ATTRIBUTE_TEMPORARY
				, NULL);

		if (handle == INVALID_HANDLE_VALUE)
		{
			int rc = GetLastError();
			throw new std::exception("OpenTempfile", rc);
		}

		return handle;
	}
};