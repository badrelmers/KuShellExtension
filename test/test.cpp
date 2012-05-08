// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../dll.h"
#include "../globals.h"
#include "../KuMenuSet.h"

static
void Init()
{
	ku::hModule = GetModuleHandle(NULL);
	GetModuleFileName(ku::hModule, ku::sModulePath.GetBufferSetLength(KU_MAX_PATH), KU_MAX_PATH);
	ku::sModulePath.ReleaseBuffer();
	ku::SysVer.m_dwVersion = GetVersion();
	ku::sConfigFile = ku::sModulePath;
	LPTSTR sConfig = ku::sConfigFile.GetBufferSetLength(KU_MAX_PATH);
	LPTSTR ptr = _tcsrchr(sConfig, _T('\\'));
	if (ptr) {
		*ptr = 0;
		ku::sModuleDir = sConfig;
		*ptr = _T('\\');
		_tcscpy(ptr + 1, _T("config.xml"));
	}
	ku::sConfigFile.ReleaseBuffer();
	CKuMenuSet::InitBuiltinVars();
}

int _tmain(int argc, _TCHAR* argv[])
{
	TCHAR buf[MAX_PATH];
	CString s;
	_tcscpy(buf, _T("C:\\Program Files"));
	VERIFY( !_tcscmp(_T("C:\\Program Files"), CKuMenuSet::CMenuItem::ExpandFileName(buf, _T("~dp"), s)) );
	VERIFY( !_tcscmp(_T("\"C:\\Program Files\""), CKuMenuSet::CMenuItem::ExpandFileName(buf, _T("~qdp"), s)) );
	VERIFY( !_tcscmp(_T("\"C:\\Program Files\""), CKuMenuSet::CMenuItem::ExpandFileName(buf, _T("~q"), s)) );
	VERIFY( !_tcscmp(_T("\"C:\\Program Files\""), CKuMenuSet::CMenuItem::ExpandFileName(buf, _T("~qs"), s)) );
	_tcscpy(buf, _T("C:\\"));
	VERIFY( !_tcscmp(_T("C:\\"), CKuMenuSet::CMenuItem::ExpandFileName(buf, _T("~dp"), s)) );
	VERIFY( !_tcscmp(_T("C:\\"), CKuMenuSet::CMenuItem::ExpandFileName(buf, _T("~dpnx"), s)) );
	VERIFY( !_tcscmp(_T("C:\\"), CKuMenuSet::CMenuItem::ExpandFileName(buf, _T("~q"), s)) );
	_tcscpy(buf, _T("V:\\comic\\宮野ともちか\\ゆびさきミルクティー"));
	VERIFY( !_tcscmp(_T("V:\\comic\\宮野ともちか\\ゆびさきミルクティー"), CKuMenuSet::CMenuItem::ExpandFileName(buf, _T("~q"), s)) );
	VERIFY( !_tcscmp(_T("V:\\comic\\_YANZ6~J\\_M3C2Y~1"), CKuMenuSet::CMenuItem::ExpandFileName(buf, _T("~qs"), s)) );
	return 0;
	Init();

//	VERIFY( g_menu.ParseConfig(_T("E:\\software\\_ui_\\KuShellExtension\\config.xml")) );

	VERIFY( g_menu.FromFile(_T("E:\\software\\_ui_\\KuShellExtension\\config.xml")) );

	HMENU hMenu = CreatePopupMenu();
	UINT indexMenu = 0, idCmdFirst = 1, idCmdLast = 1000;
	g_menu.m_pData = new CKuShellExtInitData;
	g_menu.m_pData->m_iType = CKuShellExtInitData::TypeDriveFixed;
	g_menu.m_pData->m_aFiles.Add(_T("C:\\"));
	VERIFY( SUCCEEDED(g_menu.QueryContextMenu(hMenu, indexMenu, idCmdFirst, idCmdLast, 0)) );
	//TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_NONOTIFY | TPM_LEFTBUTTON, 100, 100, 0, GetDesktopWindow(), NULL);

	return 0;
}

