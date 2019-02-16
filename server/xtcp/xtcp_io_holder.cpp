/**
 * @file    xtcp_io_holder.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xtcp_io_holder.cpp
 * 创建日期：2019年02月10日
 * 文件标识：
 * 文件摘要：
 *   1. 定义 x_tcp_io_task_t 线程池的工作任务类；
 *      协助 x_tcp_io_manager_t 进行 IO 事件的执行操作。
 *   2. 定义 x_tcp_io_holder_t 辅助工作类；
 *      其与 套接字描述符 结成映射对，存储于 x_tcp_io_manager_t 中，
 *      协助 x_tcp_io_manager_t 进行 IO 事件的投递操作。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年02月10日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#include "xcomm.h"
#include "xtcp_io_holder.h"
#include "xtcp_io_manager.h"

////////////////////////////////////////////////////////////////////////////////

/**********************************************************/
/**
 * @brief 获取 x_tcp_io_task_t 任务对象的事件文本描述信息。
 */
x_cstring_t task_event_text(x_uint32_t xut_event)
{
    switch (xut_event)
    {
    case x_tcp_io_task_t::EIO_TASK_CREATED : return "EIO_TASK_CREATED"; break;
    case x_tcp_io_task_t::EIO_TASK_DESTROY : return "EIO_TASK_DESTROY"; break;
    case x_tcp_io_task_t::EIO_TASK_READING : return "EIO_TASK_READING"; break;
    case x_tcp_io_task_t::EIO_TASK_WRITING : return "EIO_TASK_WRITING"; break;
    case x_tcp_io_task_t::EIO_TASK_MSGPUMP : return "EIO_TASK_MSGPUMP"; break;

    default:
        break;
    }

    return "(unknow)";
}

////////////////////////////////////////////////////////////////////////////////
// x_tcp_io_task_t

//====================================================================

// 
// x_tcp_io_task_t : common invoking
// 

// 共用的 任务对象池
x_tcp_io_task_t::x_taskpool_t x_tcp_io_task_t::_S_comm_taskpool;

//====================================================================

// 
// x_tcp_io_task_t : constructor/destructor
// 

/**********************************************************/
/**
 * @brief 任务对象的默认构造函数。
 * 
 * @param [in ] xio_cwptr : 目标操作的 x_tcp_io_channel_t 对象。
 * @param [in ] xut_event : 任务对象所要处理事件（参看 emIoTaskEventType 枚举值）。
 */
x_tcp_io_task_t::x_tcp_io_task_t(const x_io_cwptr_t & xio_cwptr, x_uint32_t xut_event)
    : m_xio_cwptr(xio_cwptr)
    , m_xio_csptr(nullptr)
    , m_xut_event(xut_event)
{
    if (EIO_TASK_DESTROY == m_xut_event)
    {
        m_xio_csptr = m_xio_cwptr.lock();
        XASSERT(nullptr != m_xio_csptr);
        m_xio_csptr->set_wdestroy(X_TRUE);
    }

#ifdef _DEBUG
    m_xit_timev = std::chrono::system_clock::now().time_since_epoch().count();
#endif // _DEBUG
}

x_tcp_io_task_t::~x_tcp_io_task_t(void)
{
    m_xio_cwptr.reset();
    m_xio_csptr.reset();
    m_xut_event = 0;
}

//====================================================================

// 
// x_tcp_io_task_t : overrides
// 

/**********************************************************/
/**
 * @brief 任务对象执行流程的抽象接口。
 */
void x_tcp_io_task_t::run(x_running_checker_t * xchecker_ptr)
{
#ifdef _DEBUG
    std::chrono::microseconds xtime_delay =
        std::chrono::duration_cast< std::chrono::microseconds >(
            std::chrono::system_clock::now() -
            std::chrono::system_clock::time_point(std::chrono::system_clock::duration(m_xit_timev)));

    STD_TRACE("[%s] task delay time: %ld us", task_event_text(m_xut_event), xtime_delay.count());

    x_io_csptr_t xio_csptr = m_xio_cwptr.lock();
    if (nullptr != xio_csptr)
    {
        STD_TRACE("[fd=%s:%d]xmsg queue size information : [req : %d, res : %d]",
                  sockfd_remote_ip(xio_csptr->get_sockfd(), LOG_BUF(64), 64),
                  sockfd_remote_port(xio_csptr->get_sockfd()),
                  (x_int32_t)xio_csptr->req_queue_size(),
                  (x_int32_t)xio_csptr->res_queue_size());
    }
#endif // _DEBUG

    switch (m_xut_event)
    {
    case EIO_TASK_CREATED : handle_created(); break;
    case EIO_TASK_DESTROY : handle_destroy(); break;
    case EIO_TASK_READING : handle_reading(); break;
    case EIO_TASK_WRITING : handle_writing(); break;
    case EIO_TASK_MSGPUMP : handle_msgpump(); break;

    default:
        break;
    }
}

/**********************************************************/
/**
 * @brief 判断 任务对象 是否挂起。
 */
bool x_tcp_io_task_t::is_suspend(void) const
{
    x_io_csptr_t xio_csptr = m_xio_cwptr.lock();
    if ((nullptr != xio_csptr) && !xio_csptr->is_wdestroy())
    {
        return xio_csptr->is_occupied();
    }

    return false;
}

/**********************************************************/
/**
 * @brief 设置任务对象的运行标识。
 */
void x_tcp_io_task_t::set_running_flag(bool xrunning_flag)
{
    x_io_csptr_t xio_csptr = m_xio_cwptr.lock();
    if (nullptr != xio_csptr)
    {
        xio_csptr->set_occupied(xrunning_flag);
    }
}

/**********************************************************/
/**
 * @brief 获取任务对象的删除器。
 */
const x_task_deleter_t * x_tcp_io_task_t::get_deleter(void) const
{
    return (x_task_deleter_t *)&_S_comm_taskpool;
}

//====================================================================

// 
// x_tcp_io_task_t : internal invoking
// 

/**********************************************************/
/**
 * @brief 执行写操作的工作流程。
 *
 * @param [in ] xio_csptr : 目标操作的 IO 通道对象。
 * @param [in ] xit_wmsgs : 可操作的 IO 消息最大数量。
 *
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t x_tcp_io_task_t::workflow_write(x_io_csptr_t & xio_csptr, x_int16_t xit_wmsgs)
{
    using x_io_mangr_t = x_tcp_io_manager_t *;

    x_int32_t    xit_error = -1;
    x_int32_t    xit_nmsgs = xit_wmsgs;
    x_io_mangr_t xio_mangr = (x_io_mangr_t)xio_csptr->get_manager();

    do
    {
        //======================================
        // 执行 IO 消息的应答操作

        xit_error = xio_csptr->res_xmsg_writing(xit_nmsgs);
        if (0 != xit_error)
        {
            LOGE("xio_csptr->res_xmsg_writing(xit_nmsgs[%d, %d]) return error : %d",
                 xit_wmsgs, xit_nmsgs, xit_error);
            break;
        }

        //======================================
        // 触发下次的写操作事件

        // 判断是否要注册 写就绪 事件
        if (!xio_csptr->is_writable())
        {
            xit_error = xio_mangr->register_pollout(xio_csptr->get_sockfd());
            if (0 != xit_error)
            {
                LOGE("xio_mangr->register_pollout(xio_csptr->get_sockfd()[%d]) return error : %d",
                     xio_csptr->get_sockfd(), xit_error);
            }

            break;
        }

        // 若应答队列仍不为空，则提交 写事件 的任务对象，
        // 以此来触发下次的写操作事件
        if ((xio_csptr->res_queue_size() > 0) && !xio_csptr->is_wdestroy())
        {
            xio_mangr->submit_io_task(taskpool().alloc(x_io_cwptr_t(xio_csptr), EIO_TASK_WRITING));
        }

        //======================================
        xit_error = 0;
    } while (0);

    return xit_error;
}

/**********************************************************/
/**
 * @brief 初建 IO 通道对象事件 的处理流程。
 */
x_int32_t x_tcp_io_task_t::handle_created(void)
{
    using x_io_mangr_t = x_tcp_io_manager_t *;

    x_int32_t    xit_error = -1;
    x_int32_t    xit_nmsgs = 0;
    x_io_csptr_t xio_csptr = nullptr;
    x_io_mangr_t xio_mangr = nullptr;

    do
    {
        //======================================

        xio_csptr = m_xio_cwptr.lock();
        if ((nullptr == xio_csptr) || xio_csptr->is_wdestroy())
        {
            xit_error = 0;
            break;
        }

        xio_mangr = (x_io_mangr_t)xio_csptr->get_manager();
        XASSERT(nullptr != xio_mangr);

        //======================================
        // 执行 IO 消息投递工作

        xit_nmsgs = (x_int32_t)xio_csptr->req_queue_size();

        xit_error = xio_csptr->req_xmsg_pump(xit_nmsgs);
        if (0 != xit_error)
        {
            LOGE("[fd:%d] xio_csptr->req_xmsg_pump(xit_nmsgs[%d]) return error : %d",
                 xio_csptr->get_sockfd(), xit_nmsgs, xit_error);
            break;
        }

        if ((xio_csptr->req_queue_size() > 0) && !xio_csptr->is_wdestroy())
        {
            xio_mangr->submit_io_task(taskpool().alloc(x_io_cwptr_t(xio_csptr), EIO_TASK_MSGPUMP));
        }

        //======================================
        // 执行 IO 应答消息的写操作流程

        xit_error = workflow_write(xio_csptr, (x_int32_t)(xio_csptr->res_queue_size() + 1));
        if (0 != xit_error)
        {
            LOGE("workflow_write(xio_csptr[fd:%d]) return error : %d",
                 xio_csptr->get_sockfd(), xit_error);
            break;
        }

        //======================================
        // 更新 套接字 保活的时间戳

        xio_mangr->update_io_kpalive(xio_csptr->get_sockfd(), (x_handle_t)xio_csptr.get());

        //======================================
        xit_error = 0;
    } while (0);

    if ((0 != xit_error) && (nullptr != xio_csptr) && (nullptr != xio_mangr))
    {
        LOGW("Invoking : xio_mangr->remove_io_holder(xio_csptr->get_sockfd()[%d])",
                xio_csptr->get_sockfd());
        xio_mangr->remove_io_holder(xio_csptr->get_sockfd());
    }

    return xit_error;
}

/**********************************************************/
/**
 * @brief 销毁 IO 通道对象事件 的处理流程。
 */
x_int32_t x_tcp_io_task_t::handle_destroy(void)
{
    if (nullptr != m_xio_csptr)
    {
        m_xio_csptr->io_event_destroyed();
        m_xio_csptr->cleanup();
        m_xio_csptr.reset();
    }

    return 0;
}

/**********************************************************/
/**
 * @brief IO 读事件 的处理流程。
 */
x_int32_t x_tcp_io_task_t::handle_reading(void)
{
    using x_io_mangr_t = x_tcp_io_manager_t *;

    x_int32_t    xit_error = -1;
    x_int32_t    xit_nmsgs = 0;
    x_int32_t    xit_wmsgs = 0;
    x_io_csptr_t xio_csptr = nullptr;
    x_io_mangr_t xio_mangr = nullptr;

    do
    {
        //======================================

        xio_csptr = m_xio_cwptr.lock();
        if ((nullptr == xio_csptr) || xio_csptr->is_wdestroy())
        {
            xit_error = 0;
            break;
        }

        xio_mangr = (x_io_mangr_t)xio_csptr->get_manager();
        XASSERT(nullptr != xio_mangr);

        //======================================
        // 执行 IO 请求消息的读操作

        xit_error = xio_csptr->req_xmsg_reading(xit_nmsgs);
        if (0 != xit_error)
        {
            LOGE("[fd:%d] xio_csptr->req_xmsg_reading(xit_nmsgs[%d]) return error : %d",
                 xio_csptr->get_sockfd(), xit_nmsgs, xit_error);
            break;
        }

        xit_wmsgs = xit_nmsgs;

        //======================================
        // 执行 IO 消息投递工作

        xit_error = xio_csptr->req_xmsg_pump(xit_nmsgs);
        if (0 != xit_error)
        {
            LOGE("[fd:%d] xio_csptr->req_xmsg_pump(xit_nmsgs[%d, %d]) return error : %d",
                 xio_csptr->get_sockfd(), xit_wmsgs, xit_nmsgs, xit_error);
            break;
        }

        if ((xio_csptr->req_queue_size() > 0) && !xio_csptr->is_wdestroy())
        {
            xio_mangr->submit_io_task(taskpool().alloc(x_io_cwptr_t(xio_csptr), EIO_TASK_MSGPUMP));
        }

        //======================================
        // 执行 IO 应答消息的写操作流程

        xit_error = workflow_write(xio_csptr, xit_wmsgs);
        if (0 != xit_error)
        {
            LOGE("workflow_write(xio_csptr[fd:%d], [%d]) return error : %d",
                 xio_csptr->get_sockfd(), xit_wmsgs, xit_error);
            break;
        }

        //======================================
        // 更新 套接字 保活的时间戳

        xio_mangr->update_io_kpalive(xio_csptr->get_sockfd(), (x_handle_t)xio_csptr.get());

        //======================================
        xit_error = 0;
    } while (0);

    if ((0 != xit_error) && (nullptr != xio_csptr) && (nullptr != xio_mangr))
    {
        LOGW("Invoking : xio_mangr->remove_io_holder(xio_csptr->get_sockfd()[%d])",
                xio_csptr->get_sockfd());
        xio_mangr->remove_io_holder(xio_csptr->get_sockfd());
    }

    return xit_error;
}

/**********************************************************/
/**
 * @brief IO 写事件 的处理流程。
 */
x_int32_t x_tcp_io_task_t::handle_writing(void)
{
    using x_io_mangr_t = x_tcp_io_manager_t *;

    x_int32_t    xit_error = -1;
    x_io_csptr_t xio_csptr = nullptr;
    x_io_mangr_t xio_mangr = nullptr;

    do
    {
        //======================================

        xio_csptr = m_xio_cwptr.lock();
        if ((nullptr == xio_csptr) || xio_csptr->is_wdestroy())
        {
            xit_error = 0;
            break;
        }

        xio_mangr = (x_io_mangr_t)xio_csptr->get_manager();
        XASSERT(nullptr != xio_mangr);

        //======================================
        // 执行 IO 应答消息的写操作流程

        xit_error = workflow_write(xio_csptr, (x_int32_t)(xio_csptr->res_queue_size() + 1));
        if (0 != xit_error)
        {
            LOGE("workflow_write(xio_csptr[fd:%d], [%d]) return error : %d",
                 xio_csptr->get_sockfd(), (x_int32_t)(xio_csptr->res_queue_size() + 1), xit_error);
            break;
        }

        //======================================
        // 更新 套接字 保活的时间戳

        xio_mangr->update_io_kpalive(xio_csptr->get_sockfd(), (x_handle_t)xio_csptr.get());

        //======================================
        xit_error = 0;
    } while (0);

    if ((0 != xit_error) && (nullptr != xio_csptr) && (nullptr != xio_mangr))
    {
        LOGW("Invoking : xio_mangr->remove_io_holder(xio_csptr->get_sockfd()[%d])",
                xio_csptr->get_sockfd());
        xio_mangr->remove_io_holder(xio_csptr->get_sockfd());
    }

    return xit_error;
}

/**********************************************************/
/**
 * @brief IO 消息投递事件 的处理流程。
 */
x_int32_t x_tcp_io_task_t::handle_msgpump(void)
{
    using x_io_mangr_t = x_tcp_io_manager_t *;

    x_int32_t    xit_error = -1;
    x_int32_t    xit_nmsgs = 0;
    x_int32_t    xit_rmsgs = 0;
    x_io_csptr_t xio_csptr = nullptr;
    x_io_mangr_t xio_mangr = nullptr;

    do
    {
        //======================================

        xio_csptr = m_xio_cwptr.lock();
        if ((nullptr == xio_csptr) || xio_csptr->is_wdestroy())
        {
            xit_error = 0;
            break;
        }

        xio_mangr = (x_io_mangr_t)xio_csptr->get_manager();
        XASSERT(nullptr != xio_mangr);

        //======================================
        // 执行 IO 消息投递工作

        xit_nmsgs = (x_int32_t)xio_csptr->req_queue_size();
        xit_rmsgs = xit_nmsgs;

        xit_error = xio_csptr->req_xmsg_pump(xit_nmsgs);
        if (0 != xit_error)
        {
            LOGE("[fd:%d] xio_csptr->req_xmsg_pump(xit_nmsgs[%d, %d]) return error : %d",
                 xio_csptr->get_sockfd(), xit_rmsgs, xit_nmsgs, xit_error);
            break;
        }

        if ((xio_csptr->req_queue_size() > 0) && !xio_csptr->is_wdestroy())
        {
            xio_mangr->submit_io_task(taskpool().alloc(x_io_cwptr_t(xio_csptr), EIO_TASK_MSGPUMP));
        }

        //======================================
        // 执行 IO 应答消息的写操作流程

        xit_error = workflow_write(xio_csptr, (x_int32_t)(xio_csptr->res_queue_size() + 1));
        if (0 != xit_error)
        {
            LOGE("workflow_write(xio_csptr[fd:%d], [%d]) return error : %d",
                 xio_csptr->get_sockfd(), (x_int32_t)(xio_csptr->res_queue_size() + 1), xit_error);
            break;
        }

        //======================================
        // 更新 套接字 保活的时间戳

        xio_mangr->update_io_kpalive(xio_csptr->get_sockfd(), (x_handle_t)xio_csptr.get());

        //======================================
        xit_error = 0;
    } while (0);

    if ((0 != xit_error) && (nullptr != xio_csptr) && (nullptr != xio_mangr))
    {
        LOGW("Invoking : xio_mangr->remove_io_holder(xio_csptr->get_sockfd()[%d])",
                xio_csptr->get_sockfd());
        xio_mangr->remove_io_holder(xio_csptr->get_sockfd());
    }

    return xit_error;
}

////////////////////////////////////////////////////////////////////////////////
// x_tcp_io_creator_t

//====================================================================

// 
// x_tcp_io_creator_t : constructor/destructor
// 

x_tcp_io_creator_t::x_tcp_io_creator_t(void)
    : m_xio_csptr(nullptr)
{

}

x_tcp_io_creator_t::~x_tcp_io_creator_t(void)
{

}

//====================================================================

// 
// x_tcp_io_creator_t : overrides
// 

/**********************************************************/
/**
 * @brief 处理 读操作 事件。
 * 
 * @param [in ] xht_manager : IO 管理对象（x_tcp_io_manager_t）。
 * @param [in ] xfdt_sockfd : 目标操作的套接字描述符。
 * 
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t x_tcp_io_creator_t::io_reading(x_handle_t xht_manager, x_sockfd_t xfdt_sockfd)
{
    x_int32_t  xit_error   = -1;

    x_tcp_io_manager_t * xmanager_ptr = (x_tcp_io_manager_t *)xht_manager;

    do
    {
        //======================================

        if ((X_NULL == xht_manager) || (X_INVALID_SOCKFD == xfdt_sockfd))
        {
            break;
        }

        XASSERT(nullptr == m_xio_csptr);

        //======================================
        // 尝试创建业务层工作对象

        xit_error = try_create_io_channel(xht_manager, xfdt_sockfd);
        if (0 != xit_error)
        {
            LOGE("try_create_io_channel(xht_manager, xfdt_sockfd[%s:%d]) return error : %d",
                    sockfd_remote_ip(xfdt_sockfd, LOG_BUF(64), 64),
                    sockfd_remote_port(xfdt_sockfd),
                    xit_error);
            break;
        }

        if (nullptr == m_xio_csptr)
        {
            break;
        }

        //======================================
        // 提交 “业务层的 IO 初建事件” 的业务处理任务

        xmanager_ptr->submit_io_task(
            x_io_task_t::taskpool().alloc(
                x_io_cwptr_t(m_xio_csptr), x_io_task_t::EIO_TASK_CREATED));

        //======================================
        xit_error = 0;
    } while (0);

    return xit_error;
}

//====================================================================

// 
// x_tcp_io_creator_t : internal invoking
// 

/**********************************************************/
/**
 * @brief 尝试接收首个 IO 消息，创建业务层中对应的 x_tcp_io_channel_t 对象。
 * 
 * @param [in ] xht_manager : IO 管理对象（x_tcp_io_manager_t）。
 * @param [in ] xfdt_sockfd : 目标操作的套接字描述符。
 * 
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t x_tcp_io_creator_t::try_create_io_channel(x_handle_t xht_manager, x_sockfd_t xfdt_sockfd)
{
    x_int32_t  xit_error = -1;
    x_uint32_t xut_count = 0;

    x_tcp_io_manager_t * xmanager_ptr = (x_tcp_io_manager_t *)xht_manager;

    XASSERT(X_NULL != xht_manager);
    XASSERT(X_INVALID_SOCKFD != xfdt_sockfd);
    XASSERT(nullptr == m_xio_csptr);

    do
    {
        //======================================
        // 接收数据

        xut_count = m_xmsg_swap.nio_read(xfdt_sockfd, ECV_NIO_MAX_LEN, xit_error);
        if (0 != xit_error)
        {
            if (EAGAIN == xit_error)
            {
                xit_error = 0;
            }
            else
            {
                LOGE("xut_count[%d] = m_xmsg_swap.nio_read(xfdt_sockfd[%d], ...) return error : %d",
                    xut_count, xfdt_sockfd, xit_error);
                break;
            }
        }

        if (m_xmsg_swap.rlen() <= 0)
        {
            LOGW("m_xmsg_swap.rlen() <= 0");
            xit_error = 0;
            break;
        }

        //======================================
        // 尝试创建业务层工作对象

        x_tcp_io_create_args_t xio_create_args;
        xio_create_args.xht_manager = xht_manager;
        xio_create_args.xfdt_sockfd = xfdt_sockfd;
        xio_create_args.xht_message = (x_handle_t)&m_xmsg_swap;
        xio_create_args.xht_channel = X_NULL;

        xit_error = xmanager_ptr->create_io_channel(xio_create_args);
        if (0 != xit_error)
        {
            LOGE("xmanager_ptr->create_io_channel(xio_create_args[xfdt_sockfd : %d]) return error : %d",
                    xfdt_sockfd, xit_error);
            break;
        }

        //======================================

        // 若返回的 IO 通道对象句柄为 X_NULL，有可能所
        // 接收的首个 IO 消息不完整（半包），不能确定要
        // 创建具体的业务层 x_tcp_io_channel_t 对象，
        // 需要继续接收 IO 消息的其余部分数据
        if (X_NULL == xio_create_args.xht_channel)
        {
            LOGI("(X_NULL == xio_create_args.xht_channel), xfdt_sockfd[%d], Try again!", xfdt_sockfd);
            xit_error = 0;
            break;
        }

        m_xio_csptr = x_io_csptr_t((x_tcp_io_channel_t *)xio_create_args.xht_channel);
        XASSERT(nullptr != m_xio_csptr);

        // 完成创建业务层的 IO 工作对象后，m_xmsg_swap 中缓存剩余的数据，
        // 应全部转移至 x_tcp_io_channel_t 对象中
        if (m_xmsg_swap.rlen() > 0)
        {
            m_xio_csptr->req_xmsg_first_dump(m_xmsg_swap);
        }

        m_xmsg_swap = x_tcp_io_message_t();

        //======================================
        xit_error = 0;
    } while (0);

    return xit_error;
}

////////////////////////////////////////////////////////////////////////////////
// x_tcp_io_holder_t

//====================================================================

// 
// x_tcp_io_holder_t : constructor/destructor
// 

x_tcp_io_holder_t::x_tcp_io_holder_t(const x_io_csptr_t & xio_csptr)
    : m_xio_csptr(xio_csptr)
{

}

x_tcp_io_holder_t::~x_tcp_io_holder_t(void)
{
    if (nullptr != m_xio_csptr)
    {
        x_tcp_io_manager_t * xmanager_ptr =
                    (x_tcp_io_manager_t *)m_xio_csptr->get_manager();
        if (nullptr != xmanager_ptr)
        {
            xmanager_ptr->submit_io_task(
                x_io_task_t::taskpool().alloc(
                    x_io_cwptr_t(m_xio_csptr), x_io_task_t::EIO_TASK_DESTROY));
        }

        m_xio_csptr.reset();
    }
}

//====================================================================

// 
// x_tcp_io_holder_t : overrides
// 

/**********************************************************/
/**
 * @brief 处理 读操作 事件。
 * 
 * @param [in ] xht_manager : IO 管理对象（x_tcp_io_manager_t）。
 * @param [in ] xfdt_sockfd : 目标操作的套接字描述符。
 * 
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t x_tcp_io_holder_t::io_reading(x_handle_t xht_manager, x_sockfd_t xfdt_sockfd)
{
    x_int32_t  xit_error = -1;
 
    x_tcp_io_manager_t * xmanager_ptr = (x_tcp_io_manager_t *)xht_manager;

    do
    {
        //======================================

        if ((X_NULL == xht_manager) || (X_INVALID_SOCKFD == xfdt_sockfd))
        {
            break;
        }

        XASSERT(nullptr != m_xio_csptr);
        XASSERT(xfdt_sockfd == m_xio_csptr->get_sockfd());

        //======================================
        // 提交 “业务层的 IO 读事件” 的业务处理任务

        xmanager_ptr->submit_io_task(
            x_io_task_t::taskpool().alloc(
                x_io_cwptr_t(m_xio_csptr), x_io_task_t::EIO_TASK_READING));

        //======================================
        xit_error = 0;
    } while (0);

    return xit_error;
}

/**********************************************************/
/**
 * @brief 处理 写操作 事件。
 * 
 * @param [in ] xht_manager : IO 管理对象（x_tcp_io_manager_t）。
 * @param [in ] xfdt_sockfd : 目标操作的套接字描述符。
 * 
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t x_tcp_io_holder_t::io_writing(x_handle_t xht_manager, x_sockfd_t xfdt_sockfd)
{
    x_int32_t  xit_error = -1;

    x_tcp_io_manager_t * xmanager_ptr = (x_tcp_io_manager_t *)xht_manager;

    do
    {
        //======================================

        if ((X_NULL == xht_manager) || (X_INVALID_SOCKFD == xfdt_sockfd))
        {
            break;
        }

        XASSERT(nullptr != m_xio_csptr);
        XASSERT(xfdt_sockfd == m_xio_csptr->get_sockfd());

        //======================================
        // 提交 “业务层的 IO 写事件” 的业务处理任务

        xmanager_ptr->submit_io_task(
            x_io_task_t::taskpool().alloc(
                x_io_cwptr_t(m_xio_csptr), x_io_task_t::EIO_TASK_WRITING));

        //======================================
        xit_error = 0;
    } while (0);

    return xit_error;
}

/**********************************************************/
/**
 * @brief 处理 巡检 事件。
 * 
 * @param [in ] xht_manager : IO 管理对象（x_tcp_io_manager_t）。
 * @param [in ] xfdt_sockfd : 目标操作的套接字描述符。
 * 
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t x_tcp_io_holder_t::io_verify(x_handle_t xht_manager, x_sockfd_t xfdt_sockfd)
{
    if (nullptr != m_xio_csptr)
    {
        return m_xio_csptr->io_event_runtime_verify();
    }

    return 0;
}

