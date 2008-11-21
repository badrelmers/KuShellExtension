#include "StdAfx.h"
#include "KuContextMenu.h"
#include "globals.h"

//////////////////////////////////////////////////////////////////////////////////////
// CKuContextMenu
//////////////////////////////////////////////////////////////////////////////////////

CKuMenuSet CKuContextMenu::m_menu;
CString CKuContextMenu::m_sConfigFile;
BY_HANDLE_FILE_INFORMATION CKuContextMenu::m_cfgFileInfo = {0};
UINT CKuContextMenu::m_idCmdFirst = 0;
ULONG CKuContextMenu::m_uInstances = 0;
#ifdef GDIPVER
Gdiplus::GdiplusStartupInput CKuContextMenu::m_gdiplusStartupInput;
ULONG_PTR CKuContextMenu::m_gdiplusToken = 0;
#endif

CKuContextMenu::CKuContextMenu()
{
	ULONG uInstances = (ULONG) InterlockedIncrement((LONG *) &m_uInstances);
#ifdef GDIPVER
	if (uInstances == 1 && ku::SysVer.m_vMajor >= 6)
		Gdiplus::GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, NULL);
#endif
}

CKuContextMenu::~CKuContextMenu()
{
	ULONG uInstances = (ULONG) InterlockedDecrement((LONG *) &m_uInstances);
#ifdef GDIPVER
	if (uInstances == 0 && m_gdiplusToken) {
		Gdiplus::GdiplusShutdown(m_gdiplusToken);
		m_gdiplusToken = 0;
	}
#endif
}

HRESULT STDMETHODCALLTYPE CKuContextMenu::QueryInterface( 
	/* [in] */ REFIID riid,
	/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if (riid == IID_IShellExtInit)
		*ppvObject = (void *)(IShellExtInit *) this;
	else if (riid == IID_IContextMenu3)
		*ppvObject = (void *)(IContextMenu3 *) this;
	else if (riid == IID_IContextMenu2)
		*ppvObject = (void *)(IContextMenu2 *) this;
	else if (riid == IID_IContextMenu)
		*ppvObject = (void *)(IContextMenu *) this;
	else if (riid == IID_IUnknown)
		*ppvObject = (void *)(IUnknown *)(IContextMenu *) this;
	else
		return E_NOINTERFACE;
	AddRef();
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CKuContextMenu::Initialize( 
    /* [unique][in] */ 
    __in_opt  PCIDLIST_ABSOLUTE pidlFolder,
    /* [unique][in] */ 
    __in_opt  IDataObject *pdtobj,
    /* [unique][in] */ 
    __in_opt  HKEY hkeyProgID)
{
	m_menu.m_aFiles.RemoveAll();

    // If a data object pointer was passed in, save it and
    // extract the file name. 
    if (pdtobj) { 
        STGMEDIUM   medium;
        FORMATETC   fe = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        UINT        uCount;

        if(SUCCEEDED(pdtobj->GetData(&fe, &medium)))
        {
            // Get the count of files dropped.
            uCount = DragQueryFile((HDROP)medium.hGlobal, (UINT)-1, NULL, 0);

            // Get the first file name from the CF_HDROP.
            UINT i;
			m_menu.m_aFiles.SetCount(uCount);
			for (i = 0;i < uCount;i++) {
				DragQueryFile((HDROP)medium.hGlobal, i, m_menu.m_aFiles[i].GetBufferSetLength(KU_MAX_PATH), KU_MAX_PATH);
				m_menu.m_aFiles[i].ReleaseBuffer();
			}

            ReleaseStgMedium(&medium);
        }

		m_menu.m_bFromFolderBk = false;
    }
	else if (pidlFolder) {
		m_menu.m_aFiles.SetCount(1);
		SHGetPathFromIDList(pidlFolder, m_menu.m_aFiles[0].GetBufferSetLength(KU_MAX_PATH));
		m_menu.m_aFiles[0].ReleaseBuffer();

		m_menu.m_bFromFolderBk = true;
	}

	return S_OK;
}

HRESULT CKuContextMenu::LoadConfig()
{
	if (m_sConfigFile.IsEmpty()) {
		m_sConfigFile = ku::sModulePath;
		LPTSTR ptr = _tcsrchr(m_sConfigFile.GetBufferSetLength(KU_MAX_PATH), _T('\\'));
		if (ptr)
			_tcscpy(ptr + 1, _T("config.xml"));
		m_sConfigFile.ReleaseBuffer();
		CKuMenuSet::InitBuiltinVars();
	}

	HANDLE hFile;
	BY_HANDLE_FILE_INFORMATION cfgFileInfo = m_cfgFileInfo;
	if ((hFile = CreateFile(m_sConfigFile, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE && m_menu.IsEmpty())
		return E_HANDLE;
	if (!GetFileInformationByHandle(hFile, &m_cfgFileInfo) && m_menu.IsEmpty())
		return E_FAIL;
	CloseHandle(hFile);
	if (m_cfgFileInfo.nFileSizeLow != cfgFileInfo.nFileSizeLow || m_cfgFileInfo.nFileSizeHigh != cfgFileInfo.nFileSizeHigh ||
		m_cfgFileInfo.ftLastWriteTime.dwLowDateTime != cfgFileInfo.ftLastWriteTime.dwLowDateTime || 
		m_cfgFileInfo.ftLastWriteTime.dwHighDateTime != cfgFileInfo.ftLastWriteTime.dwHighDateTime)
		if (!m_menu.FromFile(m_sConfigFile))
			return E_FAIL;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CKuContextMenu::QueryContextMenu( 
	/* [in] */ 
	__in  HMENU hMenu,
	/* [in] */ 
	__in  UINT indexMenu,
	/* [in] */ 
	__in  UINT idCmdFirst,
	/* [in] */ 
	__in  UINT idCmdLast,
	/* [in] */ 
	__in  UINT uFlags)
{
	HRESULT hr = LoadConfig();
	if (hr != S_OK)
		return hr;

	m_idCmdFirst = idCmdFirst;
	if(!(CMF_DEFAULTONLY & uFlags))
		return m_menu.QueryContextMenu(hMenu, indexMenu, idCmdFirst, idCmdLast, uFlags);

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CKuContextMenu::InvokeCommand( 
	/* [in] */ 
	__in  CMINVOKECOMMANDINFO *pici)
{
	CMINVOKECOMMANDINFOEX *piciex = (CMINVOKECOMMANDINFOEX *) pici;

	UINT id;
	if (pici->cbSize == sizeof(CMINVOKECOMMANDINFOEX) && piciex->lpVerbW && !HIWORD(piciex->lpVerbW)) {
		id = LOWORD(piciex->lpVerbW);
	}
	else if(pici->lpVerb && !HIWORD(pici->lpVerb)) {
		id = LOWORD(pici->lpVerb);
	}
	else
		return E_FAIL;

	if (!m_menu.InvokeCommand(id + m_idCmdFirst))
		return E_FAIL;

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CKuContextMenu::GetCommandString( 
	/* [in] */ 
	__in  UINT_PTR idCmd,
	/* [in] */ 
	__in  UINT uType,
	/* [in] */ 
	__reserved  UINT *pReserved,
	/* [out] */ 
	__out_awcount(!(uType & GCS_UNICODE), cchMax)  LPSTR pszName,
	/* [in] */ 
	__in  UINT cchMax)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CKuContextMenu::HandleMenuMsg( 
	/* [in] */ 
	__in  UINT uMsg,
	/* [in] */ 
	__in  WPARAM wParam,
	/* [in] */ 
	__in  LPARAM lParam)
{
	return HandleMenuMsg2(uMsg, wParam, lParam, NULL);
}

/*
	codes from TortoiseSVN
*/
HRESULT STDMETHODCALLTYPE CKuContextMenu::HandleMenuMsg2( 
	/* [in] */ 
	__in  UINT uMsg,
	/* [in] */ 
	__in  WPARAM wParam,
	/* [in] */ 
	__in  LPARAM lParam,
	/* [out] */ 
	__out_opt  LRESULT *pResult)
{
	LRESULT res;
	if (pResult == NULL)
		pResult = &res;
	*pResult = FALSE;

	switch (uMsg)
	{
		case WM_MEASUREITEM:
			{
				MEASUREITEMSTRUCT* lpmis = (MEASUREITEMSTRUCT*)lParam;
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
				DRAWITEMSTRUCT* lpdis = (DRAWITEMSTRUCT*)lParam;
				if ((lpdis==NULL)||(lpdis->CtlType != ODT_MENU))
					return S_OK; //not for a menu
				HICON hIcon = m_menu.GetMenuIcon(lpdis->itemID);
				if (hIcon == NULL)
					return S_OK;
				DrawIconEx(lpdis->hDC,
					lpdis->rcItem.left - 16,
					lpdis->rcItem.top + (lpdis->rcItem.bottom - lpdis->rcItem.top - 16) / 2,
					hIcon, 16, 16,
					0, NULL, DI_NORMAL);
				*pResult = TRUE;
			}
			break;
	}

	return S_OK;
}

/*
	codes from eMule
*/
HBITMAP CKuContextMenu::Create32BitBitmap(HDC hdc, int cx, int cy, VOID **ppvBits/* = NULL */)
{
	HBITMAP hBmp = NULL;
	BITMAPINFO bmi = {0};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biWidth = cx;
	bmi.bmiHeader.biHeight = cy;
	bmi.bmiHeader.biBitCount = 32;

	HDC hdcUsed = hdc ? hdc : GetDC(NULL);
	if (hdcUsed)
	{
		hBmp = CreateDIBSection(hdcUsed, &bmi, DIB_RGB_COLORS, ppvBits, NULL, 0);
		if (hdc != hdcUsed)
			ReleaseDC(NULL, hdcUsed);
	}
	return hBmp;
}

HBITMAP CKuContextMenu::IconToBitmap(HICON hIcon, int cx, int cy)
{
	if (!hIcon)
		return NULL;

	bool bIs32Bpp = false;
	HBITMAP hBmp = NULL;
	HDC hdcDest;
	hdcDest = CreateCompatibleDC(GetDC(NULL));
	if (!hdcDest)
		return NULL;

#if 0
	ICONINFO icInfo = {0};
	GetIconInfo(hIcon, &icInfo);

	LPBITMAPINFO pBmpInfoColor, pBmpInfoMask;
	RGBQUAD *bits = (RGBQUAD *) malloc(sizeof(RGBQUAD) * cx * cy);
	DWORD *mask = (DWORD *) malloc((cx * cy) >> 2);

	pBmpInfoColor = (LPBITMAPINFO) malloc(sizeof(BITMAPINFO) + sizeof(RGBQUAD) * cx * cy);
	pBmpInfoMask = (LPBITMAPINFO) malloc(sizeof(BITMAPINFO) + sizeof(RGBQUAD) * cx * cy);
	memset(pBmpInfoColor, 0, sizeof(BITMAPINFO));
	memset(pBmpInfoMask, 0, sizeof(BITMAPINFO));
	pBmpInfoColor->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pBmpInfoMask->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

	VERIFY( GetDIBits(hdcDest, icInfo.hbmColor, 0, 0, 0, pBmpInfoColor, DIB_RGB_COLORS) );
	VERIFY( GetDIBits(hdcDest, icInfo.hbmColor, 0, cy, bits, pBmpInfoColor, DIB_RGB_COLORS) );
	VERIFY( GetDIBits(hdcDest, icInfo.hbmMask, 0, 0, 0, pBmpInfoMask, DIB_RGB_COLORS) );
	VERIFY( GetDIBits(hdcDest, icInfo.hbmMask, 0, cy, mask, pBmpInfoMask, DIB_RGB_COLORS) );
	for (int i = 0;i < cx * cy;i++) {
		if (bits[i].rgbReserved != 0) {
			bIs32Bpp = true;
			break;
		}
	}
	if (!bIs32Bpp) {
		for (int i = 0;i < cx * cy;i++) {
			TRACE(_T("%d %d\n"), i >> 4, i & 0xF);
			if (!( ( (HIWORD(mask[i >> 4]) ^ LOWORD(mask[i >> 4]) ) >> (i & 0xF) ) & 1 ))
				bits[i].rgbReserved = 0xFF;
		}
	}
	void *pDib = NULL;
	hBmp = Create32BitBitmap(hdcDest, cx, cy, &pDib);
	memcpy(pDib, bits, sizeof(RGBQUAD) * cx * cy);
	//SetDIBits(hdcDest, hBmp, 0, cy, pDib, pBmpInfoColor, DIB_RGB_COLORS);
	//hBmp = CreateBitmap(cx, cy, 1, 32, bits);
	//hBmp = CreateDIBSection(hdcDest, 
	free(pBmpInfoColor);
	free(pBmpInfoMask);
	free(bits);
	free(mask);
#else
	// this works better when the icon contains 32bpp images, otherwise, it doesn't work. :(
	hBmp = Create32BitBitmap(hdcDest, cx, cy);
	if (hBmp) {
		HBITMAP hBmpOld = (HBITMAP) SelectObject(hdcDest, hBmp);
		DrawIconEx(hdcDest, 0, 0, hIcon, cx, cy, 0, NULL, DI_NORMAL);
		SelectObject(hdcDest, hBmpOld);
		LPBITMAPINFO pBmpInfo;
		VERIFY( pBmpInfo = (LPBITMAPINFO) malloc(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * cx * cy) );
		memset(pBmpInfo, 0, sizeof(BITMAPINFO));
		pBmpInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		RGBQUAD *bits;
		VERIFY( bits = (RGBQUAD *) malloc(cx * cy * sizeof(RGBQUAD)) );
		if (GetDIBits(hdcDest, hBmp, 0, 0, 0, pBmpInfo, DIB_RGB_COLORS) &&
			GetDIBits(hdcDest, hBmp, 0, 16, bits, pBmpInfo, DIB_RGB_COLORS))
		{
			for (int i = 0;i < cx * cy;i++) {
				if (bits[i].rgbReserved != 0) {
					bIs32Bpp = true;
					break;
				}
			}
		}
		free(bits);
		free(pBmpInfo);
	}
	DeleteDC(hdcDest);

	// this works in most cases, but generates ugly images for icon contains alpha channel and depend on GDI+
	if (!bIs32Bpp) {
		DeleteObject(hBmp);
		Gdiplus::Bitmap icon(hIcon);
		icon.GetHBITMAP(Gdiplus::Color::Transparent, &hBmp);
	}
#endif
	return hBmp;
}

HBITMAP CKuContextMenu::IconToBitmap(HICON hIcon)
{
	return IconToBitmap(hIcon, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
}

//////////////////////////////////////////////////////////////////////////////////////
// CKuContextMenuFactory
//////////////////////////////////////////////////////////////////////////////////////
CKuContextMenuFactory::CKuContextMenuFactory()
{
}

CKuContextMenuFactory::~CKuContextMenuFactory()
{
}

HRESULT STDMETHODCALLTYPE CKuContextMenuFactory::QueryInterface( 
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

HRESULT STDMETHODCALLTYPE CKuContextMenuFactory::CreateInstance( 
	/* [unique][in] */ IUnknown *pUnkOuter,
	/* [in] */ REFIID riid,
	/* [iid_is][out] */ void **ppvObject)
{
    if (pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    CKuContextMenu *pKCM = new CKuContextMenu;

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

HRESULT STDMETHODCALLTYPE CKuContextMenuFactory::LockServer( 
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
