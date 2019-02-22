/**
 * @file    xftp_dltask.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xftp_dltask.cpp
 * 创建日期：2019年02月21日
 * 文件标识：
 * 文件摘要：文件下载的任务对象工作类。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年02月21日
 * 版本摘要：
 * 
 * 取代版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#include "stdafx.h"
#include "xftp_dltask.h"

////////////////////////////////////////////////////////////////////////////////
// x_ftp_dltask_t

//====================================================================

// 
// x_ftp_dltask_t : constructor/destructor
// 

x_ftp_dltask_t::x_ftp_dltask_t(void)
{

}

x_ftp_dltask_t::~x_ftp_dltask_t(void)
{

}

//====================================================================

// 
// x_ftp_dltask_t : overrides
// 

/**********************************************************/
/**
 * @brief 任务对象执行流程的抽象接口。
 */
void x_ftp_dltask_t::run(x_running_checker_t * xchecker_ptr)
{
    while (xchecker_ptr->is_enable_running())
    {

    }
}

/**********************************************************/
/**
 * @brief 判断 任务对象 是否挂起。
 * @note  若任务对象处于挂起状态，工作线程提取任务时，则跳过该对象。
 */
bool x_ftp_dltask_t::is_suspend(void) const
{
    return false;
}

/**********************************************************/
/**
 * @brief 设置任务对象的运行标识。
 */
void x_ftp_dltask_t::set_running_flag(bool xrunning_flag)
{

}

/**********************************************************/
/**
 * @brief 获取任务对象的删除器。
 */
const x_task_deleter_t * x_ftp_dltask_t::get_deleter(void) const
{
    return x_task_t::get_deleter();
}

