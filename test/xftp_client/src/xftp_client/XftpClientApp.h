/**
 * @file    XftpClientApp.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * �ļ����ƣ�XftpClientApp.h
 * �������ڣ�2019��02��16��
 * �ļ���ʶ��
 * �ļ�ժҪ��Ӧ�ó���� App �ࡣ
 * 
 * ��ǰ�汾��1.0.0.0
 * ��    �ߣ�
 * ������ڣ�2019��02��16��
 * �汾ժҪ��
 * 
 * ȡ���汾��
 * ԭ����  ��
 * ������ڣ�
 * �汾ժҪ��
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
 * @brief Ӧ�ó���� App �ࡣ
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
     * @brief UI ���߳� ID��
     */
    inline DWORD GetAppThreadID(void) const { return m_nAppThreadID; }

    /**********************************************************/
    /**
     * @brief Ӧ�ó�������·����
     */
    inline LPCTSTR GetAppWorkDir(void) const { return m_szAppWorkDir; }

    // data members
protected:
    DWORD  m_nAppThreadID;            ///< UI ���߳� ID
    TCHAR  m_szAppWorkDir[MAX_PATH];  ///< Ӧ�ó�������·��

    // message map handlers
protected:
    afx_msg void OnAppNotify(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

//====================================================================

extern CXftpClientApp theApp;

////////////////////////////////////////////////////////////////////////////////

