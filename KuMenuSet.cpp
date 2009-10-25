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

#include "stdafx.h"
#include "KuMenuSet.h"
#include "KuContextMenu.h"
#include "StringConv.h"
#include "globals.h"
#include "dll.h"
#include "FSLinks/FSLinks.h"

#if defined(_MSC_VER) && defined(_DEBUG)
	#pragma push_macro("new")
	#undef new
#endif
#include "pugxml.h"
#if defined(_MSC_VER) && defined(_DEBUG)
	#pragma pop_macro("new")
#endif

#define PERCENT_EXPANSION_FLAGS		"dpnxs"

CKuMenuSet g_menu;

CAtlMap<CString, CString> CKuMenuSet::m_BuiltinVars;

CKuMenuSet::CKuMenuSet()
	: m_pFirstItem(NULL)
{
}

CKuMenuSet::~CKuMenuSet()
{
	if (m_pFirstItem)
		delete m_pFirstItem;
}

bool CKuMenuSet::FromFile(LPCTSTR sPath)
{
	ASSERT(sPath);

	HANDLE hFile;
	if ((hFile = CreateFile(sPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE)
		return false;
	return FromFile(hFile);
}

bool CKuMenuSet::FromFile(HANDLE hFile)
{
	ASSERT(hFile);

	LARGE_INTEGER uSize;
	bool ret = false;
	if (GetFileSizeEx(hFile, &uSize) != INVALID_FILE_SIZE && uSize.LowPart > 0 && uSize.HighPart == 0) { // > 4GB configure file is not supported
		DWORD uRead;
		BYTE *buffer;
		buffer = (BYTE *) malloc(uSize.LowPart + sizeof(TCHAR));
		memset(buffer + uSize.LowPart, 0, sizeof(TCHAR));
		if (ReadFile(hFile, (LPVOID) buffer, uSize.LowPart, &uRead, NULL))
			ret = !!FromRaw(buffer, uSize.LowPart);
		free(buffer);
	}

	CloseHandle(hFile);
	return ret;
}

bool CKuMenuSet::FromRaw(const BYTE *pXML, DWORD uLen)
{
#ifdef _UNICODE
	switch (DetectCodePage(pXML, uLen)) {
		case CP_UTF8:
			return FromString(CStringWCharFromUTF8((LPCSTR) pXML));
		case 1200: // UTF-16LE
			return FromString((LPCTSTR) pXML);
	}
#endif
	return false;
}

bool CKuMenuSet::FromString(LPCTSTR sXML)
{
	ASSERT(sXML);

#ifdef _UNICODE
	if (*sXML == L'\uFFFE') // skip BOM
		sXML++;

	pug::xml_parser xml((TCHAR *) sXML);
#ifdef _MSC_VER
	pug::xml_node&
#else
	pug::xml_node
#endif
		doc = xml.document();
	pug::xml_node_list vars;

	doc = doc.first_element_by_name(_T("config"));
	if (doc.empty())
		return false;

	if (m_pFirstItem)
		delete m_pFirstItem;
	m_pFirstItem = new CMenuItem(this);

	pug::xml_node node = doc;
	node.moveto_child((unsigned int) 0);

	if (dll::GdiplusStartup && !ku::gdiplusToken)
		dll::GdiplusStartup(&ku::gdiplusToken, &ku::gdiplusStartupInput, NULL);
	PraseMenuItems(node, m_pFirstItem);

	return true;
#else
	#error Always keep Unicode in mind!!!
#endif
}

struct ARGS {
	CString name;
	int argc;
	LPWSTR *argv;
};

void CKuMenuSet::PraseMenuItems(pug::xml_node &node, CMenuItem *pItem)
{
	ASSERT(pItem);
	do {
		if (!_tcsicmp(node.name(), _T("var"))) {
#ifdef _MSC_VER
			pug::xml_attribute&
#else
			pug::xml_attribute
#endif
				name = node.attribute(_T("name"));

			if (!_tcslen(name.value()))
				continue;
			CString str;
			CString value;
			if (node.children() > 0 && node.child(0).has_value())
				Substitute(node.child(0).value(), value, pItem);
			if (pItem->m_pParent)
				pItem->m_pParent->m_vars.SetAt(Substitute(name, str, pItem), value);
			else
				m_vars.SetAt(Substitute(name, str, pItem), value);
			TRACE(_T("%s=%s\n"), Substitute(name, str, pItem), value.GetString());
		}
		else if (!_tcsicmp(node.name(), _T("menu")) || !_tcsicmp(node.name(), _T("menuitem"))) {
			CString sIcon;
			Substitute(node.attribute(_T("name")).value(), pItem->m_sName, pItem);
			if (pItem->m_sName.IsEmpty())
				continue;
			Substitute(node.attribute(_T("class")).value(), pItem->m_sClasses, pItem);
			Substitute(node.attribute(_T("icon")).value(), sIcon, pItem);

			if (!_tcsicmp(node.name(), _T("menuitem")) && node.children() > 0) {
				CString str;
				if (!_tcsicmp(Substitute(node.attribute(_T("action")).value(), str, pItem), _T("builtin")))
					pItem->m_eAction = CMenuItem::ACT_BULITIN;
				else
					pItem->m_eAction = CMenuItem::ACT_EXECUTE;
				Substitute(node.child(0).value(), pItem->m_sAction, pItem);
				pItem->m_dwMultiItems = 1;
				if (node.has_attribute(_T("multiple")))
					pItem->m_dwMultiItems = (DWORD) _ttoi(Substitute(node.attribute(_T("multiple")).value(), str, pItem));
				else if (pItem->m_eAction == CMenuItem::ACT_EXECUTE) {
					for (LPCTSTR ptr = pItem->m_sAction;*ptr;ptr++) {
						if (ptr[0] == _T('%')) {
							if (ptr[1] == _T('~')) {
								ptr++;
								ptr += _tcsspn(ptr + 1, _T(PERCENT_EXPANSION_FLAGS));
							}
							if (ptr[1] == _T('*') || ptr[1] == _T('@') || ptr[1] == _T('l') || ptr[1] == _T('L') || ptr[1] == _T('u')) {
								pItem->m_dwMultiItems = 0;
								break;
							}
							else if (ptr[1] >= _T('2') && ptr[1] <= _T('9')) {
								DWORD n = (DWORD) ptr[1] - (DWORD) _T('0');
								if (pItem->m_dwMultiItems < n)
									pItem->m_dwMultiItems = n;
								ptr++;
							}
							else if (ptr[1] == _T('%'))
								ptr++;
						}
					}
				}
				bool bHideMissing = GetBoolean(_T("HIDE_MISSING"));
				if ((sIcon.IsEmpty() || bHideMissing) && pItem->m_eAction == CMenuItem::ACT_EXECUTE) {
					CString sProg = pItem->m_sAction;
					LPTSTR pIcon = sProg.GetBuffer();
					PathRemoveArgs(pIcon);
					PathUnquoteSpaces(pIcon);
					sProg.ReleaseBuffer();
					// drop the entries which use the missing programs.
					// we should check the file existence here, because checking them on-the-fly may be very slow.
					if (bHideMissing && !PathFileExists(sProg)) {
						pItem->m_sName.Empty();
						pItem->m_sClasses.Empty();
						pItem->m_eAction = CMenuItem::ACT_EXECUTE;
						pItem->m_bConsole = false;
						pItem->m_dwMultiItems = 1;
						continue;
					}	
					if (sIcon.IsEmpty())
						sIcon = sProg;
				}
				Substitute(node.attribute(_T("console")).value(), str, pItem);
				pItem->m_bConsole = !!str.CompareNoCase(_T("false"));
				Substitute(node.attribute(_T("workdir")).value(), pItem->m_sWorkingDir, pItem);
			}
			if (!sIcon.IsEmpty()) {
				LPTSTR pIcon = sIcon.GetBuffer();
				LPTSTR ptr = (LPTSTR) _tcsrchr(pIcon, _T(','));
				int i = 0;
				if (ptr) {
					*ptr++ = 0;
					i = _ttoi(ptr);
				}
#ifndef _WIN64
				PVOID oldWow64;
				if (dll::Wow64DisableWow64FsRedirection)
					dll::Wow64DisableWow64FsRedirection(&oldWow64);
#endif
				if (PathFileExists(pIcon)) {
					LPCTSTR sExt = _tcsrchr(pIcon, _T('.'));
					if (!sExt)
						sExt = _T("");
					else
						sExt++;
					if (!_tcsicmp(sExt, _T("png")) || !_tcsicmp(sExt, _T("gif")) || !_tcsicmp(sExt, _T("bmp")) ||
						!_tcsicmp(sExt, _T("jpg")) || !_tcsicmp(sExt, _T("tif")))
					{
						Gdiplus::GpBitmap *pBitmap = NULL;
						if (dll::GdipCreateBitmapFromFile(pIcon, &pBitmap) == Gdiplus::Ok) {
							dll::GdipCreateHICONFromBitmap(pBitmap, &pItem->m_hIcon);
							dll::GdipCreateHBITMAPFromBitmap(pBitmap, &pItem->m_hBitmap, Gdiplus::Color::Transparent);
						}
					}
					else {
						ExtractIconEx(pIcon, i, NULL, &pItem->m_hIcon, 1);
						if (pItem->m_hIcon && ku::SysVer.m_vMajor >= 6)
							pItem->m_hBitmap = CKuContextMenu::IconToBitmap(pItem->m_hIcon);
					}
				}
#ifndef _WIN64
				if (dll::Wow64RevertWow64FsRedirection)
					dll::Wow64RevertWow64FsRedirection(oldWow64);
#endif

				sIcon.ReleaseBuffer();
			}
			if (!_tcsicmp(node.name(), _T("menu")) && node.children() > 0) {
				pItem->m_pFirstChild = new CMenuItem(this);
				pItem->m_pFirstChild->m_pParent = pItem;
#ifdef _MSC_VER
				PraseMenuItems(node.child(0), pItem->m_pFirstChild);
#else
				pug::xml_node child = node.child(0);
				PraseMenuItems(child, pItem->m_pFirstChild);
#endif
			}
			// this waste some spaces, since the last node in each group is not used.
			pItem->m_pNextSibling = new CMenuItem(this);
			pItem->m_pNextSibling->m_pPrevSibling = pItem;
			pItem->m_pNextSibling->m_pParent = pItem->m_pParent;
			pItem->GetFirstSibling()->m_pPrevSibling = pItem->m_pNextSibling; // save last child/sibling
			pItem = pItem->m_pNextSibling;
		}
		else if (!_tcsicmp(node.name(), _T("for")) && node.children() > 0) {
			CAtlArray<ARGS> vars;
			ARGS args;
			int argc = 0;
			CString str;
			for (int i = 0;i < node.attributes();i++) {
				if (!node.attribute(i).has_value())
					continue;
				args.argv = CommandLineToArgvW(Substitute(node.attribute(i).value(), str, pItem), &(args.argc));
				if (args.argv) {
					args.name = node.attribute(i).name();
					vars.Add(args);
					argc = max(argc, args.argc);
				}
			}
			CMenuItem *pChild = NULL;
			for (int i = 0;i < argc;i++) {
				size_t n, count = vars.GetCount();
				for (n = 0;n < count;n++) {
					if (vars[n].argc > i)
						pItem->m_vars.SetAt(vars[n].name, vars[n].argv[i]);
				}
				if (pChild) {
					pChild->m_pNextSibling = new CMenuItem(this);
					pChild->m_pNextSibling->m_pPrevSibling = pChild;
					pChild = pChild->m_pNextSibling;
					pItem->m_pFirstChild->m_pPrevSibling = pChild; // save last child/sibling
				}
				else {
					pChild = new CMenuItem(this);
					pItem->m_pFirstChild = pChild;
				}
				pChild->m_pParent = pItem;
#ifdef _MSC_VER
				PraseMenuItems(node.child(0), pChild);
#else
				pug::xml_node child = node.child(0);
				PraseMenuItems(child, pChild);
#endif
				pChild = pItem->GetLastChild();
			}
			if (pItem->m_pFirstChild) {
				// mount children nodes to the parent of pItem
				for (pChild = pItem->m_pFirstChild;pChild;pChild = pChild->m_pNextSibling)
					pChild->m_pParent = pItem->m_pParent;
				pChild = pItem->GetLastChild();
				if (!pItem->IsFirst()) {
					pItem->m_pPrevSibling->m_pNextSibling = pItem->m_pFirstChild;
					pItem->m_pFirstChild->m_pPrevSibling = pItem->m_pPrevSibling;
				}
				else {
					if (pItem->m_pParent)
						pItem->m_pParent->m_pFirstChild = pItem->m_pFirstChild;
					else
						m_pFirstItem = pItem->m_pFirstChild;
				}
				pItem->GetFirstSibling()->m_pPrevSibling = pItem;
				pItem->m_pPrevSibling = pChild;
				pChild->m_pNextSibling = pItem;
				pItem->m_pFirstChild = NULL;
			}
			pItem->m_vars.RemoveAll();
			size_t n, count = vars.GetCount();
			for (n = 0;n < count;n++)
				LocalFree(vars[n].argv);
		}
	} while(node.moveto_next_sibling());
}

typedef enum {
	SUBST_INIT,
	SUBST_DOLLAR,
	SUBST_AND,
	SUBST_VAR
} SUBST_STATE;

LPCTSTR CKuMenuSet::Substitute(LPCTSTR src, CString &dest, CMenuItem *pItem)
{
	dest.Empty();

	if (src) {
		SUBST_STATE state = SUBST_INIT;
		CString var;

		for (;*src;src++) {
			switch (state) {
				case SUBST_INIT:
					switch (*src) {
						case _T('$'):
							state = SUBST_DOLLAR;
							break;
						case _T('&'):
							var.Empty();
							state = SUBST_AND;
							break;
						default:
							dest += *src;
							break;
					}
					break;
				case SUBST_DOLLAR:
					switch (*src) {
						case _T('$'):
							dest += _T('$');
							state = SUBST_INIT;
							break;
						case _T('{'):
							var.Empty();
							state = SUBST_VAR;
							break;
						default:
							dest.AppendFormat(_T("$%c"), *src);
							state = SUBST_INIT;
							break;
					}
					break;
				case SUBST_VAR:
					switch (*src) {
						case _T('}'):
							{
								CString str;
								if (GetVariable(var, str, pItem))
									dest += str;
								state = SUBST_INIT;
							}
							break;
						default:
							var += *src;
							break;
					}
					break;
				case SUBST_AND:
					switch (*src) {
						case _T(';'):
							if (!var.CompareNoCase(_T("lt")))
								dest += _T('<');
							else if (!var.CompareNoCase(_T("gt")))
								dest += _T('>');
							else if (!var.CompareNoCase(_T("nbsp")))
								dest += _T(' ');
							else if (!var.CompareNoCase(_T("amp")))
								dest += _T('&');
							else if (!var.CompareNoCase(_T("quot")))
								dest += _T('"');
							else if (var[0] == _T('#')) { 
								if (var[1] == _T('x'))
									dest += (wchar_t)  _tcstoul(var.GetString() + 2, NULL, 16);
								else
									dest += (wchar_t) _tcstoul(var.GetString() + 1, NULL, 10);
							}
							state = SUBST_INIT;
							break;
						default:
							var += *src;
							break;
					}
					break;
			}
		}
	}

	return dest;
}

bool CKuMenuSet::GetOurVariable(LPCTSTR key, CString &value, CMenuItem *pItem/* = NULL*/)
{
	if (pItem)
		for (;pItem->m_pParent;pItem = pItem->m_pParent)
			if (pItem->m_pParent->m_vars.Lookup(key, value))
				return true;
	return (m_vars.Lookup(key, value) || m_BuiltinVars.Lookup(key, value));
}

bool CKuMenuSet::GetVariable(LPCTSTR key, CString &value, CMenuItem *pItem/* = NULL*/)
{
	if (!_tcsnicmp(key, _T("env:"), 4))
		return !!value.GetEnvironmentVariable(key + 4);
	if (!_tcsnicmp(key, _T("var:"), 4))
		return GetOurVariable(key + 4, value, pItem);
	return (GetOurVariable(key, value, pItem) || value.GetEnvironmentVariable(key));
}

bool CKuMenuSet::GetBoolean(LPCTSTR key, bool bDefault/* = false*/, CMenuItem *pItem/* = NULL*/)
{
	CString sValue;
	return GetOurVariable(key, sValue) ? (!_tcscmp(sValue, _T("1")) || !_tcsicmp(sValue, _T("true"))) : bDefault;
}

bool CKuMenuSet::IsNumber(LPCTSTR str)
{
	for (;*str;str++)
		if (!_istdigit(*str))
			return false;
	return true;
}

UINT CKuMenuSet::DetectCodePage(const BYTE *pBuffer, DWORD uLen)
{
	// detect from BOM.
	if (uLen >= 4) {
		if (pBuffer[0] == 0xFF && pBuffer[1] == 0xFE && pBuffer[2] == 0x00 && pBuffer[3] == 0x00)
			return 12000; // UTF-32LE
		if (pBuffer[0] == 0x00 && pBuffer[1] == 0x00 && pBuffer[2] == 0xFE && pBuffer[3] == 0xFF)
			return 12001; // UTF-32BE
	}
	if (uLen >= 3) {
		if (pBuffer[0] == 0xEF && pBuffer[1] == 0xBB && pBuffer[2] == 0xBF)
			return CP_UTF8;
	}
	if (uLen >= 2) {
		if (pBuffer[0] == 0xFF && pBuffer[1] == 0xFE)
			return 1200; // UTF-16LE
		if (pBuffer[0] == 0xFF && pBuffer[1] == 0xFE)
			return 1201; // UTF-16BE
	}

	return CP_UTF8; // treat it as UTF-8 by default.
}

HRESULT CKuMenuSet::QueryContextMenu(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	CMenuItem *pItem;

	m_idCmdFirst = idCmdFirst; // used by submenu parents
	UINT id = idCmdFirst + 1; // start from offset 1

	if (GetBoolean(_T("LEGACY_STYLE")))
		m_bVistaStyle = false;
	else
		m_bVistaStyle = ku::SysVer.m_vMajor >= 6 && dll::IsThemeActive && dll::IsThemeActive();

	m_cmd.RemoveAll();
	for (pItem = m_pFirstItem;pItem;pItem = pItem->m_pNextSibling)
		pItem->QueryContextMenu(hMenu, indexMenu, id, idCmdLast, uFlags);

	return MAKE_HRESULT(SEVERITY_SUCCESS, 0, id - idCmdFirst);
}

void CKuMenuSet::InitBuiltinVars()
{
	m_BuiltinVars.RemoveAll();

	m_BuiltinVars.SetAt(_T("CONFIG"), ku::sConfigFile);
	m_BuiltinVars.SetAt(_T("KU_SHELL_EXTENSION_DIR"), ku::sModuleDir);
	m_BuiltinVars.SetAt(_T("KU_SHELL_EXTENSION_DRIVE"), ku::sModulePath.Left(2));

	if (dll::SHGetKnownFolderPath) {
		PWSTR sOut = NULL;

		#define FOLDERID_SETVAR(id, name) \
			if (sOut) { CoTaskMemFree(sOut); sOut = NULL; } \
			if (dll::SHGetKnownFolderPath(&(id), KF_FLAG_DONT_VERIFY, NULL, &sOut) == S_OK) \
				m_BuiltinVars.SetAt((name), sOut);

		FOLDERID_SETVAR(FOLDERID_Documents, _T("Documents"));
		FOLDERID_SETVAR(FOLDERID_PublicDocuments, _T("CommonDocuments"));
		FOLDERID_SETVAR(FOLDERID_Pictures, _T("Pictures"));
		FOLDERID_SETVAR(FOLDERID_PublicPictures, _T("CommonPictures"));
		FOLDERID_SETVAR(FOLDERID_Music, _T("Music"));
		FOLDERID_SETVAR(FOLDERID_PublicMusic, _T("CommonMusic"));
		FOLDERID_SETVAR(FOLDERID_Videos, _T("Video"));
		FOLDERID_SETVAR(FOLDERID_PublicVideos, _T("CommonVideo"));
		FOLDERID_SETVAR(FOLDERID_Desktop, _T("Desktop"));
		FOLDERID_SETVAR(FOLDERID_PublicDesktop, _T("CommonDesktop"));
		FOLDERID_SETVAR(FOLDERID_Startup, _T("Startup"));
		FOLDERID_SETVAR(FOLDERID_CommonStartup, _T("CommonStartup"));
		FOLDERID_SETVAR(FOLDERID_Downloads, _T("Downloads"));
		FOLDERID_SETVAR(FOLDERID_PublicDownloads, _T("CommonDownloads"));

#ifndef _WIN64
		if (ku::bIsWow64) {
#endif
			// Win64
			m_BuiltinVars.SetAt(_T("Arch"), _T("64"));
			m_BuiltinVars.SetAt(_T("ArchName"), _T("x64"));
			m_BuiltinVars.SetAt(_T("IsWin64"), _T("64"));

			FOLDERID_SETVAR(FOLDERID_ProgramFiles, _T("ProgramFiles"));
			FOLDERID_SETVAR(FOLDERID_ProgramFilesX86, _T("ProgramFiles32"));
#ifdef _WIN64
			// 64 on Win64
			FOLDERID_SETVAR(FOLDERID_ProgramFilesX64, _T("ProgramFiles64"));
			FOLDERID_SETVAR(FOLDERID_System, _T("SysDir"));
#else
			// WOW64
			// Oops, FOLDERID_ProgramFilesX64 isn't applicable in WOW64. MSDN cheat us. :(
			m_BuiltinVars.SetAt(_T("ProgramFiles64"), CString(_tgetenv(_T("ProgramW6432"))));
			FOLDERID_SETVAR(FOLDERID_SystemX86, _T("SysDir"));
#endif
			FOLDERID_SETVAR(FOLDERID_SystemX86, _T("SysDir32"));
			FOLDERID_SETVAR(FOLDERID_System, _T("SysDir64"));
#ifndef _WIN64
		}
		else {
			// Win32
			m_BuiltinVars.SetAt(_T("Arch"), _T("32"));
			m_BuiltinVars.SetAt(_T("ArchName"), _T("x86"));
			m_BuiltinVars.SetAt(_T("IsWin32"), _T("32"));
			FOLDERID_SETVAR(FOLDERID_ProgramFiles, _T("ProgramFiles"));
			m_BuiltinVars.SetAt(sOut, _T("ProgramFiles32"));
			m_BuiltinVars.SetAt(sOut, _T("ProgramFiles64"));
			FOLDERID_SETVAR(FOLDERID_System, _T("SysDir"));
			m_BuiltinVars.SetAt(sOut, _T("SysDir32"));
			m_BuiltinVars.SetAt(sOut, _T("SysDir64"));
		}
#endif // _WIN64

		if (sOut)
			CoTaskMemFree(sOut);
	}
	else {
		CString prog, sys, syswow;

		prog.GetEnvironmentVariable(_T("ProgramFiles"));
		m_BuiltinVars.SetAt(_T("ProgramFiles"), prog);

		#define CSIDL_SETVAR(id, name) \
			if (SHGetSpecialFolderPath(NULL, sys.GetBufferSetLength(KU_MAX_PATH), (id), FALSE)) \
			{ sys.ReleaseBuffer(); m_BuiltinVars.SetAt((name), sys); } else sys.ReleaseBuffer();

		CSIDL_SETVAR(CSIDL_MYDOCUMENTS, _T("Documents"));
		CSIDL_SETVAR(CSIDL_COMMON_DOCUMENTS, _T("CommonDocuments"));
		CSIDL_SETVAR(CSIDL_MYPICTURES, _T("Pictures"));
		CSIDL_SETVAR(CSIDL_COMMON_PICTURES, _T("CommonPictures"));
		CSIDL_SETVAR(CSIDL_MYMUSIC, _T("Music"));
		CSIDL_SETVAR(CSIDL_COMMON_MUSIC, _T("CommonMusic"));
		CSIDL_SETVAR(CSIDL_MYVIDEO, _T("Video"));
		CSIDL_SETVAR(CSIDL_COMMON_VIDEO, _T("CommonVideo"));
		CSIDL_SETVAR(CSIDL_DESKTOPDIRECTORY, _T("Desktop"));
		CSIDL_SETVAR(CSIDL_COMMON_DESKTOPDIRECTORY, _T("CommonDesktop"));
		CSIDL_SETVAR(CSIDL_STARTUP, _T("Startup"));
		CSIDL_SETVAR(CSIDL_COMMON_STARTUP, _T("CommonStartup"));

		#undef CSIDL_SETVAR

		SHGetSpecialFolderPath(NULL, sys.GetBufferSetLength(KU_MAX_PATH), CSIDL_SYSTEM, FALSE);
		sys.ReleaseBuffer();

#ifndef _WIN64
		if (ku::bIsWow64) {
#endif
			// Win64
			SHGetSpecialFolderPath(NULL, syswow.GetBufferSetLength(KU_MAX_PATH), CSIDL_SYSTEMX86, FALSE);
			syswow.ReleaseBuffer();
#ifndef _WIN64
			// Wow64
			m_BuiltinVars.SetAt(_T("SysDir"), syswow);
			prog.GetEnvironmentVariable(_T("ProgramW6432"));
#else
			m_BuiltinVars.SetAt(_T("SysDir"), sys); // 64 on Win64
#endif
			// Win64
			m_BuiltinVars.SetAt(_T("Arch"), _T("64"));
			m_BuiltinVars.SetAt(_T("ArchName"), _T("x64"));
			m_BuiltinVars.SetAt(_T("IsWin64"), _T("64"));
			m_BuiltinVars.SetAt(_T("ProgramFiles64"), prog);
			prog.GetEnvironmentVariable(_T("ProgramFiles(x86)"));
			m_BuiltinVars.SetAt(_T("ProgramFiles32"), prog);
			m_BuiltinVars.SetAt(_T("SysDir64"), sys);
			m_BuiltinVars.SetAt(_T("SysDir32"), syswow);
#ifndef _WIN64
		}
		else {
			// Win32
			m_BuiltinVars.SetAt(_T("Arch"), _T("32"));
			m_BuiltinVars.SetAt(_T("ArchName"), _T("x86"));
			m_BuiltinVars.SetAt(_T("IsWin32"), _T("32"));
			m_BuiltinVars.SetAt(_T("ProgramFiles32"), prog);
			m_BuiltinVars.SetAt(_T("ProgramFiles64"), prog);
			m_BuiltinVars.SetAt(_T("SysDir"), sys);
			m_BuiltinVars.SetAt(_T("SysDir32"), sys);
			m_BuiltinVars.SetAt(_T("SysDir64"), sys);
		}
#endif // _WIN64
	}
}

bool CKuMenuSet::CMenuItem::IsOurPath(LPCTSTR sPath, bool bDir)
{
	ASSERT(sPath);

	CString sClass, sClasses(m_sClasses);

	if (sClasses.IsEmpty())
		sClasses = _T('*');
	if (bDir)
		sClass = _T("folder");
	else
		sClass = PathFindExtension(sPath);

	CString sCurClass;
	int i = 0;
	while (!(sCurClass = sClasses.Tokenize(_T(" \t\r\n"), i)).IsEmpty()) {
		if (sCurClass == _T('*') && sClass != _T("folder"))
			return true;
		if (!sClass.CompareNoCase(sCurClass))
			return true;
	}

	return false;
}

CKuMenuSet::CMenuItem::CMD_ID CKuMenuSet::CMenuItem::GetCmdId(LPCTSTR sCmd)
{
	if (!_tcscmp(sCmd, _T("DropSymLinks")))
		return CMD_ID_DROP_SYMLINKS;
	else if (!_tcscmp(sCmd, _T("DropJunctions")))
		return CMD_ID_DROP_JUNCTIONS;
	else if (!_tcscmp(sCmd, _T("DropHardLinks")))
		return CMD_ID_DROP_HARDLINKS;
	else if (!_tcscmp(sCmd, _T("Reload")))
		return CMD_ID_RELOAD;
	return CMD_ID_NULL;
}

bool CKuMenuSet::CMenuItem::ShouldShown()
{
	if (m_sName.IsEmpty())
		return false;
	if (GetKeyState(VK_CONTROL) & 0x8000)
		return true;
	if (IsSubMenu()) {
		CMenuItem *pItem;
		for (pItem = m_pFirstChild;pItem;pItem = pItem->m_pNextSibling)
			if (!pItem->IsSeparator() && pItem->ShouldShown()) {
				if (m_sClasses.IsEmpty())
					return true;
				return IsOurPath(m_pKuMenuSet->m_pData->m_aFiles[0], m_pKuMenuSet->m_pData->m_bIsDirectory);
			}
		return false;
	}
	if (m_dwMultiItems != m_pKuMenuSet->m_pData->m_aFiles.GetCount() && m_dwMultiItems != 0 && !IsSeparator())
		return false;
	switch (m_eAction) {
		case ACT_BULITIN:
			{
				int argc;
				bool ret = false;
				LPWSTR *argv;
				argv = CommandLineToArgvW(m_sAction, &argc);
				if (argv) {
					if (argc > 0) {
						CMD_ID iCmd = GetCmdId(argv[0]);
						switch (iCmd) {
							case CMD_ID_DROP_SYMLINKS:
								if (!dll::CreateSymbolicLinkW)
									break;
							case CMD_ID_DROP_HARDLINKS:
							case CMD_ID_DROP_JUNCTIONS:
								if (OpenClipboard(NULL)) {
									HDROP hDrop = (HDROP) GetClipboardData(CF_HDROP);
									if (hDrop && DragQueryFile(hDrop, (UINT)-1, NULL, 0) > 0)
										ret = true;
									else {
										HGLOBAL hText = (HGLOBAL) GetClipboardData(CF_UNICODETEXT);
										if (hText) {
											LPCWSTR str = (LPCWSTR) GlobalLock(hText);
											if (str) {
												CString sLine;
												for (LPCWSTR ptr = str;*ptr && *ptr != L'\r' && *ptr != L'\n';ptr++)
													sLine += *ptr;
												ret = (_istalpha(sLine[0]) && sLine[1] == _T(':') || sLine[0] == _T('\\') && sLine[1] == _T('\\')) &&
													PathFileExists(sLine);
												GlobalUnlock(hText);
											}
										}
									}
									CloseClipboard();
								}
								break;
							case CMD_ID_RELOAD:
								ret = true;
								break;
						}
					}
					LocalFree(argv);
				}
				if (!ret)
					return false;
			}
			// no break!!
		case ACT_EXECUTE:
			if (m_sClasses.IsEmpty() && m_pParent)
				return m_pParent->IsOurPath(m_pKuMenuSet->m_pData->m_aFiles[0], m_pKuMenuSet->m_pData->m_bIsDirectory);
			return IsOurPath(m_pKuMenuSet->m_pData->m_aFiles[0], m_pKuMenuSet->m_pData->m_bIsDirectory);
	}
	return false;
}

void CKuMenuSet::CMenuItem::QueryContextMenu(HMENU hMenu, UINT &indexMenu, UINT &idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	ASSERT(hMenu);

	if (idCmdFirst >= idCmdLast || !ShouldShown())
		return;

	HMENU hSubMenu;
	if (IsSubMenu()) {
		hSubMenu = CreatePopupMenu();
		UINT i = 0;
		CMenuItem *pItem;

		for (pItem = m_pFirstChild;pItem;pItem = pItem->m_pNextSibling)
			pItem->QueryContextMenu(hSubMenu, i, idCmdFirst, idCmdLast, uFlags);
	}
	MENUITEMINFO mi = {0};
	mi.cbSize = sizeof(MENUITEMINFO);
	if (IsSeparator()) {
		if (indexMenu == 0)
			return;
		mi.fMask = MIIM_FTYPE;
		if (GetMenuItemInfo(hMenu, indexMenu - 1, TRUE, &mi) && (mi.fType & MFT_SEPARATOR))
			return;
		mi.fType = MFT_SEPARATOR;
	}
	else {
		mi.fMask = (IsSubMenu() ? MIIM_SUBMENU : 0) | MIIM_ID | MIIM_FTYPE | MIIM_STRING | MIIM_DATA |
			(m_hIcon ? MIIM_BITMAP : 0);
		mi.fType = MFT_STRING;
		mi.dwItemData = (ULONG_PTR) this;
		if (IsSubMenu()) {
			mi.hSubMenu = hSubMenu;
			mi.wID = m_pKuMenuSet->m_idCmdFirst;
		}
		else {
			m_pKuMenuSet->m_cmd.SetAt(idCmdFirst, this);
			mi.wID = idCmdFirst++;
		}
		mi.dwTypeData = m_sName.GetBuffer();
		mi.cch = m_sName.GetLength();
		if (m_hIcon)
			mi.hbmpItem = (m_hBitmap && m_pKuMenuSet->m_bVistaStyle) ? m_hBitmap : HBMMENU_CALLBACK;
	}

	::InsertMenuItem(hMenu, indexMenu++, TRUE, &mi);

	m_sName.ReleaseBuffer();
}

bool CKuMenuSet::CMenuItem::InvokeCommand()
{
	bool bResult = true;
	switch (m_eAction) {
		case ACT_EXECUTE:
			{
				CString sExpand;
				CShellExecuteThread *pShellExecuteThread = new CShellExecuteThread;

				pShellExecuteThread->m_bConsole = m_bConsole; 
				pShellExecuteThread->m_sWorkingDir = m_pKuMenuSet->m_pData->m_aFiles[0];
				if (!m_pKuMenuSet->m_pData->m_bFromFolderBk) {
					PathRemoveFileSpec(pShellExecuteThread->m_sWorkingDir.GetBuffer());
					pShellExecuteThread->m_sWorkingDir.ReleaseBuffer();
				}

				CAtlArray<CString> &aCmds = pShellExecuteThread->m_aCmds;
				aCmds.RemoveAll();
				aCmds.SetCount(1);
				aCmds[0].Empty();

				for (LPTSTR ptr = (LPTSTR) m_sAction.GetString();*ptr;ptr++) {
					if (ptr[0] == _T('%')) {
						LPCTSTR sFlags = ptr + 1;
						size_t count = m_pKuMenuSet->m_pData->m_aFiles.GetCount();
						if (count > 0) {
							if (ptr[1] == _T('~')) {
								ptr++;
								ptr += _tcsspn(ptr + 1, _T(PERCENT_EXPANSION_FLAGS));
							}
							if (ptr[1] == _T('*')) {
								ptr[1] = _T('\0');
								for (size_t i = 0;i < count;i++)
									for (size_t n = 0;n < aCmds.GetCount();n++)
										aCmds[n].AppendFormat(_T("%s\"%s\""), (i == 0 ? _T("") : _T(" ")), ExpandFileName(m_pKuMenuSet->m_pData->m_aFiles[i], sFlags, sExpand));
								ptr[1] = _T('*');
							}
							else if (ptr[1] == _T('@')) {
								ptr[1] = _T('\0');
								if (aCmds.GetCount() < count) {
									aCmds.SetCount(count); 
									for (size_t i = 1;i < count;i++)
										aCmds[i] = aCmds[0];
								}
								for (size_t i = 0;i < count;i++)
									aCmds[i].AppendFormat(_T("\"%s\""), ExpandFileName(m_pKuMenuSet->m_pData->m_aFiles[i], sFlags, sExpand));
								ptr[1] = _T('@');
							}
							else if (ptr[1] >= _T('1') && ptr[1] <= _T('9')) {
								TCHAR ch = ptr[1];
								ptr[1] = _T('\0');
								int i = ch - _T('1');
								if (i <= count - 1)
									for (size_t n = 0;n < aCmds.GetCount();n++)
										aCmds[n] += ExpandFileName(m_pKuMenuSet->m_pData->m_aFiles[i], sFlags, sExpand);
								ptr[1] = ch;
							}
							else if (ptr[1] == _T('w')) {
								ExpandFileName(pShellExecuteThread->m_sWorkingDir, sFlags, sExpand);
								for (size_t n = 0;n < aCmds.GetCount();n++)
									aCmds[n] += sExpand;
							}
							else if (ptr[1] == _T('z')) {
								LPCTSTR str = PathFindFileName((count > 1) ? pShellExecuteThread->m_sWorkingDir : m_pKuMenuSet->m_pData->m_aFiles[0]);
								if (str)
									for (size_t n = 0;n < aCmds.GetCount();n++)
										aCmds[n] += str;
							}
							else if (ptr[1] == _T('c')) {
								int i = -1;
								if (ptr[2] >= _T('0') && ptr[2] <= _T('9')) {
									i = ptr[2] - _T('0');
									ptr++;
								}
								if (!OpenClipboard(NULL))
									break;
								HDROP hDrop = (HDROP) GetClipboardData(CF_HDROP);
								if (hDrop) {
									TCHAR sFile[KU_MAX_PATH];
									UINT uCount = DragQueryFile(hDrop, (UINT)-1, NULL, 0);
									if (i != -1) {
										if ((UINT) i < uCount && DragQueryFile(hDrop, i, sFile, _countof(sFile)))
											for (size_t n = 0;n < aCmds.GetCount();n++)
												aCmds[n] += ExpandFileName(sFile, sFlags, sExpand);
									}
									else {
										for (i = 0;(UINT) i < uCount;i++) {
											if (DragQueryFile(hDrop, i, sFile, _countof(sFile)))
												for (size_t n = 0;n < aCmds.GetCount();n++)
													aCmds[n].AppendFormat(_T("%s\"%s\""), (i == 0 ? _T("") : _T(" ")), ExpandFileName(sFile, sFlags, sExpand));
										}
									}
								}
								CloseClipboard();
							}
							else if (ptr[1] >= _T('L') || ptr[1] >= _T('l') || ptr[1] >= _T('u')) {
								size_t offset = _tcsspn(ptr + 2, _T("0123456789"));
								int iFrom = 0;
								if (offset > 0)
									_stscanf(ptr + 2, _T("%d"), &iFrom);
								if (pShellExecuteThread->m_sTempFile.IsEmpty()) {
									TCHAR sTempPath[KU_MAX_PATH];
									if (GetTempPath(_countof(sTempPath), sTempPath)) {
										if (GetTempFileName(sTempPath, _T("ku."), 0, pShellExecuteThread->m_sTempFile.GetBufferSetLength(KU_MAX_PATH))) {
											pShellExecuteThread->m_sTempFile.ReleaseBuffer();
											HANDLE hFile = CreateFile(pShellExecuteThread->m_sTempFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
											if (hFile != INVALID_HANDLE_VALUE) {
												BYTE bom[3];
												DWORD dwWritten;
												BYTE eol[4];

												switch (ptr[1]) {
													case _T('L'):
														bom[0] = 0xFF;
														bom[1] = 0xFE;
														*((wchar_t *)eol) = L'\r';
														*(((wchar_t *)eol) + 1) = L'\n';
														if (WriteFile(hFile, bom, 2, &dwWritten, NULL)) {
															for (int i = iFrom;i < count;i++) {
																ExpandFileName(m_pKuMenuSet->m_pData->m_aFiles[i], sFlags, sExpand);
																if (!WriteFile(hFile, (LPCVOID) sExpand.GetString(), sExpand.GetLength() * sizeof(wchar_t), &dwWritten, NULL) ||
																	!WriteFile(hFile, (LPCVOID) eol, 4, &dwWritten, NULL))
																	break;
															}
														}
														break;
													case _T('l'):
														eol[0] = '\r';
														eol[1] = '\n';
														for (int i = iFrom;i < count;i++) {
															CStringCharFromWChar sFile(ExpandFileName(m_pKuMenuSet->m_pData->m_aFiles[i], sFlags, sExpand));
															if (!WriteFile(hFile, (LPCVOID) sFile.GetString(), sFile.GetLength() * sizeof(char), &dwWritten, NULL) ||
																!WriteFile(hFile, (LPCVOID) eol, 2, &dwWritten, NULL))
																break;
														}
														break;
													case _T('u'):
														bom[0] = 0xEF;
														bom[1] = 0xBB;
														bom[2] = 0xBF;
														eol[0] = '\r';
														eol[1] = '\n';
														if (WriteFile(hFile, bom, 3, &dwWritten, NULL)) {
															for (int i = iFrom;i < count;i++) {
																CStringUTF8FromWChar sFile(ExpandFileName(m_pKuMenuSet->m_pData->m_aFiles[i], sFlags, sExpand));
																if (!WriteFile(hFile, (LPCVOID) sFile.GetString(), sFile.GetLength() * sizeof(char), &dwWritten, NULL) ||
																	!WriteFile(hFile, (LPCVOID) eol, 2, &dwWritten, NULL))
																	break;
															}
														}
														break;
												}
												for (size_t n = 0;n < aCmds.GetCount();n++)
													aCmds[n] += pShellExecuteThread->m_sTempFile;
												CloseHandle(hFile);
											}
											else
												pShellExecuteThread->m_sTempFile.Empty();
										}
										else
											pShellExecuteThread->m_sTempFile.ReleaseBuffer();
									}
								}

								ptr += offset;
							}
						}
						if (ptr[1] == _T('%'))
							for (size_t n = 0;n < aCmds.GetCount();n++)
								aCmds[n] += _T('%');
						ptr++;
					}
					else
						for (size_t n = 0;n < aCmds.GetCount();n++)
							aCmds[n] += ptr[0];
				}

				if (!m_sWorkingDir.IsEmpty()) {
					if (m_sWorkingDir == _T(".")) {
						pShellExecuteThread->m_sWorkingDir = pShellExecuteThread->m_aCmds[0];
						LPTSTR str = pShellExecuteThread->m_sWorkingDir.GetBuffer();
						PathRemoveArgs(str);
						PathUnquoteSpaces(str);
						PathRemoveFileSpec(str);
						pShellExecuteThread->m_sWorkingDir.ReleaseBuffer();
					}
					else if (!_tcsncmp(m_sWorkingDir, _T(".\\"), 2) && m_pKuMenuSet->m_pData->m_bIsDirectory)
						pShellExecuteThread->m_sWorkingDir = m_pKuMenuSet->m_pData->m_aFiles[0] + (m_sWorkingDir.GetString() + 1);
					else
						pShellExecuteThread->m_sWorkingDir = m_sWorkingDir;
				}

				HANDLE hThread = CreateThread(NULL, 0, ShellExecuteThread, (LPVOID) pShellExecuteThread, 0, NULL);
				if (hThread)
					CloseHandle(hThread);
			}
			break;
		case ACT_BULITIN:
			{
				int argc;
				LPWSTR *argv;
				argv = CommandLineToArgvW(m_sAction, &argc);
				if (argv) {
					if (argc > 0) {
						CMD_ID iCmd = GetCmdId(argv[0]);
						switch (iCmd) {
							case CMD_ID_DROP_SYMLINKS:
								if (!dll::CreateSymbolicLinkW) {
									bResult = false;
									break;
								}
							case CMD_ID_DROP_HARDLINKS:
							case CMD_ID_DROP_JUNCTIONS:
								{
									if (!OpenClipboard(NULL)) {
										bResult = false;
										break;
									}
									DWORD uFlags = (iCmd == CMD_ID_DROP_SYMLINKS ? DROP_SYMBOLIC : (iCmd == CMD_ID_DROP_JUNCTIONS ? DROP_JUNCTION : DROP_HARDLINK)) |
										((argc > 1 && !_tcscmp(argv[1], _T("1"))) ? DROP_ABSOLUTE : 0);
									HDROP hDrop = (HDROP) GetClipboardData(CF_HDROP);
									if (hDrop) {
										TCHAR sFile[KU_MAX_PATH];
										UINT uCount = DragQueryFile(hDrop, (UINT)-1, NULL, 0);
										for (UINT i = 0;i < uCount;i++) {
											if (DragQueryFile(hDrop, i, sFile, _countof(sFile)))
												DropLinks(m_pKuMenuSet->m_pData->m_aFiles[0], sFile, uFlags);
										}
									}
									else {
										HGLOBAL hText = (HGLOBAL) GetClipboardData(CF_UNICODETEXT);
										if (hText) {
											LPCWSTR str = (LPCWSTR) GlobalLock(hText);
											if (str) {
												CString sText(str);
												GlobalUnlock(hText);
												CString sLine;

												int i = 0;
												while (!(sLine = sText.Tokenize(_T("\t\r\n"), i)).IsEmpty()) {
													if (PathFileExists(sLine))
														DropLinks(m_pKuMenuSet->m_pData->m_aFiles[0], sLine, uFlags);
												}
											}
										}
									}
									CloseClipboard();
								}
								break;
							case CMD_ID_RELOAD:
								ku::cfgFileInfo.nFileSizeLow = ku::cfgFileInfo.nFileSizeHigh = 0; // will reload at the next time
								break;
						}
					}
					LocalFree(argv);
				}
			}
			break;
	}

	return bResult;
}

DWORD WINAPI CKuMenuSet::CMenuItem::ShellExecuteThread(LPVOID lpParameter)
{
	if (!lpParameter)
		return 1;
	CShellExecuteThread *pShellExecuteThread = (CShellExecuteThread *) lpParameter;
	CAtlArray<CString> &aCmds = pShellExecuteThread->m_aCmds;

#ifndef _WIN64
	PVOID oldWow64;
	if (dll::Wow64DisableWow64FsRedirection)
		dll::Wow64DisableWow64FsRedirection(&oldWow64);
#endif

	SHELLEXECUTEINFO shexec = {0};
	shexec.cbSize = sizeof(SHELLEXECUTEINFO);
	shexec.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_DDEWAIT | SEE_MASK_UNICODE | (pShellExecuteThread->m_bConsole ? 0 : SEE_MASK_NO_CONSOLE);
	shexec.nShow = (pShellExecuteThread->m_bConsole ? SW_SHOWNORMAL : SW_HIDE);
	shexec.lpDirectory = pShellExecuteThread->m_sWorkingDir;

	size_t i, count = aCmds.GetCount();
	for (i = 0;i < count;i++) {
		TRACE(_T("aCmds[%d]=%s\n"), i, aCmds[i].GetString());
		CString sFile;
		if (pShellExecuteThread->m_bConsole) {
			sFile = aCmds[i];
			LPTSTR pFile = sFile.GetBuffer();
			PathRemoveArgs(pFile);
			PathUnquoteSpaces(pFile);
			sFile.ReleaseBuffer();
			shexec.lpFile = sFile.GetString();
		}
		else
			shexec.lpFile = _T("cmd.exe");
		shexec.hProcess = 0;
		if (pShellExecuteThread->m_bConsole)
			shexec.lpParameters = PathGetArgs(aCmds[i]);
		else {
			sFile.Format(_T("/c \"%s\""), aCmds[i].GetString());
			shexec.lpParameters = sFile;
		}
		ShellExecuteEx(&shexec);
		if (shexec.hProcess) {
			if (i < count - 1 || !pShellExecuteThread->m_sTempFile.IsEmpty()) // don't wait for the last process
				WaitForSingleObject(shexec.hProcess, INFINITE);
			CloseHandle(shexec.hProcess);
		}
	}

#ifndef _WIN64
	if (dll::Wow64RevertWow64FsRedirection)
		dll::Wow64RevertWow64FsRedirection(oldWow64);
#endif

	delete pShellExecuteThread;
	return 0;
}

inline bool IsAcpCompatible(LPCWSTR str)
{
	BOOL bUsed = FALSE;
	WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, str, -1, NULL, 0, NULL, &bUsed);
	return !bUsed;
}

#ifdef __GNUC__
#define _CHAR wchar_t
_CHAR * __cdecl wcstok_s(_CHAR *_String, const _CHAR *_Control, _CHAR **_Context)
{
    _CHAR *token;
    const _CHAR *ctl;

    /* If string==NULL, continue with previous string */
    if (!_String)
    {
        _String = *_Context;
    }

    /* Find beginning of token (skip over leading delimiters). Note that
    * there is no token iff this loop sets string to point to the terminal null. */
    for ( ; *_String != 0 ; _String++)
    {
        for (ctl = _Control; *ctl != 0 && *ctl != *_String; ctl++)
            ;
        if (*ctl == 0)
        {
            break;
        }
    }

    token = _String;

    /* Find the end of the token. If it is not the end of the string,
    * put a null there. */
    for ( ; *_String != 0 ; _String++)
    {
        for (ctl = _Control; *ctl != 0 && *ctl != *_String; ctl++)
            ;
        if (*ctl != 0)
        {
            *_String++ = 0;
            break;
        }
    }

    /* Update the context */
    *_Context = _String;

    /* Determine if a token has been found. */
    if (token == _String)
    {
        return NULL;
    }
    else
    {
        return token;
    }
}
#undef _CHAR
#endif

LPCWSTR GetAcpCompatiblePath(LPCWSTR sPath, LPWSTR sDest, DWORD cch = MAX_PATH)
{
	if (!sDest)
		return NULL;

	LPWSTR sLong = wcsdup(sPath);
	LPWSTR sShort = (LPWSTR) malloc(cch * sizeof(wchar_t));
	GetShortPathName(sPath, sShort, cch);
	LPWSTR sLongCtx = NULL, sShortCtx = NULL;
	LPWSTR sLongTok = wcstok_s(sLong, L"/\\", &sLongCtx), sShortTok = wcstok_s(sShort, L"/\\", &sShortCtx);
	LPWSTR ptr = sDest, sTok;

	cch--;
	sDest[cch] = 0;
	while (cch > 0 && sLongTok && sShortTok) {
		if (ptr != sDest) {
			*ptr++ = L'\\';
			cch--;
		}
		sTok = IsAcpCompatible(sLongTok) ? sLongTok : sShortTok;
		for (;cch > 0 && (*ptr = *sTok++);) {
			cch--;
			ptr++;
		}
		sLongTok = wcstok_s(NULL, L"/\\", &sLongCtx);
		sShortTok = wcstok_s(NULL, L"/\\", &sShortCtx);
	}

	free(sLong);
	free(sShort);

	return sDest;
}

LPCTSTR CKuMenuSet::CMenuItem::ExpandFileName(LPCTSTR sName, LPCTSTR sFlags, CString &sDest)
{
	ASSERT(sName);
	ASSERT(sFlags);

	if (*sFlags == _T('~')) {
		CString sShortName;
		sDest.Empty();
		sFlags++;
		if (_tcschr(sFlags, _T('s'))) {
			GetAcpCompatiblePath(sName, sShortName.GetBufferSetLength(KU_MAX_PATH), KU_MAX_PATH);
			sShortName.ReleaseBuffer();
			sName = sShortName;

			if (!_tcscmp(sFlags, _T("s"))) {
				sDest = sName;
				return sDest;
			}
		}
		if (_tcschr(sFlags, _T('d'))) {
			int iDrive = PathGetDriveNumber(sName);
			if (iDrive != -1)
				sDest.Format(_T("%c:"), (TCHAR) iDrive + _T('A'));
		}
		LPTSTR sFileName = (LPTSTR) PathFindFileName(sName);
		if (_tcschr(sFlags, _T('p'))) {
			LPCTSTR sPath = _tcschr(sName, _T('\\'));
			if (sPath && sFileName >= sPath) {
				TCHAR ch = *sFileName;
				*sFileName = _T('\0');
				sDest += sPath;
				*sFileName = ch;
			}
		}
		if (sFileName) {
			LPTSTR sExt = (LPTSTR) _tcsrchr(sFileName, _T('.'));
			if (_tcschr(sFlags, _T('n'))) {
				if (sExt > sFileName) {
					*sExt = _T('\0');
					sDest += sFileName;
					*sExt = _T('.');
				}
				else
					sDest += sFileName;
			}
			if (_tcschr(sFlags, _T('x'))) {
				if (sExt)
					sDest += sExt;
			}
		}
	}
	else
		sDest = sName;

	return sDest;
}

bool CKuMenuSet::CMenuItem::DropLinks(LPCTSTR sDir, LPCTSTR sPath, DWORD uFlags)
{
	ASSERT(sDir);
	ASSERT(sPath);

	DWORD uType = (uFlags & 0xF);

	if (uType == DROP_SYMBOLIC && !dll::CreateSymbolicLinkW ||
		uType == DROP_JUNCTION && !PathIsDirectory(sPath) ||
		uType == DROP_HARDLINK && PathIsDirectory(sPath))
		return false;

	CString sLink, sTarget;
	LPCTSTR sFileName = PathFindFileName(sPath);
	if (!sFileName)
		return false;

	sLink.Format(_T("%s\\%s"), sDir, sFileName);
	LPCTSTR sType = (uType == DROP_SYMBOLIC ? _T("SymLink") : (uType == DROP_JUNCTION ? _T("Junction") : _T("HardLink")));
	for (int i = 0;PathFileExists(sLink);i++) {
		if (i == 0)
			sLink.Format(_T("%s\\%s of %s"), sDir, sType, sFileName);
		else
			sLink.Format(_T("%s\\%s %d of %s"), sDir, sType, i, sFileName);
	}

	if (uType == DROP_SYMBOLIC && !(uFlags & DROP_ABSOLUTE)) {
		BOOL bOk = PathRelativePathTo(sTarget.GetBufferSetLength(KU_MAX_PATH), sLink, 0, sPath, 0);
		sTarget.ReleaseBuffer();
		if (!bOk)
			sTarget = sPath;
	}
	else
		sTarget = sPath;

	if (uType == DROP_SYMBOLIC)
		return !!dll::CreateSymbolicLinkW(sLink, _tcsncmp(sTarget, _T(".\\"), 2) ? sTarget.GetString() : sTarget.GetString() + 2, PathIsDirectory(sPath) ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0);
	else if (uType == DROP_JUNCTION) {
		if (!CreateDirectory(sLink, NULL))
			return false;
		if (!FSLinks::CreateJunctionPoint(sLink, sTarget)) {
			RemoveDirectory(sLink);
			return false;
		}
		return true;
	}
	else if (uType == DROP_HARDLINK)
		return !!CreateHardLink(sLink, sTarget, NULL);

	return false;
}
