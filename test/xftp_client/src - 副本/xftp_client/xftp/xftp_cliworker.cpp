/**
 * @file    xftp_cliworker.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xftp_cliworker.cpp
 * 创建日期：2019年02月22日
 * 文件标识：
 * 文件摘要：XFTP 的客户端网络连接工作类（定义的接口类）。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年02月22日
 * 版本摘要：
 * 
 * 取代版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#include "stdafx.h"
#include "xftp_cliworker.h"

////////////////////////////////////////////////////////////////////////////////
// x_ftp_cliworker_t

//====================================================================

// 
// x_ftp_cliworker_t : common data types or invoking methods
// 

x_mempool_t x_ftp_cliworker_t::_S_mempool;  // 共用的内存池对象

//====================================================================

// 
// x_ftp_cliworker_t : constructor/destructor
// 

x_ftp_cliworker_t::x_ftp_cliworker_t(void)
    : m_xfdt_sockfd(X_INVALID_SOCKFD)
    , m_xbt_running(X_FALSE)
    , m_xbt_heart_enable(X_TRUE)
    , m_xut_heart_time(ECV_HEARTBEAT_TIME)
    , m_xut_seqno(0)
{

}

x_ftp_cliworker_t::~x_ftp_cliworker_t(void)
{
    if (is_startup())
    {
        shutdown();
    }
}

//====================================================================

// 
// x_ftp_cliworker_t : extensible interfaces
// 

/**********************************************************/
/**
 * @brief 投递心跳数据包信息。
 */
x_void_t x_ftp_cliworker_t::send_heartbeat(void)
{

}

/**********************************************************/
/**
 * @brief 分派应答 IO 消息，进行对应的处理操作。
 */
x_void_t x_ftp_cliworker_t::io_recved_msgctxt(const x_io_msgctxt_t & xio_msgctxt)
{

}

/**********************************************************/
/**
 * @brief 完成 IO 消息发送的通知接口。
 */
x_void_t x_ftp_cliworker_t::io_sended_msgctxt(const x_io_msgctxt_t & xio_msgctxt)
{

}

/**********************************************************/
/**
 * @brief IO 操作产生错误的通知接口。
 */
x_void_t x_ftp_cliworker_t::io_error_notify(x_int32_t xit_error, x_bool_t xbt_iosend)
{

}

//====================================================================

// 
// x_ftp_cliworker_t : public interfaces
// 

/**********************************************************/
/**
 * @brief 启动工作对象。
 *
 * @param [in ] xszt_host : 目标主机（服务器）的 IP 地址（四段式 IP 地址字符串）。
 * @param [in ] xwt_port  : 目标主机（服务器）的 端口号。
 *
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t x_ftp_cliworker_t::startup(x_cstring_t xszt_host, x_uint16_t xwt_port)
{
    x_int32_t xit_error = -1;

    do
    {
        //======================================

        shutdown();

        if (X_NULL == xszt_host || ('\0' == xszt_host[0]))
        {
            xit_error = -1;
            break;
        }

        //======================================
        // 区分 IP 与 域名 的连接

        std::vector< std::string > xvec_host;

        if (vx_ipv4_valid(xszt_host, X_NULL))
        {
            xvec_host.push_back(std::string(xszt_host));
        }
        else
        {
            xit_error = vx_gethostbyname(xszt_host, AF_INET, xvec_host);
            if (0 != xit_error)
            {
                break;
            }
        }

        //======================================
        // 发起 TCP 连接

        for (std::vector< std::string >::iterator itvec = xvec_host.begin();
             itvec != xvec_host.end();
             ++itvec)
        {
            m_xfdt_sockfd = tcp_connect(itvec->c_str(), xwt_port, ECV_CONNECT_TIMEOUT, &xit_error);
            if (X_INVALID_SOCKFD != m_xfdt_sockfd)
            {
                x_int32_t xit_option = 1;
                setsockopt(m_xfdt_sockfd, SOL_SOCKET, SO_KEEPALIVE, (const x_char_t *)&xit_option, sizeof(x_int32_t));
                setsockopt(m_xfdt_sockfd, IPPROTO_TCP, TCP_NODELAY, (const x_char_t *)&xit_option, sizeof(x_int32_t));

                break;
            }
        }

        if (X_INVALID_SOCKFD == m_xfdt_sockfd)
        {
            xit_error = -1;
            break;
        }

        //======================================
        // 启动相关工作线程

        try
        {
            m_xbt_running = X_TRUE;
            m_xthd_send = std::move(x_thread_t([this](void) -> x_void_t { thread_send_run(); }));
            m_xthd_recv = std::move(x_thread_t([this](void) -> x_void_t { thread_recv_run(); }));
        }
        catch (...)
        {
            STD_TRACE("thread exception!");
            xit_error = -1;
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
 * @brief 关闭工作对象。
 */
x_void_t x_ftp_cliworker_t::shutdown(void)
{
    m_xbt_running = X_FALSE;

    if (X_INVALID_SOCKFD != m_xfdt_sockfd)
    {
        sockfd_close(m_xfdt_sockfd);
        m_xfdt_sockfd = X_INVALID_SOCKFD;
    }

    {
        std::lock_guard< x_locker_t > xautolock_task(m_xlock_mqreq);
        m_xnotify_mqreq.notify_all();
    }

    if (m_xthd_send.joinable())
    {
        m_xthd_send.join();
    }

    if (m_xthd_recv.joinable())
    {
        m_xthd_recv.join();
    }

    {
        m_xlock_mqreq.lock();

        while (m_xmqueue_req.size() > 0)
        {
            mempool().recyc((mblock_t)m_xmqueue_req.front());
            m_xmqueue_req.pop_front();
        }

        m_xlock_mqreq.unlock();
    }

    mempool().release_timeout_memblock(0);
}

/**********************************************************/
/**
 * @brief 投递网络请求 IO 消息数据包。
 */
x_int32_t x_ftp_cliworker_t::post_req_xmsg(x_io_msgctxt_t & xio_msgctxt, x_bool_t xbt_seqn)
{
    if (xio_msgctxt.io_size > 0x0000FFFF)
    {
        return -1;
    }

    x_uint32_t  xut_io_size = IO_HDSIZE + xio_msgctxt.io_size;
    x_uchar_t * xct_io_dptr = (x_uchar_t *)mempool().alloc(xut_io_size);
    if (X_NULL == xct_io_dptr)
    {
        return -1;
    }

    if (xbt_seqn)
    {
        xio_msgctxt.io_seqn = m_xut_seqno.fetch_add(1);
    }

    if (IOCTX_ERR_OK != io_set_context(xct_io_dptr, xut_io_size, &xio_msgctxt))
    {
        mempool().recyc((mblock_t)xct_io_dptr);
        xct_io_dptr = X_NULL;
        xut_io_size = 0;
        return -1;
    }

    {
        std::lock_guard< x_locker_t > xautolock(m_xlock_mqreq);
        m_xmqueue_req.push_back((x_io_msghead_t *)xct_io_dptr);

        m_xnotify_mqreq.notify_all();

        xct_io_dptr = X_NULL;
        xut_io_size = 0;
    }

    return 0;
}

//====================================================================

// 
// x_ftp_cliworker_t : internal invoking
// 

/**********************************************************/
/**
 * @brief 数据发送操作的工作线程的入口函数。
 */
x_void_t x_ftp_cliworker_t::thread_send_run(void)
{
    x_int32_t  xit_error = 0;
    x_uint32_t xut_iolen = 0;

    x_io_msghead_t * xio_msghead_ptr = X_NULL;

    while (m_xbt_running)
    {
        //======================================
        // 等待请求的 IO 消息加入队列

        if (m_xmqueue_req.empty())
        {
            std::unique_lock< x_locker_t > xunique_locker(m_xlock_mqreq);
            m_xnotify_mqreq.wait(
                xunique_locker,
                [this](void) -> bool
            {
                return (!m_xmqueue_req.empty() || !m_xbt_running);
            });
        }

        //======================================
        // 提取待发送的 IO 消息

        xio_msghead_ptr = X_NULL;
        {
            std::lock_guard< x_locker_t > xautolock(m_xlock_mqreq);

            if (m_xmqueue_req.size() > 0)
            {
                xio_msghead_ptr = m_xmqueue_req.front();
                m_xmqueue_req.pop_front();
            }
        }

        if (X_NULL == xio_msghead_ptr)
        {
            continue;
        }

        //======================================
        // 执行数据投递（发送）流程

        xut_iolen = IO_HDSIZE + vx_ntohs(xio_msghead_ptr->io_size);

        tcp_send(m_xfdt_sockfd,
                 (x_char_t *)xio_msghead_ptr,
                 (x_int32_t)xut_iolen,
                 0,
                 &xit_error);

        if (0 == xit_error)
        {
            x_io_msgctxt_t xio_msgctxt;
            io_get_context((x_uchar_t *)xio_msghead_ptr, xut_iolen, &xio_msgctxt);
            io_sended_msgctxt(xio_msgctxt);
        }
        else
        {
            io_error_notify(xit_error, X_TRUE);
        }

        mempool().recyc((mblock_t)xio_msghead_ptr);
        xio_msghead_ptr = X_NULL;

        //======================================
    }

    m_xbt_running = X_FALSE;
}

/**********************************************************/
/**
 * @brief 数据接收操作的工作线程的入口函数。
 */
x_void_t x_ftp_cliworker_t::thread_recv_run(void)
{
    x_int32_t xit_error = -1;

    fd_set fd_reader;
    fd_set fd_error;
    struct timeval tmval;

    x_uint32_t xut_read_bytes = 0;
    x_uint32_t xut_data_size = 0;
    std::vector< x_uchar_t > xvec_buffer;
    xvec_buffer.resize(1024 * 1024, 0);
    x_uchar_t * xct_io_dptr = xvec_buffer.data();
    x_uint32_t  xut_io_size = (x_uint32_t)xvec_buffer.size();

    // 心跳包计时器
    x_uint64_t xut_heart_tick = get_time_tick();
    x_uint64_t xut_value_time = xut_heart_tick;

    while (m_xbt_running)
    {
        FD_ZERO(&fd_reader);
        FD_ZERO(&fd_error);
        FD_SET(m_xfdt_sockfd, &fd_reader);
        FD_SET(m_xfdt_sockfd, &fd_error);

        // check if the m_xfdt_sockfd is ready
        tmval.tv_sec = 0;
        tmval.tv_usec = get_heartbeat_time() * 1000;
        select(0, &fd_reader, X_NULL, &fd_error, &tmval);

        // 产生错误
        if (0 != FD_ISSET(m_xfdt_sockfd, &fd_error))
        {
            io_error_notify(WSAGetLastError(), X_FALSE);
            break;
        }

        // 数据接收检测
        if (0 != FD_ISSET(m_xfdt_sockfd, &fd_reader))
        {
            x_int32_t xit_nread = recv(m_xfdt_sockfd, (x_char_t *)(xct_io_dptr + xut_read_bytes), xut_io_size - xut_read_bytes, 0);
            if (xit_nread <= 0)
            {
                io_error_notify(WSAGetLastError(), X_FALSE);
                break;
            }

            if (!m_xbt_running)
            {
                break;
            }

            xut_read_bytes += xit_nread;

            // 回调数据至应用层
            xut_data_size = xut_read_bytes;
            io_recved(xct_io_dptr, xut_data_size);

            // 继续执行异步接收操作
            if (xut_data_size < xut_read_bytes)
            {
                xut_read_bytes -= xut_data_size;
                memmove(xct_io_dptr, xct_io_dptr + xut_data_size, xut_read_bytes);
            }
            else
            {
                xut_read_bytes = 0;
            }

            // 接收到新的数据，刷新心跳计时器，尽可能忽略发送心跳包操作
            if (xit_nread > 0)
            {
                xut_heart_tick = get_time_tick();
            }
        }

        // 投递心跳包
        if (is_enable_auto_heartbeat())
        {
            xut_value_time = get_time_tick();
            if ((xut_value_time < xut_heart_tick) || (xut_value_time > (xut_heart_tick + (x_uint64_t)get_heartbeat_time())))
            {
                xut_heart_tick = xut_value_time;
                send_heartbeat();
            }
        }
    }

    m_xbt_running = X_FALSE;
}

/******************************************************************************/
/**
 * @brief 接收到网络的数据 IO 流。
 *
 * @param [in    ] xct_io_dptr : 网络数据流缓存。
 * @param [in,out] xut_io_dlen : 数据流缓存长度（入参，表示有效的数据流字节数；回参，表示处理到的字节数）。
 *
 * @return x_int32_t
 *         - 返回 -1，表示该连接对象的套接字失效，将进行关闭操作；
 *         - 返回  0，表示该连接对象的套接字有效，将继续保活操作。
 */
x_int32_t x_ftp_cliworker_t::io_recved(x_uchar_t * const xct_io_dptr, x_uint32_t & xut_io_dlen)
{
    x_int32_t xit_err = 0;

    x_uint32_t xut_dlen = xut_io_dlen;

    do
    {
        // 查找数据包前缀
        x_int32_t xit_vpos = io_find_ldcode(xct_io_dptr, xut_dlen);

        // 若无前缀，为非法数据包，立即判定此为恶意连接，终止操作
        if (-1 == xit_vpos)
        {
            STD_TRACE("io_find_ldcode(xct_io_dptr, xut_dlen[%d]) return -1", xut_dlen);

            xut_io_dlen = 0xFFFFFFFF;
            xit_err = -1;
            break;
        }

        x_io_msgctxt_t xio_msgctxt;

        // 数据解包操作（逐个数据包的读取和投递工作）
        x_int32_t xit_read_pos = xit_vpos;
        while (xit_read_pos < (x_int32_t)xut_dlen)
        {
            // 查找缓存中的首个数据包
            xit_vpos = io_find_context(xct_io_dptr + xit_read_pos, xut_dlen - xit_read_pos, &xio_msgctxt);
            if (-1 == xit_vpos)
            {
                STD_TRACE("io_find_context(xct_io_dptr + xit_read_pos[%d], xut_dlen[%d] - xit_read_pos[%d], ...) return -1",
                          xit_read_pos, xut_dlen, xit_read_pos);
                break;
            }

            // 投递分包
            io_recved_msgctxt(xio_msgctxt);

            xit_read_pos += (xit_vpos + IO_HDSIZE + xio_msgctxt.io_size);
        }

        xut_io_dlen = xit_read_pos;
    } while (0);

    return xit_err;
}

/**********************************************************/
/**
 * @brief 分派应答 IO 消息，进行对应的处理操作。
 */
x_void_t x_ftp_cliworker_t::io_recved_msgctxt(const x_io_msgctxt_t & xio_msgctxt)
{

}

