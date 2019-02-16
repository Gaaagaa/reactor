/**
 * @file    xtcp_io_channel.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xtcp_io_channel.cpp
 * 创建日期：2019年01月17日
 * 文件标识：
 * 文件摘要：定义网络 IO 通道对象（业务层工作对象）的抽象接口类。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年01月17日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#include "xcomm.h"
#include "xtcp_io_channel.h"
#include "xtcp_io_manager.h"

////////////////////////////////////////////////////////////////////////////////
// x_tcp_io_channel_t

//====================================================================

// 
// x_tcp_io_channel_t : constructor/destructor
// 

x_tcp_io_channel_t::x_tcp_io_channel_t(x_handle_t xht_manager, x_sockfd_t xfdt_sockfd)
    : m_xht_manager(xht_manager)
    , m_xfdt_sockfd(xfdt_sockfd)
    , m_xut_status(EIO_STATUS_READABLE | EIO_STATUS_WRITABLE)
{

}

x_tcp_io_channel_t::~x_tcp_io_channel_t(void)
{

}

//====================================================================

// 
// x_tcp_io_channel_t : extensible interfaces
// 

/**********************************************************/
/**
 * @brief 投递请求操作的 IO 消息。
 * @note  该接口服务于 x_tcp_io_task_t 对象，可重载该接口，实现消息分包等功能。
 * 
 * @param [in,out] xio_message : 投递的 IO 消息。
 * 
 * @return x_int32_t
 *         - 返回值 >=  0，表示操作产生的事件数量，
 *           若重载该接口，进行消息分包操作，则产生的事件数量可能会大于 1。
 *         - 返回值 == -1，表示产生错误，后续则可关闭该业务层工作对象。
 */
x_int32_t x_tcp_io_channel_t::post_req_xmsg(x_tcp_io_message_t & xio_message)
{
    x_mqautolock_t xautolock(m_xmqueue_req);
    m_xmqueue_req.push(std::move(xio_message));
    return 1;
}

/**********************************************************/
/**
 * @brief 拉取应答操作的 IO 消息。
 * @note  该接口服务于 x_tcp_io_task_t 对象，取到的 IO 消息，用于写操作，即应答操作。
 * 
 * @param [out] xio_message : 操作成功返回的 IO 消息。
 * 
 * @return x_int32_t
 *         - 返回值 ==  1，表示操作成功（提取到一个应答的 IO 消息）；
 *         - 返回值 ==  0，表示操作失败（应答消息队列为空）；
 *         - 返回值 == -1，表示操作产生错误。
 */
x_int32_t x_tcp_io_channel_t::pull_res_xmsg(x_tcp_io_message_t & xio_message)
{
    x_mqautolock_t xautolock(m_xmqueue_res);
    if (!m_xmqueue_res.empty())
    {
        xio_message = std::move(m_xmqueue_res.front());
        m_xmqueue_res.pop();
        return 1;
    }

    return 0;
}

/**********************************************************/
/**
 * @brief 运行时的巡检操作接口（可重载该接口，实时判断对象的有效性）。
 * 
 * @return x_int32_t
 *         - 返回 0，表示对象持续有效；
 *         - 返回 其他值（错误码），表示对象失效（之后对象将会转入等待销毁的状态）。
 */
x_int32_t x_tcp_io_channel_t::io_event_runtime_verify(void)
{
    return 0;
}

/**********************************************************/
/**
 * @brief 处理 “接收 IO 请求消息” 的事件（重载该接口，实现具体业务功能）。
 */
x_int32_t x_tcp_io_channel_t::io_event_requested(x_tcp_io_message_t & xio_message)
{
    return 0;
}

/**********************************************************/
/**
 * @brief 处理 “完成 IO 应答消息” 的事件（可重载该接口，实现具体的完成通知工作）。
 */
x_int32_t x_tcp_io_channel_t::io_event_responsed(x_tcp_io_message_t & xio_message)
{
    return 0;
}

/**********************************************************/
/**
 * @brief 处理 “IO 通道对象被销毁” 的事件（可重载该接口，处理相关资源释放/清理工作）。
 */
x_int32_t x_tcp_io_channel_t::io_event_destroyed(void)
{
    return 0;
}

//====================================================================

// 
// x_tcp_io_channel_t : public interfaces
// 


//====================================================================

// 
// x_tcp_io_channel_t : internal invoking
// 

/**********************************************************/
/**
 * @brief 将 IO 消息加入到应答队列。
 */
x_void_t x_tcp_io_channel_t::push_res_xmsg(x_iomsg_t && xio_message)
{
    x_mqautolock_t xautolock(m_xmqueue_res);
    m_xmqueue_res.push(std::forward< x_iomsg_t >(xio_message));
}

/**********************************************************/
/**
 * @brief 将 IO 消息加入到应答队列。
 */
x_void_t x_tcp_io_channel_t::push_res_xmsg(const x_iomsg_t & xio_message)
{
    x_mqautolock_t xautolock(m_xmqueue_res);
    m_xmqueue_res.push(xio_message);
}

/**********************************************************/
/**
 * @brief 数据清理。
 */
x_void_t x_tcp_io_channel_t::cleanup(void)
{
    m_xht_manager = X_NULL;
    m_xfdt_sockfd = X_INVALID_SOCKFD;
    m_xut_status  = 0;

    m_xmsg_reading = x_iomsg_t();
    m_xmsg_writing = x_iomsg_t();

    {
        x_mqautolock_t xautolock(m_xmqueue_req);
        while (!m_xmqueue_req.empty())
            m_xmqueue_req.pop();
    }

    {
        x_mqautolock_t xautolock(m_xmqueue_res);
        while (!m_xmqueue_res.empty())
            m_xmqueue_res.pop();
    }
}

//====================================================================

// 
// x_tcp_io_channel_t : for x_tcp_io_task_t invoking
// 

/**********************************************************/
/**
 * @brief 将首个 IO 请求消息转移到 请求队列中。
 * @note  该接口只在创建完对应业务层工作对象后被调用。
 * @see   x_tcp_io_creator_t::try_create_io_channel()
 */
x_int32_t x_tcp_io_channel_t::req_xmsg_first_dump(x_iomsg_t & xio_message)
{
    XASSERT(m_xmsg_reading.is_empty());
    m_xmsg_reading = std::move(xio_message);

    return post_req_xmsg(m_xmsg_reading);
}

/**********************************************************/
/**
 * @brief 执行 IO 请求消息的读取流程。
 * 
 * @param [out] xit_rmsgs : 操作返回所读取到的 IO 消息对象数量。
 * 
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t x_tcp_io_channel_t::req_xmsg_reading(x_int32_t & xit_rmsgs)
{
    x_int32_t  xit_error = -1;
    x_uint32_t xut_count = 0;

    xit_rmsgs = 0;

    set_readable(X_TRUE);

    do
    {
        //======================================
        // 读取 IO 请求消息

        xut_count = m_xmsg_reading.nio_read(m_xfdt_sockfd, ECV_NIO_MAX_LEN, xit_error);
        if (0 != xit_error)
        {
            if (EAGAIN == xit_error)
            {
                xit_error = 0;
                set_readable(X_FALSE);
            }
            else
            {
                LOGE("xut_count[%d] = m_xmsg_reading.nio_read(xfdt_sockfd[%d], ...) return error : %d",
                    xut_count, m_xfdt_sockfd, xit_error);
                break;
            }
        }

        if (m_xmsg_reading.rlen() <= 0)
        {
            LOGW("m_xmsg_reading.rlen() <= 0");
            xit_error = 0;
            break;
        }

        //======================================
        // 投递请求消息

        xit_rmsgs = post_req_xmsg(m_xmsg_reading);
        if (xit_rmsgs < 0)
        {
            LOGI("xfdt_sockfd[%d] will be closed! xit_rmsgs : %d", m_xfdt_sockfd, xit_rmsgs);
            xit_error = xit_rmsgs;
            break;
        }

        //======================================
        xit_error = 0;
    } while (0);

    return xit_error;
}

/**********************************************************/
/**
 * @brief 为 x_tcp_io_task_t 对象提供的 IO 请求消息的投递（消息泵）操作。
 * 
 * @param [in, out] xit_rmsgs : 入参，投递 IO 请求消息操作的最大数量；回参，实际投递处理的数量。
 * 
 * @return x_int32_t
 *         - 返回 0，表示操作过程中未产生错误；
 *         - 返回 其他值，表示操作过程中产生的错误码。
 */
x_int32_t x_tcp_io_channel_t::req_xmsg_pump(x_int32_t & xit_rmsgs)
{
    x_int32_t xit_error = 0;
    x_int32_t xit_count = 0;

    x_tcp_io_message_t xio_message;

    for (x_int32_t xit_iter = 0; xit_iter < xit_rmsgs; ++xit_iter)
    {
        //======================================
        // 提取 IO 请求消息

        {
            x_mqautolock_t xautolock(m_xmqueue_req);
            if (m_xmqueue_req.empty())
            {
                xit_error = 0;
                break;
            }
            xio_message = std::move(m_xmqueue_req.front());
            m_xmqueue_req.pop();
        }

        //======================================
        // 投递 IO 请求消息

        xit_count += 1;

        xit_error = io_event_requested(xio_message);
        if (0 != xit_error)
        {
            LOGE("io_event_requested(xio_message) return error : %d", xit_error);
            break;
        }

        //======================================
    }

    xit_rmsgs = xit_count;

    return xit_error;
}

/**********************************************************/
/**
 * @brief 执行 IO 应答消息的写入流程。
 * 
 * @param [in, out] xit_wmsgs : 入参，写处理 IO 应答消息操作的最大数量；回参，实际写处理的数量。
 * 
 * @return x_int32_t
 *         - 返回 0，表示操作过程中未产生错误；
 *         - 返回 其他值，表示操作过程中产生的错误码。
 */
x_int32_t x_tcp_io_channel_t::res_xmsg_writing(x_int32_t & xit_wmsgs)
{
    x_int32_t  xit_error = 0;
    x_int32_t  xit_count = 0;
    x_uint32_t xut_bytes = 0;

    set_writable(X_TRUE);

    for (x_int32_t xit_iter = 0; xit_iter < xit_wmsgs; ++xit_iter)
    {
        //======================================
        // 提取 IO 应答消息

        if (!m_xmsg_writing.is_writable())
        {
            x_mqautolock_t xautolock(m_xmqueue_res);
            if (m_xmqueue_res.empty())
            {
                xit_error = 0;
                break;
            }
            m_xmsg_writing = std::move(m_xmqueue_res.front());
            m_xmqueue_res.pop();
        }

        //======================================
        // 执行 IO 应答消息的写入操作

        xut_bytes = m_xmsg_writing.nio_write(m_xfdt_sockfd, ECV_NIO_MAX_LEN, xit_error);
        if (0 != xit_error)
        {
            if (EAGAIN == xit_error)
            {
                xit_error = 0;
                set_writable(X_FALSE);
            }
            else
            {
                LOGE("xut_bytes[%d] = m_xmsg_writing.nio_write(m_xfdt_sockfd[%d], ...) return error : %d",
                     xut_bytes, m_xfdt_sockfd, xit_error);
            }

            break;
        }

        // 若 IO 应答消息，仍然处于可写入的状态，
        // 则终止完成通知，转到下次的写事件再执行写数据操作
        if (m_xmsg_writing.is_writable())
        {
            xit_error = 0;
            break;
        }

        //======================================
        // 执行 “完成 IO 应答消息” 的通知

        xit_count += 1;

        xit_error = io_event_responsed(m_xmsg_writing);
        if (0 != xit_error)
        {
            LOGE("io_event_responsed(m_xmsg_writing) return error : %d", xit_error);
            break;
        }

        //======================================
    }

    xit_wmsgs = xit_count;

    return xit_error;
}
