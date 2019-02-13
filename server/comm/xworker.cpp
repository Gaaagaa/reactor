/**
 * @file    xworker.cpp
 * <pre>
 * Copyright (c) 2018, Gaaagaa All rights reserved.
 * 
 * 文件名称：xworker.cpp
 * 创建日期：2018年12月20日
 * 文件标识：
 * 文件摘要：程序工作进程的业务处理类。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2018年12月20日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#include "xcomm.h"
#include "xworker.h"
#include "xftp_server.h"

#include <unistd.h>
#include <signal.h>

////////////////////////////////////////////////////////////////////////////////
// x_worker_t

//====================================================================

// 
// x_worker_t : common invoking
// 

/**********************************************************/
/**
 * @brief 创建工作进程对象。
 */
x_worker_t * x_worker_t::create_worker(void)
{
    x_ssize_t xst_wpid = fork();
    if (xst_wpid >= 0)
    {
        return (new x_worker_t(xst_wpid));
    }

    return X_NULL;
}

#if ((defined _DEBUG) || (defined DEBUG))
/**********************************************************/
/**
 * @brief 创建测试的工作进程对象。
 */
x_worker_t * x_worker_t::create_test_worker(void)
{
    return (new x_worker_t(getpid()));
}
#endif // ((defined _DEBUG) || (defined DEBUG))

/**********************************************************/
/**
 * @brief 销毁工作进程对象。
 * 
 * @param[in ] xobject_ptr : 工作进程对象。
 * @param[in ] xbt_kill    : 是否立即杀掉进程。
 */
x_void_t x_worker_t::destroy_worker(x_worker_t * xobject_ptr, x_bool_t xbt_kill)
{
    if (X_NULL == xobject_ptr)
    {
        return;
    }

    if (xbt_kill && (xobject_ptr->get_pid() > 0))
    {
        kill(xobject_ptr->get_pid(), SIGKILL);
    }

    delete xobject_ptr;
}

//====================================================================

// 
// x_worker_t : constructor/destructor
// 

x_worker_t::x_worker_t(x_ssize_t xst_wpid)
    : m_xst_wpid(xst_wpid)
{

}

x_worker_t::~x_worker_t(void)
{

}

//====================================================================

// 
// x_worker_t : public interfaces
// 

/**********************************************************/
/**
 * @brief 工作进程的执行流程接口。
 */
x_int32_t x_worker_t::run(void)
{
    x_int32_t xit_error = -1;

    //======================================

    xit_error = x_ftp_server_t::instance().startup();
    if (0 != xit_error)
    {
        LOGE("x_ftp_server_t::instance().startup() return error : %d", xit_error);
        return xit_error;
    }

    //======================================

    while (X_TRUE)
    {
        if (x_event_handler_t::instance().event_queue_size() > 0)
            x_event_handler_t::instance().app_notify_proc();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    //======================================

    x_ftp_server_t::instance().shutdown();

    //======================================

    return xit_error;
}

