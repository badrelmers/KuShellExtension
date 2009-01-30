#pragma once

#include "RefCount.h"

#ifndef HAVE_ATL
#include "KuString.h"
#if defined(_MSC_VER) && defined(_DEBUG)
	#pragma push_macro("new")
	#undef new
#endif
#include "KuATL.h"
#if defined(_MSC_VER) && defined(_DEBUG)
	#pragma pop_macro("new")
#endif
#endif

#ifndef PCIDLIST_ABSOLUTE
#define PCIDLIST_ABSOLUTE LPCITEMIDLIST
#endif

class CKuShellExtInitData
{
	IMPLEMENT_REFCOUNT()
public:
	CKuShellExtInitData() : m_bFromFolderBk(false) {}

	bool m_bFromFolderBk;
	CAtlArray<CString> m_aFiles;
};

class CKuShellExtInit : public IShellExtInit
{
	IMPLEMENT_INTERFACE
public:
	CKuShellExtInit() { m_pData = new CKuShellExtInitData; }
	virtual ~CKuShellExtInit() {}

	// IShellExtInit
	virtual HRESULT STDMETHODCALLTYPE Initialize( 
		/* [unique][in] */ 
		PCIDLIST_ABSOLUTE pidlFolder,
		/* [unique][in] */ 
		IDataObject *pdtobj,
		/* [unique][in] */ 
		HKEY hkeyProgID);

	CRefCountPtr<CKuShellExtInitData> m_pData;
};
