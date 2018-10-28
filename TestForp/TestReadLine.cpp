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

	};
}