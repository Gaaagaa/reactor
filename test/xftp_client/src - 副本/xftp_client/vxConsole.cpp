/*
* Copyright (c) 2013, Gaaagaa
* All rights reserved.
* 
* 文件名称：vxConsole.cpp
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

#include "stdafx.h"
#include "vxConsole.h"

#include <locale.h>

////////////////////////////////////////////////////////////////////////////////
// vxConsole

//====================================================================

// 
// vxConsole : singleton invoking
// 

vxConsole & vxConsole::Instance(void)
{
	static vxConsole s_Instance;
	return s_Instance;
}

//====================================================================

// 
// vxConsole constructor/destructor
// 

vxConsole::vxConsole(void)
{
	AllocConsole();

	setlocale(LC_ALL, "chs");

#pragma warning(disable:4996)
	freopen("conin$",  "r+t", stdin );
	freopen("conout$", "w+t", stdout);
	freopen("conout$", "w+t", stderr);
#pragma warning(default:4996)

#if 0
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (NULL != hStdOut)
	{
		SMALL_RECT srect;
		srect.Left   = 0;
		srect.Top    = 0;
		srect.Right  = GetSystemMetrics(SM_CXSCREEN);
		srect.Bottom = GetSystemMetrics(SM_CYFULLSCREEN) - GetSystemMetrics(SM_CYCAPTION);

		SetConsoleWindowInfo(hStdOut, FALSE, &srect);
	}
#endif

}

vxConsole::~vxConsole(void)
{
	fclose(stderr);
	fclose(stdout);
	fclose(stdin);

	FreeConsole();
}

//====================================================================

// 
// vxConsole : public interfaces
// 

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
BOOL vxConsole::SetConsoleCaption(LPCTSTR lpCaption)
{
	return ::SetConsoleTitle(lpCaption);
}

