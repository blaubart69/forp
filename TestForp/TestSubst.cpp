#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestForp
{		
	TEST_CLASS(TestCommandlineSubst)
	{
	public:
		TEST_METHOD(NoSubst)
		{
			LPWSTR buffer;
			SubstPercentSymbols(L"Hallo Sumsi", L"Bernhard", &buffer);
			Assert::AreEqual(L"Hallo Sumsi", buffer);
		}
		TEST_METHOD(OneReplacement)
		{
			LPWSTR buffer;
			SubstPercentSymbols(L"Hallo %1", L"Bernhard", &buffer);
			Assert::AreEqual(L"Hallo Bernhard", buffer);
		}
		TEST_METHOD(TwoReplacement)
		{
			LPWSTR buffer;
			SubstPercentSymbols(L"Hallo %2 %1", L"Spindler Bernhard", &buffer);
			Assert::AreEqual(L"Hallo Bernhard Spindler", buffer);
		}
		TEST_METHOD(OneReplacementTwoTimes)
		{
			LPWSTR buffer;
			SubstPercentSymbols(L"Hallo %1 %1", L"Spindler Bernhard", &buffer);
			Assert::AreEqual(L"Hallo Spindler Spindler", buffer);
		}

		TEST_METHOD(GivenOneSubstAndNothingSpecifiedInFormatString)
		{
			LPWSTR buffer;
			SubstPercentSymbols(L"Hallo %2", L"1", &buffer);
			Assert::AreEqual(L"Hallo (null)", buffer);
		}
	};
}