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
#include "KuShellExtInit.h"
#include "KuContextMenu.h"

//////////////////////////////////////////////////////////////////////////////////////
// CKuShellExtInit
//////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CKuShellExtInit::QueryInterface( 
	/* [in] */ REFIID riid,
	/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if (riid == IID_IShellExtInit || riid == IID_IUnknown)
		*ppvObject = this;
	else if (riid == IID_IContextMenu3 || riid == IID_IContextMenu2 || riid == IID_IContextMenu) {
		CKuContextMenu *p = new CKuContextMenu;
		p->AddRef();
		p->m_pData = m_pData;
		if (m_pData->m_aFiles.GetCount() > 0)
			m_pData = new CKuShellExtInitData; // The data has been initialized and transfered, prepare new one for subsequent call.
		*ppvObject = p;
		return S_OK;
	}
	else {
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}
	AddRef();
	return S_OK;
}

STDMETHODIMP CKuShellExtInit::Initialize( 
    /* [unique][in] */ 
    PCIDLIST_ABSOLUTE pidlFolder,
    /* [unique][in] */ 
    IDataObject *pdtobj,
    /* [unique][in] */ 
    HKEY hkeyProgID)
{
	m_pData->m_aFiles.RemoveAll();
	m_pData->m_iType = CKuShellExtInitData::TypeFile;

    // If a data object pointer was passed in, save it and
    // extract the file name. 
    if (pdtobj) { 
        STGMEDIUM   medium;
        FORMATETC   fe = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        UINT        uCount;

        if (SUCCEEDED(pdtobj->GetData(&fe, &medium)))
        {
            // Get the count of files dropped.
            uCount = DragQueryFile((HDROP)medium.hGlobal, (UINT)-1, NULL, 0);

            // Get the first file name from the CF_HDROP.
            UINT i;
			m_pData->m_aFiles.SetCount(uCount);
			for (i = 0;i < uCount;i++) {
				DragQueryFile((HDROP) medium.hGlobal, i, m_pData->m_aFiles[i].GetBufferSetLength(KU_MAX_PATH), KU_MAX_PATH);
				m_pData->m_aFiles[i].ReleaseBuffer();
			}

            ReleaseStgMedium(&medium);
        }
		else
			return E_UNEXPECTED;

		if (m_pData->m_aFiles.IsEmpty() || m_pData->m_aFiles[0].IsEmpty())
			return E_UNEXPECTED;

		m_pData->m_bFromFolderBk = false;
		if (PathIsDirectory(m_pData->m_aFiles[0]))
			m_pData->m_iType = CKuShellExtInitData::TypeDirectory;
    }
	else if (pidlFolder) {
		m_pData->m_aFiles.SetCount(1);
		// in windows 7 Explorer crashes when i right-click on Libraries or any item there-under, Doucment, Music, Pictures, Video, Explorer crashed with: "Microsoft Visual C++ Runtime Library... as described here too https://shellfix.nirsoft.net/fix_shell_problem.html?id=789&dll=KuShellExtension64.dll
		// The Libraries folder in Windows 7 is a virtual namespace folder (not a real filesystem folder on disk). It uses a special PIDL/GUID instead of a normal path, and many older shell extensions (including KuShellExtension) crash when they try to query its properties, expand variables like %u, %z, %~dpn1, %*, etc., or just classify the item during context-menu building.
		// the problem occurs when i right-click the empty space (background) inside the virtual Libraries folder.
		// Here is exactly what is happening: When you right-click a directory background, Windows passes a pidlFolder (a pointer to an item identifier list) to the extension's Initialize function. The code calls SHGetPathFromIDList to convert that PIDL into a string path.
		// Because Libraries is a virtual folder, SHGetPathFromIDList fails. However, the original author did not check the return value of that function. When it fails, the string buffer contains random, uninitialized memory garbage. Because it contains garbage, the subsequent m_pData->m_aFiles[0].IsEmpty() check returns FALSE (it thinks it has a valid path). The extension then tries to process that garbage memory as a real file path, which causes the C++ Runtime crash.
		// By checking bValidPath, if Windows passes the virtual Libraries folder PIDL to the extension, the code instantly recognizes that it's not a real file system path and returns E_UNEXPECTED. This tells Windows Explorer to silently skip loading the context menu for this specific click, preventing the crash entirely while keeping your context menu perfectly functional for real directory backgrounds.

		// FIX: We MUST capture the return value. SHGetPathFromIDList returns FALSE for virtual folders.
		BOOL bValidPath = SHGetPathFromIDList(pidlFolder, m_pData->m_aFiles[0].GetBufferSetLength(KU_MAX_PATH));
		m_pData->m_aFiles[0].ReleaseBuffer();

		// If the API failed to resolve a physical path (like in Windows 7 Libraries) 
		// or the path is empty, abort initialization gracefully to prevent memory crashes.
		if (!bValidPath || m_pData->m_aFiles[0].IsEmpty()) {
			m_pData->m_aFiles.RemoveAll();
			return E_UNEXPECTED;
		}

		m_pData->m_bFromFolderBk = true;
		m_pData->m_iType = CKuShellExtInitData::TypeDirectory;
	}
	else
		return E_UNEXPECTED;

	if (PathIsRoot(m_pData->m_aFiles[0])) {
		CString sPath = m_pData->m_aFiles[0];
		if (sPath[sPath.GetLength() - 1] != _T('\\'))
			sPath += _T('\\');
		switch (GetDriveType(sPath)) {
			case DRIVE_REMOVABLE:
				m_pData->m_iType = CKuShellExtInitData::TypeDriveRemovable;
				break;
			case DRIVE_FIXED:
				m_pData->m_iType = CKuShellExtInitData::TypeDriveFixed;
				break;
			case DRIVE_REMOTE:
				m_pData->m_iType = CKuShellExtInitData::TypeDriveNetwork;
				break;
			case DRIVE_CDROM:
				m_pData->m_iType = CKuShellExtInitData::TypeDriveOptical;
				break;
			case DRIVE_RAMDISK:
				m_pData->m_iType = CKuShellExtInitData::TypeDriveRamdisk;
				break;
			case DRIVE_UNKNOWN:
				m_pData->m_iType = CKuShellExtInitData::TypeDrive;
				break;
		}
	}

	if (m_pData->GetRefCount() > 1)
		m_pData = new CKuShellExtInitData; // The data has been initialized and transfered, prepare new one for subsequent call.
	return S_OK;
}
