#include "pch.h"

void * __cdecl operator new (size_t size)
{
	return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
}

void * __cdecl operator new[](unsigned __int64 size)
{
	return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
}

void __cdecl operator delete(void *ptrToRelease, size_t size)
{
	HeapFree(GetProcessHeap(), 0, ptrToRelease);
}

void __cdecl operator delete[](void* memToFree)
{
	HeapFree(GetProcessHeap(), 0, memToFree);
}

//
// The _purecall function is a Microsoft-specific implementation detail of the Microsoft Visual C++ compiler. 
// This function is not intended to be called by your code directly, and it has no public header declaration. 
// It is documented here because it is a public export of the C Runtime Library.
// https://msdn.microsoft.com/en-us/library/ff798096.aspx
//
extern "C" int __cdecl _purecall() { return -1; }

extern "C" void mainCRTStartup(void)
{
	int argc;
	LPWSTR* argv;
	argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	int Exitcode = main(argc, argv);

	LocalFree(argv);

	ExitProcess(Exitcode);
}

