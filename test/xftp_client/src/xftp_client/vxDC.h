/*
* Copyright (c) 2012, 百年千岁
* All rights reserved.
* 
* 文件名称：vxDC.h
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

#ifndef __VXDC_H__
#define __VXDC_H__

////////////////////////////////////////////////////////////////////////////////

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
HBITMAP vxCreateBitmap(int cx, int cy, int nBits, void ** pBits);

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
void vxDrawRect(HDC hDC, LPRECT lpRect, int cwLine, COLORREF clrLine);

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
void vxFillSolidRect(HDC hDC, LPRECT lpRect, COLORREF clrRect);

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
BOOL vxDrawText(HDC hDC, LPCTSTR szText, COLORREF clrText, HBRUSH hbrBkgnd, LPRECT lpDstRect, DWORD dwDTAligned);

////////////////////////////////////////////////////////////////////////////////
// vxMemDC

class vxMemDC
{
	// constructor/destructor
public:
	vxMemDC(HDC hCompatibleDC, int cx, int cy, int nBits = 32);
	vxMemDC(void);
	virtual ~vxMemDC(void);

private:
	vxMemDC(const vxMemDC & obj);

	// class properties
public:
	typedef enum emSaveType        ///< 图像保存格式
	{
		STF_RGB  = 0,     ///< RGB 位图格式
		STF_PNG  = 1,     ///< PNG 图像格式
		STF_JPG  = 2,     ///< JPG 图像格式
	} emSaveType;

	// common invoking
public:
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
	static BOOL SaveToFile(vxMemDC & hMemDC, LPCTSTR szFile, emSaveType fileType);

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
	static BOOL Assign(vxMemDC & hMemDC, int max_cx, int max_cy);

	// overrides
private:
	vxMemDC & operator = (const vxMemDC & obj);
	vxMemDC & operator = (int nFlags);

	// public interfaces
public:
	/*******************************************************************************
	* Description:
	*     数据初始化接口。
	*/
	BOOL Initial(HDC hCompatibleDC, int cx, int cy, int nBits = 32);

	/*******************************************************************************
	* Description:
	*     数据释放。
	*/
	BOOL Release(void);

	/*******************************************************************************
	* Description:
	*     内存DC的尺寸最大宽度。
	*/
	int MaxCX(void) const;

	/*******************************************************************************
	* Description:
	*     内存DC的尺寸最大高度。
	*/
	int MaxCY(void) const;

	/*******************************************************************************
	* Description:
	*     附加新的位图句柄至内存 DC 中，返回旧的位图句柄。
	*/
	HBITMAP Attach(HBITMAP hBitmap);

	/*******************************************************************************
	* Description:
	*     分离出内存位图句柄。
	*/
	HBITMAP Detach(void);

	/*******************************************************************************
	* Description:
	*     HDC 操作符重载。
	*/
	operator HDC();
	operator HDC() const;

	/*******************************************************************************
	* Description:
	*     RGB数据。
	*/
	inline void * GetBits(void) { return m_pBits; }

	/*******************************************************************************
	* Description:
	*     像素位（24或32）。
	*/
	inline int BitCount(void) const { return m_nBitCount; }

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
	BOOL GetAreaRgbBits(LPBYTE lpRgbBits, int nRgbBytes, LPRECT lpRect, BOOL bFlipV = FALSE);

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
	BOOL CopyAreaRgbBits(LPBYTE lpScanBits, int nStride, LPRECT lpRect, BOOL bFlipV = FALSE);

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
	BOOL SetAreaRgbBits(LPBYTE lpRgbBits, int nBits, LPRECT lpRect, BOOL bFlipV = FALSE);

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
	BOOL SetAreaRgb24Bits(LPBYTE lpRgbBits, LPRECT lpRect, BOOL bFlipV = FALSE);

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
	BOOL DrawRgbBits(LPBYTE lpRgbBits, int nBits, int width, int height, LPRECT lpSrcRect, LPRECT lpDstRect);

protected:
	HDC     m_hDC;
	HBITMAP m_hBitmap;
	HBITMAP m_hOldBitmap;
	void  * m_pBits;
	int     m_nBitCount;

	int     m_cxMaxDC;    // 内存DC的尺寸最大宽度
	int     m_cyMaxDC;    // 内存DC的尺寸最大高度

};

////////////////////////////////////////////////////////////////////////////////
// vxPaintDC

class vxPaintDC
{
	// constructor/destructor
public:
	vxPaintDC(HDC& hDC, BOOL bIsDrawOnGlass = TRUE);
	virtual ~vxPaintDC();

	// class property
public:

	// public interfaces
public:
	void FillGradient(const LPRECT lpRect, COLORREF colorStart, COLORREF colorFinish,
					BOOL bHorz = TRUE, int nStartFlatPercentage = 0, int nEndFlatPercentage = 0);

	void FillGradient2(const LPRECT lpRect, COLORREF colorStart, COLORREF colorFinish,
					int nAngle = 0 /* 0 - 360 */);

	// self invoking methods
protected:
	void _FillGradient(const LPRECT lpRect, COLORREF colorStart, COLORREF colorFinish,
					BOOL bHorz = TRUE, int nStartFlatPercentage = 0, int nEndFlatPercentage = 0);

	// class data
protected:
	HDC   & m_hDC;
	BOOL    m_bIsDrawOnGlass;

};

////////////////////////////////////////////////////////////////////////////////
// vxRgbPainter

class vxRgbPainter
{
	// constructor/destructor
public:
	vxRgbPainter(void);
	virtual ~vxRgbPainter(void);

	// class properties
public:
	enum SAVE_TYPE        ///< 图像保存格式
	{
		STF_RGB  = 0,     ///< RGB 位图格式
		STF_PNG  = 1,     ///< PNG 图像格式
		STF_JPG  = 2,     ///< JPG 图像格式
	};

	// public interfaces
public:
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
	BOOL Initial(int cx, int cy, int nBits = 24);

	/*******************************************************************************
	* FunctionName:
	*     Release
	* Description:
	*     对象数据释放接口。
	*/
	void Release(void);

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
	BOOL EraseBkgnd(COLORREF clrBkgnd, LPRECT lpRect);

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
	BOOL DrawRgbImage(LPBYTE lpRgbData, int width, int height, LPRECT lpSrcRect, LPRECT lpDstRect);

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
	BOOL DrawRgbImageEx(LPBYTE lpRgbData, int width, int height, LPRECT lpSrcRect, LPRECT lpDstRect);

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
	BOOL DrawTextInfo(LPCTSTR szTextInfo, COLORREF clrText, HBRUSH hbrBkgnd, LPRECT lpDstRect);

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
	inline BOOL GetAreaRgbBits(LPBYTE lpRgbBits, int nRgbBytes, LPRECT lpRect, BOOL bFlipV = FALSE)
	{
		return m_hMemDC.GetAreaRgbBits(lpRgbBits, nRgbBytes, lpRect, bFlipV);
	}

	/*******************************************************************************
	* FunctionName:
	*     SetAreaRgbBits
	* Description:
	*     设置指定区域的 RGB 图像数据。
	* Parameter:
	*     @[in ] lpRgbBits : RGB 图像数据输入的缓存。
	*     @[in ] nBits     : RGB 图像像素位数（24 或 32）。
	*     @[in ] lpRectDst : 指定设置的区域（矩形区域的 宽度/高度，必须等同于 RGB 图像的 宽度/高度）。
	*     @[in ] bFlipV    : 是否按照垂直翻转的方式拷贝。
	* ReturnValue:
	*     成功，返回 TRUE；失败，返回 FALSE。
	*/
	inline BOOL SetAreaRgbBits(LPBYTE lpRgbBits, int nBits, LPRECT lpRectDst, BOOL bFlipV = FALSE)
	{
		return m_hMemDC.SetAreaRgbBits(lpRgbBits, nBits, lpRectDst, bFlipV);
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
	inline BOOL SetAreaRgb24Bits(LPBYTE lpRgbBits, LPRECT lpRect, BOOL bFlipV = FALSE)
	{
		return m_hMemDC.SetAreaRgb24Bits(lpRgbBits, lpRect, bFlipV);
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
	BOOL SaveToFile(LPCTSTR szFile, SAVE_TYPE fileType);

	/*******************************************************************************
	* FunctionName:
	*     IsInitial
	* Description:
	*     是否初始化。
	*/
	inline BOOL IsInitial(void) const
	{
		return ((NULL != (HDC)m_hMemDC) && (NULL != m_hDrawDIB));
	}

	/*******************************************************************************
	* FunctionName:
	*     HandleDC
	* Description:
	*     对象的内存 DC 句柄。
	*/
	inline HDC HandleDC(void) { return (HDC)m_hMemDC; }

	/*******************************************************************************
	* FunctionName:
	*     MemDC
	* Description:
	*     内存 DC 对象。
	*/
	inline vxMemDC& MemDC(void) { return m_hMemDC; }

	/*******************************************************************************
	* FunctionName:
	*     GetBits
	* Description:
	*     返回内存 DC 对象的像素（RGB数据）存储地址。
	*/
	inline LPBYTE GetBits(void) { return (LPBYTE)m_hMemDC.GetBits(); }

	/*******************************************************************************
	* FunctionName:
	*     GetWidth
	* Description:
	*     返回内存 DC 对象的图像宽度。
	*/
	inline int GetWidth(void) const { return m_hMemDC.MaxCX(); }

	/*******************************************************************************
	* FunctionName:
	*     GetHeight
	* Description:
	*     返回内存 DC 对象的图像高度。
	*/
	inline int GetHeight(void) const { return m_hMemDC.MaxCY(); }

	// class data
protected:
	vxMemDC          m_hMemDC;         ///< 绘图使用的内存 DC
	HANDLE           m_hDrawDIB;       ///< 绘图的 DIB 句柄
	BITMAPINFO       m_bmpInfo;        ///< 位图结构信息

};

////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
* Description:
*     图像显示操作（绘制 RGB 图像）的回调函数类型。
* Parameter:
*     @[in ] hDC: 目标绘图 DC 。
*     @[in ] lpRectPaint: 目标绘制区域。
*     @[in ] lParam: 回调的用户数据。
* ReturnValue:
*     void
*/
typedef void (CALLBACK *RGBPAINT_CALLBACK)(HDC hDC, LPRECT lpRectPaint, LPARAM lParam);

////////////////////////////////////////////////////////////////////////////////
// vxVfwWndPainter

class vxVfwWndPainter
{
	// constructor/destructor
public:
	vxVfwWndPainter(void);
	~vxVfwWndPainter(void);

	// public interfaces
public:
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
	BOOL BeginPaint(HWND hPlayWnd, int lwidth, int lheight, BOOL bFixedSize, int nBits);

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
	BOOL DrawRGB(LPBYTE lpRgbData, int width, int height, LPRECT lpSrcRect, LPRECT lpDstRect);

	/*******************************************************************************
	* FunctionName:
	*     EndPaint
	* Description:
	*     结束 RGB 图像的绘制显示。
	*/
	void EndPaint(void);

	/*******************************************************************************
	* FunctionName:
	*     IsBegin
	* Description:
	*     返回操作开启标识。
	*/
	inline BOOL IsBegin(void) const { return ((NULL != m_hDrawDIB) && ::IsWindow(m_hPlayWnd)); }

	/*******************************************************************************
	* FunctionName:
	*     MaxCX
	* Description:
	*     绘制图像的最大有效宽度值。
	*/
	inline int MaxCX(void) const { return m_bmpInfo.bmiHeader.biWidth;  }

	/*******************************************************************************
	* FunctionName:
	*     MaxCY
	* Description:
	*     绘制图像的最大有效高度值。
	*/
	inline int MaxCY(void) const { return m_bmpInfo.bmiHeader.biHeight; }

	/*******************************************************************************
	* FunctionName:
	*     PaintWnd
	* Description:
	*     绘图显示的目标窗口句柄。
	*/
	inline HWND PaintWnd(void) const { return m_hPlayWnd; }

	/*******************************************************************************
	* FunctionName:
	*     PaintDC
	* Description:
	*     绘图操作的目标 DC 。
	*/
	inline HDC PaintDC(void) const { return m_hPaintDC; }

	/*******************************************************************************
	* FunctionName:
	*     SetPaintCBK
	* Description:
	*     设置绘图显示操作的回调通知函数。
	*/
	inline void SetPaintCBK(RGBPAINT_CALLBACK vfwExtendDrawFunc, LPARAM lParam)
	{
		m_funcPaintCBK = vfwExtendDrawFunc;
		m_lParamCBK    = lParam;
	}

	// class data
protected:
	HWND               m_hPlayWnd;        ///< 绘图显示目标的目标窗口句柄
	BITMAPINFO         m_bmpInfo;         ///< 绘图缓存的 RGB 图像描述信息
	LPVOID             m_lpBits;          ///< 绘图缓存地址
	HBITMAP            m_hBitmap;         ///< HBITMAP 位图操作句柄
	HANDLE             m_hDrawDIB;        ///< DIB 操作句柄
	HDC                m_hPaintDC;        ///< 目标显示 DC 句柄
	HBRUSH             m_hOldBrush;       ///< 保存原窗口 DC 的 HBRUSH 句柄

	BOOL               m_bFixedPaint;     ///< 标识是否按固定图像尺寸开启绘图操作

	RGBPAINT_CALLBACK  m_funcPaintCBK;    ///< 绘图显示操作的回调通知函数
	LPARAM             m_lParamCBK;       ///< 绘图显示操作的回调用户数据

};

////////////////////////////////////////////////////////////////////////////////

#endif //__VXDC_H__


