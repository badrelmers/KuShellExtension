#pragma once

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

#define SYMBOLIC_LINK_FLAG_DIRECTORY (0x1)

extern HMODULE hModule;
extern TCHAR sModulePath[];
extern TCHAR *sBlackList[];
extern SYSVER SysVer;

}
