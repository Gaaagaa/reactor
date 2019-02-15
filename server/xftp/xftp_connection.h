/**
 * @file    xftp_connection.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xftp_connection.h
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

#ifndef __XFTP_CONNECTION_H__
#define __XFTP_CONNECTION_H__

#include "xtcp_io_channel.h"
#include "xftp_msgctxt.h"
#include <list>

////////////////////////////////////////////////////////////////////////////////
// x_ftp_channel_t

/**
 * @class x_ftp_channel_t
 * @brief 业务层连接对象的基类（定义 IO 消息分包规则）。
 */
class x_ftp_channel_t : public x_tcp_io_channel_t
{
    // common invoking
public:
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
    static x_uint32_t xmsg_split(x_tcp_io_message_t & xio_message,
                                 std::list< x_tcp_io_message_t > & xlst_iomsg);

    // constructor/destructor
protected:
    explicit x_ftp_channel_t(x_handle_t xht_manager, x_sockfd_t xfdt_sockfd);
    virtual ~x_ftp_channel_t(void);

    // overrides
protected:
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
    virtual x_int32_t post_req_xmsg(x_tcp_io_message_t & xio_message);

    // extensible interfaces
public:
    /**********************************************************/
    /**
     * @brief 连接类型。
     */
    virtual x_uint16_t ctype(void) const = 0;

    // public interfaces
public:


    // internal invoking
protected:
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
    x_int32_t post_res_xmsg(const x_io_msgctxt_t & xio_msgctxt);

};

////////////////////////////////////////////////////////////////////////////////
// x_ftp_connection_t

/**
 * @class x_ftp_connection_t< _Ty, _Xt >
 * @brief 业务层连接对象的模板接口类。
 * 
 * @param [in ] _Ty : 派生的具体业务层连接对象工作类。
 * @param [in ] _Xt : 业务层连接对象的类型标识。
 */
template< class _Ty, x_uint16_t _Xt >
class x_ftp_connection_t : public x_ftp_channel_t
{
    // common data types
public:
    /**
     * @enum  emConstValue
     * @brief 相关的枚举常量值。
     */
    typedef enum emConstValue
    {
        ECV_CONNECTION_TYPE  = _Xt,  ///< 业务层工作对象的连接类型
    } emConstValue;

    using x_type_t  = _Ty;
    using x_super_t = x_ftp_connection_t< _Ty, _Xt >;

    // common invoking
public:
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
    static x_int32_t create(x_handle_t xht_manager,
                            x_sockfd_t xfdt_sockfd,
                            x_handle_t xht_msgctxt,
                            x_handle_t & xht_channel)
    {
        x_type_t * xthis_ptr = new x_type_t(xht_manager, xfdt_sockfd);
        xht_channel = (x_handle_t)(xthis_ptr);
        XASSERT(X_NULL != xht_channel);

        return 0;
    }

    // constructor/destructor
protected:
    explicit x_ftp_connection_t(x_handle_t xht_manager, x_sockfd_t xfdt_sockfd)
        : x_ftp_channel_t(xht_manager, xfdt_sockfd)
    {

    }

    virtual ~x_ftp_connection_t(void)
    {

    }

    // overrides
public:
    /**********************************************************/
    /**
     * @brief 连接类型。
     */
    virtual x_uint16_t ctype(void) const override
    {
        return _Xt;
    }

};

////////////////////////////////////////////////////////////////////////////////

#endif // __XFTP_CONNECTION_H__
