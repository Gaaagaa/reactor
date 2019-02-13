/**
 * @file    xevent_notifier.h
 * <pre>
 * Copyright (c) 2017, 百年千岁 All rights reserved.
 * 
 * 文件名称：xevent_notifier.h
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

#ifndef __XEVENT_NOTIFIER_H__
#define __XEVENT_NOTIFIER_H__

#include "xtypes.h"
#include "xmempool.h"

#include <mutex>
#include <map>
#include <queue>

////////////////////////////////////////////////////////////////////////////////

class x_mempool_t;

/**
 * @struct x_async_event_t
 * @brief  异步事件的结构体描述信息。
 */
typedef struct x_async_event_t
{
    x_uint32_t    xut_etype;    ///< 事件类型
    x_uint32_t    xut_evtid;    ///< 事件标识 ID
    x_handle_t    xht_ectxt;    ///< 事件的上下文描述信息
    x_uint32_t    xut_esize;    ///< 事件的数据体缓存长度
#ifdef _MSC_VER
#pragma warning(disable:4200)
#endif // _MSC_VER
    x_uchar_t     xct_edptr[0]; ///< 事件的数据体
#ifdef _MSC_VER
#pragma warning(default:4200)
#endif // _MSC_VER
} x_async_event_t;

/**********************************************************/
/**
 * @brief 事件投递后的回调通知接口的函数类型。
 * 
 * @param [in ] xht_context : 回调操作的上下文描述句柄。
 * 
 * @return x_int32_t
 *         - 操作状态值（未定义）。
 */
typedef x_int32_t (* x_func_epost_cbk_t)(x_handle_t xht_context);

/**********************************************************/
/**
 * @brief 事件通知的函数类型。
 * 
 * @param [in ] xut_evtid : 事件标识 ID。
 * @param [in ] xht_ectxt : 事件上下文描述。
 * @param [in ] xut_esize : 通知参数（事件数据体大小）。
 * @param [in ] xct_edptr : 通知参数（事件数据体缓存）。
 * 
 * @return x_int32_t
 *         - 操作状态值（未定义）。
 */
typedef x_int32_t (* x_func_notify_t)(x_uint32_t  xut_evtid,
                                      x_handle_t  xht_ectxt,
                                      x_uint32_t  xut_esize,
                                      x_uchar_t * xct_edptr);

////////////////////////////////////////////////////////////////////////////////
// x_event_notifier_t

/**
 * @class x_event_notifier_t
 * @brief 异步事件通知操作的工作类。
 */
class x_event_notifier_t
{
    // constructor/destructor
public:
    x_event_notifier_t(void);
    virtual ~x_event_notifier_t(void);

    // internal data types
protected:
    typedef std::mutex x_lock_t;

    /**
     * @class x_autolock_t
     * @brief 实现自动锁操作（利用 构造/析构 函数，调用锁对象的 加锁/解锁 操作）。
     */
    class x_autolock_t
    {
        // constructor/destructor
    public:
        x_autolock_t(const x_lock_t * xlock_ptr)
            : m_xlock_ptr(const_cast< x_lock_t * >(xlock_ptr))
        {
            m_xlock_ptr->lock();
        }

        ~x_autolock_t(void)
        {
            m_xlock_ptr->unlock();
        }

        // data members
    private:
        x_lock_t * m_xlock_ptr;  ///< 锁对象
    };

    /**
     * @class x_map_nofity_func_t [ 事件类型，通知接口函数，访问锁 ] 
     * @brief 用于定义 事件类型的通知接口函数映射表。
     */
    class x_map_nofity_func_t : public std::map< x_uint32_t, x_func_notify_t >
                              , public x_lock_t
    {
        // constructor/destructor
    public:
        x_map_nofity_func_t(void)  {  }
        ~x_map_nofity_func_t(void) {  }
    };

    /**
     * @class x_queue_event_t
     * @brief 异步事件存储队列类型。
     */
    class x_queue_event_t : public std::queue< x_async_event_t * >
                          , public x_lock_t
    {
        // constructor/destructor
    public:
        x_queue_event_t(void)  {  }
        ~x_queue_event_t(void) {  }
    };

    /**
     * @enum  emConstValue
     * @brief 枚举常量值。
     */
    typedef enum emConstValue
    {
        ECV_DEFAULT_MAX_EVENTS = 0x0000FFFF,    ///< 事件队列的默认最大容纳数量值
    } emConstValue;

    // public interfaces
public:
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
    x_int32_t start(x_func_epost_cbk_t xfunc_ptr, x_handle_t xht_ctxt);

    /**********************************************************/
    /**
     * @brief 停止工作对象。
     */
    x_void_t stop(void);

    /**********************************************************/
    /**
     * @brief 返回对象的工作状态是否启动。
     */
    inline x_bool_t is_start(void) const
    {
        return (((X_NULL == m_xfunc_cptr) && (X_NULL != m_xht_thread)) ||
                ((X_NULL != m_xfunc_cptr) && (X_NULL == m_xht_thread)));
    }

    /**********************************************************/
    /**
     * @brief 标识通知事件是否自动投递。
     */
    inline x_void_t enable_auto_notify(x_bool_t xbt_auto_notify)
    {
        m_xbt_auto_notify = xbt_auto_notify;
    }

    /**********************************************************/
    /**
     * @brief 返回当前事件队列中事件数量。
     */
    inline x_uint32_t event_queue_size(void) const
    {
        return static_cast< x_uint32_t >(m_queue_aevent.size());
    }

    /**********************************************************/
    /**
     * @brief 设置 异步事件队列的最大容纳事件数量。
     */
    inline x_void_t set_queue_max_size(x_uint32_t xut_max_events)
    {
        m_xut_max_events = xut_max_events;
    }

    /**********************************************************/
    /**
     * @brief 获取 统计到的丢弃（未执行通知的）事件数量。
     */
    inline x_uint32_t discard_events_count(void) const
    {
        return m_xut_discard_events;
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
    x_int32_t app_notify_proc(x_uint32_t xut_max_notify_count = 0xFFFFFFFF);

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
    x_bool_t register_notify_type(x_uint32_t xut_notify_type, x_func_notify_t xfunc_notify);

    /**********************************************************/
    /**
     * @brief 反注册事件通知类型。
     * 
     * @param [in ] xut_notify_type : 事件通知类型标识。
     */
    x_void_t unregister_notify_type(x_uint32_t xut_notify_type);

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
    x_async_event_t * alloc_event(x_uint32_t xut_etype, x_uint32_t xut_evtid, x_handle_t xht_ectxt, x_uint32_t xut_esize);

    /**********************************************************/
    /**
     * @brief 回收事件对象至内存池中。
     * 
     * @param [in ] xevent_ptr : 回收的事件对象。
     */
    x_void_t recyc_event(x_async_event_t * xevent_ptr);

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
    x_int32_t post_event(x_async_event_t * xevent_ptr);

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
    x_int32_t post_event(x_uint32_t  xut_etype,
                         x_uint32_t  xut_evtid,
                         x_handle_t  xht_ectxt,
                         x_uint32_t  xut_esize,
                         x_uchar_t * xct_edptr);

    /**********************************************************/
    /**
     * @brief 清除事件队列中的所有事件。
     */
    x_void_t cleanup_event_queue(void);

    // internal call
protected:
    /**********************************************************/
    /**
     * @brief 自身工作线程执行的事件通知操作流程。
     */
    x_void_t notify_proc(void);

    /**********************************************************/
    /**
     * @brief 执行事件通知操作流程（仅由 notify_proc() 调用）。
     */
    x_bool_t aevent_notify_proc(void);

    // class data
protected:
    x_func_epost_cbk_t    m_xfunc_cptr;          ///< 事件投递后的回调操作接口
    x_bool_t              m_xbt_auto_notify;     ///< 标识通知事件是否自动投递
    x_handle_t            m_xht_context;         ///< 通知的线程 上下文描述句柄

    volatile x_bool_t     m_xbt_run_notify;      ///< 标识工作线程是否继续 事件通知流程
    x_handle_t            m_xht_thread;          ///< 工作线程句柄

    x_mempool_t           m_mem_pool;            ///< 数据操作的内存池

    x_map_nofity_func_t   m_map_notify_func;     ///< 事件类型的通知接口函数映射表
    x_queue_event_t       m_queue_aevent;        ///< 异步事件队列
    x_uint32_t            m_xut_max_events;      ///< 异步事件队列的最大容纳事件数量
    x_uint32_t            m_xut_discard_events;  ///< 统计丢弃的事件数量
};

////////////////////////////////////////////////////////////////////////////////

#endif // __XEVENT_NOTIFIER_H__
