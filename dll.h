#if defined(DLL_GETPROCADDR) && !defined(DLL_H_GETPROCADDR) || defined(DLL_DEFINE) && !defined(DLL_H_DEFINE) || \
	!defined(DLL_GETPROCADDR) && !defined(DLL_DEFINE) && !defined(DLL_H_DECLARE)

#if defined(DLL_GETPROCADDR)
	#define DLL_H_GETPROCADDR
	#define DECLARE_DLL_PROC(name, dll, ret, call, ...) \
		name = (name##Proc) GetProcAddress(GetModuleHandle(_T(dll)), #name);
#elif defined(DLL_DEFINE)
	#define DLL_H_DEFINE
	#define DECLARE_DLL_PROC(name, dll, ret, call, ...) \
		name##Proc name = NULL;
#else
	#define DLL_H_DECLARE
	#define DECLARE_DLL_PROC(name, dll, ret, call, ...) \
		typedef ret (call *name##Proc)(__VA_ARGS__); \
		extern name##Proc name;
#endif

#ifdef DLL_GETPROCADDR
void Init() {
#else
namespace dll {
#endif



DECLARE_DLL_PROC(CreateSymbolicLinkW, "kernel32", BOOL, APIENTRY, LPCWSTR lpSymlinkFileName, LPCWSTR lpTargetFileName, DWORD dwFlags)
#ifndef _WIN64
DECLARE_DLL_PROC(IsWow64Process, "kernel32", BOOL, WINAPI, HANDLE hProcess, PBOOL Wow64Process)
DECLARE_DLL_PROC(Wow64DisableWow64FsRedirection, "kernel32", BOOL, WINAPI, PVOID* OldValue)
DECLARE_DLL_PROC(Wow64RevertWow64FsRedirection, "kernel32", BOOL, WINAPI, PVOID OldValue)
#endif



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
