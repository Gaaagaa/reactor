/**
 * @file    xtcp_io_message.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xtcp_io_message.h
 * 创建日期：2019年01月17日
 * 文件标识：
 * 文件摘要：定义网络 IO 消息的接口类。
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

#ifndef __XTCP_IO_MESSAGE_H__
#define __XTCP_IO_MESSAGE_H__

#include "xmempool.h"

////////////////////////////////////////////////////////////////////////////////
// x_tcp_io_message_t

/**
 * @class x_tcp_io_message_t
 * @brief 网络 IO 消息的接口类。
 */
class x_tcp_io_message_t final
{
    // constructor/destructor
public:
    explicit x_tcp_io_message_t(void);
    explicit x_tcp_io_message_t(x_uint32_t xut_buf_size);
    explicit x_tcp_io_message_t(x_uchar_t * xct_buf_dptr, x_uint32_t xut_buf_size);
    virtual ~x_tcp_io_message_t(void);

    x_tcp_io_message_t(x_tcp_io_message_t && xobject);
    x_tcp_io_message_t & operator = (x_tcp_io_message_t && xobject);
    x_tcp_io_message_t(const x_tcp_io_message_t & xobject);
    x_tcp_io_message_t & operator = (const x_tcp_io_message_t & xobject);

    // common invoking
public:
    /**********************************************************/
    /**
     * @brief 返回共用的内存池对象。
     */
    static inline x_mempool_t & xmsg_mempool(void) { return _S_xmpt_comm; }

private:
    static x_mempool_t _S_xmpt_comm;    ///< 所有 IO 消息对象共用的内存池

    // public interfaces
public:
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
    x_uint32_t nio_read(x_sockfd_t xfdt_sockfd, x_uint32_t xut_max_len, x_int32_t & xit_error);

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
    x_uint32_t nio_write(x_sockfd_t xfdt_sockfd, x_uint32_t xut_max_len, x_int32_t & xit_error);

    /**********************************************************/
    /**
     * @brief 与另外的 IO 消息对象交换数据。
     */
    x_void_t swap(x_tcp_io_message_t & xobject);

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
    x_uint32_t append(x_uchar_t * xct_buf_dptr, x_uint32_t xut_buf_size);

    /**********************************************************/
    /**
     * @brief IO 消息对象的数据缓存。
     */
    inline x_uchar_t * data(void) { return m_xct_buf_dptr; }

    /**********************************************************/
    /**
     * @brief IO 消息对象的数据缓存。
     */
    inline const x_uchar_t * data(void) const { return m_xct_buf_dptr; }

    /**********************************************************/
    /**
     * @brief IO 消息对象的数据缓存最大容量（字节数）。
     */
    inline x_uint32_t capacity(void) const { return m_xut_buf_size; }

    /**********************************************************/
    /**
     * @brief IO 消息对象可读取的数据长度（字节数）。
     */
    inline x_uint32_t rlen(void) const { return m_xut_msg_rlen; }

    /**********************************************************/
    /**
     * @brief IO 消息对象已写入的数据长度（字节数）。
     */
    inline x_uint32_t wlen(void) const { return m_xut_msg_wlen; }

    /**********************************************************/
    /**
     * @brief 是否可继续读取数据。
     */
    inline x_bool_t is_readable(void) const { return (m_xut_buf_size > m_xut_msg_rlen); }

    /**********************************************************/
    /**
     * @brief 是否可继续写入数据。
     */
    inline x_bool_t is_writable(void) const { return (m_xut_msg_rlen > m_xut_msg_wlen); }

    /**********************************************************/
    /**
     * @brief 是否为空。
     */
    inline x_bool_t is_empty(void) const { return (0 == m_xut_msg_rlen); }

    /**********************************************************/
    /**
     * @brief 缓存是否为 null 。
     */
    inline x_bool_t is_null(void) const { return (X_NULL == m_xct_buf_dptr); }

    /**********************************************************/
    /**
     * @brief 重置 IO 相关的描述信息。
     */
    inline x_void_t reset(x_uint16_t xut_rlen = 0, x_uint32_t xut_wlen = 0)
    {
        m_xut_msg_rlen = xut_rlen;
        m_xut_msg_wlen = xut_wlen;
    }

    // internal invoking
private:
    /**********************************************************/
    /**
     * @brief 扩大数据缓存。
     */
    x_bool_t grow_up(x_uint32_t xut_grow_size);

    // data members
private:
    x_uchar_t   * m_xct_buf_dptr;   ///< 数据缓存地址
    x_uint32_t    m_xut_buf_size;   ///< 数据缓存大小
    x_uint32_t    m_xut_msg_rlen;   ///< 消息可读取的数据长度
    x_uint32_t    m_xut_msg_wlen;   ///< 消息已写入的数据长度
};

////////////////////////////////////////////////////////////////////////////////

#endif // __XTCP_IO_MESSAGE_H__
