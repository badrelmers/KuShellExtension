#pragma once

#include "CUnknown.h"
#include "KuMenuSet.h"

#ifdef _WIN64
// {72F6A03F-7B17-4e65-AE37-666FC9024FA2}
static const CLSID CLSID_KU_CONTEXT_MENU = 
{ 0x72f6a03f, 0x7b17, 0x4e65, { 0xae, 0x37, 0x66, 0x6f, 0xc9, 0x2, 0x4f, 0xa2 } };
#define CLSID_KU_CONTEXT_MENU_STR _T("{72F6A03F-7B17-4e65-AE37-666FC9024FA2}")
#else
// {ACF4C166-8665-462d-B701-D4978E0009A1}
static const CLSID CLSID_KU_CONTEXT_MENU = 
{ 0xacf4c166, 0x8665, 0x462d, { 0xb7, 0x1, 0xd4, 0x97, 0x8e, 0x0, 0x9, 0xa1 } };
#define CLSID_KU_CONTEXT_MENU_STR _T("{ACF4C166-8665-462d-B701-D4978E0009A1}")
#endif

class CKuContextMenu : public IShellExtInit, public IContextMenu3
{
	IMPLEMENT_INTERFACE
public:
	CKuContextMenu();
	virtual ~CKuContextMenu();

	// IShellExtInit
    virtual HRESULT STDMETHODCALLTYPE Initialize( 
        /* [unique][in] */ 
        __in_opt  PCIDLIST_ABSOLUTE pidlFolder,
        /* [unique][in] */ 
        __in_opt  IDataObject *pdtobj,
        /* [unique][in] */ 
        __in_opt  HKEY hkeyProgID);

	// IContextMenu
	virtual HRESULT STDMETHODCALLTYPE QueryContextMenu( 
		/* [in] */ 
		__in  HMENU hMenu,
		/* [in] */ 
		__in  UINT indexMenu,
		/* [in] */ 
		__in  UINT idCmdFirst,
		/* [in] */ 
		__in  UINT idCmdLast,
		/* [in] */ 
		__in  UINT uFlags);

	virtual HRESULT STDMETHODCALLTYPE InvokeCommand( 
		/* [in] */ 
		__in  CMINVOKECOMMANDINFO *pici);

	virtual HRESULT STDMETHODCALLTYPE GetCommandString( 
		/* [in] */ 
		__in  UINT_PTR idCmd,
		/* [in] */ 
		__in  UINT uType,
		/* [in] */ 
		__reserved  UINT *pReserved,
		/* [out] */ 
		__out_awcount(!(uType & GCS_UNICODE), cchMax)  LPSTR pszName,
		/* [in] */ 
		__in  UINT cchMax);

	// IContextMenu2
	virtual HRESULT STDMETHODCALLTYPE HandleMenuMsg( 
		/* [in] */ 
		__in  UINT uMsg,
		/* [in] */ 
		__in  WPARAM wParam,
		/* [in] */ 
		__in  LPARAM lParam);

	// IContextMenu3
	virtual HRESULT STDMETHODCALLTYPE HandleMenuMsg2( 
		/* [in] */ 
		__in  UINT uMsg,
		/* [in] */ 
		__in  WPARAM wParam,
		/* [in] */ 
		__in  LPARAM lParam,
		/* [out] */ 
		__out_opt  LRESULT *pResult);

	static CString m_sConfigFile;
	static HBITMAP Create32BitBitmap(HDC hdc, int cx, int cy, VOID **ppvBits = NULL);
	static HBITMAP IconToBitmap(HICON hIcon, int cx, int cy);
	static HBITMAP IconToBitmap(HICON hIcon);
private:
	static HRESULT LoadConfig();
	static CKuMenuSet m_menu;
	static BY_HANDLE_FILE_INFORMATION m_cfgFileInfo;
	static UINT m_idCmdFirst;
	static ULONG m_uInstances;
#ifdef GDIPVER
	static Gdiplus::GdiplusStartupInput m_gdiplusStartupInput;
	static ULONG_PTR m_gdiplusToken;
#endif
};

class CKuContextMenuFactory : public IClassFactory
{
	IMPLEMENT_INTERFACE
public:
	CKuContextMenuFactory();
	virtual ~CKuContextMenuFactory();

	// IClassFactory
	virtual /* [local] */ HRESULT STDMETHODCALLTYPE CreateInstance( 
		/* [unique][in] */ IUnknown *pUnkOuter,
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ void **ppvObject);

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE LockServer( 
		/* [in] */ BOOL fLock);
};
