/**
 * @file    xftp_echo.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xftp_echo.h
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

#ifndef __XFTP_ECHO_H__
#define __XFTP_ECHO_H__

#include "xftp_connection.h"

////////////////////////////////////////////////////////////////////////////////
// x_ftp_echo_t

/**
 * @class x_ftp_echo_t
 * @brief 提供 ECHO 测试服务的业务层工作对象。
 */
class x_ftp_echo_t : public x_ftp_connection_t< x_ftp_echo_t, 0x0010 >
{
    friend x_super_t;

    // common data types
public:
    /**
     * @enum  emIoContextCmid
     * @brief IO 操作的命令 ID 表。
     */
    typedef enum emIoContextCmid
    {
        CMID_ECHO_LOGIN  = x_super_t::ECV_CONNECTION_TYPE,  ///< 登录

        CMID_ECHO_HBEAT  = 0x2000,  ///< 心跳
        CMID_ECHO_TEXT   = 0x2010,  ///< 文本内容反射
    } emIoContextCmid;

    // constructor/destructor
private:
    explicit x_ftp_echo_t(x_handle_t xht_manager, x_sockfd_t xfdt_sockfd);
    virtual ~x_ftp_echo_t(void);

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
     * @brief 处理 IO 请求命令：文本内容反射。
     */
    x_int32_t iocmd_text(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size);

    // data members
private:
    std::string   m_xstr_name;  ///< 建立连接的标识名称
};

////////////////////////////////////////////////////////////////////////////////

#endif // __XFTP_ECHO_H__

