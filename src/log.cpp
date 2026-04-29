#include "stdafx.h"
#include "log.h"

bool g_bLogEnabled = false;

void OutputDebugStringFormat(LPCTSTR fmt, ...)
{
	va_list ap;
	CString s;

	va_start(ap, fmt);
	s.FormatV(fmt, ap);
	OutputDebugString(s);
}

