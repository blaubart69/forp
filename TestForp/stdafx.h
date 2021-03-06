// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

// Headers for CppUnitTest
#include "CppUnitTest.h"
#include <string>

// TODO: reference additional headers your program requires here

#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used 
#define STRICT
#include <Windows.h>

#include "..\forp\forp.h"
#include "..\forp\readline\readline.h"

#include "..\forp\LineReader\StreamTokenizer.h"
#include "..\forp\LineReader\LineReader.h"

#include "HelpTempFile.h"

#include "..\forp\Readline2\readline2.h"