#if defined(DLL_GETPROCADDR) && !defined(DLL_H_GETPROCADDR) || defined(DLL_DEFINE) && !defined(DLL_H_DEFINE) || \
	!defined(DLL_GETPROCADDR) && !defined(DLL_DEFINE) && !defined(DLL_H_DECLARE)

#if defined(DLL_GETPROCADDR)
	#define DLL_H_GETPROCADDR
	#define DECLARE_DLL_PROC(name, dll, ret, call, args) { \
		HMODULE hModule = GetModuleHandle(_T(dll)); \
		if (!hModule) hModule = LoadLibrary(_T(dll) _T(".dll")); \
		if (hModule) name = (name##Proc) GetProcAddress(hModule , #name); }
#elif defined(DLL_DEFINE)
	#define DLL_H_DEFINE
	#define DECLARE_DLL_PROC(name, dll, ret, call, args) \
		name##Proc name = NULL;
#else
	#define DLL_H_DECLARE
	#define DECLARE_DLL_PROC(name, dll, ret, call, args) \
		typedef ret (call *name##Proc) args; \
		extern name##Proc name;
#endif

#ifdef DLL_GETPROCADDR
void Init() {
#else
namespace dll {
#endif

#ifndef SYMBOLIC_LINK_FLAG_DIRECTORY
#define SYMBOLIC_LINK_FLAG_DIRECTORY (0x1)
#endif
DECLARE_DLL_PROC(CreateSymbolicLinkW, "kernel32", BOOL, APIENTRY, (LPCWSTR lpSymlinkFileName, LPCWSTR lpTargetFileName, DWORD dwFlags))

#ifndef _WIN64
DECLARE_DLL_PROC(IsWow64Process, "kernel32", BOOL, WINAPI, (HANDLE hProcess, PBOOL Wow64Process))
DECLARE_DLL_PROC(Wow64DisableWow64FsRedirection, "kernel32", BOOL, WINAPI, (PVOID* OldValue))
DECLARE_DLL_PROC(Wow64RevertWow64FsRedirection, "kernel32", BOOL, WINAPI, (PVOID OldValue))
#endif

DECLARE_DLL_PROC(GdiplusStartup, "gdiplus", BOOL, WINAPI, (ULONG_PTR *token, const Gdiplus::GdiplusStartupInput *input, Gdiplus::GdiplusStartupOutput *output))
DECLARE_DLL_PROC(GdiplusShutdown, "gdiplus", VOID, WINAPI, (ULONG_PTR token))
DECLARE_DLL_PROC(GdipCreateBitmapFromHICON, "gdiplus", Gdiplus::GpStatus, WINGDIPAPI, (HICON hicon, Gdiplus::GpBitmap** bitmap))
DECLARE_DLL_PROC(GdipCreateHBITMAPFromBitmap, "gdiplus", Gdiplus::GpStatus, WINGDIPAPI, (Gdiplus::GpBitmap* bitmap, HBITMAP* hbmReturn, Gdiplus::ARGB background))
DECLARE_DLL_PROC(GdipDisposeImage, "gdiplus", Gdiplus::GpStatus, WINGDIPAPI, (Gdiplus::GpImage *image))

#undef DECLARE_DLL_PROC
#if defined(DLL_DEFINE)
	#define DLL_GETPROCADDR
	#include "dll.h"
	#undef DLL_GETPROCADDR
#elif !defined(DLL_GETPROCADDR)
	void Init();
#endif

}

#endif
