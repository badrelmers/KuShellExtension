#include "stdafx.h"
#include "StringConv.h"

LPCWSTR StringUTF8ToUTF16(LPCSTR sUTF8, CStringW &sUTF16, int iChars/* = -1*/)
{
	if (!sUTF8)
		return NULL;

	sUTF16.Empty();
	int iLen = MultiByteToWideChar(CP_UTF8, 0, sUTF8, iChars, NULL, 0);
	if (iLen > 0) {
		iLen = MultiByteToWideChar(CP_UTF8, 0, sUTF8, iChars, sUTF16.GetBufferSetLength(iLen), iLen + 1);
		sUTF16.ReleaseBuffer();
		return (iLen > 0) ? sUTF16 : NULL;
	}

	return *sUTF8 != 0 ? sUTF16 : NULL;
}

LPCWSTR StringANSIToUTF16(LPCSTR sANSI, CStringW &sUTF16, int iChars/* = -1*/, UINT codepage/* = CP_ACP*/)
{
	if (!sANSI)
		return NULL;

	sUTF16.Empty();
	int iLen = MultiByteToWideChar(codepage, 0, sANSI, iChars, NULL, 0);
	if (iLen > 0) {
		iLen = MultiByteToWideChar(codepage, 0, sANSI, iChars, sUTF16.GetBufferSetLength(iLen), iLen + 1);
		sUTF16.ReleaseBuffer();
		return (iLen > 0) ? sUTF16 : NULL;
	}

	return (*sANSI != 0) ? sUTF16 : NULL;
}

LPCSTR StringUTF16ToUTF8(LPCWSTR sUTF16, CStringA &sUTF8, int iChars/* = -1*/)
{
	if (!sUTF16)
		return NULL;

	sUTF8.Empty();
	int iLen = WideCharToMultiByte(CP_UTF8, 0, sUTF16, iChars, NULL, 0, NULL, NULL);
	if (iLen > 0) {
		iLen = WideCharToMultiByte(CP_UTF8, 0, sUTF16, iChars, sUTF8.GetBufferSetLength(iLen), iLen + 1, NULL, NULL);
		sUTF8.ReleaseBuffer();
		return (iLen > 0) ? sUTF8 : NULL;
	}

	return (*sUTF16 != 0) ? sUTF8 : NULL;
}

LPCSTR StringANSIToUTF8(LPCSTR sANSI, CStringA &sUTF8, int iChars/* = -1*/, UINT codepage/* = CP_ACP*/)
{
	return StringUTF16ToUTF8(CStringUTF16FromANSI(sANSI, iChars, codepage), sUTF8);
}

LPCSTR StringUTF16ToANSI(LPCWSTR sUTF16, CStringA &sANSI, int iChars/* = -1*/, char chDef/* = '?'*/, UINT codepage/* = CP_ACP*/)
{
	if (!sUTF16)
		return NULL;

	sANSI.Empty();
	int iLen = WideCharToMultiByte(codepage, WC_NO_BEST_FIT_CHARS, sUTF16, iChars, NULL, 0, &chDef, NULL);
	if (iLen > 0) {
		iLen = WideCharToMultiByte(codepage, WC_NO_BEST_FIT_CHARS, sUTF16, iChars, sANSI.GetBufferSetLength(iLen), iLen + 1, &chDef, NULL);
		sANSI.ReleaseBuffer();
		return (iLen > 0) ? sANSI : NULL;
	}

	return (*sUTF16 != 0) ? sANSI : NULL;
}

LPCSTR StringUTF8ToANSI(LPCSTR sUTF8, CStringA &sANSI, int iChars/* = -1*/, char chDef/* = '?'*/, UINT codepage/* = CP_ACP*/)
{
	return StringUTF16ToANSI(CStringUTF16FromUTF8(sUTF8, iChars), sANSI, iChars, chDef, codepage);
}

template <class SRC_T, class DEST_T>
SRC_T _StringDummyConv(SRC_T sSrc, DEST_T sDest, int iChars = -1)
{
	if (!sSrc)
		return NULL;
	if (iChars >= 0)
		sDest.SetString(sSrc, iChars);
	else
		sDest = sSrc;
	return sDest;
}

LPCWSTR _StringDummyConvW(LPCWSTR sSrc, CStringW &sDest, int iChars/* = -1*/)
{
	return _StringDummyConv(sSrc, sDest, iChars);
}

LPCSTR _StringDummyConvA(LPCSTR sSrc, CStringA &sDest, int iChars/* = -1*/)
{
	return _StringDummyConv(sSrc, sDest, iChars);
}
