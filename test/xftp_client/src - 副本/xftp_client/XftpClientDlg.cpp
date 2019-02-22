/**
 * @file    XftpClientDlg.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：XftpClientDlg.cpp
 * 创建日期：2019年02月16日
 * 文件标识：
 * 文件摘要：应用程序的主对话框类。
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
#include "afxdialogex.h"

////////////////////////////////////////////////////////////////////////////////
// CXftpClientDlg dialog

//====================================================================

// 
// CXftpClientDlg : constructor/destructor
// 

CXftpClientDlg::CXftpClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_XFTP_CLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

//====================================================================

// 
// CXftpClientDlg : overrides
// 

void CXftpClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_FILES, m_wndListFiles);
    DDX_Control(pDX, IDC_LIST_DLTASK, m_wndListTasks);
}

BOOL CXftpClientDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon

    //======================================

    CString strText;

    //======================================

    RECT rcWnd;
    GetWindowRect(&rcWnd);
    m_szInit.cx = rcWnd.right - rcWnd.left;
    m_szInit.cy = rcWnd.bottom - rcWnd.top;

    //======================================

    int nCol = 0;

    m_wndListFiles.SetExtendedStyle(m_wndListFiles.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
    m_wndListFiles.InsertColumn(nCol++, TEXT(""), LVCFMT_CENTER, 100);
    m_wndListFiles.InsertColumn(nCol++, TEXT("文件名称"), LVCFMT_LEFT  , 120);
    m_wndListFiles.InsertColumn(nCol++, TEXT("文件大小"), LVCFMT_CENTER, 80 );
    m_wndListFiles.DeleteColumn(0);
    m_wndListFiles.SetRowHeight(24);
    m_wndListFiles.EnableSortItem(FALSE);
    m_wndListFiles.EnableLayoutColumn(TRUE);

    nCol = 0;

    m_wndListTasks.SetExtendedStyle(m_wndListTasks.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_SINGLEROW);
    m_wndListTasks.InsertColumn(nCol++, TEXT(""), LVCFMT_CENTER, 100);
    m_wndListTasks.InsertColumn(nCol++, TEXT("文件名称"), LVCFMT_LEFT  , 120);
    m_wndListTasks.InsertColumn(nCol++, TEXT("文件大小"), LVCFMT_CENTER, 80 );
    m_wndListTasks.InsertColumn(nCol++, TEXT("下载状态"), LVCFMT_CENTER, 100);
    m_wndListTasks.InsertColumn(nCol++, TEXT("传输速度"), LVCFMT_CENTER, 100);
    m_wndListTasks.DeleteColumn(0);
    m_wndListTasks.SetRowHeight(24);
    m_wndListTasks.EnableSortItem(FALSE);
    m_wndListTasks.EnableLayoutColumn(TRUE);

    //======================================
    // initial

    strText = theApp.ReadText(TEXT("SVRCFG"), TEXT("HOST"), TEXT("127.0.0.1"));
    GetDlgItem(IDC_IPA_IPHOST)->SetWindowText(strText);

    strText = theApp.ReadText(TEXT("SVRCFG"), TEXT("PORT"), TEXT("10086"));
    GetDlgItem(IDC_EDIT_PORT)->SetWindowText(strText);

    CString strPath = theApp.GetAppWorkDir();
    strPath += TEXT("FileCache\\");

    strText = theApp.ReadText(TEXT("SYSCFG"), TEXT("PATH"), (LPCTSTR)strPath);
    GetDlgItem(IDC_EDIT_LOCALPATH)->SetWindowText(strText);
    if (!strText.IsEmpty())
    {
        MkPathDir(strText);
    }

    //======================================

    init_msg_handler();

    //======================================

    return TRUE;
}

void CXftpClientDlg::OnOK()
{

}

void CXftpClientDlg::OnCancel()
{
    if (m_xftp_client.is_startup())
    {
        if (IDOK != ::MessageBox(m_hWnd,
                                 TEXT("网络连接尚未断开，确定现在要退出程序吗？"),
                                 TEXT("系统提示"),
                                 MB_OKCANCEL | MB_ICONINFORMATION))
        {
            return;
        }
    }

    CDialogEx::OnCancel();
}

//====================================================================

// 
// CXftpClientDlg : msg handlers
// 

/**********************************************************/
/**
 * @brief 初始化消息通知的相关操作。
 */
x_int32_t CXftpClientDlg::init_msg_handler(void)
{
    XVERIFY(__subscriber::register_mkey(XMKEY_WCLI_FLIST, &CXftpClientDlg::on_msg_flist));
    XVERIFY(__subscriber::jointo_dispatch(this));
    XVERIFY(__subscriber::register_msg_diapatch());

    return 0;
}

/**********************************************************/
/**
 * @brief 重置（反初始化）消息通知的相关操作。
 */
x_void_t CXftpClientDlg::reset_msg_handler(void)
{
    __subscriber::reset_dispatch();
    __subscriber::unregister_msg_diapatch();
}

/**********************************************************/
/**
 * @brief 处理 获取文件列表 的消息。
 */
x_void_t CXftpClientDlg::on_msg_flist(x_uint32_t xut_size, x_pvoid_t xpvt_dptr)
{
    m_wndListFiles.DeleteAllItems();

    x_ftp_client_t::x_list_file_t xlst_files;
    m_xftp_client.get_cache_flist(xlst_files);

    TCHAR szText[TEXT_LEN_256] = { 0 };
    CString strText;

    for (x_ftp_client_t::x_list_file_t::iterator itlst = xlst_files.begin();
         itlst != xlst_files.end();
         ++itlst)
    {
        if (itlst->first.empty())
        {
            continue;
        }

        szText[0] = TEXT('\0');
        Utf8ToText(szText, TEXT_LEN_256, itlst->first.c_str());

        x_int32_t xit_item = m_wndListFiles.InsertItem(m_wndListFiles.GetItemCount(), szText);
        m_wndListFiles.SetItemText(xit_item, 0, szText);
        
        strText.Format(TEXT("%d"), itlst->second);
        m_wndListFiles.SetItemText(xit_item, 1, strText);
    }
}

//====================================================================

// 
// CXftpClientDlg : message map handlers
// 

BEGIN_MESSAGE_MAP(CXftpClientDlg, CDialogEx)
    ON_WM_DESTROY()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_GETMINMAXINFO()
    ON_MESSAGE(WM_KICKIDLE, &CXftpClientDlg::OnKickIdle)
    ON_BN_CLICKED(IDC_BTN_SVRCNT, &CXftpClientDlg::OnBnClickedBtnSvrcnt)
    ON_BN_CLICKED(IDC_BTN_SVRDOWN, &CXftpClientDlg::OnBnClickedBtnSvrdown)
    ON_BN_CLICKED(IDC_BTN_FLIST, &CXftpClientDlg::OnBnClickedBtnFlist)
    ON_BN_CLICKED(IDC_BTN_ENLOAD, &CXftpClientDlg::OnBnClickedBtnEnload)
    ON_BN_CLICKED(IDC_BTN_DOWNPAUSE, &CXftpClientDlg::OnBnClickedBtnDownpause)
    ON_BN_CLICKED(IDC_BTN_DOWNSTOP, &CXftpClientDlg::OnBnClickedBtnDownstop)
    ON_BN_CLICKED(IDC_BTN_LOCALPATH, &CXftpClientDlg::OnBnClickedBtnLocalpath)
END_MESSAGE_MAP()

/**********************************************************/
/**
 * @brief WM_DESTROY message handler.
 */
void CXftpClientDlg::OnDestroy()
{
    //======================================

    reset_msg_handler();

    //======================================

    CString strText;

    GetDlgItem(IDC_IPA_IPHOST)->GetWindowText(strText);
    theApp.WriteText(TEXT("SVRCFG"), TEXT("HOST"), strText);

    GetDlgItem(IDC_EDIT_PORT)->GetWindowText(strText);
    theApp.WriteText(TEXT("SVRCFG"), TEXT("PORT"), strText);

    GetDlgItem(IDC_EDIT_LOCALPATH)->GetWindowText(strText);
    theApp.WriteText(TEXT("SYSCFG"), TEXT("PATH"), strText);

    //======================================

    if (m_xftp_client.is_startup())
    {
        m_xftp_client.shutdown();
    }

    if (m_xthreadpool.is_startup())
    {
        m_xthreadpool.shutdown();
    }

    //======================================

    CDialogEx::OnDestroy();
}

/**********************************************************/
/**
 * @brief WM_PAINT message handler.
 */
void CXftpClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width()  - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

/**********************************************************/
/**
 * @brief WM_QUERYDRAGICON message handler.
 */
HCURSOR CXftpClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/**********************************************************/
/**
 * @brief WM_GETMINMAXINFO message handler.
 */
void CXftpClientDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
    CDialogEx::OnGetMinMaxInfo(lpMMI);

    lpMMI->ptMinTrackSize.x = m_szInit.cx;
    lpMMI->ptMinTrackSize.y = m_szInit.cy;
}

/**********************************************************/
/**
 * @brief WM_KICKIDLE message handler.
 */
LRESULT CXftpClientDlg::OnKickIdle(WPARAM wParam, LPARAM lParam)
{
    BOOL bWork  = m_xftp_client.is_startup();
    BOOL bLogin = m_xftp_client.is_login();

    BOOL bSelItemF = (m_wndListFiles.GetSelectedCount() > 0) && bLogin;
    BOOL bSelItemT = (m_wndListTasks.GetSelectedCount() > 0) && bLogin;

    GetDlgItem(IDC_IPA_IPHOST )->EnableWindow(!bWork);
    GetDlgItem(IDC_EDIT_PORT  )->EnableWindow(!bWork);
    GetDlgItem(IDC_BTN_SVRCNT )->EnableWindow(!bWork);
    GetDlgItem(IDC_BTN_SVRDOWN)->EnableWindow(bWork);

    GetDlgItem(IDC_BTN_FLIST    )->EnableWindow(bLogin);
    GetDlgItem(IDC_BTN_ENLOAD   )->EnableWindow(bSelItemF);
    GetDlgItem(IDC_BTN_DOWNPAUSE)->EnableWindow(bSelItemT);
    GetDlgItem(IDC_BTN_DOWNSTOP )->EnableWindow(bSelItemT);
    GetDlgItem(IDC_BTN_LOCALPATH)->EnableWindow(!bLogin);

    return 0;
}

/**********************************************************/
/**
 * @brief WM_COMMAND[IDC_BTN_SVRCNT] message handler.
 */
void CXftpClientDlg::OnBnClickedBtnSvrcnt()
{
    CString   strText;
    x_int32_t xit_error = -1;

    //======================================

    GetDlgItem(IDC_EDIT_LOCALPATH)->GetWindowText(strText);
    if (strText.IsEmpty() || !PathIsDirectory(strText))
    {
        ::MessageBox(m_hWnd,
                     TEXT("本地文件存储目录设置不正确，请先设置好文件下载所存储的目录！"),
                     TEXT("系统提示"),
                     MB_ICONINFORMATION | MB_OK);
        GetDlgItem(IDC_EDIT_LOCALPATH)->SetFocus();
        return;
    }

    //======================================

    if (m_xftp_client.is_startup())
    {
        return;
    }

    // IP
    GetDlgItem(IDC_IPA_IPHOST)->GetWindowText(strText);
    x_char_t xszt_host[TEXT_LEN_64] = { 0 };
    TextToAnsi(xszt_host, TEXT_LEN_64, (LPCTSTR)strText);

    // PORT
    GetDlgItem(IDC_EDIT_PORT)->GetWindowText(strText);
    x_uint16_t xwt_port = (x_uint16_t)_tstoi(strText);

     // 发起网络连接
     xit_error = m_xftp_client.startup(xszt_host, xwt_port);
     if (0 != xit_error)
     {
         ::MessageBox(m_hWnd, TEXT("连接服务终端失败！"), TEXT("系统提示"), MB_ICONINFORMATION | MB_OK);
         return;
     }

     //======================================
}

/**********************************************************/
/**
 * @brief WM_COMMAND[IDC_BTN_SVRDOWN] message handler.
 */
void CXftpClientDlg::OnBnClickedBtnSvrdown()
{
    m_xftp_client.shutdown();
}

/**********************************************************/
/**
 * @brief WM_COMMAND[BN_CLICKED, IDC_BTN_FLIST] message handler.
 */
void CXftpClientDlg::OnBnClickedBtnFlist()
{
    if (!m_xftp_client.is_startup())
    {
        return;
    }

    m_wndListFiles.DeleteAllItems();

    x_io_msgctxt_t xio_msgctxt;
    xio_msgctxt.io_cmid = x_ftp_client_t::CMID_WCLI_FLIST;
    xio_msgctxt.io_size = 0;
    xio_msgctxt.io_dptr = X_NULL;
    m_xftp_client.post_req_xmsg(xio_msgctxt, X_TRUE);
}

/**********************************************************/
/**
 * @brief WM_COMMAND[BN_CLICKED, IDC_BTN_ENLOAD] message handler.
 */
void CXftpClientDlg::OnBnClickedBtnEnload()
{

}

/**********************************************************/
/**
 * @brief WM_COMMAND[BN_CLICKED, IDC_BTN_DOWNPAUSE] message handler.
 */
void CXftpClientDlg::OnBnClickedBtnDownpause()
{
    if (!m_xftp_client.is_startup())
    {
        return;
    }

}

/**********************************************************/
/**
 * @brief WM_COMMAND[BN_CLICKED, IDC_BTN_DOWNSTOP] message handler.
 */
void CXftpClientDlg::OnBnClickedBtnDownstop()
{
    if (!m_xftp_client.is_startup())
    {
        return;
    }

}

/**********************************************************/
/**
 * @brief WM_COMMAND[BN_CLICKED, IDC_BTN_LOCALPATH] message handler.
 */
void CXftpClientDlg::OnBnClickedBtnLocalpath()
{
    TCHAR szPath[MAX_PATH] = { 0 };

    BROWSEINFO xBInfo;
    xBInfo.hwndOwner      = m_hWnd;
    xBInfo.pidlRoot       = NULL;
    xBInfo.pszDisplayName = szPath;
    xBInfo.lpszTitle      = TEXT("请选择存储目录：");
    xBInfo.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_USENEWUI;
    xBInfo.lpfn           = NULL;
    xBInfo.lParam         = 0;
    xBInfo.iImage         = 0;

    //弹出选择目录对话框
    LPITEMIDLIST lpItemIdList = SHBrowseForFolder(&xBInfo);

    if ((NULL != lpItemIdList) && SHGetPathFromIDList(lpItemIdList, szPath))
    {
        x_int32_t xit_len = (x_int32_t)_tcslen(szPath);
        if ((xit_len > 0) && (xit_len < MAX_PATH))
        {
            if ((TEXT('\\') != szPath[xit_len - 1]) || (TEXT('/') != szPath[xit_len - 1]))
                szPath[xit_len] = TEXT('\\');
            GetDlgItem(IDC_EDIT_LOCALPATH)->SetWindowText(szPath);
        }

        CoTaskMemFree(lpItemIdList);
        lpItemIdList = NULL;
    }
}


