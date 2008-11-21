#include "stdafx.h"
#include "CUnknown.h"
#include "globals.h"

ULONG CUnknown::g_uRefCount = 0;

namespace ku {

HMODULE hModule = NULL;
TCHAR sModulePath[KU_MAX_PATH];
TCHAR *sBlackList[] = {_T("utorrent.exe"), NULL};

#ifndef _WIN64
BOOL bIsWow64 = FALSE;
#endif

SYSVER SysVer = {0};


}
