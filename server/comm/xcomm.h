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

#ifndef XASSERT
#if ((defined _DEBUG) || (defined DEBUG))
#include <cassert>
#define XASSERT(xptr)    assert(xptr)
#else // !((defined _DEBUG) || (defined DEBUG))
#define XASSERT(xptr)
#endif // ((defined _DEBUG) || (defined DEBUG))
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

/**
 * @enum  x_em_event_observer_type_t
 * @brief 定义所有事件观察者类型的枚举值。
 */
typedef enum x_em_event_observer_type_t
{
    EM_EOT_MASTER    = 0x00000100,    ///< master 观察者对象接收的事件类型
} x_em_event_observer_type_t;

////////////////////////////////////////////////////////////////////////////////

#include "xobserver.h"
#include "xlog_handler.h"
#include "xconfig.h"
#include "xevent_handler.h"

////////////////////////////////////////////////////////////////////////////////

#ifndef XVERIFY

#define XVERIFY(xptr)                            \
    do                                           \
    {                                            \
        if (!(xptr))                             \
        {                                        \
            LOGE("[%s, %d]XVERIFY(%s) error!",   \
                 __FILE__, __LINE__, ""#xptr);   \
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

/**********************************************************/
/**
 * @brief 利用文件锁的方式判断进程是否未单例运行。
 * 
 * @param [in ] xszt_fname : 文件锁的文件路径（如 /tmp/[进程实例名].pid ）。
 * 
 * @return x_int32_t
 *         - 返回 ≥ 0 文件描述符，表示当前进程未存在多个实例运行；
 *         - 返回 -1，进程单例运行的文件锁操作失败（可能已经存在另外的进程实例）。
 */
x_int32_t singleton_run(x_cstring_t xszt_fname);

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

#endif // __XCOMM_H__
