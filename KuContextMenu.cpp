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
#include "KuContextMenu.h"
#include "dll.h"
#include "globals.h"

//////////////////////////////////////////////////////////////////////////////////////
// CKuContextMenu
//////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CKuContextMenu::QueryInterface( 
	/* [in] */ REFIID riid,
	/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if (riid == IID_IShellExtInit) {
		CKuShellExtInit *p = new CKuShellExtInit;
		p->AddRef();
		m_pData = p->m_pData;
		*ppvObject = p;
		return S_OK;
	}
	else if (riid == IID_IContextMenu3 || riid == IID_IContextMenu2 || riid == IID_IContextMenu || riid == IID_IUnknown)
		*ppvObject = this;
	else {
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}
	AddRef();
	return S_OK;
}

STDMETHODIMP CKuContextMenu::QueryContextMenu( 
	/* [in] */ 
	HMENU hMenu,
	/* [in] */ 
	UINT indexMenu,
	/* [in] */ 
	UINT idCmdFirst,
	/* [in] */ 
	UINT idCmdLast,
	/* [in] */ 
	UINT uFlags)
{
	if (!m_pData)
		return E_INVALIDARG;
	g_menu.m_pData = m_pData;
	m_pData = NULL;

	HRESULT hr = ku::LoadConfig();
	if (hr != S_OK)
		return hr;

	m_idCmdFirst = idCmdFirst;
	if(!(CMF_DEFAULTONLY & uFlags))
		return g_menu.QueryContextMenu(hMenu, indexMenu, idCmdFirst, idCmdLast, uFlags);

	return S_OK;
}

#ifndef CMIC_MASK_UNICODE
#include <pshpack8.h>
typedef struct _CMINVOKECOMMANDINFOEX
    {
    DWORD cbSize;
    DWORD fMask;
    HWND hwnd;
    LPCSTR lpVerb;
    LPCSTR lpParameters;
    LPCSTR lpDirectory;
    int nShow;
    DWORD dwHotKey;
    HANDLE hIcon;
    LPCSTR lpTitle;
    LPCWSTR lpVerbW;
    LPCWSTR lpParametersW;
    LPCWSTR lpDirectoryW;
    LPCWSTR lpTitleW;
    POINT ptInvoke;
    } 	CMINVOKECOMMANDINFOEX;
#include <poppack.h>
#define CMIC_MASK_UNICODE       SEE_MASK_UNICODE
#endif

STDMETHODIMP CKuContextMenu::InvokeCommand( 
	/* [in] */ 
	CMINVOKECOMMANDINFO *pici)
{
	CMINVOKECOMMANDINFOEX *piciex = (CMINVOKECOMMANDINFOEX *) pici;

	bool bUnicode = pici->cbSize == sizeof(CMINVOKECOMMANDINFOEX) && (pici->fMask & CMIC_MASK_UNICODE);

	// pici->lpVerb == 0 or pici->lpVerbW == 0 are invalid because we offer IDs from offset 1
	UINT id;
	if (pici->lpVerb && !HIWORD(pici->lpVerb)) // it seems lpVerb is more reliable than lpVerbW
		id = LOWORD(pici->lpVerb);
	else if (bUnicode && piciex->lpVerbW && !HIWORD(piciex->lpVerbW))
		id = LOWORD(piciex->lpVerbW);
	else
		id = *(bUnicode ? (WORD *) piciex->lpVerbW : (WORD *) pici->lpVerb);
	if (!g_menu.InvokeCommand(id + m_idCmdFirst))
		return E_FAIL;

	return S_OK;
}

STDMETHODIMP CKuContextMenu::GetCommandString( 
	/* [in] */ 
	UINT_PTR idCmd,
	/* [in] */ 
	UINT uType,
	/* [in] */ 
	UINT *pReserved,
	/* [out] */ 
	LPSTR pszName,
	/* [in] */ 
	UINT cchMax)
{
	// stores menu ID in the buffer for verb.
	switch (uType) {
		case GCS_VERBA:
			if ((cchMax * sizeof(char)) < sizeof(WORD))
				return E_OUTOFMEMORY;
			*((WORD *) pszName) = idCmd & 0xFFFF;
			break;
		case GCS_VERBW:
			if ((cchMax * sizeof(wchar_t)) < sizeof(WORD))
				return E_OUTOFMEMORY;
			*((WORD *) pszName) = idCmd & 0xFFFF;
			break;
	}
    return S_OK;
}

STDMETHODIMP CKuContextMenu::HandleMenuMsg( 
	/* [in] */ 
	UINT uMsg,
	/* [in] */ 
	WPARAM wParam,
	/* [in] */ 
	LPARAM lParam)
{
	return HandleMenuMsg2(uMsg, wParam, lParam, NULL);
}

/*
	codes from TortoiseSVN
*/
STDMETHODIMP CKuContextMenu::HandleMenuMsg2( 
	/* [in] */ 
	UINT uMsg,
	/* [in] */ 
	WPARAM wParam,
	/* [in] */ 
	LPARAM lParam,
	/* [out] */ 
	LRESULT *pResult)
{
	LRESULT res;
	if (pResult == NULL)
		pResult = &res;
	*pResult = FALSE;

	switch (uMsg)
	{
		case WM_MEASUREITEM:
			{
				MEASUREITEMSTRUCT* lpmis = (MEASUREITEMSTRUCT*) lParam;
				if (!lpmis)
					break;
				lpmis->itemWidth += 2;
				if (lpmis->itemHeight < 16)
					lpmis->itemHeight = 16;
				*pResult = TRUE;
			}
			break;
		case WM_DRAWITEM:
			{
				DRAWITEMSTRUCT* lpdis = (DRAWITEMSTRUCT*) lParam;
				if (lpdis == NULL || lpdis->CtlType != ODT_MENU)
					return S_OK; // it's not a menu
				HICON hIcon = ((CKuMenuSet::CMenuItem *) lpdis->itemData)->GetIcon();
				if (hIcon == NULL)
					return S_OK;
				// lpdis->rcItem.left will be 17 here in most cases.
				// However, the desktop view in Win7 will be 2 here.
				DrawIconEx(lpdis->hDC,
					lpdis->rcItem.left - (lpdis->rcItem.left >= 16 ? 16 : 1),
					lpdis->rcItem.top + (lpdis->rcItem.bottom - lpdis->rcItem.top - 16) / 2,
					hIcon, 16, 16,
					0, NULL, DI_NORMAL);
				*pResult = TRUE;
			}
			break;
	}

	return S_OK;
}
