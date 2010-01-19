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
	CKuShellExtInitData() : m_bFromFolderBk(false), m_iType(0) {}

	bool m_bFromFolderBk;
	enum {
		TypeFile,
		TypeDirectory = 1,
		TypeDrive = 2 | TypeDirectory,
		TypeDriveRemovable = (1 << 2) | TypeDrive,
		TypeDriveFixed     = (2 << 2) | TypeDrive,
		TypeDriveNetwork   = (3 << 2) | TypeDrive,
		TypeDriveOptical   = (4 << 2) | TypeDrive,
		TypeDriveRamdisk   = (5 << 2) | TypeDrive,
	};
	int m_iType;
	CAtlArray<CString> m_aFiles;
};

class CKuShellExtInit : public IShellExtInit
{
	IMPLEMENT_INTERFACE
public:
	CKuShellExtInit() { m_pData = new CKuShellExtInitData; }
	virtual ~CKuShellExtInit() {}

	// IShellExtInit
	STDMETHOD(Initialize)( 
		/* [unique][in] */ 
		PCIDLIST_ABSOLUTE pidlFolder,
		/* [unique][in] */ 
		IDataObject *pdtobj,
		/* [unique][in] */ 
		HKEY hkeyProgID);

	CRefCountPtr<CKuShellExtInitData> m_pData;
};
