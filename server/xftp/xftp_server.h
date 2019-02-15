/**
 * @file    xftp_server.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xftp_server.h
 * 创建日期：2019年01月21日
 * 文件标识：
 * 文件摘要：xftp 服务器工作对象。
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

#ifndef __XFTP_SERVER_H__
#define __XFTP_SERVER_H__

#include "xtcp_io_server.h"

////////////////////////////////////////////////////////////////////////////////
// x_ftp_server_t

/**
 * @class x_ftp_server_t
 * @brief xftp 服务器工作对象。
 */
class x_ftp_server_t : protected x_tcp_io_server_t
{
    // common data types
public:
    /**
     * @brief 各类 x_tcp_io_channel_t 对象创建接口的函数类型。
     * 
     * @param [in ] xht_manager : 业务层工作对象所隶属的 IO 管理模块句柄。
     * @param [in ] xfdt_sockfd : 业务层工作对象的 套接字描述符。
     * @param [in ] xht_msgctxt : 指向创建业务层工作对象的 IO 请求消息（首个 IO 消息）。
     * @param [out] xht_vhandle : 操作成功所返回的 x_tcp_io_channel_t 对象句柄。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    typedef x_int32_t (* x_func_create_t)(x_handle_t xht_manager,
                                          x_sockfd_t xfdt_sockfd,
                                          x_handle_t xht_msgctxt,
                                          x_handle_t & xht_vhandle);

    /** [ 类型，创建接口函数 ] */
    typedef std::map< x_uint16_t, x_func_create_t > x_map_func_create_t;

    // common invoking
public:
    /**********************************************************/
    /**
     * @brief x_master_t 在基本组件初始化完成后，额外进行数据初始化操作所使用的回调接口。
     * 
     * @param [in ] xht_context : 回调的上下文句柄。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    static x_int32_t init_extra_callback(x_handle_t xht_context);

    /**********************************************************/
    /**
     * @brief x_ftp_server_t 对象的单例调用接口。
     */
    static x_ftp_server_t & instance(void);

private:
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
    static x_int32_t xio_event_callback(x_sockfd_t xfdt_sockfd,
                                        x_handle_t xht_optargs,
                                        x_uint32_t xut_ioevent,
                                        x_handle_t xht_context);

    // constructor/destructor
private:
    explicit x_ftp_server_t(void);
    virtual ~x_ftp_server_t(void);

    x_ftp_server_t(const x_ftp_server_t & xobject);
    x_ftp_server_t & operator=(const x_ftp_server_t & xobject);

    // public interfaces
public:
    /**********************************************************/
    /**
     * @brief 启动 XTFP 网络服务。
     */
    x_int32_t startup(void);

    /**********************************************************/
    /**
     * @brief 关闭 XTFP 网络服务。
     */
    x_void_t shutdown(void);

    /**********************************************************/
    /**
     * @brief 判断是否已经启动。
     */
    inline x_bool_t is_startup(void) const { return x_tcp_io_server_t::is_startup(); }

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
    x_bool_t register_iotype(x_uint16_t xut_iotype, x_func_create_t xfunc_create);

    /**********************************************************/
    /**
     * @brief 移除业务层工作对象的创建接口。
     */
    x_void_t unregister_iotype(x_uint16_t xut_iotype);

    // internal invoking
private:
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
    x_int32_t xio_event(x_sockfd_t xfdt_sockfd,
                        x_handle_t xht_optargs,
                        x_uint32_t xut_ioevent);

    /**********************************************************/
    /**
     * @brief 处理 “套接字接收连接” 的事件回调操作（该接口仅由 xio_event() 调用）。
     */
    x_int32_t xio_event_accept(x_sockfd_t xfdt_sockfd, x_handle_t xht_optargs);

    /**********************************************************/
    /**
     * @brief 处理 “建立业务层工作对象” 的事件回调操作（该接口仅由 xio_event() 调用）。
     */
    x_int32_t xio_event_create(x_sockfd_t xfdt_sockfd, x_handle_t xht_optargs);

    // data members
private:
    x_map_func_create_t    m_xmap_fcreate;   ///< 各类 业务层工作对象 的 创建接口函数 的 映射表
};

////////////////////////////////////////////////////////////////////////////////

#endif // __XFTP_SERVER_H__
