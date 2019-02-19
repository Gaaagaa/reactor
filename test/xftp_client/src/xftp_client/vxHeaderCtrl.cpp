/*
* Copyright (c) 2013, 百年千岁
* All rights reserved.
* 
* 文件名称：vxHeaderCtrl.cpp
* 文件标识：
* 摘    要：表头控件（用于 vxListCtrl 控件的表头）。
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
#include "vxHeaderCtrl.h"

////////////////////////////////////////////////////////////////////////////////
// Track mouse

static void CALLBACK TrackMouseTimerProc(HWND hWnd, UINT /*uMsg*/, UINT idEvent, DWORD /*dwTime*/)
{
	RECT	rect;
	POINT	pt;

	::GetClientRect(hWnd, &rect);
	::MapWindowPoints(hWnd, NULL, (LPPOINT)&rect, 2);

	::GetCursorPos(&pt);
	if (!::PtInRect(&rect, pt) || (WindowFromPoint(pt) != hWnd))
	{
		if (!::KillTimer(hWnd, idEvent))
		{
			// Error killing the timer!
		}

		::PostMessage(hWnd, WM_MOUSELEAVE, 0, MAKELPARAM(pt.x, pt.y));
	}
}

static BOOL TrackMouse(LPTRACKMOUSEEVENT ptme)
{
	ASSERT(ptme != NULL);
	if (ptme->cbSize < sizeof(TRACKMOUSEEVENT))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (!::IsWindow(ptme->hwndTrack)) 
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (!(ptme->dwFlags & TME_LEAVE)) 
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return (BOOL)::SetTimer(ptme->hwndTrack, ptme->dwFlags, 100, (TIMERPROC)TrackMouseTimerProc);
}

////////////////////////////////////////////////////////////////////////////////
// vxHeaderCtrl

IMPLEMENT_DYNAMIC(vxHeaderCtrl, CHeaderCtrl)

//==============================================================================

// 
// constructor/destructor
// 

vxHeaderCtrl::vxHeaderCtrl(void)
			: m_bMouseDown(FALSE)
			, m_nFocusColItem(-1)
			, m_bTracked(FALSE)
			, m_fHeight_Times(1.0F)
			, m_nMinHeight(24)
{
	m_clrText    = LCC_HEAD_TEXT;
	m_rgba_start = vxCOLORREF_TO_RGBA(LCC_HEAD_TOP);
	m_rgba_end   = vxCOLORREF_TO_RGBA(LCC_HEAD_BOTTOM);
}

vxHeaderCtrl::~vxHeaderCtrl(void)
{

}

//==============================================================================

// 
// public interfaces
// 

/*******************************************************************************
* Description:
*     设置背景的颜色。
* Parameter:
*     @[in ] clr_start: 立体渐变背景的起始颜色（带 Alpha 通道）。
*     @[in ] clr_end: 立体渐变背景的终止颜色（带 Alpha 通道）。
*     @[in ] bRedraw: 是否立即重绘控件。
* ReturnValue:
*      成功，返回 TRUE；
*      失败，返回 FALSE。
*/
BOOL vxHeaderCtrl::SetBkgndColor(RGBQUAD clr_start, RGBQUAD clr_end, BOOL bRedraw /* = TRUE */)
{
	m_rgba_start = clr_start;
	m_rgba_end   = clr_end;
	if (bRedraw) Invalidate(TRUE);
	return TRUE;
}

/*******************************************************************************
* Description:
*     获取列文本对齐方式。
* Parameter:
*     @[in ] nCol: 列 ID 。
* ReturnValue:
*      返回 { DT_LEFT, DT_RIGHT, DT_CENTER } 其中之一值，默认 返回 DT_LEFT 。
*/
UINT vxHeaderCtrl::GetColTextAlign(int nCol)
{
	HDITEM hdi;
	const UINT DTFormat[3] = { DT_LEFT, DT_RIGHT, DT_CENTER };

	UINT nDTFormat = DT_LEFT;
	hdi.mask       = HDI_FORMAT;
	if (!GetItem(nCol, &hdi))
		return nDTFormat;
	nDTFormat = DTFormat[max(0, min(hdi.fmt & 0x0F, 2))];
	return nDTFormat;
}

//==============================================================================

// 
// self invoking methods
// 

/*******************************************************************************
* Description:
*     填充背景。
* Parameter:
*     @[in ] hDC: 目标 DC。
*     @[in ] rcArea: 背景区域。
*     @[in ] rgba_start: 立体渐变背景的起始颜色（带 Alpha 通道）。
*     @[in ] rgba_end: 立体渐变背景的终止颜色（带 Alpha 通道）。
*/
void vxHeaderCtrl::FillBkgnd(HDC hDC, const RECT &rcArea, const RGBQUAD &rgba_start, const RGBQUAD &rgba_end)
{
	TRIVERTEX rcVertex[2];

	rcVertex[0].x     = rcArea.left;
	rcVertex[0].y     = rcArea.top;
	rcVertex[0].Red   = rgba_start.rgbRed      << 8;
	rcVertex[0].Green = rgba_start.rgbGreen    << 8;
	rcVertex[0].Blue  = rgba_start.rgbBlue     << 8;
	rcVertex[0].Alpha = rgba_start.rgbReserved << 8;

	rcVertex[1].x     = rcArea.right; 
	rcVertex[1].y     = rcArea.bottom;
	rcVertex[1].Red   = rgba_end.rgbRed        << 8;
	rcVertex[1].Green = rgba_end.rgbGreen      << 8;
	rcVertex[1].Blue  = rgba_end.rgbBlue       << 8;
	rcVertex[1].Alpha = rgba_end.rgbReserved   << 8;

	GRADIENT_RECT Gradient_rect;
	Gradient_rect.UpperLeft  = 0;
	Gradient_rect.LowerRight = 1;

	::GradientFill(hDC, rcVertex, 2, &Gradient_rect, 1, GRADIENT_FILL_RECT_V);
}

/*******************************************************************************
* Description:
*     测试点落于头部的某项。
* Parameter:
*     @[in ] point: 测试点。
* ReturnValue:
*      成功，返回 项ID ；
*      失败，返回 -1 。
*/
int vxHeaderCtrl::HitItem(const CPoint &point)
{
	HDHITTESTINFO hdhti = { 0 };
	hdhti.pt = point;
	int iItem = HitTest(&hdhti);
	if (-1 == iItem)
		return iItem;

	CRect rcItem(0, 0, 0, 0);
	if (!GetItemRect(iItem, rcItem))
		return -1;
	rcItem.DeflateRect(3, 0);
	return rcItem.PtInRect(point) ? iItem : -1;
}

/*******************************************************************************
* Description:
*     重绘子项。
* Parameter:
*     @[in ] hDC: 目标绘图 DC （若为 NULL 时，将由 GetDC() 获取控件客户区 DC 作为目标 DC）。
*     @[in ] iItem: 子项 ID 。
*     @[in ] itemAction: { ODA_DRAWENTIRE, ODA_SELECT, ODA_FOCUS } 之中取值。
*/
void vxHeaderCtrl::RedrawItem(HDC hDC, int iItem, UINT itemAction)
{
	RECT rcItem;
	if (!GetItemRect(iItem, &rcItem))
		return;

	CDC * pDC = (NULL != hDC) ? CDC::FromHandle(hDC) : GetDC();

	DRAWITEMSTRUCT dis;
	dis.CtlType    = 100;
	dis.hDC        = pDC->GetSafeHdc();
	dis.itemAction = itemAction;
	dis.hwndItem   = GetSafeHwnd(); 
	dis.rcItem     = rcItem;
	dis.itemID     = iItem;
	DrawItem(&dis);

	(NULL != hDC) ? NULL : ReleaseDC(pDC);
}

//==============================================================================

// 
// overrides
// 

void vxHeaderCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC * pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

	// 填充背景
	CRect rectArea = lpDrawItemStruct->rcItem;
	RGBQUAD rgba_start = { 0, 0, 0, 0 };
	RGBQUAD rgba_end   = m_rgba_end;
	switch (lpDrawItemStruct->itemAction)
	{
	case ODA_DRAWENTIRE:   rgba_start = m_rgba_start;     break;
	case ODA_SELECT:
		{
			rgba_start.rgbRed      = (m_rgba_start.rgbRed      + m_rgba_end.rgbRed     ) / 3;
			rgba_start.rgbGreen    = (m_rgba_start.rgbGreen    + m_rgba_end.rgbGreen   ) / 3;
			rgba_start.rgbBlue     = (m_rgba_start.rgbBlue     + m_rgba_end.rgbBlue    ) / 3;
			rgba_start.rgbReserved = (m_rgba_start.rgbReserved + m_rgba_end.rgbReserved) / 3;
		}
	   break;
	case ODA_FOCUS:
		{
			rgba_start.rgbRed      = (m_rgba_start.rgbRed      + m_rgba_end.rgbRed     ) / 2;
			rgba_start.rgbGreen    = (m_rgba_start.rgbGreen    + m_rgba_end.rgbGreen   ) / 2;
			rgba_start.rgbBlue     = (m_rgba_start.rgbBlue     + m_rgba_end.rgbBlue    ) / 2;
			rgba_start.rgbReserved = (m_rgba_start.rgbReserved + m_rgba_end.rgbReserved) / 2;
		}
		break;
	default:               rgba_start = m_rgba_start;     break;
	}

	if (!::IsWindowEnabled(GetParent()->GetSafeHwnd()))
	{
		rgba_start = vxCOLORREF_TO_RGBA(LCC_DISABLED);
		rgba_end   = vxCOLORREF_TO_RGBA(LCC_DISABLED);
	}

	FillBkgnd(lpDrawItemStruct->hDC, rectArea, rgba_start, rgba_end);

	HDITEM hdi;
	const UINT DTFormat[3] = { DT_LEFT, DT_RIGHT, DT_CENTER };
	// 获取文本和绘制的对齐方式
	TCHAR szText[MAX_PATH] = { 0 };
	UINT nDTFormat = DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS;
	hdi.mask       = HDI_TEXT | HDI_FORMAT;
	hdi.pszText    = szText;
	hdi.cchTextMax = MAX_PATH;
	if (!GetItem(lpDrawItemStruct->itemID, &hdi))
		return;
	nDTFormat = DTFormat[max(0, min(hdi.fmt & 0x0F, 2))] | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS;

	HGDIOBJ holdFont = pDC->SelectObject(::GetStockObject(DEFAULT_GUI_FONT));
	int nBkMode = pDC->SetBkMode(TRANSPARENT);
	COLORREF clrOldText = pDC->SetTextColor(m_clrText);

	// 计算文本输出矩形区域
	RECT rcText = rectArea;
	pDC->DrawText(szText, &rcText, nDTFormat | DT_CALCRECT);
	int dy = ((rectArea.bottom - rectArea.top) - (rcText.bottom - rcText.top)) / 2;
	::OffsetRect(&rcText, 0, dy);
	rcText.left  = rectArea.left + 3;
	rcText.right = rectArea.right - 3;

	// 绘制文本
	pDC->DrawText(szText, &rcText, nDTFormat);

	pDC->SetTextColor(clrOldText);
	pDC->SetBkMode(nBkMode);
	pDC->SelectObject(holdFont);

	// 绘制分割线（边框）
	pDC->Draw3dRect(&lpDrawItemStruct->rcItem, vxRGBA_TO_RGB(rgba_start), GetSysColor(COLOR_GRAYTEXT));
}

//==============================================================================

// 
// vxHeaderCtrl message handlers
// 

BEGIN_MESSAGE_MAP(vxHeaderCtrl, CHeaderCtrl)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(HDM_LAYOUT, &vxHeaderCtrl::OnLayout)
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_MESSAGE(WM_MOUSELEAVE, &vxHeaderCtrl::OnMouseLeave)
END_MESSAGE_MAP()

/*******************************************************************************
* Description:
*     WM_PAINT message handler.
*/
void vxHeaderCtrl::OnPaint()
{
	CPaintDC dc(this);

	CRect rectCli;
	GetClientRect(rectCli);

	// 创建内存 DC，使用双缓冲方式绘图
	BOOL bReInitMemDC = (NULL == (HDC)m_memDC) ||
		(m_memDC.MaxCX() < (2 * rectCli.Width())) || (m_memDC.MaxCY() < (2 * rectCli.Height()));
	if (bReInitMemDC)
	{
		m_memDC.Release();
		m_memDC.Initial(dc.m_hDC, 2 * rectCli.Width(), 2 * rectCli.Height());
	}

	RGBQUAD rgbStart = m_rgba_start;
	RGBQUAD rgbEnd   = m_rgba_end;
	if (!::IsWindowEnabled(GetParent()->GetSafeHwnd()))
	{
		rgbStart.rgbRed = rgbStart.rgbGreen = rgbStart.rgbBlue = 240;
		rgbEnd.rgbRed   = rgbEnd.rgbGreen   = rgbEnd.rgbBlue   = 240;
	}
	CDC::FromHandle(m_memDC)->FillSolidRect(
		CRect(0, 0, 2 * rectCli.Width(), 2 * rectCli.Height()), vxRGBA_TO_RGB(rgbStart));

	// 绘制子项
	for (int iItem = 0; iItem < GetItemCount(); ++iItem)
	{
		if (iItem != GetFocusColItem())
			RedrawItem(m_memDC, iItem, ODA_DRAWENTIRE);
		else
			RedrawItem(m_memDC, iItem, m_bMouseDown ? ODA_SELECT : ODA_FOCUS);
	}

	// 子项区域
	CRect rectItemArea = rectCli;
	if (GetItemCount() > 0)
	{
		CRect rcItem(0, 0, 0, 0);
		if (GetItemRect(GetItemCount() - 1, rcItem))
			rectItemArea.right = rcItem.right;
	}

	// 填充剩余区域背景
	CRect rectArea = rectCli;
	rectArea.left = rectItemArea.right;
	FillBkgnd(m_memDC, rectArea, rgbStart, rgbEnd);

	::BitBlt(dc.GetSafeHdc(), rectCli.left, rectCli.top, rectCli.Width(), rectCli.Height(),
		m_memDC, rectCli.left, rectCli.top, SRCCOPY);
}

/*******************************************************************************
* Description:
*     WM_ERASEBKGND message handler.
*/
BOOL vxHeaderCtrl::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

/*******************************************************************************
* Description:
*     MESSAGE[HDM_LAYOUT] message handler.
*/
LRESULT vxHeaderCtrl::OnLayout(WPARAM wParam, LPARAM lParam)
{
	LRESULT l_res = CHeaderCtrl::DefWindowProc(HDM_LAYOUT, wParam, lParam);

	HD_LAYOUT &hd_layout = *(HD_LAYOUT *)lParam;
	int nHeight = (int)(hd_layout.pwpos->cy * m_fHeight_Times);
	if (nHeight < m_nMinHeight) nHeight = m_nMinHeight;

	hd_layout.pwpos->cy = nHeight;
	hd_layout.prc->top  = nHeight;

	return l_res;
}

/*******************************************************************************
* Description:
*     WM_KILLFOCUS message handler.
*/
void vxHeaderCtrl::OnKillFocus(CWnd* pNewWnd)
{
	if (m_bMouseDown)
	{
		POINT pt;
		::GetCursorPos(&pt);
		::PostMessage(m_hWnd, WM_MOUSELEAVE, 0, MAKELPARAM(pt.x, pt.y));
	}
	CHeaderCtrl::OnKillFocus(pNewWnd);
}

/*******************************************************************************
* Description:
*     WM_LBUTTONDOWN message handler.
*/
void vxHeaderCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	// 设置控件按下标识特性
	m_bMouseDown = TRUE;
	SetFocusColItem(HitItem(point));

	CHeaderCtrl::OnLButtonDown(nFlags, point);
}

/*******************************************************************************
* Description:
*     WM_MOUSEMOVE message handler.
*/
void vxHeaderCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	int iItem = HitItem(point);
	if (-1 != iItem)
	{
		if (!m_bMouseDown && (iItem != GetFocusColItem()))
		{
			RedrawItem(NULL, iItem, ODA_FOCUS);

			if (-1 != GetFocusColItem())
				RedrawItem(NULL, GetFocusColItem(), ODA_DRAWENTIRE);
			SetFocusColItem(iItem);
		}
	}
	else
	{
		if (!m_bMouseDown && (-1 != GetFocusColItem()))
		{
			RedrawItem(NULL, GetFocusColItem(), ODA_DRAWENTIRE);
			SetFocusColItem(iItem);
		}
	}

	if (!m_bTracked)
	{
		m_bTracked = TRUE;

		TRACKMOUSEEVENT trackmouseevent;
		trackmouseevent.cbSize = sizeof(trackmouseevent);
		trackmouseevent.dwFlags = TME_LEAVE;
		trackmouseevent.hwndTrack = GetSafeHwnd();
		trackmouseevent.dwHoverTime = HOVER_DEFAULT;
		TrackMouse(&trackmouseevent);
	}

	CHeaderCtrl::OnMouseMove(nFlags, point);
}

/*******************************************************************************
* Description:
*     WM_LBUTTONUP message handler.
*/
void vxHeaderCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	int iItem = HitItem(point);
	if (m_bMouseDown && (GetFocusColItem() != iItem))
	{
		RedrawItem(NULL, iItem, ODA_FOCUS);

		if (-1 != GetFocusColItem())
			RedrawItem(NULL, GetFocusColItem(), ODA_DRAWENTIRE);
		SetFocusColItem(iItem);
	}

	SetFocusColItem(iItem);
	m_bMouseDown = FALSE;

	CHeaderCtrl::OnLButtonUp(nFlags, point);
}

/*******************************************************************************
* Description:
*     WM_MOUSELEAVE message handler
*/
LRESULT vxHeaderCtrl::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	m_bTracked = FALSE;

	CPoint point(lParam);
	if (!m_bMouseDown && (-1 != GetFocusColItem()))
	{
		RedrawItem(NULL, GetFocusColItem(), ODA_DRAWENTIRE);
		SetFocusColItem(-1);
	}

	return 0;
}

