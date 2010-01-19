/* This file is part of KuShellExtension
 * Copyright (C) 2008-2009 Kai-Chieh Ku (kjackie@gmail.com)
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
	else
		return E_NOINTERFACE;
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

        if(SUCCEEDED(pdtobj->GetData(&fe, &medium)))
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

		m_pData->m_bFromFolderBk = false;
		if (PathIsDirectory(m_pData->m_aFiles[0]))
			m_pData->m_iType = CKuShellExtInitData::TypeDirectory;
    }
	else if (pidlFolder) {
		m_pData->m_aFiles.SetCount(1);
		SHGetPathFromIDList(pidlFolder, m_pData->m_aFiles[0].GetBufferSetLength(KU_MAX_PATH));
		m_pData->m_aFiles[0].ReleaseBuffer();

		m_pData->m_bFromFolderBk = true;
		m_pData->m_iType = CKuShellExtInitData::TypeDirectory;
	}
	
	if (PathIsRoot(m_pData->m_aFiles[0])) {
		CString sPath = m_pData->m_aFiles[0];
		if (sPath[sPath.GetLength() - 1] != _T('\\'))
			sPath += _T('\\');
		switch(GetDriveType(sPath)) {
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
