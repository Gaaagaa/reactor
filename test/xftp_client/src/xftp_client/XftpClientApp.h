/**
 * @file    XftpClientApp.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：XftpClientApp.h
 * 创建日期：2019年02月16日
 * 文件标识：
 * 文件摘要：应用程序的 App 类。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年02月16日
 * 版本摘要：
 * 
 * 取代版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#pragma once

#ifndef __AFXWIN_H__
#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "vxIniFile.h"

////////////////////////////////////////////////////////////////////////////////
// CXftpClientApp:
// See xftp_client.cpp for the implementation of this class
//

/**
 * @class CXftpClientApp
 * @brief 应用程序的 App 类。
 */
class CXftpClientApp : public CWinApp, public vxIniFile
{
    // constructor/destructor
public:
	CXftpClientApp(void);

    // overrides
public:
	virtual BOOL InitInstance();

    // public interfaces
public:
    /**********************************************************/
    /**
     * @brief UI 的线程 ID。
     */
    inline DWORD GetAppThreadID(void) const { return m_nAppThreadID; }

    /**********************************************************/
    /**
     * @brief 应用程序启动路径。
     */
    inline LPCTSTR GetAppWorkDir(void) const { return m_szAppWorkDir; }

    // data members
protected:
    DWORD  m_nAppThreadID;            ///< UI 的线程 ID
    TCHAR  m_szAppWorkDir[MAX_PATH];  ///< 应用程序启动路径

    // message map handlers
protected:
    afx_msg void OnAppNotify(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

//====================================================================

extern CXftpClientApp theApp;

////////////////////////////////////////////////////////////////////////////////

