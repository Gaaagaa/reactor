/**
 * @file    xftp_msgctxt.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xftp_msgctxt.h
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

#ifndef __XFTP_MSGCTXT_H__
#define __XFTP_MSGCTXT_H__

#include "xtypes.h"

////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

////////////////////////////////////////////////////////////////////////////////

/**********************************************************/
/**
 * @brief 等效于 伯克利套接字 API 的 ntohs() 接口。
 */
x_uint16_t vx_ntohs(x_uint16_t xut_short);

/**********************************************************/
/**
 * @brief 等效于 伯克利套接字 API 的 htons() 接口。
 */
x_uint16_t vx_htons(x_uint16_t xut_short);

/**********************************************************/
/**
 * @brief 等效于 伯克利套接字 API 的 ntohl() 接口。
 */
x_uint32_t vx_ntohl(x_uint32_t xut_long);

/**********************************************************/
/**
 * @brief 等效于 伯克利套接字 API 的 htonl() 接口。
 */
x_uint32_t vx_htonl(x_uint32_t xut_long);

/**********************************************************/
/**
 * @brief 字节序转换：64 位整数从 网络字节序 转成 主机字节序。
 */
x_ullong_t vx_ntohll(x_ullong_t xult_llong);

/**********************************************************/
/**
 * @brief 字节序转换：64 位整数从 主机字节序 转成 网络字节序。
 */
x_ullong_t vx_htonll(x_ullong_t xult_llong);

////////////////////////////////////////////////////////////////////////////////

/**
 * @struct x_io_msghead_t
 * @brief  网络 IO 消息头部描述信息。
 */
typedef struct x_io_msghead_t
{
    x_uint16_t   io_lead;    ///< 前导码（内存从低到高排列为 0xEF, 0xFE）
    x_uint16_t   io_csum;    ///< 校验和（值 = io_seqn + io_cmid + io_size + io_dptr）
    x_uint16_t   io_seqn;    ///< 标识号（通信操作时，请求与应答间的对等操作标识号）
    x_uint16_t   io_cmid;    ///< 命令ID
    x_uint16_t   io_size;    ///< 数据体长度（size(io_dptr)）

#ifdef _MSC_VER
#pragma warning(disable:4200)
#endif // _MSC_VER

    x_uchar_t    io_dptr[0]; ///< 数据体

#ifdef _MSC_VER
#pragma warning(default:4200)
#endif // _MSC_VER
} x_io_msghead_t;

#define IO_HDSIZE               (sizeof(x_io_msghead_t))
#define IO_LEAD_SIZE            (sizeof(x_uint16_t))
#define IO_CHKSUM_BPOS          (sizeof(x_uint16_t) + sizeof(x_uint16_t))

/**
 * @struct x_io_msgctxt_t
 * @brief  业务层 IO 消息的上下文描述信息。
 */
typedef struct x_io_msgctxt_t
{
    x_uint16_t    io_seqn;  ///< 标识号（通信操作时，请求与应答间的对等操作标识号）
    x_uint16_t    io_cmid;  ///< 命令ID
    x_uint32_t    io_size;  ///< 数据体长度（size(io_dptr)）
    x_uchar_t   * io_dptr;  ///< 数据体
} x_io_msgctxt_t;

//====================================================================

/**
 * @enum  emIoContextECode
 * @brief IO 消息解析操作的错误码表。
 */
typedef enum emIoContextECode
{
    IOCTX_ERR_OK     = (x_int32_t)0x00000000,  ///< 无错
    IOCTX_ERR_UNKNOW = (x_int32_t)0xFFFFFFFF,  ///< 未知
    IOCTX_ERR_PARAM  = (x_int32_t)0x80000010,  ///< 解析参数错误
    IOCTX_ERR_PART   = (x_int32_t)0x80000020,  ///< 消息不完整
    IOCTX_ERR_STRUCT = (x_int32_t)0x80000030,  ///< 结构错误
    IOCTX_ERR_CHKSUM = (x_int32_t)0x80000040,  ///< 校验和错误
} emIoContextECode;

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
x_uint16_t io_check_sum(const x_uchar_t * xct_dptr, x_uint32_t xut_size);

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
x_int32_t io_find_ldcode(const x_uchar_t * xct_io_dptr, x_uint32_t xut_io_dlen);

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
x_int32_t io_get_context(const x_uchar_t * xct_io_dptr, x_uint32_t xut_io_dlen, x_io_msgctxt_t * xio_ctx_ptr);

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
x_int32_t io_set_context(x_uchar_t * xct_io_dptr, x_uint32_t xut_io_dlen, const x_io_msgctxt_t * xio_ctx_ptr);

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
x_int32_t io_find_context(const x_uchar_t * xct_io_dptr, x_uint32_t xut_io_dlen, x_io_msgctxt_t * xio_ctx_ptr);

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
x_int32_t io_context_rinfo(const x_uchar_t * xct_io_dptr, x_uint32_t xut_io_dlen, x_io_msgctxt_t * xio_ctx_ptr);

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
x_int32_t io_context_winfo(x_uchar_t * xct_io_dptr, x_uint32_t xut_io_dlen, const x_io_msgctxt_t * xio_ctx_ptr);

////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}; // extern "C"
#endif // __cplusplus

////////////////////////////////////////////////////////////////////////////////

#endif // __XFTP_MSGCTXT_H__
