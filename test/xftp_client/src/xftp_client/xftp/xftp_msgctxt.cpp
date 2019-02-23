/**
 * @file    xftp_msgctxt.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xftp_msgctxt.cpp
 * 创建日期：2019年01月22日
 * 文件标识：
 * 文件摘要：定义网络 IO 消息的协议头，并提供相关的数据 解析、设置 等操作接口。
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

#include "xftp_msgctxt.h"
#include <string.h>

#ifdef _MSC_VER
#include <WinSock2.h>
#else // !_MSC_VER
#include <arpa/inet.h>
#endif // _MSC_VER

////////////////////////////////////////////////////////////////////////////////

#ifndef ENABLE_XASSERT
#if ((defined _DEBUG) || (defined DEBUG))
#define ENABLE_XASSERT 1
#else // !((defined _DEBUG) || (defined DEBUG))
#define ENABLE_XASSERT 0
#endif // ((defined _DEBUG) || (defined DEBUG))
#endif // ENABLE_XASSERT

#ifndef XASSERT
#if ENABLE_XASSERT
#include <cassert>
#define XASSERT(xptr)    assert(xptr)
#else // !ENABLE_XASSERT
#define XASSERT(xptr)
#endif // ENABLE_XASSERT
#endif // XASSERT

////////////////////////////////////////////////////////////////////////////////

/**********************************************************/
/**
 * @brief 等效于 伯克利套接字 API 的 ntohs() 接口。
 */
x_uint16_t vx_ntohs(x_uint16_t xut_short)
{
    return ntohs(xut_short);
}

/**********************************************************/
/**
 * @brief 等效于 伯克利套接字 API 的 htons() 接口。
 */
x_uint16_t vx_htons(x_uint16_t xut_short)
{
    return htons(xut_short);
}

/**********************************************************/
/**
 * @brief 等效于 伯克利套接字 API 的 ntohl() 接口。
 */
x_uint32_t vx_ntohl(x_uint32_t xut_long)
{
    return ntohl(xut_long);
}

/**********************************************************/
/**
 * @brief 等效于 伯克利套接字 API 的 htonl() 接口。
 */
x_uint32_t vx_htonl(x_uint32_t xut_long)
{
    return htonl(xut_long);
}

/**********************************************************/
/**
 * @brief 判断系统是否为小端字节序。
 */
static inline x_bool_t is_little_endian(void)
{
    union x_little_endian_t
    {
        x_int32_t xit_32;
        x_int8_t  xit_8;
    } xlet_value;

    xlet_value.xit_32 = 1;

    return (1 == xlet_value.xit_8);
}

/**********************************************************/
/**
 * @brief 字节序转换：64 位整数从 网络字节序 转成 主机字节序。
 */
x_uint64_t vx_ntohll(x_uint64_t xult_llong)
{
#ifdef _MSC_VER
    static x_bool_t xbt_little_endian = is_little_endian();
    if (xbt_little_endian)
        return (((x_uint64_t)ntohl((x_uint32_t)(xult_llong & 0x00000000FFFFFFFFLL))) << 32) |
                ((x_uint64_t)ntohl((x_uint32_t)(xult_llong >> 32)));
    else
        return xult_llong;
#else // !_MSC_VER
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
    return (((x_uint64_t)ntohl((x_uint32_t)(xult_llong & 0x00000000FFFFFFFFLL))) << 32) |
            ((x_uint64_t)ntohl((x_uint32_t)(xult_llong >> 32)));
#else // (__BYTE_ORDER == __BIG_ENDIAN)
    return xult_llong;
#endif // (__BYTE_ORDER == __LITTLE_ENDIAN)
#endif // _MSC_VER
}

/**********************************************************/
/**
 * @brief 字节序转换：64 位整数从 主机字节序 转成 网络字节序。
 */
x_uint64_t vx_htonll(x_uint64_t xult_llong)
{
#ifdef _MSC_VER
    static x_bool_t xbt_little_endian = is_little_endian();
    if (xbt_little_endian)
        return (((x_uint64_t)htonl((x_uint32_t)(xult_llong & 0x00000000FFFFFFFFLL))) << 32) |
                ((x_uint64_t)htonl((x_uint32_t)(xult_llong >> 32)));
    else
        return xult_llong;
#else // !_MSC_VER
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
    return (((x_uint64_t)htonl((x_uint32_t)(xult_llong & 0x00000000FFFFFFFFLL))) << 32) |
            ((x_uint64_t)htonl((x_uint32_t)(xult_llong >> 32)));
#else // (__BYTE_ORDER == __BIG_ENDIAN)
    return xult_llong;
#endif // (__BYTE_ORDER == __LITTLE_ENDIAN)
#endif // _MSC_VER
}

////////////////////////////////////////////////////////////////////////////////

#define IO_CHECK_LEAD(xptr)     ((0xEF == (xptr)[0]) && (0xFE == (xptr)[1]))
#define IO_WRITE_LEAD(xptr)     do { (xptr)[0] = 0xEF; (xptr)[1] = 0xFE; } while(0)

/**********************************************************/
/**
 * @brief 计算校验和。
 * 
 * @param [in ] xct_dptr : 缓存地址。
 * @param [in ] xut_size : 缓存大小。
 * 
 * @return x_uint16_t
 *         - 返回 主机字节序 的校验和。
 */
x_uint16_t io_check_sum(const x_uchar_t * xct_dptr, x_uint32_t xut_size)
{
    x_uint32_t xut_iter = 0;
    x_uint32_t xut_csum = 0;

    XASSERT(X_NULL != xct_dptr);

    for (; xut_iter < xut_size; ++xut_iter)
    {
        xut_csum += *xct_dptr++;
    }

    return (x_uint16_t)(xut_csum & 0x0000FFFF);
}

/**********************************************************/
/**
 * @brief 从缓存数据中查找 首个网络 IO 消息 的前导码位置。
 * 
 * @param [in ] xct_io_dptr : 网络 IO 消息缓存。
 * @param [in ] xut_io_dlen : 网络 IO 消息缓存长度。
 * 
 * @return x_int32_t
 *         - 成功，返回 前导码偏移位置；
 *         - 失败，返回 -1。
 */
x_int32_t io_find_ldcode(const x_uchar_t * xct_io_dptr, x_uint32_t xut_io_dlen)
{
    x_int32_t xit_iter = -1;

    const x_uchar_t * xct_bptr = xct_io_dptr;
    const x_uchar_t * xct_eptr = xct_io_dptr + xut_io_dlen - IO_HDSIZE + 1;

    XASSERT(X_NULL != xct_io_dptr);

    if (xut_io_dlen < IO_HDSIZE)
    {
        return xit_iter;
    }

    for (; xct_bptr != xct_eptr; ++xct_bptr)
    {
        if (IO_CHECK_LEAD(xct_bptr))
        {
            xit_iter = (x_int32_t)(xct_bptr - xct_io_dptr);
            break;
        }
    }

    return xit_iter;
}

/**********************************************************/
/**
 * @brief 从网络 IO 消息缓存中，读取（解析出） IO 消息上下文描述信息。
 * 
 * @param [in ] xct_io_dptr : 网络 IO 消息缓存。
 * @param [in ] xut_io_dlen : 网络 IO 消息缓存长度。
 * @param [out] xio_ctx_ptr : 操作成功返回的 IO 消息上下文描述信息。
 * 
 * @return x_int32_t
 *         - 成功，返回 IOCTX_ERR_OK；
 *         - 失败，返回 错误码。
 */
x_int32_t io_get_context(const x_uchar_t * xct_io_dptr, x_uint32_t xut_io_dlen, x_io_msgctxt_t * xio_ctx_ptr)
{
    x_io_msghead_t * xio_nptr = (x_io_msghead_t *)xct_io_dptr;
    x_uint16_t       xio_size = 0;

    //======================================
    // 参数的有效验证

    XASSERT(X_NULL != xct_io_dptr);
    XASSERT(X_NULL != xio_ctx_ptr);

    if (xut_io_dlen < IO_HDSIZE)
    {
        if ((xut_io_dlen >= IO_LEAD_SIZE) && !IO_CHECK_LEAD(xct_io_dptr))
            return IOCTX_ERR_STRUCT;
        return IOCTX_ERR_PART;
    }

    if (!IO_CHECK_LEAD(xct_io_dptr))
    {
        return IOCTX_ERR_STRUCT;
    }

    //======================================
    // 数据体长度

    xio_size = vx_ntohs(xio_nptr->io_size);
    if ((IO_HDSIZE + xio_size) > xut_io_dlen)
    {
        return IOCTX_ERR_PART;
    }

    //======================================
    // 校验和判断

    if (vx_ntohs(xio_nptr->io_csum) != io_check_sum(xct_io_dptr + IO_CHKSUM_BPOS, (IO_HDSIZE - IO_CHKSUM_BPOS) + xio_size))
    {
        return IOCTX_ERR_CHKSUM;
    }

    //======================================
    // 设置返回的 IO 消息上下文描述信息

    xio_ctx_ptr->io_seqn = vx_ntohs(xio_nptr->io_seqn);
    xio_ctx_ptr->io_cmid = vx_ntohs(xio_nptr->io_cmid);
    xio_ctx_ptr->io_size = xio_size;
    xio_ctx_ptr->io_dptr = xio_nptr->io_dptr;

    //======================================

    return IOCTX_ERR_OK;
}

/**********************************************************/
/**
 * @brief 对网络 IO 消息缓存，写入 IO 消息上下文描述信息。
 * 
 * @param [out] xct_io_dptr : 网络 IO 消息缓存。
 * @param [in ] xut_io_dlen : 网络 IO 消息缓存长度。
 * @param [in ] xio_ctx_ptr : 写入的 IO 消息上下文描述信息。
 * 
 * @return x_int32_t
 *         - 成功，返回 IOCTX_ERR_OK；
 *         - 失败，返回 错误码。
 */
x_int32_t io_set_context(x_uchar_t * xct_io_dptr, x_uint32_t xut_io_dlen, const x_io_msgctxt_t * xio_ctx_ptr)
{
    x_io_msghead_t * xio_nptr = (x_io_msghead_t *)xct_io_dptr;

    //======================================
    // 参数的有效验证

    XASSERT(X_NULL != xct_io_dptr);
    XASSERT(X_NULL != xio_ctx_ptr);

    if ((xut_io_dlen < (IO_HDSIZE + xio_ctx_ptr->io_size)) ||
        (xio_ctx_ptr->io_size > 0x0000FFFF))
    {
        return IOCTX_ERR_PARAM;
    }

    // 前导码
    IO_WRITE_LEAD(xct_io_dptr);

    // 标识号
    xio_nptr->io_seqn = vx_htons(xio_ctx_ptr->io_seqn);

    // 命令ID
    xio_nptr->io_cmid = vx_htons(xio_ctx_ptr->io_cmid);

    // 数据体长度
    xio_nptr->io_size = vx_htons((x_uint16_t)(xio_ctx_ptr->io_size & 0x0000FFFF));

    // 数据体
    if ((xio_ctx_ptr->io_size > 0) && (X_NULL != xio_ctx_ptr->io_dptr))
    {
        memcpy(xio_nptr->io_dptr, xio_ctx_ptr->io_dptr, xio_ctx_ptr->io_size);
    }

    // 校验和
    xio_nptr->io_csum = vx_htons(io_check_sum(xct_io_dptr + IO_CHKSUM_BPOS, (IO_HDSIZE - IO_CHKSUM_BPOS) + xio_ctx_ptr->io_size));

    //======================================

    return IOCTX_ERR_OK;
}

/**********************************************************/
/**
 * @brief 从缓存数据中查找首个有效的网络 IO 消息。
 * 
 * @param [in ] xct_io_dptr : 网络 IO 消息缓存。
 * @param [in ] xut_io_dlen : 网络 IO 消息缓存长度。
 * @param [out] xio_ctx_ptr : 操作成功返回的 IO 消息上下文描述信息。
 * 
 * @return x_int32_t
 *         - 成功，返回 有效 IO 消息的偏移位置；
 *         - 失败，返回 -1。
 */
x_int32_t io_find_context(const x_uchar_t * xct_io_dptr, x_uint32_t xut_io_dlen, x_io_msgctxt_t * xio_ctx_ptr)
{
    x_int32_t xit_vpos = -1;
    x_int32_t xit_size = (x_int32_t)(xut_io_dlen) - IO_HDSIZE + 1;
    x_int32_t xit_iter = 0;

    XASSERT(X_NULL != xct_io_dptr);
    XASSERT(X_NULL != xio_ctx_ptr);

    if (xut_io_dlen < IO_HDSIZE)
    {
        return xit_vpos;
    }

    for (; xit_iter < xit_size; ++xit_iter)
    {
        if (IOCTX_ERR_OK == io_get_context(xct_io_dptr + xit_iter, xut_io_dlen - xit_iter, xio_ctx_ptr))
        {
            xit_vpos = xit_iter;
            break;
        }
    }

    return xit_vpos;
}

/**********************************************************/
/**
 * @brief 从网络 IO 消息缓存中，读取 IO 消息上下文描述信息（不进行校验和运算）。
 * 
 * @param [in ] xct_io_dptr : 网络 IO 消息缓存。
 * @param [in ] xut_io_dlen : 网络 IO 消息缓存长度。
 * @param [out] xio_ctx_ptr : 操作成功返回的 IO 消息上下文描述信息。
 * 
 * @return x_int32_t
 *         - 成功，返回 IOCTX_ERR_OK；
 *         - 失败，返回 错误码。
 */
x_int32_t io_context_rinfo(const x_uchar_t * xct_io_dptr, x_uint32_t xut_io_dlen, x_io_msgctxt_t * xio_ctx_ptr)
{
    x_io_msghead_t * xio_nptr = (x_io_msghead_t *)xct_io_dptr;
    x_uint16_t       xio_size = 0;

    //======================================
    // 参数的有效验证

    XASSERT(X_NULL != xct_io_dptr);
    XASSERT(X_NULL != xio_ctx_ptr);

    if (xut_io_dlen < IO_HDSIZE)
    {
        return IOCTX_ERR_PARAM;
    }

    if (!IO_CHECK_LEAD(xct_io_dptr))
    {
        return IOCTX_ERR_STRUCT;
    }

    //======================================
    // 数据体长度

    xio_size = vx_ntohs(xio_nptr->io_size);
    if ((IO_HDSIZE + xio_size) > xut_io_dlen)
    {
        return IOCTX_ERR_PART;
    }

    //======================================
    // 设置返回的 IO 消息上下文描述信息

    xio_ctx_ptr->io_seqn = vx_ntohs(xio_nptr->io_seqn);
    xio_ctx_ptr->io_cmid = vx_ntohs(xio_nptr->io_cmid);
    xio_ctx_ptr->io_size = xio_size;
    xio_ctx_ptr->io_dptr = xio_nptr->io_dptr;

    //======================================

    return IOCTX_ERR_OK;
}

/**********************************************************/
/**
 * @brief 对网络 IO 消息缓存，写入 IO 消息上下文描述信息（不进行校验和运算与数据拷贝）。
 * 
 * @param [out] xct_io_dptr : 网络 IO 消息缓存。
 * @param [in ] xut_io_dlen : 网络 IO 消息缓存长度。
 * @param [in ] xio_ctx_ptr : 写入的 IO 消息上下文描述信息。
 * 
 * @return x_int32_t
 *         - 成功，返回 IOCTX_ERR_OK；
 *         - 失败，返回 错误码。
 */
x_int32_t io_context_winfo(x_uchar_t * xct_io_dptr, x_uint32_t xut_io_dlen, const x_io_msgctxt_t * xio_ctx_ptr)
{
    x_io_msghead_t * xio_nptr = (x_io_msghead_t *)xct_io_dptr;

    //======================================
    // 参数的有效验证

    XASSERT(X_NULL != xct_io_dptr);
    XASSERT(X_NULL != xio_ctx_ptr);

    if ((xut_io_dlen < (IO_HDSIZE + xio_ctx_ptr->io_size)) ||
        (xio_ctx_ptr->io_size > 0x0000FFFF))
    {
        return IOCTX_ERR_PARAM;
    }

    // 前导码
    IO_WRITE_LEAD(xct_io_dptr);

    // 标识号
    xio_nptr->io_seqn = vx_htons(xio_ctx_ptr->io_seqn);

    // 命令ID
    xio_nptr->io_cmid = vx_htons(xio_ctx_ptr->io_cmid);

    // 数据体长度
    xio_nptr->io_size = vx_htons((x_uint16_t)(xio_ctx_ptr->io_size & 0x0000FFFF));

    //======================================

    return IOCTX_ERR_OK;
}
