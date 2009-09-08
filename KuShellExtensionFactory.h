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

class CKuShellExtensionFactory : public IClassFactory
{
	IMPLEMENT_INTERFACE
public:
	CKuShellExtensionFactory() {}
	virtual ~CKuShellExtensionFactory() {}

	// IClassFactory
	STDMETHOD(CreateInstance)( 
		/* [unique][in] */ IUnknown *pUnkOuter,
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ void **ppvObject);

	STDMETHOD(LockServer)( 
		/* [in] */ BOOL fLock);
};

