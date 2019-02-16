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
class x_ftp_query_t : public x_ftp_connection_t< x_ftp_query_t, 0x0020 >
{
    friend x_super_t;

    // common data types
public:
    /**
     * @enum  emConstValue
     * @brief 相关的枚举常量值。
     */
    typedef enum emConstValue
    {
        ECV_CONNECTION_TYPE  = 0x0020,  ///< 业务层工作对象的连接类型

        ECV_GET_MAX_FILES = 100,  ///< 获取文件列表的最大文件数量
    } emConstValue;

    /**
     * @enum  emIoContextCmid
     * @brief IO 操作的命令 ID 表。
     */
    typedef enum emIoContextCmid
    {
        CMID_QUERY_LOGIN   = x_super_t::ECV_CONNECTION_TYPE,  ///< 登录

        CMID_QUERY_HBEAT   = 0x2000,  ///< 心跳
        CMID_QUERY_FLIST   = 0x3010,  ///< 获取文件列表
    } emIoContextCmid;

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
    virtual x_int32_t io_event_requested(x_tcp_io_message_t & xio_message) override;

    /**********************************************************/
    /**
     * @brief 处理 “完成 IO 应答消息” 的事件。
     */
    virtual x_int32_t io_event_responsed(x_tcp_io_message_t & xio_message) override;

    /**********************************************************/
    /**
     * @brief 处理 “IO 通道对象被销毁” 的事件。
     */
    virtual x_int32_t io_event_destroyed(void);

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

