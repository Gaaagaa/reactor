/**
 * @file    vxFTaskListCtrl.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * �ļ����ƣ�vxFTaskListCtrl.cpp
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
 * @brief DrawItemInfo() ���ù����У����ñ��ӿ�ʵ��������ڲ���������Ļ��Ʋ�����
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
 * @brief �����ļ���С��Ϣ��
 */
void vxFTaskListCtrl::DrawSubItem_FileSize(CDC * pDC, int iItem, COLORREF clrBkgnd)
{
    // ��������
    CRect rcSubItem;
    if (!GetSubItemRect(iItem, 1, LVIR_LABEL, rcSubItem))
        return;

    // �ı�
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

    // ���뷽ʽ
    UINT dtFmt = m_wndHeaderCtrl.GetColTextAlign(1) | DT_SINGLELINE | DT_NOPREFIX | DT_TOP | DT_END_ELLIPSIS;

    // ��䱳��
    CRect rcBkgnd = rcSubItem;
    pDC->FillSolidRect(rcBkgnd, clrBkgnd);

    // �����ı������������
    RECT rcText = rcSubItem;
    pDC->DrawText(strText, &rcText, dtFmt | DT_CALCRECT);
    int dy = ((rcSubItem.bottom - rcSubItem.top) - (rcText.bottom - rcText.top)) / 2;
    ::OffsetRect(&rcText, 0, dy);
    rcText.left = rcSubItem.left + 3;
    rcText.right = rcSubItem.right - 3;

    // �����ı�
    pDC->DrawText(strText, &rcText, dtFmt);
}


/**********************************************************/
/**
 * @brief �����ļ�����״̬��
 */
void vxFTaskListCtrl::DrawSubItem_DlStatus(CDC * pDC, int iItem, COLORREF clrBkgnd)
{
    // ��������
    CRect rcSubItem;
    if (!GetSubItemRect(iItem, 2, LVIR_LABEL, rcSubItem))
        return;

    // �ı�
    CString strText = TEXT("");
    DWORD dwStatus = (((DWORD)GetItemData(iItem)) & 0xFF000000) >> 24;
    switch (dwStatus)
    {
    case 0x00: strText = TEXT("�ȴ�"  ); break;
    case 0x01: strText = TEXT("���"  ); break;
    case 0x02: strText = TEXT("�쳣"  ); break;
    case 0x10: strText = TEXT("������"); break;
    case 0x11: strText = TEXT("��ͣ��"); break;

    default:
        break;
    }

    // ���뷽ʽ
    UINT dtFmt = m_wndHeaderCtrl.GetColTextAlign(1) | DT_SINGLELINE | DT_NOPREFIX | DT_TOP | DT_END_ELLIPSIS;

    // ��䱳��
    CRect rcBkgnd = rcSubItem;
    pDC->FillSolidRect(rcBkgnd, clrBkgnd);

    // �����ı������������
    RECT rcText = rcSubItem;
    pDC->DrawText(strText, &rcText, dtFmt | DT_CALCRECT);
    int dy = ((rcSubItem.bottom - rcSubItem.top) - (rcText.bottom - rcText.top)) / 2;
    ::OffsetRect(&rcText, 0, dy);
    rcText.left = rcSubItem.left + 3;
    rcText.right = rcSubItem.right - 3;

    // �����ı�
    pDC->DrawText(strText, &rcText, dtFmt);
}

//====================================================================

// 
// vxFTaskListCtrl : message handlers
// 

BEGIN_MESSAGE_MAP(vxFTaskListCtrl, vxListCtrl)
END_MESSAGE_MAP()

