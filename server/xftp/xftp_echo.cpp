/**
 * @file    xftp_echo.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xftp_echo.cpp
 * 创建日期：2019年01月22日
 * 文件标识：
 * 文件摘要：提供 ECHO 测试服务的业务层工作对象。
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

#include "xcomm.h"
#include "xftp_echo.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

////////////////////////////////////////////////////////////////////////////////
// x_ftp_echo_t

//====================================================================

// 
// x_ftp_echo_t : common invoking
// 

static_assert((x_int32_t)x_ftp_echo_t::ECV_CONNECTION_TYPE ==
              (x_int32_t)x_ftp_echo_t::CMID_ECHO_LOGIN,
              "ECV_CONNECTION_TYPE == CMID_ECHO_LOGIN");

/**********************************************************/
/**
 * @brief x_ftp_echo_t 对象创建接口。
 * 
 * @param [in ] xht_manager : 业务层工作对象所隶属的 IO 管理模块句柄。
 * @param [in ] xfdt_sockfd : 业务层工作对象的 套接字描述符。
 * @param [in ] xht_msgctxt : 指向创建业务层工作对象的 IO 请求消息（首个 IO 消息）。
 * @param [out] xht_channel : 操作成功所返回的 x_tcp_io_channel_t 对象句柄。
 * 
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t x_ftp_echo_t::create(x_handle_t xht_manager,
                               x_sockfd_t xfdt_sockfd,
                               x_handle_t xht_msgctxt,
                               x_handle_t & xht_channel)
{
    //======================================

    x_ftp_echo_t * xthis_ptr = new x_ftp_echo_t(xht_manager, xfdt_sockfd);
    xht_channel = (x_handle_t)(xthis_ptr);
    XASSERT(X_NULL != xht_channel);

    //======================================

    x_uint32_t xut_value = 1;
    setsockopt(xfdt_sockfd, SOL_SOCKET, SO_KEEPALIVE, (const x_char_t *)&xut_value, sizeof(x_uint32_t));
    setsockopt(xfdt_sockfd, SOL_TCP   , TCP_NODELAY , (const x_char_t *)&xut_value, sizeof(x_uint32_t));

    //======================================

    return 0;
}

//====================================================================

// 
// x_ftp_echo_t : constructor/destructor
// 

x_ftp_echo_t::x_ftp_echo_t(x_handle_t xht_manager, x_sockfd_t xfdt_sockfd)
    : x_ftp_connection_t(xht_manager, xfdt_sockfd)
{

}

x_ftp_echo_t::~x_ftp_echo_t(void)
{

}

//====================================================================

// 
// x_ftp_echo_t : overrides
// 

/**********************************************************/
/**
 * @brief 处理 “接收 IO 请求消息” 的事件。
 */
x_int32_t x_ftp_echo_t::io_event_recved_xmsg(x_tcp_io_message_t & xio_message)
{
    x_int32_t xit_error = 0;

    x_io_msgctxt_t xio_msgctxt;
    if (IOCTX_ERR_OK == io_context_rinfo(xio_message.data(), xio_message.rlen(), &xio_msgctxt))
    {
        switch (xio_msgctxt.io_cmid)
        {
        case CMID_ECHO_LOGIN : xit_error = iocmd_login(xio_msgctxt.io_seqn, xio_msgctxt.io_dptr, xio_msgctxt.io_size); break;
        case CMID_ECHO_HBEAT : xit_error = iocmd_hbeat(xio_msgctxt.io_seqn, xio_msgctxt.io_dptr, xio_msgctxt.io_size); break;
        case CMID_ECHO_TEXT  : xit_error = iocmd_text (xio_msgctxt.io_seqn, xio_msgctxt.io_dptr, xio_msgctxt.io_size); break;

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
x_int32_t x_ftp_echo_t::io_event_sended_xmsg(x_tcp_io_message_t & xio_message)
{
    return 0;
}

//====================================================================

// 
// x_ftp_echo_t : internal invoking
// 

/**********************************************************/
/**
 * @brief 处理 IO 请求命令：登录。
 */
x_int32_t x_ftp_echo_t::iocmd_login(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size)
{
    if (xut_size > 0)
        m_xstr_name = std::move(std::string((x_char_t *)xct_dptr, xut_size));
    else
        m_xstr_name = "{unknow}";

    x_io_msgctxt_t xio_msgctxt;
    xio_msgctxt.io_seqn = xut_seqn;
    xio_msgctxt.io_cmid = CMID_ECHO_LOGIN;
    xio_msgctxt.io_size = 0;
    xio_msgctxt.io_dptr = X_NULL;

    post_res_xmsg(xio_msgctxt);

    return 0;
}

/**********************************************************/
/**
 * @brief 处理 IO 请求命令：心跳。
 */
x_int32_t x_ftp_echo_t::iocmd_hbeat(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size)
{
    x_io_msgctxt_t xio_msgctxt;
    xio_msgctxt.io_seqn = xut_seqn;
    xio_msgctxt.io_cmid = CMID_ECHO_HBEAT;
    xio_msgctxt.io_size = 0;
    xio_msgctxt.io_dptr = X_NULL;

    post_res_xmsg(xio_msgctxt);

    return 0;
}

/**********************************************************/
/**
 * @brief 处理 IO 请求命令：文本内容反射。
 */
x_int32_t x_ftp_echo_t::iocmd_text(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size)
{
    if ((X_NULL != xct_dptr) && (xut_size > sizeof(x_uint64_t)))
    {
        std::string xecho_text((x_char_t *)(xct_dptr + sizeof(x_uint64_t)), xut_size - sizeof(x_uint64_t));

        STD_TRACE("[%d, %d][%s:%d] %s echo[%d] : %s",
                  get_pid(),
                  get_tid(),
                  sockfd_remote_ip(get_sockfd(), LOG_BUF(64), 64),
                  sockfd_remote_port(get_sockfd()),
                  m_xstr_name.c_str(),
                  xut_size,
                  xecho_text.c_str());

#if 1
        x_io_msgctxt_t xio_msgctxt;
        xio_msgctxt.io_seqn = xut_seqn;
        xio_msgctxt.io_cmid = CMID_ECHO_TEXT;
        xio_msgctxt.io_size = xut_size;
        xio_msgctxt.io_dptr = xct_dptr;

        XVERIFY(0 == post_res_xmsg(xio_msgctxt));
#else
        for (x_int32_t xit_iter = 0; xit_iter < 10; ++xit_iter)
        {
            std::vector< x_char_t > xvec_buffer;
            xvec_buffer.resize(xut_size + TEXT_LEN_512);

            x_char_t * xct_rptr = &xvec_buffer[0];
            x_int32_t  xit_size = 0;

            memcpy(xct_rptr, xct_dptr, sizeof(x_uint64_t));
            xit_size = snprintf(xct_rptr + sizeof(x_uint64_t),
                                xvec_buffer.size() - sizeof(x_uint64_t),
                                "[%d] %s",
                                xit_iter,
                                xecho_text.c_str());

            x_io_msgctxt_t xio_msgctxt;
            xio_msgctxt.io_seqn = xut_seqn;
            xio_msgctxt.io_cmid = CMID_ECHO_TEXT;
            xio_msgctxt.io_size = (x_uint32_t )(xit_size + sizeof(x_uint64_t));
            xio_msgctxt.io_dptr = (x_uchar_t *)xct_rptr;

            XVERIFY(0 == post_res_xmsg(xio_msgctxt));
        }
#endif
    }

    return 0;
}
