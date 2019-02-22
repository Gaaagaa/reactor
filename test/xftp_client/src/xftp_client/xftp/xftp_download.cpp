/**
 * @file    xftp_download.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xftp_download.cpp
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

#include "stdafx.h"
#include "xftp_download.h"

////////////////////////////////////////////////////////////////////////////////
// x_ftp_download_t

//====================================================================

// 
// x_ftp_download_t : constructor/destructor
// 

x_ftp_download_t::x_ftp_download_t(void)
    : m_xbt_login(X_TRUE)
    , m_xstr_fpath("")
    , m_xit_fsize(0)
    , m_xbt_pause(X_FALSE)
    , m_xht_fstream(INVALID_HANDLE_VALUE)
{

}

x_ftp_download_t::~x_ftp_download_t(void)
{
    if (is_startup())
    {
        logout();
    }
}

//====================================================================

// 
// x_ftp_download_t : overrides
// 

/**********************************************************/
/**
 * @brief 投递心跳数据包信息。
 */
x_void_t x_ftp_download_t::send_heartbeat(void)
{
    x_io_msgctxt_t xio_msgctxt;
    xio_msgctxt.io_seqn = 0;
    xio_msgctxt.io_cmid = CMID_DLOAD_HBEAT;
    xio_msgctxt.io_size = 0;
    xio_msgctxt.io_dptr = X_NULL;

    post_req_xmsg(xio_msgctxt, X_TRUE);
}

/**********************************************************/
/**
 * @brief 收到 IO 消息的通知接口。
 */
x_void_t x_ftp_download_t::io_recved_msgctxt(const x_io_msgctxt_t & xio_msgctxt)
{
    switch (xio_msgctxt.io_cmid)
    {
    case CMID_DLOAD_LOGIN: iocmd_login(xio_msgctxt.io_seqn, xio_msgctxt.io_dptr, xio_msgctxt.io_size); break;
    case CMID_DLOAD_HBEAT: iocmd_hbeat(xio_msgctxt.io_seqn, xio_msgctxt.io_dptr, xio_msgctxt.io_size); break;
    case CMID_DLOAD_CHUNK: iocmd_chunk(xio_msgctxt.io_seqn, xio_msgctxt.io_dptr, xio_msgctxt.io_size); break;
    case CMID_DLOAD_PAUSE: iocmd_pause(xio_msgctxt.io_seqn, xio_msgctxt.io_dptr, xio_msgctxt.io_size); break;

    default:
        break;
    }
}

/**********************************************************/
/**
 * @brief 完成 IO 消息发送的通知接口。
 */
x_void_t x_ftp_download_t::io_sended_msgctxt(const x_io_msgctxt_t & xio_msgctxt)
{

}

/**********************************************************/
/**
 * @brief IO 操作产生错误的通知接口。
 */
x_void_t x_ftp_download_t::io_error_notify(x_int32_t xit_error, x_bool_t xbt_iosend)
{
    if (0 != xit_error)
    {
        x_msg_handler_t::instance().post_msg(EM_EST_CLIENT,
                                             XMKEY_DLOAD_IOERR,
                                             X_NULL,
                                             sizeof(x_int32_t),
                                             (x_uchar_t *)&xit_error);
    }
}

//====================================================================

// 
// x_ftp_download_t : public interfaces
// 

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
x_int32_t x_ftp_download_t::login(x_cstring_t xszt_host, x_uint16_t xwt_port, x_cstring_t xszt_fpath, x_int64_t xit_fsize)
{
    x_int32_t xit_error = -1;

    do
    {
        //======================================

        logout();
        m_xbt_login = X_FALSE;

        if ((xit_fsize <= 0) || (X_NULL == xszt_fpath) || ('\0' == *xszt_fpath))
        {
            break;
        }

        //======================================
        // 启动连接

        enable_auto_heartbeat(X_FALSE);

        xit_error = x_ftp_cliworker_t::startup(xszt_host, xwt_port);
        if (0 != xit_error)
        {
            break;
        }

        //======================================

        m_xbt_login  = X_FALSE;
        m_xstr_fpath = xszt_fpath;
        m_xit_fsize  = xit_fsize;
        m_xbt_pause  = X_FALSE;

        //======================================
        // 提交登录验证请求

        x_cstring_t xszt_fname = file_base_name(xszt_fpath);
        x_uint32_t  xut_length = (x_uint32_t)strlen(xszt_fname);

        x_io_msgctxt_t xio_msgctxt;
        xio_msgctxt.io_seqn = 0;
        xio_msgctxt.io_cmid = CMID_DLOAD_LOGIN;
        xio_msgctxt.io_size = xut_length;
        xio_msgctxt.io_dptr = (x_uchar_t *)xszt_fname;

        post_req_xmsg(xio_msgctxt, X_TRUE);

        //======================================
        xit_error = 0;
    } while (0);

    if (0 != xit_error)
    {
        logout();
    }

    return xit_error;
}

/**********************************************************/
/**
 * @brief 登出操作。
 */
x_void_t x_ftp_download_t::logout(void)
{
    m_xbt_login  = X_FALSE;
    m_xstr_fpath = "";
    m_xit_fsize  = 0;
    m_xbt_pause  = X_FALSE;

    shutdown();

    if (INVALID_HANDLE_VALUE != m_xht_fstream)
    {
        CloseHandle(m_xht_fstream);
        m_xht_fstream = INVALID_HANDLE_VALUE;
    }
}

//====================================================================

// 
// x_ftp_download_t : internal invoking
// 

/**********************************************************/
/**
 * @brief 投递文件块的下载请求。
 *
 * @param [in ] xit_offset  : 文件块偏移位置。
 * @param [in ] xut_chksize : 文件块大小。
 * @param [in ] xbt_pause   : 暂停标识。
 *
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t x_ftp_download_t::post_req_chunk(x_int64_t xit_offset, x_uint32_t xut_chksize, x_bool_t xbt_pause)
{
    // [文件读取偏移位置 8byte + 请求的文件块长度 4byte + 暂停标识 4byte]
    x_uint32_t xut_io_size = sizeof(x_int64_t) + sizeof(x_uint32_t) + sizeof(x_uint32_t);
    x_uchar_t  xct_io_dptr[sizeof(x_int64_t) + sizeof(x_uint32_t) + sizeof(x_uint32_t)];

    x_uchar_t * xct_vptr = xct_io_dptr;

    // 文件读取偏移位置
    *(x_int64_t *)(xct_vptr) = (x_int64_t)vx_htonll(xit_offset);
    xct_vptr += sizeof(x_int64_t);

    // 请求的文件块长度
    *(x_uint32_t *)(xct_vptr) = (x_uint32_t)vx_htonl(xut_chksize);
    xct_vptr += sizeof(x_uint32_t);

    // 暂停标识
    *(x_uint32_t *)(xct_vptr) = (x_uint32_t)vx_htonl(xbt_pause);

    // IO 上下文描述信息
    x_io_msgctxt_t xio_msgctxt;
    xio_msgctxt.io_seqn = 0;
    xio_msgctxt.io_cmid = CMID_DLOAD_CHUNK;
    xio_msgctxt.io_size = xut_io_size;
    xio_msgctxt.io_dptr = xct_io_dptr;

    return post_req_xmsg(xio_msgctxt, X_TRUE);
}

//====================================================================

// 
// x_ftp_download_t : io event handlers
// 

/**********************************************************/
/**
 * @brief 处理 IO 应答命令：建立网络连接。
 */
x_int32_t x_ftp_download_t::iocmd_login(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size)
{
    x_int32_t xit_error = -1;

    do 
    {
        //======================================
        // 应答的错误码验证

        if (sizeof(x_int32_t) == xut_size)
        {
            xit_error = (x_int32_t)vx_ntohl(*(x_int32_t *)xct_dptr);
        }

        if (0 != xit_error)
        {
            break;
        }

        //======================================
        // 打开本地文件，用于接收文件数据流

        if (!MkFilePathDirA(m_xstr_fpath.c_str()))
        {
            xit_error = -1;
            break;
        }

        XASSERT(INVALID_HANDLE_VALUE == m_xht_fstream);
        m_xht_fstream = CreateFileA(m_xstr_fpath.c_str(),
                                    GENERIC_WRITE,
                                    FILE_SHARE_WRITE | FILE_SHARE_READ,
                                    NULL,
                                    CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);
        if (INVALID_HANDLE_VALUE == m_xht_fstream)
        {
            xit_error = -1;
            break;
        }

        //======================================
        // 投递下载请求

        post_req_chunk(0LL, ECV_NOR_CHUNK_SIZE, X_FALSE);

        //======================================

        m_xbt_login = X_TRUE;
        enable_auto_heartbeat(X_TRUE);
        xit_error = 0;

        //======================================
    } while (0);

    if (0 != xit_error)
    {
        io_error_notify(xit_error, X_FALSE);
    }

    return 0;
}

/**********************************************************/
/**
 * @brief 处理 IO 应答命令：心跳。
 */
x_int32_t x_ftp_download_t::iocmd_hbeat(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size)
{
    return 0;
}

/**********************************************************/
/**
 * @brief 处理 IO 应答命令：下载文件块。
 */
x_int32_t x_ftp_download_t::iocmd_chunk(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size)
{
    x_int32_t xit_error = -1;
    x_bool_t  xbt_write = X_FALSE;

    do 
    {
        //======================================
        // 错误码判断

        if ((INVALID_HANDLE_VALUE == m_xht_fstream) || (X_NULL == xct_dptr) || (xut_size < sizeof(x_int32_t)))
        {
            xit_error = -1;
            break;
        }

        if (sizeof(x_int32_t) == xut_size)
        {
            xit_error = (x_int32_t)vx_ntohl(*(x_int32_t *)xct_dptr);
            break;
        }

        //======================================
        // 文件块信息参数

        if ((xut_size < (sizeof(x_int64_t) + sizeof(x_uint32_t))))
        {
            xit_error = -1;
            break;
        }

        x_uchar_t * xct_vptr = xct_dptr;

        x_int64_t  xit_offset = (x_int64_t )vx_ntohll(*(x_int64_t  *)xct_vptr); xct_vptr += sizeof(x_int64_t );
        x_uint32_t xut_lchunk = (x_uint32_t)vx_ntohl (*(x_uint32_t *)xct_vptr); xct_vptr += sizeof(x_uint32_t);

        //======================================
        // 写入文件块数据

        LARGE_INTEGER xoffset;
        xoffset.QuadPart = xit_offset;
        if (!SetFilePointerEx(m_xht_fstream, xoffset, X_NULL, FILE_BEGIN))
        {
            xit_error = -1;
            break;
        }

        xit_error = 0;

        x_ulong_t xut_count = 0;
        while (xut_count < xut_lchunk)
        {
            x_ulong_t xut_wbytes = 0;
            if (!WriteFile(m_xht_fstream, xct_vptr + xut_count, xut_lchunk - xut_count, &xut_wbytes, NULL))
            {
                xit_error = -1;
                break;
            }

            xut_count += xut_wbytes;
            if (0 == xut_wbytes)
                break;
        }

        if (xut_count != xut_lchunk)
        {
            xit_error = -1;
            break;
        }

        if (0 != xit_error)
        {
            break;
        }

        xbt_write = X_TRUE;

        //======================================
        xit_error = 0;
    } while (0);

    if ((0 != xit_error) || !xbt_write)
    {
        io_error_notify(xit_error, X_FALSE);
    }

    return 0;
}

/**********************************************************/
/**
 * @brief 处理 IO 应答命令：暂停。
 */
x_int32_t x_ftp_download_t::iocmd_pause(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size)
{
    if (sizeof(x_uint32_t) == xut_size)
    {
        m_xbt_pause = (0 != vx_ntohl(*(x_ulong_t *)xct_dptr)) ? X_TRUE : X_FALSE;

        x_msg_handler_t::instance().post_msg(
            EM_EST_CLIENT, XMKEY_DLOAD_PAUSE, X_NULL, sizeof(x_bool_t), (x_uchar_t *)&m_xbt_pause);
    }

    return 0;
}
