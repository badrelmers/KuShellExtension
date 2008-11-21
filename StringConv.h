#pragma once

LPCWSTR StringUTF8ToUTF16(LPCSTR sUTF8, CStringW &sUTF16, int iChars = -1);
LPCWSTR StringANSIToUTF16(LPCSTR sANSI, CStringW &sUTF16, int iChars = -1, UINT codepage = CP_ACP);
LPCSTR StringUTF16ToUTF8(LPCWSTR sUTF16, CStringA &sUTF8, int iChars = -1);
LPCSTR StringANSIToUTF8(LPCSTR sANSI, CStringA &sUTF8, int iChars = -1, UINT codepage = CP_ACP);
LPCSTR StringUTF16ToANSI(LPCWSTR sUTF16, CStringA &sANSI, int iChars = -1, char chDef = '?', UINT codepage = CP_ACP);
LPCSTR StringUTF8ToANSI(LPCSTR sUTF8, CStringA &sANSI, int iChars = -1, char chDef = '?', UINT codepage = CP_ACP);

LPCWSTR _StringDummyConvW(LPCWSTR sSrc, CStringW &sDest, int iChars = -1);
LPCSTR _StringDummyConvA(LPCSTR sSrc, CStringA &sDest, int iChars = -1);

#ifdef _UNICODE
#define StringTCHARToUTF16	_StringDummyConvW
#define StringTCHARToUTF8	StringUTF16ToUTF8
#define StringTCHARToANSI	StringUTF16ToANSI
inline LPCWSTR StringUTF16ToTCHAR(LPCWSTR sUTF16, CStringW &sTCHAR, int iChars = -1, char chDef = '?')
{ return _StringDummyConvW(sUTF16, sTCHAR, iChars); }
inline LPCWSTR StringUTF8ToTCHAR(LPCSTR sUTF8, CStringW &sTCHAR, int iChars = -1, char chDef = '?')
{ return StringUTF8ToUTF16(sUTF8, sTCHAR, iChars); }
#define StringANSIToTCHAR	StringANSIToUTF16
#else
#define StringTCHARToUTF16	StringANSIToUTF16
#define StringTCHARToUTF8	StringANSIToUTF8
#define StringTCHARToANSI	_StringDummyConvA
#define StringUTF16ToTCHAR	StringUTF16ToANSI
#define StringUTF8ToTCHAR	StringUTF8ToANSI
#define StringANSIToTCHAR	_StringDummyConvA
#endif

class CStringUTF16FromUTF8 : public CStringW
{
public:
	CStringUTF16FromUTF8(LPCSTR sUTF8, int iChars = -1)
	{ StringUTF8ToUTF16(sUTF8, *this, iChars); }
};

class CStringUTF16FromANSI : public CStringW
{
public:
	CStringUTF16FromANSI(LPCSTR sANSI, int iChars = -1, UINT codepage = CP_ACP)
	{ StringANSIToUTF16(sANSI, *this, iChars, codepage); }
};

class CStringUTF8FromUTF16 : public CStringA
{
public:
	CStringUTF8FromUTF16(LPCWSTR sUTF16, int iChars = -1)
	{ StringUTF16ToUTF8(sUTF16, *this, iChars); }
};

class CStringUTF8FromANSI : public CStringA
{
public:
	CStringUTF8FromANSI(LPCSTR sANSI, int iChars = -1, UINT codepage = CP_ACP)
	{ StringANSIToUTF8(sANSI, *this, iChars, codepage); }
};

class CStringANSIFromUTF16 : public CStringA
{
public:
	CStringANSIFromUTF16(LPCWSTR sUTF16, int iChars = -1, char chDef = '?', UINT codepage = CP_ACP)
	{ StringUTF16ToANSI(sUTF16, *this, iChars, chDef, codepage); }
};

class CStringANSIFromUTF8 : public CStringA
{
public:
	CStringANSIFromUTF8(LPCSTR sUTF8, int iChars = -1, char chDef = '?', UINT codepage = CP_ACP)
	{ StringUTF8ToANSI(sUTF8, *this, iChars, chDef, codepage); }
};

#ifdef _UNICODE
#define CStringTCHARFromUTF16IfNeeded(s, ...) (s)
#define CStringUTF16FromTCHARIfNeeded(s, ...) (s)
#define CStringTCHARFromANSIIfNeeded(s, ...) CStringTCHARFromANSI((s), __VA_ARGS__)
#define CStringANSIFromTCHARIfNeeded(s, ...) CStringANSIFromTCHAR((s), __VA_ARGS__)

class CStringTCHARFromUTF16 : public CStringW
{
public:
	CStringTCHARFromUTF16(LPCWSTR sUTF16, int iChars = -1, char chDef = '?')
	{ _StringDummyConvW(sUTF16, *this, iChars); }
};
class CStringTCHARFromUTF8 : public CStringW
{
public:
	CStringTCHARFromUTF8(LPCSTR sUTF8, int iChars = -1, char chDef = '?')
	{ StringUTF8ToUTF16(sUTF8, *this, iChars); }
};
typedef	CStringUTF16FromANSI	CStringTCHARFromANSI;
typedef CStringW				CStringUTF16FromTCHAR;
typedef CStringUTF8FromUTF16	CStringUTF8FromTCHAR;
typedef CStringANSIFromUTF16	CStringANSIFromTCHAR;
#else
#define CStringTCHARFromUTF16IfNeeded(s, ...) CStringTCHARFromUTF16((s), __VA_ARGS__)
#define CStringUTF16FromTCHARIfNeeded(s, ...) CStringUTF16FromTCHAR((s), __VA_ARGS__)
#define CStringTCHARFromANSIIfNeeded(s, ...) (s)
#define CStringANSIFromTCHARIfNeeded(s, ...) (s)

typedef CStringANSIFromUTF16	CStringTCHARFromUTF16;
typedef CStringANSIFromUTF8		CStringTCHARFromUTF8;
typedef	CStringA				CStringTCHARFromANSI;
typedef CStringUTF16FromANSI	CStringUTF16FromTCHAR;
typedef CStringUTF8FromANSI		CStringUTF8FromTCHAR;
typedef CStringA				CStringANSIFromTCHAR;
#endif

typedef CStringTCHARFromUTF16	CStringFromUTF16;
typedef CStringTCHARFromUTF8	CStringFromUTF8;
typedef	CStringTCHARFromANSI	CStringFromANSI;

template <class CHAR_T>
void CharConvEndian(CHAR_T *pChar)
{
	_ASSERT(sizeof(CHAR_T) % 2 == 0);
	BYTE *pCh = (BYTE *) pChar, chTemp;
	int i, j;
	for (i = 0, j = sizeof(CHAR_T) - 1;i < (sizeof(CHAR_T) >> 1);i++, j--) {
		chTemp = pCh[i];
		pCh[i] = pCh[j];
		pCh[j] = chTemp;
	}
}

// uChars = 0 ==> treat 'sSrc' as NULL-terminal
template <class CHAR_T>
UINT StringConvEndian(CHAR_T *sSrc, UINT uChars = 0)
{
	_ASSERT(sSrc);
	UINT i;
	for (i = 0;sSrc[i] && (uChars == 0 || i < uChars);i++)
		CharConvEndian(sSrc + i);
	return i;
}
