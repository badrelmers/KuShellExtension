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
