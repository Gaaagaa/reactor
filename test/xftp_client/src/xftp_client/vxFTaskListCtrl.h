/**
 * @file    vxFTaskListCtrl.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * �ļ����ƣ�vxFTaskListCtrl.h
 * �������ڣ�2019��03��07��
 * �ļ���ʶ��
 * �ļ�ժҪ���ļ�����������б�ؼ���
 * 
 * ��ǰ�汾��1.0.0.0
 * ��    �ߣ�
 * ������ڣ�2019��03��07��
 * �汾ժҪ��
 * 
 * ȡ���汾��
 * ԭ����  ��
 * ������ڣ�
 * �汾ժҪ��
 * </pre>
 */

#ifndef __VXFTASKLISTCTRL_H__
#define __VXFTASKLISTCTRL_H__

#include "vxListCtrl.h"
#include "xftp/xftp_download.h"

////////////////////////////////////////////////////////////////////////////////
// vxFTaskListCtrl

/**
 * @class vxFTaskListCtrl
 * @brief �ļ�����������б�ؼ���
 */
class vxFTaskListCtrl : public vxListCtrl
{
    DECLARE_DYNAMIC(vxFTaskListCtrl)

    // constructor/destructor
public:
    vxFTaskListCtrl(void);
    virtual ~vxFTaskListCtrl(void);

    // overrides
protected:
    /**********************************************************/
    /**
     * @brief DrawItemInfo() ���ù����У����ñ��ӿ�ʵ��������ڲ���������Ļ��Ʋ�����
     */
    virtual void DrawSubItem(CDC * pDC, int iItem, int iSubItem, COLORREF clrBkgnd);

    // internal invoking
protected:
    /**********************************************************/
    /**
     * @brief �����ļ���С��Ϣ��
     */
    void DrawSubItem_FileSize(CDC * pDC, int iItem, COLORREF clrBkgnd);

    /**********************************************************/
    /**
     * @brief �����ļ�����״̬��
     */
    void DrawSubItem_DlStatus(CDC * pDC, int iItem, COLORREF clrBkgnd);

    // data members
private:
    x_ftp_download_t  m_xftp_dload;  ///< ִ���ļ����ع������������ӹ�������

    // message handlers
protected:
    DECLARE_MESSAGE_MAP()

};

////////////////////////////////////////////////////////////////////////////////

#endif // __VXFTASKLISTCTRL_H__
