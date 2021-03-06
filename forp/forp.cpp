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

	logger->inf(L"to execute: [%s]", opts.cmdLineTemplate);
	logger->inf(L"inputfile : [%s]", opts.inputfilename);

    
	HANDLE hInput = CreateFileW(opts.inputfilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hInput == INVALID_HANDLE_VALUE)
	{
		logger->win32errfunc(L"CreateFileW", opts.inputfilename);
		rc = 2;
	}
	else
	{
		//READLINE* rl = rl_new(hInput, 4096);
		//READLINE* rl = rl_new(hInput, 8192);
		Readline rl(hInput, 4096);
		DWORD rl_rc;

		int lineNum = 0;
		while ( ((rl_rc = rl.nextLine()) == 0) && (rl.line != NULL) )
		{
			logger->inf(L"cchLen/line %3d [%s]", rl.cchLen, rl.line);
		}
		if (rl_rc != 0)
		{
			logger->err(L"readline rc: %d", rl_rc);
		}
	}

	if (hInput != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hInput);
	}
	
	return rc;
}

