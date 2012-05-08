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

#pragma once

#include "KuShellExtInit.h"

namespace pug {
	class xml_node;
}

class CSetCurrentDirectory
{
public:
	CSetCurrentDirectory(LPCTSTR sDir)
	{
		GetCurrentDirectory(KU_MAX_PATH, m_sOldDir.GetBufferSetLength(KU_MAX_PATH));
		SetCurrentDirectory(sDir);
	}
	~CSetCurrentDirectory() { SetCurrentDirectory(m_sOldDir); }
private:
	CString m_sOldDir;
};

class CKuMenuSet
{
	friend class CMenuItem;

public:
	enum ACTION {
		  ACT_EXECUTE
		, ACT_BULITIN
	};
	enum CMD_ID {
		  CMD_ID_NULL
		, CMD_ID_DROP_SYMLINKS
		, CMD_ID_DROP_JUNCTIONS
		, CMD_ID_DROP_HARDLINKS
		, CMD_ID_RELOAD
		, CMD_ID_RENAME
	};
	enum {
		MAX_MENU_ITEM = 4096
	};

	class CMenuItemData
	{
		IMPLEMENT_REFCOUNT()
	public:
		CMenuItemData() : m_eAction(ACT_EXECUTE), m_bConsole(true), m_iWindowSize(SW_SHOWNORMAL) {}
		ACTION m_eAction;
		CString m_sAction;
		CString m_sWorkingDir;
		bool m_bConsole;
		int m_iWindowSize;
	};

	class CMenuItem
	{
	public:
		HICON GetIcon() const { return m_hIcon; }

		static LPCTSTR ExpandFileName(LPCTSTR sName, LPCTSTR sFlags, CString &sDest);
		static HBITMAP Create32BitBitmap(HDC hdc, int cx, int cy, VOID **ppvBits = NULL);
		static HBITMAP IconToBitmap(HICON hIcon, int cx, int cy);
		static HBITMAP IconToBitmap(HICON hIcon);
	private:
		friend class CKuMenuSet;

		static CMD_ID GetCmdId(LPCTSTR sCmd);

		explicit CMenuItem(CKuMenuSet *pKuMenuSet)
			: m_pParent(NULL), m_pPrevSibling(NULL), m_pNextSibling(NULL), m_pFirstChild(NULL)
			, m_hIcon(NULL), m_dwMultiItems(1), m_pKuMenuSet(pKuMenuSet), m_hBitmap(NULL), m_d(new CMenuItemData)
			, m_iIconIndex(0), m_eExistence(PATH_UNKNOWN)
		{ ASSERT(m_pKuMenuSet); }
		virtual ~CMenuItem()
		{
			if (m_pNextSibling)
				delete m_pNextSibling;
			if (m_pFirstChild)
				delete m_pFirstChild;
			if (m_hIcon)
				DestroyIcon(m_hIcon);
			if (m_hBitmap)
				DeleteObject(m_hBitmap);
		}

		bool IsSubMenu() { return m_pFirstChild != NULL; }
		bool IsSeparator() { return !m_sName.Compare(_T("----")); }
		bool IsOurPath(LPCTSTR sPath, int iType);
		static bool IsOurPath(LPCTSTR sPath, int iType, LPWSTR *pClasses, int iClassesCount);
		bool ShouldShown();

		void LoadIcon(LPCTSTR sFile, int iIndex);

		void QueryContextMenu(HMENU hMenu, UINT &indexMenu, UINT &idCmdFirst, UINT idCmdLast, UINT uFlags);
		bool InvokeCommand();
		CMenuItem *GetLastChild() const
		{ return m_pFirstChild ? (m_pFirstChild->m_pPrevSibling ? m_pFirstChild->m_pPrevSibling : m_pFirstChild) : NULL; }
		CMenuItem *GetFirstSibling() const
		{ return m_pParent ? m_pParent->m_pFirstChild : m_pKuMenuSet->m_pFirstItem; }
		CMenuItem *GetLastSibling() const
		{ return m_pParent ? m_pParent->GetLastChild() : m_pKuMenuSet->GetLastItem(); }
		bool IsFirst() const
		{ return GetFirstSibling() == this; }
		bool IsLast() const
		{ return GetLastSibling() == this; }

		enum {
			DROP_SYMBOLIC = 0x00000000,
			DROP_JUNCTION = 0x00000001,
			DROP_HARDLINK = 0x00000002,
			DROP_ABSOLUTE = 0x00000010,
		};
		static bool DropLinks(LPCTSTR sDir, LPCTSTR sPath, DWORD uFlags);

		CKuMenuSet *m_pKuMenuSet;
		CMenuItem *m_pParent;
		CMenuItem *m_pPrevSibling;
		CMenuItem *m_pNextSibling;
		CMenuItem *m_pFirstChild;
		CString m_sName;
		CString m_sClasses;
		HICON m_hIcon;
		HBITMAP m_hBitmap;
		DWORD m_dwMultiItems;
		CAtlMap<CString, CString> m_vars;
		CRefCountPtr<CMenuItemData> m_d;
		CString m_sIconFile;
		int m_iIconIndex;
		CString m_sPathToTest;
		enum {
			PATH_DOES_NOT_EXISTS,
			PATH_EXISTS,
			PATH_UNKNOWN
		} m_eExistence;

		class CInvokeCommandThread
		{
		public:
			CInvokeCommandThread() : m_pArgv(NULL) {}
			virtual ~CInvokeCommandThread()
			{
				if (!m_sTempFile.IsEmpty())
					DeleteFile(m_sTempFile);
				if (m_pArgv)
					LocalFree(m_pArgv);
			}

			static DWORD WINAPI _ThreadProc(LPVOID lpParameter)
			{
				DWORD r = ((CInvokeCommandThread *) lpParameter)->ThreadProc();
				delete ((CInvokeCommandThread *) lpParameter);
				return r;
			}
			DWORD ThreadProc();

			int m_iArgc;
			LPWSTR *m_pArgv;
			CMD_ID m_cmdId;
			CString m_sTempFile;
			CRefCountPtr<CMenuItemData> m_d;
			CRefCountPtr<CKuShellExtInitData> m_pData;
		};
	};

	CKuMenuSet();
	virtual ~CKuMenuSet();

	bool FromFile(LPCTSTR sPath);
	bool FromFile(HANDLE hFile);
	bool FromRaw(const BYTE *pXML, DWORD uLen);
	bool FromString(LPCTSTR sXML);

	bool IsEmpty() const
	{ return m_pFirstItem == NULL; }

	LPCTSTR Substitute(LPCTSTR src, CString &dest, CMenuItem *pItem = NULL);
	bool GetVariable(LPCTSTR key, CString &value, CMenuItem *pItem = NULL);
	bool GetOurVariable(LPCTSTR key, CString &value, CMenuItem *pItem = NULL);
	bool GetBoolean(LPCTSTR key, bool bDefault = false, CMenuItem *pItem = NULL);
	bool ToBoolean(LPCTSTR str)
	{ return !_tcscmp(str, _T("1")) || !_tcsicmp(str, _T("true")); }

	HRESULT QueryContextMenu(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
	bool InvokeCommand(UINT id)
	{ CMenuItem *pItem; if (m_cmd.Lookup(id, pItem)) return pItem->InvokeCommand(); return false; }

	static UINT DetectCodePage(const BYTE *pBuffer, DWORD uLen);
	static bool IsNumber(LPCTSTR str);
	static void InitBuiltinVars();

	CRefCountPtr<CKuShellExtInitData> m_pData;
private:
	CMenuItem *GetLastItem()
	{ return m_pFirstItem ? (m_pFirstItem->m_pPrevSibling ? m_pFirstItem->m_pPrevSibling : m_pFirstItem) : NULL; }

	CMenuItem *m_pFirstItem;

	CAtlMap<CString, CString> m_vars;
	static CAtlMap<CString, CString> m_BuiltinVars;
	CAtlMap<UINT, CMenuItem *> m_cmd;

	bool m_bVistaStyle;
	bool m_bHideMissing;
	bool m_bDeferredIO;
	UINT m_idCmdFirst;
	HANDLE m_hDeferredIOThread;
	LONG m_iJobs;
	bool m_bNoMoreJobs;
	CMenuItem *m_aJobs[MAX_MENU_ITEM];

	void PraseMenuItems(pug::xml_node &node, CMenuItem *pItem);
	static DWORD WINAPI DeferredIOThread(LPVOID pParam)
	{
		((CKuMenuSet *) pParam)->DeferredIOThread();
		return 0;
	}
	void DeferredIOThread();
	void StopDeferredIOThread()
	{
		if (m_hDeferredIOThread) {
			InterlockedExchange(&m_iJobs, -1);
			WaitForSingleObject(m_hDeferredIOThread, INFINITE);
			CloseHandle(m_hDeferredIOThread);
			m_hDeferredIOThread = NULL;
		}
	}
};

extern CKuMenuSet g_menu;
