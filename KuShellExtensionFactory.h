#pragma once

#include "CUnknown.h"

class CKuShellExtensionFactory : public IClassFactory
{
	IMPLEMENT_INTERFACE
public:
	CKuShellExtensionFactory();
	virtual ~CKuShellExtensionFactory();

	// IClassFactory
	virtual /* [local] */ HRESULT STDMETHODCALLTYPE CreateInstance( 
		/* [unique][in] */ IUnknown *pUnkOuter,
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ void **ppvObject);

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE LockServer( 
		/* [in] */ BOOL fLock);
};

