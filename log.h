#pragma once

extern bool g_bLogEnabled;

void OutputDebugStringFormat(LPCTSTR fmt, ...);
#define LOG_ALWAYS(fmt, ...) OutputDebugStringFormat(_T("KuShellExtension: ") fmt, __VA_ARGS__)

#if !defined(NDEBUG) || defined(DEBUG_LOGGING)
#define LOGD(...) do { if (g_bLogEnabled) LOG_ALWAYS(__VA_ARGS__); } while (0)
#else
#define LOGD(...) ((void) 0)
#endif

#define LOGD_RETURN(rv) do { LOGD(_T("%S return %S"), __FUNCTION__, #rv); return (rv); } while (0)

#define CONST_STR(s) ((s) ? (s) : _T(""))
