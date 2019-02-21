/**
 * @file    xftp_download.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xftp_download.cpp
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

#include "xcomm.h"
#include "xftp_download.h"
#include "xftp_server.h"

////////////////////////////////////////////////////////////////////////////////
// x_ftp_download_t

//====================================================================

// 
// x_ftp_download_t : constructor/destructor
// 

x_ftp_download_t::x_ftp_download_t(x_handle_t xht_manager, x_sockfd_t xfdt_sockfd)
    : x_super_t(xht_manager, xfdt_sockfd)
    , m_xstr_fname("")
    , m_xstr_fpath("")
    , m_xit_fsize(0)
    , m_xht_fstream(X_NULL)
{

}

x_ftp_download_t::~x_ftp_download_t(void)
{

}

//====================================================================

// 
// x_ftp_download_t : overrides
// 

/**********************************************************/
/**
 * @brief 处理 “接收 IO 请求消息” 的事件。
 */
x_int32_t x_ftp_download_t::io_event_requested(x_tcp_io_message_t & xio_message)
{
    x_int32_t xit_error = 0;

    x_io_msgctxt_t xio_msgctxt;
    if (IOCTX_ERR_OK == io_context_rinfo(xio_message.data(), xio_message.rlen(), &xio_msgctxt))
    {
        switch (xio_msgctxt.io_cmid)
        {
        case CMID_DLOAD_LOGIN : xit_error = iocmd_login(xio_msgctxt.io_seqn, xio_msgctxt.io_dptr, xio_msgctxt.io_size); break;
        case CMID_DLOAD_HBEAT : xit_error = iocmd_hbeat(xio_msgctxt.io_seqn, xio_msgctxt.io_dptr, xio_msgctxt.io_size); break;
        case CMID_DLOAD_CHUNK : xit_error = iocmd_chunk(xio_msgctxt.io_seqn, xio_msgctxt.io_dptr, xio_msgctxt.io_size); break;
        case CMID_DLOAD_PAUSE : xit_error = iocmd_pause(xio_msgctxt.io_seqn, xio_msgctxt.io_dptr, xio_msgctxt.io_size); break;

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
x_int32_t x_ftp_download_t::io_event_responsed(x_tcp_io_message_t & xio_message)
{
    x_int32_t xit_error = 0;

    x_io_msgctxt_t xio_msgctxt;
    if (IOCTX_ERR_OK == io_context_rinfo(xio_message.data(), xio_message.rlen(), &xio_msgctxt))
    {
        switch (xio_msgctxt.io_cmid)
        {
        case CMID_DLOAD_LOGIN : xit_error = iores_login(xio_msgctxt.io_seqn, xio_msgctxt.io_dptr, xio_msgctxt.io_size); break;
        case CMID_DLOAD_CHUNK : xit_error = iores_chunk(xio_msgctxt.io_seqn, xio_msgctxt.io_dptr, xio_msgctxt.io_size); break;

        default:
            break;
        }
    }

    return xit_error;
}

/**********************************************************/
/**
 * @brief 处理 “IO 通道对象被销毁” 的事件。
 */
x_int32_t x_ftp_download_t::io_event_destroyed(void)
{
    if (X_NULL != m_xht_fstream)
    {
        fclose((FILE *)m_xht_fstream);
        m_xht_fstream = X_NULL;
    }

    return 0;
}

//====================================================================

// 
// x_ftp_download_t : internal invoking
// 

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
x_int32_t x_ftp_download_t::post_res_chunk(x_uint16_t xut_seqn, x_int64_t xit_offset, x_uint32_t xut_rdsize)
{
    x_int32_t xit_error  = 0;
    x_bool_t  xbt_finish = X_FALSE;

    do
    {
        //======================================
        // 参数有效校验

        if ((X_NULL == m_xht_fstream) || (xut_rdsize <= 0))
        {
            LOGE("[fd:%d](X_NULL == m_xht_fstream) || (xut_rdsize[%d] <= 0)",
                 get_sockfd(), xut_rdsize);
            xit_error = -1;
            break;
        }

        if (xit_offset >= m_xit_fsize)
        {
            LOGE("[fd:%d](xit_offset[%lld] >= m_xit_fsize[%lld])",
                 get_sockfd(), xit_offset, m_xit_fsize);
            xit_error  = 0;
            xbt_finish = X_TRUE;
            break;
        }

        if (xut_rdsize > ECV_MAX_CHUNK_SIZE)
        {
            xut_rdsize = ECV_MAX_CHUNK_SIZE;
        }

        if ((xit_offset + xut_rdsize) > m_xit_fsize)
        {
            xut_rdsize = (x_uint32_t)(m_xit_fsize - xit_offset);
        }

        //======================================
        // 偏移文件的读指针位置

        if (-1 == fseek((FILE *)m_xht_fstream, xit_offset, SEEK_SET))
        {
            LOGE("[fd:%d]-1 == fseek((FILE *)m_xht_fstream, xit_offset[%lld], SEEK_SET) error : %d",
                 get_sockfd(), xit_offset, errno);
            xit_error = -1;
            break;
        }

        //======================================
        // 读取数据

        x_tcp_io_message_t xio_message(IO_HDSIZE + sizeof(x_int64_t) + sizeof(x_uint32_t) + xut_rdsize);
        x_uchar_t * xct_dptr = xio_message.data() + IO_HDSIZE;

        *(x_ullong_t *)(xct_dptr) = vx_htonll(xit_offset); xct_dptr += sizeof(x_ullong_t);
        *(x_ulong_t  *)(xct_dptr) = vx_htonl (xut_rdsize); xct_dptr += sizeof(x_ulong_t );
        XVERIFY(xut_rdsize == (x_uint32_t)fread(xct_dptr, sizeof(x_uchar_t), xut_rdsize, (FILE *)m_xht_fstream));

        //======================================
        // 设置应答消息的头部信息后，加入到消息应答队列

        x_io_msgctxt_t xio_msgctxt;
        xio_msgctxt.io_seqn = xut_seqn;
        xio_msgctxt.io_cmid = CMID_DLOAD_CHUNK;
        xio_msgctxt.io_size = sizeof(x_int64_t) + sizeof(x_uint32_t) + xut_rdsize;
        xio_msgctxt.io_dptr = X_NULL; // 设置为 X_NULL 的时候，则不进行数据拷贝操作

        XVERIFY(IOCTX_ERR_OK == io_set_context(xio_message.data(), xio_message.capacity(), &xio_msgctxt));

        xio_message.reset(IO_HDSIZE + xio_msgctxt.io_size, 0);
        push_res_xmsg(std::move(xio_message));

        //======================================
        xit_error = 0;
    } while (0);

    // 产生错误的应答操作
    if ((0 != xit_error) || xbt_finish)
    {
        m_xbt_pause = X_TRUE;

        xit_error = vx_htonl(xit_error);

        x_io_msgctxt_t xio_msgctxt;
        xio_msgctxt.io_seqn = xut_seqn;
        xio_msgctxt.io_cmid = CMID_DLOAD_CHUNK;
        xio_msgctxt.io_size = sizeof(x_int32_t);
        xio_msgctxt.io_dptr = (x_uchar_t *)&xit_error;

        post_res_xmsg(xio_msgctxt);
    }

    return xit_error;
}

//====================================================================

// 
// x_ftp_download_t : requested message handlers
// 

/**********************************************************/
/**
 * @brief 处理 IO 请求命令：登录。
 */
x_int32_t x_ftp_download_t::iocmd_login(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size)
{
    x_int32_t xit_error = 0;

    do
    {
        //======================================

        if (xut_size <= 0)
        {
            LOGW("[fd:%d]file name is empty!", get_sockfd());
            xit_error = -1;
        }

        //======================================
        // 读取文件基本信息

        m_xstr_fname = std::move(std::string((x_char_t *)xct_dptr, xut_size));

        xit_error = x_ftp_server_t::instance().file_info(m_xstr_fname.c_str(), m_xstr_fpath, m_xit_fsize);
        if (0 != xit_error)
        {
            LOGE("[fd:%d]x_ftp_server_t::file_info(m_xstr_fname.c_str()[%s], ...) return error : %d",
                 get_sockfd(), m_xstr_fname.c_str(), xit_error);
            break;
        }

        //======================================
        // （只读方式）打开文件流操作句柄

        FILE * xfs_handle = fopen(m_xstr_fpath.c_str(), "rb");
        if (X_NULL == xfs_handle)
        {
            LOGE("[fd:%d]fopen(m_xstr_fpath.c_str()[%s], \"rb\") return X_NULL, errno : %d",
                 get_sockfd(), m_xstr_fpath.c_str(), errno);
            xit_error = -1;
            break;
        }

        m_xht_fstream = (x_handle_t)xfs_handle;

        //======================================
        xit_error = 0;
    } while (0);

    xit_error = vx_htonl(xit_error);

    x_io_msgctxt_t xio_msgctxt;
    xio_msgctxt.io_seqn = xut_seqn;
    xio_msgctxt.io_cmid = CMID_DLOAD_LOGIN;
    xio_msgctxt.io_size = sizeof(x_int32_t);
    xio_msgctxt.io_dptr = (x_uchar_t *)&xit_error;

    post_res_xmsg(xio_msgctxt);

    return 0;
}

/**********************************************************/
/**
 * @brief 处理 IO 请求命令：心跳。
 */
x_int32_t x_ftp_download_t::iocmd_hbeat(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size)
{
    x_io_msgctxt_t xio_msgctxt;
    xio_msgctxt.io_seqn = xut_seqn;
    xio_msgctxt.io_cmid = CMID_DLOAD_HBEAT;
    xio_msgctxt.io_size = 0;
    xio_msgctxt.io_dptr = X_NULL;

    post_res_xmsg(xio_msgctxt);

    return 0;
}

/**********************************************************/
/**
 * @brief 处理 IO 请求命令：下载文件块。
 */
x_int32_t x_ftp_download_t::iocmd_chunk(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size)
{
    x_int32_t xit_error = 0;

    do
    {
        //======================================
        // 提取请求参数

        // [文件读取偏移位置 8byte + 请求的文件块长度 4byte + 暂停标识 4byte]
        if ((X_NULL == xct_dptr) ||
            ((sizeof(x_int64_t) + sizeof(x_uint32_t) + sizeof(x_uint32_t)) != xut_size))
        {
            LOGE("[fd:%d] xut_size == %d", get_sockfd(), xut_size);
            xit_error = -1;
            break;
        }

        x_int64_t  xit_offset = (x_int64_t )vx_ntohll(*(x_ullong_t *)(xct_dptr));
        x_uint32_t xut_rdsize = (x_uint32_t)vx_ntohl (*(x_ulong_t  *)(xct_dptr + sizeof(x_ullong_t)));
        x_uint32_t xut_mpause = (x_uint32_t)vx_ntohl (*(x_ulong_t  *)(xct_dptr + sizeof(x_ullong_t) + sizeof(x_uint32_t)));

        //======================================

        m_xbt_pause = (0 != xut_mpause) ? X_TRUE : X_FALSE;

        xit_error = post_res_chunk(xut_seqn, xit_offset, xut_rdsize);
        if (0 != xit_error)
        {
            LOGE("[fd:%d]post_res_chunk(xut_seqn[%d], xit_offset[%lld], xut_rdsize[%d]) return error : %d",
                 get_sockfd(), xut_seqn, xit_offset, xut_rdsize, xit_error);
            break;
        }

        //======================================
        xit_error = 0;
    } while (0);

    return 0;
}

/**********************************************************/
/**
 * @brief 处理 IO 请求命令：暂停下载。
 */
x_int32_t x_ftp_download_t::iocmd_pause(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size)
{
    if ((X_NULL != xct_dptr) && (sizeof(x_uint32_t) == xut_size))
    {
        m_xbt_pause = (0 != vx_ntohl(*(x_uint32_t *)xct_dptr)) ? X_TRUE : X_FALSE;
    }

    x_uint32_t xut_pause = vx_htonl((x_uint32_t)m_xbt_pause);

    x_io_msgctxt_t xio_msgctxt;
    xio_msgctxt.io_seqn = xut_seqn;
    xio_msgctxt.io_cmid = CMID_DLOAD_PAUSE;
    xio_msgctxt.io_size = sizeof(x_uint32_t);
    xio_msgctxt.io_dptr = (x_uchar_t *)&xut_pause;

    post_res_xmsg(xio_msgctxt);

    return 0;
}

//====================================================================

// 
// x_ftp_download_t : responsed message handlers
// 

/**********************************************************/
/**
 * @brief 处理 IO 完成应答命令：登录。
 */
x_int32_t x_ftp_download_t::iores_login(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size)
{
    x_int32_t xit_error = 0;

    if (sizeof(x_int32_t) == xut_size)
    {
        xit_error = (x_int32_t)vx_ntohl(*(x_int32_t *)xct_dptr);
        if (0 != xit_error)
        {
            LOGW("[fd:%d] login error : %d", get_sockfd(), xit_error);
        }
    }

    return xit_error;
}

/**********************************************************/
/**
 * @brief 处理 IO 完成应答命令：下载文件块。
 */
x_int32_t x_ftp_download_t::iores_chunk(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size)
{
    if (m_xbt_pause)
    {
        return 0;
    }

    x_int32_t xit_error = 0;

    do
    {
        //======================================

        // [文件读取偏移位置 8byte + 请求的文件块长度 4byte]
        if ((X_NULL == xct_dptr) ||
            (xut_size < (sizeof(x_int64_t) + sizeof(x_uint32_t))))
        {
            LOGE("[fd:%d] xut_size == %d", get_sockfd(), xut_size);
            xit_error = -1;
            break;
        }

        x_int64_t  xit_offset = (x_int64_t )vx_ntohll(*(x_ullong_t *)(xct_dptr));
        x_uint32_t xut_rdsize = (x_uint32_t)vx_ntohl (*(x_ulong_t  *)(xct_dptr + sizeof(x_ullong_t)));

        xit_offset += xut_rdsize;

        //======================================

        xit_error = post_res_chunk(xut_seqn, xit_offset, xut_rdsize);
        if (0 != xit_error)
        {
            LOGE("[fd:%d]post_res_chunk(xut_seqn[%d], xit_offset[%lld], xut_rdsize[%d]) return error : %d",
                 get_sockfd(), xut_seqn, xit_offset, xut_rdsize, xit_error);
            break;
        }

        //======================================
        xit_error = 0;
    } while (0);

    return 0;
}
