/**
 * @file    xmsg_publisher.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xmsg_publisher.cpp
 * 创建日期：2019年02月14日
 * 文件标识：
 * 文件摘要：消息发布者的工作类。
 * 特别鸣谢：Lee哥
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

#include "xmsg_publisher.h"

#include <string.h>
#include <thread>
#include <chrono>

////////////////////////////////////////////////////////////////////////////////
// x_msg_publisher_t

//====================================================================

// 
// x_msg_publisher_t : constructor/destructor
// 

x_msg_publisher_t::x_msg_publisher_t(void)
    : m_xfunc_notify(X_NULL)
    , m_xht_context(X_NULL)
    , m_xbt_notify(X_TRUE)
    , m_xbt_running(X_FALSE)
    , m_xht_thread(X_NULL)
    , m_xut_max_qsize(ECV_MQUEUE_MAX_SIZE)
    , m_xut_discards(0)
{

}

x_msg_publisher_t::~x_msg_publisher_t(void)
{
    stop();
}

//====================================================================

// 
// x_msg_publisher_t : public interfaces
// 

/**********************************************************/
/**
 * @brief 启动工作对象。
 * @note
 * <pre>
 *   主要有两种工作模式，如下：
 *      1、(X_NULL != xfunc_notify) 时，投递消息后立即回调该接口，以便通知外部执行相应操作；
 *      2、(X_NULL == xfunc_notify) 时，工作对象自身的工作线程执行通知操作。
 * </pre>
 *
 * @param [in ] xfunc_notify : 消息投递后的回调通知操作接口。
 * @param [in ] xht_context  : 通知操作回调的 上下文描述句柄。
 *
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t x_msg_publisher_t::start(x_func_notify_t xfunc_notify, x_handle_t xht_context)
{
    x_int32_t xit_error = -1;

    do 
    {
        if (is_start())
        {
            break;
        }

        m_xfunc_notify = xfunc_notify;
        m_xht_context  = xht_context;

        // 使用独立工作线程模式
        if (X_NULL == m_xfunc_notify)
        {
            m_xbt_running = X_TRUE;
            m_xht_thread = new std::thread([this]() { thread_dispatch(); });
            if (X_NULL == m_xht_thread)
            {
                stop();

                xit_error = -1;
                m_xbt_running = X_FALSE;
                break;
            }
        }

        xit_error = 0;
    } while (0);

    return xit_error;
}

/**********************************************************/
/**
 * @brief 停止工作对象。
 */
x_void_t x_msg_publisher_t::stop(void)
{
    m_xbt_running = X_FALSE;
    if (X_NULL != m_xht_thread)
    {
        {
            x_autolock_t xautolock(m_xlock_queue);
            m_xmq_notify.notify_all();
        }

        std::thread * xthread_ptr = (std::thread *)m_xht_thread;
        if (xthread_ptr->joinable())
            xthread_ptr->join();
        delete xthread_ptr;

        m_xht_thread = X_NULL;
    }

    m_xfunc_notify = X_NULL;
    m_xht_context  = X_NULL;

    cleanup();
}

/**********************************************************/
/**
 * @brief 执行消息分派的操作流程。
 * @note  不使用 内部工作线程工作的情况下，使用该接口进行消息分派操作。
 *
 * @param [in ] xut_max_count : 最大分派操作次数。
 *
 * @return x_int32_t
 *         - 返回执行通知消息数量。
 */
x_int32_t x_msg_publisher_t::msg_dispatch(x_uint32_t xut_max_count /*= 0xFFFFFFFF*/)
{
    if (X_NULL != m_xht_thread)
    {
        return 0;
    }

    x_uint32_t xut_count = queue_size();

    // 限制分派次数为当前消息队列大小，
    // 防止消息分派过程中，发布新的消息，
    // 导致无休止的执行消息投递操作
    if (xut_max_count > xut_count)
    {
        xut_max_count = xut_count;
    }

    xut_count = 0;
    while (!m_xmsg_queue.empty())
    {
        // 消息分派
        if (msg_dispatch_proc())
        {
            if (++xut_count > xut_max_count)
            {
                break;
            }
        }
    }

    return (x_int32_t)xut_count;
}

//====================================================================

// 
// x_msg_publisher_t : 消息订阅的相关操作接口
// 

/**********************************************************/
/**
 * @brief 注册消息订阅接口。
 *
 * @param [in ] xut_sub_mtype  : 订阅的消息类型标识。
 * @param [in ] xfunc_dispatch : 消息分派的操作接口。
 *
 * @return x_bool_t
 *         - 成功，返回 X_TRUE；
 *         - 失败，返回 X_FALSE 。
 */
x_bool_t x_msg_publisher_t::register_subscribe_type(
    x_uint32_t xut_sub_mtype, x_func_dispatch_t xfunc_dispatch)
{
    x_autolock_t xautolock(m_xlock_mapfunc);
    x_map_dispfunc_t::iterator itfind = m_xmap_dispfunc.find(xut_sub_mtype);
    if (itfind != m_xmap_dispfunc.end())
    {
        return X_FALSE;
    }

    m_xmap_dispfunc.insert(std::make_pair(xut_sub_mtype, xfunc_dispatch));
    return X_TRUE;
}

/**********************************************************/
/**
 * @brief 反注册注册消息订阅接口。
 *
 * @param [in ] xut_sub_mtype : 订阅的消息类型标识。
 */
x_void_t x_msg_publisher_t::unregister_subscribe_type(x_uint32_t xut_sub_mtype)
{
    x_autolock_t xautolock(m_xlock_mapfunc);
    m_xmap_dispfunc.erase(xut_sub_mtype);
}

/**********************************************************/
/**
 * @brief 从内存池中申请消息对象（所申请的消息，使用完之后需要调用回收接口）。
 *
 * @param [in ] xut_mtype : 消息类型。
 * @param [in ] xut_msgid : 消息标识 ID 。
 * @param [in ] xht_mctxt : 消息关联的上下文描述句柄。
 * @param [in ] xut_msize : 消息数据体大小。
 *
 * @return x_msgctxt_t *
 *         - 消息对象。
 */
x_msg_publisher_t::x_msgctxt_t *
x_msg_publisher_t::alloc_msg(x_uint32_t xut_mtype,
                             x_uint32_t xut_msgid,
                             x_handle_t xht_mctxt,
                             x_uint32_t xut_msize)
{
    x_msgctxt_t * xmsg_ptr =
        (x_msgctxt_t *)m_xmsg_mempool.alloc(xut_msize + sizeof(x_msgctxt_t));
    if (X_NULL == xmsg_ptr)
    {
        return X_NULL;
    }

    xmsg_ptr->xut_mtype = xut_mtype;
    xmsg_ptr->xut_msgid = xut_msgid;
    xmsg_ptr->xht_mctxt = xht_mctxt;
    xmsg_ptr->xut_msize = xut_msize;

    return xmsg_ptr;
}

/**********************************************************/
/**
 * @brief 回收消息对象至内存池中。
 *
 * @param [in ] xmsg_ptr : 回收的消息对象。
 */
x_void_t x_msg_publisher_t::recyc_msg(x_msgctxt_t * xmsg_ptr)
{
    m_xmsg_mempool.recyc((mblock_t)xmsg_ptr);
}

/**********************************************************/
/**
 * @brief 发布消息（发布后的消息对象，会在其通知完成后自动回收，
 *        所以在调用此接口后无需对该消息对象进行手动回收）。
 *
 * @param [in ] xmsg_ptr : 投递的消息对象。
 *
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t x_msg_publisher_t::post_msg(x_msgctxt_t * xmsg_ptr)
{
    if (X_NULL == xmsg_ptr)
    {
        return -1;
    }

    {
        x_autolock_t xautolock(m_xlock_queue);

        // 若消息队列达到上限，则主动丢弃早期投递进来的消息，避免消息队列膨胀
        if (m_xmsg_queue.size() > m_xut_max_qsize)
        {
            // 从队列中提取要分派的消息对象
            x_msgctxt_t * xmsg_rptr = m_xmsg_queue.front();
            m_xmsg_queue.pop();

            if (X_NULL != xmsg_rptr)
            {
                // 回收消息对象
                recyc_msg(xmsg_rptr);
                xmsg_rptr = X_NULL;

                // 递增消息丢弃计数值
                m_xut_discards += 1;
            }
        }

        // 将消息对象放入投递队列中
        m_xmsg_queue.push(xmsg_ptr);
        m_xmq_notify.notify_one();
    }

    // 通知操作
    if (m_xbt_notify && (X_NULL != m_xfunc_notify))
    {
        m_xfunc_notify(m_xht_context);
    }

    return 0;
}

/**********************************************************/
/**
 * @brief 发布消息。
 *
 * @param [in ] xut_mtype : 消息类型。
 * @param [in ] xut_msgid : 消息标识 ID 。
 * @param [in ] xht_mctxt : 消息关联的上下文描述句柄。
 * @param [in ] xut_msize : 消息数据体大小。
 * @param [in ] xct_mdptr : 消息数据体缓存。
 *
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t x_msg_publisher_t::post_msg(x_uint32_t  xut_mtype,
                                      x_uint32_t  xut_msgid,
                                      x_handle_t  xht_mctxt,
                                      x_uint32_t  xut_msize,
                                      x_uchar_t * xct_mdptr)
{
    x_msgctxt_t * xmsg_ptr =
            alloc_msg(xut_mtype, xut_msgid, xht_mctxt, xut_msize);
    if (X_NULL == xmsg_ptr)
    {
        return -1;
    }

    if ((xut_msize > 0) && (X_NULL != xct_mdptr))
    {
        memcpy(xmsg_ptr->xct_mdptr, xct_mdptr, xut_msize);
    }

    x_int32_t xit_error = post_msg(xmsg_ptr);
    if (0 != xit_error)
    {
        recyc_msg(xmsg_ptr);
        xmsg_ptr = X_NULL;
    }

    return xit_error;
}

/**********************************************************/
/**
 * @brief 清除消息队列中的所有消息。
 */
x_void_t x_msg_publisher_t::cleanup(void)
{
    {
        x_autolock_t xautolock(m_xlock_queue);
        while (!m_xmsg_queue.empty())
        {
            recyc_msg(m_xmsg_queue.front());
            m_xmsg_queue.pop();
        }
    }

    m_xmsg_mempool.release_pool();
    m_xut_discards = 0;
}

//====================================================================

// 
// x_msg_publisher_t : internal invoking
// 

/**********************************************************/
/**
 * @brief 自身工作线程执行的消息分派操作流程。
 */
x_void_t x_msg_publisher_t::thread_dispatch(void)
{
    std::chrono::system_clock::time_point xtime_end;

    while (m_xbt_running)
    {
        // 消息加入队列
        {
            std::unique_lock< x_locker_t > xunique_locker(m_xlock_queue);
            m_xmq_notify.wait(xunique_locker,
                              [this](void) -> bool
                              {
                                  return (!m_xbt_running || (queue_size() > 0));
                              });
        }

        if (!m_xbt_running)
        {
            break;
        }

        if (0 == queue_size())
        {
            continue;
        }

        // 消息通知
        msg_dispatch_proc();
    }
}

/**********************************************************/
/**
 * @brief 执行消息分派操作流程（仅由 thread_dispatch() 调用）。
 */
x_bool_t x_msg_publisher_t::msg_dispatch_proc(void)
{
    x_msgctxt_t * xmsg_ptr = X_NULL;

    // 从队列中读取操作的消息对象
    {
        x_autolock_t xautolock(m_xlock_queue);
        if (!m_xmsg_queue.empty())
        {
            xmsg_ptr = m_xmsg_queue.front();
            m_xmsg_queue.pop();
        }
    }

    if (X_NULL == xmsg_ptr)
    {
        return X_FALSE;
    }

    x_bool_t          xbt_dispatch = X_FALSE;
    x_func_dispatch_t xfunc_invoke = X_NULL;

    // 获取消息分派的接口
    {
        x_autolock_t xautolock(m_xlock_mapfunc);
        x_map_dispfunc_t::iterator itfind = m_xmap_dispfunc.find(xmsg_ptr->xut_mtype);
        if (itfind != m_xmap_dispfunc.end())
        {
            xfunc_invoke = itfind->second;
        }
    }

    // 分派消息对象
    if (X_NULL != xfunc_invoke)
    {
        xfunc_invoke(xmsg_ptr->xut_msgid,
                     xmsg_ptr->xht_mctxt,
                     xmsg_ptr->xut_msize,
                     xmsg_ptr->xct_mdptr);
        xbt_dispatch = X_TRUE;
    }

    // 回收消息对象
    recyc_msg(xmsg_ptr);
    xmsg_ptr = X_NULL;

    return xbt_dispatch;
}
