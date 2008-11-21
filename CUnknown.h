#pragma once

class CUnknown
{
public:
	CUnknown()
		: m_uRefCount(1)
	{
		InterlockedIncrement((LONG *) &g_uRefCount);
	}
	virtual ~CUnknown()
	{
		InterlockedDecrement((LONG *) &g_uRefCount);
	}

	ULONG AddRef()
	{ return InterlockedIncrement((LONG *) &m_uRefCount); }

	ULONG Release()
	{ return InterlockedDecrement((LONG *) &m_uRefCount); }

	static ULONG g_uRefCount;
private:
	ULONG m_uRefCount;
};

#define IMPLEMENT_INTERFACE \
public: \
	virtual HRESULT STDMETHODCALLTYPE QueryInterface( \
		/* [in] */ REFIID riid, \
		/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject); \
	virtual ULONG STDMETHODCALLTYPE AddRef() { return m_Unknown.AddRef(); } \
	virtual ULONG STDMETHODCALLTYPE Release() { ULONG ret = m_Unknown.Release(); if (ret == 0) delete this; return ret; } \
private: \
	CUnknown m_Unknown;
