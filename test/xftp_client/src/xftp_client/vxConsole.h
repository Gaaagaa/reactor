/*
* Copyright (c) 2013, Gaaagaa
* All rights reserved.
* 
* 文件名称：vxConsole.h
* 文件标识：
* 文件摘要：GUI 程序中，使用控制台窗口输出信息的控制类。
* 
* 当前版本：1.0.0.0
* 作    者：
* 完成日期：2013年12月16日
* 版本摘要：
* 
* 取代版本：
* 原作者  ：
* 完成日期：
* 版本摘要：
* 
*/

#ifndef __VXCONSOLE_H__
#define __VXCONSOLE_H__

////////////////////////////////////////////////////////////////////////////////
// vxConsole

class vxConsole
{
	// constructor/destructor
private:
	vxConsole(void);
	~vxConsole(void);

	// singleton invoking
public:
	static vxConsole & Instance(void);

	// public interfaces
public:
	/**********************************************************/
	/**
	 * @brief 设置标题栏显示文本。
	 * 
	 * @param [in ] lpCaption : 标题栏文本信息。
	 * 
	 * @return BOOL
	 *         - 成功，返回 TRUE；
	 *         - 失败，返回 FALSE。
	 */
	BOOL SetConsoleCaption(LPCTSTR lpCaption);

	// class data
protected:

};

////////////////////////////////////////////////////////////////////////////////

/** 使用标准设备输出信息 */
#define STD_TRACE(xszt_format, ...)               \
        do                                        \
        {                                         \
            printf((xszt_format), ##__VA_ARGS__); \
            printf("\n");                         \
        } while (0)                               \

////////////////////////////////////////////////////////////////////////////////

#endif // __VXCONSOLE_H__

