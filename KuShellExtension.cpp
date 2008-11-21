// KuShellExtension.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "KuShellExtension.h"
#include "KuContextMenu.h"
#include "globals.h"
#include "dll.h"

#ifdef _WIN64
#define KU_SHELL_EXTENSION_STR _T("!!KuShellExtension64")
#else
#define KU_SHELL_EXTENSION_STR _T("!!KuShellExtension32")
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			{
				_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); //detect memory leaks

				GetModuleFileName(NULL, ku::sModulePath, KU_MAX_PATH);
				LPCTSTR sProcessName = PathFindFileName(ku::sModulePath);
				if (!sProcessName)
					return FALSE;
				int i;
				for (i = 0;ku::sBlackList[i];i++)
					if (!_tcsicmp(sProcessName, ku::sBlackList[i]))
						return FALSE;

				ku::hModule = hModule;
				GetModuleFileName(hModule, ku::sModulePath, KU_MAX_PATH);
				dll::Init();
				ku::SysVer.m_dwVersion = GetVersion();
#ifndef _WIN64
				if (dll::IsWow64Process)
					if (!dll::IsWow64Process(GetCurrentProcess(), &ku::bIsWow64))
						ku::bIsWow64 = FALSE;
#endif
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

STDAPI DllRegisterServer(void)
{
	HKEY hKey;
	HRESULT ret = S_OK;

	if (RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\Classes\\*\\shellex\\ContextMenuHandlers\\") KU_SHELL_EXTENSION_STR,
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) != ERROR_SUCCESS)
		return SELFREG_E_CLASS;
	if (RegSetValueEx(hKey, NULL, 0, REG_SZ, (const BYTE *) CLSID_KU_CONTEXT_MENU_STR, ARRSIZE(CLSID_KU_CONTEXT_MENU_STR) * sizeof(TCHAR)) != ERROR_SUCCESS)
		ret = SELFREG_E_CLASS;
	RegCloseKey(hKey);

	if (RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\Classes\\Directory\\shellex\\ContextMenuHandlers\\") KU_SHELL_EXTENSION_STR,
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) != ERROR_SUCCESS)
		return SELFREG_E_CLASS;
	if (RegSetValueEx(hKey, NULL, 0, REG_SZ, (const BYTE *) CLSID_KU_CONTEXT_MENU_STR, ARRSIZE(CLSID_KU_CONTEXT_MENU_STR) * sizeof(TCHAR)) != ERROR_SUCCESS)
		ret = SELFREG_E_CLASS;
	RegCloseKey(hKey);

	if (RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\Classes\\Directory\\Background\\shellex\\ContextMenuHandlers\\") KU_SHELL_EXTENSION_STR,
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) != ERROR_SUCCESS)
		return SELFREG_E_CLASS;
	if (RegSetValueEx(hKey, NULL, 0, REG_SZ, (const BYTE *) CLSID_KU_CONTEXT_MENU_STR, ARRSIZE(CLSID_KU_CONTEXT_MENU_STR) * sizeof(TCHAR)) != ERROR_SUCCESS)
		ret = SELFREG_E_CLASS;
	RegCloseKey(hKey);

	if (RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\Classes\\Drive\\shellex\\ContextMenuHandlers\\") KU_SHELL_EXTENSION_STR,
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) != ERROR_SUCCESS)
		return SELFREG_E_CLASS;
	if (RegSetValueEx(hKey, NULL, 0, REG_SZ, (const BYTE *) CLSID_KU_CONTEXT_MENU_STR, ARRSIZE(CLSID_KU_CONTEXT_MENU_STR) * sizeof(TCHAR)) != ERROR_SUCCESS)
		ret = SELFREG_E_CLASS;
	RegCloseKey(hKey);

	if (RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\Classes\\CLSID\\") CLSID_KU_CONTEXT_MENU_STR,
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) != ERROR_SUCCESS)
		return SELFREG_E_CLASS;
	if (RegSetValueEx(hKey, NULL, 0, REG_SZ, (const BYTE *) KU_SHELL_EXTENSION_STR, ARRSIZE(KU_SHELL_EXTENSION_STR) * sizeof(TCHAR)) != ERROR_SUCCESS)
			ret = SELFREG_E_CLASS;
	RegCloseKey(hKey);

	if (RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\Classes\\CLSID\\") CLSID_KU_CONTEXT_MENU_STR _T("\\InProcServer32"),
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) != ERROR_SUCCESS)
		return SELFREG_E_CLASS;
	else if (RegSetValueEx(hKey, NULL, 0, REG_SZ, (const BYTE *) ku::sModulePath, (DWORD) _tcslen(ku::sModulePath) * sizeof(TCHAR)) != ERROR_SUCCESS ||
		RegSetValueEx(hKey, _T("ThreadingModel"), 0, REG_SZ, (const BYTE *) _T("Apartment"), ARRSIZE(_T("Apartment")) * sizeof(TCHAR)) != ERROR_SUCCESS)
			ret = SELFREG_E_CLASS;
	RegCloseKey(hKey);

	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"),
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS)
		RegSetValueEx(hKey, CLSID_KU_CONTEXT_MENU_STR, 0, REG_SZ, (const BYTE *) KU_SHELL_EXTENSION_STR, ARRSIZE(KU_SHELL_EXTENSION_STR) * sizeof(TCHAR));

	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	return ret;
}

STDAPI DllUnregisterServer(void)
{
	if (SHDeleteKey(HKEY_CURRENT_USER, _T("Software\\Classes\\*\\shellex\\ContextMenuHandlers\\") KU_SHELL_EXTENSION_STR) != ERROR_SUCCESS ||
		SHDeleteKey(HKEY_CURRENT_USER, _T("Software\\Classes\\Directory\\shellex\\ContextMenuHandlers\\") KU_SHELL_EXTENSION_STR) != ERROR_SUCCESS ||
		SHDeleteKey(HKEY_CURRENT_USER, _T("Software\\Classes\\Directory\\Background\\shellex\\ContextMenuHandlers\\") KU_SHELL_EXTENSION_STR) != ERROR_SUCCESS ||
		SHDeleteKey(HKEY_CURRENT_USER, _T("Software\\Classes\\Drive\\shellex\\ContextMenuHandlers\\") KU_SHELL_EXTENSION_STR) != ERROR_SUCCESS ||
		SHDeleteKey(HKEY_CURRENT_USER, _T("Software\\Classes\\CLSID\\") CLSID_KU_CONTEXT_MENU_STR) != ERROR_SUCCESS)
		return SELFREG_E_CLASS;

	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	return S_OK;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID * ppv)
{
    IUnknown *pResult = 0;

    if (rclsid == CLSID_KU_CONTEXT_MENU)
        pResult = (IUnknown *)(IClassFactory *) new CKuContextMenuFactory;
    else
        return CLASS_E_CLASSNOTAVAILABLE;

    if (pResult) {
        if(SUCCEEDED(pResult->QueryInterface(riid, ppv)))
            // Release extra refcount from QueryInterface
            pResult->Release();
        else {
            delete pResult;
            return E_UNEXPECTED;
        }
    }
    else
        return E_OUTOFMEMORY;

	return S_OK;
}

STDAPI DllCanUnloadNow(void)
{
	ULONG uZero = 0;
	InterlockedExchange((LONG *) &uZero, CUnknown::g_uRefCount);
	if (uZero == 0)
		return S_OK;
	return S_FALSE;
}
