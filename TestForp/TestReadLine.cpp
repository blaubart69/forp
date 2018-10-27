#include "stdafx.h"
//#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestForp
{
	TEST_CLASS(TestReadline)
	{
	public:
		
		LPWSTR line;
		DWORD cchLen;
		HelpTempFile* hTmp;
		READLINE* rl;

		TEST_METHOD_INITIALIZE(initMeth)
		{
			hTmp = new HelpTempFile;
		}

		TEST_METHOD_CLEANUP(cleanMeth)
		{
			rl_readline(rl, &line, &cchLen);
			Assert::IsTrue(0 == cchLen);
			Assert::IsNull(line);
			delete hTmp;

			rl_delete(rl);
		}

		TEST_METHOD(OneLineWithCrLf)
		{
			hTmp->WriteContentA("berni\r\n");

			rl = rl_new(hTmp->getReadHandle(), 8);
			rl_readline(rl, &line, &cchLen);
			Assert::AreEqual(L"berni", line);
			Assert::IsTrue(5 == cchLen);
		}
		TEST_METHOD(OneLineWithCrLfW)
		{
			hTmp->WriteUTF16LEBOM();
			hTmp->WriteContentW(L"berni\r\n");

			rl = rl_new(hTmp->getReadHandle(), 16);
			rl_readline(rl, &line, &cchLen);
			Assert::AreEqual(L"berni", line);
			Assert::IsTrue(5 == cchLen);
		}
		TEST_METHOD(OneLineWithoutCrLf)
		{
			hTmp->WriteContentA("berni");

			rl = rl_new(hTmp->getReadHandle(), 8);
			rl_readline(rl, &line, &cchLen);
			Assert::AreEqual(L"berni", line);
			Assert::IsTrue(5 == cchLen);
		}
		TEST_METHOD(OneLineWithoutCrLfW_bufferToSmall)
		{
			hTmp->WriteContentW(L"berni");

			rl = rl_new(hTmp->getReadHandle(), 8);
			DWORD rc = rl_readline(rl, &line, &cchLen);
			Assert::IsTrue(rc == ERROR_INSUFFICIENT_BUFFER);
			Assert::IsNull(line);
		}
		TEST_METHOD(OneLineWithoutCrLfW)
		{
			hTmp->WriteContentW(L"berni");

			rl = rl_new(hTmp->getReadHandle(), 32);
			DWORD rc = rl_readline(rl, &line, &cchLen);
			Assert::AreEqual(L"berni", line);
			Assert::IsTrue(5 == cchLen);
		}
		TEST_METHOD(TwoLinesWithinBuffer)
		{
			hTmp->WriteContentA("berni\r\nspindler\r\n");

			rl = rl_new(hTmp->getReadHandle(), 64);
			
			rl_readline(rl, &line, &cchLen);
			Assert::AreEqual(L"berni", line);
			Assert::IsTrue(5 == cchLen);

			rl_readline(rl, &line, &cchLen);
			Assert::AreEqual(L"spindler", line);
			Assert::IsTrue(8 == cchLen);
		}
		TEST_METHOD(TwoLinesWithinBufferW)
		{
			hTmp->WriteContentW(L"berni\r\nspindler\r\n");

			rl = rl_new(hTmp->getReadHandle(), 64);

			rl_readline(rl, &line, &cchLen);
			Assert::AreEqual(L"berni", line);
			Assert::IsTrue(5 == cchLen);

			rl_readline(rl, &line, &cchLen);
			Assert::AreEqual(L"spindler", line);
			Assert::IsTrue(8 == cchLen);
		}
		TEST_METHOD(TwoLinesWithinBufferW_secondWithoutCrLf)
		{
			hTmp->WriteContentW(L"berni\r\nspindler");

			rl = rl_new(hTmp->getReadHandle(), 64);

			rl_readline(rl, &line, &cchLen);
			Assert::AreEqual(L"berni", line);
			Assert::IsTrue(5 == cchLen);

			rl_readline(rl, &line, &cchLen);
			Assert::AreEqual(L"spindler", line);
			Assert::IsTrue(8 == cchLen);
		}
	};
}