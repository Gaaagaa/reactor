/**
 * @file    xtcp_io_server.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xtcp_io_server.cpp
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

#include "xcomm.h"
#include "xtcp_io_server.h"

#include <fcntl.h>
#include <sys/resource.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

////////////////////////////////////////////////////////////////////////////////
// x_tcp_io_server_t

//====================================================================

// 
// x_tcp_io_server_t : common invoking methods
// 

/**********************************************************/
/**
 * @brief 创建绑定指定(本地) 地址 和 端口号 的 TCP套接字。
 * 
 * @param [in ] xszt_host : 指定的地址（四段式 IP 地址，为 X_NULL 时，将使用 INADDR_ANY）。
 * @param [in ] xwt_port  : 指定的本地端口号。
 * 
 * @return x_sockfd_t
 *         - 成功，返回 套接字的文件描述符；
 *         - 失败，返回 X_INVALID_SOCKFD。
 */
x_sockfd_t x_tcp_io_server_t::create_and_bind_sockfd(x_cstring_t xszt_host, x_uint16_t xwt_port)
{
    x_int32_t  xit_error   = -1;
    x_sockfd_t xfdt_sockfd = X_INVALID_SOCKFD;
    x_int32_t  xit_option  = 0;

    struct sockaddr_in xaddr_in;

    do
    {
        //======================================

        // 创建套接字
        xfdt_sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (xfdt_sockfd < 0)
        {
            xit_error = -1;
            LOGE("socket(AF_INET, SOCK_STREAM, 0) return xfdt_sockfd[%d], last error : %d", \
                 xfdt_sockfd, errno);
            break;
        }

        //======================================
        // 绑定监听的 地址 和 端口号

        memset(&xaddr_in, 0, sizeof(struct sockaddr_in));
        xaddr_in.sin_family = AF_INET;
        xaddr_in.sin_port   = htons(xwt_port);
        if ((X_NULL != xszt_host) && ('\0' != xszt_host[0]))
            inet_pton(AF_INET, xszt_host, &xaddr_in.sin_addr.s_addr);
        else
            xaddr_in.sin_addr.s_addr = INADDR_ANY;

        xit_error = bind(xfdt_sockfd, (struct sockaddr *)&xaddr_in, sizeof(struct sockaddr_in));
        if (0 != xit_error)
        {
            LOGE("bind() failed! [host: %s, port: %d, errno: %d]", \
                 ((X_NULL != xszt_host) ? xszt_host : "INADDR_ANY"), xwt_port, errno);
            break;
        }

        // 设置地址重用选项
        xit_option = 1;
        xit_error = setsockopt(xfdt_sockfd,
                               SOL_SOCKET,
                               SO_REUSEADDR,
                               (const void *)&xit_option,
                               sizeof(x_int32_t));
        if (0 != xit_error)
        {
            LOGE("setsockopt(xfdt_sockfd, SOL_SOCKET, SO_REUSEADDR, ...) return xit_error[%d], last error : %d", \
                 xit_error, errno);
            break;
        }

        //======================================
        xit_error = 0;
    } while (0);

    if ((0 != xit_error) && (X_INVALID_SOCKFD != xfdt_sockfd))
    {
        sockfd_close(xfdt_sockfd);
        xfdt_sockfd = X_INVALID_SOCKFD;
    }

    return xfdt_sockfd;
}

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
x_sockfd_t x_tcp_io_server_t::create_listen_sockfd(x_cstring_t xszt_host, x_uint16_t xwt_port)
{
    x_int32_t  xit_error   = -1;
    x_sockfd_t xfdt_listen = X_INVALID_SOCKFD;

    do
    {
        //======================================

        // 创建套接字
        xfdt_listen = create_and_bind_sockfd(xszt_host, xwt_port);
        if (X_INVALID_SOCKFD == xfdt_listen)
        {
            xit_error = -1;
            LOGE("create_and_bind_sockfd(xszt_host[%s], xwt_port[%d]) return X_INVALID_SOCKFD", \
                 ((X_NULL != xszt_host) && ('\0' != xszt_host[0])) ? xszt_host : "", xwt_port);
            break;
        }

        //======================================
        // 设置监听模式

        xit_error = listen(xfdt_listen, SOMAXCONN);
        if (0 != xit_error)
        {
            LOGE("listen(xfdt_listen[%s:%d], SOMAXCONN) return xit_error[%d], last error : %d",
                 sockfd_local_ip(xfdt_listen, LOG_BUF(64), 64),
                 sockfd_local_port(xfdt_listen),
                 xit_error,
                 errno);
            break;
        }

        //======================================
        xit_error = 0;
    } while (0);

    if ((0 != xit_error) && (X_INVALID_SOCKFD != xfdt_listen))
    {
        sockfd_close(xfdt_listen);
        xfdt_listen = X_INVALID_SOCKFD;
    }

    return xfdt_listen;
}

/**********************************************************/
/**
 * @brief 存活检测/巡检 的事件回调接口。
 * 
 * @param [in ] xfdt_sockfd : 套接字描述符。
 * @param [in ] xut_ioecode : 回调的事件通知码（参看 x_kpalive_t::emIoAliveEventCode 枚举值）。
 * @param [in ] xht_context : 回调的上下文标识信息。
 * 
 */
x_void_t x_tcp_io_server_t::kpalive_callback(x_sockfd_t xfdt_sockfd, x_uint32_t xut_ioecode, x_handle_t xht_context)
{
    x_tcp_io_server_t * xthis_ptr = (x_tcp_io_server_t *)xht_context;
    xthis_ptr->kpalive_handle(xfdt_sockfd, xut_ioecode);
}

//====================================================================

// 
// x_tcp_io_server_t : constructor/destructor
// 

x_tcp_io_server_t::x_tcp_io_server_t(void)
    : m_xbt_running(X_TRUE)
    , m_xfdt_listen(X_INVALID_SOCKFD)
    , m_xfdt_epollfd(X_INVALID_SOCKFD)
    , m_xio_kpalive(&x_tcp_io_server_t::kpalive_callback, (x_handle_t)this)
    , m_xio_manager((x_handle_t)this)
{

}

x_tcp_io_server_t::~x_tcp_io_server_t(void)
{

}

//====================================================================

// 
// x_tcp_io_server_t : public interfaces
// 

/**********************************************************/
/**
 * @brief 启动 TCP 网络服务工作的管理模块。
 * 
 * @param [in ] xwct_config : 工作配置参数。
 * @param [in ] xfdt_listen : 要监听的套接字（若为 X_INVALID_SOCKFD 时，则使用 xwct_config 中的参数创建）。
 * 
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t x_tcp_io_server_t::startup(const x_workconf_t & xwct_config, x_sockfd_t xfdt_listen)
{
    x_int32_t xit_error = -1;

    do
    {
        //======================================

        if (is_startup())
        {
            shutdown();
        }

        // 存储工作配置参数
        store_config(xwct_config);

        // 设置进程可打开的文件数量
        if (!set_max_fds(m_xwct_config.xut_epoll_maxsockfds + ECV_MIN_SOCKFDS))
        {
            LOGE("set_max_fds(%d) return X_FALSE", m_xwct_config.xut_epoll_maxsockfds + ECV_MIN_SOCKFDS);
            xit_error = -1;
            break;
        }

        //======================================

        // 创建 epoll 文件描述符
        m_xfdt_epollfd = epoll_create(1);
        if (-1 == m_xfdt_epollfd)
        {
            LOGE("epoll_create(1) cannot create epollfd, last error code : %d", errno);
            xit_error = errno;
            m_xfdt_epollfd = X_INVALID_SOCKFD;
            break;
        }

        // 设置监听套接字
        if (X_INVALID_SOCKFD != xfdt_listen)
        {
            m_xfdt_listen = xfdt_listen;
        }
        else
        {
            m_xfdt_listen = create_listen_sockfd(m_xwct_config.xszt_host, m_xwct_config.xut_port);
            if (X_INVALID_SOCKFD == m_xfdt_listen)
            {
                LOGE("create_listen_sockfd(host[%s], port[%d]) failed, last error code : %d", \
                     m_xwct_config.xszt_host, m_xwct_config.xut_port, errno);
                xit_error = errno;
                break;
            }
        }

        //======================================
        // 构建工作线程组

        try
        {
            m_xbt_running = X_TRUE;

            m_xthd_listen  = std::move(x_thread_t([this](void) -> x_void_t { thread_listen();  }));
            m_xthd_epollio = std::move(x_thread_t([this](void) -> x_void_t { thread_epollio(); }));
        }
        catch (...)
        {
            LOGE("std::thread execption!");
            xit_error = -1;
            break;
        }

        //======================================
        // 启动 IO 保活检测模块 与 IO 管理模块

        xit_error = m_xio_kpalive.start(m_xwct_config.xut_tmout_kpalive,
                                        m_xwct_config.xut_tmout_baleful,
                                        m_xwct_config.xut_tmout_mverify);
        if (0 != xit_error)
        {
            LOGE("m_xio_kpalive.start(tmout_kpalive[%d], mout_baleful[%d], tmout_mverify[%d]) return error : %d",
                 m_xwct_config.xut_tmout_kpalive,
                 m_xwct_config.xut_tmout_baleful,
                 m_xwct_config.xut_tmout_mverify,
                 xit_error);
            break;
        }

        xit_error = m_xio_manager.start(m_xwct_config.xut_ioman_threads);
        if (0 != xit_error)
        {
            LOGE("m_xio_manager.startup(m_xwct_config.xut_ioman_threads[%d]) return error : %d",
                 m_xwct_config.xut_ioman_threads, xit_error);
            break;
        }

        //======================================
        xit_error = 0;
    } while (0);

    if (0 != xit_error)
    {
        shutdown();
    }

    return xit_error;
}

/**********************************************************/
/**
 * @brief 关闭 TCP 网络服务工作的管理模块。
 */
x_void_t x_tcp_io_server_t::shutdown(void)
{
    m_xbt_running = X_FALSE;

    if (X_INVALID_SOCKFD != m_xfdt_listen)
    {
        sockfd_close(m_xfdt_listen);
        m_xfdt_listen = X_INVALID_SOCKFD;
    }

    if (X_INVALID_SOCKFD != m_xfdt_epollfd)
    {
        sockfd_close(m_xfdt_epollfd);
        m_xfdt_epollfd = X_INVALID_SOCKFD;
    }

    if (m_xthd_listen.joinable())
    {
        m_xthd_listen.join();
    }

    if (m_xthd_epollio.joinable())
    {
        m_xthd_epollio.join();
    }

    m_xio_kpalive.stop();
    m_xio_manager.stop();
}

//====================================================================

// 
// x_tcp_io_server_t : internal invoking
// 

/**********************************************************/
/**
 * @brief 保存相关的工作配置参数。
 */
x_void_t x_tcp_io_server_t::store_config(const x_workconf_t & xwct_config)
{
    m_xwct_config = xwct_config;

    // 校正 最大连接数
    m_xwct_config.xut_epoll_maxsockfds =
        limit_bound(m_xwct_config.xut_epoll_maxsockfds,
                    ECV_MIN_SOCKFDS,
                    ECV_MAX_SOCKFDS);

    // 校正 epoll_wait() 等待操作的最大事件数量
    m_xwct_config.xut_epoll_waitevents =
        limit_bound(m_xwct_config.xut_epoll_waitevents,
                    ECV_MIN_EPEVENTS,
                    ECV_MAX_EPEVENTS);

    // 校正 IO 管理模块的工作线程数量
    m_xwct_config.xut_ioman_threads =
        limit_bound(m_xwct_config.xut_ioman_threads,
                    1,
                    limit_lower(2 * std::thread::hardware_concurrency(), 1));
}

/**********************************************************/
/**
 * @brief 设置进程可打开的最大文件数量。
 */
x_bool_t x_tcp_io_server_t::set_max_fds(x_size_t xst_maxfds)
{
    struct rlimit xrlimit;
    if (getrlimit(RLIMIT_NOFILE, &xrlimit) < 0)
    {
        LOGE("getrlimit(RLIMIT_NOFILE, &xrlimit) error code : %d", errno);
        return X_FALSE;
    }

    if ((x_size_t)xrlimit.rlim_cur < xst_maxfds)
    {
        xrlimit.rlim_cur = (rlim_t)xst_maxfds;
        xrlimit.rlim_max = xrlimit.rlim_max + xst_maxfds;

        if (setrlimit(RLIMIT_NOFILE, &xrlimit) < 0)
        {
            LOGE("setrlimit(RLIMIT_NOFILE, &xrlimit[%d, %d]) error code : %d",
                    (x_int32_t)xrlimit.rlim_cur, (x_int32_t)xrlimit.rlim_max, errno);
            return X_FALSE;
        }
    }

    return X_TRUE;
}

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
x_int32_t x_tcp_io_server_t::add_sockfd_event(x_sockfd_t xfdt_sockfd, x_uint32_t xut_events)
{
    x_int32_t xit_error = -1;

    struct epoll_event xevent;
    xevent.events  = xut_events;
    xevent.data.fd = xfdt_sockfd;

    xit_error = epoll_ctl(m_xfdt_epollfd, EPOLL_CTL_ADD, xfdt_sockfd, &xevent);
    if (-1 == xit_error)
    {
        LOGE("epoll_ctl(m_xfdt_epollfd[%d], EPOLL_CTL_ADD, xfdt_sockfd[%d], &xevent[%d]) last error : %d", \
             m_xfdt_epollfd, xfdt_sockfd, xut_events, errno);

        xit_error = (EEXIST == errno) ? 0 : errno;
    }

    return xit_error;
}

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
x_int32_t x_tcp_io_server_t::mod_sockfd_event(x_sockfd_t xfdt_sockfd, x_uint32_t xut_events)
{
    x_int32_t xit_error = -1;

    struct epoll_event xevent;
    xevent.events  = xut_events;
    xevent.data.fd = xfdt_sockfd;

    xit_error = epoll_ctl(m_xfdt_epollfd, EPOLL_CTL_MOD, xfdt_sockfd, &xevent);
    if (-1 == xit_error)
    {
        LOGE("epoll_ctl(m_xfdt_epollfd[%d], EPOLL_CTL_MOD, xfdt_sockfd[%d], &xevent[%d]) last error : %d", \
             m_xfdt_epollfd, xfdt_sockfd, xut_events, errno);
        xit_error = errno;
    }

    return xit_error;
}

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
x_int32_t x_tcp_io_server_t::del_sockfd_event(x_sockfd_t xfdt_sockfd)
{
    x_int32_t xit_error = -1;

    struct epoll_event xevent;
    xevent.events  = 0;
    xevent.data.fd = xfdt_sockfd;

    xit_error = epoll_ctl(m_xfdt_epollfd, EPOLL_CTL_DEL, xfdt_sockfd, &xevent);
    if (-1 == xit_error)
    {
        LOGE("epoll_ctl(m_xfdt_epollfd[%d], EPOLL_CTL_DEL, xfdt_sockfd[%d], ...) last error : %d", \
             m_xfdt_epollfd, xfdt_sockfd, errno);
        xit_error = errno;
    }

    return xit_error;
}

/**********************************************************/
/**
 * @brief 关闭套接字。
 */
/**********************************************************/
/**
 * @brief 删除 并 关闭 套接字。
 */
x_void_t x_tcp_io_server_t::remove_off(x_sockfd_t xfdt_sockfd)
{
    del_sockfd_event(xfdt_sockfd);
    sockfd_close(xfdt_sockfd);
}

/**********************************************************/
/**
 * @brief 设置套接字是否监听 写就绪 事件。
 */
x_int32_t x_tcp_io_server_t::set_pollout(x_sockfd_t xfdt_sockfd, x_bool_t xbt_pollout)
{
    x_uint32_t xut_events = EPOLLET | EPOLLIN;
    if (xbt_pollout)
    {
        xut_events |= EPOLLOUT;
    }

    return mod_sockfd_event(xfdt_sockfd, xut_events);
}

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
x_int32_t x_tcp_io_server_t::set_non_block(x_sockfd_t xfdt_sockfd)
{
    x_int32_t xit_error = -1;
    x_int32_t xit_flags = 0;

    do
    {
        xit_flags = fcntl(xfdt_sockfd, F_GETFL);
        if (-1 == xit_flags)
        {
            LOGE("fcntl(xfdt_sockfd[%d], F_GETFL) return -1, last error : %d", xfdt_sockfd, errno);
            xit_error = errno;
            break;
        }

        xit_flags |= O_NONBLOCK;
        xit_error  = fcntl(xfdt_sockfd, F_SETFL, xit_flags);
        if (-1 == xit_error)
        {
            LOGE("fcntl(xfdt_sockfd[%d], F_SETFL) return -1, last error : %d", xfdt_sockfd, errno);
            xit_error = errno;
            break;
        }

        xit_error = 0;
    } while (0);

    return xit_error;
}

/**********************************************************/
/**
 * @brief 负责监听操作的工作线程的执行流程。
 */
x_void_t x_tcp_io_server_t::thread_listen(void)
{
    x_sockfd_t      xfdt_sockfd = X_INVALID_SOCKFD;
    x_uint32_t      xut_addrlen = sizeof(struct sockaddr);
    struct sockaddr xaddr_client;

    const x_int32_t xit_nthread = 0;

    while (m_xbt_running)
    {
        xfdt_sockfd = accept(m_xfdt_listen, &xaddr_client, &xut_addrlen);
        if (-1 == xfdt_sockfd)
        {
            if (!((EAGAIN == errno) || (EWOULDBLOCK == errno) || (EINTR == errno)))
            {
                LOGE("[thread_index: 0] accept(m_xfdt_listen[%d], ...) return -1, last error : %d",
                     m_xfdt_listen, errno);
            }

            continue;
        }

        LOGI("accept() client[fd:%d] : [local -> %s:%d] <=> [remote -> %s:%d]",
             xfdt_sockfd,
             sockfd_local_ip(xfdt_sockfd, LOG_BUF(64), 64),
             sockfd_local_port(xfdt_sockfd),
             sockfd_remote_ip(xfdt_sockfd, LOG_BUF(64), 64),
             sockfd_remote_port(xfdt_sockfd));

        io_handle_accept(xit_nthread, xfdt_sockfd);
    }
}

/**********************************************************/
/**
 * @brief 工作线程的执行流程。
 */
x_void_t x_tcp_io_server_t::thread_epollio(void)
{
    x_int32_t xit_error = 0;
    x_int32_t xit_wait  = 0;
    x_int32_t xit_iter  = 0;

    const x_int32_t xit_nthread = 1;

    std::vector< struct epoll_event > xvec_events;
    xvec_events.resize(workconf().xut_epoll_waitevents);
    x_int32_t xit_size = (x_int32_t)xvec_events.size();

    while (m_xbt_running)
    {
        xit_wait = epoll_wait(m_xfdt_epollfd, xvec_events.data(), xit_size, -1);
        if (xit_wait <= 0)
        {
            continue;
        }

        for (xit_iter = 0; xit_iter < xit_wait; ++xit_iter)
        {
            struct epoll_event & xevent = xvec_events[xit_iter];

            if ((xevent.events & EPOLLERR  ) ||
#ifdef EPOLLRDHUP
                (xevent.events & EPOLLRDHUP) ||
#endif // EPOLLRDHUP
                (xevent.events & EPOLLHUP  ))
            {
                io_handle_close(xit_nthread, xevent.data.fd);
                continue;
            }

            if (xevent.events & EPOLLIN)
            {
                xit_error = io_handle_read(xit_nthread, xevent.data.fd);
                if (0 != xit_error)
                {
                    io_handle_close(xit_nthread, xevent.data.fd);
                    continue;
                }
            }

            if (xevent.events & EPOLLOUT)
            {
                xit_error = io_handle_write(xit_nthread, xevent.data.fd);
                if (0 != xit_error)
                {
                    io_handle_close(xit_nthread, xevent.data.fd);
                    continue;
                }
            }
        }
    }
}

/**********************************************************/
/**
 * @brief 存活检测/巡检 的事件回调接口。
 * 
 * @param [in ] xfdt_sockfd : 套接字描述符。
 * @param [in ] xut_ioecode : 回调的事件通知码（参看 x_kpalive_t::emIoAliveEventCode 枚举值）。
 * 
 */
x_void_t x_tcp_io_server_t::kpalive_handle(x_sockfd_t xfdt_sockfd, x_uint32_t xut_ioecode)
{
    const x_int32_t xit_nthread = 2;

    x_int32_t xit_error = -1;

    switch (xut_ioecode)
    {
    case x_kpalive_t::EIO_AEC_TIMEOUT :
        {
            LOGI("[thread_index: %d] xfdt_sockfd[%s:%d] timeout!",
                 xit_nthread,
                 sockfd_remote_ip(xfdt_sockfd, LOG_BUF(64), 64),
                 sockfd_remote_port(xfdt_sockfd));

            io_handle_close(xit_nthread, xfdt_sockfd);
        }
        break;

    case x_kpalive_t::EIO_AEC_MVERIFY :
        {
            xit_error = m_xio_manager.io_event_verify();
            if (0 != xit_error)
            {
                LOGE("m_xio_manager.io_event_verify() return error : %d", xit_error);
            }
        }
        break;

    default:
        break;
    }
}

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
x_int32_t x_tcp_io_server_t::io_handle_accept(x_int32_t xit_nthread, x_sockfd_t xfdt_sockfd)
{
    x_int32_t  xit_error = -1;

    do
    {
        //======================================

        // 判断是否已经达到最大连接数量
        if (m_xio_manager.count() >= m_xwct_config.xut_epoll_maxsockfds)
        {
            LOGE("[thread_index: %d] m_xio_manager.count()[%d] >= m_xwct_config.xut_epoll_maxsockfds[%d]",
                 xit_nthread, m_xio_manager.count(), m_xwct_config.xut_epoll_maxsockfds);
            break;
        }

        // 设置套接字为非阻塞模式
        xit_error = set_non_block(xfdt_sockfd);
        if (0 != xit_error)
        {
            LOGE("[thread_index: %d] set_non_block(xfdt_sockfd[%d]) return error : %d",
                 xit_nthread, xfdt_sockfd, xit_error);
            break;
        }

        // 向 IO 管理模块发出 “接收到套接字连接事件” 的通知
        xit_error = m_xio_manager.io_event_accept(xfdt_sockfd);
        if (0 != xit_error)
        {
            LOGE("[thread_index: %d] m_xio_manager.io_event_accept(xfdt_sockfd[%d]) return error : %d",
                 xit_nthread, xfdt_sockfd, xit_error);
            break;
        }

        // 使新增的套接字监听 可读 事件
        xit_error = add_sockfd_event(xfdt_sockfd, EPOLLET | EPOLLIN);
        if (0 != xit_error)
        {
            LOGE("[thread_index: %d] add_sockfd_event(xfdt_sockfd[%d], EPOLLET | EPOLLIN) return error : %d",
                 xit_nthread, xfdt_sockfd, xit_error);
            m_xio_manager.io_event_close(xfdt_sockfd);
            break;
        }

        //======================================

        m_xio_kpalive.post_event(x_kpalive_t::EIOA_JOINTO, xfdt_sockfd, X_NULL, get_time_tick());

        //======================================

        xit_error   = 0;
        xfdt_sockfd = X_INVALID_SOCKFD;
    } while (0);

    if (X_INVALID_SOCKFD != xfdt_sockfd)
    {
        sockfd_close(xfdt_sockfd);
        xfdt_sockfd = X_INVALID_SOCKFD;
    }

    return xit_error;
}

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
x_int32_t x_tcp_io_server_t::io_handle_close(x_int32_t xit_nthread, x_sockfd_t xfdt_sockfd)
{
    x_int32_t xit_error = m_xio_manager.io_event_close(xfdt_sockfd);
    if (0 != xit_error)
    {
        LOGE("[thread_index: %d]m_xio_manager.io_event_close(xfdt_sockfd[%d]) return error : %d",
             xit_nthread, xfdt_sockfd, xit_error);
    }

    m_xio_kpalive.post_event(x_kpalive_t::EIOA_REMOVE, xfdt_sockfd, X_NULL, 0);

    remove_off(xfdt_sockfd);

    return xit_error;
}

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
x_int32_t x_tcp_io_server_t::io_handle_read(x_int32_t xit_nthread, x_sockfd_t xfdt_sockfd)
{
    x_int32_t xit_error = m_xio_manager.io_event_read(xfdt_sockfd);
    if (0 != xit_error)
    {
        LOGE("[thread_index: %d]m_xio_manager.io_event_read(xfdt_sockfd[%d]) return error : %d",
             xit_nthread, xfdt_sockfd, xit_error);
    }

    return xit_error;
}

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
x_int32_t x_tcp_io_server_t::io_handle_write(x_int32_t xit_nthread, x_sockfd_t xfdt_sockfd)
{
    x_int32_t xit_error = m_xio_manager.io_event_write(xfdt_sockfd);
    if (0 != xit_error)
    {
        LOGE("[thread_index: %d]m_xio_manager.io_event_write(xfdt_sockfd[%d]) return error : %d",
             xit_nthread, xfdt_sockfd, xit_error);
    }
    else
    {
        // 去除 写就绪 事件
        xit_error = set_pollout(xfdt_sockfd, X_FALSE);
        if (0 != xit_error)
        {
            LOGE("set_pollout(xfdt_sockfd[%d], X_FALSE) return error : %d", xfdt_sockfd, xit_error);
        }
    }

    return xit_error;
}
