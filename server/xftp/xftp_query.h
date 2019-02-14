/**
 * @file    xftp_query.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xftp_query.h
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

#ifndef __XFTP_QUERY_H__
#define __XFTP_QUERY_H__

#include "xftp_connection.h"

////////////////////////////////////////////////////////////////////////////////
// x_ftp_query_t

/**
 * @class x_ftp_query_t
 * @brief 提供 xftp 的信息查询服务的业务层工作对象。
 */
class x_ftp_query_t : public x_ftp_connection_t
{
    // common data types
public:
    /**
     * @enum  emConstValue
     * @brief 相关的枚举常量值。
     */
    typedef enum emConstValue
    {
        ECV_CONNECTION_TYPE  = 0x0020,  ///< 业务层工作对象的连接类型
    } emConstValue;

    /**
     * @enum  emIoContextCmid
     * @brief IO 操作的命令 ID 表。
     */
    typedef enum emIoContextCmid
    {
        CMID_QUERY_LOGIN      = 0x0020,  ///< 登录
        CMID_QUERY_HBEAT      = 0x2000,  ///< 心跳

        CMID_QUERY_FLIST      = 0x3010,  ///< 获取文件列表
    } emIoContextCmid;

    // common invoking
public:
    /**********************************************************/
    /**
     * @brief x_ftp_query_t 对象创建接口。
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
                            x_handle_t & xht_channel);

    // constructor/destructor
private:
    explicit x_ftp_query_t(x_handle_t xht_manager, x_sockfd_t xfdt_sockfd);
    virtual ~x_ftp_query_t(void);

    // overrides
protected:
    /**********************************************************/
    /**
     * @brief 处理 “接收 IO 请求消息” 的事件。
     */
    virtual x_int32_t io_event_recved_xmsg(x_tcp_io_message_t & xio_message) override;

    /**********************************************************/
    /**
     * @brief 处理 “完成 IO 应答消息” 的事件。
     */
    virtual x_int32_t io_event_sended_xmsg(x_tcp_io_message_t & xio_message) override;

    // internal invoking
protected:
    /**********************************************************/
    /**
     * @brief 处理 IO 请求命令：登录。
     */
    x_int32_t iocmd_login(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size);

    /**********************************************************/
    /**
     * @brief 处理 IO 请求命令：心跳。
     */
    x_int32_t iocmd_hbeat(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size);

    /**********************************************************/
    /**
     * @brief 处理 IO 请求命令：获取文件列表。
     */
    x_int32_t iocmd_flist(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size);

    // data members
private:
    std::string   m_xstr_name;  ///< 建立连接的标识名称
};

////////////////////////////////////////////////////////////////////////////////

#endif // __XFTP_QUERY_H__

