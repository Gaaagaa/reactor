/**
 * @file    xevent_notifier.cpp
 * <pre>
 * Copyright (c) 2017, 百年千岁 All rights reserved.
 * 
 * 文件名称：xevent_notifier.cpp
 * 创建日期：2017年03月30日
 * 文件标识：
 * 文件摘要：异步事件通知操作的工作类。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2017年03月30日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#include "xevent_notifier.h"

#include <string.h>
#include <thread>
#include <chrono>

////////////////////////////////////////////////////////////////////////////////
// x_event_notifier_t

//====================================================================

// 
// x_event_notifier_t : constructor/destructor
// 

x_event_notifier_t::x_event_notifier_t(void)
            : m_xfunc_cptr(X_NULL)
            , m_xbt_auto_notify(X_TRUE)
            , m_xht_context(X_NULL)
            , m_xbt_run_notify(X_FALSE)
            , m_xht_thread(X_NULL)
            , m_xut_max_events(ECV_DEFAULT_MAX_EVENTS)
            , m_xut_discard_events(0)
{

}

x_event_notifier_t::~x_event_notifier_t(void)
{
    stop();
}

//====================================================================

// 
// x_event_notifier_t : public interfaces
// 

/**********************************************************/
/**
 * @brief
 * <pre>
 *   启动工作对象。主要有两种工作模式，如下：
 *      1、(X_NULL != xfunc_ptr) 时，投递事件后立即回调该接口，以便通知外部执行相应操作；
 *      2、(X_NULL == xfunc_ptr) 时，工作对象自身的工作线程执行通知操作。
 * </pre>
 * 
 * @param [in ] xfunc_ptr : 事件投递后的回调操作接口（若为 X_NULL 时，则启动工作对象自身的工作线程执行通知操作）。
 * @param [in ] xht_ctxt  : 通知操作的 上下文描述句柄。
 * 
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t x_event_notifier_t::start(x_func_epost_cbk_t xfunc_ptr, x_handle_t xht_ctxt)
{
    x_int32_t xit_err = -1;

    do 
    {
        if (is_start())
        {
            break;
        }

        m_xfunc_cptr  = xfunc_ptr;
        m_xht_context = xht_ctxt;

        // 使用独立工作线程模式
        if (X_NULL == m_xfunc_cptr)
        {
            m_xbt_run_notify = X_TRUE;
            m_xht_thread = new std::thread([this]() { notify_proc(); });
            if (X_NULL == m_xht_thread)
            {
                stop();

                xit_err = -1;
                m_xbt_run_notify = X_FALSE;
                break;
            }
        }

        xit_err = 0;
    } while (0);

    return xit_err;
}

/**********************************************************/
/**
 * @brief 停止工作对象。
 */
x_void_t x_event_notifier_t::stop(void)
{
    m_xbt_run_notify = X_FALSE;
    if (X_NULL != m_xht_thread)
    {
        std::thread * xthread_ptr = (std::thread *)m_xht_thread;
        if (xthread_ptr->joinable())
            xthread_ptr->join();
        delete xthread_ptr;

        m_xht_thread = X_NULL;
    }

    m_xfunc_cptr  = X_NULL;
    m_xht_context = X_NULL;

    cleanup_event_queue();
}

/**********************************************************/
/**
 * @brief 触发应用程序的通知操作流程。
 * 
 * @param [in ] xut_max_notify_count : 最大通知操作次数。
 * 
 * @return x_int32_t
 *         - 返回执行通知事件数量。
 */
x_int32_t x_event_notifier_t::app_notify_proc(x_uint32_t xut_max_notify_count /*= 0xFFFFFFFF*/)
{
    if (X_NULL != m_xht_thread)
    {
        return 0;
    }

    x_uint32_t xut_notify_count = 0;

    // 限制投递次数为当前事件队列大小，
    // 防止事件通知过程中，产生新事件，
    // 致使无休止的执行事件投递操作
    if (xut_max_notify_count > event_queue_size())
    {
        xut_max_notify_count = event_queue_size();
    }

    while (m_queue_aevent.size() > 0)
    {
        // 事件通知
        if (aevent_notify_proc())
        {
            if (++xut_notify_count > xut_max_notify_count)
            {
                break;
            }
        }
    }

    return static_cast< x_int32_t >(xut_notify_count);
}

//====================================================================
// 异步通知事件

/**********************************************************/
/**
 * @brief 注册事件通知类型。
 * 
 * @param [in ] xut_notify_type : 事件通知类型标识。
 * @param [in ] xfunc_notify    : 事件通知类型接口。
 * 
 * @return x_bool_t
 *         - 成功，返回 X_TRUE；
 *         - 失败，返回 X_FALSE 。
 */
x_bool_t x_event_notifier_t::register_notify_type(x_uint32_t xut_notify_type, x_func_notify_t xfunc_notify)
{
    x_autolock_t xautolock(&m_map_notify_func);
    x_map_nofity_func_t::iterator itfind = m_map_notify_func.find(xut_notify_type);
    if (itfind != m_map_notify_func.end())
    {
        return X_FALSE;
    }

    m_map_notify_func.insert(std::make_pair(xut_notify_type, xfunc_notify));
    return X_TRUE;
}

/**********************************************************/
/**
 * @brief 反注册事件通知类型。
 * 
 * @param [in ] xut_notify_type : 事件通知类型标识。
 */
x_void_t x_event_notifier_t::unregister_notify_type(x_uint32_t xut_notify_type)
{
    x_autolock_t xautolock(&m_map_notify_func);
    m_map_notify_func.erase(xut_notify_type);
}

/**********************************************************/
/**
 * @brief 从内存池中申请事件对象（所申请的事件，使用完之后需要调用回收接口）。
 * 
 * @param [in ] xut_etype : 事件的类型。
 * @param [in ] xut_evtid : 事件的标识 ID。
 * @param [in ] xht_ectxt : 事件的上下文描述。
 * @param [in ] xut_esize : 事件的数据体大小。
 * 
 * @return x_async_event_t *
 *         - 事件对象。
 */
x_async_event_t * x_event_notifier_t::alloc_event(x_uint32_t xut_etype,
                                                  x_uint32_t xut_evtid,
                                                  x_handle_t xht_ectxt,
                                                  x_uint32_t xut_esize)
{
    x_async_event_t * xevent_ptr =
        (x_async_event_t *)m_mem_pool.alloc(xut_esize + sizeof(x_async_event_t));
    if (X_NULL == xevent_ptr)
    {
        return X_NULL;
    }

    xevent_ptr->xut_etype = xut_etype;
    xevent_ptr->xut_evtid = xut_evtid;
    xevent_ptr->xht_ectxt = xht_ectxt;
    xevent_ptr->xut_esize = xut_esize;

    return xevent_ptr;
}

/**********************************************************/
/**
 * @brief 回收事件对象至内存池中。
 * 
 * @param [in ] xevent_ptr : 回收的事件对象。
 */
x_void_t x_event_notifier_t::recyc_event(x_async_event_t * xevent_ptr)
{
    m_mem_pool.recyc((mblock_t)xevent_ptr);
}

/**********************************************************/
/**
 * @brief 投递事件（投递后的事件对象，会在其通知完成后自动回收，
 *        所以在调用此接口后无需对该事件对象进行手动回收）。
 * 
 * @param [in ] xevent_ptr : 投递的事件对象。
 * 
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t x_event_notifier_t::post_event(x_async_event_t * xevent_ptr)
{
    if (X_NULL == xevent_ptr)
    {
        return -1;
    }

    // 将异步事件放入投递队列中
    {
        x_autolock_t xautolock(&m_queue_aevent);
        m_queue_aevent.push(xevent_ptr);
    }

    // 若事件队列达到上限，则主动丢弃早期投递进来的事件，避免事件队列膨胀
    if (m_queue_aevent.size() > m_xut_max_events)
    {
        xevent_ptr = X_NULL;

        // 从队列中读取投递的异步事件对象
        {
            x_autolock_t xautolock(&m_queue_aevent);
            xevent_ptr = m_queue_aevent.front();
            m_queue_aevent.pop();
        }

        if (X_NULL != xevent_ptr)
        {
            // 回收异步事件
            recyc_event(xevent_ptr);

            // 递增事件丢弃计数值
            m_xut_discard_events += 1;
        }
    }

    // 通知操作
    if (m_xbt_auto_notify && (X_NULL != m_xfunc_cptr))
    {
        m_xfunc_cptr(m_xht_context);
    }

    return 0;
}

/**********************************************************/
/**
 * @brief 直接投递事件。
 * 
 * @param [in ] xut_etype : 事件的类型。
 * @param [in ] xut_evtid : 事件的标识 ID。
 * @param [in ] xht_ectxt : 事件的上下文描述。
 * @param [in ] xut_esize : 事件的数据体大小。
 * @param [in ] xct_edptr : 事件的数据体缓存。
 * 
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t x_event_notifier_t::post_event(x_uint32_t  xut_etype,
                                         x_uint32_t  xut_evtid,
                                         x_handle_t  xht_ectxt,
                                         x_uint32_t  xut_esize,
                                         x_uchar_t * xct_edptr)
{
    x_async_event_t * xevent_ptr = alloc_event(xut_etype, xut_evtid, xht_ectxt, xut_esize);
    if (X_NULL == xevent_ptr)
    {
        return -1;
    }

    if ((xut_esize > 0) && (X_NULL != xct_edptr))
    {
        memcpy(xevent_ptr->xct_edptr, xct_edptr, xut_esize);
    }

    x_int32_t xit_err = post_event(xevent_ptr);
    if (0 != xit_err)
    {
        recyc_event(xevent_ptr);
        xevent_ptr = X_NULL;
    }

    return xit_err;
}

/**********************************************************/
/**
 * @brief 清除事件队列中的所有事件。
 */
x_void_t x_event_notifier_t::cleanup_event_queue(void)
{
    {
        x_autolock_t xautolock(&m_queue_aevent);
        while (m_queue_aevent.size() > 0)
        {
            recyc_event(m_queue_aevent.front());
            m_queue_aevent.pop();
        }
    }

    m_mem_pool.release_pool();
    m_xut_discard_events = 0;
}

//====================================================================

// 
// x_event_notifier_t : internal call
// 

/**********************************************************/
/**
 * @brief 自身工作线程执行的事件通知操作流程。
 */
x_void_t x_event_notifier_t::notify_proc(void)
{
    std::chrono::system_clock::time_point xtime_end;

    while (m_xbt_run_notify)
    {
        // 事件通知
        aevent_notify_proc();

        // 暂停操作
        xtime_end = std::chrono::system_clock::now() + std::chrono::milliseconds(50);
        while (m_xbt_run_notify && (0 == m_queue_aevent.size()) &&
               (std::chrono::system_clock::now() < xtime_end))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

/**********************************************************/
/**
 * @brief 执行事件通知操作流程。
 */
x_bool_t x_event_notifier_t::aevent_notify_proc(void)
{
    if (m_queue_aevent.size() == 0)
    {
        return X_FALSE;
    }

    x_async_event_t * xevent_ptr = X_NULL;

    // 从队列中读取投递的异步事件对象
    {
        x_autolock_t xautolock(&m_queue_aevent);
        xevent_ptr = m_queue_aevent.front();
        m_queue_aevent.pop();
    }

    if (X_NULL == xevent_ptr)
    {
        return X_FALSE;
    }

    x_bool_t xbt_notify = X_FALSE;

    // 投递异步事件对象
    {
        x_autolock_t xautolock(&m_map_notify_func);
        x_map_nofity_func_t::iterator itfind = m_map_notify_func.find(xevent_ptr->xut_etype);
        if (itfind != m_map_notify_func.end())
        {
            itfind->second(xevent_ptr->xut_evtid,
                           xevent_ptr->xht_ectxt,
                           xevent_ptr->xut_esize,
                           xevent_ptr->xct_edptr);

            xbt_notify = X_TRUE;
        }
    }

    // 回收异步事件
    recyc_event(xevent_ptr);
    xevent_ptr = X_NULL;

    return xbt_notify;
}
