/**
 * @file    xtcp_io_server.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xtcp_io_server.h
 * 创建日期：2019年01月03日
 * 文件标识：
 * 文件摘要：负责 TCP 网络服务工作的管理类。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年01月03日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#ifndef __XTCP_IO_SERVER_H__
#define __XTCP_IO_SERVER_H__

#include "xtcp_io_keepalive.h"
#include "xtcp_io_manager.h"

////////////////////////////////////////////////////////////////////////////////
// x_tcp_io_server_t

/**
 * @class x_tcp_io_server_t
 * @brief 负责 TCP 网络服务工作的管理类。
 */
class x_tcp_io_server_t
{
    friend x_tcp_io_manager_t;

    // common data types
public:
    /**
     * @struct x_workconf_t
     * @brief  x_tcp_io_server_t 对象的工作配置参数。
     */
    typedef struct x_workconf_t
    {
        x_char_t    xszt_host[TEXT_LEN_64];   ///< 监听的地址（四段式 IP 地址，为 空 时，将使用 INADDR_ANY）
        x_uint16_t  xut_port;                 ///< 监听的端口号
        x_uint32_t  xut_epoll_maxsockfds;     ///< 支持打开套接字描述符的最大数量
        x_uint32_t  xut_epoll_waitevents;     ///< 工作线程内每次执行 epoll_wait() 等待的最大事件数量
        x_uint32_t  xut_ioman_threads;        ///< 处理业务层 IO 消息的工作线程数量
        x_uint32_t  xut_tmout_kpalive;        ///< 检测存活的超时时间（单位 毫秒）
        x_uint32_t  xut_tmout_baleful;        ///< 检测恶意连接的超时时间（单位 毫秒）
        x_uint32_t  xut_tmout_mverify;        ///< 定时巡检的超时时间（单位 毫秒）

        x_workconf_t(void)
        {
            memset(this, 0, sizeof(x_workconf_t));
        }

        x_workconf_t & operator = (const x_workconf_t & xobject)
        {
            if (this != &xobject)
                memcpy(this, &xobject, sizeof(x_workconf_t));
            return *this;
        }
    } x_workconf_t;

    /**
     * @enum  emConstValue
     * @brief 相关的枚举常量值。
     */
    typedef enum emConstValue
    {
        ECV_MIN_SOCKFDS  = 1024,        ///< 支持的最小套接字连接数
        ECV_MAX_SOCKFDS  = 128 * 1024,  ///< 支持的最大套接字连接数
        ECV_MIN_EPEVENTS = 1,           ///< 执行 epoll_wait() 等待操作的最小事件数量
        ECV_MAX_EPEVENTS = 32,          ///< 执行 epoll_wait() 等待操作的最大事件数量
    } emConstValue;

private:
    using x_thread_t      = std::thread            ;
    using x_list_thread_t = std::list< x_thread_t >;
    using x_kpalive_t     = x_tcp_io_keepalive_t   ;
    using x_manager_t     = x_tcp_io_manager_t     ;

    // common invoking
public:
    /**********************************************************/
    /**
     * @brief 创建绑定指定端口号 的 TCP套接字。
     * 
     * @param [in ] xszt_host : 指定的地址（四段式 IP 地址，为 X_NULL 时，将使用 INADDR_ANY）。
     * @param [in ] xwt_port  : 指定的端口号。
     * 
     * @return x_sockfd_t
     *         - 成功，返回 套接字的文件描述符；
     *         - 失败，返回 X_INVALID_SOCKFD。
     */
    static x_sockfd_t create_and_bind_sockfd(x_cstring_t xszt_host, x_uint16_t xwt_port);

private:
    /**********************************************************/
    /**
     * @brief 存活检测/巡检 的事件回调接口。
     * 
     * @param [in ] xfdt_sockfd : 套接字描述符。
     * @param [in ] xut_ioecode : 回调的事件通知码（参看 x_kpalive_t::emIoAliveEventCode 枚举值）。
     * @param [in ] xht_context : 回调的上下文标识信息。
     * 
     */
    static x_void_t kpalive_callback(x_sockfd_t xfdt_sockfd, x_uint32_t xut_ioecode, x_handle_t xht_context);

    // constructor/destructor
public:
    explicit x_tcp_io_server_t(void);
    virtual ~x_tcp_io_server_t(void);

    x_tcp_io_server_t(const x_tcp_io_server_t & xobject) = delete;
    x_tcp_io_server_t & operator=(const x_tcp_io_server_t & xobject) = delete;
    x_tcp_io_server_t(x_tcp_io_server_t && xobject) = delete;
    x_tcp_io_server_t & operator=(x_tcp_io_server_t && xobject) = delete;

    // public interfaces
public:
    /**********************************************************/
    /**
     * @brief 启动 TCP 网络服务工作的管理模块。
     * 
     * @param [in ] xwct_config : 工作配置参数。
     * @param [in ] xfdt_listen : 要监听的套接字。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t startup(const x_workconf_t & xwct_config, x_sockfd_t xfdt_listen);

    /**********************************************************/
    /**
     * @brief 关闭 TCP 网络服务工作的管理模块。
     */
    x_void_t shutdown(void);

    /**********************************************************/
    /**
     * @brief 判断 TCP 网络服务工作的管理模块 是否已经启动。
     */
    inline x_bool_t is_startup(void) const { return (X_INVALID_SOCKFD != m_xfdt_listen); }

    /**********************************************************/
    /**
     * @brief 返回当前连接的套接字数量。
     */
    inline x_uint32_t count_sockfd(void) const { return m_xio_manager.count(); }

    /**********************************************************/
    /**
     * @brief 相关的工作配置参数。
     */
    inline const x_workconf_t & workconf(void) const { return m_xwct_config; }

    /**********************************************************/
    /**
     * @brief 投递 IO 存活检测的控制事件。
     * 
     * @param [in ] xut_ioetype : 事件类型（参看 x_tcp_io_keepalive_t::emIoAliveEventType）。
     * @param [in ] xfdt_sockfd : 存活检测的套接字描述符（索引键）。
     * @param [in ] xht_channel : 套接字描述符所关联的工作句柄。
     * @param [in ] xut_tmstamp : 最新的活动（保活）时间戳。
     * 
     * @return x_int32_t
     *         - 返回 0，表示投递成功；
     *         - 返回 -1，表示投递失败（工作流程未启动）。
     */
    inline x_int32_t post_ioalive_event(x_uint32_t xut_ioetype,
                                        x_sockfd_t xfdt_sockfd,
                                        x_handle_t xht_channel,
                                        x_uint64_t xut_tmstamp)
    {
        return m_xio_kpalive.post_event(xut_ioetype, xfdt_sockfd, xht_channel, xut_tmstamp);
    }

    /**********************************************************/
    /**
     * @brief 设置套接字 IO 事件的回调通知接口。
     */
    inline x_void_t set_io_event_callback(x_func_ioecbk_t xfunc_ptr, x_handle_t xht_context)
    {
        m_xio_manager.set_io_event_callback(xfunc_ptr, xht_context);
    }

    // internal invoking
protected:
    /**********************************************************/
    /**
     * @brief 保存相关的工作配置参数。
     */
    x_void_t store_config(const x_workconf_t & xwct_config);

    /**********************************************************/
    /**
     * @brief 设置进程可打开的最大文件数量。
     */
    x_bool_t set_max_fds(x_size_t xst_maxfds);

    /**********************************************************/
    /**
     * @brief 将套接字添加到 epoll 事件管理中。
     * 
     * @param [in ] xfdt_sockfd : 指定的套接字。
     * @param [in ] xut_events  : 事件集。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t add_sockfd_event(x_sockfd_t xfdt_sockfd, x_uint32_t xut_events);

    /**********************************************************/
    /**
     * @brief 修改套接字在 epoll 事件管理的事件集。
     * 
     * @param [in ] xfdt_sockfd : 指定的套接字。
     * @param [in ] xut_events  : 事件集。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t mod_sockfd_event(x_sockfd_t xfdt_sockfd, x_uint32_t xut_events);

    /**********************************************************/
    /**
     * @brief 将套接字从 epoll 事件管理中移除。
     * 
     * @param [in ] xfdt_sockfd : 指定的套接字。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t del_sockfd_event(x_sockfd_t xfdt_sockfd);

    /**********************************************************/
    /**
     * @brief 删除 并 关闭 套接字。
     */
    x_void_t remove_off(x_sockfd_t xfdt_sockfd);

    /**********************************************************/
    /**
     * @brief 设置套接字是否监听 写就绪 事件。
     */
    x_int32_t set_pollout(x_sockfd_t xfdt_sockfd, x_bool_t xbt_pollout);

    /**********************************************************/
    /**
     * @brief 将套接字改成非阻塞模式。
     * 
     * @param [in ] xfdt_sockfd : 指定的套接字。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t set_non_block(x_sockfd_t xfdt_sockfd);

    /**********************************************************/
    /**
     * @brief 创建 TCP 的监听套接字。
     * 
     * @param [in ] xszt_host : 指定监听的地址（四段式 IP 地址，为 X_NULL 时，将使用 INADDR_ANY）。
     * @param [in ] xwt_port  : 指定监听的端口号。
     * 
     * @return x_sockfd_t
     *         - 成功，返回 套接字的文件描述符；
     *         - 失败，返回 X_INVALID_SOCKFD。
     */
    x_sockfd_t create_listen_sockfd(x_cstring_t xszt_host, x_uint16_t xwt_port);

private:
    /**********************************************************/
    /**
     * @brief 负责监听操作的工作线程的执行流程。
     */
    x_void_t thread_listen(void);

    /**********************************************************/
    /**
     * @brief 工作线程的执行流程。
     */
    x_void_t thread_epollio(void);

    /**********************************************************/
    /**
     * @brief 存活检测/巡检 的事件回调接口。
     * 
     * @param [in ] xfdt_sockfd : 套接字描述符。
     * @param [in ] xut_ioecode : 回调的事件通知码（参看 x_kpalive_t::emIoAliveEventCode 枚举值）。
     * 
     */
    x_void_t kpalive_handle(x_sockfd_t xfdt_sockfd, x_uint32_t xut_ioecode);

    /**********************************************************/
    /**
     * @brief 工作线程中，处理 套接字接收 的操作接口。
     * 
     * @param [in ] xit_nthread : 工作线程的索引编号。
     * @param [in ] xfdt_sockfd : 套接字描述符。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t io_handle_accept(x_int32_t xit_nthread, x_sockfd_t xfdt_sockfd);

    /**********************************************************/
    /**
     * @brief 工作线程中，处理 套接字（产生错误）关闭 的操作接口。
     * 
     * @param [in ] xit_nthread : 工作线程的索引编号。
     * @param [in ] xfdt_sockfd : 套接字描述符。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t io_handle_close(x_int32_t xit_nthread, x_sockfd_t xfdt_sockfd);

    /**********************************************************/
    /**
     * @brief 工作线程中，处理 套接字数据读取 的操作接口。
     * 
     * @param [in ] xit_nthread : 工作线程的索引编号。
     * @param [in ] xfdt_sockfd : 套接字描述符。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t io_handle_read(x_int32_t xit_nthread, x_sockfd_t xfdt_sockfd);

    /**********************************************************/
    /**
     * @brief 工作线程中，处理 套接字数据写入 的操作接口。
     * 
     * @param [in ] xit_nthread : 工作线程的索引编号。
     * @param [in ] xfdt_sockfd : 套接字描述符。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t io_handle_write(x_int32_t xit_nthread, x_sockfd_t xfdt_sockfd);

    // data members
private:
    x_workconf_t m_xwct_config;   ///< 相关的工作配置参数
    x_bool_t     m_xbt_running;   ///< 工作线程继续运行的标识值

    x_sockfd_t   m_xfdt_listen;   ///< 监听套接字描述符
    x_thread_t   m_xthd_listen;   ///< 负责监听操作的工作线程

    x_sockfd_t   m_xfdt_epollfd;  ///< epoll 文件描述符
    x_thread_t   m_xthd_epollio;  ///< 执行 epoll_wait() 操作的 IO 事件投递操作的工作线程

    x_kpalive_t  m_xio_kpalive;   ///< IO 存活检测与巡检操作的工作对象
    x_manager_t  m_xio_manager;   ///< IO 句柄对象的管理器对象
};

////////////////////////////////////////////////////////////////////////////////

#endif // __XTCP_IO_SERVER_H__
