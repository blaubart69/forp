#pragma once

typedef struct _opts
{
	int parallel;
	bool dryrun;
	LPWSTR cmdLineToExec;
	size_t cchcmdLineToExecSize;
} OPTS;

bool getopts(int argc, wchar_t *argv[], OPTS* opts);
DWORD SubstPercentSymbols(LPCWSTR input, LPCWSTR replacements, LPWSTR* buffer);
