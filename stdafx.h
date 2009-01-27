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

#ifdef HAVE_ATL
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

#ifndef TRACE
	#ifdef HAVE_ATL
		#include <atlbase.h>
		#define TRACE(...) ATLTRACE(atlTraceGeneral, 0, __VA_ARGS__)
	#else
		#define TRACE(...)
	#endif
#endif

/*
This part of codes map the new operator to the debug version. Although the map is contains in "crtdbg.h",
it is not really work (we will always get a wrong information show us the leaked memory blocks are allocated in "crtdbg.h").
*/
#ifdef _MSC_VER
	#define _CRTDBG_MAP_ALLOC
	#include <crtdbg.h>
	#ifdef _DEBUG
		#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
	#endif
#endif

#ifndef ASSERT
	#ifdef _ASSERTE
		#define ASSERT(expr) _ASSERTE(expr)
	#else
		#ifdef _DEBUG
			#include <assert.h>
			#define ASSERT(expr) assert(expr)
		#else
			#define ASSERT(expr)
		#endif
	#endif
#endif

#ifndef VERIFY
	#ifdef _DEBUG
		#define VERIFY(expr) ASSERT(expr)
	#else
		#define VERIFY(expr) expr
	#endif
#endif

#ifndef _countof
#define _countof(a) (sizeof(a) / sizeof(a[0]))
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
