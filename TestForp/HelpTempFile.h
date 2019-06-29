#pragma once

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

class HelpTempFile
{

private:
	WCHAR _tempFilename[MAX_PATH];

	HANDLE _hWriteHandle;
	HANDLE _hReadHandle;

	bool _utf16bomwritten = false;

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

	HelpTempFile(UINT unique, LPCWSTR baseDir, LPCWSTR prefix)
	{
		GetTempFileNameW(
			  baseDir
			, prefix
			, unique
			, _tempFilename
		);

		_hWriteHandle =
			CreateFile(
				_tempFilename
				, GENERIC_WRITE
				, FILE_SHARE_READ
				, NULL
				, CREATE_ALWAYS
				, FILE_ATTRIBUTE_TEMPORARY
				, NULL);

		if (_hWriteHandle == INVALID_HANDLE_VALUE)
		{
			int rc = GetLastError();
			WCHAR err[128];
			wsprintf(err, L"CreateFile() rc: %d, [%d] [%s]", rc, unique, _tempFilename);
			Assert::Fail(err);
		}

		_hReadHandle = OpenTempfileForReading(_tempFilename);

	}
	~HelpTempFile()
	{
		CloseHandle(this->_hWriteHandle);
		CloseHandle(this->_hReadHandle);
		DeleteFileW(_tempFilename);
	}

	HANDLE getReadHandle()
	{
		return this->_hReadHandle;
	}

	void WriteW(LPCWSTR stuff)
	{
		if (!_utf16bomwritten)
		{
			_utf16bomwritten = true;
			WriteUTF16LEBOM();
		}

		DWORD len = lstrlen(stuff) * sizeof(WCHAR);

		DWORD written;
		WriteFile(
			this->_hWriteHandle
			, stuff
			, len
			, &written
			, NULL
		);
	}
	void WriteA(LPCSTR stuff)
	{
		DWORD written;
		WriteFile(
			this->_hWriteHandle
			, stuff
			, lstrlenA(stuff)
			, &written
			, NULL
		);
	}
	void WriteUTF8BOM()
	{
		// UTF8 ... 0xEF,0xBB,0xBF
		unsigned char BOM[3] = { 0xEF,0xBB,0xBF };
		WriteBuff(BOM, 3);
	}
	void WriteUTF16LEBOM()
	{
		unsigned char BOM[2] = { 0xFF, 0xFE };
		WriteBuff(BOM, 2);
		_utf16bomwritten = true;
	}
	void WriteBuff(unsigned char* buf, int cbLen)
	{
		DWORD written;
		WriteFile(
			this->_hWriteHandle
			, buf
			, cbLen
			, &written
			, NULL
		);
	}
};