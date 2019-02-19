/**
* @file    vxWinHelpFunc.h
* <pre>
* Copyright (c) 2016, Gaaagaa All rights reserved.
* 
* 文件名称：vxWinHelpFunc.h
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

#ifndef __VXWINHELPFUNC_H__
#define __VXWINHELPFUNC_H__

#include <Windows.h>

////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

////////////////////////////////////////////////////////////////////////////////

/**
 * @enum  emScanResult
 * @brief ScanDirectory() 操作返回值。
 */
typedef enum emScanResult
{
	SCAN_OK     = 0,    ///< 扫描完成
	SCAN_ERR    = 1,    ///< 扫描错误
	SCAN_ABORT  = 2,    ///< 扫描终止
} emScanResult;

/******************************************************************************/
/**
* @brief ScanDirectory() 操作的结果回调处理函数。
* 
* @param [in ] szPathName : 回调扫描到的文件路径名。
* @param [in ] szFileName : 回调扫描到的文件名。
* @param [in ] dwContext  : 回调的用户数据。
* 
* @return BOOL
*         - 返回 TRUE，继续执行扫描操作；
*         - 返回 FALSE，停止扫描操作。
*/
typedef BOOL (__stdcall * FINDFILE_FNCBK )(LPCTSTR szPathName, LPCTSTR szFileName, DWORD_PTR dwContext);
typedef BOOL (__stdcall * FINDFILE_FNCBKA)(LPCSTR  szPathName, LPCSTR  szFileName, DWORD_PTR dwContext);
typedef BOOL (__stdcall * FINDFILE_FNCBKW)(LPCWSTR szPathName, LPCWSTR szFileName, DWORD_PTR dwContext);

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
int ScanDirectory (LPCTSTR szPathDir, LPCTSTR szFilter, BOOL bScanSubDir, FINDFILE_FNCBK  xFuncFName, DWORD_PTR dwContext);
int ScanDirectoryA(LPCSTR  szPathDir, LPCSTR  szFilter, BOOL bScanSubDir, FINDFILE_FNCBKA xFuncFName, DWORD_PTR dwContext);
int ScanDirectoryW(LPCWSTR szPathDir, LPCWSTR szFilter, BOOL bScanSubDir, FINDFILE_FNCBKW fncbkFName, DWORD_PTR dwContext);

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
int CleanupDir (LPCTSTR szPathDir, LPCTSTR szFilter, BOOL bScanSubDir);
int CleanupDirA(LPCSTR  szPathDir, LPCSTR  szFilter, BOOL bScanSubDir);
int CleanupDirW(LPCWSTR szPathDir, LPCWSTR szFilter, BOOL bScanSubDir);

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
BOOL MkPathDir (LPCTSTR szDir);
BOOL MkPathDirA(LPCSTR  szDir);
BOOL MkPathDirW(LPCWSTR szDir);

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
BOOL MkFilePathDir (LPCTSTR szFilePath);
BOOL MkFilePathDirA(LPCSTR  szFilePath);
BOOL MkFilePathDirW(LPCWSTR szFilePath);

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
LPCTSTR FileBaseName (LPCTSTR szFilePath);
LPCSTR  FileBaseNameA(LPCSTR  szFilePath);
LPCWSTR FileBaseNameW(LPCWSTR szFilePath);

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
LPCTSTR FileExtName (LPCTSTR szFilePath);
LPCSTR  FileExtNameA(LPCSTR  szFilePath);
LPCWSTR FileExtNameW(LPCWSTR szFilePath);

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
BOOL IsFileExist (LPCTSTR szFilePathName);
BOOL IsFileExistA(LPCSTR  szFilePathName);
BOOL IsFileExistW(LPCWSTR szFilePathName);

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
ULONG StatGetFileSize (LPCTSTR szFilePath);
ULONG StatGetFileSizeA(LPCSTR  szFilePath);
ULONG StatGetFileSizeW(LPCWSTR szFilePath);

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
int ReadFileToMem (LPCTSTR szFileName, LPBYTE lpData, DWORD dwLen);
int ReadFileToMemA(LPCSTR  szFileName, LPBYTE lpData, DWORD dwLen);
int ReadFileToMemW(LPCWSTR szFileName, LPBYTE lpData, DWORD dwLen);

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
int WriteMemToFile (LPCTSTR szFileName, LPBYTE lpData, DWORD dwLen);
int WriteMemToFileA(LPCSTR  szFileName, LPBYTE lpData, DWORD dwLen);
int WriteMemToFileW(LPCWSTR szFileName, LPBYTE lpData, DWORD dwLen);

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
int RecycleDeleteFile (LPCTSTR szFileName, BOOL bRecycle);
int RecycleDeleteFileA(LPCSTR  szFileName, BOOL bRecycle);
int RecycleDeleteFileW(LPCWSTR szFileName, BOOL bRecycle);

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
void DestroyThread(HANDLE hThread, UINT nTimeout);

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
int StartupSessionProcess (LPTSTR szAppName, LPTSTR szCmdLine);
int StartupSessionProcessA(LPSTR  szAppName, LPSTR  szCmdLine);
int StartupSessionProcessW(LPWSTR szAppName, LPWSTR szCmdLine);

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
BOOL GetCurrentUserSID (LPTSTR szDest, int cchDest);
BOOL GetCurrentUserSIDA(LPSTR  szDest, int cchDest);
BOOL GetCurrentUserSIDW(LPWSTR szDest, int cchDest);

//==============================================================================
// 字符串转换操作相关函数

/******************************************************************************/
/**
* ANSI <==> TEXT
*/
BOOL AnsiToText(LPTSTR szText, int cchDest, LPCSTR szAnsi);
BOOL TextToAnsi(LPSTR szAnsi, int cchDest, LPCTSTR szText);

/******************************************************************************/
/**
* ANSI <==> UNICODE
*/
BOOL AnsiToUnicode(LPWSTR szUnicode, int cchDest, LPCSTR szAnsi);
BOOL UnicodeToAnsi(LPSTR szAnsi, int cchDest, LPWSTR szUnicode);

/******************************************************************************/
/**
* ANSI <==> UTF8
*/
BOOL AnsiToUtf8(LPSTR szUtf8, int cchDest, LPCSTR szAnsi);
BOOL Utf8ToAnsi(LPSTR szAnsi, int cchDest, LPCSTR szUtf8);

/******************************************************************************/
/**
* UTF8 <==> TEXT
*/
BOOL Utf8ToText(LPTSTR szText, int cchDest, LPCSTR szUtf8);
BOOL TextToUtf8(LPSTR szUtf8, int cchDest, LPCTSTR szText);

/******************************************************************************/
/**
* UTF8 <==> UNICODE
*/
BOOL Utf8ToUnicode(LPWSTR szUnicode, int cchDest, LPCSTR szUtf8);
BOOL UnicodeToUtf8(LPSTR szUtf8, int cchDest, LPWSTR szUnicode);

////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
};
#endif // __cplusplus

////////////////////////////////////////////////////////////////////////////////

#endif // __VXWINHELPFUNC_H__
