#include "stdafx.h"
#include "NTFSLink.h"

#define RETURN(r) { ret = (r); goto FINALIZE; }

__inline
HANDLE OpenJunctionPointFile(LPCTSTR sFile, DWORD dwDesiredAccess)
{
	return CreateFile(sFile, dwDesiredAccess, 0, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, 0);
}

BOOL WINAPI GetReparsePointTarget(LPCWSTR sLinkPath, LPWSTR sTarget, DWORD cch)
{
	HANDLE hFile;
	char buf[REPARSE_DATA_BUFFER_HEADER_SIZE + MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
	PREPARSE_DATA_BUFFER pBuffer;
	DWORD len;
	BOOL ret = TRUE;

	pBuffer = (PREPARSE_DATA_BUFFER) buf;

	if ((hFile = OpenJunctionPointFile(sLinkPath, GENERIC_READ)) == INVALID_HANDLE_VALUE)
		return FALSE;

	if (!DeviceIoControl(hFile, FSCTL_GET_REPARSE_POINT, NULL, 0, pBuffer, REPARSE_DATA_BUFFER_HEADER_SIZE + MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &len, NULL))
		RETURN(FALSE);

	if (pBuffer->ReparseTag == IO_REPARSE_TAG_SYMLINK) {
		len = pBuffer->SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(WCHAR);
		if (len >= cch)
			RETURN(FALSE);
		memcpy(sTarget,
			pBuffer->SymbolicLinkReparseBuffer.PathBuffer + pBuffer->SymbolicLinkReparseBuffer.SubstituteNameOffset / sizeof(WCHAR),
			pBuffer->SymbolicLinkReparseBuffer.SubstituteNameLength);
		sTarget[len] = 0;
	}
	else if (pBuffer->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT) {
		len = pBuffer->MountPointReparseBuffer.SubstituteNameLength / sizeof(WCHAR);
		if (len >= cch)
			RETURN(FALSE);
		memcpy(sTarget,
			pBuffer->MountPointReparseBuffer.PathBuffer + pBuffer->MountPointReparseBuffer.SubstituteNameOffset / sizeof(WCHAR),
			pBuffer->MountPointReparseBuffer.SubstituteNameLength);
		sTarget[len] = 0;
	}
	else
		ret = FALSE;

FINALIZE:
	CloseHandle(hFile);
	return ret;
}

static const size_t MOUNT_POINT_PATH_OFFSET = offsetof(REPARSE_DATA_BUFFER, MountPointReparseBuffer.PathBuffer);
BOOL WINAPI CreateJunctionPoint(LPCWSTR sLinkPath, LPCWSTR sTarget)
{
	HANDLE hFile;
	char buf[MOUNT_POINT_PATH_OFFSET + 32767 * sizeof(WCHAR)];
	PREPARSE_DATA_BUFFER pBuffer;
	DWORD len;
	BOOL ret = TRUE;

	if (!CreateDirectory(sLinkPath, NULL))
		return FALSE;

	pBuffer = (PREPARSE_DATA_BUFFER) buf;

	pBuffer->ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;
	pBuffer->Reserved = 0;

	if (sTarget[0] == L'\\' && sTarget[1] == L'?') {
		wcscpy(pBuffer->MountPointReparseBuffer.PathBuffer, sTarget);
	}
	else {
		wcscpy(pBuffer->MountPointReparseBuffer.PathBuffer, L"\\??\\");
		if (!GetFullPathName(sTarget, 32767 - 4, pBuffer->MountPointReparseBuffer.PathBuffer + 4, NULL))
			return FALSE;
	}

	len = wcslen(pBuffer->MountPointReparseBuffer.PathBuffer) * sizeof(WCHAR);

	// Set PrintName to ""
	*((WCHAR *)(((char *) pBuffer->MountPointReparseBuffer.PathBuffer) + len + sizeof(WCHAR))) = 0;

	pBuffer->MountPointReparseBuffer.SubstituteNameLength = len;
	pBuffer->MountPointReparseBuffer.SubstituteNameOffset = 0;
	pBuffer->MountPointReparseBuffer.PrintNameLength = 0;
	pBuffer->MountPointReparseBuffer.PrintNameOffset = len + sizeof(WCHAR);
	// + sizeof(WCHAR) * 2 for the '\0's of SubstituteName and PrintName
	pBuffer->ReparseDataLength = len + sizeof(WCHAR) * 2 + MOUNT_POINT_PATH_OFFSET - REPARSE_DATA_BUFFER_HEADER_SIZE;

	if ((hFile = OpenJunctionPointFile(sLinkPath, GENERIC_WRITE)) == INVALID_HANDLE_VALUE) {
		RemoveDirectory(sLinkPath);
		return FALSE;
	}

	if (!DeviceIoControl(hFile, FSCTL_SET_REPARSE_POINT, pBuffer, pBuffer->ReparseDataLength + REPARSE_DATA_BUFFER_HEADER_SIZE, NULL, 0, &len, NULL)) {
		RemoveDirectory(sLinkPath);
		ret = FALSE;
	}

	CloseHandle(hFile);
	return ret;
}

BOOL WINAPI DeleteReparsePointFile(LPCWSTR sLinkPath)
{
	HANDLE hFile;
	REPARSE_GUID_DATA_BUFFER rgdb = {0};
	DWORD len;

	if ((hFile = OpenJunctionPointFile(sLinkPath, GENERIC_WRITE)) == INVALID_HANDLE_VALUE)
		return FALSE;

	rgdb.ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;
	if (!DeviceIoControl(hFile, FSCTL_DELETE_REPARSE_POINT, &rgdb, REPARSE_GUID_DATA_BUFFER_HEADER_SIZE, NULL, 0, &len, NULL)) {
		CloseHandle(hFile);
		return FALSE;
	}

	CloseHandle(hFile);

	return RemoveDirectory(sLinkPath) || DeleteFile(sLinkPath);
}
