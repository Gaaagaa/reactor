/*
* Copyright (c) 2012, Gaaagaa
* All rights reserved.
* 
* 文件名称：vxIniFile.h
* 创建日期：2012年12月4日
* 文件标识：
* 文件摘要：INI 文件读写类。
* 
* 当前版本：1.0.0.0
* 作    者：
* 完成日期：2012年12月4日
* 版本摘要：
* 
* 取代版本：
* 原作者  ：
* 完成日期：
* 版本摘要：
* 
*/

#ifndef __VXINIFILE_H__
#define __VXINIFILE_H__

#include <Windows.h>
#include <atlbase.h>

////////////////////////////////////////////////////////////////////////////////
// vxIniFile

class vxIniFileA
{
	// constructor/destructor
public:
	vxIniFileA(LPCSTR szFile = NULL)
	{
		memset(m_szBuffer, 0, MAX_PATH * sizeof(CHAR));
		SetIniFileName(szFile);
	}

	virtual ~vxIniFileA()
	{

	}

	// public interfaces
public:
	/******************************************************************************/
	/**
	* 设置操作的 INI 文件名。
	*/
	void SetIniFileName(LPCSTR szIniFile)
	{
		if (NULL != szIniFile)
		{
			strcpy_s(m_szIniFile, MAX_PATH, szIniFile);
		}
		else
		{
			memset(m_szIniFile, 0, MAX_PATH * sizeof(CHAR));
			::GetModuleFileNameA(NULL, m_szIniFile, MAX_PATH);
			strcpy_s(m_szIniFile + strlen(m_szIniFile) - strlen(".exe"), strlen(".exe") + 1, ".ini");
		}
	}

	/******************************************************************************/
	/**
	* 返回 操作的 INI 文件名。
	*/
	inline LPCSTR GetIniFileName() const { return m_szIniFile; }

	/******************************************************************************/
	/**
	* @brief
	* <pre>
	*   访问类型 : public 
	*   功能描述 : 根据指定的 [Section, Key] 读取整形配置参量。
	* </pre>
	* 
	* @param [in ] szSection : 配置参量的段区。
	* @param [in ] szKey     : 配置参量的索引键。
	* @param [in ] nDefValue : 默认值。
	* 
	* @return int
	*         - 返回 配置参量。
	*/
	int ReadInt(LPCSTR szSection, LPCSTR szKey, int nDefValue)
	{
		return (int)GetPrivateProfileIntA(szSection,  szKey, nDefValue, m_szIniFile);
	}

	/******************************************************************************/
	/**
	* @brief
	* <pre>
	*   访问类型 : public 
	*   功能描述 : 向指定的 [Section, Key] 写入整形配置参量。
	* </pre>
	* 
	* @param [in ] szSection : 配置参量的段区。
	* @param [in ] szKey     : 配置参量的索引键。
	* @param [in ] nValue    : 配置参量值。
	* 
	* @return void
	*         
	*/
	void WriteInt(LPCSTR szSection, LPCSTR szKey, int nValue)
	{
		CHAR szIntValue[32];
		sprintf_s(szIntValue, 32, "%d", nValue);
		WritePrivateProfileStringA(szSection, szKey, szIntValue, m_szIniFile);
	}

	/******************************************************************************/
	/**
	* @brief
	* <pre>
	*   访问类型 : public 
	*   功能描述 : 根据指定的 [Section, Key] 读取字符串配置参量（字符串长度小于 MAX_PATH）。
	* </pre>
	* 
	* @param [in ] szSection : 配置参量的段区。
	* @param [in ] szKey     : 配置参量的索引键。
	* @param [in ] szDefText : 默认值。
	* 
	* @return LPCTSTR
	*         - 返回 配置参量。
	*/
	LPCSTR ReadText(LPCSTR szSection, LPCSTR szKey, LPCSTR szDefText)
	{
		memset(m_szBuffer, 0, MAX_PATH * sizeof(CHAR));
		GetPrivateProfileStringA(szSection,  szKey, szDefText, m_szBuffer, MAX_PATH, m_szIniFile);
		m_szBuffer[MAX_PATH - 1] = TEXT('\0');
		return m_szBuffer;
	}

	/******************************************************************************/
	/**
	* @brief
	* <pre>
	*   访问类型 : public 
	*   功能描述 : 根据指定的 [Section, Key] 读取字符串配置参量（字符串长度不受限）。
	* </pre>
	* 
	* @param [in ] szSection : 配置参量的段区。
	* @param [in ] szKey     : 配置参量的索引键。
	* @param [in ] szDefText : 默认值。
	* @param [out] szRead    : 参量输出缓存。
	* @param [in ] nLen      : 参量输出缓存长度。
	* 
	* @return LPCTSTR
	*         - 返回 配置参量（szRead 缓存地址）。
	*/
	LPCSTR ReadTextEx(LPCSTR szSection, LPCSTR szKey, LPCSTR szDefText, LPSTR szRead, UINT nLen)
	{
		GetPrivateProfileStringA(szSection,  szKey, szDefText, szRead, nLen, m_szIniFile);
		return szRead;
	}

	/******************************************************************************/
	/**
	* @brief
	* <pre>
	*   访问类型 : public 
	*   功能描述 : 向指定的 [Section, Key] 写入字符串配置参量。
	* </pre>
	* 
	* @param [in ] szSection : 配置参量的段区。
	* @param [in ] szKey     : 配置参量的索引键。
	* @param [in ] szText    : 配置参量值。
	* 
	* @return void
	*         
	*/
	void WriteText(LPCSTR szSection, LPCSTR szKey, LPCSTR szText)
	{
		WritePrivateProfileStringA(szSection, szKey, szText, m_szIniFile);
	}

	// class data
protected:
	CHAR           m_szIniFile[MAX_PATH];      ///< 保存操作的 INI 文件名
	CHAR           m_szBuffer[MAX_PATH];       ///< 用于读取字符串配置参量时的字符串缓存

};

class vxIniFileW
{
	// constructor/destructor
public:
	vxIniFileW(LPCWSTR szFile = NULL)
	{
		memset(m_szBuffer, 0, MAX_PATH * sizeof(WCHAR));
		SetIniFileName(szFile);
	}

	virtual ~vxIniFileW()
	{

	}

	// public interfaces
public:
	/******************************************************************************/
	/**
	* 设置操作的 INI 文件名。
	*/
	void SetIniFileName(LPCWSTR szIniFile)
	{
		if (NULL != szIniFile)
		{
			wcscpy_s(m_szIniFile, MAX_PATH, szIniFile);
		}
		else
		{
			memset(m_szIniFile, 0, MAX_PATH * sizeof(WCHAR));
			GetModuleFileNameW(NULL, m_szIniFile, MAX_PATH);
			wcscpy_s(m_szIniFile + wcslen(m_szIniFile) - wcslen(L".exe"), wcslen(L".exe") + 1, L".ini");
		}
	}

	/******************************************************************************/
	/**
	* 返回 操作的 INI 文件名。
	*/
	inline LPCWSTR GetIniFileName() const { return m_szIniFile; }

	/******************************************************************************/
	/**
	* @brief
	* <pre>
	*   访问类型 : public 
	*   功能描述 : 根据指定的 [Section, Key] 读取整形配置参量。
	* </pre>
	* 
	* @param [in ] szSection : 配置参量的段区。
	* @param [in ] szKey     : 配置参量的索引键。
	* @param [in ] nDefValue : 默认值。
	* 
	* @return int
	*         - 返回 配置参量。
	*/
	int ReadInt(LPCWSTR szSection, LPCWSTR szKey, int nDefValue)
	{
		return (int)GetPrivateProfileIntW(szSection,  szKey, nDefValue, m_szIniFile);
	}

	/******************************************************************************/
	/**
	* @brief
	* <pre>
	*   访问类型 : public 
	*   功能描述 : 向指定的 [Section, Key] 写入整形配置参量。
	* </pre>
	* 
	* @param [in ] szSection : 配置参量的段区。
	* @param [in ] szKey     : 配置参量的索引键。
	* @param [in ] nValue    : 配置参量值。
	* 
	* @return void
	*         
	*/
	void WriteInt(LPCTSTR szSection, LPCTSTR szKey, int nValue)
	{
		WCHAR szIntValue[32];
		swprintf_s(szIntValue, 32, L"%d", nValue);
		WritePrivateProfileStringW(szSection, szKey, szIntValue, m_szIniFile);
	}

	/******************************************************************************/
	/**
	* @brief
	* <pre>
	*   访问类型 : public 
	*   功能描述 : 根据指定的 [Section, Key] 读取字符串配置参量（字符串长度小于 MAX_PATH）。
	* </pre>
	* 
	* @param [in ] szSection : 配置参量的段区。
	* @param [in ] szKey     : 配置参量的索引键。
	* @param [in ] szDefText : 默认值。
	* 
	* @return LPCTSTR
	*         - 返回 配置参量。
	*/
	LPCWSTR ReadText(LPCWSTR szSection, LPCWSTR szKey, LPCWSTR szDefText)
	{
		memset(m_szBuffer, 0, MAX_PATH * sizeof(WCHAR));
		GetPrivateProfileStringW(szSection,  szKey, szDefText, m_szBuffer, MAX_PATH, m_szIniFile);
		m_szBuffer[MAX_PATH - 1] = L'\0';
		return m_szBuffer;
	}

	/******************************************************************************/
	/**
	* @brief
	* <pre>
	*   访问类型 : public 
	*   功能描述 : 根据指定的 [Section, Key] 读取字符串配置参量（字符串长度不受限）。
	* </pre>
	* 
	* @param [in ] szSection : 配置参量的段区。
	* @param [in ] szKey     : 配置参量的索引键。
	* @param [in ] szDefText : 默认值。
	* @param [out] szRead    : 参量输出缓存。
	* @param [in ] nLen      : 参量输出缓存长度。
	* 
	* @return LPCTSTR
	*         - 返回 配置参量（szRead 缓存地址）。
	*/
	LPCWSTR ReadTextEx(LPCWSTR szSection, LPCWSTR szKey, LPWSTR szDefText, LPWSTR szRead, UINT nLen)
	{
		GetPrivateProfileStringW(szSection,  szKey, szDefText, szRead, nLen, m_szIniFile);
		return szRead;
	}

	/******************************************************************************/
	/**
	* @brief
	* <pre>
	*   访问类型 : public 
	*   功能描述 : 向指定的 [Section, Key] 写入字符串配置参量。
	* </pre>
	* 
	* @param [in ] szSection : 配置参量的段区。
	* @param [in ] szKey     : 配置参量的索引键。
	* @param [in ] szText    : 配置参量值。
	* 
	* @return void
	*         
	*/
	void WriteText(LPCWSTR szSection, LPCWSTR szKey, LPCWSTR szText)
	{
		WritePrivateProfileStringW(szSection, szKey, szText, m_szIniFile);
	}

	// class data
protected:
	WCHAR           m_szIniFile[MAX_PATH];      ///< 保存操作的 INI 文件名
	WCHAR           m_szBuffer[MAX_PATH];       ///< 用于读取字符串配置参量时的字符串缓存

};

#ifdef UNICODE
typedef vxIniFileW vxIniFile;
#else // UNICODE
typedef vxIniFileA vxIniFile;
#endif // UNICODE

////////////////////////////////////////////////////////////////////////////////

template< class _Ty = CWinApp >
class vxRegApp
{
	// constructor/destructor
public:
	vxRegApp()
	{
		InitAppWorkDir();
	}

	~vxRegApp()
	{

	}

	// public interfaces
public:
	/******************************************************************************/
	/**
	* @brief
	* <pre>
	*   访问类型 : public 
	*   功能描述 : 从注册表中读取指定 整数值。
	* </pre>
	* 
	* @param [in ] lpszPath : 注册表的路径名称。
	* @param [in ] lpszName : 注册表的值的名称。
	* @param [out] nValue   : 操作成功返回的整数值。
	* 
	* @return BOOL
	*         - 成功，返回 TRUE；
	*         - 失败，返回 FALSE。
	*/
	BOOL RegReadInt(LPCTSTR lpszPath, LPCTSTR lpszName, int & nValue)
	{
		CRegKey reg;
		HKEY hKey = static_cast< _Ty * >(this)->GetAppRegistryKey();
		if (ERROR_SUCCESS == reg.Open(hKey, lpszPath))
		{
			return (ERROR_SUCCESS == reg.QueryDWORDValue(lpszName, (DWORD&)nValue));
		}

		return FALSE;
	}

	/******************************************************************************/
	/**
	* @brief
	* <pre>
	*   访问类型 : public 
	*   功能描述 : 向注册表中写入指定 整数值。
	* </pre>
	* 
	* @param [in ] lpszPath : 注册表的路径名称。
	* @param [in ] lpszName : 注册表的值的名称。
	* @param [in ] nValue   : 要写入的整数值。
	* 
	* @return BOOL
	*         - 成功，返回 TRUE；
	*         - 失败，返回 FALSE。
	*/
	BOOL RegWriteInt(LPCTSTR lpszPath, LPCTSTR lpszName, int nValue)
	{
		CRegKey reg;
		HKEY hKey = static_cast< _Ty * >(this)->GetAppRegistryKey();
		if (ERROR_SUCCESS == reg.Create(hKey, lpszPath))
		{
			return (ERROR_SUCCESS == reg.SetDWORDValue(lpszName, (DWORD)nValue));
		}

		return FALSE;
	}

	/******************************************************************************/
	/**
	* @brief
	* <pre>
	*   访问类型 : public 
	*   功能描述 : 从注册表读取指定 字符串。
	* </pre>
	* 
	* @param [in ] lpszPath : 注册表的路径名称。
	* @param [in ] lpszName : 注册表的值的名称。
	* @param [out] strVal   : 操作成功返回的字符串。
	* 
	* @return BOOL
	*         - 成功，返回 TRUE；
	*         - 失败，返回 FALSE。
	*/
	BOOL RegReadString(LPCTSTR lpszPath, LPCTSTR lpszName, CString & strVal)
	{
		CRegKey reg;
		HKEY hKey = static_cast< _Ty * >(this)->GetAppRegistryKey();
		if (ERROR_SUCCESS == reg.Open(hKey, lpszPath))
		{
			ULONG ulLen = 0;

			LONG lRet = reg.QueryStringValue(lpszName, NULL, &ulLen);
			if (ERROR_SUCCESS == lRet)
			{
				lRet = reg.QueryStringValue(lpszName, (LPTSTR)strVal.GetBufferSetLength(ulLen + 1), &ulLen);
				strVal.ReleaseBuffer();
				if (ERROR_SUCCESS == lRet)
				{
					return TRUE;
				}
			}
		}

		return FALSE;
	}

	/******************************************************************************/
	/**
	* @brief
	* <pre>
	*   访问类型 : public 
	*   功能描述 : 向注册表写入字符串。
	* </pre>
	* 
	* @param [in ] lpszPath : 注册表的路径名称。
	* @param [in ] lpszName : 注册表的值的名称。
	* @param [in ] lpszVal  : 要写入的字符串。
	* 
	* @return BOOL
	*         - 成功，返回 TRUE；
	*         - 失败，返回 FALSE。
	*/
	BOOL RegWriteString(LPCTSTR lpszPath, LPCTSTR lpszName, LPCTSTR lpszVal)
	{
		CRegKey reg;
		HKEY hKey = static_cast< _Ty * >(this)->GetAppRegistryKey();
		if (ERROR_SUCCESS == reg.Create(hKey, lpszPath))
		{
			return (ERROR_SUCCESS == reg.SetStringValue(lpszName, lpszVal));
		}

		return FALSE;
	}

	/******************************************************************************/
	/**
	* 获取程序启动路径。
	*/
	inline LPCTSTR GetAppWorkDir() const
	{
#ifdef UNICODE
		return m_szWorkDirW;
#else // UNICODE
		return m_szWorkDirA;
#endif // UNICODE
	}

	/******************************************************************************/
	/**
	* 获取程序启动路径（按多字节字符串格式返回）。
	*/
	inline LPCSTR GetAppWorkDirA() const { return m_szWorkDirA; }

	/******************************************************************************/
	/**
	* 获取程序启动路径（按宽字节字符串格式返回）。
	*/
	inline LPCWSTR GetAppWorkDirW() const { return m_szWorkDirW; }

	// inner invoking
protected:
	/******************************************************************************/
	/**
	* 初始化程序启动路径。
	*/
	void InitAppWorkDir()
	{
		//======================================

		::GetModuleFileNameW(NULL, m_szWorkDirW, MAX_PATH);
		int len = (int)wcslen(m_szWorkDirW);
		while (len-- > 0)
		{
			if ((L'\\' == m_szWorkDirW[len]) || (L'/' == m_szWorkDirW[len]))
				break;
			else
				m_szWorkDirW[len] = L'\0';
		}

		//======================================

		::GetModuleFileNameA(NULL, m_szWorkDirA, MAX_PATH);

		len = (int)strlen(m_szWorkDirA);
		while (len-- > 0)
		{
			if (('\\' == m_szWorkDirA[len]) || ('/' == m_szWorkDirA[len]))
				break;
			else
				m_szWorkDirA[len] = '\0';
		}

		//======================================
	}

	// class data
protected:
	WCHAR    m_szWorkDirW[MAX_PATH];   ///< 程序启动路径
	CHAR     m_szWorkDirA[MAX_PATH];   ///< 程序启动路径

};

////////////////////////////////////////////////////////////////////////////////
// vxGNameLock

class vxGNameLock
{
	// constructor/destructor
public:
	vxGNameLock(BOOL bInitialOwner = FALSE, LPCTSTR szName = NULL, int * err = NULL)
	{
		m_hMutex = CreateMutex(NULL, bInitialOwner, szName);
		if (NULL != err)
		{
			*err = GetLastError();
		}
	}

	~vxGNameLock(void)
	{
		if (NULL != m_hMutex)
		{
			CloseHandle(m_hMutex);
			m_hMutex = NULL;
		}
	}

	// public interfaces
public:
	inline void Lock(void)
	{
		if (NULL != m_hMutex)
		{
			return;
		}

		DWORD dwWait = WaitForSingleObject(m_hMutex, INFINITE);
	}

	inline void UnLock(void)
	{
		if (NULL != m_hMutex)
		{
			return;
		}

		BOOL bRelease = ReleaseMutex(m_hMutex);
	}

	// class data
protected:
	HANDLE         m_hMutex;
};

////////////////////////////////////////////////////////////////////////////////


#endif // __VXINIFILE_H__
