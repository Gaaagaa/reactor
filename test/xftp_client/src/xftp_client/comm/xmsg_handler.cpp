/**
 * @file    xmsg_handler.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xmsg_handler.cpp
 * 创建日期：2019年02月14日
 * 文件标识：
 * 文件摘要：实现 消息通知管理的控制对象 和 相关的订阅者接口类。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年02月14日
 * 版本摘要：
 * 
 * 取代版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#include "stdafx.h"
#include "XftpClientApp.h"
#include "xmsg_handler.h"

////////////////////////////////////////////////////////////////////////////////
// x_msg_handler_t

//====================================================================

// 
// x_msg_handler_t : common invoking
// 

/**********************************************************/
/**
 * @brief x_msg_handler_t 对象的单例调用接口。
 */
x_msg_handler_t & x_msg_handler_t::instance(void)
{
    static x_msg_handler_t _S_instance;
    return _S_instance;
}

//====================================================================

// 
// x_msg_handler_t : constructor/destructor
// 

x_msg_handler_t::x_msg_handler_t(void)
{

}

x_msg_handler_t::~x_msg_handler_t(void)
{
    if (is_open())
    {
        close();
    }
}

//====================================================================

// 
// x_msg_handler_t : public interfaces
// 

/**********************************************************/
/**
 * @brief 打开 消息通知管理模块。
 */
x_int32_t x_msg_handler_t::open(x_uint32_t xut_app_msgid)
{
    static x_uint32_t _S_xut_msgid = xut_app_msgid;

    enable_notify_callback(X_TRUE);
    return start(
        [](x_handle_t xht_context) -> x_int32_t
        {
            return PostThreadMessage(theApp.GetAppThreadID(), *(x_uint32_t *)xht_context, 0, 0);
        },
        (x_handle_t)&_S_xut_msgid);
}

/**********************************************************/
/**
 * @brief 关闭 消息通知管理模块。
 */
x_void_t x_msg_handler_t::close(void)
{
    stop();
}

