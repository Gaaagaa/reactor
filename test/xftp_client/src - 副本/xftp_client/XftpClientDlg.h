/**
 * @file    XftpClientDlg.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * �ļ����ƣ�XftpClientDlg.h
 * �������ڣ�2019��02��16��
 * �ļ���ʶ��
 * �ļ�ժҪ��Ӧ�ó�������Ի����ࡣ
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

#include "vxListCtrl.h"
#include "xftp/xftp_client.h"
#include "xthreadpool.h"

////////////////////////////////////////////////////////////////////////////////
// CXftpClientDlg dialog

/**
 * @class CXftpClientDlg
 * @brief Ӧ�ó�������Ի����ࡣ
 */
class CXftpClientDlg : public CDialogEx, public x_spec_subscriber_t< CXftpClientDlg, EM_EST_CLIENT >
{
    // common data types
public:
    /**
     * @enum  emWndCtrlID
     * @brief ��ؿؼ� ID ö��ֵ��
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
     * @brief ��ʼ����Ϣ֪ͨ����ز�����
     */
    x_int32_t init_msg_handler(void);

    /**********************************************************/
    /**
     * @brief ���ã�����ʼ������Ϣ֪ͨ����ز�����
     */
    x_void_t reset_msg_handler(void);

    /**********************************************************/
    /**
     * @brief ���� ��ȡ�ļ��б� ����Ϣ��
     */
    x_void_t on_msg_flist(x_uint32_t xut_size, x_pvoid_t xpvt_dptr);

    // data members
protected:
	HICON   m_hIcon;   ///< ����ͼ��
    SIZE    m_szInit;  ///< ���ڳ�ʼ�ߴ�

    vxListCtrl      m_wndListFiles;  ///< �ļ��б�ؼ�
    vxListCtrl      m_wndListTasks;  ///< ���������б�ؼ�

    x_ftp_client_t  m_xftp_client;   ///< �ͻ��˵��������ӹ�������
    x_threadpool_t  m_xthreadpool;   ///< ����ִ���첽����������̳߳ض���

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
