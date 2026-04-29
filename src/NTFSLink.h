#pragma once

/*
 * http://msdn.microsoft.com/en-us/library/ms791514.aspx
 */
#ifndef SYMLINK_FLAG_RELATIVE
#define SYMLINK_FLAG_RELATIVE 1
#endif

#ifndef REPARSE_DATA_BUFFER_HEADER_SIZE
typedef struct _REPARSE_DATA_BUFFER {
	ULONG  ReparseTag;
	USHORT  ReparseDataLength;
	USHORT  Reserved;
	union {
		struct {
			USHORT  SubstituteNameOffset;
			USHORT  SubstituteNameLength;
			USHORT  PrintNameOffset;
			USHORT  PrintNameLength;
			ULONG  Flags;
			WCHAR  PathBuffer[1];
		} SymbolicLinkReparseBuffer;
		struct {
			USHORT  SubstituteNameOffset;
			USHORT  SubstituteNameLength;
			USHORT  PrintNameOffset;
			USHORT  PrintNameLength;
			WCHAR  PathBuffer[1];
		} MountPointReparseBuffer;
		struct {
			UCHAR  DataBuffer[1];
		} GenericReparseBuffer;
	};
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

#define REPARSE_DATA_BUFFER_HEADER_SIZE  FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer)

#define MAXIMUM_REPARSE_DATA_BUFFER_SIZE  ( 16 * 1024 )
#endif
#ifdef __GNUC__
#undef REPARSE_DATA_BUFFER_HEADER_SIZE
#define REPARSE_DATA_BUFFER_HEADER_SIZE 8
#endif

BOOL WINAPI GetReparsePointTarget(LPCWSTR sLinkPath, LPWSTR sTarget, DWORD cch);
BOOL WINAPI CreateJunctionPoint(LPCWSTR sLinkPath, LPCWSTR sTarget);
BOOL WINAPI DeleteReparsePointFile(LPCWSTR sLinkPath);
