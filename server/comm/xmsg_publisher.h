/**
 * @file    xmsg_publisher.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xmsg_publisher.h
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

#ifndef __XMSG_PUBLISHER_H__
#define __XMSG_PUBLISHER_H__

#include "xtypes.h"
#include "xmempool.h"

#include <mutex>
#include <condition_variable>
#include <map>
#include <queue>

////////////////////////////////////////////////////////////////////////////////
// x_msg_publisher_t

/**
 * @class x_msg_publisher_t
 * @brief 消息发布者的工作类。
 */
class x_msg_publisher_t
{
    // common data types
public:
    /**
     * @struct x_pubsub_msg_t
     * @brief  发布/订阅 所使用的消息结构体描述信息。
     */
    typedef struct x_msgctxt_t
    {
        x_uint32_t    xut_mtype;    ///< 消息类型
        x_uint32_t    xut_msgid;    ///< 消息标识 ID
        x_handle_t    xht_mctxt;    ///< 消息关联的上下文描述句柄
        x_uint32_t    xut_msize;    ///< 消息数据体大小

#ifdef _MSC_VER
#pragma warning(disable:4200)
#endif // _MSC_VER

        x_uchar_t     xct_mdptr[0]; ///< 消息数据体

#ifdef _MSC_VER
#pragma warning(default:4200)
#endif // _MSC_VER

    } x_msgctxt_t;

    /**********************************************************/
    /**
     * @brief 消息投递到缓存队列后的回调通知接口的函数类型。
     *
     * @param [in ] xht_context : 回调操作的上下文描述句柄。
     *
     * @return x_int32_t
     *         - 操作状态值（未定义）。
     */
    typedef x_int32_t(*x_func_notify_t)(x_handle_t xht_context);

    /**********************************************************/
    /**
     * @brief 订阅者 消息分派操作接口 的函数类型。
     *
     * @param [in ] xut_msgid : 消息标识 ID。
     * @param [in ] xht_mctxt : 消息上下文描述句柄。
     * @param [in ] xut_msize : 消息数据体大小。
     * @param [in ] xct_mdptr : 消息数据体缓存。
     *
     * @return x_int32_t
     *         - 操作状态值（未定义）。
     */
    typedef x_int32_t (* x_func_dispatch_t )(x_uint32_t  xut_msgid,
                                             x_handle_t  xht_mctxt,
                                             x_uint32_t  xut_msize,
                                             x_uchar_t * xct_mdptr);

protected:
    /**
     * @enum  emConstValue
     * @brief 枚举常量值。
     */
    typedef enum emConstValue
    {
        ECV_MQUEUE_MAX_SIZE = 0x0000FFFF,    ///< 消息队列的默认最大容纳数量值
    } emConstValue;

    using x_locker_t       = std::mutex;
    using x_autolock_t     = std::lock_guard< x_locker_t >;
    using x_cvnotify_t     = std::condition_variable;
    using x_map_dispfunc_t = std::map< x_uint32_t, x_func_dispatch_t >;
    using x_msg_queue_t    = std::queue< x_msgctxt_t * >;

    // constructor/destructor
public:
    x_msg_publisher_t(void);
    virtual ~x_msg_publisher_t(void);

    x_msg_publisher_t(x_msg_publisher_t && xobject) = delete;
    x_msg_publisher_t & operator=(x_msg_publisher_t && xobject) = delete;
    x_msg_publisher_t(const x_msg_publisher_t & xobject) = delete;
    x_msg_publisher_t & operator=(const x_msg_publisher_t & xobject) = delete;

    // public interfaces
public:
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
    x_int32_t start(x_func_notify_t xfunc_notify, x_handle_t xht_context);

    /**********************************************************/
    /**
     * @brief 停止工作对象。
     */
    x_void_t stop(void);

    /**********************************************************/
    /**
     * @brief 判断工作对象是否已经启动。
     */
    inline x_bool_t is_start(void) const
    {
        return (((X_NULL == m_xfunc_notify) && (X_NULL != m_xht_thread)) ||
                ((X_NULL != m_xfunc_notify) && (X_NULL == m_xht_thread)));
    }

    /**********************************************************/
    /**
     * @brief 消息投递至队列后，是否进行回调通知。
     */
    inline x_void_t enable_notify_callback(x_bool_t xbt_notify)
    {
        m_xbt_notify = xbt_notify;
    }

    /**********************************************************/
    /**
     * @brief 返回当前消息队列中消息数量。
     */
    inline x_uint32_t queue_size(void) const
    {
        return static_cast< x_uint32_t >(m_xmsg_queue.size());
    }

    /**********************************************************/
    /**
     * @brief 设置 消息队列最大容纳的消息数量。
     */
    inline x_void_t set_queue_max_size(x_uint32_t xut_max_size)
    {
        m_xut_max_qsize = xut_max_size;
    }

    /**********************************************************/
    /**
     * @brief 获取 统计到的丢弃（未执行通知的）消息数量。
     */
    inline x_uint32_t discard_msg_count(void) const
    {
        return m_xut_discards;
    }

    /**********************************************************/
    /**
     * @brief 执行消息发布分派的操作流程
     * @note  不使用 内部工作线程工作的情况下，使用该接口进行消息分派操作。
     * 
     * @param [in ] xut_max_count : 最大发布分派操作次数。
     * 
     * @return x_int32_t
     *         - 返回执行通知消息数量。
     */
    x_int32_t msg_dispatch(x_uint32_t xut_max_count = 0xFFFFFFFF);

    // 消息订阅的相关操作接口
public:
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
    x_bool_t register_subscribe_type(x_uint32_t xut_sub_mtype, x_func_dispatch_t xfunc_dispatch);

    /**********************************************************/
    /**
     * @brief 反注册注册消息订阅接口。
     * 
     * @param [in ] xut_sub_mtype : 订阅的消息类型标识。
     */
    x_void_t unregister_subscribe_type(x_uint32_t xut_sub_mtype);

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
    x_msgctxt_t * alloc_msg(x_uint32_t xut_mtype, x_uint32_t xut_msgid, x_handle_t xht_mctxt, x_uint32_t xut_msize);

    /**********************************************************/
    /**
     * @brief 回收消息对象至内存池中。
     * 
     * @param [in ] xmsg_ptr : 回收的消息对象。
     */
    x_void_t recyc_msg(x_msgctxt_t * xmsg_ptr);

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
    x_int32_t post_msg(x_msgctxt_t * xmsg_ptr);

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
    x_int32_t post_msg(x_uint32_t  xut_mtype,
                       x_uint32_t  xut_msgid,
                       x_handle_t  xht_mctxt,
                       x_uint32_t  xut_msize,
                       x_uchar_t * xct_mdptr);

    /**********************************************************/
    /**
     * @brief 清除消息队列中的所有消息。
     */
    x_void_t cleanup(void);

    // internal call
protected:
    /**********************************************************/
    /**
     * @brief 自身工作线程执行的消息分派操作流程。
     */
    x_void_t thread_dispatch(void);

    /**********************************************************/
    /**
     * @brief 执行消息分派操作流程（仅由 thread_dispatch() 调用）。
     */
    x_bool_t msg_dispatch_proc(void);

    // class data
protected:
    x_func_notify_t  m_xfunc_notify;   ///< 消息投递后的回调操作接口
    x_handle_t       m_xht_context;    ///< 通知的线程 上下文描述句柄
    x_bool_t         m_xbt_notify;     ///< 消息投递至队列后，是否进行回调通知

    x_bool_t         m_xbt_running;    ///< 标识工作线程是否继续运行
    x_handle_t       m_xht_thread;     ///< 工作线程句柄

    x_mempool_t      m_xmsg_mempool;   ///< 消息对象所使用的内存池

    x_locker_t       m_xlock_mapfunc;  ///< m_xmap_dispfunc 的同步操作锁
    x_map_dispfunc_t m_xmap_dispfunc;  ///< 订阅者的 消息分派接口 映射表

    x_cvnotify_t     m_xmq_notify;     ///< 消息加入队列时使用的 条件通知变量
    x_locker_t       m_xlock_queue;    ///< m_xmsg_queue 的同步操作锁
    x_msg_queue_t    m_xmsg_queue;     ///< 消息队列

    x_uint32_t       m_xut_max_qsize;  ///< 消息队列最大容纳的消息数量
    x_uint32_t       m_xut_discards;   ///< 统计丢弃的消息数量
};

////////////////////////////////////////////////////////////////////////////////

#endif // __XMSG_PUBLISHER_H__
