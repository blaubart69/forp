#pragma once

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

class HelpTempFile
{

private:
	HANDLE hFile;
	HANDLE hReadHandle;

	bool utf16bomwritten = false;

	HANDLE OpenTempfileForReading(LPCWSTR tempFilename)
	{
		HANDLE handle =
			CreateFileW(
				tempFilename
				, GENERIC_READ
				, FILE_SHARE_WRITE
				, NULL
				, OPEN_EXISTING
				, FILE_ATTRIBUTE_TEMPORARY
				, NULL);

		if (handle == INVALID_HANDLE_VALUE)
		{
			int rc = GetLastError();
			WCHAR err[128];
			wsprintf(err, L"OpenTempfile() rc: %d", rc);
			Assert::Fail(err);
		}

		return handle;
	}

public:

	HelpTempFile(UINT unique)
	{
		WCHAR tempFilename[MAX_PATH];

		GetTempFileNameW(
			L"c:\\temp\\rl"
			, L"RL_"
			, unique
			, tempFilename
		);

		hFile =
			CreateFile(
				tempFilename
				, GENERIC_WRITE
				, FILE_SHARE_READ
				, NULL
				, CREATE_ALWAYS
				, FILE_ATTRIBUTE_TEMPORARY
				, NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			int rc = GetLastError();
			WCHAR err[128];
			wsprintf(err, L"CreateFile() rc: %d, [%d] [%s]", rc, unique, tempFilename);
			Assert::Fail(err);
		}

		hReadHandle = OpenTempfileForReading(tempFilename);

	}
	~HelpTempFile()
	{
		CloseHandle(this->hFile);
		CloseHandle(this->hReadHandle);
	}

	HANDLE getReadHandle()
	{
		return this->hReadHandle;
	}

	void WriteContentW(LPCWSTR stuff)
	{
		if (!utf16bomwritten)
		{
			utf16bomwritten = true;
			WriteUTF16LEBOM();
		}

		DWORD len = lstrlen(stuff) * sizeof(WCHAR);

		DWORD written;
		WriteFile(
			this->hFile
			, stuff
			, len
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
		utf16bomwritten = true;
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

	
};