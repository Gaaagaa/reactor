/**
 * @file    xtcp_io_manager.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xtcp_io_manager.h
 * 创建日期：2019年01月16日
 * 文件标识：
 * 文件摘要：辅助 x_tcp_io_server_t 对象，进行 套接字 和 IO 通道对象(x_tcp_io_channel_t)
 *          进行映射、消息投递 等工作的管理类。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年01月16日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#ifndef __XTCP_IO_MANAGER_H__
#define __XTCP_IO_MANAGER_H__

#include "xthreadpool.h"

////////////////////////////////////////////////////////////////////////////////

/**
 * @enum  emIoEventCallback
 * @brief 套接字的 IO 事件回调通知码。
 * @note  配合 x_func_ioecbk_t 函数接口实现上层对接工作。
 */
typedef enum emIoEventCallback
{
    EIO_ECBK_ACCEPT = 0x00000100,  ///< 套接字接收连接通知
    EIO_ECBK_CREATE = 0x00000200,  ///< 建立 IO 通道对象的回调操作码（参看 x_tcp_io_create_args_t 说明）
} emIoEventCallback;

/**
 * @struct x_tcp_io_create_args_t
 * @brief  建立 IO 通道对象所需的回调参数。
 * @note
 * <pre>
 *   在 x_tcp_io_manager_t 对象通过 x_func_ioecbk_t 接口回调创建 IO 通道对象时，
 *   [xut_ioecode == EIO_ECBK_CREATE，xht_handler == &x_tcp_io_create_args_t]
 *   实现细节请参看 x_tcp_io_manager_t::create_io_channel() 的相关调用。
 * </pre>
 */
typedef struct x_tcp_io_create_args_t
{
    x_handle_t    xht_manager;  ///< 指向 x_tcp_io_manager_t 对象
    x_sockfd_t    xfdt_sockfd;  ///< 关联的套接字描述符
    x_handle_t    xht_message;  ///< 指向 x_tcp_io_message_t 对象
    x_handle_t    xht_channel;  ///< 完成回调操作时，设置该值为业务层具体的 x_tcp_io_channel_t 对象指针
} x_tcp_io_create_args_t;

/**
 * @brief 套接字 IO 事件回调操作接口的函数类型。
 * 
 * @param [in ] xfdt_sockfd : 套接字描述符。
 * @param [in ] xht_optargs : 回调至业务层的操作参数。
 * @param [in ] xut_ioevent : 回调的 IO 事件（参看 emIoEventCallback 枚举值）。
 * @param [in ] xht_context : 回调的上下文标识信息。
 * 
 * @return x_int32_t
 *         - 返回事件操作状态码（或错误码）。
 */
typedef x_int32_t (* x_func_ioecbk_t)(x_sockfd_t xfdt_sockfd,
                                      x_handle_t xht_optargs,
                                      x_uint32_t xut_ioevent,
                                      x_handle_t xht_context);

////////////////////////////////////////////////////////////////////////////////
// x_tcp_io_manager_t

/**
 * @class x_tcp_io_manager_t
 * @brief 辅助 x_tcp_io_server_t 对象，进行 套接字 和 IO通道对象(x_tcp_io_channel_t)
 *        进行映射、消息投递 等工作的操作类。
 */
class x_tcp_io_manager_t final
{
    // common data types
public:
    /**
     * @enum  emConstValue
     * @brief 枚举常量值。
     */
    typedef enum emConstValue
    {
        ECV_MAPSOCK_CAPACITY  = 256 * 1024,        ///< 套接字映射表的目录容量
        ECV_TIMEOUT_MEMBLOCK  = 3 * 3600 * 1000,   ///< 内存池中的内存块回收的超时时间（单位 毫秒）
    } emConstValue;

private:
    using x_thread_t = std::thread;

    // constructor/destructor
public:
    explicit x_tcp_io_manager_t(x_handle_t xht_tcpserver);
    ~x_tcp_io_manager_t(void);

    // public interfaces
public:
    /**********************************************************/
    /**
     * @brief 启动 IO 管理模块。
     * 
     * @param [in ] xut_threads : 业务处理的工作线程的数量（若为 0，将取 hardware_concurrency() 返回值的 2倍 + 1）。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t start(x_uint32_t xut_threads);

    /**********************************************************/
    /**
     * @brief 停止 IO 管理模块。
     */
    x_void_t stop(void);

    /**********************************************************/
    /**
     * @brief 判断 IO 管理模块是否已经启动。
     */
    inline x_bool_t is_start(void) const { return m_xthreadpool.is_startup(); }

    /**********************************************************/
    /**
     * @brief 设置套接字 IO 事件的回调通知接口。
     */
    inline x_void_t set_io_event_callback(x_func_ioecbk_t xfunc_ptr, x_handle_t xht_context)
    {
        m_xfunc_iocbk  = xfunc_ptr;
        m_xht_cbk_ctxt = xht_context;
    }

    /**********************************************************/
    /**
     * @brief 注册套接字的 写就绪 事件。
     */
    x_int32_t register_pollout(x_sockfd_t xfdt_sockfd);

    /**********************************************************/
    /**
     * @brief 移除所管理的套接字。
     */
    x_int32_t remove_io_holder(x_sockfd_t xfdt_sockfd);

    /**********************************************************/
    /**
     * @brief 更新套接字的保活时间戳。
     */
    x_int32_t update_io_kpalive(x_sockfd_t xfdt_sockfd, x_handle_t xht_channel);

    /**********************************************************/
    /**
     * @brief 返回当前管理的 IO句柄对象(x_tcp_io_channel_t)。
     */
    x_uint32_t count(void) const;

    /**********************************************************/
    /**
     * @brief 提交 IO 处理的任务对象。
     */
    x_void_t submit_io_task(x_task_ptr_t xtask_ptr);

    /**********************************************************/
    /**
     * @brief 回调方式创建业务层工作对象。
     */
    x_int32_t create_io_channel(x_tcp_io_create_args_t & xio_create_args);

    // public interfaces : io event
public:
    /**********************************************************/
    /**
     * @brief 处理 接收到套接字连接事件 的操作接口。
     * 
     * @param [in ] xfdt_sockfd : 触发该事件的套接字描述符。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t io_event_accept(x_sockfd_t xfdt_sockfd);

    /**********************************************************/
    /**
     * @brief 处理 套接字关闭事件 的操作接口。
     * 
     * @param [in ] xfdt_sockfd : 触发该事件的套接字描述符。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t io_event_close(x_sockfd_t xfdt_sockfd);

    /**********************************************************/
    /**
     * @brief 处理 套接字可读事件 的操作接口。
     * 
     * @param [in ] xfdt_sockfd : 触发该事件的套接字描述符。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t io_event_read(x_sockfd_t xfdt_sockfd);

    /**********************************************************/
    /**
     * @brief 处理 套接字可写事件 的操作接口。
     * 
     * @param [in ] xfdt_sockfd : 触发该事件的套接字描述符。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t io_event_write(x_sockfd_t xfdt_sockfd);

    /**********************************************************/
    /**
     * @brief 处理 套接字定时巡检事件 的操作接口。
     * 
     * @param [in ] xfdt_sockfd : 触发该事件的套接字描述符。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t io_event_verify(x_sockfd_t xfdt_sockfd);

    // inner invoking
protected:
    /**********************************************************/
    /**
     * @brief 套接字映射表遍历清除操作的回调函数接口。
     *
     * @param [in    ] xht_maptbl  : 映射表的操作句柄。
     * @param [in    ] xfdt_sockfd : 套接字。
     * @param [in,out] xht_handler : 子项关联的映射句柄（可进行回参设置）。
     *
     * @return x_bool_t
     *         - 返回 X_TRUE ，继续遍历操作；
     *         - 返回 X_FALSE，则终止遍历操作。
     */
    x_bool_t mapsockfd_cleanup(x_handle_t xht_maptbl, x_sockfd_t xfdt_sockfd, x_handle_t * xht_handler);

    /**********************************************************/
    /**
     * @brief 强制清理掉所有的 IO句柄对象(x_tcp_io_channel_t)。
     */
    x_void_t cleanup(void);

    // data members
private:
    x_handle_t      m_xht_tcpserver;   ///< 所隶属的 x_tcp_io_server_t 对象句柄
    x_handle_t      m_xht_mapsockfd;   ///< 套接字映射表操作句柄

    x_func_ioecbk_t m_xfunc_iocbk;     ///< 套接字 IO 事件的回调通知接口函数
    x_handle_t      m_xht_cbk_ctxt;    ///< 套接字 IO 事件回调通知的上下文句柄

    x_threadpool_t  m_xthreadpool;     ///< 负责驱动 业务层工作流程 的线程池
};

////////////////////////////////////////////////////////////////////////////////

#endif // __XTCP_IO_MANAGER_H__
