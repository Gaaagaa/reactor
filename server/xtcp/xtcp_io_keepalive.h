/**
 * @file    xtcp_io_keepalive.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xtcp_io_keepalive.h
 * 创建日期：2019年01月22日
 * 文件标识：
 * 文件摘要：实现 IO 存活检测（保活机制）的工具类。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年01月22日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#ifndef __XTCP_IO_KEEPALIVE_H__
#define __XTCP_IO_KEEPALIVE_H__

#include "xtypes.h"
#include <thread>
#include <mutex>
#include <map>
#include <list>

////////////////////////////////////////////////////////////////////////////////
// x_tcp_io_keepalive_t

/**
 * @class x_tcp_io_keepalive_t
 * @brief 实现 IO 存活检测（保活机制）的工具类。
 * 
 * @note
 * <pre>
 *   相关概念解释：
 *     1. 恶意连接：指的是建立套接字连接后(accept()返回)，
 *                 一直未接收到远程的请求数据，无法确定需要提供具体服务类型。
 * </pre>
 */
class x_tcp_io_keepalive_t
{
    // common data types
public:
    using x_eventtype_t = x_uint32_t;
    using x_iokeytype_t = x_sockfd_t;
    using x_iochannel_t = x_handle_t;
    using x_timestamp_t = x_uint64_t;

    /**
     * @enum  emIoAliveEventType
     * @brief 外部请求操作的事件类型。
     */
    typedef enum emIoAliveEventType
    {
        EIOA_JOINTO  = 0x0010,  ///< 加入 IO 存活检测
        EIOA_UPDATE  = 0x0020,  ///< 更新 IO 存活检测
        EIOA_REMOVE  = 0x0030,  ///< 移除 IO 存活检测
    } emIoAliveEventType;

    /**
     * @enum  emConstValue
     * @brief 相关的枚举常量值。
     */
    typedef enum emConstValue
    {
        ECV_TIMEOUT_KPALIVE = 8 * 60 * 1000,   ///< 检测存活的最大超时时间（单位 毫秒）
        ECV_TIMEOUT_BALEFUL = 4 * 60 * 1000,   ///< 检测恶意的最大超时时间（单位 毫秒）
        ECV_TIMEOUT_MVERIFY = 4 * 60 * 1000,   ///< 定时巡检的最小超时时间（单位 毫秒）
        ECV_TMSTAMP_MAXPLUS = 100000,          ///< 时间戳最大微调值
    } emConstValue;

    /**
     * @enum  emIoAliveEventCode
     * @brief x_tcp_io_keepalive_t 对象回调的事件通知码。
     */
    typedef enum emIoAliveEventCode
    {
        EIO_AEC_TIMEOUT  = 0x0010,   ///< 超时
        EIO_AEC_MVERIFY  = 0x0020,   ///< 巡检
    } emIoAliveEventCode;

    /**
     * @brief x_tcp_io_keepalive_t 对象的事件回调接口的函数类型。
     * 
     * @param [in ] xfdt_sockfd : 套接字描述符。
     * @param [in ] xut_ioecode : 回调的事件通知码（参看 emIoAliveEventCode 枚举值）。
     * @param [in ] xht_context : 回调的上下文标识信息。
     * 
     */
    typedef x_void_t (* x_func_ioalive_t)(x_sockfd_t xfdt_sockfd,
                                          x_uint32_t xut_ioecode,
                                          x_handle_t xht_context);

private:
    /**
     * @struct x_event_t
     * @brief  外部请求操作的事件描述信息。
     */
    typedef struct x_event_t
    {
        x_eventtype_t  xevt_optype;  ///< 事件类型（参看 emIoAliveEventType）
        x_iokeytype_t  xkey_sockfd;  ///< 存活检测的套接字描述符（索引键）
        x_iochannel_t  xioc_handle;  ///< 套接字描述符所关联的工作句柄
        x_timestamp_t  xtms_kalive;  ///< 最新的活动（保活）时间戳

        x_event_t(void)
        {
            memset(this, 0, sizeof(x_event_t));
        }

        x_event_t & operator = (const x_event_t & xobject)
        {
            if (this != &xobject)
                memcpy(this, &xobject, sizeof(x_event_t));
            return *this;
        }
    } x_event_t;

    /**
     * @struct x_io_ndesc_t
     * @brief  IO 节点描述信息。
     */
    typedef struct x_io_ndesc_t
    {
        x_iokeytype_t  xkey_sockfd;  ///< 存活检测套接字描述符（索引键）
        x_iochannel_t  xioc_handle;  ///< 套接字描述符所关联的工作句柄
        x_timestamp_t  xtms_kalive;  ///< 最新的活动（保活）时间戳

        x_io_ndesc_t(void)
        {
            memset(this, 0, sizeof(x_io_ndesc_t));
        }

        x_io_ndesc_t(x_iokeytype_t xkey_sockfd,
                     x_iochannel_t xioc_handle,
                     x_timestamp_t xtms_kalive)
        {
            this->xkey_sockfd = xkey_sockfd;
            this->xioc_handle = xioc_handle;
            this->xtms_kalive = xtms_kalive;
        }

        x_io_ndesc_t & operator = (const x_io_ndesc_t & xobject)
        {
            if (this != &xobject)
                memcpy(this, &xobject, sizeof(x_io_ndesc_t));
            return *this;
        }
    } x_io_ndesc_t;

    using x_thread_t    = std::thread;
    using x_lock_t      = std::mutex;
    using x_lst_event_t = std::list< x_event_t >;
    using x_map_ndesc_t = std::map< x_iokeytype_t, x_io_ndesc_t >;
    using x_map_tskey_t = std::map< x_timestamp_t, x_iokeytype_t >;

    // common invoking
protected:
    /**********************************************************/
    /**
     * @brief 当前时间戳（校正后的数值）。
     */
    static x_timestamp_t redefine_time_tick(void);

    // constructor/destructor
public:
    explicit x_tcp_io_keepalive_t(x_func_ioalive_t xfunc_ptr, x_handle_t xht_context);
    ~x_tcp_io_keepalive_t(void);

    // public interfaces
public:
    /**********************************************************/
    /**
     * @brief 启动工作流程。
     * 
     * @param [in ] xut_tmout_kpalive : 检测存活的超时时间（单位 毫秒）。
     * @param [in ] xut_tmout_baleful : 检测恶意连接的超时时间（单位 毫秒）。
     * @param [in ] xut_tmout_mverify : 定时巡检的超时时间（单位 毫秒）。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t start(x_uint64_t xut_tmout_kpalive,
                    x_uint64_t xut_tmout_baleful,
                    x_uint64_t xut_tmout_mverify);

    /**********************************************************/
    /**
     * @brief 停止工作流程。
     */
    x_void_t stop(void);

    /**********************************************************/
    /**
     * @brief 是否已经启动。
     */
    inline x_bool_t is_start(void) const { return m_xbt_running; }

    /**********************************************************/
    /**
     * @brief 设置事件回调的操作接口。
     */
    inline x_void_t set_event_callback(x_func_ioalive_t xfunc_ptr,
                                       x_handle_t xht_context)
    {
        m_xfunc_ioalive = xfunc_ptr;
        m_xht_iocontext = xht_context;
    }

    /**********************************************************/
    /**
     * @brief 投递控制事件（参看 emIoAliveEventType 描述的事件类型），请求更新 IO 节点描述信息。
     * 
     * @param [in ] xevt_optype : 事件类型（参看 emIoAliveEventType）。
     * @param [in ] xkey_sockfd : 存活检测的套接字描述符（索引键）。
     * @param [in ] xioc_handle : 套接字描述符所关联的工作句柄。
     * @param [in ] xtms_kalive : 最新的活动（保活）时间戳。
     * 
     * @return x_int32_t
     *         - 返回 0，表示投递成功；
     *         - 返回 -1，表示投递失败（工作流程未启动）。
     */
    x_int32_t post_event(x_eventtype_t xevt_optype,
                         x_iokeytype_t xkey_sockfd,
                         x_iochannel_t xioc_handle,
                         x_timestamp_t xtms_kalive);

    // internal invoking
private:
    /**********************************************************/
    /**
     * @brief 将 [ 时间戳，IO 节点索引键 ] 插入到目标映射表。
     * 
     * @param [out   ] xmap_tskey  : 操作的目标映射表（时间戳 到 IO 节点索引键 的映射表）。
     * @param [in,out] xtst_value  : 入参，具体的时间戳值；回参，矫正过的时间戳值。
     * @param [in    ] xkey_sockfd : IO 节点的索引键值。
     * 
     * @return x_bool_t
     *         - 成功，返回 X_TRUE;
     *         - 失败，返回 X_FALSE。
     */
    x_bool_t insert_tskey(x_map_tskey_t & xmap_tskey,
                          x_timestamp_t & xtst_value,
                          x_iokeytype_t xkey_sockfd);

    /**********************************************************/
    /**
     * @brief 同步更新 IO 节点描述信息的映射表 中的信息。
     */
    x_void_t update_xmap_ndesc(void);

    /**********************************************************/
    /**
     * @brief 执行存活检测流程。
     */
    x_void_t keepalive_proc(void);

    /**********************************************************/
    /**
     * @brief 工作线程运行的入口函数。
     */
    x_void_t thread_run(void);

    // data members
private:
    x_func_ioalive_t  m_xfunc_ioalive;   ///< 事件回调的函数指针
    x_handle_t        m_xht_iocontext;   ///< 事件回调的上下文句柄

    x_timestamp_t  m_xut_tmout_kpalive;  ///< 检测存活的超时时间（时间值被校正过）
    x_timestamp_t  m_xut_tmout_baleful;  ///< 检测恶意连接的超时时间（时间值被校正过）
    x_timestamp_t  m_xut_tmout_mverify;  ///< 定时巡检的超时时间（单位为 毫秒）

    x_bool_t       m_xbt_running;  ///< 标识工作线程是否可继续运行
    x_thread_t     m_xio_thread;   ///< 工作线程对象
    x_lock_t       m_xlkt_lstevt;  ///< 请求操作的事件队列的同步操作锁
    x_lst_event_t  m_xlst_event;   ///< 请求操作的事件队列
    x_map_ndesc_t  m_xmap_ndesc;   ///< IO 节点描述信息的映射表
    x_map_tskey_t  m_xmap_kalive;  ///< 活动（保活）时间戳 到 IO 节点索引键 的映射表
};

////////////////////////////////////////////////////////////////////////////////

#endif // __XTCP_IO_KEEPALIVE_H__
