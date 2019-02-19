/*
* Copyright (c) 2012, 百年千岁
* All rights reserved.
* 
* 文件名称：vxDC.cpp
* 文件标识：
* 摘    要：DC 绘图辅助类。
* 
* 当前版本：1.0
* 作    者：百年千岁
* 完成日期：2012年10月4日
* 
* 取代版本：
* 原作者  ：
* 完成日期：
*/

#include "stdafx.h"
#include "vxDC.h"

#include <math.h>
#include <assert.h>
#include <algorithm>

#include <atlimage.h>

#define DRAWDIB_INCLUDE_STRETCHDIB
#include <Vfw.h>
#pragma comment(lib, "vfw32.lib")

////////////////////////////////////////////////////////////////////////////////

#ifndef X_ASSERT
#ifdef _DEBUG
#define X_ASSERT(exps)  assert((exps))
#else // _DEBUG
#define X_ASSERT(express)
#endif // _DEBUG
#endif // X_ASSERT

////////////////////////////////////////////////////////////////////////////////

#define PI     3.141593

////////////////////////////////////////////////////////////////////////////////

inline int nbytes_line(int widthpix, int nBits)
{
	return (int)(((widthpix * nBits) + 31) / 32 * 4);
}

/******************************************************************************/
/**
* @brief
* <pre>
*   访问类型 : public 
*   功能描述 : 创建内存位图操作句柄。
* </pre>
* 
* @param [in ] cx    : 位图宽度。
* @param [in ] cy    : 位图高度（正、负值均可）。
* @param [in ] nBits : 位图像素位数（24 或 32）。
* @param [in ] pBits : 操作成功返回的 RGB 像素数据内存地址。
* 
* @return HBITMAP
*         - 成功，返回 位图操作句柄；
*         - 失败，返回 NULL。
*/
HBITMAP vxCreateBitmap(int cx, int cy, int nBits, void ** pBits)
{
	X_ASSERT(0 < cx);
	X_ASSERT(0 != cy);

	if (((24 != nBits) && (32 != nBits)) || (cx <= 0) /*|| (cy == 0)*/)
	{
		return NULL;
	}

	if (NULL != pBits)
	{
		*pBits = NULL;
	}

	BITMAPINFO bi;
	memset(&bi, 0, sizeof(BITMAPINFO));

	// Fill in the BITMAPINFOHEADER
	bi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth       = cx;
	bi.bmiHeader.biHeight      = cy;
	bi.bmiHeader.biSizeImage   = 0;
	bi.bmiHeader.biPlanes      = 1;
	bi.bmiHeader.biBitCount    = nBits;
	bi.bmiHeader.biCompression = BI_RGB;

	LPVOID pData = NULL;
	HBITMAP hBmp = ::CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, &pData, NULL, 0);

	if ((NULL != pData) && (NULL != hBmp) && (NULL != pBits))
	{
		*pBits = pData;
	}

	return hBmp;
}

/******************************************************************************/
/**
* @brief
* <pre>
*   访问类型 : public 
*   功能描述 : 向目标 DC 绘制矩形。
* </pre>
* 
* @param [in ] hDC     : 目标 DC。
* @param [in ] lpRect  : 目标矩形区域。
* @param [in ] cwLine  : 矩形线条宽度。
* @param [in ] clrLine : 矩形线条颜色。
* 
* @return void
*         
*/
void vxDrawRect(HDC hDC, LPRECT lpRect, int cwLine, COLORREF clrLine)
{
	X_ASSERT((NULL != hDC) && (NULL != lpRect));

	COLORREF clrSwap = SetBkColor(hDC, clrLine);

	int cx = (int)(std::abs)(lpRect->right - lpRect->left);
	int cy = (int)(std::abs)(lpRect->bottom - lpRect->top);

	if ((cx <= cwLine) || (cy <= cwLine))
	{
		ExtTextOut(hDC, 0, 0, ETO_OPAQUE, lpRect, NULL, 0, NULL);
	}
	else
	{
		RECT rcL = { lpRect->left, lpRect->top, lpRect->left + cwLine, lpRect->bottom };
		ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rcL, NULL, 0, NULL);

		RECT rcT = { lpRect->left + cwLine, lpRect->top, lpRect->right, lpRect->top + cwLine };
		ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rcT, NULL, 0, NULL);

		RECT rcR = { lpRect->right - cwLine, lpRect->top + cwLine, lpRect->right, lpRect->bottom };
		ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rcR, NULL, 0, NULL);

		RECT rcB = { lpRect->left, lpRect->bottom - cwLine, lpRect->right - cwLine, lpRect->bottom };
		ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rcB, NULL, 0, NULL);
	}

	SetBkColor(hDC, clrSwap);
}

/******************************************************************************/
/**
* @brief
* <pre>
*   访问类型 : public 
*   功能描述 : 使用指定颜色填充目标 DC 的矩形区域。
* </pre>
* 
* @param [in ] hDC     : 目标 DC。
* @param [in ] lpRect  : 目标矩形区域。
* @param [in ] clrRect : 填充的颜色。
* 
* @return void
*         
*/
void vxFillSolidRect(HDC hDC, LPRECT lpRect, COLORREF clrRect)
{
	X_ASSERT((NULL != hDC) && (NULL != lpRect));

	COLORREF clrSwap = SetBkColor(hDC, clrRect);
	ExtTextOut(hDC, 0, 0, ETO_OPAQUE, lpRect, NULL, 0, NULL);
	SetBkColor(hDC, clrSwap);
}

/******************************************************************************/
/**
* @brief
* <pre>
*   访问类型 : public 
*   功能描述 : 绘制 文本信息 。
* </pre>
* 
* @param [in ] hDC         : 目标 DC。
* @param [in ] szText      : 文本信息。
* @param [in ] clrText     : 文本颜色。
* @param [in ] hbrBkgnd    : 背景画刷（若为 NULL，则背景透明）。
* @param [in ] lpDstRect   : 目标矩形区域。
* @param [in ] dwDTAligned : 对齐方式（如 DT_LEFT、DT_CENTER、DT_RIGHT 等的组合值）。
* 
* @return BOOL
*         - 成功，返回 TRUE；
*         - 失败，返回 FALSE。
*/
BOOL vxDrawText(HDC hDC, LPCTSTR szText, COLORREF clrText, HBRUSH hbrBkgnd, LPRECT lpDstRect, DWORD dwDTAligned)
{
	BOOL bRet = FALSE;

	do 
	{
		// 输入数据有效性判断
		if ((NULL == hDC) || (NULL == szText) || (_tcslen(szText) <= 0) || (NULL == lpDstRect))
		{
			break;
		}

		// 绘制文本信息流程
		BOOL     bTransparent = (NULL == hbrBkgnd);
		HGDIOBJ  hSwapBrush   = SelectObject(hDC, bTransparent ? GetStockObject(NULL_BRUSH) : (HGDIOBJ)hbrBkgnd);
		int      nSwapBkMode  = bTransparent ? SetBkMode(hDC, TRANSPARENT) : GetBkMode(hDC);
		COLORREF clrSwapText  = SetTextColor(hDC, clrText);

		int cchText = (int)_tcslen(szText);
		DrawText(hDC, szText, cchText, lpDstRect, dwDTAligned);

		SelectObject(hDC, hSwapBrush);
		SetBkMode   (hDC, nSwapBkMode);
		SetTextColor(hDC, clrSwapText);

		bRet = TRUE;
	} while (0);

	return bRet;
}

////////////////////////////////////////////////////////////////////////////////
// vxMemDC

// 
// vxMemDC : common invoking
// 

/******************************************************************************/
/**
* @brief
* <pre>
*   访问类型 : public static 
*   功能描述 : 保存内存 DC 的图像数据至文件。
* </pre>
* 
* @param [in ] szFile   : 文件名。
* @param [in ] fileType : 保存的文件类型。
* 
* @return BOOL
*         - 成功，返回 TRUE；
*         - 失败，返回 FALSE 。
*/
BOOL vxMemDC::SaveToFile(vxMemDC & hMemDC, LPCTSTR szFile, emSaveType fileType)
{
	BOOL bOK = FALSE;

	do 
	{
		// 初始化判断
		if (NULL == (HDC)hMemDC)
		{
			break;
		}

		// 使用 CImage 类的文件保存接口
		CImage img;
		img.Attach(hMemDC.Detach());

		HRESULT hr = E_FAIL;
		switch (fileType)
		{
		case STF_RGB: hr = img.Save(szFile, Gdiplus::ImageFormatBMP);  break;
		case STF_PNG: hr = img.Save(szFile, Gdiplus::ImageFormatPNG);  break;
		case STF_JPG: hr = img.Save(szFile, Gdiplus::ImageFormatJPEG); break;
		default: break;
		}

		hMemDC.Attach(img.Detach());

		bOK = SUCCEEDED(hr);
	} while (0);

	return bOK;
}

/******************************************************************************/
/**
* @brief
* <pre>
*   访问类型 : public static 
*   功能描述 : 自动调整 内存DC 的大小。
* </pre>
* 
* @param [out] hMemDC : 操作的内存 DC 对象。
* @param [in ] max_cx : 最大宽度。
* @param [in ] max_cy : 最大高度。
* 
* @return x_bool_t
*         - 返回 TRUE ，表示 内存 DC 的大小调整有效；
*         - 返回 FALSE，表示 内存 DC 的大小调整无效。
*/
BOOL vxMemDC::Assign(vxMemDC & hMemDC, int max_cx, int max_cy)
{
	BOOL bAssign = ((NULL == (HDC)hMemDC) || (hMemDC.MaxCX() < max_cx) || (hMemDC.MaxCY() < max_cy));

	if (!bAssign)
	{
		return TRUE;
	}

	vxMemDC hNewMemDC;

	if (NULL == hMemDC.GetBits())
		hNewMemDC.Initial((HDC)hMemDC, max_cx, max_cy, hMemDC.BitCount());
	else
		hNewMemDC.Initial(NULL, max_cx, max_cy, hMemDC.BitCount());

	if (NULL != (HDC)hNewMemDC)
	{
		hMemDC    = hNewMemDC;
		hNewMemDC = 0;
		return (NULL != (HDC)hMemDC);
	}

	return FALSE;
}

//==============================================================================

// 
// constructor/destructor
// 

vxMemDC::vxMemDC(HDC hCompatibleDC, int cx, int cy, int nBits /*= 32*/)
		: m_hDC(NULL)
		, m_hBitmap(NULL)
		, m_hOldBitmap(NULL)
		, m_pBits(NULL)
		, m_nBitCount(32)
		, m_cxMaxDC(0)
		, m_cyMaxDC(0)
{
	BOOL bInit = Initial(hCompatibleDC, cx, cy, nBits);
	X_ASSERT(bInit);
}

vxMemDC::vxMemDC(void)
		: m_hDC(NULL)
		, m_hBitmap(NULL)
		, m_hOldBitmap(NULL)
		, m_pBits(NULL)
		, m_nBitCount(32)
		, m_cxMaxDC(0)
		, m_cyMaxDC(0)
{

}

vxMemDC::~vxMemDC(void)
{
	Release();
}

vxMemDC::vxMemDC(const vxMemDC & obj)
{
	operator=(obj);
}

//==============================================================================

// 
// vxMemDC : overrides
// 

vxMemDC & vxMemDC::operator = (const vxMemDC & obj)
{
	if (this != &obj)
	{
		Release();

		m_hDC        = obj.m_hDC       ;
		m_hBitmap    = obj.m_hBitmap   ;
		m_hOldBitmap = obj.m_hOldBitmap;
		m_pBits      = obj.m_pBits     ;
		m_nBitCount  = obj.m_nBitCount ;
		m_cxMaxDC    = obj.m_cxMaxDC   ;
		m_cyMaxDC    = obj.m_cyMaxDC   ;
	}

	return *this;
}

vxMemDC & vxMemDC::operator = (int nFlags)
{
	if (0 == nFlags)
	{
		m_hDC        = NULL;
		m_hBitmap    = NULL;
		m_hOldBitmap = NULL;
		m_pBits      = NULL;
		m_nBitCount  = 32;
		m_cxMaxDC    = 0;
		m_cyMaxDC    = 0;
	}

	return *this;
}

//==============================================================================

// 
// public interfaces
// 

/*******************************************************************************
* Description:
*     数据初始化接口。
*/
BOOL vxMemDC::Initial(HDC hCompatibleDC, int cx, int cy, int nBits /*= 32*/)
{
	hCompatibleDC = NULL; ///< 仅支持内存模式的 DC

	if (NULL != m_hDC)
		return FALSE;
	if (NULL != m_hBitmap)
		return FALSE;

	m_nBitCount = (24 == nBits) ? nBits : 32;

	m_hDC = ::CreateCompatibleDC(hCompatibleDC);
	X_ASSERT(NULL != m_hDC);

	if (NULL == hCompatibleDC)
		m_hBitmap = vxCreateBitmap((std::abs)(cx), cy, m_nBitCount, (LPVOID*)&m_pBits);
	else
		m_hBitmap = ::CreateCompatibleBitmap(hCompatibleDC, (std::abs)(cx), (std::abs)(cy));

	if (NULL != m_hBitmap)
	{
		m_cxMaxDC = (std::abs)(cx);
		m_cyMaxDC = (std::abs)(cy);
	}

	X_ASSERT(NULL != m_hBitmap);
	m_hOldBitmap = (HBITMAP)::SelectObject(m_hDC, (HGDIOBJ)m_hBitmap);

	return TRUE;
}

/*******************************************************************************
* Description:
*     数据释放。
*/
BOOL vxMemDC::Release(void)
{
	if (NULL != m_hDC)
	{
		HBITMAP hBitmap = Detach();
		if (NULL != hBitmap)
		{
			::DeleteObject((HGDIOBJ)hBitmap);
			m_pBits = NULL;
			m_nBitCount = 32;
		}

		::DeleteDC(m_hDC);

		m_hDC = NULL;
		m_hBitmap = NULL;
		m_hOldBitmap = NULL;

		m_cxMaxDC = 0;
		m_cyMaxDC = 0;
	}

	return TRUE;
}

/*******************************************************************************
* Description:
*     内存DC的尺寸最大宽度。
*/
int vxMemDC::MaxCX(void) const
{
	return (std::abs)(m_cxMaxDC);
}

/*******************************************************************************
* Description:
*     内存DC的尺寸最大高度。
*/
int vxMemDC::MaxCY(void) const
{
	return (std::abs)(m_cyMaxDC);
}

/*******************************************************************************
* Description:
*     附加新的位图句柄至内存 DC 中，返回旧的位图句柄。
*/
HBITMAP vxMemDC::Attach(HBITMAP hBitmap)
{
	X_ASSERT(NULL != m_hDC);
	X_ASSERT(NULL != hBitmap);

	HBITMAP hOldBitmap = Detach();
	m_hBitmap = hBitmap;
	m_hOldBitmap = (HBITMAP)::SelectObject(m_hDC, (HGDIOBJ)m_hBitmap);

	return m_hOldBitmap;
}

/*******************************************************************************
* Description:
*     分离出内存位图句柄。
*/
HBITMAP vxMemDC::Detach(void)
{
	X_ASSERT(NULL != m_hDC);

	::SelectObject(m_hDC, (HGDIOBJ)m_hOldBitmap);
	HBITMAP hDetachBitmap = m_hBitmap;
	m_hBitmap = NULL;
	m_hOldBitmap = NULL;

	return hDetachBitmap;
}

/*******************************************************************************
* Description:
*     HDC 操作符重载。
*/
vxMemDC::operator HDC()
{
	return m_hDC;
}

vxMemDC::operator HDC() const
{
	return m_hDC;
}

/*******************************************************************************
* FunctionName:
*     GetAreaRgbBits
* Description:
*     获取指定区域的 RGB 图像数据。
* Parameter:
*     @[out] lpRgbBits : 数据输出的缓存。
*     @[in ] nRgbBytes : 数据输出的缓存长度。
*     @[in ] lpRect    : 指定获取的区域。
*     @[in ] bFlipV    : 是否按照垂直翻转的方式拷贝。
* ReturnValue:
*     成功，返回 TRUE；失败，返回 FALSE。
*/
BOOL vxMemDC::GetAreaRgbBits(LPBYTE lpRgbBits, int nRgbBytes, LPRECT lpRect, BOOL bFlipV /*= FALSE*/)
{
	BOOL bOK = FALSE;

	do 
	{
		if (NULL == m_pBits)
		{
			break;
		}

		// 参数有效判断
		if ((NULL == lpRgbBits) || (nRgbBytes <= 0) || (NULL == lpRect))
		{
			break;
		}

		int cx = lpRect->right - lpRect->left;
		int cy = lpRect->bottom - lpRect->top;
		if ((cx <= 0) || (cx > MaxCX()) || (cy <= 0) || (cy > MaxCY()))
		{
			break;
		}

		// 缓存大小有效判断
		int dst_bytesline = nbytes_line(cx, BitCount());
		if (nRgbBytes < (dst_bytesline * cy))
		{
			break;
		}

		// 区域有效判断
		RECT  rectDC  = { 0, 0, MaxCX(), MaxCY() };
		POINT pointLT = { lpRect->left, lpRect->top };
		POINT pointRB = { lpRect->right - 1, lpRect->bottom - 1 };
		if (!PtInRect(&rectDC, pointLT) || !PtInRect(&rectDC, pointRB))
		{
			break;
		}

		// 执行拷贝操作
		LPBYTE lpMemBits  = (LPBYTE)GetBits();
		int src_bytesline = nbytes_line(MaxCX(), BitCount());
		int src_bytesleft = lpRect->left * BitCount() / 8;

		if (bFlipV)
		{
			for (int i = lpRect->top; i < lpRect->bottom; ++i)
			{
				BYTE * srcBits = lpMemBits + src_bytesline * i + src_bytesleft;
				BYTE * dstBits = lpRgbBits + dst_bytesline * (lpRect->bottom - i - 1);

				memcpy(dstBits, srcBits, src_bytesline);
			}
		}
		else
		{
			for (int i = lpRect->top; i < lpRect->bottom; ++i)
			{
				BYTE * srcBits = lpMemBits + src_bytesline * i + src_bytesleft;
				BYTE * dstBits = lpRgbBits + dst_bytesline * (i - lpRect->top);

				memcpy(dstBits, srcBits, src_bytesline);
			}
		}

		bOK = TRUE;
	} while (0);

	return bOK;
}

/******************************************************************************/
/**
* @brief
* <pre>
*   访问类型 : public 
*   功能描述 : 获取指定区域的 RGB 图像数据。
* </pre>
* 
* @param [out] lpScanBits : RGB 图像数据输出缓存。
* @param [in ] nStride    : 拷贝操作时的 步进长度。
* @param [in ] lpRect     : 指定获取的区域。
* @param [in ] bFlipV     : 是否按照垂直翻转的方式拷贝。
* 
* @return BOOL
*         - 成功，返回 TRUE；
*         - 失败，返回 FALSE。
*/
BOOL vxMemDC::CopyAreaRgbBits(LPBYTE lpScanBits, int nStride, LPRECT lpRect, BOOL bFlipV /*= FALSE*/)
{
	BOOL bOK = FALSE;

	do 
	{
		if (NULL == m_pBits)
		{
			break;
		}

		// 参数有效判断
		if ((NULL == lpScanBits) || (nStride <= 0) || (NULL == lpRect))
		{
			break;
		}

		int cx = lpRect->right - lpRect->left;
		int cy = lpRect->bottom - lpRect->top;
		if ((cx <= 0) || (cx > MaxCX()) || (cy <= 0) || (cy > MaxCY()))
		{
			break;
		}

		// 目标输出缓存的行步进值
		register int dst_bytesline = nStride;

		// 区域有效判断
		RECT  rectDC  = { 0, 0, MaxCX(), MaxCY() };
		POINT pointLT = { lpRect->left, lpRect->top };
		POINT pointRB = { lpRect->right - 1, lpRect->bottom - 1 };
		if (!PtInRect(&rectDC, pointLT) || !PtInRect(&rectDC, pointRB))
		{
			break;
		}

		// 执行拷贝操作
		LPBYTE lpMemBits  = (LPBYTE)GetBits();
		register int src_bytesline = nbytes_line(MaxCX(), BitCount());
		int src_bytesleft = lpRect->left * BitCount() / 8;

		int line_bytes = ((src_bytesline < dst_bytesline) ? src_bytesline : dst_bytesline);

		if (bFlipV)
		{
			register BYTE * srcBits = lpMemBits  + src_bytesline * lpRect->top + src_bytesleft;
			register BYTE * dstBits = lpScanBits + dst_bytesline * (lpRect->bottom - lpRect->top - 1);

			for (int i = lpRect->top; i < lpRect->bottom; ++i)
			{
				memcpy(dstBits, srcBits, line_bytes);

				srcBits += src_bytesline;
				dstBits -= dst_bytesline;
			}
		}
		else
		{
			register BYTE * srcBits = lpMemBits  + src_bytesline * lpRect->top + src_bytesleft;
			register BYTE * dstBits = lpScanBits + dst_bytesline * (lpRect->top - lpRect->top);

			for (int i = lpRect->top; i < lpRect->bottom; ++i)
			{
				memcpy(dstBits, srcBits, line_bytes);

				srcBits += src_bytesline;
				dstBits += dst_bytesline;
			}
		}

		bOK = TRUE;
	} while (0);

	return bOK;
}

/*******************************************************************************
* FunctionName:
*     SetAreaRgbBits
* Description:
*     设置指定区域的 RGB 图像数据。
* Parameter:
*     @[in ] lpRgbBits : RGB 图像数据输入的缓存。
*     @[in ] nBits     : RGB 图像像素位数（24 或 32）。
*     @[in ] lpRect    : 指定设置的区域（矩形区域的 宽度/高度，必须等同于 RGB 图像的 宽度/高度）。
*     @[in ] bFlipV    : 是否按照垂直翻转的方式拷贝。
* ReturnValue:
*     成功，返回 TRUE；失败，返回 FALSE。
*/
BOOL vxMemDC::SetAreaRgbBits(LPBYTE lpRgbBits, int nBits, LPRECT lpRect, BOOL bFlipV /*= FALSE*/)
{
	BOOL bOK = FALSE;

	do 
	{
		if (NULL == m_pBits)
		{
			break;
		}

		// 参数有效验证
		if ((NULL == lpRgbBits) || (BitCount() != nBits) || (NULL == lpRect))
		{
			break;
		}

		int cx = lpRect->right - lpRect->left;
		int cy = lpRect->bottom - lpRect->top;

		if ((cx <= 0) || (cx > MaxCX()) || (cy <= 0) || (cy > MaxCY()))
		{
			break;
		}

		// 区域有效判断
		RECT  rectDC  = { 0, 0, MaxCX(), MaxCY() };
		POINT pointLT = { lpRect->left, lpRect->top };
		POINT pointRB = { lpRect->right - 1, lpRect->bottom - 1 };
		if (!PtInRect(&rectDC, pointLT) || !PtInRect(&rectDC, pointRB))
		{
			break;
		}

		// 执行拷贝操作
		LPBYTE lpDstBits  = (LPBYTE)GetBits();
		int dst_bytesline = nbytes_line(MaxCX(), BitCount());
		int dst_bytesleft = lpRect->left * BitCount() / 8;

		int rgb_bytesline = nbytes_line(cx, nBits);

		if (bFlipV)
		{
			for (int i = lpRect->top; i < lpRect->bottom; ++i)
			{
				BYTE * dstBits = lpDstBits + dst_bytesline * i + dst_bytesleft;
				BYTE * rgbBits = lpRgbBits + rgb_bytesline * (lpRect->bottom - i - 1);

				memcpy(dstBits, rgbBits, rgb_bytesline);
			}
		}
		else
		{
			for (int i = lpRect->top; i < lpRect->bottom; ++i)
			{
				BYTE * dstBits = lpDstBits + dst_bytesline * i + dst_bytesleft;
				BYTE * rgbBits = lpRgbBits + rgb_bytesline * (i - lpRect->top);

				memcpy(dstBits, rgbBits, rgb_bytesline);
			}
		}

		bOK = TRUE;
	} while (0);

	return bOK;
}

/*******************************************************************************
* FunctionName:
*     SetAreaRgb24Bits
* Description:
*     设置指定区域的 RGB 图像数据。
* Parameter:
*     @[in ] lpRgbBits : RGB 图像数据输入的缓存（针对24位的 RGB 数据，图像数据行宽度紧密存储，未按照 4 字节对齐）。
*     @[in ] lpRect    : 指定设置的区域（矩形区域的 宽度/高度，必须等同于 RGB 图像的 宽度/高度）。
*     @[in ] bFlipV    : 是否按照垂直翻转的方式拷贝。
* ReturnValue:
*     成功，返回 TRUE；失败，返回 FALSE。
*/
BOOL vxMemDC::SetAreaRgb24Bits(LPBYTE lpRgbBits, LPRECT lpRect, BOOL bFlipV /*= FALSE*/)
{
	BOOL bOK = FALSE;

	do 
	{
		if (NULL == m_pBits)
		{
			break;
		}

		// 参数有效验证
		if ((NULL == lpRgbBits) || (BitCount() != 24) || (NULL == lpRect))
		{
			break;
		}

		int cx = lpRect->right - lpRect->left;
		int cy = lpRect->bottom - lpRect->top;

		if ((cx <= 0) || (cx > MaxCX()) || (cy <= 0) || (cy > MaxCY()))
		{
			break;
		}

		// 区域有效判断
		RECT  rectDC  = { 0, 0, MaxCX(), MaxCY() };
		POINT pointLT = { lpRect->left, lpRect->top };
		POINT pointRB = { lpRect->right - 1, lpRect->bottom - 1 };
		if (!PtInRect(&rectDC, pointLT) || !PtInRect(&rectDC, pointRB))
		{
			break;
		}

		// 执行拷贝操作
		LPBYTE lpDstBits  = (LPBYTE)GetBits();
		int dst_bytesline = nbytes_line(MaxCX(), BitCount());
		int dst_bytesleft = lpRect->left * BitCount() / 8;

		int rgb_bytesline = 3 * cx;

		if (bFlipV)
		{
			for (int i = lpRect->top; i < lpRect->bottom; ++i)
			{
				BYTE * dstBits = lpDstBits + dst_bytesline * i + dst_bytesleft;
				BYTE * rgbBits = lpRgbBits + rgb_bytesline * (lpRect->bottom - i - 1);

				memcpy(dstBits, rgbBits, rgb_bytesline);
			}
		}
		else
		{
			for (int i = lpRect->top; i < lpRect->bottom; ++i)
			{
				BYTE * dstBits = lpDstBits + dst_bytesline * i + dst_bytesleft;
				BYTE * rgbBits = lpRgbBits + rgb_bytesline * (i - lpRect->top);

				memcpy(dstBits, rgbBits, rgb_bytesline);
			}
		}

		bOK = TRUE;
	} while (0);

	return bOK;
}

/******************************************************************************/
/**
* @brief
* <pre>
*   访问类型 : public 
*   功能描述 : 绘制 RGB 数据图像，要注意，输入的 RGB 数据，像素位数要与初始化位数一致。
* </pre>
* 
* @param [in ] lpRgbBits : RGB 图像数据输入的缓存。
* @param [in ] nBits     : RGB 图像像素位数（24 或 32）。
* @param [in ] width     : 图像宽度。
* @param [in ] height    : 图像高度。
* @param [in ] lpSrcRect : 图像源矩形区域（传参指针若为 NULL，则取图像源的全区域）。
* @param [in ] lpDstRect : 目标矩形区域（传参指针若为 NULL，则取内存 DC 的全区域）。
* 
* @return BOOL
*         - 成功，返回 TRUE；
*         - 失败，返回 FALSE。
*/
BOOL vxMemDC::DrawRgbBits(LPBYTE lpRgbBits, int nBits, int width, int height, LPRECT lpSrcRect, LPRECT lpDstRect)
{
	BOOL bRet = FALSE;

	do 
	{
		//======================================

		// 初始化判断
		if (NULL == m_hDC)
		{
			break;
		}

		// 输入数据有效性判断
		if ((NULL == lpRgbBits) || (nBits != BitCount()) || (width <= 0) || (0 == height))
		{
			break;
		}

#if 1
		DIBSECTION dibSection;
		if (0 == GetObject((HANDLE)m_hBitmap, sizeof(DIBSECTION), &dibSection))
		{
			break;
		}
#endif
		//======================================

		// 构建绘图的图像信息
		BITMAPINFO biRGB;
		memset(&biRGB, 0, sizeof(BITMAPINFO));
		biRGB.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
		biRGB.bmiHeader.biWidth       = width;
		biRGB.bmiHeader.biHeight      = height;
		biRGB.bmiHeader.biSizeImage   = 0;
		biRGB.bmiHeader.biPlanes      = 1;
		biRGB.bmiHeader.biBitCount    = nBits;
		biRGB.bmiHeader.biCompression = BI_RGB;

		// 目标绘图坐标
		int xDst  = (NULL == lpDstRect) ? 0       : lpDstRect->left;
		int yDst  = (NULL == lpDstRect) ? 0       : lpDstRect->top;
		int dxDst = (NULL == lpDstRect) ? MaxCX() : (lpDstRect->right - lpDstRect->left);
		int dyDst = (NULL == lpDstRect) ? MaxCY() : (lpDstRect->bottom - lpDstRect->top);

		// 源图像坐标
		int xSrc  = (NULL == lpSrcRect) ? 0      : lpSrcRect->left;
		int ySrc  = (NULL == lpSrcRect) ? 0      : lpSrcRect->top;
		int dxSrc = (NULL == lpSrcRect) ? width  : (lpSrcRect->right - lpSrcRect->left);
		int dySrc = (NULL == lpSrcRect) ? (std::abs)(height) : (lpSrcRect->bottom - lpSrcRect->top);

		//======================================

		int nSwapBltMode = SetStretchBltMode(m_hDC, COLORONCOLOR);

		bRet = (GDI_ERROR != StretchDIBits(m_hDC, xDst, yDst, dxDst, dyDst, xSrc, ySrc, dxSrc, dySrc, lpRgbBits, &biRGB, DIB_RGB_COLORS, SRCCOPY));

		SetStretchBltMode(m_hDC, nSwapBltMode);

		//======================================

	} while (0);

	return bRet;
}

////////////////////////////////////////////////////////////////////////////////
// vxPaintDC

//==============================================================================

// 
// constructor/destructor
// 


vxPaintDC::vxPaintDC(HDC& hDC, BOOL bIsDrawOnGlass /* = TRUE */)
		: m_hDC(hDC)
		, m_bIsDrawOnGlass(bIsDrawOnGlass)
{

}

vxPaintDC::~vxPaintDC()
{

}

//==============================================================================

// 
// self invoking methods
// 

void vxPaintDC::_FillGradient(const LPRECT lpRect, COLORREF colorStart, COLORREF colorFinish,
			BOOL bHorz /* = TRUE */, int nStartFlatPercentage /* = 0 */, int nEndFlatPercentage /* = 0 */)
{
	if (NULL == lpRect)
		return;

	if (colorStart == colorFinish)
	{
		HBRUSH hbr = ::CreateSolidBrush(colorStart);
		X_ASSERT(NULL != hbr);
		::FillRect(m_hDC, lpRect, hbr);
		::DeleteObject((HGDIOBJ)hbr);
		return;
	}

	if (nStartFlatPercentage > 0)
	{
		X_ASSERT(nStartFlatPercentage <= 100);

		if (bHorz)
		{
			RECT rectTop;
			memcpy(&rectTop, lpRect, sizeof(RECT));
			rectTop.bottom = rectTop.top + (rectTop.bottom - rectTop.top) * nStartFlatPercentage / 100;
			lpRect->top = rectTop.bottom;

			HBRUSH hbr = ::CreateSolidBrush(colorFinish);
			X_ASSERT(NULL != hbr);
			::FillRect(m_hDC, &rectTop, hbr);
			::DeleteObject((HGDIOBJ)hbr);
		}
		else
		{
			RECT rectLeft;
			memcpy(&rectLeft, lpRect, sizeof(RECT));
			rectLeft.right = rectLeft.left + (rectLeft.right - rectLeft.left) * nStartFlatPercentage / 100;
			lpRect->left = rectLeft.right;

			HBRUSH hbr = ::CreateSolidBrush(colorStart);
			X_ASSERT(NULL != hbr);
			::FillRect(m_hDC, &rectLeft, hbr);
			::DeleteObject((HGDIOBJ)hbr);
		}
	}

	if (nEndFlatPercentage > 0)
	{
		X_ASSERT(nEndFlatPercentage <= 100);

		if (bHorz)
		{
			RECT rectBottom;
			memcpy(&rectBottom, lpRect, sizeof(RECT));
			rectBottom.top = rectBottom.bottom - (rectBottom.bottom - rectBottom.top) * nEndFlatPercentage / 100;
			lpRect->bottom = rectBottom.top;

			HBRUSH hbr = ::CreateSolidBrush(colorStart);
			X_ASSERT(NULL != hbr);
			::FillRect(m_hDC, &rectBottom, hbr);
			::DeleteObject((HGDIOBJ)hbr);
		}
		else
		{
			RECT rectRight;
			memcpy(&rectRight, lpRect, sizeof(RECT));
			rectRight.left = rectRight.right - (rectRight.right - rectRight.left) * nEndFlatPercentage / 100;
			lpRect->right = rectRight.left;

			HBRUSH hbr = ::CreateSolidBrush(colorFinish);
			X_ASSERT(NULL != hbr);
			::FillRect(m_hDC, &rectRight, hbr);
			::DeleteObject((HGDIOBJ)hbr);
		}
	}

	if (nEndFlatPercentage + nStartFlatPercentage > 100)
	{
		X_ASSERT(FALSE);
		return;
	}

	// this will make 2^6 = 64 fountain steps
	int nShift = 6;
	int nSteps = 1 << nShift;

	for (int i = 0; i < nSteps; i++)
	{
		// do a little alpha blending
		BYTE bR = (BYTE)((GetRValue(colorStart) * (nSteps - i) + GetRValue(colorFinish) * i) >> nShift);
		BYTE bG = (BYTE)((GetGValue(colorStart) * (nSteps - i) + GetGValue(colorFinish) * i) >> nShift);
		BYTE bB = (BYTE)((GetBValue(colorStart) * (nSteps - i) + GetBValue(colorFinish) * i) >> nShift);

		HBRUSH hbr = ::CreateSolidBrush(RGB(bR, bG, bB));

		// then paint with the resulting color
		RECT r2;
		memcpy(&r2, lpRect, sizeof(RECT));
		if (bHorz)
		{
			r2.bottom = lpRect->bottom - ((i * (lpRect->bottom - lpRect->top)) >> nShift);
			r2.top = lpRect->bottom - (((i + 1) * (lpRect->bottom - lpRect->top)) >> nShift);
			if ((r2.bottom - r2.top) > 0)
			{
				::FillRect(m_hDC, &r2, hbr);
			}
		}
		else
		{
			r2.left = lpRect->left + ((i * (lpRect->right - lpRect->left)) >> nShift);
			r2.right = lpRect->left + (((i + 1) * (lpRect->right - lpRect->left)) >> nShift);
			if ((r2.right - r2.left) > 0)
			{
				::FillRect(m_hDC, &r2, hbr);
			}
		}

		::DeleteObject((HGDIOBJ)hbr);
	}
}

//==============================================================================

// 
// public interfaces
// 

void vxPaintDC::FillGradient(const LPRECT lpRect, COLORREF colorStart, COLORREF colorFinish,
				BOOL bHorz /* = TRUE */, int nStartFlatPercentage /* = 0 */, int nEndFlatPercentage /* = 0 */)
{
	if (NULL == lpRect)
		return;

	if (!m_bIsDrawOnGlass)
	{
		_FillGradient(lpRect, colorStart, colorFinish, bHorz, nStartFlatPercentage, nEndFlatPercentage);
	}
	else
	{
		RECT rt;
		memcpy(&rt, lpRect, sizeof(RECT));

		SIZE size;
		size.cx = abs(rt.right - rt.left);
		size.cy = abs(rt.bottom - rt.top);
		if (size.cx == 0 || size.cy == 0)
		{
			return;
		}

		//--------------------------------------------
		// Copy screen content into the memory bitmap:
		//--------------------------------------------
		vxMemDC dcMem(m_hDC, size.cx, size.cy);

		COLORREF* pBits;
		HBITMAP hmbpDib = vxCreateBitmap(size.cx, size.cy, 32, (LPVOID*)&pBits);

		if (hmbpDib == NULL || pBits == NULL)
		{
			X_ASSERT(FALSE);
			return;
		}

		::SelectObject(dcMem, (HGDIOBJ)hmbpDib);

		HDC hMemDC = dcMem;
		vxPaintDC dm(hMemDC);
		RECT rcPaint;
		rcPaint.left = 0; rcPaint.top = 0; rcPaint.right = size.cx; rcPaint.bottom = size.cy;
		dm._FillGradient(&rcPaint, colorStart, colorFinish, bHorz, nStartFlatPercentage, nEndFlatPercentage);

		int sizeImage = size.cx * size.cy;
		for (int i = 0; i < sizeImage; i++)
		{
			*pBits |= 0xFF000000;
			pBits++;
		}

		//--------------------------------
		// Copy bitmap back to the screen:
		//--------------------------------
		::BitBlt(m_hDC, rt.left, rt.top, size.cx, size.cy, dcMem, 0, 0, SRCCOPY);

		::DeleteObject(hmbpDib);
	}
}

void vxPaintDC::FillGradient2(const LPRECT lpRect, COLORREF colorStart, COLORREF colorFinish, int nAngle /* = 0 */)
{
	if (NULL == lpRect)
	{
		return;
	}

	if (((lpRect->right - lpRect->left) <= 0) || ((lpRect->bottom - lpRect->top) <= 0))
	{
		return;
	}

	if (colorStart == colorFinish)
	{
		HBRUSH hbr = ::CreateSolidBrush(colorStart);
		X_ASSERT(NULL != hbr);
		::FillRect(m_hDC, lpRect, hbr);
		::DeleteObject((HGDIOBJ)hbr);
		return;
	}

	//----------------------
	// Process simple cases:
	//----------------------
	switch (nAngle)
	{
	case 0:
	case 360:
		FillGradient(lpRect, colorStart, colorFinish, FALSE);
		return;

	case 90:
		FillGradient(lpRect, colorStart, colorFinish, TRUE);
		return;

	case 180:
		FillGradient(lpRect, colorFinish, colorStart, FALSE);
		return;

	case 270:
		FillGradient(lpRect, colorFinish, colorStart, TRUE);
		return;
	}

	//--------------------------------------------
	// Copy screen content into the memory bitmap:
	//--------------------------------------------
	vxMemDC dcMem(m_hDC, lpRect->right - lpRect->left, lpRect->bottom - lpRect->top);

	HPEN hOldPen = (HPEN)::SelectObject(dcMem, (HGDIOBJ)::GetStockObject(NULL_PEN));

	int nShift = 6;
	int nSteps = 1 << nShift;

	const double fAngle = PI * (nAngle + 180) / 180;
	const int nOffset = (int) (cos(fAngle) * (lpRect->bottom - lpRect->top));
	const int nTotalWidth = (lpRect->right - lpRect->left) + abs(nOffset);

	const int xStart = nOffset > 0 ? - nOffset : 0;

	for (int i = 0; i < nSteps; i++)
	{
		// do a little alpha blending
		BYTE bR = (BYTE)((GetRValue(colorStart) * (nSteps - i) + GetRValue(colorFinish) * i) >> nShift);
		BYTE bG = (BYTE)((GetGValue(colorStart) * (nSteps - i) + GetGValue(colorFinish) * i) >> nShift);
		BYTE bB = (BYTE)((GetBValue(colorStart) * (nSteps - i) + GetBValue(colorFinish) * i) >> nShift);

		HBRUSH hbr = ::CreateSolidBrush(RGB(bR, bG, bB));

		int x11 = xStart + ((i * nTotalWidth) >> nShift);
		int x12 = xStart + (((i + 1) * nTotalWidth) >> nShift);

		if (x11 == x12)
		{
			continue;
		}

		int x21 = x11 + nOffset;
		int x22 = x21 + (x12 - x11);

		POINT points[4];
		points[0].x = x11;
		points[0].y = 0;
		points[1].x = x12;
		points[1].y = 0;
		points[2].x = x22;
		points[2].y = (lpRect->bottom - lpRect->top);
		points[3].x = x21;
		points[3].y = (lpRect->bottom - lpRect->top);

		HBRUSH hOldBrush = (HBRUSH)::SelectObject(dcMem, (HGDIOBJ)hbr);
		::Polygon(dcMem, points, 4);
		::SelectObject(dcMem, (HGDIOBJ)hOldBrush);

		::DeleteObject((HGDIOBJ)hbr);
	}

	::SelectObject(dcMem, hOldPen);

	//--------------------------------
	// Copy bitmap back to the screen:
	//--------------------------------
	::BitBlt(m_hDC, lpRect->left, lpRect->top, (lpRect->right - lpRect->left), (lpRect->bottom - lpRect->top), dcMem, 0, 0, SRCCOPY);
}

////////////////////////////////////////////////////////////////////////////////
// vxRgbPainter

//==============================================================================

// 
// vxRgbPainter constructor/destructor
// 

vxRgbPainter::vxRgbPainter(void)
{
	m_hDrawDIB = NULL;
	memset(&m_bmpInfo, 0, sizeof(BITMAPINFO));
}

vxRgbPainter::~vxRgbPainter(void)
{
	Release();
}

//==============================================================================

// 
// vxRgbPainter public interfaces
// 

/*******************************************************************************
* FunctionName:
*     Initial
* Description:
*     对象初始化接口。
* Parameter:
*     @[in ] cx: 设置的绘图区域最大宽度。
*     @[in ] cy: 设置的绘图区域最大高度。
*     @[in ] nBits: 绘图像素比特位数（仅支持 24位 和 32位）。
* ReturnValue:
*     成功，返回 TRUE；失败，返回 FALSE 。
*/
BOOL vxRgbPainter::Initial(int cx, int cy, int nBits /*= 24*/)
{
	BOOL bRet = FALSE;

	do 
	{
		// 初始化内存 DC
		if (!m_hMemDC.Initial(NULL, cx, cy, nBits))
			break;

		// 设置位图头结构信息
		memset(&m_bmpInfo, 0, sizeof(BITMAPINFO));
		m_bmpInfo.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
		m_bmpInfo.bmiHeader.biWidth         = cx;
		m_bmpInfo.bmiHeader.biHeight        = cy;
		m_bmpInfo.bmiHeader.biPlanes        = 1;
		m_bmpInfo.bmiHeader.biBitCount      = nBits;
		m_bmpInfo.bmiHeader.biCompression   = BI_RGB;
		m_bmpInfo.bmiHeader.biSizeImage     = 0;
		m_bmpInfo.bmiHeader.biXPelsPerMeter = 0;
		m_bmpInfo.bmiHeader.biYPelsPerMeter = 0;
		m_bmpInfo.bmiHeader.biClrUsed       = 0;
		m_bmpInfo.bmiHeader.biClrImportant  = 0;

		// 使用 VFW 的 API 绘制 RGB 数据
		m_hDrawDIB = ::DrawDibOpen();
		if (NULL == m_hDrawDIB)
			break;

		bRet = TRUE;

	} while (0);

	return bRet;
}

/*******************************************************************************
* FunctionName:
*     Release
* Description:
*     对象数据释放接口。
*/
void vxRgbPainter::Release(void)
{
	if (NULL != m_hDrawDIB)
	{
		::DrawDibEnd(m_hDrawDIB);
		::DrawDibClose(m_hDrawDIB);

		m_hDrawDIB = NULL;
	}

	m_hMemDC.Release();
}

/*******************************************************************************
* FunctionName:
*     EraseBkgnd
* Description:
*     使用指定背景色擦除背景。
* Parameter:
*     @[in ] clrBkgnd: 指定的背景色。
*     @[in ] lpRect: 擦除区域（传参指针若为 NULL，则目标区域为全区域）。
* ReturnValue:
*     成功，返回 TRUE；失败，返回 FALSE 。
*/
BOOL vxRgbPainter::EraseBkgnd(COLORREF clrBkgnd, LPRECT lpRect)
{
	BOOL bRet = FALSE;

	do 
	{
		// 初始化判断
		if (!IsInitial())
		{
			break;
		}

		RECT rect;
		rect.left   = (NULL == lpRect) ? 0 : lpRect->left;
		rect.top    = (NULL == lpRect) ? 0 : lpRect->top;
		rect.right  = (NULL == lpRect) ? m_hMemDC.MaxCX() : lpRect->right;
		rect.bottom = (NULL == lpRect) ? m_hMemDC.MaxCY() : lpRect->bottom;

		COLORREF clrSwapBkgnd = ::SetBkColor(HandleDC(), clrBkgnd);
		::ExtTextOut(HandleDC(), 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
		::SetBkColor(HandleDC(), clrSwapBkgnd);

		bRet = TRUE;

	} while (0);

	return bRet;
}

/*******************************************************************************
* FunctionName:
*     DrawRgbImage
* Description:
*     绘制 RGB 数据图像，要注意，输入的 RGB 数据，像素位数要与初始化位数一致。
* Parameter:
*     @[in ] lpRgbData: RGB 图像数据。
*     @[in ] width: 图像宽度。
*     @[in ] height: 图像高度。
*     @[in ] lpSrcRect: 图像源矩形区域（传参指针若为 NULL，则取图像源的全区域）。
*     @[in ] lpDstRect: 目标矩形区域（传参指针若为 NULL，则取内存 DC 的全区域）。
* ReturnValue:
*     成功，返回 TRUE；失败，返回 FALSE 。
*/
BOOL vxRgbPainter::DrawRgbImage(LPBYTE lpRgbData, int width, int height,
								LPRECT lpSrcRect, LPRECT lpDstRect)
{
	BOOL bRet = FALSE;

	do 
	{
		// 初始化判断
		if (!IsInitial())
		{
			break;
		}

		// 输入数据有效性判断
		if (NULL == lpRgbData)
		{
			break;
		}

		// 构建绘图的图像信息
		BITMAPINFO biRGB;
		memcpy(&biRGB, &m_bmpInfo, sizeof(BITMAPINFO));
		biRGB.bmiHeader.biWidth  = width;
		biRGB.bmiHeader.biHeight = height;

		// 绘图流程
		HDC hMemDC = HandleDC();
		bRet = ::DrawDibBegin(m_hDrawDIB, hMemDC, width, height, &biRGB.bmiHeader, width, height, DDF_BUFFER);

		// 目标绘图坐标
		int xDst  = (NULL == lpDstRect) ? 0           : lpDstRect->left;
		int yDst  = (NULL == lpDstRect) ? 0           : lpDstRect->top;
		int dxDst = (NULL == lpDstRect) ? GetWidth()  : (lpDstRect->right - lpDstRect->left);
		int dyDst = (NULL == lpDstRect) ? GetHeight() : (lpDstRect->bottom - lpDstRect->top);
		// 源图像坐标
		int xSrc  = (NULL == lpSrcRect) ? 0      : lpSrcRect->left;
		int ySrc  = (NULL == lpSrcRect) ? 0      : lpSrcRect->top;
		int dxSrc = (NULL == lpSrcRect) ? width  : (lpSrcRect->right - lpSrcRect->left);
		int dySrc = (NULL == lpSrcRect) ? (std::abs)(height) : (lpSrcRect->bottom - lpSrcRect->top);

		bRet &= ::DrawDibDraw(m_hDrawDIB, hMemDC, xDst, yDst, dxDst, dyDst,
							&biRGB.bmiHeader, lpRgbData, xSrc, ySrc, dxSrc, dySrc, DDF_BUFFER);
		if (!bRet)
		{
			int nError = GetLastError();
		}

		bRet &= ::DrawDibEnd(m_hDrawDIB);

	} while (0);

	return bRet;
}

/*******************************************************************************
* FunctionName:
*     DrawRgbImageEx
* Description:
*     绘制 RGB 数据图像，要注意，输入的 RGB 数据，像素位数要与初始化位数一致。
* Parameter:
*     @[in ] lpRgbData: RGB 图像数据。
*     @[in ] width: 图像宽度。
*     @[in ] height: 图像高度。
*     @[in ] lpSrcRect: 图像源矩形区域（传参指针若为 NULL，则取图像源的全区域）。
*     @[in ] lpDstRect: 目标矩形区域（传参指针若为 NULL，则取内存 DC 的全区域）。
* ReturnValue:
*     成功，返回 TRUE；失败，返回 FALSE 。
*/
BOOL vxRgbPainter::DrawRgbImageEx(LPBYTE lpRgbData, int width, int height, LPRECT lpSrcRect, LPRECT lpDstRect)
{
	BOOL bRet = FALSE;

	do 
	{
		// 初始化判断
		if (!IsInitial())
		{
			break;
		}

		// 输入数据有效性判断
		if (NULL == lpRgbData)
		{
			break;
		}

		// 构建绘图的图像信息
		BITMAPINFO biRGB;
		memcpy(&biRGB, &m_bmpInfo, sizeof(BITMAPINFO));
		biRGB.bmiHeader.biWidth  = width;
		biRGB.bmiHeader.biHeight = height;

		// 目标绘图坐标
		int xDst  = (NULL == lpDstRect) ? 0           : lpDstRect->left;
		int yDst  = (NULL == lpDstRect) ? 0           : lpDstRect->top;
		int dxDst = (NULL == lpDstRect) ? GetWidth()  : (lpDstRect->right - lpDstRect->left);
		int dyDst = (NULL == lpDstRect) ? GetHeight() : (lpDstRect->bottom - lpDstRect->top);
		// 源图像坐标
		int xSrc  = (NULL == lpSrcRect) ? 0      : lpSrcRect->left;
		int ySrc  = (NULL == lpSrcRect) ? 0      : lpSrcRect->top;
		int dxSrc = (NULL == lpSrcRect) ? width  : (lpSrcRect->right - lpSrcRect->left);
		int dySrc = (NULL == lpSrcRect) ? (std::abs)(height) : (lpSrcRect->bottom - lpSrcRect->top);

		StretchDIB(&m_bmpInfo.bmiHeader, GetBits(), xDst, yDst, dxDst, dyDst, &biRGB.bmiHeader, lpRgbData, xSrc, ySrc, dxSrc, dySrc);
		bRet = TRUE;

	} while (0);

	return bRet;
}

/*******************************************************************************
* FunctionName:
*     DrawTextInfo
* Description:
*     绘制 文本信息 。
* Parameter:
*     @[in ] szTextInfo: 文本信息。
*     @[in ] clrText: 文本颜色。
*     @[in ] hbrBkgnd: 背景画刷（若为 NULL，则背景透明）。
*     @[in ] lpDstRect: 目标矩形区域。
* ReturnValue:
*     成功，返回 TRUE；失败，返回 FALSE 。
*/
BOOL vxRgbPainter::DrawTextInfo(LPCTSTR szTextInfo, COLORREF clrText, HBRUSH hbrBkgnd, LPRECT lpDstRect)
{
	BOOL bRet = FALSE;

	do 
	{
		// 初始化判断
		if (!IsInitial())
		{
			break;
		}

		// 输入数据有效性判断
		if ((NULL == szTextInfo) || (_tcslen(szTextInfo) <= 0) || (NULL == lpDstRect))
		{
			break;
		}

		// 绘制文本信息流程
		HDC hMemDC = HandleDC();
		BOOL bTransparent = (NULL == hbrBkgnd);
		HGDIOBJ  hOldBrush  = SelectObject(hMemDC, bTransparent ? GetStockObject(NULL_BRUSH) : (HGDIOBJ)hbrBkgnd);
		int      nOldBkMode = bTransparent ? SetBkMode(hMemDC, TRANSPARENT) : GetBkMode(hMemDC);
		COLORREF clrOldText = SetTextColor(hMemDC, clrText);

		int cchText = (int)_tcslen(szTextInfo);
		DrawText(hMemDC, szTextInfo, cchText, lpDstRect, DT_LEFT | DT_CALCRECT);
		DrawText(hMemDC, szTextInfo, cchText, lpDstRect, DT_LEFT);

		SelectObject(hMemDC, hOldBrush);
		SetBkMode(hMemDC, nOldBkMode);
		SetTextColor(hMemDC, clrOldText);

		bRet = TRUE;
	} while (0);

	return bRet;
}

/*******************************************************************************
* FunctionName:
*     SaveToFile
* Description:
*     保存绘制的图像至文件。
* Parameter:
*     @[in ] szFile: 文件名。
*     @[in ] fileType: 保存的文件类型。
* ReturnValue:
*     成功，返回 TRUE；失败，返回 FALSE 。
*/
BOOL vxRgbPainter::SaveToFile(LPCTSTR szFile, SAVE_TYPE fileType)
{
	BOOL bRet = FALSE;

	do 
	{
		// 初始化判断
		if (!IsInitial())
		{
			break;
		}

		// 使用 CImage 类的文件保存接口
		CImage img;
		img.Attach(m_hMemDC.Detach());

		HRESULT hr = E_FAIL;
		switch (fileType)
		{
		case STF_RGB: hr = img.Save(szFile, Gdiplus::ImageFormatBMP);  break;
		case STF_PNG: hr = img.Save(szFile, Gdiplus::ImageFormatPNG);  break;
		case STF_JPG: hr = img.Save(szFile, Gdiplus::ImageFormatJPEG); break;
		default: break;
		}

		m_hMemDC.Attach(img.Detach());

		bRet = SUCCEEDED(hr);
	} while (0);

	return bRet;
}

////////////////////////////////////////////////////////////////////////////////
// vxVfwWndPainter

//==============================================================================

// 
// vxVfwWndPainter constructor/destructor
// 

vxVfwWndPainter::vxVfwWndPainter(void)
			: m_hPlayWnd(NULL)
			, m_lpBits(NULL)
			, m_hBitmap(NULL)
			, m_hDrawDIB(NULL)
			, m_hPaintDC(NULL)
			, m_hOldBrush(NULL)
			, m_bFixedPaint(TRUE)
			, m_funcPaintCBK(NULL)
			, m_lParamCBK(0)
{
	memset(&m_bmpInfo, 0, sizeof(BITMAPINFOHEADER));
}

vxVfwWndPainter::~vxVfwWndPainter(void)
{
	if (IsBegin())
	{
		EndPaint();
	}
}

//==============================================================================

// 
// vxVfwWndPainter public interfaces
// 

/*******************************************************************************
* FunctionName:
*     BeginPaint
* Description:
*     开启窗口的 RGB 图像绘图显示。
* Parameter:
*     @[in ] hPlayWnd: 图像显示的目标窗口句柄。
*     @[in ] lwidth: 初始化图像的缓存宽度。
*     @[in ] lheight: 初始化图像的缓存高度。
*     @[in ] bFixedSize: （视频流播放时前后）绘制的 RGB 图像是否使用确定的尺寸。
*     @[in ] nBits: 绘图像素比特位数（仅支持 24位 和 32位）。
* ReturnValue:
*     成功，返回 TRUE；失败，返回 FALSE。
*/
BOOL vxVfwWndPainter::BeginPaint(HWND hPlayWnd, int lwidth, int lheight, BOOL bFixedSize, int nBits)
{
	BOOL bInit = FALSE;

	do 
	{
		// 参数有效判断
		if ((NULL == hPlayWnd) || !::IsWindow(hPlayWnd) || (lwidth <= 1) || (lheight < 1) || ((24 != nBits) && (32 != nBits)))
		{
			break;
		}

		m_hPlayWnd = hPlayWnd;

		m_bmpInfo.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
		m_bmpInfo.bmiHeader.biWidth       = lwidth;
		m_bmpInfo.bmiHeader.biHeight      = lheight;
		m_bmpInfo.bmiHeader.biSizeImage   = 0;
		m_bmpInfo.bmiHeader.biPlanes      = 1;
		m_bmpInfo.bmiHeader.biBitCount    = nBits;
		m_bmpInfo.bmiHeader.biCompression = BI_RGB;

		m_hBitmap = ::CreateDIBSection(NULL, &m_bmpInfo, DIB_RGB_COLORS, &m_lpBits, NULL, 0);
		if (NULL == m_hBitmap) break;

		m_hDrawDIB = ::DrawDibOpen();
		if (NULL == m_hDrawDIB)
		{
			break;
		}

		m_hPaintDC = ::GetDC(m_hPlayWnd);
		if (NULL == m_hPaintDC)
		{
			break;
		}

		m_hOldBrush = (HBRUSH)::SelectObject(m_hPaintDC, GetStockObject(NULL_BRUSH));

		m_bFixedPaint = bFixedSize;
		if (m_bFixedPaint)
		{
			bInit = ::DrawDibBegin(m_hDrawDIB, m_hPaintDC, lwidth, lheight, &m_bmpInfo.bmiHeader, lwidth, lheight, DDF_SAME_HDC);
			if (!bInit)
			{
				int err = GetLastError();
				break;
			}
		}

		bInit = TRUE;

	} while (0);

	if (!bInit)
	{
		EndPaint();
	}

	return bInit;
}

/*******************************************************************************
* FunctionName:
*     DrawRGB
* Description:
*     绘制 RGB 数据图像，要注意，输入的 RGB 数据，像素位数要与初始化位数一致。
* Parameter:
*     @[in ] lpRgbData: RGB 图像数据。
*     @[in ] width: 图像宽度。
*     @[in ] height: 图像高度。
*     @[in ] lpSrcRect: 图像源矩形区域（传参指针若为 NULL，则取图像源的全区域）。
*     @[in ] lpDstRect: 目标矩形区域（传参指针若为 NULL，则取内存 DC 的全区域）。
* ReturnValue:
*     成功，返回 TRUE；失败，返回 FALSE 。
*/
BOOL vxVfwWndPainter::DrawRGB(LPBYTE lpRgbData, int width, int height, LPRECT lpSrcRect, LPRECT lpDstRect)
{
	BOOL bDrawProc = FALSE;

	do 
	{
		// 操作是否开启判断
		if (!IsBegin())
		{
			break;
		}

		// 输入数据有效性判断
		if (NULL == lpRgbData)
		{
			break;
		}

		if (m_bFixedPaint)
		{
			if ((MaxCX() != width) || (MaxCY() != height))
			{
				break;
			}
		}

		// 构建绘图的图像信息
		BITMAPINFO biRGB;
		memcpy(&biRGB, &m_bmpInfo, sizeof(BITMAPINFO));
		biRGB.bmiHeader.biWidth  = width;
		biRGB.bmiHeader.biHeight = height;

		// 绘图流程
		if (!m_bFixedPaint)
		{
			bDrawProc = ::DrawDibBegin(m_hDrawDIB, m_hPaintDC, width, height, &biRGB.bmiHeader, width, height, DDF_BUFFER);
		}

		// 目标绘图坐标
		int xDst  = (NULL == lpDstRect) ? 0       : lpDstRect->left;
		int yDst  = (NULL == lpDstRect) ? 0       : lpDstRect->top;
		int dxDst = (NULL == lpDstRect) ? MaxCX() : (lpDstRect->right - lpDstRect->left);
		int dyDst = (NULL == lpDstRect) ? MaxCY() : (lpDstRect->bottom - lpDstRect->top);
		// 源图像坐标
		int xSrc  = (NULL == lpSrcRect) ? 0      : lpSrcRect->left;
		int ySrc  = (NULL == lpSrcRect) ? 0      : lpSrcRect->top;
		int dxSrc = (NULL == lpSrcRect) ? width  : (lpSrcRect->right - lpSrcRect->left);
		int dySrc = (NULL == lpSrcRect) ? height : (lpSrcRect->bottom - lpSrcRect->top);

		bDrawProc = ::DrawDibDraw(m_hDrawDIB, m_hPaintDC, xDst, yDst, dxDst, dyDst, &biRGB.bmiHeader, lpRgbData, xSrc, ySrc, dxSrc, dySrc, DDF_SAME_HDC);
		if (!bDrawProc)
		{
			int err = GetLastError();
		}

		if (!m_bFixedPaint)
		{
			bDrawProc = ::DrawDibEnd(m_hDrawDIB);
		}

		if (NULL != m_funcPaintCBK)
		{
			RECT rcPaint = { xDst, yDst, xDst + dxDst, yDst + dyDst };
			m_funcPaintCBK(m_hPaintDC, &rcPaint, m_lParamCBK);
		}

		bDrawProc = TRUE;

	} while (0);

	return bDrawProc;
}

/*******************************************************************************
* FunctionName:
*     EndPaint
* Description:
*     结束 RGB 图像的绘制显示。
*/
void vxVfwWndPainter::EndPaint(void)
{
	if (NULL != m_hDrawDIB)
	{
		::DrawDibEnd(m_hDrawDIB);
		::DrawDibClose(m_hDrawDIB);
		m_hDrawDIB = NULL;
	}
	if (NULL != m_hPaintDC)
	{
		::SelectObject(m_hPaintDC, (HGDIOBJ)m_hOldBrush);
		::ReleaseDC(m_hPlayWnd, m_hPaintDC);
		m_hPaintDC = NULL;
	}
	if (NULL != m_hBitmap)
	{
		::DeleteObject((HGDIOBJ)m_hBitmap);
		m_hBitmap = NULL;
	}
	if ((NULL != m_hPlayWnd) && ::IsWindow(m_hPlayWnd))
	{
		InvalidateRect(m_hPlayWnd, NULL, TRUE);
		m_hPlayWnd = NULL;
	}

	memset(&m_bmpInfo, 0, sizeof(BITMAPINFOHEADER));

	m_lpBits = NULL;
	m_hOldBrush = NULL;

	m_bFixedPaint = FALSE;
}


