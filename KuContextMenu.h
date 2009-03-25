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

#pragma once

#include "RefCount.h"
#include "KuMenuSet.h"

// NOTE: Inherit multiple COM interfaces cause problems in MinGW, it seems caused by __stdcall class methods.
class CKuContextMenu : public IContextMenu3
{
	IMPLEMENT_INTERFACE
public:
	CKuContextMenu() : m_idCmdFirst(0) {}
	virtual ~CKuContextMenu() {}

	// IContextMenu
	virtual HRESULT STDMETHODCALLTYPE QueryContextMenu( 
		/* [in] */ 
		HMENU hMenu,
		/* [in] */ 
		UINT indexMenu,
		/* [in] */ 
		UINT idCmdFirst,
		/* [in] */ 
		UINT idCmdLast,
		/* [in] */ 
		UINT uFlags);

	virtual HRESULT STDMETHODCALLTYPE InvokeCommand( 
		/* [in] */ 
		CMINVOKECOMMANDINFO *pici);

	virtual HRESULT STDMETHODCALLTYPE GetCommandString( 
		/* [in] */ 
		UINT_PTR idCmd,
		/* [in] */ 
		UINT uType,
		/* [in] */ 
		UINT *pReserved,
		/* [out] */ 
		LPSTR pszName,
		/* [in] */ 
		UINT cchMax);

	// IContextMenu2
	virtual HRESULT STDMETHODCALLTYPE HandleMenuMsg( 
		/* [in] */ 
		UINT uMsg,
		/* [in] */ 
		WPARAM wParam,
		/* [in] */ 
		LPARAM lParam);

	// IContextMenu3
	virtual HRESULT STDMETHODCALLTYPE HandleMenuMsg2( 
		/* [in] */ 
		UINT uMsg,
		/* [in] */ 
		WPARAM wParam,
		/* [in] */ 
		LPARAM lParam,
		/* [out] */ 
		LRESULT *pResult);

	CRefCountPtr<CKuShellExtInitData> m_pData;

	static HBITMAP Create32BitBitmap(HDC hdc, int cx, int cy, VOID **ppvBits = NULL);
	static HBITMAP IconToBitmap(HICON hIcon, int cx, int cy);
	static HBITMAP IconToBitmap(HICON hIcon);
private:
	UINT m_idCmdFirst;
};
