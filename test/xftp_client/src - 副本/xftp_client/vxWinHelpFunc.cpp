/**
* @file    vxWinHelpFunc.cpp
* <pre>
* Copyright (c) 2016, Gaaagaa All rights reserved.
* 
* 文件名称：vxWinHelpFunc.cpp
* 创建日期：2016年03月31日
* 文件标识：
* 文件摘要：Windows 平台上简单功能的实现函数。
* 
* 当前版本：1.0.0.0
* 作    者：
* 完成日期：2016年03月31日
* 版本摘要：
* 
* 取代版本：
* 原作者  ：
* 完成日期：
* 版本摘要：
* </pre>
*/

#include "stdafx.h"
#include "vxWinHelpFunc.h"

#include <cassert>

#include <io.h>
#include <stdio.h>
#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include <WtsApi32.h>
#pragma comment(lib, "wtsapi32.lib")

#include <UserEnv.h>
#pragma comment(lib, "userenv.lib")

#pragma comment(lib, "advapi32.lib")

////////////////////////////////////////////////////////////////////////////////

/******************************************************************************/
/**
* @brief
* <pre>
*   访问类型 : public 
*   功能描述 : 目录文件扫描操作。
* </pre>
* 
* @param [in ] szPathDir  : 扫描的目标目录路径。
* @param [in ] szFilter   : 过滤器规则。
* @param [in ] bSubDir    : 是否递归扫描子目录。
* @param [in ] xFuncFName : 扫描到文件后的回调操作函数。
* @param [in ] dwContext  : 用户数据（用于回调函数的回传参数）。
* 
* @return int
*         - 返回 emScanResult 类型值（参看 emScanResult 枚举定义）。
*/
int ScanDirectory(LPCTSTR szPathDir, LPCTSTR szFilter, BOOL bScanSubDir, FINDFILE_FNCBK xFuncFName, DWORD_PTR dwContext)
{
#ifdef UNICODE
	return ScanDirectoryW(szPathDir, szFilter, bScanSubDir, (FINDFILE_FNCBKW)xFuncFName, dwContext);
#else // UNICODE
	return ScanDirectoryA(szPathDir, szFilter, bScanSubDir, (FINDFILE_FNCBKA)xFuncFName, dwContext);
#endif // UNICODE
}

int ScanDirectoryA(LPCSTR szPathDir, LPCSTR szFilter, BOOL bScanSubDir, FINDFILE_FNCBKA xFuncFName, DWORD_PTR dwContext)
{
	int err = SCAN_OK;

	CHAR szPath[MAX_PATH];
	sprintf_s(szPath, MAX_PATH, "%s\\%s", szPathDir, (NULL != szFilter) ? szFilter : "*.*");

	WIN32_FIND_DATAA findData;
	HANDLE hFind = FindFirstFileA(szPath, &findData);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		return SCAN_ERR;
	}

	// 处理之后查找下一个，直到都找完
	do 
	{
		// 忽略 "." 和 ".."
		if (('.' == findData.cFileName[0]) && (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			if (('\0' == findData.cFileName[1]) ||
				(('.' == findData.cFileName[1]) && ('\0' == findData.cFileName[2])))
			{
				continue;
			}
		}

		// 文件路径名
		sprintf_s(szPath, MAX_PATH, "%s\\%s", szPathDir, findData.cFileName);

		// 对文件，用回调函数处理扫描到的文件
		if (NULL != xFuncFName)
		{
			if (!xFuncFName(szPath, findData.cFileName, dwContext))
			{
				err = SCAN_ABORT;
				break;
			}
		}

		// 递归调用，扫描子目录
		if (bScanSubDir && (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			if (SCAN_ABORT == ScanDirectoryA(szPath, szFilter, bScanSubDir, xFuncFName, dwContext))
			{
				err = SCAN_ABORT;
				break;
			}
		}

	} while (FindNextFileA(hFind, &findData));

	FindClose(hFind);

	return err;
}

int ScanDirectoryW(LPCWSTR szPathDir, LPCWSTR szFilter, BOOL bScanSubDir, FINDFILE_FNCBKW xFuncFName, DWORD_PTR dwContext)
{
	int err = SCAN_OK;

	WCHAR szPath[MAX_PATH];
	swprintf_s(szPath, MAX_PATH, L"%s\\%s", szPathDir, (NULL != szFilter) ? szFilter : L"*.*");

	WIN32_FIND_DATAW findData;
	HANDLE hFind = FindFirstFileW(szPath, &findData);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		return SCAN_ERR;
	}

	// 处理之后查找下一个，直到都找完
	do 
	{
		// 忽略 "." 和 ".."
		if ((L'.' == findData.cFileName[0]) && (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			if ((L'\0' == findData.cFileName[1]) ||
				((L'.' == findData.cFileName[1]) && (L'\0' == findData.cFileName[2])))
			{
				continue;
			}
		}

		// 文件路径名
		swprintf_s(szPath, MAX_PATH, L"%s\\%s", szPathDir, findData.cFileName);

		// 对文件，用回调函数处理扫描到的文件
		if (NULL != xFuncFName)
		{
			if (!xFuncFName(szPath, findData.cFileName, dwContext))
			{
				err = SCAN_ABORT;
				break;
			}
		}

		// 递归调用，扫描子目录
		if (bScanSubDir && (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			if (SCAN_ABORT == ScanDirectoryW(szPath, szFilter, bScanSubDir, xFuncFName, dwContext))
			{
				err = SCAN_ABORT;
				break;
			}
		}

	} while (FindNextFileW(hFind, &findData));

	FindClose(hFind);

	return err;
}

/******************************************************************************/
/**
* @brief
* <pre>
*   访问类型 : public 
*   功能描述 : 删除目录下所有文件扫描操作。
* </pre>
* 
* @param [in ] szPathDir   : 操作的目标目录路径。
* @param [in ] szFilter    : 过滤器规则。
* @param [in ] bScanSubDir : 是否递归扫描删除子目录的文件。
* 
* @return int
*         - 返回删除的文件数量。
*/
int CleanupDir(LPCTSTR szPathDir, LPCTSTR szFilter, BOOL bScanSubDir)
{
#ifdef UNICODE
	return CleanupDirW(szPathDir, szFilter, bScanSubDir);
#else // UNICODE
	return CleanupDirA(szPathDir, szFilter, bScanSubDir);
#endif // UNICODE
}

int CleanupDirA(LPCSTR szPathDir, LPCSTR szFilter, BOOL bScanSubDir)
{
	int scanResult = 0;

	CHAR szPath[MAX_PATH];
	sprintf_s(szPath, MAX_PATH, "%s\\%s", szPathDir, szFilter);

	WIN32_FIND_DATAA findData;
	HANDLE hFind = FindFirstFileA(szPath, &findData);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		return 0;
	}

	// 处理之后查找下一个，直到都找完
	do 
	{
		// 忽略 "." 和 ".."
		if (('.' == findData.cFileName[0]) && (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			if (('\0' == findData.cFileName[1]) ||
				(('.' == findData.cFileName[1]) && ('\0' == findData.cFileName[2])))
			{
				continue;
			}
		}

		// 路径名
		sprintf_s(szPath, MAX_PATH, "%s\\%s", szPathDir, findData.cFileName);

		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// 递归调用，扫描子目录
			if (bScanSubDir)
			{
				scanResult += CleanupDirA(szPath, szFilter, bScanSubDir);
				RemoveDirectoryA(szPath);
			}
		}
		else
		{
			// 执行删除操作
			if (DeleteFileA(szPath))
				scanResult += 1;
		}

	} while (FindNextFileA(hFind, &findData));

	FindClose(hFind);

	return scanResult;
}

int CleanupDirW(LPCWSTR szPathDir, LPCWSTR szFilter, BOOL bScanSubDir)
{
	int scanResult = 0;

	WCHAR szPath[MAX_PATH];
	swprintf_s(szPath, MAX_PATH, L"%s\\%s", szPathDir, szFilter);

	WIN32_FIND_DATAW findData;
	HANDLE hFind = FindFirstFileW(szPath, &findData);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		return 0;
	}

	// 处理之后查找下一个，直到都找完
	do 
	{
		// 忽略 "." 和 ".."
		if ((L'.' == findData.cFileName[0]) && (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			if ((L'\0' == findData.cFileName[1]) ||
				((L'.' == findData.cFileName[1]) && (L'\0' == findData.cFileName[2])))
			{
				continue;
			}
		}

		// 路径名
		swprintf_s(szPath, MAX_PATH, L"%s\\%s", szPathDir, findData.cFileName);

		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// 递归调用，扫描子目录
			if (bScanSubDir)
			{
				scanResult += CleanupDirW(szPath, szFilter, bScanSubDir);
				RemoveDirectoryW(szPath);
			}
		}
		else
		{
			// 执行删除操作
			if (DeleteFileW(szPath))
				scanResult += 1;
		}

	} while (FindNextFileW(hFind, &findData));

	FindClose(hFind);

	return scanResult;
}

/******************************************************************************/
/**
* @brief
* <pre>
*   访问类型 : public 
*   功能描述 : 创建全路径目录（递归方式创建完整目录结构）。
* </pre>
* 
* @param [in ] szDir : 路径目录。
* 
* @return BOOL
*         - 成功，返回 TRUE；
*         - 失败，返回 FALSE。
*/
BOOL MkPathDir(LPCTSTR szDir)
{
#ifdef UNICODE
	return MkPathDirW(szDir);
#else // UNICODE
	return MkPathDirA(szDir);
#endif // UNICODE
}

BOOL MkPathDirA(LPCSTR szDir)
{
	int err = -1;

	CHAR cp = '\0';
	CHAR szPath[MAX_PATH];
	CHAR *szPt = szPath;

	if ((szDir == NULL) || (strlen(szDir) > MAX_PATH))
		return FALSE;

	memset(szPath, 0, MAX_PATH * sizeof(CHAR));

	while (*szPt++ = cp = *szDir++)
	{
		if ((cp == '\\') || (cp == '/') || (*szDir == '\0'))
		{
			if (_access_s(szPath, 0) == 0)
			{
				err = 0;
			}
			else
			{
				err = _mkdir(szPath);
				if (err != 0)
					break;
			}
		}
	}

	return (err == 0);
}

BOOL MkPathDirW(LPCWSTR szDir)
{
	int err = -1;

	WCHAR cp = L'\0';
	WCHAR szPath[MAX_PATH];
	WCHAR *szPt = szPath;

	if ((szDir == NULL) || (wcslen(szDir) > MAX_PATH))
		return FALSE;

	memset(szPath, 0, MAX_PATH * sizeof(WCHAR));

	while (*szPt++ = cp = *szDir++)
	{
		if ((cp == L'\\') || (cp == L'/') || (*szDir == L'\0'))
		{
			if (_waccess_s(szPath, 0) == 0)
			{
				err = 0;
			}
			else
			{
				err = _wmkdir(szPath);
				if (err != 0)
					break;
			}
		}
	}

	return (err == 0);
}

/******************************************************************************/
/**
* @brief
* <pre>
*   访问类型 : public 
*   功能描述 : 根据给定的文件全路径名，创建其路径目录。
* </pre>
* 
* @param [in ] szFilePath : 文件全路径名。
* 
* @return BOOL
*         - 成功，返回 TRUE；
*         - 失败，返回 FALSE。
*/
BOOL MkFilePathDir(LPCTSTR szFilePath)
{
#ifdef UNICODE
	return MkFilePathDirW(szFilePath);
#else // UNICODE
	return MkFilePathDirA(szFilePath);
#endif // UNICODE
}

BOOL MkFilePathDirA(LPCSTR szFilePath)
{
	int nEndPos = 0;
	CHAR szPath[MAX_PATH];

	if (NULL == szFilePath)
	{
		return FALSE;
	}

	LPCSTR szEndPos = strrchr(szFilePath, '\\');
	if (NULL == szEndPos)
	{
		szEndPos = strrchr(szFilePath, '/');
		if (NULL == szEndPos)
			return FALSE;
	}
	else
	{
		LPCSTR szREndPos = strrchr(szEndPos, '/');
		if (NULL != szREndPos)
			szEndPos = szREndPos;
	}

	nEndPos = (int)(szEndPos - szFilePath + 1);
	strncpy_s(szPath, MAX_PATH, szFilePath, nEndPos);
	szPath[nEndPos] = '\0';

	return MkPathDirA(szPath);
}

BOOL MkFilePathDirW(LPCWSTR szFilePath)
{
	int nEndPos = 0;
	WCHAR szPath[MAX_PATH];

	if (NULL == szFilePath)
	{
		return FALSE;
	}

	LPCWSTR szEndPos = wcsrchr(szFilePath, L'\\');
	if (NULL == szEndPos)
	{
		szEndPos = wcsrchr(szFilePath, L'/');
		if (NULL == szEndPos)
			return FALSE;
	}
	else
	{
		LPCWSTR szREndPos = wcsrchr(szEndPos, L'/');
		if (NULL != szREndPos)
			szEndPos = szREndPos;
	}

	nEndPos = (int)(szEndPos - szFilePath + 1);
	wcsncpy_s(szPath, MAX_PATH, szFilePath, nEndPos);
	szPath[nEndPos] = L'\0';

	return MkPathDirW(szPath);
}

/******************************************************************************/
/**
* @brief
* <pre>
*   访问类型 : public
*   功能描述 : 从文件全路径名中，获取其文件名。
*             （例如：D:\Folder\FileName.txt 返回 FileName.txt）。
* </pre>
*
* @param [in ] szFilePath : 文件全路径名。
*
* @return LPCTSTR
*         - 返回对应的 文件名。
*/
LPCTSTR FileBaseName(LPCTSTR szFilePath)
{
#ifdef UNICODE
	return FileBaseNameW(szFilePath);
#else // UNICODE
	return FileBaseNameA(szFilePath);
#endif // UNICODE
}

LPCSTR FileBaseNameA(LPCSTR szFilePath)
{
	register CHAR * wszIter = (CHAR *)szFilePath;
	register CHAR * wszIptr = (CHAR *)szFilePath;

	if (NULL == wszIptr)
	{
		return NULL;
	}

	while ('\0' != *wszIter)
	{
		if (('\\' == *wszIter) || ('/' == *wszIter))
			wszIptr = wszIter + 1;
		wszIter += 1;
	}

	return (LPCSTR)wszIptr;
}

LPCWSTR FileBaseNameW(LPCWSTR wszFilePath)
{
	register WCHAR * wszIter = (WCHAR *)wszFilePath;
	register WCHAR * wszIptr = (WCHAR *)wszFilePath;

	if (NULL == wszIptr)
	{
		return NULL;
	}

	while (L'\0' != *wszIter)
	{
		if ((L'\\' == *wszIter) || (L'/' == *wszIter))
			wszIptr = wszIter + 1;
		wszIter += 1;
	}

	return (LPCWSTR)wszIptr;
}

/******************************************************************************/
/**
* @brief
* <pre>
*   访问类型 : public
*   功能描述 : 从文件全路径名中，获取其文件扩展名。
*             （例如：D:\Folder\FileName.txt 返回 txt）。
* </pre>
* 
* @param [in ] szFilePath : 文件全路径名。
* 
* @return LPCTSTR
*         - 返回对应的 文件扩展名。
*/
LPCTSTR FileExtName(LPCTSTR szFilePath)
{
#ifdef UNICODE
	return FileExtNameW(szFilePath);
#else // UNICODE
	return FileExtNameA(szFilePath);
#endif // UNICODE
}

LPCSTR  FileExtNameA(LPCSTR  szFilePath)
{
	register CHAR * wszIter = (CHAR *)szFilePath;
	register CHAR * wszIptr = (CHAR *)szFilePath;

	if (NULL == wszIptr)
	{
		return NULL;
	}

	while ('\0' != *wszIter)
	{
		if ('.' == *wszIter)
			wszIptr = wszIter + 1;
		wszIter += 1;
	}

	return (LPCSTR)wszIptr;
}

LPCWSTR FileExtNameW(LPCWSTR wszFilePath)
{
	register WCHAR * wszIter = (WCHAR *)wszFilePath;
	register WCHAR * wszIptr = (WCHAR *)wszFilePath;

	if (NULL == wszIptr)
	{
		return NULL;
	}

	while (L'\0' != *wszIter)
	{
		if (L'.' == *wszIter)
			wszIptr = wszIter + 1;
		wszIter += 1;
	}

	return (LPCWSTR)wszIptr;
}

/******************************************************************************/
/**
* @brief
* <pre>
*   访问类型 : public 
*   功能描述 : 根据给定的文件路径名，判断文件是否存在。
* </pre>
* 
* @param [in ] szFilePathName : 文件路径名。
* 
* @return BOOL
*         - 返回 TRUE，表示存在；
*         - 返回 FALSE，表示不存在。
*/
BOOL IsFileExist(LPCTSTR szFilePathName)
{
#ifdef UNICODE
	return IsFileExistW(szFilePathName);
#else // UNICODE
	return IsFileExistA(szFilePathName);
#endif // UNICODE
}

BOOL IsFileExistA(LPCSTR szFilePathName)
{
	if ((NULL != szFilePathName) &&
		(0 == _access_s(szFilePathName, 0)) &&
		!PathIsDirectoryA(szFilePathName))
	{
		return TRUE;
	}

	return FALSE;
}

BOOL IsFileExistW(LPCWSTR szFilePathName)
{
	if ((NULL != szFilePathName) &&
		(0 == _waccess_s(szFilePathName, 0)) &&
		!PathIsDirectoryW(szFilePathName))
	{
		return TRUE;
	}

	return FALSE;
}

/******************************************************************************/
/**
* @brief
* <pre>
*   访问类型 : public 
*   功能描述 : 根据给定的文件路径名，返回文件大小。
* </pre>
* 
* @param [in ] szFilePath : 文件路径名。
* 
* @return ULONG
*         - 文件大小（按字节为单位）。
*/
ULONG StatGetFileSize(LPCTSTR szFilePath)
{
#ifdef UNICODE
	return StatGetFileSizeW(szFilePath);
#else // UNICODE
	return StatGetFileSizeA(szFilePath);
#endif // UNICODE
}

ULONG StatGetFileSizeA(LPCSTR szFilePath)
{
	ULONG ulSize = 0;
	struct _stat statbuf;

	if (NULL == szFilePath)
	{
		return 0;
	}

	if (_stat(szFilePath, &statbuf) < 0)
		return ulSize;
	else
		ulSize = statbuf.st_size;

	return ulSize;
}

ULONG StatGetFileSizeW(LPCWSTR szFilePath)
{
	ULONG ulSize = 0;
	struct _stat statbuf;

	if (NULL == szFilePath)
	{
		return 0;
	}

	if (_wstat(szFilePath, &statbuf) < 0)
		return ulSize;
	else
		ulSize = statbuf.st_size;

	return ulSize;
}

/******************************************************************************/
/**
* @brief
* <pre>
*   访问类型 : public 
*   功能描述 : 将整个文件数据读取到缓存中（缓存长度必须大于等于文件大小）。
* </pre>
* 
* @param [in ] szFileName : 文件名。
* @param [in ] bUnicode   : 文件名是否为 UNICODE 字符串。
* @param [out] lpData     : 缓存数据。
* @param [in ] dwLen      : 缓存数据长度。
* 
* @return int
*         - 成功，返回 读取到的字节数；
*         - 失败，返回 -1（通过 GetLastError() 获取 系统错误码）。
*/
int ReadFileToMemV(LPVOID szFileName, BOOL bUnicode, LPBYTE lpData, DWORD dwLen)
{
	int err = -1;

	HANDLE hFile = INVALID_HANDLE_VALUE;

	DWORD dwDesiredAccess       = GENERIC_READ;
	DWORD dwCreationDisposition = OPEN_EXISTING;

	do 
	{
		if (NULL == szFileName)
		{
			SetLastError(STG_E_INVALIDPOINTER);
			break;
		}

		// 打开文件
		if (bUnicode)
		{
			hFile = CreateFileW((LPCWSTR)szFileName, dwDesiredAccess,
				FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
		}
		else
		{
			hFile = CreateFileA((LPCSTR)szFileName, dwDesiredAccess,
				FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
		}
		if (INVALID_HANDLE_VALUE == hFile)
		{
			break;
		}

		// 读取文件大小
		DWORD dwFileSize = GetFileSize(hFile, NULL);
		if (dwFileSize > dwLen)
		{
			break;
		}

		// 读取数据
		DWORD dwReadCount = 0;
		while (dwReadCount < dwFileSize)
		{
			DWORD dwReadBytes = 0;
			if (!ReadFile(hFile, lpData + dwReadCount, dwFileSize - dwReadCount, &dwReadBytes, NULL))
			{
				break;
			}

			dwReadCount += dwReadBytes;
			if (0 == dwReadBytes)
			{
				break;
			}
		}

		// 若读取字节数不等于待文件大小，则产生错误
		if (dwReadCount != dwFileSize)
		{
			// ...
		}

		err = (int)dwFileSize;

	} while (0);

	if (INVALID_HANDLE_VALUE != hFile)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	return err;
}

/******************************************************************************/
/**
* @brief
* <pre>
*   访问类型 : public 
*   功能描述 : 将整个文件数据读取到缓存中（缓存长度必须大于等于文件大小）。
* </pre>
* 
* @param [in ] szFileName : 文件名。
* @param [out] lpData     : 缓存数据。
* @param [in ] dwLen      : 缓存数据长度。
* 
* @return int
*         - 成功，返回 读取到的字节数；
*         - 失败，返回 -1（通过 GetLastError() 获取 系统错误码）。
*/
int ReadFileToMem(LPCTSTR szFileName, LPBYTE lpData, DWORD dwLen)
{
#ifdef UNICODE
	return ReadFileToMemV((LPVOID)szFileName, TRUE, lpData, dwLen);
#else // UNICODE
	return ReadFileToMemV((LPVOID)szFileName, FALSE, lpData, dwLen);
#endif // UNICODE
}

int ReadFileToMemA(LPCSTR szFileName, LPBYTE lpData, DWORD dwLen)
{
	return ReadFileToMemV((LPVOID)szFileName, FALSE, lpData, dwLen);
}

int ReadFileToMemW(LPCWSTR szFileName, LPBYTE lpData, DWORD dwLen)
{
	return ReadFileToMemV((LPVOID)szFileName, TRUE, lpData, dwLen);
}

/******************************************************************************/
/**
* @brief
* <pre>
*   访问类型 : public 
*   功能描述 : 将缓存数据写入文件（直接创建新文件写入数据）。
* </pre>
* 
* @param [in ] szFileName : 文件名。
* @param [in ] bUnicode   : 文件名是否为 UNICODE 字符串。
* @param [in ] lpData     : 缓存数据。
* @param [in ] dwLen      : 缓存数据长度。
* 
* @return int
*         - 成功，返回 写入的字节数；
*         - 失败，返回 -1（通过 GetLastError() 获取 系统错误码）。
*/
int WriteMemToFileV(LPVOID szFileName, BOOL bUnicode, LPBYTE lpData, DWORD dwLen)
{
	int err = -1;

	HANDLE hFile = INVALID_HANDLE_VALUE;

	DWORD dwDesiredAccess       = GENERIC_WRITE;
	DWORD dwCreationDisposition = CREATE_ALWAYS;

	do 
	{
		if (NULL == szFileName)
		{
			SetLastError(STG_E_INVALIDPOINTER);
			break;
		}

		// 创建存储文件
		if (bUnicode)
		{
			hFile = CreateFileW((LPCWSTR)szFileName, dwDesiredAccess,
				FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
		}
		else
		{
			hFile = CreateFileA((LPCSTR)szFileName, dwDesiredAccess,
				FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
		}
		if (INVALID_HANDLE_VALUE == hFile)
		{
			break;
		}

		// 写入数据
		DWORD dwWriteCount = 0;
		while (dwWriteCount < dwLen)
		{
			DWORD dwWriteBytes = 0;
			if (!WriteFile(hFile, lpData + dwWriteCount, dwLen - dwWriteCount, &dwWriteBytes, NULL))
			{
				break;
			}

			dwWriteCount += dwWriteBytes;
			if (0 == dwWriteBytes)
			{
				break;
			}
		}

		// 若写入字节数不等于待写入长度，则可能是磁盘空间不足
		if (dwWriteCount != dwLen)
		{
			// ...
		}

		err = dwWriteCount;

	} while (0);

	if (INVALID_HANDLE_VALUE != hFile)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	return err;
}

/******************************************************************************/
/**
* @brief
* <pre>
*   访问类型 : public 
*   功能描述 : 将缓存数据写入文件（直接创建新文件写入数据）。
* </pre>
* 
* @param [in ] szFileName : 文件名。
* @param [in ] lpData     : 缓存数据。
* @param [in ] dwLen      : 缓存数据长度。
* 
* @return int
*         - 成功，返回 写入的字节数；
*         - 失败，返回 -1（通过 GetLastError() 获取 系统错误码）。
*/
int WriteMemToFile(LPCTSTR szFileName, LPBYTE lpData, DWORD dwLen)
{
#ifdef UNICODE
	return WriteMemToFileV((LPVOID)szFileName, TRUE, lpData, dwLen);
#else // UNICODE
	return WriteMemToFileV((LPVOID)szFileName, FALSE, lpData, dwLen);
#endif // UNICODE
}

int WriteMemToFileA(LPCSTR szFileName, LPBYTE lpData, DWORD dwLen)
{
	return WriteMemToFileV((LPVOID)szFileName, FALSE, lpData, dwLen);
}

int WriteMemToFileW(LPCWSTR szFileName, LPBYTE lpData, DWORD dwLen)
{
	return WriteMemToFileV((LPVOID)szFileName, TRUE, lpData, dwLen);
}

/******************************************************************************/
/**
* @brief
* <pre>
*   访问类型 : public 
*   功能描述 : 文件删除操作，可设置操作为 是否将文件放入回收站 。
* </pre>
* 
* @param [in ] szFileName : 文件路径名。
* @param [in ] bRecycle   : 删除操作，是否为 将文件放入回收站。
* 
* @return int
*         - 成功，返回 0；
*         - 失败，返回 操作错误码。
*/
int RecycleDeleteFile(LPCTSTR szFileName, BOOL bRecycle)
{
#ifdef UNICODE
	return RecycleDeleteFileW(szFileName, bRecycle);
#else // UNICODE
	return RecycleDeleteFileA(szFileName, bRecycle);
#endif // UNICODE
}

int RecycleDeleteFileA(LPCSTR szFileName, BOOL bRecycle)
{
	CHAR szPathName[MAX_PATH + 4];

	if (NULL == szFileName)
	{
		SetLastError(STG_E_INVALIDPOINTER);
		return STG_E_INVALIDPOINTER;
	}

	// copy string
	{
		register int    iter_rem = MAX_PATH + 4;
		register CHAR * iter_dst = (CHAR *)szPathName;
		register CHAR * iter_src = (CHAR *)szFileName;
		while ((0 != (*iter_dst++ = *iter_src++)) && (--iter_rem > 0)) ;
		while (--iter_rem > 0) *iter_dst++ = 0;
	}

	SHFILEOPSTRUCTA fOperator;

	fOperator.hwnd                  = NULL;
	fOperator.wFunc                 = FO_DELETE;
	fOperator.pFrom                 = szPathName;
	fOperator.pTo                   = NULL;
	fOperator.fFlags                = FOF_SILENT | FOF_NOERRORUI | FOF_NOCONFIRMATION;
	fOperator.fAnyOperationsAborted = FALSE;
	fOperator.hNameMappings         = NULL;
	fOperator.lpszProgressTitle     = NULL;

	if (bRecycle)
		fOperator.fFlags |= FOF_ALLOWUNDO;
	else
		fOperator.fFlags &= ~FOF_ALLOWUNDO;

	return SHFileOperationA(&fOperator);
}

int RecycleDeleteFileW(LPCWSTR szFileName, BOOL bRecycle)
{
	WCHAR szPathName[MAX_PATH + 4];

	if (NULL == szFileName)
	{
		SetLastError(STG_E_INVALIDPOINTER);
		return STG_E_INVALIDPOINTER;
	}

	// copy string
	{
		register int     iter_rem = MAX_PATH + 4;
		register WCHAR * iter_dst = (WCHAR *)szPathName;
		register WCHAR * iter_src = (WCHAR *)szFileName;
		while ((0 != (*iter_dst++ = *iter_src++)) && (--iter_rem > 0)) ;
		while (--iter_rem > 0) *iter_dst++ = 0;
	}

	SHFILEOPSTRUCTW fOperator;

	fOperator.hwnd                  = NULL;
	fOperator.wFunc                 = FO_DELETE;
	fOperator.pFrom                 = szPathName;
	fOperator.pTo                   = NULL;
	fOperator.fFlags                = FOF_SILENT | FOF_NOERRORUI | FOF_NOCONFIRMATION;
	fOperator.fAnyOperationsAborted = FALSE;
	fOperator.hNameMappings         = NULL;
	fOperator.lpszProgressTitle     = NULL;

	if (bRecycle)
		fOperator.fFlags |= FOF_ALLOWUNDO;
	else
		fOperator.fFlags &= ~FOF_ALLOWUNDO;

	return SHFileOperationW(&fOperator);
}

/******************************************************************************/
/**
* @brief
* <pre>
*   访问类型 : public 
*   功能描述 : 销毁工作线程。
* </pre>
* 
* @param [in ] hThread  : 线程句柄。
* @param [in ] nTimeout : 等待的超时时间。
* 
* @return void
*         
*/
void DestroyThread(HANDLE hThread, UINT nTimeout)
{
	assert(GetCurrentThread() != hThread);

	DWORD dwExitCode = 0;
	if ((NULL != hThread) && GetExitCodeThread(hThread, &dwExitCode) && (STILL_ACTIVE == dwExitCode))
	{
		DWORD dwWait = WaitForSingleObject(hThread, nTimeout);
		if (WAIT_TIMEOUT == dwWait)
		{
#pragma warning(disable: 6258)
			TerminateThread(hThread, -1);
#pragma warning(default: 6258)
		}
	}

	CloseHandle(hThread);
}

/******************************************************************************/
/**
* @brief
* <pre>
*   访问类型 : public 
*   功能描述 : 在当前用户环境下，启动指定进程。
* </pre>
* 
* @param [in ] szAppName : 进程名（参看 系统 API CreateProcessAsUser() 的 lpApplicationName 参数说明）。
* @param [in ] szCmdLine : 命令行（参看 系统 API CreateProcessAsUser() 的 lpCommandLine     参数说明）。
* 
* @return int
*         - 成功，返回 0；
*         - 失败，返回 错误码。
*/
int StartupSessionProcess(LPTSTR szAppName, LPTSTR szCmdLine)
{
#ifdef UNICODE
	return StartupSessionProcessW(szAppName, szCmdLine);
#else // UNICODE
	return StartupSessionProcessA(szAppName, szCmdLine);
#endif // UNICODE
}

int StartupSessionProcessA(LPSTR szAppName, LPSTR szCmdLine)
{
	//======================================

	int    nECode          = -1;       ///< 操作错误码
	DWORD  dwOperatorFlags = 0;        ///< 操作组合值标识
	DWORD  dwSessionId     = 0;        ///< 当前会话的 ID
	HANDLE hUserToken      = NULL;     ///< 当前登录用户的令牌
	HANDLE hUserTokenDup   = NULL;     ///< 复制的用户令牌
	HANDLE hPToken         = NULL;     ///< 进程令牌
	LPVOID lpEnvBlock      = NULL;     ///< 进程环境块

	LUID                lDebugUID;     ///< Debug 权限的 UID
	TOKEN_PRIVILEGES    tokenPriv;     ///< 当前进程的令牌权限

	STARTUPINFOA        xStartupInfo;  ///< 启动描述信息
	PROCESS_INFORMATION xProcessInfo;  ///< 进程创建后返回的描述信息

	//======================================

	do 
	{
		//======================================
		// 相关参数初始化

		memset(&lDebugUID, 0, sizeof(LUID));
		memset(&tokenPriv, 0, sizeof(TOKEN_PRIVILEGES));

		memset(&xStartupInfo, 0, sizeof(STARTUPINFO));
		memset(&xProcessInfo, 0, sizeof(PROCESS_INFORMATION));

		// 指定创建进程的窗口站，Windows 下唯一可交互的窗口站就是 winsta0\default
		xStartupInfo.cb        = sizeof(STARTUPINFO);
		xStartupInfo.lpDesktop = "winsta0\\default";

		//======================================

		// 得到当前活动的会话 ID ，即登录用户的会话 ID
		dwSessionId = WTSGetActiveConsoleSessionId();

		// 读取当前登录用户的令牌信息
		if (!WTSQueryUserToken(dwSessionId, &hUserToken))
		{
			nECode = GetLastError();
			break;
		}

		//======================================

		// 打开进程令牌
		dwOperatorFlags = TOKEN_ADJUST_PRIVILEGES |
						  TOKEN_QUERY             |
						  TOKEN_DUPLICATE         |
						  TOKEN_ASSIGN_PRIMARY    |
						  TOKEN_ADJUST_SESSIONID  |
						  TOKEN_READ              |
						  TOKEN_WRITE             ;
		if (!OpenProcessToken(GetCurrentProcess(), dwOperatorFlags, &hPToken))
		{
			nECode = GetLastError();
			break;
		}

		//======================================

		// 查找 DEBUG 权限的 UID
		if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &lDebugUID))
		{
			nECode = GetLastError();
			break;
		}

		// 设置令牌信息
		tokenPriv.PrivilegeCount           = 1;
		tokenPriv.Privileges[0].Luid       = lDebugUID;
		tokenPriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		//======================================

		// 复制当前用户的令牌
		if (!DuplicateTokenEx(hPToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &hUserTokenDup))
		{
			nECode  = GetLastError();
			break;
		}

		// 设置当前进程的令牌信息
		if (!SetTokenInformation(hUserTokenDup, TokenSessionId, (void *)&dwSessionId, sizeof(DWORD)))
		{
			nECode = GetLastError();
			break;
		}

		// 应用令牌权限
		if (!AdjustTokenPrivileges(hUserTokenDup, FALSE, &tokenPriv, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
		{
			nECode = GetLastError();
			break;
		}

		//======================================

		// 创建参数
		dwOperatorFlags = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE;

		// 创建进程环境块，保证环境块是在用户的环境下
		if (CreateEnvironmentBlock(&lpEnvBlock, hUserTokenDup, TRUE))
		{
			dwOperatorFlags |= CREATE_UNICODE_ENVIRONMENT;
		}

		// 创建用户进程
		if (!CreateProcessAsUserA(hUserTokenDup, szAppName, szCmdLine, NULL, NULL, FALSE,
								  dwOperatorFlags, lpEnvBlock, NULL, &xStartupInfo, &xProcessInfo))
		{
			nECode = GetLastError();
			break;
		}

		//======================================

		nECode = 0;

	} while (0);

	//======================================
	// 清理操作

	if (NULL != hUserToken)
	{
		CloseHandle(hUserToken);
		hUserToken = NULL;
	}

	if (NULL != hUserTokenDup)
	{
		CloseHandle(hUserTokenDup);
		hUserTokenDup = NULL;
	}

	if (NULL != hPToken)
	{
		CloseHandle(hPToken);
		hPToken = NULL;
	}

	if (NULL != lpEnvBlock)
	{
		DestroyEnvironmentBlock(lpEnvBlock);
		lpEnvBlock = NULL;
	}

	//======================================

	return nECode;
}

int StartupSessionProcessW(LPWSTR szAppName, LPWSTR szCmdLine)
{
	//======================================

	int    nECode          = -1;       ///< 操作错误码
	DWORD  dwOperatorFlags = 0;        ///< 操作组合值标识
	DWORD  dwSessionId     = 0;        ///< 当前会话的 ID
	HANDLE hUserToken      = NULL;     ///< 当前登录用户的令牌
	HANDLE hUserTokenDup   = NULL;     ///< 复制的用户令牌
	HANDLE hPToken         = NULL;     ///< 进程令牌
	LPVOID lpEnvBlock      = NULL;     ///< 进程环境块

	LUID                lDebugUID;     ///< Debug 权限的 UID
	TOKEN_PRIVILEGES    tokenPriv;     ///< 当前进程的令牌权限

	STARTUPINFO         xStartupInfo;  ///< 启动描述信息
	PROCESS_INFORMATION xProcessInfo;  ///< 进程创建后返回的描述信息

	//======================================

	do 
	{
		//======================================
		// 相关参数初始化

		memset(&lDebugUID, 0, sizeof(LUID));
		memset(&tokenPriv, 0, sizeof(TOKEN_PRIVILEGES));

		memset(&xStartupInfo, 0, sizeof(STARTUPINFO));
		memset(&xProcessInfo, 0, sizeof(PROCESS_INFORMATION));

		// 指定创建进程的窗口站，Windows 下唯一可交互的窗口站就是 winsta0\default
		xStartupInfo.cb        = sizeof(STARTUPINFO);
		xStartupInfo.lpDesktop = TEXT("winsta0\\default");

		//======================================

		// 得到当前活动的会话 ID ，即登录用户的会话 ID
		dwSessionId = WTSGetActiveConsoleSessionId();

		// 读取当前登录用户的令牌信息
		if (!WTSQueryUserToken(dwSessionId, &hUserToken))
		{
			nECode = GetLastError();
			break;
		}

		//======================================

		// 打开进程令牌
		dwOperatorFlags = TOKEN_ADJUST_PRIVILEGES |
						  TOKEN_QUERY             |
						  TOKEN_DUPLICATE         |
						  TOKEN_ASSIGN_PRIMARY    |
						  TOKEN_ADJUST_SESSIONID  |
						  TOKEN_READ              |
						  TOKEN_WRITE             ;
		if (!OpenProcessToken(GetCurrentProcess(), dwOperatorFlags, &hPToken))
		{
			nECode = GetLastError();
			break;
		}

		//======================================

		// 查找 DEBUG 权限的 UID
		if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &lDebugUID))
		{
			nECode = GetLastError();
			break;
		}

		// 设置令牌信息
		tokenPriv.PrivilegeCount           = 1;
		tokenPriv.Privileges[0].Luid       = lDebugUID;
		tokenPriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		//======================================

		// 复制当前用户的令牌
		if (!DuplicateTokenEx(hPToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &hUserTokenDup))
		{
			nECode  = GetLastError();
			break;
		}

		// 设置当前进程的令牌信息
		if (!SetTokenInformation(hUserTokenDup, TokenSessionId, (void *)&dwSessionId, sizeof(DWORD)))
		{
			nECode = GetLastError();
			break;
		}

		// 应用令牌权限
		if (!AdjustTokenPrivileges(hUserTokenDup, FALSE, &tokenPriv, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
		{
			nECode = GetLastError();
			break;
		}

		//======================================

		// 创建参数
		dwOperatorFlags = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE;

		// 创建进程环境块，保证环境块是在用户的环境下
		if (CreateEnvironmentBlock(&lpEnvBlock, hUserTokenDup, TRUE))
		{
			dwOperatorFlags |= CREATE_UNICODE_ENVIRONMENT;
		}

		// 创建用户进程
		if (!CreateProcessAsUserW(hUserTokenDup, szAppName, szCmdLine, NULL, NULL, FALSE,
								  dwOperatorFlags, lpEnvBlock, NULL, &xStartupInfo, &xProcessInfo))
		{
			nECode = GetLastError();
			break;
		}

		//======================================

		nECode = 0;

	} while (0);

	//======================================
	// 清理操作

	if (NULL != hUserToken)
	{
		CloseHandle(hUserToken);
		hUserToken = NULL;
	}

	if (NULL != hUserTokenDup)
	{
		CloseHandle(hUserTokenDup);
		hUserTokenDup = NULL;
	}

	if (NULL != hPToken)
	{
		CloseHandle(hPToken);
		hPToken = NULL;
	}

	if (NULL != lpEnvBlock)
	{
		DestroyEnvironmentBlock(lpEnvBlock);
		lpEnvBlock = NULL;
	}

	//======================================

	return nECode;
}

/******************************************************************************/
/**
* @brief
* <pre>
*   访问类型 : public 
*   功能描述 : 获取当前用户 SID （字符串形式 返回）。
* </pre>
* 
* @param [out] szDest  : 操作输出（字符串）的缓存。
* @param [in ] cchDest : 缓存大小。
* 
* @return BOOL
*         - 成功，返回 TRUE；
*         - 失败，返回 FALSE。
*/
BOOL GetCurrentUserSID(LPTSTR szDest, int cchDest)
{
#ifdef _UNICODE
	return GetCurrentUserSIDW(szDest, cchDest);
#else // _UNICODE
	return GetCurrentUserSIDA(szDest, cchDest);
#endif // _UNICODE
}

BOOL GetCurrentUserSIDA(LPSTR szDest, int cchDest)
{
	//======================================

	CHAR szUName[MAX_PATH]     = { 0 };
	CHAR szDomain[MAX_PATH]    = { 0 };
	CHAR szSIDBuffer[MAX_PATH] = { 0 };

	DWORD dwUNSize = MAX_PATH * sizeof(CHAR);
	DWORD dwDMSize = MAX_PATH * sizeof(CHAR);
	DWORD dwSBSize = MAX_PATH * sizeof(CHAR);

	SID_NAME_USE SidNameUse;

	PSID_IDENTIFIER_AUTHORITY SID_Authority_ptr = NULL;

	int nIPos = 0;
	int nIter = 0;
	int nSubs = 0;

	//======================================

	if ((NULL == szDest) || (cchDest <= 0))
	{
		return FALSE;
	}

	//======================================

	if (!GetUserNameA(szUName, &dwUNSize))
	{
		return FALSE;
	}

	//======================================

	if (!LookupAccountNameA(NULL, szUName, (PSID)szSIDBuffer, &dwSBSize,
							szDomain, &dwDMSize, &SidNameUse))
	{
		return FALSE;
	}

	if (!IsValidSid((PSID)szSIDBuffer))
	{
		return FALSE;
	}

	//======================================

	SID_Authority_ptr = GetSidIdentifierAuthority((PSID)szSIDBuffer);
	if (NULL == SID_Authority_ptr)
	{
		return FALSE;
	}

	nIPos += sprintf_s(szDest + nIPos, cchDest - nIPos,
					   "S-%lu-%lu", SID_REVISION, SID_Authority_ptr->Value[5]);
	if (nIPos >= cchDest)
	{
		return FALSE;
	}

	nSubs = *GetSidSubAuthorityCount((PSID)szSIDBuffer);
	for (nIter = 0; nIter < nSubs; ++nIter)
	{
		nIPos += sprintf_s(szDest + nIPos, cchDest - nIPos,
						   "-%lu", *GetSidSubAuthority((PSID)szSIDBuffer, nIter));
		if (nIPos >= cchDest)
		{
			return FALSE;
		}
	}

	//======================================

	return TRUE;
}

BOOL GetCurrentUserSIDW(LPWSTR szDest, int cchDest)
{
	//======================================

	WCHAR szUName[MAX_PATH]     = { 0 };
	WCHAR szDomain[MAX_PATH]    = { 0 };
	WCHAR szSIDBuffer[MAX_PATH] = { 0 };

	DWORD dwUNSize = MAX_PATH * sizeof(WCHAR);
	DWORD dwDMSize = MAX_PATH * sizeof(WCHAR);
	DWORD dwSBSize = MAX_PATH * sizeof(WCHAR);

	SID_NAME_USE SidNameUse;

	PSID_IDENTIFIER_AUTHORITY SID_Authority_ptr = NULL;

	int nIPos = 0;
	int nIter = 0;
	int nSubs = 0;

	//======================================

	if ((NULL == szDest) || (cchDest <= 0))
	{
		return FALSE;
	}

	//======================================

	if (!GetUserNameW(szUName, &dwUNSize))
	{
		return FALSE;
	}

	//======================================

	if (!LookupAccountNameW(NULL, szUName, (PSID)szSIDBuffer, &dwSBSize,
							szDomain, &dwDMSize, &SidNameUse))
	{
		return FALSE;
	}

	if (!IsValidSid((PSID)szSIDBuffer))
	{
		return FALSE;
	}

	//======================================

	SID_Authority_ptr = GetSidIdentifierAuthority((PSID)szSIDBuffer);
	if (NULL == SID_Authority_ptr)
	{
		return FALSE;
	}

	nIPos += swprintf_s(szDest + nIPos, cchDest - nIPos,
						L"S-%lu-%lu", SID_REVISION, SID_Authority_ptr->Value[5]);
	if (nIPos >= cchDest)
	{
		return FALSE;
	}

	nSubs = *GetSidSubAuthorityCount((PSID)szSIDBuffer);
	for (nIter = 0; nIter < nSubs; ++nIter)
	{
		nIPos += swprintf_s(szDest + nIPos, cchDest - nIPos,
							L"-%lu", *GetSidSubAuthority((PSID)szSIDBuffer, nIter));
		if (nIPos >= cchDest)
		{
			return FALSE;
		}
	}

	//======================================

	return TRUE;
}

//==============================================================================
// 字符串转换操作相关函数

/******************************************************************************/
/**
* ANSI <==> TEXT
*/
BOOL AnsiToText(LPTSTR szText, int cchDest, LPCSTR szAnsi)
{
	if ((NULL == szText) || (cchDest <= 0) || (NULL == szAnsi))
	{
		return FALSE;
	}

#ifdef _UNICODE
	return (0 != MultiByteToWideChar(CP_ACP, 0, szAnsi, -1, szText, cchDest));
#else // _UNICODE
	return (0 == strcpy_s(szText, cchDest, szAnsi));
#endif // _UNICODE
}

BOOL TextToAnsi(LPSTR szAnsi, int cchDest, LPCTSTR szText)
{
	if ((NULL == szAnsi) || (cchDest <= 0) || (NULL == szText))
	{
		return FALSE;
	}

#ifdef _UNICODE
	return (0 != WideCharToMultiByte(CP_ACP, 0, szText, -1, szAnsi, cchDest, NULL, NULL));
#else // _UNICODE
	return (0 == strcpy_s(szAnsi, cchDest, szText));
#endif // _UNICODE
}

/******************************************************************************/
/**
* ANSI <==> UNICODE
*/
BOOL AnsiToUnicode(LPWSTR szUnicode, int cchDest, LPCSTR szAnsi)
{
	if ((NULL == szUnicode) || (cchDest <= 0) || (NULL == szAnsi))
	{
		return FALSE;
	}

	if (0 == MultiByteToWideChar(CP_ACP, 0, szAnsi, -1, szUnicode, cchDest))
	{
		return FALSE;
	}

	return TRUE;
}

BOOL UnicodeToAnsi(LPSTR szAnsi, int cchDest, LPWSTR szUnicode)
{
	if ((NULL == szAnsi) || (cchDest <= 0) || (NULL == szUnicode))
	{
		return FALSE;
	}

	if (0 == WideCharToMultiByte(CP_ACP, 0, szUnicode, -1, szAnsi, cchDest, NULL, NULL))
	{
		return FALSE;
	}

	return TRUE;
}

/******************************************************************************/
/**
* ANSI <==> UTF8
*/
BOOL AnsiToUtf8(LPSTR szUtf8, int cchDest, LPCSTR szAnsi)
{
	// calculate count
	int nNumsChar = MultiByteToWideChar(CP_ACP, 0, szAnsi, -1, NULL, 0);
	if (0 == nNumsChar)
	{
		szUtf8[0] = '\0';
		return TRUE;
	}

	// to wide char.
	LPWSTR szWText = (LPWSTR)calloc(nNumsChar + 1, sizeof(WCHAR));
	if (NULL == szWText)
	{
		szUtf8[0] = '\0';
		return FALSE;
	}

	nNumsChar = MultiByteToWideChar(CP_ACP, 0, szAnsi, -1, szWText, nNumsChar + 1);
	if (0 == nNumsChar)
	{
		free(szWText);
		szWText = NULL;
		return FALSE;
	}

	// to utf8
	nNumsChar = WideCharToMultiByte(CP_UTF8, 0, szWText, -1, szUtf8, cchDest, NULL, NULL);
	free(szWText);
	szWText = NULL;

	return (0 != nNumsChar);
}

BOOL Utf8ToAnsi(LPSTR szAnsi, int cchDest, LPCSTR szUtf8)
{
	// calculate count
	int nNumsChar = MultiByteToWideChar(CP_UTF8, 0, szUtf8, -1, NULL, 0);
	if (0 == nNumsChar)
	{
		szAnsi[0] = '\0';
		return TRUE;
	}

	// to wide char.
	LPWSTR szWText = (LPWSTR)calloc(nNumsChar + 1, sizeof(WCHAR));
	if (NULL == szWText)
	{
		szAnsi[0] = '\0';
		return FALSE;
	}

	nNumsChar = MultiByteToWideChar(CP_UTF8, 0, szUtf8, -1, szWText, nNumsChar + 1);
	if (0 == nNumsChar)
	{
		free(szWText);
		szWText = NULL;
		return FALSE;
	}

	// to ansi
	nNumsChar = WideCharToMultiByte(CP_ACP, 0, szWText, -1, szAnsi, cchDest, NULL, NULL);
	free(szWText);
	szWText = NULL;

	return (0 != nNumsChar);
}

/******************************************************************************/
/**
* UTF8 <==> TEXT
*/
BOOL Utf8ToText(LPTSTR szText, int cchDest, LPCSTR szUtf8)
{
	if ((NULL == szText) || (cchDest <= 0) || (NULL == szUtf8))
	{
		return FALSE;
	}

#ifdef _UNICODE
	return (0 != MultiByteToWideChar(CP_UTF8, 0, szUtf8, -1, szText, cchDest));
#else // _UNICODE
	return Utf8ToAnsi(szText, cchDest, szUtf8);
#endif // _UNICODE
}

BOOL TextToUtf8(LPSTR szUtf8, int cchDest, LPCTSTR szText)
{
	if ((NULL == szText) || (cchDest <= 0) || (NULL == szUtf8))
	{
		return FALSE;
	}

#ifdef _UNICODE
	return (0 != WideCharToMultiByte(CP_UTF8, 0, szText, -1, szUtf8, cchDest, NULL, NULL));
#else // _UNICODE
	return AnsiToUtf8(szUtf8, cchDest, szText);
#endif // _UNICODE
}

/******************************************************************************/
/**
* UTF8 <==> UNICODE
*/
BOOL Utf8ToUnicode(LPWSTR szUnicode, int cchDest, LPCSTR szUtf8)
{
	if ((NULL == szUnicode) || (cchDest <= 0) || (NULL == szUtf8))
	{
		return FALSE;
	}

	if (0 == MultiByteToWideChar(CP_UTF8, 0, szUtf8, -1, szUnicode, cchDest))
	{
		return FALSE;
	}

	return TRUE;
}

BOOL UnicodeToUtf8(LPSTR szUtf8, int cchDest, LPWSTR szUnicode)
{
	if ((NULL == szUtf8) || (cchDest <= 0) || (NULL == szUnicode))
	{
		return FALSE;
	}

	if (0 == WideCharToMultiByte(CP_UTF8, 0, szUnicode, -1, szUtf8, cchDest, NULL, NULL))
	{
		return FALSE;
	}

	return TRUE;
}
