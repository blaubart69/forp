#pragma once

//#pragma comment(lib, "shell32.lib")

int main(int argc, wchar_t *argv[]);

void * __cdecl operator new    (size_t size);
void   __cdecl operator delete (void *ptrToRelease, size_t size);

