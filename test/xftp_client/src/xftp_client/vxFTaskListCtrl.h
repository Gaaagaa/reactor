/**
 * @file    vxFTaskListCtrl.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：vxFTaskListCtrl.h
 * 创建日期：2019年03月07日
 * 文件标识：
 * 文件摘要：文件下载任务的列表控件。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年03月07日
 * 版本摘要：
 * 
 * 取代版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
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
 * @brief 文件下载任务的列表控件。
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
     * @brief DrawItemInfo() 调用过程中，调用本接口实现子项的内部各个子项的绘制操作。
     */
    virtual void DrawSubItem(CDC * pDC, int iItem, int iSubItem, COLORREF clrBkgnd);

    // internal invoking
protected:
    /**********************************************************/
    /**
     * @brief 绘制文件大小信息。
     */
    void DrawSubItem_FileSize(CDC * pDC, int iItem, COLORREF clrBkgnd);

    /**********************************************************/
    /**
     * @brief 绘制文件下载状态。
     */
    void DrawSubItem_DlStatus(CDC * pDC, int iItem, COLORREF clrBkgnd);

    // data members
private:
    x_ftp_download_t  m_xftp_dload;  ///< 执行文件下载工作的网络连接工作对象

    // message handlers
protected:
    DECLARE_MESSAGE_MAP()

};

////////////////////////////////////////////////////////////////////////////////

#endif // __VXFTASKLISTCTRL_H__
