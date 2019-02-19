/**
 * @file    XftpClientApp.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：XftpClientApp.cpp
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

#include "stdafx.h"
#include "XftpClientApp.h"
#include "XftpClientDlg.h"

////////////////////////////////////////////////////////////////////////////////

const UINT WMR_APP_NOTIFY = ::RegisterWindowMessage(TEXT("{0136CB32-2B8B-42C4-B2B7-30EFD03CD959}"));

/**
 * @class vxWSASocketInit
 * @brief 自动 加载/卸载 WinSock 库的操作类。
 */
class vxWSASocketInit
{
    // constructor/destructor
public:
    vxWSASocketInit(x_int32_t xit_main_ver = 2, x_int32_t xit_sub_ver = 0)
    {
        WSAStartup(MAKEWORD(xit_main_ver, xit_sub_ver), &m_wsaData);
    }

    ~vxWSASocketInit(x_void_t)
    {
        WSACleanup();
    }

    // class data
protected:
    WSAData      m_wsaData;
};

////////////////////////////////////////////////////////////////////////////////
// CXftpClientApp

// The one and only CXftpClientApp object
CXftpClientApp theApp;

//====================================================================

// 
// CXftpClientApp : constructor/destructor
// 

CXftpClientApp::CXftpClientApp(void)
    : m_nAppThreadID(0)
{

}

//====================================================================

// 
// CXftpClientApp : overrides
// 

BOOL CXftpClientApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(TEXT("XftpClient"));

    //======================================

    // 初始化套接字库
    vxWSASocketInit xWSASocketInit;

    m_nAppThreadID = GetCurrentThreadId();
    x_msg_handler_t::instance().open(WMR_APP_NOTIFY);

    //======================================

    ::GetModuleFileName(NULL, m_szAppWorkDir, MAX_PATH);
    x_int32_t xit_len = (x_int32_t)_tcslen(m_szAppWorkDir);
    while (xit_len-- > 0)
    {
        if ((TEXT('\\') == m_szAppWorkDir[xit_len]) || (TEXT('/') == m_szAppWorkDir[xit_len]))
            break;
        else
            m_szAppWorkDir[xit_len] = TEXT('\0');
    }

    //======================================

	CXftpClientDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

    //======================================

	// Delete the shell manager created above.
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	return FALSE;
}

//====================================================================

// 
// CXftpClientApp : message map handlers
// 

BEGIN_MESSAGE_MAP(CXftpClientApp, CWinApp)
    ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
    ON_REGISTERED_THREAD_MESSAGE(WMR_APP_NOTIFY, &CXftpClientApp::OnAppNotify)
END_MESSAGE_MAP()

/**********************************************************/
/**
 * @brief WMR_APP_NOTIFY message handler.
 */
void CXftpClientApp::OnAppNotify(WPARAM wParam, LPARAM lParam)
{
    x_msg_handler_t::instance().msg_dispatch();
}


