/**
 * @file    vxFTaskListCtrl.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：vxFTaskListCtrl.cpp
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

#include "stdafx.h"
#include "vxFTaskListCtrl.h"

////////////////////////////////////////////////////////////////////////////////
// vxFTaskListCtrl

IMPLEMENT_DYNAMIC(vxFTaskListCtrl, vxListCtrl)

//====================================================================

// 
// vxFTaskListCtrl : constructor/destructor
// 

vxFTaskListCtrl::vxFTaskListCtrl(void)
{

}

vxFTaskListCtrl::~vxFTaskListCtrl(void)
{

}

//====================================================================

// 
// vxFTaskListCtrl : overrides
// 

/**********************************************************/
/**
 * @brief DrawItemInfo() 调用过程中，调用本接口实现子项的内部各个子项的绘制操作。
 */
void vxFTaskListCtrl::DrawSubItem(CDC * pDC, int iItem, int iSubItem, COLORREF clrBkgnd)
{
    switch (iSubItem)
    {
    case 1 : DrawSubItem_FileSize(pDC, iItem, clrBkgnd);              break;
    case 2 : DrawSubItem_DlStatus(pDC, iItem, clrBkgnd);              break;
    default: vxListCtrl::DrawSubItem(pDC, iItem, iSubItem, clrBkgnd); break;
    }
}

//====================================================================

// 
// vxListCtrl : internal invoking
// 

/**********************************************************/
/**
 * @brief 绘制文件大小信息。
 */
void vxFTaskListCtrl::DrawSubItem_FileSize(CDC * pDC, int iItem, COLORREF clrBkgnd)
{
    // 矩形区域
    CRect rcSubItem;
    if (!GetSubItemRect(iItem, 1, LVIR_LABEL, rcSubItem))
        return;

    // 文本
    CString strText = GetItemText(iItem, 1);
    x_int64_t xit_fsize = _tstoll(strText);
    if (xit_fsize < 1024LL)
        strText.Format(TEXT("%ld B"), xit_fsize);
    else if (xit_fsize < (1024LL * 1024LL))
        strText.Format(TEXT("%.2lf KB"), (xit_fsize / 1024.0));
    else if (xit_fsize < (1024LL * 1024LL * 1024LL))
        strText.Format(TEXT("%.2lf MB"), (xit_fsize / (1024.0 * 1024.0)));
    else
        strText.Format(TEXT("%.2lf GB"), (xit_fsize / (1024.0 * 1024.0 * 1024.0)));

    // 对齐方式
    UINT dtFmt = m_wndHeaderCtrl.GetColTextAlign(1) | DT_SINGLELINE | DT_NOPREFIX | DT_TOP | DT_END_ELLIPSIS;

    // 填充背景
    CRect rcBkgnd = rcSubItem;
    pDC->FillSolidRect(rcBkgnd, clrBkgnd);

    // 计算文本输出矩形区域
    RECT rcText = rcSubItem;
    pDC->DrawText(strText, &rcText, dtFmt | DT_CALCRECT);
    int dy = ((rcSubItem.bottom - rcSubItem.top) - (rcText.bottom - rcText.top)) / 2;
    ::OffsetRect(&rcText, 0, dy);
    rcText.left = rcSubItem.left + 3;
    rcText.right = rcSubItem.right - 3;

    // 绘制文本
    pDC->DrawText(strText, &rcText, dtFmt);
}


/**********************************************************/
/**
 * @brief 绘制文件下载状态。
 */
void vxFTaskListCtrl::DrawSubItem_DlStatus(CDC * pDC, int iItem, COLORREF clrBkgnd)
{
    // 矩形区域
    CRect rcSubItem;
    if (!GetSubItemRect(iItem, 2, LVIR_LABEL, rcSubItem))
        return;

    // 文本
    CString strText = TEXT("");
    DWORD dwStatus = (((DWORD)GetItemData(iItem)) & 0xFF000000) >> 24;
    switch (dwStatus)
    {
    case 0x00: strText = TEXT("等待"  ); break;
    case 0x01: strText = TEXT("完成"  ); break;
    case 0x02: strText = TEXT("异常"  ); break;
    case 0x10: strText = TEXT("下载中"); break;
    case 0x11: strText = TEXT("暂停中"); break;

    default:
        break;
    }

    // 对齐方式
    UINT dtFmt = m_wndHeaderCtrl.GetColTextAlign(1) | DT_SINGLELINE | DT_NOPREFIX | DT_TOP | DT_END_ELLIPSIS;

    // 填充背景
    CRect rcBkgnd = rcSubItem;
    pDC->FillSolidRect(rcBkgnd, clrBkgnd);

    // 计算文本输出矩形区域
    RECT rcText = rcSubItem;
    pDC->DrawText(strText, &rcText, dtFmt | DT_CALCRECT);
    int dy = ((rcSubItem.bottom - rcSubItem.top) - (rcText.bottom - rcText.top)) / 2;
    ::OffsetRect(&rcText, 0, dy);
    rcText.left = rcSubItem.left + 3;
    rcText.right = rcSubItem.right - 3;

    // 绘制文本
    pDC->DrawText(strText, &rcText, dtFmt);
}

//====================================================================

// 
// vxFTaskListCtrl : message handlers
// 

BEGIN_MESSAGE_MAP(vxFTaskListCtrl, vxListCtrl)
END_MESSAGE_MAP()

