#include "../pch.h"

DWORD SubstPercentSymbols(LPCWSTR input, LPCWSTR replacements, LPWSTR* buffer)
{
	int NumArgs;
	LPWSTR* argv;

	argv = CommandLineToArgvW(replacements, &NumArgs);
	if (argv == NULL)
	{
		Log::Instance()->win32err(L"CommandLineToArgvW(SubstPercentSymbols)", replacements);
		return 0;
	}

	DWORD charsWritten = FormatMessageW(
		  FORMAT_MESSAGE_ALLOCATE_BUFFER
		| FORMAT_MESSAGE_ARGUMENT_ARRAY
		| FORMAT_MESSAGE_FROM_STRING
		, input
		, 0		// 	dwMessageId
		, 0		//	dwLanguageId
		, (LPWSTR)buffer
		, 1		// nSize
		, (va_list*)argv
	);
	if (charsWritten == 0)
	{
		Log::Instance()->win32err(L"FormatMessageW(SubstPercentSymbols)", input);
	}

	LocalFree(argv);

	return charsWritten;
}