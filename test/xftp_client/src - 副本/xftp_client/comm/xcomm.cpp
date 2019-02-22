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

#pragma comment(lib, "ws2_32.lib")

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

/**********************************************************/
/**
 * @brief 返回套接字当前操作失败的错误码。
 */
static x_int32_t sockfd_lasterror(void)
{
#ifdef _MSC_VER
    return (x_int32_t)WSAGetLastError();
#else // !_MSC_VER
    return errno;
#endif // _MSC_VER
}

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
x_bool_t vx_ipv4_valid(x_cstring_t xszt_vptr, x_uint32_t * xut_value)
{
    x_uchar_t xct_ipv[4] = { 0, 0, 0, 0 };

    x_int32_t xit_itv = 0;
    x_int32_t xit_sum = 0;
    x_bool_t  xbt_okv = X_FALSE;

    x_char_t    xct_iter = '\0';
    x_cchar_t * xct_iptr = xszt_vptr;

    if (X_NULL == xszt_vptr)
    {
        return X_FALSE;
    }

    for (xct_iter = *xszt_vptr; X_TRUE; xct_iter = *(++xct_iptr))
    {
        if ((xct_iter != '\0') && (xct_iter >= '0') && (xct_iter <= '9'))
        {
            xit_sum = 10 * xit_sum + (xct_iter - '0');
            xbt_okv = X_TRUE;
        }
        else if (xbt_okv && (('\0' == xct_iter) || ('.' == xct_iter)) && (xit_itv < (x_int32_t)sizeof(xct_ipv)) && (xit_sum <= 0xFF))
        {
            xct_ipv[xit_itv++] = xit_sum;
            xit_sum = 0;
            xbt_okv = X_FALSE;
        }
        else
            break;

        if ('\0' == xct_iter)
        {
            break;
        }
    }

#define MAKE_IPV4_VALUE(b1,b2,b3,b4)  ((x_uint32_t)(((x_uint32_t)(b1)<<24)+((x_uint32_t)(b2)<<16)+((x_uint32_t)(b3)<<8)+((x_uint32_t)(b4))))

    xbt_okv = (xit_itv == sizeof(xct_ipv)) ? X_TRUE : X_FALSE;
    if (X_NULL != xut_value)
    {
        *xut_value = xbt_okv ? MAKE_IPV4_VALUE(xct_ipv[0], xct_ipv[1], xct_ipv[2], xct_ipv[3]) : 0xFFFFFFFF;
    }

#undef MAKE_IPV4_VALUE

    return xbt_okv;
}

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
x_int32_t vx_gethostbyname(x_cstring_t xszt_dname, x_int32_t xit_family, std::vector< std::string > & xvec_host)
{
    x_int32_t xit_err = -1;

    struct addrinfo   xai_hint;
    struct addrinfo * xai_rptr = X_NULL;
    struct addrinfo * xai_iptr = X_NULL;

    x_char_t xszt_ipaddr[TEXT_LEN_256] = { 0 };

    do
    {
        //======================================

        if (X_NULL == xszt_dname)
        {
            break;
        }

        memset(&xai_hint, 0, sizeof(xai_hint));
        xai_hint.ai_family = xit_family;
        xai_hint.ai_socktype = SOCK_STREAM;

        xit_err = getaddrinfo(xszt_dname, X_NULL, &xai_hint, &xai_rptr);
        if (0 != xit_err)
        {
            break;
        }

        //======================================

        for (xai_iptr = xai_rptr; X_NULL != xai_iptr; xai_iptr = xai_iptr->ai_next)
        {
            if (xit_family != xai_iptr->ai_family)
            {
                continue;
            }

            memset(xszt_ipaddr, 0, TEXT_LEN_256);
            if (X_NULL == inet_ntop(xit_family, &(((struct sockaddr_in *)(xai_iptr->ai_addr))->sin_addr), xszt_ipaddr, TEXT_LEN_256))
            {
                continue;
            }

            xvec_host.push_back(std::string(xszt_ipaddr));
        }

        //======================================

        xit_err = (xvec_host.size() > 0) ? 0 : -3;
    } while (0);

    if (X_NULL != xai_rptr)
    {
        freeaddrinfo(xai_rptr);
        xai_rptr = X_NULL;
    }

    return xit_err;
}

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
x_sockfd_t tcp_connect(x_cstring_t xszt_host, x_uint16_t xwt_port, x_uint32_t xut_tmout, x_int32_t * xit_err)
{
    x_int32_t   xit_ecode = 0;
    x_sockfd_t  xfdt_sockfd = X_INVALID_SOCKFD;

    sockaddr_in skaddr_host;
    fd_set      fd_rset;
    fd_set      fd_wset;
    timeval     tmval;
    x_uint32_t  xut_value = 0;
    x_int32_t   xit_count = 0;

#ifndef _MSC_VER
    x_int32_t xit_fcntl_flags = 0;
#endif // _MSC_VER

    do
    {
        //======================================

        skaddr_host.sin_family = AF_INET;
        skaddr_host.sin_port = htons(xwt_port);
        if (X_NULL != xszt_host)
            inet_pton(AF_INET, xszt_host, &skaddr_host.sin_addr.s_addr);
        else
            skaddr_host.sin_addr.s_addr = INADDR_NONE;
        if (INADDR_NONE == skaddr_host.sin_addr.s_addr)
        {
            xit_ecode = -1;
            break;
        }

        xfdt_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (X_INVALID_SOCKFD == xfdt_sockfd)
        {
            xit_ecode = sockfd_lasterror();
            break;
        }

        // set the xfdt_sockfd in non-blocking
#ifdef _MSC_VER
        xut_value = 1;
        ioctlsocket(xfdt_sockfd, FIONBIO, (u_long *)&xut_value);
#else // !_MSC_VER
        xit_fcntl_flags = fcntl(xfdt_sockfd, F_GETFL, 0);
        fcntl(xfdt_sockfd, F_SETFL, xit_fcntl_flags | O_NONBLOCK);
#endif // _MSC_VER

        //======================================

        if (0 != connect(xfdt_sockfd, (struct sockaddr *)&skaddr_host, sizeof(skaddr_host)))
        {
            xit_ecode = sockfd_lasterror();
#ifdef _MSC_VER
            if (WSAEWOULDBLOCK != xit_ecode)
#else // !_MSC_VER
            if (EINPROGRESS != xit_ecode)
#endif // _MSC_VER
            {
                sockfd_close(xfdt_sockfd);
                xfdt_sockfd = X_INVALID_SOCKFD;
                break;
            }
        }

        //======================================

        FD_ZERO(&fd_rset);
        FD_SET(xfdt_sockfd, &fd_rset);

        FD_ZERO(&fd_wset);
        FD_SET(xfdt_sockfd, &fd_wset);

        // check if the xfdt_sockfd is ready
        tmval.tv_sec = (xut_tmout / 1000);
        tmval.tv_usec = (xut_tmout % 1000) * 1000;

        xit_count = select((x_int32_t)(xfdt_sockfd + 1),
                           &fd_rset,
                           &fd_wset,
                           X_NULL,
                           (xut_tmout > 0) ? &tmval : X_NULL);
        if (xit_count <= 0)
        {
            xit_ecode = -1;
            break;
        }

        if (!FD_ISSET(xfdt_sockfd, &fd_rset) && !FD_ISSET(xfdt_sockfd, &fd_wset))
        {
#ifdef _MSC_VER
            xit_ecode = WSAETIMEDOUT;
#else // !_MSC_VER
            xit_ecode = ETIMEDOUT;
#endif // _MSC_VER
            break;
        }

        xit_ecode = 0;
        xut_value = sizeof(x_int32_t);
        if (0 != getsockopt(xfdt_sockfd, SOL_SOCKET, SO_ERROR, (x_char_t *)&xit_ecode, (socklen_t *)&xut_value))
        {
            sockfd_close(xfdt_sockfd);
            xfdt_sockfd = X_INVALID_SOCKFD;
            break;
        }

        if (0 != xit_ecode)
        {
            break;
        }

        //======================================
        // restore the xfdt_sockfd mode

#ifdef _MSC_VER
        xut_value = 0;
        ioctlsocket(xfdt_sockfd, FIONBIO, (u_long *)&xut_value);
#else // !_MSC_VER
        fcntl(xfdt_sockfd, F_SETFL, xit_fcntl_flags);
#endif // _MSC_VER

        xut_value = 1;
        setsockopt(xfdt_sockfd, SOL_SOCKET, SO_KEEPALIVE, (const x_char_t *)&xut_value, sizeof(x_uint32_t));
        setsockopt(xfdt_sockfd, IPPROTO_TCP, TCP_NODELAY, (const x_char_t *)&xut_value, sizeof(x_uint32_t));

        //======================================

        xit_ecode = 0;
    } while (0);

    if (0 != xit_ecode)
    {
        if (X_INVALID_SOCKFD != xfdt_sockfd)
        {
            sockfd_close(xfdt_sockfd);
            xfdt_sockfd = X_INVALID_SOCKFD;
        }
    }

    if (X_NULL != xit_err)
    {
        *xit_err = xit_ecode;
    }

    return xfdt_sockfd;
}

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
x_int32_t tcp_send(x_sockfd_t xfdt_sockfd, const x_char_t * xct_wptr, x_int32_t xit_size, x_int32_t xit_flags, x_int32_t * xit_err)
{
    x_int32_t xit_ncount = 0;
    x_int32_t xit_nbytes = 0;

#define SET_ERR(err)  do { if (X_NULL != xit_err) *xit_err = (err); } while(0)

    SET_ERR(0);
    if ((X_INVALID_SOCKFD == xfdt_sockfd) || (X_NULL == xct_wptr))
    {
        SET_ERR(-1);
        return 0;
    }

    xit_ncount = send(xfdt_sockfd, xct_wptr, xit_size, xit_flags);
    if (-1 == xit_ncount)
    {
        SET_ERR(sockfd_lasterror());
        return 0;
    }

    while (xit_ncount < xit_size)
    {
        xit_nbytes = send(xfdt_sockfd, xct_wptr + xit_ncount, xit_size - xit_ncount, xit_flags);
        if (-1 == xit_nbytes)
        {
            SET_ERR(sockfd_lasterror());
            break;
        }

        xit_ncount += xit_nbytes;
    }

#undef SET_ERR

    return xit_ncount;
}

