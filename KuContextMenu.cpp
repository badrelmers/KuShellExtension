/* This file is part of KuShellExtension
 * Copyright (C) 2008-2010 Kai-Chieh Ku (kjackie@gmail.com)
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
#include "KuContextMenu.h"
#include "dll.h"
#include "globals.h"

//////////////////////////////////////////////////////////////////////////////////////
// CKuContextMenu
//////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CKuContextMenu::QueryInterface( 
	/* [in] */ REFIID riid,
	/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if (riid == IID_IShellExtInit) {
		CKuShellExtInit *p = new CKuShellExtInit;
		p->AddRef();
		m_pData = p->m_pData;
		*ppvObject = p;
		return S_OK;
	}
	else if (riid == IID_IContextMenu3 || riid == IID_IContextMenu2 || riid == IID_IContextMenu || riid == IID_IUnknown)
		*ppvObject = this;
	else {
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}
	AddRef();
	return S_OK;
}

STDMETHODIMP CKuContextMenu::QueryContextMenu( 
	/* [in] */ 
	HMENU hMenu,
	/* [in] */ 
	UINT indexMenu,
	/* [in] */ 
	UINT idCmdFirst,
	/* [in] */ 
	UINT idCmdLast,
	/* [in] */ 
	UINT uFlags)
{
	if (!m_pData)
		return E_INVALIDARG;
	g_menu.m_pData = m_pData;
	m_pData = NULL;

	HRESULT hr = ku::LoadConfig();
	if (hr != S_OK)
		return hr;

	m_idCmdFirst = idCmdFirst;
	if(!(CMF_DEFAULTONLY & uFlags))
		return g_menu.QueryContextMenu(hMenu, indexMenu, idCmdFirst, idCmdLast, uFlags);

	return S_OK;
}

#ifndef CMIC_MASK_UNICODE
#include <pshpack8.h>
typedef struct _CMINVOKECOMMANDINFOEX
    {
    DWORD cbSize;
    DWORD fMask;
    HWND hwnd;
    LPCSTR lpVerb;
    LPCSTR lpParameters;
    LPCSTR lpDirectory;
    int nShow;
    DWORD dwHotKey;
    HANDLE hIcon;
    LPCSTR lpTitle;
    LPCWSTR lpVerbW;
    LPCWSTR lpParametersW;
    LPCWSTR lpDirectoryW;
    LPCWSTR lpTitleW;
    POINT ptInvoke;
    } 	CMINVOKECOMMANDINFOEX;
#include <poppack.h>
#define CMIC_MASK_UNICODE       SEE_MASK_UNICODE
#endif

STDMETHODIMP CKuContextMenu::InvokeCommand( 
	/* [in] */ 
	CMINVOKECOMMANDINFO *pici)
{
	CMINVOKECOMMANDINFOEX *piciex = (CMINVOKECOMMANDINFOEX *) pici;

	bool bUnicode = pici->cbSize == sizeof(CMINVOKECOMMANDINFOEX) && (pici->fMask & CMIC_MASK_UNICODE);

	// pici->lpVerb == 0 or pici->lpVerbW == 0 are invalid because we offer IDs from offset 1
	UINT id;
	if(pici->lpVerb && !HIWORD(pici->lpVerb)) // it seems lpVerb is more reliable than lpVerbW
		id = LOWORD(pici->lpVerb);
	else if (bUnicode && piciex->lpVerbW && !HIWORD(piciex->lpVerbW))
		id = LOWORD(piciex->lpVerbW);
	else
		id = *(bUnicode ? (WORD *) piciex->lpVerbW : (WORD *) pici->lpVerb);
	if (!g_menu.InvokeCommand(id + m_idCmdFirst))
		return E_FAIL;

	return S_OK;
}

STDMETHODIMP CKuContextMenu::GetCommandString( 
	/* [in] */ 
	UINT_PTR idCmd,
	/* [in] */ 
	UINT uType,
	/* [in] */ 
	UINT *pReserved,
	/* [out] */ 
	LPSTR pszName,
	/* [in] */ 
	UINT cchMax)
{
	// stores menu ID in the buffer for verb.
	switch (uType) {
		case GCS_VERBA:
			if ((cchMax * sizeof(char)) < sizeof(WORD))
				return E_OUTOFMEMORY;
			*((WORD *) pszName) = idCmd & 0xFFFF;
			break;
		case GCS_VERBW:
			if ((cchMax * sizeof(wchar_t)) < sizeof(WORD))
				return E_OUTOFMEMORY;
			*((WORD *) pszName) = idCmd & 0xFFFF;
			break;
	}
    return S_OK;
}

STDMETHODIMP CKuContextMenu::HandleMenuMsg( 
	/* [in] */ 
	UINT uMsg,
	/* [in] */ 
	WPARAM wParam,
	/* [in] */ 
	LPARAM lParam)
{
	return HandleMenuMsg2(uMsg, wParam, lParam, NULL);
}

/*
	codes from TortoiseSVN
*/
STDMETHODIMP CKuContextMenu::HandleMenuMsg2( 
	/* [in] */ 
	UINT uMsg,
	/* [in] */ 
	WPARAM wParam,
	/* [in] */ 
	LPARAM lParam,
	/* [out] */ 
	LRESULT *pResult)
{
	LRESULT res;
	if (pResult == NULL)
		pResult = &res;
	*pResult = FALSE;

	switch (uMsg)
	{
		case WM_MEASUREITEM:
			{
				MEASUREITEMSTRUCT* lpmis = (MEASUREITEMSTRUCT*) lParam;
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
				DRAWITEMSTRUCT* lpdis = (DRAWITEMSTRUCT*) lParam;
				if (lpdis == NULL || lpdis->CtlType != ODT_MENU)
					return S_OK; // it's not a menu
				HICON hIcon = ((CKuMenuSet::CMenuItem *) lpdis->itemData)->GetIcon();
				if (hIcon == NULL)
					return S_OK;
				// lpdis->rcItem.left will be 17 here in most cases.
				// However, the desktop view in Win7 will be 2 here.
				DrawIconEx(lpdis->hDC,
					lpdis->rcItem.left - (lpdis->rcItem.left >= 16 ? 16 : 1),
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
		// TODO: Find a better way to detect a icon is 32bpp or not.
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

	// this works in most cases, but generates ugly images for such icons contains alpha channel and depend on GDI+
	if (!bIs32Bpp && dll::GdipCreateBitmapFromHICON) {
		if (dll::GdiplusStartup && !ku::gdiplusToken)
			dll::GdiplusStartup(&ku::gdiplusToken, &ku::gdiplusStartupInput, NULL);
		if (ku::gdiplusToken) {
			DeleteObject(hBmp);
			Gdiplus::GpBitmap *pBitmap = NULL;
			if (dll::GdipCreateBitmapFromHICON(hIcon, &pBitmap) == Gdiplus::Ok) {
				dll::GdipCreateHBITMAPFromBitmap(pBitmap, &hBmp, Gdiplus::Color::Transparent);
				dll::GdipDisposeImage((Gdiplus::GpImage *) pBitmap);
			}
		}
	}
#endif
	return hBmp;
}

HBITMAP CKuContextMenu::IconToBitmap(HICON hIcon)
{
	return IconToBitmap(hIcon, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
}
