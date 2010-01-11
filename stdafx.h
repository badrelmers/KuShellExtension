/* This file is part of KuShellExtension
 * Copyright (C) 2008-2009 Kai-Chieh Ku (kjackie@gmail.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <winioctl.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <io.h>
#include <fcntl.h>

#ifdef HAVE_ATL
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit
#include <atldef.h>
#include <atlbase.h>
#include <atlstr.h>
#include <atlcoll.h>
#include <atlsimpcoll.h>
#endif

#include <shlwapi.h>
#include <shlobj.h>
#include <shellapi.h>
#include <olectl.h>

#ifdef __GNUC__
//#define __out_bcount(...)
//#define __out_ecount(...)
//#define __field_ecount_opt(...)
//#define __reserved
//#define __out_awcount(...)

#include <pshpack8.h>
namespace Gdiplus {
	enum Status {
		Ok = 0
	};
	#include <GdiPlusInit.h>

	typedef Status GpStatus;
	typedef DWORD ARGB;

	class GpImage;
	class GpBitmap;
	namespace Color
	{
		enum
		{
			Transparent = 0x00FFFFFF
		};
	}
}
#include <poppack.h>
#define WINGDIPAPI __stdcall
#else
#include <gdiplus.h>
#endif

#ifndef TRACE
	#ifdef _DEBUG
		#if defined(HAVE_ATL) || defined(ATLTRACE)
			#include <atlbase.h>
			#define TRACE(...) ATLTRACE(atlTraceGeneral, 0, __VA_ARGS__)
		#else
			#define TRACE(...) _ftprintf(stderr, __VA_ARGS__)
		#endif
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
