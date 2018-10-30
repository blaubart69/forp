#include "pch.h"

void printUsage(void);
void concatArgsToOneString(int argc, wchar_t *argv[], int startIdx, LPWSTR* line, SIZE_T* len);

bool getopts(int argc, wchar_t *argv[], OPTS* opts)
{
	opts->dryrun = false;
	opts->parallel = 16;
	opts->cmdLineTemplate = NULL;
	opts->inputfilename = NULL;

	bool showHelp = false;
	bool ok = false;

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
			case L'f':
				++i;
				opts->inputfilename = argv[i];
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
	}
	else if (i >= argc)
	{
		Log::Instance()->err(L"no command to execute given");
	} 
	else if (opts->inputfilename == NULL)
	{
		Log::Instance()->err(L"must specifiy an input file");
	}
	else
	{
		concatArgsToOneString(argc, argv, i, &opts->cmdLineTemplate, &opts->cchcmdLineToExecSize);
		ok = true;
	}

	if (!ok)
	{
		printUsage();
	}

	return ok;
}

void concatArgsToOneString(int argc, wchar_t *argv[], int startIdx, LPWSTR* line, SIZE_T* cchLen)
{
	*cchLen = lstrlenW(GetCommandLineW());
	*line = (LPWSTR)HeapAlloc(GetProcessHeap(), 0, (*cchLen) * sizeof(WCHAR));
	*line[0] = L'\0';

	for (int i = startIdx; i < argc; ++i)
	{
		StrCatBuffW(*line, argv[i], (int)*cchLen);
		if (i + 1 < argc)
		{
			StrCatBuffW(*line, L" ", (int)*cchLen);
		}
	}
}

void printUsage(void)
{
	Log::Instance()->writeLine(
		L"usage: forp [OPTIONS] {program to exec with params}"
		L"\nOptions:"
		L"\n  -f     file with arguments per line"
		L"\n  -n     dry run"
		L"\n  -t     how many processes to run in parallel"
		L"\n  -d     debug"
		L"\n  -h     show this help"
		L"\n");
}