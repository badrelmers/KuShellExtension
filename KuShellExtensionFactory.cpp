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
