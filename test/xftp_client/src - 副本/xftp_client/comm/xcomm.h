/**
 * @file    xcomm.h
 * <pre>
 * Copyright (c) 2018, Gaaagaa All rights reserved.
 * 
 * 文件名称：xcomm.h
 * 创建日期：2018年12月20日
 * 文件标识：
 * 文件摘要：包含公共数据类型、接口等的声明头文件。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2018年12月20日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#ifndef __XCOMM_H__
#define __XCOMM_H__

#include "xtypes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <list>
#include <map>
#include <vector>
#include <queue>
#include <string>
#include <memory>
#include <tuple>
#include <thread>
#include <chrono>

////////////////////////////////////////////////////////////////////////////////
// defined global macro

#define XVERSION_MAJOR   0x0100
#define XVERSION_MINOR   0x0000

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

#ifndef XUNUSED
#define XUNUSED(x) ((void)(x))
#endif // XUNUSED

#ifndef SELECTANY
#ifdef _MSC_VER
#define SELECTANY   __declspec(selectany)
#else // !_MSC_VER
#define SELECTANY   __attribute__((weak))
#endif // _MSC_VER
#endif // SELECTANY

#ifndef ENABLE_LOG_OUTPUT
#define ENABLE_LOG_OUTPUT 1
#endif // ENABLE_LOG_OUTPUT

#if ((defined _DEBUG) || (defined DEBUG))
#ifndef ENABLE_LOG_EMBED_STD
#define ENABLE_LOG_EMBED_STD 1
#endif // ENABLE_LOG_EMBED_STD
#endif // ((defined _DEBUG) || (defined DEBUG))

////////////////////////////////////////////////////////////////////////////////

#ifndef XVERIFY

#define XVERIFY(xptr)                            \
    do                                           \
    {                                            \
        if (!(xptr))                             \
        {                                        \
            XASSERT(X_FALSE);                    \
        }                                        \
    } while (0)                                  \

#endif // XVERIFY

////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus

/**********************************************************/
/**
 * @brief 限制 某个值 在 指定闭区间值域 内取值。
 */
template< typename _Ty, typename _Tw = _Ty, typename _Tu = _Ty >
inline _Ty limit_bound(_Ty xst_value, const _Tw & xst_lower, const _Tu & xst_upper)
{
    XASSERT(static_cast< _Ty >(xst_lower) <= static_cast< _Ty >(xst_upper));
    if (xst_value > static_cast< _Ty >(xst_upper))
        return static_cast< _Ty >(xst_upper);
    if (xst_value < static_cast< _Ty >(xst_lower))
        return static_cast< _Ty >(xst_lower);
    return xst_value;
}

/**********************************************************/
/**
 * @brief 限制 某个值 的下限值。
 */
template< typename _Ty, typename _Tw = _Ty >
inline _Ty limit_lower(_Ty xst_value, const _Tw & xst_lower)
{
    if (xst_value < static_cast< _Ty >(xst_lower))
        return static_cast< _Ty >(xst_lower);
    return xst_value;
}

/**********************************************************/
/**
 * @brief 限制 某个值 的上限值。
 */
template< typename _Ty, typename _Tu = _Ty >
inline _Ty limit_upper(_Ty xst_value, const _Tu & xst_upper)
{
    if (xst_value > static_cast< _Ty >(xst_upper))
        return static_cast< _Ty >(xst_upper);
    return xst_value;
}

#endif // __cplusplus

////////////////////////////////////////////////////////////////////////////////

/**********************************************************/
/**
 * @brief 获取版本号的数值。
 */
x_uint32_t get_version_value(void);

/**********************************************************/
/**
 * @brief 获取版本号的文本信息。
 */
x_cstring_t get_version_text(void);

/**********************************************************/
/**
 * @brief 系统时钟（单位为 ms 毫秒，1970年1月1日到现在的时间）。
 */
x_uint64_t get_time_tick(void);

/**********************************************************/
/**
 * @brief 获取当前进程 ID 值。
 */
x_uint32_t get_pid(void);

/**********************************************************/
/**
 * @brief 获取当前线程 ID 值。
 */
x_uint32_t get_tid(void);

/**********************************************************/
/**
 * @brief 从文件全路径名中，获取其文件名（例如：/Folder/filename.txt 返回 filename.txt）。
 * 
 * @param [in ] xszt_file_path : 文件全路径名。
 * 
 * @return x_cstring_t
 *         - 返回对应的 文件名。
 */
x_cstring_t file_base_name(x_cstring_t xszt_file_path);

////////////////////////////////////////////////////////////////////////////////

/**********************************************************/
/**
 * @brief 关闭套接字。
 */
x_int32_t sockfd_close(x_sockfd_t xfdt_sockfd);

/**********************************************************/
/**
 * @brief 关闭套接字。
 * 
 * @param [in ] xfdt_sockfd : 套接字。
 * @param [in ] xit_how     : 关闭方式（0，关闭读这一半；1，关闭写这一半；2，读和写都关闭）。
 * 
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 -1。
 */
x_int32_t sockfd_shutdown(x_sockfd_t xfdt_sockfd, x_int32_t xit_how);

/**********************************************************/
/**
 * @brief 由套接字获取其本地连接的 IP 地址字符串。
 *
 * @param [in ] xfdt_sockfd  : 操作的套接字。
 * @param [out] xszt_ip_bptr : 操作成功 返回 IP 地址字符串的数据接收缓存。
 * @param [in ] xut_size     : 返回 IP 地址字符串的数据接收缓存大小。
 *
 * @return x_cstring_t
 *         - 成功，返回 本地连接的 IP 地址字符串；
 *         - 失败，返回 "\0"（空字符串）。
 */
x_cstring_t sockfd_local_ip(x_sockfd_t xfdt_sockfd, x_string_t xszt_ip_bptr, x_uint32_t xut_size);

/**********************************************************/
/**
 * @brief 由套接字获取其本地连接的 端口号。
 *
 * @param [in ] xfdt_sockfd : 操作的套接字。
 *
 * @return x_uint16_t
 *         - 本地连接的端口号。
 */
x_uint16_t sockfd_local_port(x_sockfd_t xfdt_sockfd);

/**********************************************************/
/**
 * @brief 由套接字获取其远程连接的 IP 地址字符串。
 *
 * @param [in ] xfdt_sockfd  : 操作的套接字。
 * @param [out] xszt_ip_bptr : 操作成功 返回 IP 地址字符串的数据接收缓存。
 * @param [in ] xut_size     : 返回 IP 地址字符串的数据接收缓存大小。
 *
 * @return x_cstring_t
 *         - 成功，返回 远程连接的 IP 地址字符串；
 *         - 失败，返回 "\0"（空字符串）。
 */
x_cstring_t sockfd_remote_ip(x_sockfd_t xfdt_sockfd, x_string_t xszt_ip_bptr, x_uint32_t xut_size);

/**********************************************************/
/**
 * @brief 由套接字获取其远程连接的 端口号。
 *
 * @param [in ] xfdt_sockfd : 操作的套接字。
 *
 * @return x_uint16_t
 *         - 远程连接的端口号。
 */
x_uint16_t sockfd_remote_port(x_sockfd_t xfdt_sockfd);

////////////////////////////////////////////////////////////////////////////////

/**********************************************************/
/**
 * @brief 判断字符串是否为有效的 4 段式 IP 地址格式。
 *
 * @param [in ] xszt_vptr : 判断的字符串。
 * @param [out] xut_value : 若入参不为 X_NULL，则操作成功时，返回对应的 IP 地址值。
 *
 * @return x_bool_t
 *         - 成功，返回 X_TRUE；
 *         - 失败，返回 X_FALSE。
 */
x_bool_t vx_ipv4_valid(x_cstring_t xszt_vptr, x_uint32_t * xut_value);

/**********************************************************/
/**
 * @brief 获取 域名下的 IP 地址表（取代系统的 gethostbyname() API 调用）。
 *
 * @param [in ] xszt_dname : 指定的域名（格式如：www.163.com）。
 * @param [in ] xit_family : 期待返回的套接口地址结构的类型。
 * @param [out] xvec_host  : 操作成功返回的地址列表。
 *
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t vx_gethostbyname(x_cstring_t xszt_dname, x_int32_t xit_family, std::vector< std::string > & xvec_host);

/**********************************************************/
/**
 * @brief 创建 TCP 的套接字连接。
 *
 * @param [in ] xszt_host : 目标主机的 IP 地址（服务端）。
 * @param [in ] xwt_port  : 连接的目标主机端口号。
 * @param [in ] xut_tmout : 连接超时时间（单位按 毫秒 ms 计）。
 * @param [out] xit_err   : 操作返回的错误码（入参不为 X_NULL 时，返回操作有效）。
 *
 * @return x_sockfd_t
 *         - 连接成功，返回 套接字；
 *         - 失败，返回 X_INVALID_SOCKFD。
 */
x_sockfd_t tcp_connect(x_cstring_t xszt_host, x_uint16_t xwt_port, x_uint32_t xut_tmout, x_int32_t * xit_err);

/**********************************************************/
/**
 * @brief 执行 TCP 连接套接字的数据发送操作。
 *
 * @param [in ] xfdt_sockfd : 连接套接字。
 * @param [in ] xct_wptr    : 数据发送缓存。
 * @param [in ] xit_size    : 数据发送缓存长度。
 * @param [in ] xit_flags   : 发送标识（对应 NPI send() 中的 flags 参数）。
 * @param [out] xit_err     : 操作返回的错误码（入参不为 X_NULL 时，返回操作有效）。
 *
 * @return x_int32_t
 *         - 实际发送的数据字节数量。
 */
x_int32_t tcp_send(x_sockfd_t xfdt_sockfd, const x_char_t * xct_wptr, x_int32_t xit_size, x_int32_t xit_flags, x_int32_t * xit_err);

////////////////////////////////////////////////////////////////////////////////

#endif // __XCOMM_H__
