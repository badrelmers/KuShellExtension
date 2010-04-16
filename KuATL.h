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

#include <map>
#include <vector>

template<class KT, class VT>
class CKuMap : public std::map<KT, VT>
{
public:
	typedef std::map<KT, VT> baseT;
	typedef typename baseT::const_iterator const_iterator;
	typedef typename baseT::iterator iterator;
	typedef typename baseT::value_type value_type;

	CKuMap() {}
	virtual ~CKuMap() {}

	void RemoveAll()
	{
		baseT::clear();
	}

	bool Lookup(const KT &key, VT &value) const
	{
		const const_iterator &iter = baseT::find(key);
		if (iter == baseT::end())
			return false;
		value = iter->second;
		return true;
	}

	iterator SetAt(const KT &key, const VT &value)
	{
		baseT::erase(key);
		return baseT::insert(value_type(key, value)).first;
	}
};

template <class T>
class CKuArray : public std::vector<T>
{
public:
	typedef std::vector<T> baseT;

	CKuArray() {}
	virtual ~CKuArray() {}

	void RemoveAll()
	{
		baseT::clear();
	}

	bool SetCount(size_t nNewSize, int nGrowBy = - 1)
	{
		baseT::resize(nNewSize);
		return true;
	}

	size_t GetCount() const
	{
		return baseT::size();
	}

	size_t Add(const T &element)
	{
		baseT::push_back(element);
		return GetCount() - 1;
	}
};

#ifndef __ATLCOLL_H__
#define CAtlMap CKuMap
#define CAtlArray CKuArray
#endif
