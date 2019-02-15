/**
 * @file    xftp_query.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xftp_query.cpp
 * 创建日期：2019年02月14日
 * 文件标识：
 * 文件摘要：提供 xftp 的信息查询服务的业务层工作对象。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年02月14日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#include "xcomm.h"
#include "xftp_query.h"

////////////////////////////////////////////////////////////////////////////////
// x_ftp_query_t

//====================================================================

// 
// x_ftp_query_t : constructor/destructor
// 

x_ftp_query_t::x_ftp_query_t(x_handle_t xht_manager, x_sockfd_t xfdt_sockfd)
    : x_super_t(xht_manager, xfdt_sockfd)
{

}

x_ftp_query_t::~x_ftp_query_t(void)
{

}

//====================================================================

// 
// x_ftp_query_t : overrides
// 

/**********************************************************/
/**
 * @brief 处理 “接收 IO 请求消息” 的事件。
 */
x_int32_t x_ftp_query_t::io_event_recved_xmsg(x_tcp_io_message_t & xio_message)
{
    x_int32_t xit_error = 0;

    x_io_msgctxt_t xio_msgctxt;
    if (IOCTX_ERR_OK == io_context_rinfo(xio_message.data(), xio_message.rlen(), &xio_msgctxt))
    {
        switch (xio_msgctxt.io_cmid)
        {
        case CMID_QUERY_LOGIN : xit_error = iocmd_login(xio_msgctxt.io_seqn, xio_msgctxt.io_dptr, xio_msgctxt.io_size); break;
        case CMID_QUERY_HBEAT : xit_error = iocmd_hbeat(xio_msgctxt.io_seqn, xio_msgctxt.io_dptr, xio_msgctxt.io_size); break;
        case CMID_QUERY_FLIST : xit_error = iocmd_flist(xio_msgctxt.io_seqn, xio_msgctxt.io_dptr, xio_msgctxt.io_size); break;

        default:
            break;
        }
    }

    return xit_error;
}

/**********************************************************/
/**
 * @brief 处理 “完成 IO 应答消息” 的事件。
 */
x_int32_t x_ftp_query_t::io_event_sended_xmsg(x_tcp_io_message_t & xio_message)
{
    return 0;
}

//====================================================================

// 
// x_ftp_query_t : internal invoking
// 

/**********************************************************/
/**
 * @brief 处理 IO 请求命令：登录。
 */
x_int32_t x_ftp_query_t::iocmd_login(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size)
{
    if (xut_size > 0)
        m_xstr_name = std::move(std::string((x_char_t *)xct_dptr, xut_size));
    else
        m_xstr_name = "{unknow}";

    x_io_msgctxt_t xio_msgctxt;
    xio_msgctxt.io_seqn = xut_seqn;
    xio_msgctxt.io_cmid = CMID_QUERY_LOGIN;
    xio_msgctxt.io_size = 0;
    xio_msgctxt.io_dptr = X_NULL;

    post_res_xmsg(xio_msgctxt);

    return 0;
}

/**********************************************************/
/**
 * @brief 处理 IO 请求命令：心跳。
 */
x_int32_t x_ftp_query_t::iocmd_hbeat(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size)
{
    x_io_msgctxt_t xio_msgctxt;
    xio_msgctxt.io_seqn = xut_seqn;
    xio_msgctxt.io_cmid = CMID_QUERY_HBEAT;
    xio_msgctxt.io_size = 0;
    xio_msgctxt.io_dptr = X_NULL;

    post_res_xmsg(xio_msgctxt);

    return 0;
}

/**********************************************************/
/**
 * @brief 处理 IO 请求命令：获取文件列表。
 */
x_int32_t x_ftp_query_t::iocmd_flist(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size)
{

    return 0;
}
