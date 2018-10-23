#include "pch.h"

void printUsage(void);

bool getopts(int argc, wchar_t *argv[], OPTS* opts)
{
	opts->dryrun = false;
	opts->parallel = 16;
	opts->cmdLineToExec = NULL;

	bool showHelp = false;

	int i;
	for (i = 1; i < argc; ++i)
	{
		if (argv[i][0] == L'-' && argv[i][1] != 0)
		{
			switch (argv[i][1])
			{
			case L't':
				++i;
				opts->parallel = StrToIntW((const wchar_t*)argv[i]);
				break;
			case L'n':	opts->dryrun = true;			break;
			case L'd':	Log::Instance()->setLevel(3);	break;
			case L'h':	showHelp = true;				break;
			default:
				break;
			}
		}
		else
		{
			break;
		}
	}

	if (showHelp)
	{
		printUsage();
		return false;
	}

	if (i >= argc)
	{
		Log::Instance()->err(L"no command to execute given");
		printUsage();
		return false;
	}

	opts->cchcmdLineToExecSize = lstrlenW(GetCommandLineW());
	opts->cmdLineToExec = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, opts->cchcmdLineToExecSize);

	for (; i < argc; ++i)
	{
		StrCatBuffW(opts->cmdLineToExec, argv[i], (int)opts->cchcmdLineToExecSize);
		if (i + 1 < argc)
		{
			StrCatBuffW(opts->cmdLineToExec, L" ", (int)opts->cchcmdLineToExecSize);
		}
	}

	return 0;
}

void printUsage(void)
{
	Log::Instance()->writeLine(
		L"usage: forp [OPTIONS] {program to exec with params}"
		L"\nOptions:"
		L"\n  -n     dry run"
		L"\n  -t     how many processes to run in parallel"
		L"\n  -d     debug outputs"
		L"\n  -h     show this help"
		L"\n");
}