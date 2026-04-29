/* This file is part of KuShellExtension
 * Copyright (C) 2008-2010 Kai-Chieh Ku (kjackie@gmail.com)
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

#pragma once

#ifndef HAVE_ATL
#include "KuString.h"
#endif

class CKuMenuSet;

typedef union {
	DWORD m_dwVersion;
	struct {
		BYTE m_vMajor;
		BYTE m_vMinor;
		BYTE m_vVersion;
		BYTE m_vBuild;
	};
} SYSVER;

namespace ku {

#ifndef _WIN64
extern BOOL bIsWow64;
#endif

extern HMODULE hModule;
extern CString sModulePath;
extern CString sModuleDir;
extern TCHAR *sBlackList[];
extern SYSVER SysVer;

extern Gdiplus::GdiplusStartupInput gdiplusStartupInput;
extern ULONG_PTR gdiplusToken;

extern CString sConfigFile;
extern BY_HANDLE_FILE_INFORMATION cfgFileInfo;

HRESULT LoadConfig();

}
