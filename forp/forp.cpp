#include "pch.h"

int main(int argc, wchar_t *argv[])
{
	Log* logger = Log::Instance();
	int rc = 0;

	OPTS opts;
	if (!getopts(argc, argv, &opts))
	{
		return 1;
	}
    
	HANDLE hInput = CreateFileW(opts.inputfilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hInput == INVALID_HANDLE_VALUE)
	{
		logger->win32errfunc(L"CreateFileW", opts.inputfilename);
		rc = 2;
	}
	else
	{
		READLINE* rl = rl_new(hInput, 256);
		LPWSTR line = NULL;
		DWORD cchLen;
		DWORD rl_rc;
		while ( ((rl_rc = rl_readline(rl, &line, &cchLen)) == 0) && (line != NULL) )
		{
			logger->writeLine(L"line [%s]", line);
		}
	}

	if (hInput != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hInput);
	}
	
	return rc;
}

