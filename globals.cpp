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
		LPTSTR ptr = _tcsrchr(sConfigFile.GetBufferSetLength(KU_MAX_PATH), _T('\\'));
		if (ptr)
			_tcscpy(ptr + 1, _T("config.xml"));
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
