/**
 * @file    xftp_download.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xftp_download.h
 * 创建日期：2019年02月22日
 * 文件标识：
 * 文件摘要：XFTP 的客户端文件下载工作类。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年02月22日
 * 版本摘要：
 * 
 * 取代版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#ifndef __XFTP_DOWNLOAD_H__
#define __XFTP_DOWNLOAD_H__

#include "xftp_cliworker.h"
#include <fstream>

////////////////////////////////////////////////////////////////////////////////
// x_ftp_download_t

/**
 * @class x_ftp_download_t
 * @brief XFTP 的客户端文件下载工作类。
 */
class x_ftp_download_t : public x_ftp_cliworker_t
{
    // common data types
public:
    /**
     * @enum  emConstValue
     * @brief 相关的枚举常量值。
     */
    typedef enum emConstValue
    {
        ECV_NOR_CHUNK_SIZE = 32 * 1024,  ///< IO 操作的文件块常规长度
        ECV_MAX_CHUNK_SIZE = 60 * 1024,  ///< IO 操作的文件块最大长度
    } emConstValue;

    /**
     * @enum  emIoContextCmid
     * @brief IO 操作的命令 ID 表。
     */
    typedef enum emIoContextCmid
    {
        CMID_DLOAD_LOGIN = 0x0030,  ///< 登录
        CMID_DLOAD_HBEAT = 0x2000,  ///< 心跳
        CMID_DLOAD_CHUNK = 0x2010,  ///< 下载文件块
        CMID_DLOAD_PAUSE = 0x2020,  ///< 暂停下载
    } emIoContextCmid;

    // constructor/destructor
public:
    x_ftp_download_t(void);
    virtual ~x_ftp_download_t(void);

    // overrides
public:
    /**********************************************************/
    /**
     * @brief 连接类型。
     */
    virtual x_uint16_t ctype(void) const
    {
        return CMID_DLOAD_LOGIN;
    }

protected:
    /**********************************************************/
    /**
     * @brief 投递心跳数据包信息。
     */
    virtual x_void_t send_heartbeat(void) override;

    /**********************************************************/
    /**
     * @brief 收到 IO 消息的通知接口。
     */
    virtual x_void_t io_recved_msgctxt(const x_io_msgctxt_t & xio_msgctxt) override;

    /**********************************************************/
    /**
     * @brief 完成 IO 消息发送的通知接口。
     */
    virtual x_void_t io_sended_msgctxt(const x_io_msgctxt_t & xio_msgctxt) override;

    /**********************************************************/
    /**
     * @brief IO 操作产生错误的通知接口。
     */
    virtual x_void_t io_error_notify(x_int32_t xit_error, x_bool_t xbt_iosend) override;

    // public interfaces
public:
    /**********************************************************/
    /**
     * @brief 登录操作。
     *
     * @param [in ] xszt_host  : 目标主机（服务器）的 IP 地址（四段式 IP 地址字符串）。
     * @param [in ] xwt_port   : 目标主机（服务器）的 端口号。
     * @param [in ] xszt_fpath : 要下载的文件路径名。
     * @param [in ] xit_fsize  : 要下载的文件大小。
     *
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t login(x_cstring_t xszt_host, x_uint16_t xwt_port, x_cstring_t xszt_fpath, x_int64_t xit_fsize);

    /**********************************************************/
    /**
     * @brief 登出操作。
     */
    x_void_t logout(void);

    /**********************************************************/
    /**
     * @brief 是否已经登录。
     */
    inline x_bool_t is_login(void) const { return (is_startup() && m_xbt_login); }

    // internal invoking
protected:
    /**********************************************************/
    /**
     * @brief 投递文件块的下载请求。
     *
     * @param [in ] xit_offset  : 文件块偏移位置。
     * @param [in ] xit_chksize : 文件块大小。
     *
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t post_req_chunk(x_int64_t xit_offset, x_int64_t xit_chksize);

    // io event handlers
protected:
    /**********************************************************/
    /**
     * @brief 处理 IO 应答命令：建立网络连接。
     */
    x_int32_t iocmd_login(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size);

    /**********************************************************/
    /**
     * @brief 处理 IO 应答命令：心跳。
     */
    x_int32_t iocmd_hbeat(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size);

    /**********************************************************/
    /**
     * @brief 处理 IO 应答命令：下载文件块。
     */
    x_int32_t iocmd_chunk(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size);

    /**********************************************************/
    /**
     * @brief 处理 IO 应答命令：暂停。
     */
    x_int32_t iocmd_pause(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size);

    // data members
protected:
    x_bool_t      m_xbt_login;    ///< 标识登录成功（网络连接建立成功）

    std::string   m_xstr_fpath;   ///< 文件路径名
    x_int64_t     m_xit_fsize;    ///< 文件大小
    x_bool_t      m_xbt_pause;    ///< 暂停标识

    std::ofstream m_xht_fstream;  ///< 文件写操作对象
};

////////////////////////////////////////////////////////////////////////////////

#define XMKEY_DLOAD_IOERR   ((x_uint32_t)(x_ftp_download_t::CMID_DLOAD_LOGIN << 16) | (x_uint32_t)x_ftp_download_t::CMID_DLOAD_LOGIN)
#define XMKEY_DLOAD_CHUNK   ((x_uint32_t)(x_ftp_download_t::CMID_DLOAD_LOGIN << 16) | (x_uint32_t)x_ftp_download_t::CMID_DLOAD_CHUNK)
#define XMKEY_DLOAD_PAUSE   ((x_uint32_t)(x_ftp_download_t::CMID_DLOAD_LOGIN << 16) | (x_uint32_t)x_ftp_download_t::CMID_DLOAD_PAUSE)

////////////////////////////////////////////////////////////////////////////////

#endif // __XFTP_DOWNLOAD_H__

