/*
* Copyright (c) 2013, 百年千岁
* All rights reserved.
* 
* 文件名称：vxListCtrl.cpp
* 文件标识：
* 摘    要：列表控件（主要针对报表样式的风格）。
* 
* 当前版本：1.0.0.0
* 作    者：
* 完成日期：2013年4月22日
* 版本摘要：
* 
* 取代版本：
* 原作者  ：
* 完成日期：
* 版本摘要：
* 
*/

#include "stdafx.h"
#include "vxListCtrl.h"

#include <numeric>

////////////////////////////////////////////////////////////////////////////////
// vxListCtrl

IMPLEMENT_DYNAMIC(vxListCtrl, CListCtrl)

//==============================================================================

// 
// vxListCtrl : common invoking
// 

/******************************************************************************/
/**
* 默认的排序比较函数。
*/
int CALLBACK vxListCtrl::CompareEntry(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	SortParam  * sort_ptr = (SortParam  *)lParamSort;
	vxListCtrl * this_ptr = (vxListCtrl *)sort_ptr->SP_ListCtrl;

	return this_ptr->vxOnSortCompareItem(lParam1, lParam2,
							sort_ptr->SP_Colume, sort_ptr->SP_Asc, sort_ptr->SP_Reserved);
}

//==============================================================================

// 
// constructor/destructor
// 

vxListCtrl::vxListCtrl(void)
			: m_bInitial(FALSE)
			, m_nRowHeight(24)
			, m_bEnablePaintProc(TRUE)
			, m_bEnableSortItem(FALSE)
			, m_bEnableLayoutColumn(FALSE)
			, m_cxLayout(0)
{

}

vxListCtrl::~vxListCtrl(void)
{

}

//==============================================================================

// 
// vxListCtrl : overrides
// 

void vxListCtrl::PreSubclassWindow()
{
	InitListCtrl();
}

void vxListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (GetExtendedStyle() & LVS_EX_CHECKBOXES)
	{
		// 绘制子项勾选框
		DrawItemCheckBox(lpDrawItemStruct);
	}
	// 绘制子项图标
	DrawItemIcon(lpDrawItemStruct);
	// 绘制子项文本数据
	DrawItemInfo(lpDrawItemStruct);
}

//==============================================================================

// 
// vxListCtrl : extensible
// 

/******************************************************************************/
/**
* 控件初始化。
*/
BOOL vxListCtrl::InitListCtrl()
{
	if (!m_bInitial)
	{
		ModifyStyle(0, WS_CLIPCHILDREN | LVS_OWNERDRAWFIXED);
		BOOL bRet = m_wndHeaderCtrl.SubclassWindow(GetHeaderCtrl()->GetSafeHwnd());

		SetBkColor(LCC_BKGND);
		SetTextColor(LCC_TEXT);
		SetRowHeight(m_nRowHeight, TRUE);

		m_bInitial = bRet;
	}

	return m_bInitial;
}

/******************************************************************************/
/**
* 绘制子项勾选框。
*/
void vxListCtrl::DrawItemCheckBox(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	int iItem = lpDrawItemStruct->itemID;
	CRect rcItem = lpDrawItemStruct->rcItem;
	CDC *pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

	CRect rcBox;     // 勾选框矩形区域
	if (!GetItemCheckBoxRect(iItem, rcBox))
		return;

	// 填充背景
	CRect rcBkgnd = rcBox;
	int nInf = (rcItem.Height() - rcBox.Height()) / 2;
	rcBkgnd.InflateRect(nInf, nInf);
	if (!::IsWindowEnabled(m_hWnd))
		pDC->FillSolidRect(rcBkgnd, LCC_DISABLED);
	else if (lpDrawItemStruct->itemState & ODS_SELECTED)
		pDC->FillSolidRect(rcBkgnd, LCC_LITEM_SEL);
	else
		pDC->FillSolidRect(rcBkgnd,
		(lpDrawItemStruct->itemID % 2 == 0) ?
		LCC_LITEM_EVEN : LCC_LITEM_ODD);

	// 绘制勾选框
	pDC->DrawFrameControl(rcBox, DFC_BUTTON, DFCS_BUTTONCHECK | (GetCheck(iItem) ? DFCS_CHECKED : 0));
}

/******************************************************************************/
/**
* 绘制子项图标。
*/
void vxListCtrl::DrawItemIcon(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	int iItem = lpDrawItemStruct->itemID;
	CRect rcItem = lpDrawItemStruct->rcItem;
	CDC *pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
}

/******************************************************************************/
/**
* 绘制子项中的信息。
*/
void vxListCtrl::DrawItemInfo(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	int iItem = lpDrawItemStruct->itemID;
	CRect rcItem = lpDrawItemStruct->rcItem;
	CDC *pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

	// 选择背景填充颜色，并设置文本颜色
	COLORREF clrBkgnd = 0;
	if (IsWindowEnabled())
	{
		if (lpDrawItemStruct->itemState & ODS_SELECTED)
			clrBkgnd = LCC_LITEM_SEL;
		else
			clrBkgnd = (lpDrawItemStruct->itemID % 2 == 0) ? LCC_LITEM_EVEN : LCC_LITEM_ODD;
	}
	else
	{
		clrBkgnd = LCC_DISABLED;
	}

	COLORREF clrTextSwap = pDC->SetTextColor(LCC_TEXT);
	HGDIOBJ  hFontSwap   = pDC->SelectObject(::GetStockObject(DEFAULT_GUI_FONT));
	int      nBkModeSwap = pDC->SetBkMode(TRANSPARENT);

	for (int iSubItem = 0; iSubItem < m_wndHeaderCtrl.GetItemCount(); ++iSubItem)
	{
		DrawSubItem(pDC, iItem, iSubItem, clrBkgnd);
	}

#if 0
	// 扩展剩余部分的背景填充
	CRect rcReItem;
	GetClientRect(rcReItem);
	rcReItem.top    = rcItem.top;
	rcReItem.left   = rcItem.right;
	rcReItem.bottom = rcItem.bottom;
	pDC->FillSolidRect(rcReItem, clrBkgnd);
#endif

	pDC->SetBkMode(nBkModeSwap);
	pDC->SelectObject(hFontSwap);
	pDC->SetTextColor(clrTextSwap);
}

/******************************************************************************/
/**
* DrawItemInfo() 调用过程中，调用本接口实现子项的内部各个子项的绘制操作。
*/
void vxListCtrl::DrawSubItem(CDC * pDC, int iItem, int iSubItem, COLORREF clrBkgnd)
{
	// 矩形区域
	CRect rcSubItem;
	if (!GetSubItemRect(iItem, iSubItem, LVIR_LABEL, rcSubItem))
		return;

	// 文本
	CString strText = GetItemText(iItem, iSubItem);
	// 对齐方式
	UINT dtFmt = m_wndHeaderCtrl.GetColTextAlign(iSubItem) | DT_SINGLELINE | DT_NOPREFIX | DT_TOP | DT_END_ELLIPSIS;

	// 填充背景
	CRect rcBkgnd = rcSubItem;
	if (0 == iSubItem)
		rcBkgnd.left -= 2;
	pDC->FillSolidRect(rcBkgnd, clrBkgnd);

	// 计算文本输出矩形区域
	RECT rcText = rcSubItem;
	pDC->DrawText(strText, &rcText, dtFmt | DT_CALCRECT);
	int dy = ((rcSubItem.bottom - rcSubItem.top) - (rcText.bottom - rcText.top)) / 2;
	::OffsetRect(&rcText, 0, dy);
	rcText.left  = rcSubItem.left  + 3;
	rcText.right = rcSubItem.right - 3;

	// 绘制文本
	pDC->DrawText(strText, &rcText, dtFmt);
}

/******************************************************************************/
/**
* @brief
* <pre>
*   访问类型 : virtual protected 
*   功能描述 : 鼠标点击列表头触发排序操作时，SortItems() 接口调用的项比对回调函数。
* </pre>
* 
* @param [in ] lParam1   : 比对操作的项参数。
* @param [in ] lParam2   : 比对操作的项参数。
* @param [in ] nColume   : 列索引号。
* @param [in ] bAsc      : TRUE，升序；FALSE，降序。
* @param [in ] lReserved : 保留参数。
* 
* @return int
*         - 比对差值。
*/
int vxListCtrl::vxOnSortCompareItem(LPARAM lParam1, LPARAM lParam2,
									int nColume, BOOL bAsc, LPARAM lReserved)
{
	return (int)(lParam1 - lParam2);
}

//==============================================================================

// 
// vxListCtrl : public interfaces
// 

/******************************************************************************/
/**
* 设置行高。
*/
BOOL vxListCtrl::SetRowHeight(int nRowHeight, BOOL bRelayout /*= TRUE*/)
{
	m_nRowHeight = nRowHeight;

	if (bRelayout)
	{
		CRect rectWnd;
		GetWindowRect(rectWnd);

		WINDOWPOS wndpos;
		wndpos.hwnd  = m_hWnd;
		wndpos.cx    = rectWnd.Width();
		wndpos.cy    = rectWnd.Height();
		wndpos.flags = SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER;

		SendMessage(WM_WINDOWPOSCHANGED, 0, (LPARAM)&wndpos);
	}

	return TRUE;
}

/******************************************************************************/
/**
* 设置表头高度最小值。
*/
BOOL vxListCtrl::SetHeadMinHeight(int nMinHeight)
{
	m_wndHeaderCtrl.SetMinHeight(nMinHeight);
	if (::IsWindow(m_wndHeaderCtrl.m_hWnd))
	{
		CRect rectWnd;
		GetWindowRect(rectWnd);

		WINDOWPOS wndpos;
		wndpos.hwnd  = m_hWnd;
		wndpos.cx    = rectWnd.Width();
		wndpos.cy    = rectWnd.Height();
		wndpos.flags = SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER;

		SendMessage(WM_WINDOWPOSCHANGED, 0, (LPARAM)&wndpos);
	}

	return TRUE;
}

/******************************************************************************/
/**
* 获取子项的勾选框矩形区域；需保证控件具备 LVS_EX_CHECKBOXES 样式。
*/
BOOL vxListCtrl::GetItemCheckBoxRect(int iItem, LPRECT lpRect)
{
	ASSERT(NULL != lpRect);

	if (!(GetExtendedStyle() & LVS_EX_CHECKBOXES))
		return FALSE;

	CRect rcItem;
	if (!GetItemRect(iItem, rcItem, LVIR_BOUNDS))
		return FALSE;

	const int nCheckBoxSize = 14;
	int offset = (rcItem.Height() - nCheckBoxSize) / 2;

	lpRect->left   = rcItem.left  + 5;
	lpRect->top    = rcItem.top   + offset;
	lpRect->right  = lpRect->left + nCheckBoxSize;
	lpRect->bottom = lpRect->top  + nCheckBoxSize;

	return TRUE;
}

/******************************************************************************/
/**
* 启用/禁用 列表控件表头自动布局功能。
*/
void vxListCtrl::EnableLayoutColumn(BOOL bEnable /*= TRUE*/)
{
	m_bEnableLayoutColumn = bEnable;
	if (!m_bEnableLayoutColumn)
	{
		return;
	}

	m_vecHeadColWidth.clear();
	for (int i = 0; i < m_wndHeaderCtrl.GetItemCount(); ++i)
	{
		m_vecHeadColWidth.push_back(GetColumnWidth(i));
	}

	m_cxLayout = 0;
}

/******************************************************************************/
/**
* 获取键值。
*/
HANDLE vxListCtrl::GetKey(LPCTSTR szKeyName, HANDLE hKeyDefualt /*= INVALID_HANDLE_VALUE*/) const
{
	CString strKey = szKeyName;
	std::map< CString, HANDLE >::const_iterator itfind = m_mapKey.find(strKey);
	if (itfind != m_mapKey.end())
	{
		return itfind->second;
	}

	return hKeyDefualt; // GetProp(m_hWnd, szKeyName);
}

/******************************************************************************/
/**
* 设置键值。
*/
void vxListCtrl::SetKey(LPCTSTR szKeyName, HANDLE hKey)
{
	CString strKey = szKeyName;
	m_mapKey[strKey] = hKey;
}

/******************************************************************************/
/**
* 重置（清空）所有键值映射。
*/
void vxListCtrl::ResetKeyMap()
{
	m_mapKey.clear();
}

//==============================================================================

// 
// vxListCtrl : inner invoking
// 

/******************************************************************************/
/**
* @brief
* <pre>
*   访问类型 : protected 
*   功能描述 : 执行自动布局表头宽度。
* </pre>
* 
* @param [in ] cx : 当前窗口客户区宽度。
* 
* @return void
*         
*/
void vxListCtrl::AutoLayoutColumn(int cx)
{
	//======================================
	// 参数有效检查
	if (!(GetStyle() & LVS_REPORT)
		|| !IsEnableLayoutColumn()
		|| m_vecHeadColWidth.empty()
		|| (m_vecHeadColWidth.size() != (size_t)m_wndHeaderCtrl.GetItemCount()))
	{
		return;
	}

	//======================================

	int cxLayout = cx - 4;
	if ((cxLayout <= 0) || (m_cxLayout == cxLayout))
	{
		return;
	}

	//======================================

	int cxColSum = std::accumulate(m_vecHeadColWidth.begin(), m_vecHeadColWidth.end(), 0);
	if ((cxColSum <= 0) /*|| (cxLayout < cxColSum)*/)
	{
		return;
	}

	EnablePaintProc(FALSE);

	double dbMul = cxLayout * 1.0 / cxColSum;
	for (int i = 0; i < m_wndHeaderCtrl.GetItemCount(); ++i)
	{
		int nWidth = (int)(dbMul * m_vecHeadColWidth[i]);
		SetColumnWidth(i, nWidth);
	}

	m_cxLayout = cxLayout;

	EnablePaintProc(TRUE);
	RedrawWindow();
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

//==============================================================================

// 
// vxListCtrl message handlers
// 

BEGIN_MESSAGE_MAP(vxListCtrl, CListCtrl)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_MEASUREITEM()
	ON_WM_MEASUREITEM_REFLECT()
	ON_NOTIFY_REFLECT(NM_CLICK, &vxListCtrl::OnNMClick)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, &vxListCtrl::OnLvnColumnclick)
END_MESSAGE_MAP()

/*******************************************************************************
* Description:
*     WM_CREATE message handler.
*/
int vxListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListCtrl::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}

	InitListCtrl();

	return 0;
}

/*******************************************************************************
* Description:
*     WM_PAINT message handler.
*/
void vxListCtrl::OnPaint()
{
	CPaintDC dc(this);

	if (!m_bEnablePaintProc)
	{
		return;
	}

	CRect rectCli;
	GetClientRect(rectCli);

	BOOL bReInit = (NULL == (HDC)m_memDC) || (m_memDC.MaxCX() < rectCli.Width()) || (m_memDC.MaxCY() < rectCli.Height());
	if (bReInit)
	{
		m_memDC.Release();
		if (!m_memDC.Initial(NULL, rectCli.Width(), rectCli.Height()))
			return;
	}

	CDC *pDC = CDC::FromHandle((HDC)m_memDC);
	pDC->FillSolidRect(rectCli, GetBkColor());

	CListCtrl::DefWindowProc(WM_PAINT, (WPARAM)(HDC)m_memDC, NULL);

	dc.BitBlt(rectCli.left, rectCli.top, rectCli.Width(), rectCli.Height(),
				CDC::FromHandle((HDC)m_memDC), 0, 0, SRCCOPY);
}

/*******************************************************************************
* Description:
*     WM_ERASEBKGND message handler.
*/
BOOL vxListCtrl::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

/*******************************************************************************
* Description:
*     WM_SIZE message handler.
*/
void vxListCtrl::OnSize(UINT nType, int cx, int cy)
{
	//AutoLayoutColumn(cx);
	CListCtrl::OnSize(nType, cx, cy);

	SetTimer(IDT_TIMER_UPDATE, 50, NULL);
}

/*******************************************************************************
* Description:
*     WM_TIMER message handler.
*/
void vxListCtrl::OnTimer(UINT_PTR nIDEvent)
{
	if (IDT_TIMER_UPDATE == nIDEvent)
	{
		KillTimer(IDT_TIMER_UPDATE);

		RECT rcCient;
		GetClientRect(&rcCient);
		AutoLayoutColumn(rcCient.right - rcCient.left);
	}

	CListCtrl::OnTimer(nIDEvent);
}

/*******************************************************************************
* Description:
*     WM_MEASUREITEM message handler.
*/
void vxListCtrl::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	CListCtrl::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}

/*******************************************************************************
* Description:
*     WM_MEASUREITEM_REFLECT message handler.
*/
void vxListCtrl::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	if (m_nRowHeight > 0)
	{
		lpMeasureItemStruct->itemHeight = m_nRowHeight;
	}
}

/*******************************************************************************
* Description:
*     ON_NOTIFY_REFLECT[NM_CLICK] message handler.
*/
void vxListCtrl::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast< LPNMITEMACTIVATE >(pNMHDR);

	// 处理鼠标点击勾选框事件
	do 
	{
		if (-1 == pNMItemActivate->iItem)
			break;

		// 控件是否有勾选框样式
		if (!(GetExtendedStyle() & LVS_EX_CHECKBOXES))
			break;

		// 勾选框矩形区域
		CRect rcBox;
		if (!GetItemCheckBoxRect(pNMItemActivate->iItem, rcBox))
			break;
		CPoint point((LPARAM)GetMessagePos());
		ScreenToClient(&point);

		// 判断是否点在 勾选框 上
		if (!rcBox.PtInRect(point))
			break;

		SetCheck(pNMItemActivate->iItem, !GetCheck(pNMItemActivate->iItem));
	} while (0);

	*pResult = 0;
}

/*******************************************************************************
* Description:
*     ON_NOTIFY_REFLECT[LVN_COLUMNCLICK] message handler.
*/
void vxListCtrl::OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast< LPNMLISTVIEW >(pNMHDR);
	*pResult = 0;

	if (!m_bEnableSortItem)
	{
		return;
	}

	CString strText;

	//======================================
	// 列 升序/降序 选择

	int nColume = pNMLV->iSubItem;

	TCHAR szHdTextBuf[MAX_PATH];
	memset(szHdTextBuf, 0, MAX_PATH * sizeof(TCHAR));

	HDITEM hdItem;
	memset(&hdItem, 0, sizeof(HDITEM));

	hdItem.mask       = HDI_TEXT;
	hdItem.pszText    = szHdTextBuf;
	hdItem.cchTextMax = MAX_PATH;
	if (!m_wndHeaderCtrl.GetItem(nColume, &hdItem))
	{
		return;
	}

	strText.Format(TEXT("COLUME : %s"), szHdTextBuf);

	BOOL bAsc = (NULL != GetKey(strText, (HANDLE)TRUE));

	//======================================
	// 触发排列操作

	SortParam spDataPtr;
	spDataPtr.SP_ListCtrl = (HANDLE)this;
	spDataPtr.SP_Asc      = bAsc;
	spDataPtr.SP_Colume   = nColume;
	spDataPtr.SP_Reserved = NULL;

	SortItems(&vxListCtrl::CompareEntry, (DWORD_PTR)&spDataPtr);

	// 逆转 升序/降序
	SetKey(strText, (HANDLE)(!bAsc));

	//======================================
}
