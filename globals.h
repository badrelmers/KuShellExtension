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
extern TCHAR *sBlackList[];
extern SYSVER SysVer;

extern Gdiplus::GdiplusStartupInput gdiplusStartupInput;
extern ULONG_PTR gdiplusToken;

extern CString sConfigFile;
extern BY_HANDLE_FILE_INFORMATION cfgFileInfo;

HRESULT LoadConfig();

}
