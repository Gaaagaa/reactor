/**
 * @file    xcomm.cpp
 * <pre>
 * Copyright (c) 2018, Gaaagaa All rights reserved.
 * 
 * 文件名称：xcomm.cpp
 * 创建日期：2018年12月20日
 * 文件标识：
 * 文件摘要：包含公共数据类型、接口等的实现源文件。
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

#include "xcomm.h"

#ifdef _MSC_VER

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <windows.h>

#else // !_MSC_VER

#include <unistd.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <arpa/inet.h>

#define gettid() syscall(__NR_gettid)

#endif // _MSC_VER

////////////////////////////////////////////////////////////////////////////////

/**********************************************************/
/**
 * @brief 获取版本号的数值。
 */
x_uint32_t get_version_value(void)
{
    return ((XVERSION_MAJOR << 16) | (XVERSION_MINOR));
}

/**********************************************************/
/**
 * @brief 获取版本号的文本信息。
 */
x_cstring_t get_version_text(void)
{
    static x_char_t xszt_version[TEXT_LEN_16] = { '\0' };
    if ('\0' == xszt_version[0])
    {
        snprintf(xszt_version, TEXT_LEN_16, "%d.%d.%d.%d",
                 XVERSION_MAJOR >> 8,
                 XVERSION_MAJOR & 0x00FF,
                 XVERSION_MINOR >> 8,
                 XVERSION_MINOR & 0x00FF);
    }

    return xszt_version;
}

/**********************************************************/
/**
 * @brief 系统时钟（单位为 ms 毫秒，1970年1月1日到现在的时间）。
 */
x_uint64_t get_time_tick(void)
{
#ifdef _MSC_VER
    // 1601 ~ 1970 年之间的时间 百纳秒数
    const x_uint64_t NS100_1970 = 116444736000000000ULL;

    FILETIME       xfilev_time;
    ULARGE_INTEGER xvalue_time;

    GetSystemTimeAsFileTime(&xfilev_time);
    xvalue_time.LowPart  = xfilev_time.dwLowDateTime;
    xvalue_time.HighPart = xfilev_time.dwHighDateTime;

    return ((xvalue_time.QuadPart - NS100_1970) / 10000ULL);
#else // !_MSC_VER
    struct timeval tmval;
    gettimeofday(&tmval, X_NULL);

    return (x_uint64_t)(tmval.tv_sec * 1000ULL + tmval.tv_usec / 1000ULL);
#endif // _MSC_VER
}

/**********************************************************/
/**
 * @brief 获取当前进程 ID 值。
 */
x_uint32_t get_pid(void)
{
#ifdef _MSC_VER
    return (x_uint32_t)GetCurrentProcessId();
#else // !_MSC_VER
    return (x_uint32_t)getpid();
#endif // _MSC_VER
}

/**********************************************************/
/**
 * @brief 获取当前线程 ID 值。
 */
x_uint32_t get_tid(void)
{
#ifdef _MSC_VER
    return (x_uint32_t)GetCurrentThreadId();
#else // !_MSC_VER
    return (x_uint32_t)gettid();
#endif // _MSC_VER
}

/**********************************************************/
/**
 * @brief 从文件全路径名中，获取其文件名（例如：/Folder/filename.txt 返回 filename.txt）。
 * 
 * @param [in ] xszt_file_path : 文件全路径名。
 * 
 * @return x_cstring_t
 *         - 返回对应的 文件名。
 */
x_cstring_t file_base_name(x_cstring_t xszt_file_path)
{
    register x_char_t * xct_iter = (x_char_t *)xszt_file_path;
    register x_char_t * xct_vpos = (x_char_t *)xszt_file_path;

    if (X_NULL == xct_vpos)
    {
        return X_NULL;
    }

    while (*xct_iter)
    {
        if (('\\' == *xct_iter) || ('/' == *xct_iter))
            xct_vpos = xct_iter + 1;
        xct_iter += 1;
    }

    return (x_cstring_t)xct_vpos;
}

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
x_int32_t singleton_run(x_cstring_t xszt_fname)
{
    x_int32_t    xit_fd = -1;
    struct flock xflocker;
    x_char_t     xszt_fnbuf[TEXT_LEN_32] = { 0 };

    //======================================
    // open file

    if ((X_NULL == xszt_fname) || ('\0' == xszt_fname[0]))
    {
        return X_FALSE;
    }

    xit_fd = open(xszt_fname, O_RDWR | O_CREAT, (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
    if (xit_fd < 0)
    {
        return -1;
    }

    //======================================
    // lock file

    xflocker.l_type   = F_WRLCK;
    xflocker.l_start  = 0;
    xflocker.l_whence = SEEK_SET;
    xflocker.l_len    = 0;

    if (-1 == fcntl(xit_fd, F_SETLK, &xflocker))
    {
        close(xit_fd);
        return -1;
    }

    //======================================
    // truncate file

    ftruncate(xit_fd, 0);
    write(xit_fd, xszt_fnbuf, snprintf(xszt_fnbuf, TEXT_LEN_32, "%ld\n", (x_long_t)getpid()) + 1);

    // do not close file...

    //======================================

    return xit_fd;
}

////////////////////////////////////////////////////////////////////////////////

/**********************************************************/
/**
 * @brief 关闭套接字。
 */
x_int32_t sockfd_close(x_sockfd_t xfdt_sockfd)
{
#ifdef _MSC_VER
    return closesocket(xfdt_sockfd);
#else // !_MSC_VER
    return close(xfdt_sockfd);
#endif // _MSC_VER
}

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
x_int32_t sockfd_shutdown(x_sockfd_t xfdt_sockfd, x_int32_t xit_how)
{
    return shutdown(xfdt_sockfd, xit_how);
}

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
x_cstring_t sockfd_local_ip(x_sockfd_t xfdt_sockfd, x_string_t xszt_ip_bptr, x_uint32_t xut_size)
{
    struct sockaddr_in sa;
    x_int32_t xit_skaddr_len = sizeof(sa);
    if (0 == getsockname(xfdt_sockfd, (struct sockaddr *)&sa, (socklen_t *)&xit_skaddr_len))
    {
        return inet_ntop(AF_INET, &sa.sin_addr, xszt_ip_bptr, xut_size);
    }

    return "\0";
}

/**********************************************************/
/**
 * @brief 由套接字获取其本地连接的 端口号。
 *
 * @param [in ] xfdt_sockfd : 操作的套接字。
 *
 * @return x_uint16_t
 *         - 本地连接的端口号。
 */
x_uint16_t sockfd_local_port(x_sockfd_t xfdt_sockfd)
{
    struct sockaddr_in sa;
    x_int32_t xit_skaddr_len = sizeof(sa);
    if (0 == getsockname(xfdt_sockfd, (struct sockaddr *)&sa, (socklen_t *)&xit_skaddr_len))
    {
        return ntohs(sa.sin_port);
    }

    return (x_uint16_t)-1;
}

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
x_cstring_t sockfd_remote_ip(x_sockfd_t xfdt_sockfd, x_string_t xszt_ip_bptr, x_uint32_t xut_size)
{
    struct sockaddr_in sa;
    x_int32_t xit_skaddr_len = sizeof(sa);
    if (0 == getpeername(xfdt_sockfd, (struct sockaddr *)&sa, (socklen_t *)&xit_skaddr_len))
    {
        return inet_ntop(AF_INET, &sa.sin_addr, xszt_ip_bptr, xut_size);
    }

    return "\0";
}

/**********************************************************/
/**
 * @brief 由套接字获取其远程连接的 端口号。
 *
 * @param [in ] xfdt_sockfd : 操作的套接字。
 *
 * @return x_uint16_t
 *         - 远程连接的端口号。
 */
x_uint16_t sockfd_remote_port(x_sockfd_t xfdt_sockfd)
{
    struct sockaddr_in sa;
    x_int32_t xit_skaddr_len = sizeof(sa);
    if (0 == getpeername(xfdt_sockfd, (struct sockaddr *)&sa, (socklen_t *)&xit_skaddr_len))
    {
        return ntohs(sa.sin_port);
    }

    return (x_uint16_t)-1;
}

////////////////////////////////////////////////////////////////////////////////

