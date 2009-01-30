#include "stdafx.h"
#include "KuShellExtInit.h"
#include "KuContextMenu.h"

//////////////////////////////////////////////////////////////////////////////////////
// CKuShellExtInit
//////////////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE CKuShellExtInit::QueryInterface( 
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

HRESULT STDMETHODCALLTYPE CKuShellExtInit::Initialize( 
    /* [unique][in] */ 
    PCIDLIST_ABSOLUTE pidlFolder,
    /* [unique][in] */ 
    IDataObject *pdtobj,
    /* [unique][in] */ 
    HKEY hkeyProgID)
{
	m_pData->m_aFiles.RemoveAll();

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
    }
	else if (pidlFolder) {
		m_pData->m_aFiles.SetCount(1);
		SHGetPathFromIDList(pidlFolder, m_pData->m_aFiles[0].GetBufferSetLength(KU_MAX_PATH));
		m_pData->m_aFiles[0].ReleaseBuffer();

		m_pData->m_bFromFolderBk = true;
	}

	if (m_pData->GetRefCount() > 1)
		m_pData = new CKuShellExtInitData; // The data has been initialized and transfered, prepare new one for subsequent call.
	return S_OK;
}
