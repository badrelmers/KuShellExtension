#pragma once

#include <map>
#include <vector>

template<class KT, class VT>
class CKuMap : public std::map<KT, VT>
{
public:
	CKuMap() {}
	virtual ~CKuMap() {}

	void RemoveAll()
	{
		clear();
	}

	bool Lookup(const KT &key, VT &value) const
	{
		const_iterator &iter = find(key);
		if (iter == end())
			return false;
		value = iter->second;
		return true;
	}

	iterator SetAt(const KT &key, const VT &value)
	{
		erase(key);
		return insert(value_type(key, value)).first;
	}
};

template <class T>
class CKuArray : public std::vector<T>
{
public:
	CKuArray() {}
	virtual ~CKuArray() {}

	void RemoveAll()
	{
		clear();
	}

	bool SetCount(size_t nNewSize, int nGrowBy = - 1)
	{
		resize(nNewSize);
		return true;
	}

	size_t GetCount() const
	{
		return size();
	}

	size_t Add(const T &element)
	{
		push_back(element);
		return GetCount() - 1;
	}
};

#ifndef __ATLCOLL_H__
#define CAtlMap CKuMap
#define CAtlArray CKuArray
#endif
