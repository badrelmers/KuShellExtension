#include "stdafx.h"
#include "KuShellExtensionFactory.h"
#include "KuContextMenu.h"

//////////////////////////////////////////////////////////////////////////////////////
// CKuShellExtensionFactory
//////////////////////////////////////////////////////////////////////////////////////
CKuShellExtensionFactory::CKuShellExtensionFactory()
{
}

CKuShellExtensionFactory::~CKuShellExtensionFactory()
{
}

HRESULT STDMETHODCALLTYPE CKuShellExtensionFactory::QueryInterface( 
	/* [in] */ REFIID riid,
	/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if (riid == IID_IClassFactory)
		*ppvObject = (void *)(IClassFactory *) this;
	else if (riid == IID_IUnknown)
		*ppvObject = (void *)(IUnknown *) this;
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

	CKuContextMenu *pKCM;

	try {
		pKCM = new CKuContextMenu;
	} catch (...) {
		return E_OUTOFMEMORY;
	}

	if (pKCM) {
		if (SUCCEEDED(pKCM->QueryInterface(riid, ppvObject))) {
			// Release extra refcount from QueryInterface
			pKCM->Release();
		}
		else {
			delete pKCM;
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
        InterlockedIncrement((LONG *) &CUnknown::g_uRefCount);
	else {
		if (CUnknown::g_uRefCount > 0)
			InterlockedDecrement((LONG *) &CUnknown::g_uRefCount);
		else
			return E_FAIL;
	}
	return S_OK;
}

