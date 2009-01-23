#pragma once

/*
	CKuStringT: A copy on write CString-like string class
*/

#ifndef _WIN32
#define _vsnwprintf vswprintf
#else
#define strcasecmp stricmp
#define wcscasecmp wcsicmp
#endif

#ifndef ASSERT
#define ASSERT(expr)
#endif
#ifndef VERIFY
#define VERIFY(expr) expr
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)
#endif

class CKuStringUtilA
{
public:
	inline static int FormatV(char *dest, size_t cch, const char *fmt, va_list ap)
	{ return vsnprintf(dest, cch, fmt, ap); }
	inline static int Compare(const char *a, const char *b)
	{ return strcmp(a, b); }
	inline static int CompareNoCase(const char *a, const char *b)
	{ return strcasecmp(a, b); }
	inline static size_t SpanIncluding(const char *str, const char *strCharSet)
	{ return strspn(str, strCharSet); }
	inline static size_t SpanExcluding(const char *str, const char *strCharSet)
	{ return strcspn(str, strCharSet); }
	inline static int GetEnvironmentVariable(const char *lpName, char *lpBuffer, unsigned long nSize)
#ifdef _WIN32
	{ return ::GetEnvironmentVariableA(lpName, lpBuffer, nSize); }
#else
	{
		if (lpBuffer) {
			int ret = 0;
			const char *env = getenv(lpName);
			while (ret < nSize && (*lpBuffer++ = *env++))
				ret++;
			return ret;
		}
		return strlen(getenv(lpName));
	}
#endif
	inline static bool IsSpace(char c)
	{ return !!isspace((unsigned char) c); }
};

class CKuStringUtilW
{
public:
	inline static int FormatV(wchar_t *dest, size_t cch, const wchar_t *fmt, va_list ap)
	{ return _vsnwprintf(dest, cch, fmt, ap); }
	inline static int Compare(const wchar_t *a, const wchar_t *b)
	{ return wcscmp(a, b); }
	inline static int CompareNoCase(const wchar_t *a, const wchar_t *b)
	{ return wcscasecmp(a, b); }
	inline static size_t SpanIncluding(const wchar_t *str, const wchar_t *strCharSet)
	{ return wcsspn(str, strCharSet); }
	inline static size_t SpanExcluding(const wchar_t *str, const wchar_t *strCharSet)
	{ return wcscspn(str, strCharSet); }
	inline static int GetEnvironmentVariable(const wchar_t *lpName, wchar_t *lpBuffer, unsigned long nSize)
#ifdef _WIN32
	{ return ::GetEnvironmentVariableW(lpName, lpBuffer, nSize); }
#else
	{
		int ret = wcstombs(NULL, lpName, 0);
		if (ret < 0)
			return ret;
		char *sName = (char *) malloc(ret + 1);
		wcstombs(sName, lpName, ret);
		ret = mbstowcs(lpBuffer, getenv(sName), nSize);
		free(sName);
		return ret;
	}
#endif
	inline static bool IsSpace(wchar_t c)
	{ return !!iswspace((wint_t) c); }
};

template<class T, class U>
class CKuStringT
{
	typedef T * STRT;
	typedef const T * CSTRT;
	typedef int SIZET;
public:
	inline static void CopyChars(STRT dst, CSTRT src, SIZET len = -1)
	{
		if (len < 0)
			while (*dst = *src++)
				dst++;
		else {
			if (len > 0)
				memcpy(dst, src, len * sizeof(T));
			dst[len] = 0;
		}
	}
	inline static SIZET StringLength(CSTRT str)
	{
		if (!str)
			return 0;
		SIZET len = 0;
		while (*str++)
			len++;
		return len;
	}
	inline static STRT Find(STRT str, T ch)
	{
		for (;*str;*str++)
			if (*str == ch)
				return str;
		return NULL;
	}
	inline static CSTRT Find(CSTRT str, T ch)
	{ return Find((STRT) str, ch); }
private:
	typedef bool (*IsFunc)(T ch, CSTRT str);

	inline static bool IsSpace(T ch, CSTRT sSet)
	{ return !!U::IsSpace(ch); }
	inline static bool IsInSet(T ch, CSTRT sSet)
	{ return !!Find(sSet, ch); }
	inline static bool IsTheChar(T ch, CSTRT sSet)
	{ return ch == *sSet; }

	class CKuStringDataT
	{
		friend class CKuStringT;

		CKuStringDataT()
		{
			Init();
		}
		CKuStringDataT(const CKuStringDataT &d)
		{
			Init();
			Copy(d);
		}
		CKuStringDataT(CSTRT src, SIZET len = -1)
		{
			Init();
			Copy(src, len);
		}
		CKuStringDataT(T ch)
		{
			Init();
			Alloc(1);
			m_sString[0] = ch;
		}
		~CKuStringDataT()
		{
			if (m_sData)
				free(m_sData);
		}

		unsigned int AddRef() { return ++m_iRefCount; }
		unsigned int Release()
		{
			unsigned int uRefCount = --m_iRefCount;
			if (uRefCount == 0)
				delete this;
			return uRefCount;
		}

		CKuStringDataT &Copy(CSTRT src, SIZET len = -1)
		{
			if (len == -1)
				len = StringLength(src);
			Alloc(len);
			CopyChars(m_sString, src, len);
			m_iLength = len;
			return *this;
		}
		CKuStringDataT &Copy(const CKuStringDataT &d)
		{
			return Copy(d.m_sString, d.m_iLength);
		}
		bool Alloc(SIZET iCount)
		{
			ASSERT(m_sString >= m_sData);
			if (m_sData == m_sString) {
				if (!m_sData)
					m_sData = m_sString = (STRT) malloc((iCount + 1) * sizeof(T));
				else if (iCount > m_iSize)
					m_sData = m_sString = (STRT) realloc(m_sData, (iCount + 1) * sizeof(T));
				if (!m_sData)
					return false;
				m_sData[iCount] = 0;
				m_iSize = iCount;
			}
			else if (((SIZET)(m_sString - m_sData)) + iCount > m_iSize) {
				STRT sNew = (STRT) malloc((iCount + 1) * sizeof(T));
				if (!sNew)
					return false;
				CopyChars(sNew, m_sString, m_iLength);
				free(m_sData);
				m_sData = m_sString = sNew;
				m_iSize = iCount;
			}
			return true;
		}
		void FreeExtra(bool bForced = false)
		{
			if (!m_sData)
				return;
			if (m_iLength == 0) {
				free(m_sData);
				m_sData = m_sString = NULL;
				m_iSize = 0;
				Alloc(0);
			}
			else if (m_iSize > (m_iLength << 1) || bForced && m_iSize > m_iLength) {
				STRT sNew = (STRT) malloc(m_iLength + 1);
				CopyChars(sNew, m_sString, m_iLength);
				free(m_sData);
				m_iSize = m_iLength;
				m_sData = m_sString = sNew;
			}
		}
		void Offset(SIZET iOffset = 1)
		{
			m_sString += iOffset;
			m_iLength -= iOffset;
		}
		void Count()
		{
			m_sData[m_iSize] = 0;
			m_iLength = StringLength(m_sString);
		}
		void Init()
		{
			m_sData = m_sString = NULL;
			m_iSize = m_iLength = 0;
			m_iRefCount = 1;
		}

		STRT m_sString;
		STRT m_sData;
		SIZET m_iLength;
		SIZET m_iSize;
		unsigned int volatile m_iRefCount;
	};

	void Init()
	{
#ifndef _MSC_VER
		m_NULL = 0;
#endif
		m_pData = NULL;
		m_bDirty = false;
	}
	// Make sure data is ready for write. i.e. refcount = 1
	void New(bool bCopy = true)
	{
		if (!m_pData)
			m_pData = new CKuStringDataT;
		else if (m_pData->m_iRefCount > 1) {
			CKuStringDataT *pData = m_pData;
			if (bCopy)
				m_pData = new CKuStringDataT(*pData);
			else
				m_pData = new CKuStringDataT;
			pData->Release();
		}
		else if (!bCopy)
			m_pData->m_iLength = 0;
		m_bDirty = false;
	}

	CKuStringT &TrimLeft(IsFunc isfunc, CSTRT sSet)
	{
		if (!IsEmpty()) {
			New();
			while (isfunc(m_pData->m_sString[0], sSet))
				m_pData->Offset();
		}
		return *this;
	}
	CKuStringT &TrimRight(IsFunc isfunc, CSTRT sSet)
	{
		if (!IsEmpty()) {
			New();
			STRT str = m_pData->m_sString;
			CSTRT ptr = str + m_pData->m_iLength - 1;
			for (;ptr >= str && isfunc(*ptr, sSet);ptr--)
				m_pData->m_iLength--;
			str[m_pData->m_iLength] = 0;
		}
		return *this;
	}
	CKuStringT &Trim(IsFunc isfunc, CSTRT sSet)
	{
		TrimRight(isfunc, sSet);
		return TrimLeft(isfunc, sSet);
	}
public:
	CKuStringT()
	{ Init(); }
	virtual ~CKuStringT()
	{
		if (m_pData)
			m_pData->Release();
	}

	SIZET GetLength() const
	{ return m_pData ? m_pData->m_iLength : 0; }

	void Empty()
	{
		if (m_pData) {
			m_pData->Release();
			Init();
		}
	}
	bool IsEmpty() const
	{ return GetLength() == 0; }

	CKuStringT(const CKuStringT &str)
	{
		Init();
		SetString(str);
	}
	CKuStringT &SetString(const CKuStringT &str)
	{
		if (m_pData != str.m_pData) {
			if (str.m_pData) {
				if (m_pData)
					m_pData->Release();
				if (str.m_bDirty)
					m_pData = new CKuStringDataT(*str.m_pData);
				else {
					m_pData = str.m_pData;
					m_pData->AddRef();
				}
			}
			else
				Empty();
		}
		return *this;
	}
	CKuStringT & operator = (const CKuStringT &str)
	{ return SetString(str); }

	CKuStringT(CSTRT str, SIZET len = -1)
	{
		Init();
		SetString(str, len);
	}
	CKuStringT &SetString(CSTRT str, SIZET len = -1)
	{
		if (m_pData)
			m_pData->Release();
		m_pData = new CKuStringDataT(str, len);
		m_bDirty = false;
		return *this;
	}
	CKuStringT & operator = (CSTRT str)
	{ return SetString(str); }

	explicit CKuStringT(T ch)
	{
		Init();
		SetString(ch);
	}
	CKuStringT &SetString(T ch)
	{
		if (m_pData)
			m_pData->Release();
		m_pData = new CKuStringDataT(ch);
		m_bDirty = false;
		return *this;
	}
	CKuStringT & operator = (T ch)
	{ return SetString(ch); }


	CSTRT GetString() const
	{ return m_pData ? m_pData->m_sString : &m_NULL; }
	operator CSTRT () const
	{ return GetString(); }

	void SetAt(SIZET i, T ch)
	{ ASSERT(i < GetLength()); m_pData->m_sString[i] = ch; }
	T GetAt(SIZET i) const
	{ ASSERT(i < GetLength()); return m_pData->m_sString[i]; }
	T &GetAt(SIZET i)
	{ ASSERT(i < GetLength()); return m_pData->m_sString[i]; }
	T operator [] (SIZET i) const
	{ return GetAt(i); }
	T &operator [] (SIZET i)
	{ return GetAt(i); }

	int GetAllocLength() const
	{ return m_pData ? m_pData->m_iSize : 0; }
	void Preallocate(int nLength)
	{
		if (!m_pData)
			m_pData = new CKuStringDataT;
		m_pData->Alloc(nLength);
	}
	void FreeExtra(bool bForced = false)
	{ ASSERT(!m_bDirty); if (m_pData) m_pData->FreeExtra(bForced); }

	STRT GetBufferSetLength(SIZET len)
	{
		GetBuffer();
		m_pData->Alloc(len);
		return m_pData->m_sString;
	}
	STRT GetBuffer(SIZET len)
	{ return GetBufferSetLength(len); }
	STRT GetBuffer()
	{
		New();
		m_bDirty = true;
		return m_pData->m_sString;
	}
	void ReleaseBufferSetLength(SIZET len)
	{
		if (m_bDirty) {
			m_pData->m_iLength = len;
			m_pData->m_sString[m_pData->m_iLength] = 0;
			m_bDirty = false;
			FreeExtra();
		}
	}
	void ReleaseBuffer(SIZET len = -1)
	{
		if (len != -1)
			ReleaseBufferSetLength(len);
		else if (m_bDirty) {
			m_pData->Count();
			m_bDirty = false;
			FreeExtra();
		}
	}

	CKuStringT& Append(CSTRT str, SIZET len = -1)
	{
		ASSERT(!m_bDirty);
		ASSERT(str);
		if (*str) {
			New();
			if (len == -1)
				len = StringLength(str);
			m_pData->Alloc(m_pData->m_iLength + len);
			CopyChars(m_pData->m_sString + m_pData->m_iLength, str, len);
			m_pData->m_iLength += len;
		}
		return *this;
	}
	CKuStringT& operator += (CSTRT str)
	{ return Append(str); }

	CKuStringT& Append(T ch)
	{
		ASSERT(!m_bDirty);
		New();
		m_pData->Alloc(m_pData->m_iLength + 1);
		m_pData->m_sString[m_pData->m_iLength++] = ch;
		m_pData->m_sString[m_pData->m_iLength] = 0;
		return *this;
	}
	CKuStringT& operator += (T ch)
	{ return Append(ch); }

	CKuStringT operator + (CSTRT str) const
	{
		CKuStringT ret(*this);
		return ret.Append(str);
	}
	friend CKuStringT operator + (CSTRT a, const CKuStringT &b)
	{
		CKuStringT ret(a);
		return ret.Append(b);
	}

	CKuStringT operator + (T ch) const
	{
		CKuStringT ret(*this);
		return ret.Append(ch);
	}
	friend CKuStringT operator + (T a, const CKuStringT &b)
	{
		CKuStringT ret(a);
		return ret.Append(b);
	}

	int FormatV(CSTRT fmt, va_list ap)
	{
		ASSERT(fmt);
		New(false);
		int len = U::FormatV(NULL, 0, fmt, ap);
		m_pData->Alloc(len + 1);
		// NOTE: The length argument of format function has deferent implementions.
		// Use safe one, though it may waste 1 character space.
		m_pData->m_iLength = U::FormatV(m_pData->m_sString, len + 1, fmt, ap);
		return m_pData->m_iLength;
	}
	int Format(CSTRT fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		return FormatV(fmt, ap);
	}

	int AppendFormatV(CSTRT fmt, va_list ap)
	{
		ASSERT(fmt);
		New();
		int len = U::FormatV(NULL, 0, fmt, ap);
		// NOTE: The length argument of format function has deferent implementions.
		// Use safe one, though it may waste 1 character space.
		m_pData->Alloc(m_pData->m_iLength + len + 1);
		m_pData->m_iLength += U::FormatV(m_pData->m_sString + m_pData->m_iLength, len + 1, fmt, ap);
		return m_pData->m_iLength;
	}
	int AppendFormat(CSTRT fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		return AppendFormatV(fmt, ap);
	}

	CSTRT GetEnvironmentVariable(CSTRT sName)
	{
		SIZET len = U::GetEnvironmentVariable(sName, NULL, 0);
		if (len == 0)
			Empty();
		else {
			New(false);
			m_pData->Alloc(len);
			U::GetEnvironmentVariable(sName, m_pData->m_sString, len);
		}
		return m_pData->m_sString;
	}

	CKuStringT Mid(SIZET iFirst, SIZET iLen) const
	{ ASSERT(iFirst + iLen <= GetLength()); return CKuStringT(GetString() + iFirst, iLen); }
	CKuStringT Mid(SIZET iFirst) const
	{ ASSERT(iFirst <= GetLength()); return CKuStringT(GetString() + iFirst); }
	CKuStringT Left(SIZET iLen) const
	{ ASSERT(iLen <= GetLength()); return CKuStringT(GetString(), iLen); }
	CKuStringT Right(SIZET iLen) const
	{ ASSERT(iLen <= GetLength()); return CKuStringT(GetString() + GetLength() - iLen); }

	void Truncate(SIZET nNewLength)
	{
		ASSERT(!m_bDirty);
		ASSERT(nNewLength <= GetLength());
		if (nNewLength < GetLength()) {
			New();
			m_pData->m_sString[nNewLength] = 0;
			m_pData->m_iLength = nNewLength;
		}
	}

	CKuStringT &CutMid(SIZET iFirst)
	{
		ASSERT(!m_bDirty);
		ASSERT(iFirst <= GetLength());
		ASSERT(iFirst >= 0);
		if (iFirst > 0) {
			New();
			m_pData->Offset(iFirst);
		}
		return *this;
	}
	CKuStringT &CutMid(SIZET iFirst, SIZET iLen)
	{
		if (iLen == 0)
			Empty();
		else {
			CutMid(iFirst);
			Truncate(iLen);
		}
		return *this;
	}
	CKuStringT &CutLeft(SIZET iLen)
	{ Truncate(iLen); return *this; }
	CKuStringT &CutRight(SIZET iLen)
	{ return CutMid(GetLength() - iLen); }

	CKuStringT& TrimLeft()
	{ return TrimLeft(IsSpace, NULL); }
	CKuStringT& TrimLeft(T ch)
	{ return TrimLeft(IsTheChar, &ch); }
	CKuStringT& TrimLeft(CSTRT sSet)
	{ return TrimLeft(IsInSet, sSet); }
	CKuStringT& TrimRight()
	{ return TrimRight(IsSpace, NULL); }
	CKuStringT& TrimRight(T ch)
	{ return TrimRight(IsTheChar, &ch); }
	CKuStringT& TrimRight(CSTRT sSet)
	{ return TrimRight(IsInSet, sSet); }
	CKuStringT& Trim()
	{ return Trim(IsSpace, NULL); }
	CKuStringT& Trim(T ch)
	{ return Trim(IsTheChar, &ch); }
	CKuStringT& Trim(CSTRT sSet)
	{ return Trim(IsInSet, sSet); }

	CKuStringT SpanIncluding(CSTRT pszCharSet) const
	{
		ASSERT(pszCharSet);
		CKuStringT sRet;
		if (!IsEmpty()) {
			size_t len = U::SpanIncluding(m_pData->m_sString, pszCharSet);
			if (len > 0) {
				STRT ptr = sRet.GetBufferSetLength(len);
				CopyChars(ptr, m_pData->m_sString, len);
				sRet.ReleaseBuffer();
			}
		}
		return sRet;
	}

	CKuStringT SpanExcluding(CSTRT pszCharSet) const
	{
		ASSERT(pszCharSet);
		CKuStringT sRet;
		if (!IsEmpty()) {
			size_t len = U::SpanExcluding(m_pData->m_sString, pszCharSet);
			if (len > 0) {
				STRT ptr = sRet.GetBufferSetLength(len);
				CopyChars(ptr, m_pData->m_sString, len);
				sRet.ReleaseBuffer();
			}
		}
		return sRet;
	}

	CKuStringT Tokenize(CSTRT pszTokens, SIZET& iStart) const
	{
		ASSERT(!m_bDirty);
		ASSERT(pszTokens);
		ASSERT(iStart >= 0);
		CKuStringT sRet;
		if (iStart < GetLength()) {
			iStart += U::SpanIncluding(m_pData->m_sString + iStart, pszTokens);
			if (iStart < GetLength()) {
				size_t len = U::SpanExcluding(m_pData->m_sString + iStart, pszTokens);
				if (len > 0) {
					STRT ptr = sRet.GetBufferSetLength(len);
					CopyChars(ptr, m_pData->m_sString + iStart, len);
					sRet.ReleaseBuffer();
				}
				iStart += len;
			}
		}
		return sRet;
	}

	int Compare(CSTRT str) const
	{ return U::Compare(m_pData->m_sString, str); }
	int CompareNoCase(CSTRT str) const
	{ return U::CompareNoCase(m_pData->m_sString, str); }

	bool operator == (CSTRT str) const
	{ return Compare(str) == 0; }
	bool operator > (CSTRT str) const
	{ return Compare(str) > 0; }
	bool operator < (CSTRT str) const
	{ return Compare(str) < 0; }
	bool operator >= (CSTRT str) const
	{ return Compare(str) >= 0; }
	bool operator <= (CSTRT str) const
	{ return Compare(str) <= 0; }

	friend bool operator == (CSTRT a, const CKuStringT &b)
	{ return b.Compare(a) == 0; }
	friend bool operator > (CSTRT a, const CKuStringT &b)
	{ return b.Compare(a) < 0; }
	friend bool operator < (CSTRT a, const CKuStringT &b)
	{ return b.Compare(a) > 0; }
	friend bool operator >= (CSTRT a, const CKuStringT &b)
	{ return b.Compare(a) <= 0; }
	friend bool operator <= (CSTRT a, const CKuStringT &b)
	{ return b.Compare(a) >= 0; }

	bool operator == (const CKuStringT &str) const
	{ return Compare(str) == 0; }
	bool operator > (const CKuStringT &str) const
	{ return Compare(str) > 0; }
	bool operator < (const CKuStringT &str) const
	{ return Compare(str) < 0; }
	bool operator >= (const CKuStringT &str) const
	{ return Compare(str) >= 0; }
	bool operator <= (const CKuStringT &str) const
	{ return Compare(str) <= 0; }

	int Compare(T ch) const
	{ T str[2] = {ch, 0}; return Compare(str); }
	int CompareNoCase(T ch) const
	{ T str[2] = {ch, 0}; return CompareNoCase(str); }

	bool operator == (T ch) const
	{ return Compare(ch) == 0; }
	bool operator > (T ch) const
	{ return Compare(ch) > 0; }
	bool operator < (T ch) const
	{ return Compare(ch) < 0; }
	bool operator >= (T ch) const
	{ return Compare(ch) >= 0; }
	bool operator <= (T ch) const
	{ return Compare(ch) <= 0; }

	friend bool operator == (T a, const CKuStringT &b)
	{ return b.Compare(a) == 0; }
	friend bool operator > (T a, const CKuStringT &b)
	{ return b.Compare(a) < 0; }
	friend bool operator < (T a, const CKuStringT &b)
	{ return b.Compare(a) > 0; }
	friend bool operator >= (T a, const CKuStringT &b)
	{ return b.Compare(a) <= 0; }
	friend bool operator <= (T a, const CKuStringT &b)
	{ return b.Compare(a) >= 0; }

// simple hash function for CMap
#ifdef _MSC_VER
	operator ULONG_PTR() const
	{
		ULONG_PTR ret = 0;
		STRT ptr = (STRT) &ret;
		int i;
		for (i = 0;i < sizeof(ULONG_PTR) / sizeof(T) && i < GetLength();i++)
			ptr[i] = m_pData->m_sString[i];
		return ret;
	}
#endif
private:
#ifdef _MSC_VER
	const static T m_NULL = 0;
#else
	T m_NULL;
#endif
	CKuStringDataT *m_pData;
	bool m_bDirty;
};

typedef CKuStringT<char, CKuStringUtilA> CKuStringA;
typedef CKuStringT<wchar_t, CKuStringUtilW> CKuStringW;

#if defined(_WIN32) && defined(_UNICODE)
typedef CKuStringW CKuString;
typedef CKuStringUtilW CKuStringUtil;
#else
typedef CKuStringA CKuString;
typedef CKuStringUtilA CKuStringUtil;
#endif

#ifndef __CSTRINGT_H__
typedef CKuStringA CStringA;
typedef CKuStringW CStringW;
typedef CKuString CString;
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif
