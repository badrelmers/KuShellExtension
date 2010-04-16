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

#include "stdafx.h"
#include "globals.h"
#include "KuMenuSet.h"

ULONG CRefCount::m_uInstances = 0;

namespace ku {

#ifndef _WIN64
BOOL bIsWow64 = FALSE;
#endif

HMODULE hModule = NULL;
CString sModulePath;
CString sModuleDir;
TCHAR *sBlackList[] = {_T("utorrent.exe"), NULL};
SYSVER SysVer = {0};

Gdiplus::GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken = 0;

CString sConfigFile;
BY_HANDLE_FILE_INFORMATION cfgFileInfo = {0};

HRESULT LoadConfig()
{
	if (sConfigFile.IsEmpty()) {
		sConfigFile = sModulePath;
		LPTSTR sConfig = sConfigFile.GetBufferSetLength(KU_MAX_PATH);
		LPTSTR ptr = _tcsrchr(sConfig, _T('\\'));
		if (ptr) {
			*ptr = 0;
			sModuleDir = sConfig;
			*ptr = _T('\\');
			_tcscpy(ptr + 1, _T("config.xml"));
		}
		sConfigFile.ReleaseBuffer();
		CKuMenuSet::InitBuiltinVars();
	}

	HANDLE hFile;
	BY_HANDLE_FILE_INFORMATION oldInfo = cfgFileInfo;
	if ((hFile = CreateFile(sConfigFile, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE)
		return E_HANDLE;
	if (!GetFileInformationByHandle(hFile, &cfgFileInfo))
		return E_FAIL;
	CloseHandle(hFile);
	if (oldInfo.nFileSizeLow != cfgFileInfo.nFileSizeLow || oldInfo.nFileSizeHigh != cfgFileInfo.nFileSizeHigh ||
		oldInfo.ftLastWriteTime.dwLowDateTime != cfgFileInfo.ftLastWriteTime.dwLowDateTime || 
		oldInfo.ftLastWriteTime.dwHighDateTime != cfgFileInfo.ftLastWriteTime.dwHighDateTime)
		if (!g_menu.FromFile(sConfigFile))
			return E_FAIL;
	return S_OK;
}

}
