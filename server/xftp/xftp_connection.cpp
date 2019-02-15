/**
 * @file    xftp_connection.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xftp_connection.cpp
 * 创建日期：2019年01月24日
 * 文件标识：
 * 文件摘要：所有 xftp 服务器的业务层连接对象的基类（定义 IO 消息分包规则）。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年01月24日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#include "xcomm.h"
#include "xftp_connection.h"

////////////////////////////////////////////////////////////////////////////////
// x_ftp_channel_t

//====================================================================

// 
// x_ftp_channel_t : common invoking
// 

/**********************************************************/
/**
 * @brief IO 消息的分割处理接口（分包操作接口）。
 * 
 * @param [in,out] xio_message : 入参，待分割的 IO 消息对象；
 *                               回参，分割后剩余的部分（半包，也有可能为空）。
 * @param [out   ] xlst_iomsg  : 存储分割出来的多个完整 IO 消息对象。
 * 
 * @return x_uint32_t
 *         - 返回分割到的 IO 消息数量。
 */
x_uint32_t x_ftp_channel_t::xmsg_split(x_tcp_io_message_t & xio_message,
                                       std::list< x_tcp_io_message_t > & xlst_iomsg)
{
    x_uchar_t * xct_dptr = xio_message.data();
    x_uint32_t  xut_dlen = xio_message.rlen();
    x_int32_t   xit_vpos = -1;
    x_int32_t   xit_vlen = 0;

    x_io_msgctxt_t xio_msgctxt;

    if ((X_NULL == xct_dptr) || (xut_dlen <= 0))
    {
        return 0;
    }

    while (-1 != (xit_vpos = io_find_context(xct_dptr, xut_dlen, &xio_msgctxt)))
    {
        xct_dptr = xct_dptr + xit_vpos;
        xut_dlen = IO_HDSIZE + xio_msgctxt.io_size;

        xlst_iomsg.push_back(
            std::forward< x_tcp_io_message_t >(
                x_tcp_io_message_t(xct_dptr + xit_vpos, IO_HDSIZE + xio_msgctxt.io_size)));

        xit_vlen = xit_vpos + IO_HDSIZE + xio_msgctxt.io_size;
        xct_dptr = xct_dptr + xit_vlen;
        xut_dlen = xut_dlen - xit_vlen;
    }

    if (xio_message.rlen() != xut_dlen)
    {
        if ((X_NULL != xct_dptr) && (xut_dlen > 0))
        {
            // 将剩余的半包消息返还回去
            xio_message = std::move(x_tcp_io_message_t(xct_dptr, xut_dlen));
        }
        else
        {
            // 清空
            xio_message = x_tcp_io_message_t();
        }
    }

    return (x_uint32_t)xlst_iomsg.size();
}

//====================================================================

// 
// x_ftp_channel_t : constructor/destructor
// 

x_ftp_channel_t::x_ftp_channel_t(x_handle_t xht_manager, x_sockfd_t xfdt_sockfd)
    : x_tcp_io_channel_t(xht_manager, xfdt_sockfd)
{

}

x_ftp_channel_t::~x_ftp_channel_t(void)
{

}

//====================================================================

// 
// x_ftp_channel_t : overrides
// 

/**********************************************************/
/**
 * @brief 投递请求操作的 IO 消息。
 * 
 * @param [in,out] xio_message : 投递的 IO 消息。
 * 
 * @return x_int32_t
 *         - 返回值 >= 0，表示操作产生的事件数量。
 *         - 返回值 <  0，表示产生错误，后续则可关闭该业务层工作对象。
 */
x_int32_t x_ftp_channel_t::post_req_xmsg(x_tcp_io_message_t & xio_message)
{
    std::list< x_tcp_io_message_t > xlst_iomsg;

    //======================================

    if (xio_message.rlen() < IO_HDSIZE)
    {
        return 0;
    }

    // 若整个 IO 消息数据包中无 前导码，则可判断为 非法操作
    if (-1 == io_find_ldcode(xio_message.data(), xio_message.rlen()))
    {
        LOGE("[fd:%d]io_find_ldcode(xio_message.data(), xio_message.rlen()[%d]) return -1!",
                get_sockfd(), xio_message.rlen());
        return -1;
    }

    //======================================
    // 分包操作

    if (xmsg_split(xio_message, xlst_iomsg) <= 0)
    {
        // 若 IO 消息总长度达到上限值（64KB），
        // 仍未能分包，则判断为 非法数据包
        if (xio_message.rlen() >= 0xFFFF)
        {
            LOGE("[fd:%d]xio_message.rlen()[%d] >= 0xFFFF",
                 get_sockfd(), xio_message.rlen());
            return -1;
        }
        else
        {
            return 0;
        }
    }

    //======================================
    // 加入到请求操作的 IO 消息队列

    x_mqautolock_t xautolock(m_xmqueue_req);
    for (std::list< x_tcp_io_message_t >::iterator itlst = xlst_iomsg.begin();
            itlst != xlst_iomsg.end();
            ++itlst)
    {
        m_xmqueue_req.push(std::move(*itlst));
    }

    //======================================

    return (x_int32_t)xlst_iomsg.size();
}

//====================================================================

// 
// x_ftp_channel_t : public interfaces
// 


//====================================================================

// 
// x_ftp_channel_t : internal invoking
// 

/**********************************************************/
/**
 * @brief 投递应答操作的 IO 消息（加入 IO 应答消息队列，等待发送）。
 * 
 * @param [in ] xio_msgctxt : IO 应答消息的上下文描述信息。
 * 
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t x_ftp_channel_t::post_res_xmsg(const x_io_msgctxt_t & xio_msgctxt)
{
    if (X_NULL == get_manager())
    {
        LOGE("X_NULL == get_manager()");
        return -1;
    }

    if (xio_msgctxt.io_size > 0x0000FFFF)
    {
        LOGE("xio_msgctxt.io_size[%d] > 0x0000FFFF", xio_msgctxt.io_size);
        return -1;
    }

    x_tcp_io_message_t xio_message(IO_HDSIZE + xio_msgctxt.io_size);
    if (IOCTX_ERR_OK != io_set_context(xio_message.data(), xio_message.capacity(), &xio_msgctxt))
    {
        return -1;
    }

    xio_message.reset(IO_HDSIZE + xio_msgctxt.io_size, 0);

    push_res_xmsg(std::move(xio_message));

    return 0;
}

