// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef NO_ATL
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit
#include <atldef.h>
#include <atlbase.h>
#include <atlstr.h>
#include <atlcoll.h>
#include <atlsimpcoll.h>
#else
#include "KuString.h"
#include "KuATL.h"
#endif

#include <shlwapi.h>
#include <shobjidl.h>
#include <shlobj.h>
#include <shellapi.h>
#include <olectl.h>

#include <gdiplus.h>

#include "pugxml.h"
#include "FSLinks/FSLinks.h"

/*
This part of codes map the new operator to the debug version. Although the map is contains in "crtdbg.h",
it is not really work (we will always get a wrong information show us the leaked memory blocks are allocated in "crtdbg.h").
*/
#ifdef _MSC_VER
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#include <crtdbg.h>
#endif

#ifndef ARRSIZE
#define ARRSIZE(a) (sizeof(a) / sizeof(a[0]))
#endif

#ifndef NO_ATL
#ifndef TRACE
#define TRACE(...) ATLTRACE(atlTraceGeneral, 0, __VA_ARGS__)
#endif

#ifndef ASSERT
#define ASSERT(expr) ATLASSERT(expr)
#endif

#ifndef VERIFY
#ifdef _DEBUG
#define VERIFY(expr) ASSERT(expr)
#else
#define VERIFY(expr) expr
#endif
#endif
#else
#define TRACE(...)
#define ASSERT(expr)
#define VERIFY(expr) expr
#endif

class LastErrorMsg
{
public:
	LastErrorMsg() : lpErrMsgBuffer(NULL)
	{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), 0, (LPTSTR) &lpErrMsgBuffer, 0, NULL);
	}
	~LastErrorMsg() { if (lpErrMsgBuffer) LocalFree(lpErrMsgBuffer); }
	operator LPCTSTR() {return (LPCTSTR) lpErrMsgBuffer;}
private:
	LPTSTR lpErrMsgBuffer;
};

#define KU_MAX_PATH 1024
