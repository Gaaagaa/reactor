/**
 * @file    XftpClientDlg.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：XftpClientDlg.h
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

#pragma once

#include "vxListCtrl.h"
#include "xftp/xftp_client.h"
#include "xthreadpool.h"

////////////////////////////////////////////////////////////////////////////////
// CXftpClientDlg dialog

/**
 * @class CXftpClientDlg
 * @brief 应用程序的主对话框类。
 */
class CXftpClientDlg : public CDialogEx, public x_spec_subscriber_t< CXftpClientDlg, EM_EST_CLIENT >
{
    // common data types
public:
    /**
     * @enum  emWndCtrlID
     * @brief 相关控件 ID 枚举值。
     */
    typedef enum emWndCtrlID
    {
        IDD = IDD_XFTP_CLIENT_DIALOG,
    } emWndCtrlID;

    // constructor/destructor
public:
	CXftpClientDlg(CWnd* pParent = nullptr);

    // overrides
protected:
	virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void OnCancel();

    // msg handlers
protected:
    /**********************************************************/
    /**
     * @brief 初始化消息通知的相关操作。
     */
    x_int32_t init_msg_handler(void);

    /**********************************************************/
    /**
     * @brief 重置（反初始化）消息通知的相关操作。
     */
    x_void_t reset_msg_handler(void);

    /**********************************************************/
    /**
     * @brief 处理 获取文件列表 的消息。
     */
    x_void_t on_msg_flist(x_uint32_t xut_size, x_pvoid_t xpvt_dptr);

    // data members
protected:
	HICON   m_hIcon;   ///< 窗口图标
    SIZE    m_szInit;  ///< 窗口初始尺寸

    vxListCtrl      m_wndListFiles;  ///< 文件列表控件
    vxListCtrl      m_wndListTasks;  ///< 下载任务列表控件

    x_ftp_client_t  m_xftp_client;   ///< 客户端的网络连接工作对象
    x_threadpool_t  m_xthreadpool;   ///< 用于执行异步操作任务的线程池对象

	// message map handlers
protected:
    afx_msg void OnDestroy();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
    afx_msg LRESULT OnKickIdle(WPARAM wParam, LPARAM lParam);
    afx_msg void OnBnClickedBtnSvrcnt();
    afx_msg void OnBnClickedBtnSvrdown();
    afx_msg void OnBnClickedBtnFlist();
    afx_msg void OnBnClickedBtnEnload();
    afx_msg void OnBnClickedBtnDownpause();
    afx_msg void OnBnClickedBtnDownstop();
    afx_msg void OnBnClickedBtnLocalpath();
	DECLARE_MESSAGE_MAP()

 };

////////////////////////////////////////////////////////////////////////////////
