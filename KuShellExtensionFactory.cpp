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
#include "KuShellExtensionFactory.h"
#include "KuContextMenu.h"

//////////////////////////////////////////////////////////////////////////////////////
// CKuShellExtensionFactory
//////////////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE CKuShellExtensionFactory::QueryInterface( 
	/* [in] */ REFIID riid,
	/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if (riid == IID_IClassFactory || riid == IID_IUnknown)
		*ppvObject = this;
	else
		return E_NOINTERFACE;
	AddRef();
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CKuShellExtensionFactory::CreateInstance( 
	/* [unique][in] */ IUnknown *pUnkOuter,
	/* [in] */ REFIID riid,
	/* [iid_is][out] */ void **ppvObject)
{
    if (pUnkOuter)
        return CLASS_E_NOAGGREGATION;

	IUnknown *pObj = NULL;

	if (riid == IID_IShellExtInit) {
		try {
			pObj = reinterpret_cast<IUnknown *>(new CKuShellExtInit);
			pObj->AddRef();
		} catch (...) { return E_OUTOFMEMORY; }
	}
	else if (riid == IID_IContextMenu3 || riid == IID_IContextMenu2 || riid == IID_IContextMenu) {
		try {
			pObj = reinterpret_cast<IUnknown *>(new CKuContextMenu);
			pObj->AddRef();
		} catch (...) { return E_OUTOFMEMORY; }
	}
    else
        return CLASS_E_CLASSNOTAVAILABLE;

	if (pObj) {
		if (SUCCEEDED(pObj->QueryInterface(riid, ppvObject))) {
			// Release extra refcount from QueryInterface
			pObj->Release();
		}
		else {
			delete pObj;
			return E_UNEXPECTED;
		}
	}
	else
		return E_OUTOFMEMORY;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CKuShellExtensionFactory::LockServer( 
	/* [in] */ BOOL fLock)
{
	if (fLock)
        InterlockedIncrement((LONG *) &CRefCount::m_uInstances);
	else {
		ULONG uInstances = 0;
		InterlockedExchange((LONG *) &uInstances, CRefCount::m_uInstances);
		if (uInstances > 0)
			InterlockedDecrement((LONG *) &CRefCount::m_uInstances);
		else
			return E_FAIL;
	}
	return S_OK;
}
