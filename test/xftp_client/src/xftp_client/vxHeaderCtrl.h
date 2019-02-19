/*
* Copyright (c) 2013, 百年千岁
* All rights reserved.
* 
* 文件名称：vxHeaderCtrl.h
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

#ifndef __VXHEADERCTRL_H__
#define __VXHEADERCTRL_H__

#include "vxDC.h"

////////////////////////////////////////////////////////////////////////////////

inline RGBQUAD vxRGBA(BYTE r, BYTE g, BYTE b, BYTE a)
{
	RGBQUAD rgba = { b, g, r, a };
	return rgba;
}

inline COLORREF vxRGBA_TO_RGB(const RGBQUAD &rgba)
{
	return RGB(rgba.rgbRed, rgba.rgbGreen, rgba.rgbBlue);
}

inline RGBQUAD vxCOLORREF_TO_RGBA(const COLORREF &rgb)
{
	RGBQUAD rgba = { GetBValue(rgb), GetGValue(rgb), GetRValue(rgb), (BYTE)(rgb >> 24) };
	return rgba;
}

////////////////////////////////////////////////////////////////////////////////

// ListCtrl 的外部颜色配置标识 ID
typedef enum vxLCColorVID
{
	LCCID_BKGND       = 101,         ///< 背景
	LCCID_TEXT        = 102,         ///< 文本
	LCCID_DISABLED    = 103,         ///< 禁用状态
	LCCID_HEAD_TEXT   = 110,         ///< 列表头部文本颜色
	LCCID_HEAD_TOP    = 111,         ///< 列表头部控件上部
	LCCID_HEAD_BOTTOM = 112,         ///< 列表头部控件底部
	LCCID_LITEM_SEL   = 121,         ///< 子项选中状态背景
	LCCID_LITEM_ODD   = 122,         ///< 奇子项默认背景
	LCCID_LITEM_EVEN  = 123,         ///< 偶子项默认背景
} vxLCColorVID;

typedef enum vxLCColor
{
	LCC_BKGND         = RGB(255, 255, 255),  ///< 背景
	LCC_TEXT          = RGB(0  , 0  , 0  ),  ///< 文本
	LCC_DISABLED      = RGB(244, 247, 252),  ///< 禁用状态
	LCC_HEAD_TEXT     = RGB(0  , 0  , 0  ),  ///< 列表头部文本颜色
	LCC_HEAD_TOP      = RGB(196, 196, 196),  ///< 列表头部控件上部
	LCC_HEAD_BOTTOM   = RGB(164, 164, 164),  ///< 列表头部控件底部
	LCC_LITEM_SEL     = RGB(160, 160, 160),  ///< 子项选中状态背景
	LCC_LITEM_ODD     = RGB(240, 240, 240),  ///< 奇子项默认背景
	LCC_LITEM_EVEN    = RGB(255, 255, 255),  ///< 偶子项默认背景
} vxLCColor;

////////////////////////////////////////////////////////////////////////////////
// vxHeaderCtrl

class vxHeaderCtrl : public CHeaderCtrl
{
	DECLARE_DYNAMIC(vxHeaderCtrl)

	// constructor/destructor
public:
	vxHeaderCtrl(void);
	virtual ~vxHeaderCtrl(void);

	// class properties
public:
	typedef enum emConstValue
	{
		ECV_DISABLE_COLOR    = RGB(240, 240, 240),
	} emConstValue;

	inline int SetFocusColItem(int nColItem)
	{
		m_nFocusColItem = nColItem;
		return m_nFocusColItem;
	}

	inline int GetFocusColItem(void) const
	{
		return m_nFocusColItem;
	}

	inline int SetMinHeight(int nMinHeight)
	{
		m_nMinHeight = nMinHeight;
		return m_nMinHeight;
	}

	inline int GetMinHeight(void) const
	{
		return m_nMinHeight;
	}

	// public interfaces
public:
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
	BOOL SetBkgndColor(RGBQUAD clr_start, RGBQUAD clr_end, BOOL bRedraw = TRUE);

	/*******************************************************************************
	* Description:
	*     获取列文本对齐方式。
	* Parameter:
	*     @[in ] nCol: 列 ID 。
	* ReturnValue:
	*      返回 { DT_LEFT, DT_RIGHT, DT_CENTER } 其中之一值，默认 返回 DT_LEFT 。
	*/
	UINT GetColTextAlign(int nCol);

	// self invoking methods
protected:
	/*******************************************************************************
	* Description:
	*     填充背景。
	* Parameter:
	*     @[in ] hDC: 目标 DC。
	*     @[in ] rcArea: 背景区域。
	*     @[in ] rgba_start: 立体渐变背景的起始颜色（带 Alpha 通道）。
	*     @[in ] rgba_end: 立体渐变背景的终止颜色（带 Alpha 通道）。
	*/
	void FillBkgnd(HDC hDC, const RECT &rcArea, const RGBQUAD &rgba_start, const RGBQUAD &rgba_end);

	/*******************************************************************************
	* Description:
	*     测试点落于头部的某项。
	* Parameter:
	*     @[in ] point: 测试点。
	* ReturnValue:
	*      成功，返回 项ID ；
	*      失败，返回 -1 。
	*/
	int HitItem(const CPoint &point);

	/*******************************************************************************
	* Description:
	*     重绘子项。
	* Parameter:
	*     @[in ] hDC: 目标绘图 DC （若为 NULL 时，将由 GetDC() 获取控件客户区 DC 作为目标 DC）。
	*     @[in ] iItem: 子项 ID 。
	*     @[in ] itemAction: { ODA_DRAWENTIRE, ODA_SELECT, ODA_FOCUS } 之中取值。
	*/
	void RedrawItem(HDC hDC, int iItem, UINT itemAction);

	// overrides
protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	// class data
private:
	BOOL        m_bMouseDown;
	int         m_nFocusColItem;
	BOOL        m_bTracked;

protected:
	vxMemDC     m_memDC;            // 用于绘制的缓存 DC
	COLORREF    m_clrText;          // 文本颜色
	RGBQUAD     m_rgba_start;       // 立体渐变背景的起始颜色（带 Alpha 通道）
	RGBQUAD     m_rgba_end;         // 立体渐变背景的终止颜色（带 Alpha 通道）

	float       m_fHeight_Times;    // 表头高度倍数（默认值为 1）
	int         m_nMinHeight;       // 表头高度最小值

	// message handlers
protected:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnLayout(WPARAM wParam, LPARAM lParam);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

};

#endif //__VXHEADERCTRL_H__


