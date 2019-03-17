#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestForp
{
	TEST_CLASS(TestUTF8Convert)
	{
	public:
		TEST_METHOD(invalidUTF8Codepoint)
		{
			int rc = 0;
			// 61 62 63 C3 B6 C3 A4 C3 BC
			char mbString[] = { 'A', 'B', 'C', 0xC3 };

			WCHAR out[16];

			int widecharsWritten;

			if ((widecharsWritten = MultiByteToWideChar(
				CP_UTF8								// CodePage 
				, MB_ERR_INVALID_CHARS				// dwFlags
				, mbString							// lpMultiByteStr
				, 4									// cbMultiByte 
				, out								// lpWideCharStr
				, 16								// cchWideChar 
			)) == 0)
			{
				rc = GetLastError();
			}
			Assert::AreNotEqual(0, rc);
		}
		TEST_METHOD(validUTF8Codepoint)
		{
			int rc;
			// 61 62 63 C3 B6 C3 A4 C3 BC
			char mbString[] = { 'a', 'b', 'c', 0xC3, 0xB6 };

			WCHAR out[16];

			int widecharsWritten;

			if ((widecharsWritten = MultiByteToWideChar(
				CP_UTF8								// CodePage 
				, MB_ERR_INVALID_CHARS				// dwFlags
				, mbString							// lpMultiByteStr
				, 5									// cbMultiByte 
				, out								// lpWideCharStr
				, 16								// cchWideChar 
			)) == 0)
			{
				rc = GetLastError();
			}
			else
			{
				out[widecharsWritten] = L'\0';
			}
			Assert::AreEqual(L"abcö", out);
		}
		TEST_METHOD(invalidUTF8CodepointWithoutErrorFlag)
		{
			int rc;
			// 61 62 63 C3 B6 C3 A4 C3 BC
			char mbString[] = { 0x61, 0x62, 0x63, 0xC3 };

			WCHAR out[16];

			int widecharsWritten;

			if ((widecharsWritten = MultiByteToWideChar(
				CP_UTF8								// CodePage 
				, 0									// dwFlags
				, mbString							// lpMultiByteStr
				, 4									// cbMultiByte 
				, out								// lpWideCharStr
				, 16								// cchWideChar 
			)) == 0)
			{
				rc = GetLastError();
			}
		}
	};
}