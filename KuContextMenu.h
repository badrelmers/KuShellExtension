#pragma once

#include "CUnknown.h"
#include "KuMenuSet.h"

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
	static HRESULT LoadConfig();
	static CKuMenuSet m_menu;
private:
	static BY_HANDLE_FILE_INFORMATION m_cfgFileInfo;
	static UINT m_idCmdFirst;
	static ULONG m_uInstances;
#ifdef GDIPVER
	static Gdiplus::GdiplusStartupInput m_gdiplusStartupInput;
	static ULONG_PTR m_gdiplusToken;
#endif
};
