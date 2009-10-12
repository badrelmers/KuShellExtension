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

// KuShellExtension.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "KuShellExtensionFactory.h"
#include "KuContextMenu.h"
#include "globals.h"
#include "dll.h"

// Default CLSIDs, can be changed in configure file.
#ifdef _WIN64
#define CLSID_CONFIG _T("CLSID64")
// {72F6A03F-7B17-4e65-AE37-666FC9024FA2}
static CLSID g_CLSID = 
{ 0x72f6a03f, 0x7b17, 0x4e65, { 0xae, 0x37, 0x66, 0x6f, 0xc9, 0x2, 0x4f, 0xa2 } };
#else
#define CLSID_CONFIG _T("CLSID32")
// {ACF4C166-8665-462d-B701-D4978E0009A1}
static CLSID g_CLSID = 
{ 0xacf4c166, 0x8665, 0x462d, { 0xb7, 0x1, 0xd4, 0x97, 0x8e, 0x0, 0x9, 0xa1 } };
#endif

static CString g_sName;
static CString g_sCLSID;

void RedirectIOToConsole()
{
	HANDLE hConHandle;
	int iStdHandle;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;

	FILE *fp;

	// allocate a console for this app
	AllocConsole();

	// set the screen buffer to be big enough to let us scroll text
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),
		&coninfo);

	// maximum lines
	coninfo.dwSize.Y = 2000;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),
		coninfo.dwSize);

	// redirect unbuffered STDOUT to the console
	hConHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	iStdHandle = _open_osfhandle((intptr_t) hConHandle, _O_TEXT);
	fp = _fdopen( iStdHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );

	// redirect unbuffered STDIN to the console
	hConHandle = GetStdHandle(STD_INPUT_HANDLE);
	iStdHandle = _open_osfhandle((intptr_t) hConHandle, _O_TEXT);
	fp = _fdopen( iStdHandle, "r" );
	*stdin = *fp;
	setvbuf( stdin, NULL, _IONBF, 0 );

	// redirect unbuffered STDERR to the console
	hConHandle = GetStdHandle(STD_ERROR_HANDLE);
	iStdHandle = _open_osfhandle((intptr_t) hConHandle, _O_TEXT);
	fp = _fdopen( iStdHandle, "w" );
	*stderr = *fp;
	setvbuf( stderr, NULL, _IONBF, 0 );

	// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
	// point to console as well
#ifdef _IOSTREAM_
	std::ios::sync_with_stdio();
#endif
}

bool InitConfig()
{
	static bool bInit = false;
	if (bInit)
		return true;

#if defined(_DEBUG) && !defined(HAVE_ATL)
	RedirectIOToConsole();
#endif

	bInit = true;
	dll::Init();
#ifndef _WIN64
	if (dll::IsWow64Process && !dll::IsWow64Process(GetCurrentProcess(), &ku::bIsWow64))
		ku::bIsWow64 = FALSE;
#endif

	ku::LoadConfig();
	if (g_menu.GetOurVariable(CLSID_CONFIG, g_sCLSID)) {
		if (g_sCLSID.GetLength() != 38)
			return false;
		g_sCLSID.MakeUpper();

		DWORD dwData[5] = {0};
		// use DWORDs instead of passing CLSID members directly, because %X uses 4 bytes.
		if (_stscanf(g_sCLSID, _T("{%08X-%04X-%04X-%04X-%04X%08X}"), &g_CLSID.Data1, dwData, dwData + 1, dwData + 2, dwData + 3, dwData + 4) != 6)
			return false;
		g_CLSID.Data2 = (WORD) dwData[0];
		g_CLSID.Data3 = (WORD) dwData[1];
		g_CLSID.Data4[0] = (BYTE) (dwData[2] >> 8);
		g_CLSID.Data4[1] = (BYTE) dwData[2];
		g_CLSID.Data4[2] = (BYTE) (dwData[3] >> 8);
		g_CLSID.Data4[3] = (BYTE) dwData[3];
		g_CLSID.Data4[4] = (BYTE) (dwData[4] >> 24);
		g_CLSID.Data4[5] = (BYTE) (dwData[4] >> 16);
		g_CLSID.Data4[6] = (BYTE) (dwData[4] >> 8);
		g_CLSID.Data4[7] = (BYTE) dwData[4];
	}
	else
		g_sCLSID.Format(_T("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"), g_CLSID.Data1, g_CLSID.Data2, g_CLSID.Data3,
			g_CLSID.Data4[0], g_CLSID.Data4[1], g_CLSID.Data4[2], g_CLSID.Data4[3], g_CLSID.Data4[4], g_CLSID.Data4[5], g_CLSID.Data4[6], g_CLSID.Data4[7]);

	g_sName.Format(_T("!!KuShellExtension-%s"), g_sCLSID.GetString());

	return true;
}

EXTERN_C BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			{
#ifdef _MSC_VER
				_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); //detect memory leaks
#endif
				GetModuleFileName(NULL, ku::sModulePath.GetBufferSetLength(KU_MAX_PATH), KU_MAX_PATH);
				ku::sModulePath.ReleaseBuffer();
				LPCTSTR sProcessName = _tcsrchr(ku::sModulePath, _T('\\'));
				if (!sProcessName)
					return FALSE;
				sProcessName++;
				int i;
				for (i = 0;ku::sBlackList[i];i++)
					if (!_tcsicmp(sProcessName, ku::sBlackList[i]))
						return FALSE;
				ku::hModule = hModule;
				GetModuleFileName(hModule, ku::sModulePath.GetBufferSetLength(KU_MAX_PATH), KU_MAX_PATH);
				ku::sModulePath.ReleaseBuffer();
				ku::SysVer.m_dwVersion = GetVersion();
			}
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

STDAPI DllRegisterServer()
{
	if (!InitConfig())
		return SELFREG_E_CLASS;

	DWORD dwNameCcb = (g_sName.GetLength() + 1) * sizeof(TCHAR);
	DWORD dwClsidCcb = (g_sCLSID.GetLength() + 1) * sizeof(TCHAR);

	if (SHSetValue(HKEY_CURRENT_USER, _T("Software\\Classes\\*\\shellex\\ContextMenuHandlers\\") + g_sName, NULL, REG_SZ, (LPCVOID) g_sCLSID.GetString(), dwClsidCcb) != ERROR_SUCCESS ||
		SHSetValue(HKEY_CURRENT_USER, _T("Software\\Classes\\Directory\\shellex\\ContextMenuHandlers\\") + g_sName, NULL, REG_SZ, (LPCVOID) g_sCLSID.GetString(), dwClsidCcb) != ERROR_SUCCESS ||
		SHSetValue(HKEY_CURRENT_USER, _T("Software\\Classes\\Directory\\Background\\shellex\\ContextMenuHandlers\\") + g_sName, NULL, REG_SZ, (LPCVOID) g_sCLSID.GetString(), dwClsidCcb) != ERROR_SUCCESS ||
		SHSetValue(HKEY_CURRENT_USER, _T("Software\\Classes\\LibraryFolder\\shellex\\ContextMenuHandlers\\") + g_sName, NULL, REG_SZ, (LPCVOID) g_sCLSID.GetString(), dwClsidCcb) != ERROR_SUCCESS ||
		SHSetValue(HKEY_CURRENT_USER, _T("Software\\Classes\\LibraryFolder\\Background\\shellex\\ContextMenuHandlers\\") + g_sName, NULL, REG_SZ, (LPCVOID) g_sCLSID.GetString(), dwClsidCcb) != ERROR_SUCCESS ||
		SHSetValue(HKEY_CURRENT_USER, _T("Software\\Classes\\Drive\\shellex\\ContextMenuHandlers\\") + g_sName, NULL, REG_SZ, (LPCVOID) g_sCLSID.GetString(), dwClsidCcb) != ERROR_SUCCESS ||
		SHSetValue(HKEY_CURRENT_USER, _T("Software\\Classes\\CLSID\\") + g_sCLSID, NULL, REG_SZ, (LPCVOID) g_sName.GetString(), dwNameCcb) != ERROR_SUCCESS ||
		SHSetValue(HKEY_CURRENT_USER, _T("Software\\Classes\\CLSID\\") + g_sCLSID + _T("\\InProcServer32"), NULL, REG_SZ, (LPCVOID) ku::sModulePath.GetString(), (DWORD) (ku::sModulePath.GetLength() + 1) * sizeof(TCHAR)) != ERROR_SUCCESS ||
		SHSetValue(HKEY_CURRENT_USER, _T("Software\\Classes\\CLSID\\") + g_sCLSID + _T("\\InProcServer32"), _T("ThreadingModel"), REG_SZ, (LPCVOID) _T("Apartment"), sizeof(_T("Apartment"))) != ERROR_SUCCESS)
		return SELFREG_E_CLASS;

	// Require admininstrator's rights!
	SHSetValue(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"), g_sCLSID, REG_SZ, (LPCVOID) g_sName.GetString(), dwNameCcb);

	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	return S_OK;
}

STDAPI DllUnregisterServer()
{
	if (!InitConfig())
		return SELFREG_E_CLASS;

	if (SHDeleteKey(HKEY_CURRENT_USER, _T("Software\\Classes\\*\\shellex\\ContextMenuHandlers\\") + g_sName) != ERROR_SUCCESS ||
		SHDeleteKey(HKEY_CURRENT_USER, _T("Software\\Classes\\Directory\\shellex\\ContextMenuHandlers\\") + g_sName) != ERROR_SUCCESS ||
		SHDeleteKey(HKEY_CURRENT_USER, _T("Software\\Classes\\Directory\\Background\\shellex\\ContextMenuHandlers\\") + g_sName) != ERROR_SUCCESS ||
		SHDeleteKey(HKEY_CURRENT_USER, _T("Software\\Classes\\LibraryFolder\\shellex\\ContextMenuHandlers\\") + g_sName) != ERROR_SUCCESS ||
		SHDeleteKey(HKEY_CURRENT_USER, _T("Software\\Classes\\LibraryFolder\\Background\\shellex\\ContextMenuHandlers\\") + g_sName) != ERROR_SUCCESS ||
		SHDeleteKey(HKEY_CURRENT_USER, _T("Software\\Classes\\Drive\\shellex\\ContextMenuHandlers\\") + g_sName) != ERROR_SUCCESS ||
		SHDeleteKey(HKEY_CURRENT_USER, _T("Software\\Classes\\CLSID\\") + g_sCLSID) != ERROR_SUCCESS)
		return SELFREG_E_CLASS;

	// Require admininstrator's rights!
	SHDeleteValue(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"), g_sCLSID);

	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	return S_OK;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID * ppv)
{
	InitConfig();

    IUnknown *pObj = NULL;

	if (rclsid == g_CLSID) {
		try {
			pObj = new CKuShellExtensionFactory;
			pObj->AddRef();
		} catch (...) { return E_OUTOFMEMORY; }
	}
    else
        return CLASS_E_CLASSNOTAVAILABLE;

    if (pObj) {
        if(SUCCEEDED(pObj->QueryInterface(riid, ppv)))
            // Release extra refcount from QueryInterface
			pObj->Release();
        else {
            delete pObj;
            return E_UNEXPECTED;
        }
    }
    else
        return E_OUTOFMEMORY;

	return S_OK;
}

STDAPI DllCanUnloadNow()
{
	ULONG uZero = 0;
	InterlockedExchange((LONG *) &uZero, CRefCount::m_uInstances);
	if (uZero == 0) {
		if (dll::GdiplusShutdown && ku::gdiplusToken) {
			dll::GdiplusShutdown(ku::gdiplusToken);
			ku::gdiplusToken = 0;
		}
		return S_OK;
	}
	return S_FALSE;
}
