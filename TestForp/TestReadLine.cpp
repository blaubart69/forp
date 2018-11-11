#include "stdafx.h"
//#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;

namespace TestForp
{
	volatile LONG64 counter;

	TEST_CLASS(TestReadline)
	{
	private:

		HelpTempFile* hTmp;
		READLINE* rl;

		DWORD LastRc = 0;

		//
		// Assert helper
		//
		void AssertReadline(LPCWSTR expectedLine)
		{
			LPWSTR line;
			DWORD cchLen;

			DWORD rc = rl_readline(this->rl, &line, &cchLen);
			if (rc != 0)
			{
				WCHAR err[128];
				wsprintf(err, L"rl_readline() returned rc: %d", rc);
				Assert::Fail( err );
			}

			int expectedLen = expectedLine == NULL ? 0 : lstrlenW(expectedLine);

			Assert::AreEqual(expectedLine, line);
			Assert::IsTrue(expectedLen == cchLen);
		}

		void initReadline(DWORD bufSize)
		{
			rl = rl_new(hTmp->getReadHandle(), bufSize);
		}

	public:
		
		TEST_CLASS_INITIALIZE(classInit)
		{
			counter = 0;
		}
		TEST_METHOD_INITIALIZE(initMeth)
		{
			InterlockedIncrement64(&counter);
			hTmp = new HelpTempFile((UINT)counter);
		}

		TEST_METHOD_CLEANUP(cleanMeth)
		{
			if (LastRc == 0)
			{
				AssertReadline(NULL);
			}
			delete hTmp;
			rl_delete(rl);
		}
		//
		// one line with crlf
		//
		TEST_METHOD(OneLineWithCrLf)
		{
			hTmp->WriteContentA("Berni\r\n");
			initReadline(8);
			AssertReadline(L"Berni");
		}
		TEST_METHOD(OneLineWithCrLfW)
		{
			hTmp->WriteContentW(L"berni\r\n");
			initReadline(16);
			AssertReadline(L"berni");
		}
		TEST_METHOD(OneLineWithCrLf_UTF8BOM)
		{
			hTmp->WriteUTF8BOM();
			hTmp->WriteContentA("berni\r\n");
			initReadline(16);
			AssertReadline(L"berni");
		}
		//
		// one line without crlf
		//
		TEST_METHOD(OneLineWithoutCrLf)
		{
			hTmp->WriteContentA("berni");
			initReadline(8);
			AssertReadline(L"berni");
		}
		TEST_METHOD(OneLineWithoutCrLfW)
		{
			hTmp->WriteContentW(L"berni");
			initReadline(32);
			AssertReadline(L"berni");
		}
		TEST_METHOD(OneLineWithoutCrLf_UTF8)
		{
			hTmp->WriteUTF8BOM();
			hTmp->WriteContentA("berni");
			initReadline(16);
			AssertReadline(L"berni");
		}
		//
		// empty file, only BOMs
		//
		TEST_METHOD(EmptyFileA_no_BOM)
		{
			initReadline(8);
			AssertReadline(NULL);
		}
		TEST_METHOD(EmptyFileA_UTF8BOM)
		{
			hTmp->WriteUTF8BOM();
			initReadline(8);
			AssertReadline(NULL);
		}
		TEST_METHOD(EmptyFileA_UTF16BOM)
		{
			hTmp->WriteUTF16LEBOM();
			initReadline(8);
			AssertReadline(NULL);
		}
		TEST_METHOD(OneLineWithoutCrLfW_bufferToSmall)
		{
			hTmp->WriteContentW(L"berni");

			LPWSTR line;
			DWORD  cchLen;

			initReadline(8);
			DWORD rc = rl_readline(rl, &line, &cchLen);
			Assert::IsTrue(rc == ERROR_INSUFFICIENT_BUFFER);
			LastRc = rc;
			Assert::IsNull(line);
			Assert::IsTrue(0 == cchLen);
		}
		TEST_METHOD(TwoLinesWithinBufferA)
		{
			hTmp->WriteContentA("berni\r\nspindler\r\n");

			initReadline(64);
			AssertReadline(L"berni");
			AssertReadline(L"spindler");
		}
		TEST_METHOD(TwoLinesWithinBufferW)
		{
			hTmp->WriteContentW(L"berni\r\nspindler\r\n");

			initReadline(64);
			AssertReadline(L"berni");
			AssertReadline(L"spindler");
		}
		TEST_METHOD(TwoLinesWithinBufferUTF8)
		{
			hTmp->WriteUTF8BOM();
			hTmp->WriteContentA("berni\r\nspindler\r\n");

			initReadline(64);
			AssertReadline(L"berni");
			AssertReadline(L"spindler");
		}
		TEST_METHOD(TwoLinesWithinBufferW_secondWithoutCrLf)
		{
			hTmp->WriteContentW(L"berni\r\nspindler");

			initReadline(64);
			AssertReadline(L"berni");
			AssertReadline(L"spindler");
		}
		TEST_METHOD(TwoLinesWithinBufferA_secondWithoutCrLf)
		{
			hTmp->WriteContentA("berni\r\nspindler");

			initReadline(64);
			AssertReadline(L"berni");
			AssertReadline(L"spindler");
		}
		TEST_METHOD(TwoLinesWithinBufferUTF8_secondWithoutCrLf)
		{
			hTmp->WriteUTF8BOM();
			hTmp->WriteContentA("berni\r\nspindler");

			initReadline(64);
			AssertReadline(L"berni");
			AssertReadline(L"spindler");
		}
		TEST_METHOD(SecondLineDoesNotFitInBuffer_A)
		{
			hTmp->WriteContentA("12345\r\nabcdefghijkl");

			initReadline(16);
			AssertReadline(L"12345");
			AssertReadline(L"abcdefghijkl");
		}
		TEST_METHOD(SecondLineDoesNotFitInBuffer_W)
		{
			hTmp->WriteContentW(L"12345\r\nabcdefghijkl");

			initReadline(13*2);
			AssertReadline(L"12345");
			AssertReadline(L"abcdefghijkl");
		}
		TEST_METHOD(ReadUmlautAsUTF8)
		{
			hTmp->WriteContentA("B�rnie");

			initReadline(16);
			AssertReadline(L"B�rnie");
		}
		//
		// emtpy line
		//
		TEST_METHOD(OneEmtpyLineA)
		{
			hTmp->WriteContentA("\r\n");
			initReadline(16);
			AssertReadline(L"");
		}
		TEST_METHOD(OneEmtpyLineW)
		{
			hTmp->WriteContentW(L"\r\n");
			initReadline(16);
			AssertReadline(L"");
		}
		TEST_METHOD(OneEmtpyLineUTF8)
		{
			hTmp->WriteUTF8BOM();
			hTmp->WriteContentA("\r\n");
			initReadline(16);
			AssertReadline(L"");
		}
		//
		// emtpy lines
		//
		TEST_METHOD(TwoEmtpyLineA)
		{
			hTmp->WriteContentA("\r\n\r\n");
			initReadline(16);
			AssertReadline(L"");
			AssertReadline(L"");
		}
		TEST_METHOD(TwoEmtpyLineW)
		{
			hTmp->WriteContentW(L"\r\n\r\n");
			initReadline(16);
			AssertReadline(L"");
			AssertReadline(L"");
		}
		TEST_METHOD(TwoEmtpyLineUTF8)
		{
			hTmp->WriteUTF8BOM();
			hTmp->WriteContentA("\r\n\r\n");
			initReadline(16);
			AssertReadline(L"");
			AssertReadline(L"");
		}
		//
		// buffer full with newlines 
		//
		TEST_METHOD(SixemptyLinesA)
		{
			hTmp->WriteContentA("\r\n\r\n\r\n\r\n\r\n\r\n");
			initReadline(16);
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
		}
		TEST_METHOD(SixemptyLinesW)
		{
			hTmp->WriteContentW(L"\r\n\r\n\r\n\r\n\r\n\r\n");
			initReadline(128);
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
		}
		TEST_METHOD(SixemptyLinesUTF8)
		{
			hTmp->WriteUTF8BOM();
			hTmp->WriteContentA("\r\n\r\n\r\n\r\n\r\n\r\n");
			initReadline(16);
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
		}
		TEST_METHOD(SixEmptyLinesWithTooSmallBufferA)
		{
			hTmp->WriteContentA("\r\n\r\n\r\n\r\n\r\n\r\n");
			initReadline(4);
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
		}
		TEST_METHOD(SixEmptyLinesWithTooSmallBufferW)
		{
			hTmp->WriteContentW(L"\r\n\r\n\r\n\r\n\r\n\r\n");
			initReadline(8);

			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
		}
		//
		// UNIX ...
		//
		TEST_METHOD(UnixOneEmptyLineA)
		{
			hTmp->WriteContentA("\n");
			initReadline(6);
			AssertReadline(L"");
		}
		TEST_METHOD(UnixTwoEmptyLineA)
		{
			hTmp->WriteContentA("\n\n");
			initReadline(6);
			AssertReadline(L"");
			AssertReadline(L"");
		}
		TEST_METHOD(UnixOneEmptyLineW)
		{
			hTmp->WriteContentW(L"\n");
			initReadline(6);
			AssertReadline(L"");
		}
		TEST_METHOD(UnixTwoEmptyLineW)
		{
			hTmp->WriteContentW(L"\n\n");
			initReadline(6);
			AssertReadline(L"");
			AssertReadline(L"");
		}
		TEST_METHOD(UnixOneLineWithLfA)
		{
			hTmp->WriteContentA("Berni\n");
			initReadline(6);
			AssertReadline(L"Berni");
		}
		TEST_METHOD(UnixOneLineWithLfW)
		{
			hTmp->WriteContentW(L"Berni\n");
			initReadline(16);
			AssertReadline(L"Berni");
		}
		TEST_METHOD(UnixOneLineWithLfUTF8_bufferWithBOMTooSmallButTextFits)
		{
			hTmp->WriteUTF8BOM();
			hTmp->WriteContentA("Berni\n");
			initReadline(6);

			AssertReadline(L"Berni");
		}
		TEST_METHOD(LastLineHasOnlyOneCharA)
		{
			hTmp->WriteContentA("11111\r\n22222\r\n33333\r\n44444\r\nx");
			initReadline(21);
			AssertReadline(L"11111");
			AssertReadline(L"22222");
			AssertReadline(L"33333");
			AssertReadline(L"44444");
			AssertReadline(L"x");
		}
		TEST_METHOD(LastLineHasOnlyOneCharWithOnlyEmptyLinesBeforeA)
		{
			hTmp->WriteContentA("\n\n\n\n\n\nx");
			initReadline(21);
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"x");
		}
	};
}