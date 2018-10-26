#include "stdafx.h"
//#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestForp
{
	TEST_CLASS(TestReadline)
	{
	public:
		TEST_METHOD(OneLineWithCrLf)
		{
			//HelpTempFile* hTmp = new HelpTempFile();
			HelpTempFile hTmp;
			hTmp.WriteContentA("berni\r\n");

			HANDLE handle = hTmp.OpenTempfile();

			READLINE* rl = rl_new(handle, 8);
			LPWSTR line;
			DWORD cchLen;
			rl_readline(rl, &line, &cchLen);

			Assert::AreEqual(L"berni", line);

			rl_delete(rl);
		}
	};
}