/**
 * @file    xftp_server.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xftp_server.cpp
 * 创建日期：2019年01月21日
 * 文件标识：
 * 文件摘要：
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年01月21日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#include "xcomm.h"
#include "xftp_server.h"

#include "xftp_msgctxt.h"
#include "xftp_echo.h"

////////////////////////////////////////////////////////////////////////////////
// x_ftp_server_t

//====================================================================

// 
// x_ftp_server_t : common invoking
// 

/**********************************************************/
/**
 * @brief x_ftp_server_t 对象的单例调用接口。
 */
x_ftp_server_t & x_ftp_server_t::instance(void)
{
    static x_ftp_server_t _S_instance;
    return _S_instance;
}

/**********************************************************/
/**
 * @brief 套接字 IO 事件回调通知接口。
 * 
 * @param [in ] xfdt_sockfd : 套接字描述符。
 * @param [in ] xht_optargs : 回调至业务层的操作参数。
 * @param [in ] xut_ioevent : 回调的 IO 事件（参看 emIoEventCallback 枚举值）。
 * @param [in ] xht_context : 回调的上下文标识信息。
 * 
 * @return x_int32_t
 *         - 返回事件操作状态码（或错误码）。
 */
x_int32_t x_ftp_server_t::xio_event_callback(x_sockfd_t xfdt_sockfd,
                                             x_handle_t xht_optargs,
                                             x_uint32_t xut_ioevent,
                                             x_handle_t xht_context)
{
    x_ftp_server_t * xthis_ptr = (x_ftp_server_t *)xht_context;
    return xthis_ptr->xio_event(xfdt_sockfd, xht_optargs, xut_ioevent);
}

//====================================================================

// 
// x_ftp_server_t : constructor/destructor
// 

x_ftp_server_t::x_ftp_server_t(void)
{
    x_tcp_io_server_t::set_io_event_callback(
        &x_ftp_server_t::xio_event_callback, (x_handle_t)this);
}

x_ftp_server_t::~x_ftp_server_t(void)
{

}

//====================================================================

// 
// x_ftp_server_t : public interfaces
// 

/**********************************************************/
/**
 * @brief 启动 XTFP 网络服务。
 */
x_int32_t x_ftp_server_t::startup(void)
{
    x_int32_t xit_error = -1;

    do
    {
        //======================================

        if (is_startup())
        {
            xit_error = 0;
            break;
        }

        //======================================
        // 注册相关的业务层工作对象

#define REGISTER_IOTYPE(name)   XVERIFY(register_iotype(name::ECV_CONNECTION_TYPE, &name::create))

        REGISTER_IOTYPE(x_ftp_echo_t);

#undef  REGISTER_IOTYPE

        //======================================
        // 读取相关配置参数

        x_config_t & xconfig = x_config_t::instance();

        x_tcp_io_server_t::x_work_param_t xwpt_config;
        xconfig.read_str("server", "host", xwpt_config.xszt_host, TEXT_LEN_64, "");
        xwpt_config.xut_port             = xconfig.read_int("server", "port"            , 10086);
        xwpt_config.xut_epoll_maxsockfds = xconfig.read_int("server", "epoll_maxsockfds", 4096 );
        xwpt_config.xut_epoll_waitevents = xconfig.read_int("server", "epoll_waitevents", 256  );
        xwpt_config.xut_ioman_threads    = xconfig.read_int("server", "ioman_threads"   , 4    );
        xwpt_config.xut_tmout_kpalive    = xconfig.read_int("server", "tmout_kpalive"   , 8 * 60 * 1000);
        xwpt_config.xut_tmout_baleful    = xconfig.read_int("server", "tmout_baleful"   , 4 * 60 * 1000);
        xwpt_config.xut_tmout_mverify    = xconfig.read_int("server", "tmout_mverify"   , 4 * 60 * 1000);

        //======================================

        xit_error = x_tcp_io_server_t::startup(xwpt_config, X_NULL);
        if (0 != xit_error)
        {
            LOGE("x_tcp_io_server_t::startup(xwpt_config, X_NULL) return error : %d", xit_error);
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
 * @brief 关闭 XTFP 网络服务。
 */
x_void_t x_ftp_server_t::shutdown(void)
{
    x_tcp_io_server_t::shutdown();
    m_xmap_fcreate.clear();
}

/**********************************************************/
/**
 * @brief 注册业务层工作对象的创建接口。
 * 
 * @param [in ] xut_iotype   : 业务层工作对象 的 类型标识。
 * @param [in ] xfunc_create : 业务层工作对象 的 创建接口。
 * 
 * @return x_bool_t
 *         - 成功，返回 X_TRUE；
 *         - 失败，返回 X_FALSE。
 */
x_bool_t x_ftp_server_t::register_iotype(x_uint16_t xut_iotype, x_func_create_t xfunc_create)
{
    x_map_func_create_t::iterator itfind = m_xmap_fcreate.find(xut_iotype);
    if (itfind == m_xmap_fcreate.end())
    {
        m_xmap_fcreate.insert(std::make_pair(xut_iotype, xfunc_create));
        return X_TRUE;
    }

    return X_FALSE;
}

/**********************************************************/
/**
 * @brief 移除业务层工作对象的创建接口。
 */
x_void_t x_ftp_server_t::unregister_iotype(x_uint16_t xut_iotype)
{
    m_xmap_fcreate.erase(xut_iotype);
}

//====================================================================

// 
// x_ftp_server_t : internal invoking
// 

/**********************************************************/
/**
 * @brief 套接字 IO 事件回调通知接口。
 * 
 * @param [in ] xfdt_sockfd : 套接字描述符。
 * @param [in ] xht_optargs : 回调至业务层的操作参数。
 * @param [in ] xut_ioevent : 回调的 IO 事件（参看 emIoEventCallback 枚举值）。
 * 
 * @return x_int32_t
 *         - 返回事件操作状态码（或错误码）。
 */
x_int32_t x_ftp_server_t::xio_event(x_sockfd_t xfdt_sockfd,
                                    x_handle_t xht_optargs,
                                    x_uint32_t xut_ioevent)
{
    x_int32_t xit_error = 0;

    switch (xut_ioevent)
    {
    case EIO_ECBK_ACCEPT : xit_error = xio_event_accept(xfdt_sockfd, xht_optargs); break;
    case EIO_ECBK_CREATE : xit_error = xio_event_create(xfdt_sockfd, xht_optargs); break;

    default:
        break;
    }

    return xit_error;
}

/**********************************************************/
/**
 * @brief 处理 “套接字接收连接” 的事件回调操作（该接口仅由 xio_event() 调用）。
 */
x_int32_t x_ftp_server_t::xio_event_accept(x_sockfd_t xfdt_sockfd, x_handle_t xht_optargs)
{
    return 0;
}

/**********************************************************/
/**
 * @brief 处理 “建立业务层工作对象” 的事件回调操作（该接口仅由 xio_event() 调用）。
 */
x_int32_t x_ftp_server_t::xio_event_create(x_sockfd_t xfdt_sockfd, x_handle_t xht_optargs)
{
    x_int32_t xit_error = 0;

    x_tcp_io_create_args_t & xcreate_args = *(x_tcp_io_create_args_t *)xht_optargs;

    x_tcp_io_message_t & xio_message  = *(x_tcp_io_message_t *)xcreate_args.xht_message;
    x_io_msgctxt_t xio_msgctxt;

    x_map_func_create_t::iterator itfind;

    do
    {
        //======================================
        // 确定首个 IO 消息数据包的有效性，
        // 然后提取出来，作为创建业务层工作对象的参数

        xit_error = io_get_context(xio_message.data(), xio_message.rlen(), &xio_msgctxt);
        if (IOCTX_ERR_OK != xit_error)
        {
            if (IOCTX_ERR_PART == xit_error)
            {
                xit_error = 0;
                xcreate_args.xht_channel = X_NULL;
            }
            else
            {
                LOGE("io_get_context(..., xio_message.rlen()[%d], &xio_msgctxt) return error : %d",
                     xio_message.rlen(), xit_error);
                xit_error = -1;
            }

            break;
        }

        //======================================
        // 创建业务层工作对象

        itfind = m_xmap_fcreate.find(xio_msgctxt.io_cmid);
        if (itfind == m_xmap_fcreate.end())
        {
            LOGE("This type[%d] of connection is not supported!", xio_msgctxt.io_cmid);
            xit_error = -1;
            break;
        }

        xit_error = itfind->second(xcreate_args.xht_manager,
                                   xcreate_args.xfdt_sockfd,
                                   (x_handle_t)&xio_msgctxt,
                                   xcreate_args.xht_channel);
        if (0 != xit_error)
        {
            LOGE("itfind->second(..., xfdt_sockfd[%s:%d], xio_msgctxt[io_cmid: %d, io_size: %d], ...) return error : %d",
                 sockfd_remote_ip(xcreate_args.xfdt_sockfd, LOG_BUF(64), 64),
                 sockfd_remote_port(xcreate_args.xfdt_sockfd),
                 xio_msgctxt.io_cmid,
                 xio_msgctxt.io_size,
                 xit_error);
            break;
        }

        //======================================
        xit_error = 0;
    } while (0);

    return xit_error;
}
