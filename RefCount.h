#pragma once

class CRefCount
{
public:
	CRefCount() : m_uRefCount(0)
	{ InterlockedIncrement((LONG *) &m_uInstances); }
	virtual ~CRefCount()
	{ InterlockedDecrement((LONG *) &m_uInstances); }

	ULONG AddRef()
	{ return InterlockedIncrement((LONG *) &m_uRefCount); }
	ULONG Release()
	{ ASSERT(m_uRefCount > 0); return InterlockedDecrement((LONG *) &m_uRefCount); }
	ULONG GetRefCount() const
	{ ULONG uRefCount = 0; InterlockedExchange((LONG *) &uRefCount, m_uRefCount); return uRefCount; }
	static ULONG m_uInstances;private:
	ULONG m_uRefCount;
};

#define IMPLEMENT_REFCOUNT(...) \
public: \
	virtual ULONG __VA_ARGS__ AddRef() { return m_RefCount.AddRef(); } \
	virtual ULONG __VA_ARGS__ Release() { ULONG ret = m_RefCount.Release(); if (ret == 0) delete this; return ret; } \
	virtual ULONG __VA_ARGS__ GetRefCount() const { return m_RefCount.GetRefCount(); } \
private: \
	CRefCount m_RefCount;

#define IMPLEMENT_INTERFACE \
public: \
	virtual HRESULT STDMETHODCALLTYPE QueryInterface( \
		/* [in] */ REFIID riid, \
		/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject); \
	IMPLEMENT_REFCOUNT(STDMETHODCALLTYPE)

template<typename T>
class CRefCountPtr
{
public:
	CRefCountPtr() : m_pObject(NULL) {}
	CRefCountPtr(const CRefCountPtr &ptr) : m_pObject(ptr.m_pObject) { m_pObject->AddRef(); }
	CRefCountPtr(T *pObject) : m_pObject(pObject) { m_pObject->AddRef(); }
	CRefCountPtr(const T &Object) : m_pObject(&Object) { m_pObject->AddRef(); }
	~CRefCountPtr() { if (m_pObject) m_pObject->Release(); }

	void Attach(T *pObject) { if (m_pObject) m_pObject->Release(); m_pObject = pObject; }
	T *Detach() { T *pObject = m_pObject; m_pObject = NULL; return pObject; }

	CRefCountPtr& operator = (T *pObject) { if (m_pObject) m_pObject->Release(); if (m_pObject = pObject) m_pObject->AddRef(); return *this; }
	CRefCountPtr& operator = (const T &Object) { return *this = &Object; }
	CRefCountPtr& operator = (const CRefCountPtr &ptr) { return *this = ptr.m_pObject; }

	T * Get() const { return m_pObject; }
	T * operator -> () const { return m_pObject; }
	operator T * () const { return m_pObject; }
private:
	T *m_pObject;
};
