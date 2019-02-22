/*
* Copyright (c) 2013, Gaaagaa
* All rights reserved.
* 
* 文件名称：vxListCtrl.h
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

#ifndef __VXLISTCTRL_H__
#define __VXLISTCTRL_H__

#include "vxHeaderCtrl.h"

////////////////////////////////////////////////////////////////////////////////
// vxListCtrl

class vxListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(vxListCtrl)

	// constructor/destructor
public:
	vxListCtrl(void);
	virtual ~vxListCtrl(void);

	// class properties
public:
	typedef enum emWndCtrlID
	{
		IDT_TIMER_UPDATE = 0x7FFE,         ///< UI 更新的定时器
	} emWndCtrlID;

protected:
	/**
	 * @struct SortParam
	 * @brief  排序操作的 lParamSort 的指向参数。
	 */
	typedef struct SortParam
	{
		HANDLE   SP_ListCtrl;  ///< 列表控件
		BOOL     SP_Asc;       ///< TRUE，升序操作；FALSE，降序操作
		UINT     SP_Colume;    ///< 排序列序号
		LPARAM   SP_Reserved;  ///< 保留参数
	} SortParam;

	// common invoking
protected:
	/******************************************************************************/
	/**
	* 默认的排序比较函数。
	*/
	static int CALLBACK CompareEntry(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

	// overrides
protected:
	virtual void PreSubclassWindow();
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	// extensible
protected:
	/******************************************************************************/
	/**
	* 控件初始化。
	*/
	virtual BOOL InitListCtrl();

	/******************************************************************************/
	/**
	* 绘制子项勾选框。
	*/
	virtual void DrawItemCheckBox(LPDRAWITEMSTRUCT lpDrawItemStruct);

	/******************************************************************************/
	/**
	* 绘制子项图标。
	*/
	virtual void DrawItemIcon(LPDRAWITEMSTRUCT lpDrawItemStruct);

	/******************************************************************************/
	/**
	* 绘制子项中的信息。
	*/
	virtual void DrawItemInfo(LPDRAWITEMSTRUCT lpDrawItemStruct);

	/******************************************************************************/
	/**
	* DrawItemInfo() 调用过程中，调用本接口实现子项的内部各个子项的绘制操作。
	*/
	virtual void DrawSubItem(CDC * pDC, int iItem, int iSubItem, COLORREF clrBkgnd);

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
	virtual int vxOnSortCompareItem(LPARAM lParam1, LPARAM lParam2,
									int nColume, BOOL bAsc, LPARAM lReserved);

	// public interfaces
public:
	/******************************************************************************/
	/**
	* 设置行高。
	*/
	BOOL SetRowHeight(int nRowHeight, BOOL bRelayout = TRUE);

	/******************************************************************************/
	/**
	* 设置表头高度最小值。
	*/
	BOOL SetHeadMinHeight(int nMinHeight);

	/******************************************************************************/
	/**
	* 获取子项的勾选框矩形区域；需保证控件具备 LVS_EX_CHECKBOXES 样式。
	*/
	BOOL GetItemCheckBoxRect(int iItem, LPRECT lpRect);

	/******************************************************************************/
	/**
	* 是否可用控件的重绘流程。
	*/
	inline void EnablePaintProc(BOOL bEnable = TRUE) { m_bEnablePaintProc = bEnable; }

	/******************************************************************************/
	/**
	* 是否可进行项排序操作。
	*/
	inline void EnableSortItem(BOOL bEnable = TRUE) { m_bEnableSortItem = bEnable; }

	/******************************************************************************/
	/**
	* 启用/禁用 列表控件表头自动布局功能。
	*/
	void EnableLayoutColumn(BOOL bEnable = TRUE);

	/******************************************************************************/
	/**
	* 是否可自动调整表头布局。
	*/
	inline BOOL IsEnableLayoutColumn(void) const { return m_bEnableLayoutColumn; }

	/******************************************************************************/
	/**
	* 获取键值。
	*/
	HANDLE GetKey(LPCTSTR szKeyName, HANDLE hKeyDefualt = INVALID_HANDLE_VALUE) const;

	/******************************************************************************/
	/**
	* 设置键值。
	*/
	void SetKey(LPCTSTR szKeyName, HANDLE hKey);

	/******************************************************************************/
	/**
	* 重置（清空）所有键值映射。
	*/
	void ResetKeyMap();

	// self invoking methods
protected:
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
	void AutoLayoutColumn(int cx);

	// class data
protected:
	BOOL                          m_bInitial;             ///< 初始化标识
	int                           m_nRowHeight;           ///< 行高
	vxHeaderCtrl                  m_wndHeaderCtrl;        ///< 表头控件
	vxMemDC                       m_memDC;                ///< 重绘使用的内存 DC

	BOOL                          m_bEnablePaintProc;     ///< 是否可用控件的重绘流程

	BOOL                          m_bEnableSortItem;      ///< 是否可进行项排序操作

	BOOL                          m_bEnableLayoutColumn;  ///< 是否可自动调整表头布局
	std::vector< int >            m_vecHeadColWidth;      ///< 保存表头的布局宽度
	int                           m_cxLayout;             ///< 记录当前布局窗口宽度

	std::map< CString, HANDLE >   m_mapKey;               ///< 存储键值对的映射表

	// message handlers
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()

};

#endif //__VXLISTCTRL_H__


