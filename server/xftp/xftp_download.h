/**
 * @file    xftp_download.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xftp_download.h
 * 创建日期：2019年02月20日
 * 文件标识：
 * 文件摘要：提供文件下载操作的业务层工作对象。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年02月20日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#ifndef __XFTP_DOWNLOAD_H__
#define __XFTP_DOWNLOAD_H__

#include "xftp_connection.h"

////////////////////////////////////////////////////////////////////////////////
// x_ftp_download_t

/**
 * @class x_ftp_download_t
 * @brief 提供文件下载操作的业务层工作对象。
 */
class x_ftp_download_t : public x_ftp_connection_t< x_ftp_download_t, 0x0030 >
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
        ECV_MAX_CHUNK_SIZE = 60 * 1024,  ///< IO 操作的文件块最大长度
    } emConstValue;

    /**
     * @enum  emIoContextCmid
     * @brief IO 操作的命令 ID 表。
     */
    typedef enum emIoContextCmid
    {
        CMID_DLOAD_LOGIN  = x_super_t::ECV_CONNECTION_TYPE,  ///< 登录
        CMID_DLOAD_HBEAT  = 0x2000,  ///< 心跳
        CMID_DLOAD_CHUNK  = 0x2010,  ///< 下载文件块
        CMID_DLOAD_PAUSE  = 0x2020,  ///< 暂停下载
    } emIoContextCmid;

    // constructor/destructor
private:
    explicit x_ftp_download_t(x_handle_t xht_manager, x_sockfd_t xfdt_sockfd);
    virtual ~x_ftp_download_t(void);

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
     * @brief 投递 “下载文件块” 的应答消息（将文件块数据传输给客户端）。
     * 
     * @param [in ] xut_seqn   : 应答消息的流水号。
     * @param [in ] xit_offset : 读取文件数据的起始偏移位置。
     * @param [in ] xut_rdsize : 读取文件块的最大数据量。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t post_res_chunk(x_uint16_t xut_seqn, x_int64_t xit_offset, x_uint32_t xut_rdsize);

    // requested message handlers
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
     * @brief 处理 IO 请求命令：下载文件块。
     */
    x_int32_t iocmd_chunk(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size);

    /**********************************************************/
    /**
     * @brief 处理 IO 请求命令：暂停下载。
     */
    x_int32_t iocmd_pause(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size);

    // responsed message handlers
protected:
    /**********************************************************/
    /**
     * @brief 处理 IO 完成应答命令：登录。
     */
    x_int32_t iores_login(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size);

    /**********************************************************/
    /**
     * @brief 处理 IO 完成应答命令：下载文件块。
     */
    x_int32_t iores_chunk(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size);

    // data members
private:
    std::string   m_xstr_fname;     ///< 请求下载的文件名
    std::string   m_xstr_fpath;     ///< 请求下载的文件全路径名
    x_int64_t     m_xit_fsize;      ///< 文件总大小

    x_handle_t    m_xht_fstream;    ///< 文件流的工作句柄
    x_bool_t      m_xbt_pause;      ///< 暂停标识
};

////////////////////////////////////////////////////////////////////////////////

#endif // __XFTP_DOWNLOAD_H__

