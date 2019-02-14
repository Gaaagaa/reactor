/**
 * @file    xtcp_io_message.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xtcp_io_message.cpp
 * 创建日期：2019年01月17日
 * 文件标识：
 * 文件摘要：实现网络 IO 消息的接口类。
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
#include "xtcp_io_message.h"

#include <sys/types.h>
#include <sys/socket.h>

////////////////////////////////////////////////////////////////////////////////
// x_tcp_io_message_t

//====================================================================

// 
// x_tcp_io_message_t : common invoking
// 

// 所有 IO 消息对象共用的内存池
x_mempool_t x_tcp_io_message_t::_S_xmpt_comm;

//====================================================================

// 
// x_tcp_io_message_t : constructor/destructor
// 

x_tcp_io_message_t::x_tcp_io_message_t(void)
    : m_xct_buf_dptr(X_NULL)
    , m_xut_buf_size(0)
    , m_xut_msg_rlen(0)
    , m_xut_msg_wlen(0)
{

}

x_tcp_io_message_t::x_tcp_io_message_t(x_uint32_t xut_buf_size)
    : m_xct_buf_dptr(X_NULL)
    , m_xut_buf_size(0)
    , m_xut_msg_rlen(0)
    , m_xut_msg_wlen(0)
{
    if (xut_buf_size > 0)
    {
        m_xut_buf_size = x_mempool_t::align_size(xut_buf_size);
        XVERIFY(X_NULL != (m_xct_buf_dptr = xmsg_mempool().alloc(m_xut_buf_size)));
    }
}

x_tcp_io_message_t::x_tcp_io_message_t(x_uchar_t * xct_buf_dptr, x_uint32_t xut_buf_size)
    : m_xct_buf_dptr(X_NULL)
    , m_xut_buf_size(0)
    , m_xut_msg_rlen(0)
    , m_xut_msg_wlen(0)
{
    if ((X_NULL != xct_buf_dptr) && (xut_buf_size > 0))
    {
        m_xut_buf_size = x_mempool_t::align_size(xut_buf_size);
        XVERIFY(X_NULL != (m_xct_buf_dptr = xmsg_mempool().alloc(m_xut_buf_size)));

        memcpy(m_xct_buf_dptr, xct_buf_dptr, xut_buf_size);
        m_xut_msg_rlen = xut_buf_size;
    }
}

x_tcp_io_message_t::~x_tcp_io_message_t(void)
{
    if (X_NULL != m_xct_buf_dptr)
    {
        XVERIFY(xmsg_mempool().recyc(m_xct_buf_dptr));

        m_xct_buf_dptr = X_NULL;
        m_xut_buf_size = 0;
        m_xut_msg_rlen = 0;
        m_xut_msg_wlen = 0;
    }
}

x_tcp_io_message_t::x_tcp_io_message_t(x_tcp_io_message_t && xobject)
    : m_xct_buf_dptr(X_NULL)
    , m_xut_buf_size(0)
    , m_xut_msg_rlen(0)
    , m_xut_msg_wlen(0)
{
    *this = std::forward< x_tcp_io_message_t >(xobject);
}

x_tcp_io_message_t & x_tcp_io_message_t::operator = (x_tcp_io_message_t && xobject)
{
    if (this == &xobject)
        return *this;

    if (X_NULL != m_xct_buf_dptr)
    {
        XVERIFY(xmsg_mempool().recyc(m_xct_buf_dptr));
    }

    m_xct_buf_dptr = xobject.m_xct_buf_dptr;
    m_xut_buf_size = xobject.m_xut_buf_size;
    m_xut_msg_rlen = xobject.m_xut_msg_rlen;
    m_xut_msg_wlen = xobject.m_xut_msg_wlen;

    xobject.m_xct_buf_dptr = X_NULL;
    xobject.m_xut_buf_size = 0;
    xobject.m_xut_msg_rlen = 0;
    xobject.m_xut_msg_wlen = 0;

    return *this;
}

x_tcp_io_message_t::x_tcp_io_message_t(const x_tcp_io_message_t & xobject)
    : m_xct_buf_dptr(X_NULL)
    , m_xut_buf_size(0)
    , m_xut_msg_rlen(0)
    , m_xut_msg_wlen(0)
{
    *this = xobject;
}

x_tcp_io_message_t & x_tcp_io_message_t::operator = (const x_tcp_io_message_t & xobject)
{
    if (this == &xobject)
        return *this;

    if (X_NULL != m_xct_buf_dptr)
    {
        XVERIFY(xmsg_mempool().recyc(m_xct_buf_dptr));

        m_xct_buf_dptr = X_NULL;
        m_xut_buf_size = 0;
        m_xut_msg_rlen = 0;
        m_xut_msg_wlen = 0;
    }

    if ((X_NULL != xobject.m_xct_buf_dptr) && (xobject.m_xut_buf_size > 0))
    {
        m_xut_buf_size = x_mempool_t::align_size(xobject.m_xut_buf_size);
        XVERIFY(X_NULL != (m_xct_buf_dptr = xmsg_mempool().alloc(m_xut_buf_size)));

        if (xobject.m_xut_msg_rlen > 0)
        {
            memcpy(m_xct_buf_dptr, xobject.m_xct_buf_dptr, xobject.m_xut_msg_rlen);
        }

        m_xut_msg_rlen = xobject.m_xut_msg_rlen;
        m_xut_msg_wlen = xobject.m_xut_msg_wlen;
    }

    return *this;
}

//====================================================================

// 
// x_tcp_io_message_t : public interfaces
// 

/**********************************************************/
/**
 * @brief 网络IO消息对象 从 非阻塞模式的套接字 读取数据。
 * 
 * @param [in ] xfdt_sockfd : 目标操作的非阻塞套接字。
 * @param [in ] xut_max_len : 限制操作的数据最大长度（字节数）。
 * @param [out] xit_error   : 操作返回的错误码。
 * 
 * @return x_uint32_t
 *         - 返回读取到的字节数。
 */
x_uint32_t x_tcp_io_message_t::nio_read(x_sockfd_t xfdt_sockfd, x_uint32_t xut_max_len, x_int32_t & xit_error)
{
    x_uint32_t xut_count = 0;
    x_int32_t  xit_bytes = 0;

    xit_error = 0;
    while (xut_count < xut_max_len)
    {
        if (!is_readable())
        {
            if (!grow_up((0 != m_xut_buf_size) ? m_xut_buf_size : x_mempool_t::ECV_ALLOC_SMALLER))
            {
                break;
            }
        }

        xit_bytes = (x_int32_t)::recv(xfdt_sockfd,
                                      m_xct_buf_dptr + m_xut_msg_rlen,
                                      m_xut_buf_size - m_xut_msg_rlen,
                                      0);
        if (xit_bytes > 0)
        {
            xut_count += xit_bytes;
            m_xut_msg_rlen += xit_bytes;
        }
        else if (0 == xit_bytes)
        {
            xit_error = -1;
            break;
        }
        else if (-1 == xit_bytes)
        {
            if ((EAGAIN == errno) || (EWOULDBLOCK == errno) || (EINTR == errno))
                xit_error = EAGAIN;
            else
                xit_error = errno;
            break;
        }
    }

    return xut_count;
}

/**********************************************************/
/**
 * @brief 网络IO消息对象 向 非阻塞模式的套接字 写入数据。
 * 
 * @param [in ] xfdt_sockfd : 目标操作的非阻塞套接字。
 * @param [in ] xut_max_len : 限制操作的数据最大长度（字节数）。
 * @param [out] xit_error   : 操作返回的错误码。
 * 
 * @return x_uint32_t
 *         - 返回已写入的字节数。
 */
x_uint32_t x_tcp_io_message_t::nio_write(x_sockfd_t xfdt_sockfd, x_uint32_t xut_max_len, x_int32_t & xit_error)
{
    x_uint32_t xut_count = 0;
    x_int32_t  xit_bytes = 0;

    xit_error = 0;
    while (xut_count < xut_max_len)
    {
        if (!is_writable())
        {
            break;
        }

        xit_bytes = (x_int32_t)::send(xfdt_sockfd,
                                      m_xct_buf_dptr + m_xut_msg_wlen,
                                      m_xut_msg_rlen - m_xut_msg_wlen,
                                      0);
        if (-1 == xit_bytes)
        {
            if ((EAGAIN == errno) || (EWOULDBLOCK == errno) || (EINTR == errno))
                xit_error = EAGAIN;
            else
                xit_error = errno;
            break;
        }

        xut_count += xit_bytes;
        m_xut_msg_wlen += xit_bytes;
    }

    return xut_count;
}

/**********************************************************/
/**
 * @brief 与另外的 IO 消息对象交换数据。
 */
x_void_t x_tcp_io_message_t::swap(x_tcp_io_message_t & xobject)
{
    if (this == &xobject)
        return;

    x_tcp_io_message_t xswap_object(std::move(xobject));
    xobject = std::move(*this);
    *this   = std::move(xswap_object);
}

/**********************************************************/
/**
 * @brief 附加缓存数据到消息数据的结尾处。
 * 
 * @param [in ] xct_buf_dptr : 附加的缓存数据。
 * @param [in ] xut_buf_size : 附加的缓存数据大小。
 * 
 * @return x_uint32_t
 *         - 返回 附加的字节数。
 */
x_uint32_t x_tcp_io_message_t::append(x_uchar_t * xct_buf_dptr, x_uint32_t xut_buf_size)
{
    if ((X_NULL == xct_buf_dptr) || (xut_buf_size <= 0))
    {
        return 0;
    }

    if ((m_xut_msg_rlen + xut_buf_size) > m_xut_buf_size)
    {
        if (!grow_up((m_xut_msg_rlen + xut_buf_size) - m_xut_buf_size))
        {
            return 0;
        }
    }

    memcpy(m_xct_buf_dptr + m_xut_msg_rlen, xct_buf_dptr, xut_buf_size);
    m_xut_msg_rlen += xut_buf_size;

    return xut_buf_size;
}

//====================================================================

// 
// x_tcp_io_message_t : internal invoking
// 

/**********************************************************/
/**
 * @brief 扩大数据缓存。
 */
x_bool_t x_tcp_io_message_t::grow_up(x_uint32_t xut_grow_size)
{
    x_uint32_t  xut_new_size = m_xut_buf_size + xut_grow_size;
    x_uchar_t * xct_new_dptr = X_NULL;

    if (0 == xut_grow_size)
    {
        return X_TRUE;
    }

    xut_new_size = x_mempool_t::align_size(m_xut_buf_size + xut_grow_size);
    xct_new_dptr = xmsg_mempool().alloc(xut_new_size);
    if (X_NULL == xct_new_dptr)
    {
        return X_FALSE;
    }

    if (X_NULL != m_xct_buf_dptr)
    {
        if (m_xut_msg_rlen > 0)
            memcpy(xct_new_dptr, m_xct_buf_dptr, m_xut_msg_rlen);
        XVERIFY(xmsg_mempool().recyc(m_xct_buf_dptr));
    }

    m_xct_buf_dptr = xct_new_dptr;
    m_xut_buf_size = xut_new_size;

    return X_TRUE;
}
