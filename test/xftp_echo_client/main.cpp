/**
 * @file    main.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：main.cpp
 * 创建日期：2019年02月13日
 * 文件标识：
 * 文件摘要：xftp 的 echo 测试客户端程序。
 * 编译命令：g++ -Wall -std=c++11 -lpthread -o xecho main.cpp xftp_msgctxt.cpp
 * 启动命令：./xecho <ip> <port> <name> <echo message text>
 *           如：./xecho 192.168.80.65 10086 Gaaagaa "0123456789"
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年02月13日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#include "xftp_msgctxt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <chrono>
#include <thread>
#include <string>

#ifdef _MSC_VER

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

 /**
  * @class vxWSASocketInit
  * @brief 自动 加载/卸载 WinSock 库的操作类。
  */
class vxWSASocketInit
{
    // constructor/destructor
public:
    vxWSASocketInit(x_int32_t xit_main_ver = 2, x_int32_t xit_sub_ver = 0)
    {
        WSAStartup(MAKEWORD(xit_main_ver, xit_sub_ver), &m_wsaData);
    }

    ~vxWSASocketInit(x_void_t)
    {
        WSACleanup();
    }

    // class data
protected:
    WSAData      m_wsaData;
};

vxWSASocketInit _S_vxWSASocketInit;

#else // !_MSC_VER

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>

#endif // _MSC_VER

////////////////////////////////////////////////////////////////////////////////

/**
 * @enum  emIoContextCmid
 * @brief IO 操作的命令 ID 表。
 */
typedef enum emIoContextCmid
{
    CMID_ECHO_LOGIN = 0x0010,  ///< 业务层工作对象类型
    CMID_ECHO_HBEAT = 0x2000,  ///< 心跳
    CMID_ECHO_TEXT  = 0x2010,  ///< 文本内容反射
} emIoContextCmid;

/**********************************************************/
/**
 * @brief 关闭套接字。
 */
x_int32_t sockfd_close(x_sockfd_t xfdt_sockfd);

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
 * @brief 发送 IO 消息。
 */
x_int32_t send_io_msgctxt(x_sockfd_t xfdt_sockfd, x_io_msgctxt_t & xio_msgctxt);

/**********************************************************/
/**
 * @brief 接收 IO 消息。
 */
x_int32_t recv_io_msgctxt(x_sockfd_t xfdt_sockfd, x_io_msgctxt_t & xio_msgctxt);

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv[])
{
    //======================================
    // 提取工作参数

    if (5 != argc)
    {
        printf("usage: %s <ip> <port> <name> <echo message text>\n", argv[0]);
        return -1;
    }

    x_cstring_t xszt_host = argv[1];
    x_uint16_t  xwt_port  = (x_uint16_t)atoi(argv[2]);
    x_cstring_t xszt_name = argv[3];
    x_cstring_t xszt_text = argv[4];
    x_int32_t   xit_mlen  = (x_int32_t)strlen(xszt_text);

    x_uint16_t xut_ioseqn = 0;

    //======================================
    // 建立网络连接

    x_sockfd_t xfdt_sockfd = tcp_connect(xszt_host, xwt_port, 5000, X_NULL);
    if (X_INVALID_SOCKFD == xfdt_sockfd)
    {
        printf("tcp_connect(...) return X_INVALID_SOCKFD\n");
        return -1;
    }

    x_io_msgctxt_t xio_msgctxt_type;
    xio_msgctxt_type.io_cmid = CMID_ECHO_LOGIN;
    xio_msgctxt_type.io_seqn = xut_ioseqn++;
    xio_msgctxt_type.io_size = (x_uint32_t )strlen(xszt_name);
    xio_msgctxt_type.io_dptr = (x_uchar_t *)xszt_name;

    // 投递 登录请求 信息
    if (-1 == send_io_msgctxt(xfdt_sockfd, xio_msgctxt_type))
    {
        printf("send_io_msgctxt(xfdt_sockfd, xio_msgctxt_type) error!\n");
        sockfd_close(xfdt_sockfd);
        return -1;
    }

    // 接收 登录应答 信息
    if (-1 == recv_io_msgctxt(xfdt_sockfd, xio_msgctxt_type))
    {
        printf("recv_io_msgctxt(xfdt_sockfd, xio_msgctxt_type) error!\n");
        sockfd_close(xfdt_sockfd);
        return -1;
    }

    // 验证是否成功
    if (CMID_ECHO_LOGIN != xio_msgctxt_type.io_cmid)
    {
        printf("(CMID_ECHO_TYPE[%d] != xio_msgctxt_type.io_cmid[%d])\n",
               CMID_ECHO_LOGIN, xio_msgctxt_type.io_cmid);
        sockfd_close(xfdt_sockfd);
        return -1;
    }

    printf("ECHO connect OK!\n");

    //======================================
    // 执行 echo 流程

    using x_clock_t  = std::chrono::system_clock;
    using x_tmtick_t = std::chrono::microseconds;

    x_int64_t xit_tmcount = 0;
    x_int64_t xit_rscount = 0;

    x_io_msgctxt_t xio_msgctxt_text;
    x_char_t xct_buffer[TEXT_LEN_4K] = { 0 };

    // 设置 ECHO 的文本信息
    memcpy(xct_buffer + sizeof(x_int64_t), xszt_text, xit_mlen);

    while (true)
    {
        xio_msgctxt_text.io_cmid = CMID_ECHO_TEXT;
        xio_msgctxt_text.io_seqn = xut_ioseqn++;
        xio_msgctxt_text.io_size = sizeof(x_int64_t) + xit_mlen;
        xio_msgctxt_text.io_dptr = (x_uchar_t *)xct_buffer;

        // 当前时间戳
        *(x_int64_t *)xct_buffer = x_clock_t::now().time_since_epoch().count();

        // 投递 echo请求 信息
        if (-1 == send_io_msgctxt(xfdt_sockfd, xio_msgctxt_text))
        {
            printf("send_io_msgctxt(xfdt_sockfd, xio_msgctxt_text) error!\n");
            break;
        }

        // 接收 echo应答 信息
        if (-1 == recv_io_msgctxt(xfdt_sockfd, xio_msgctxt_text))
        {
            printf("recv_io_msgctxt(xfdt_sockfd, xio_msgctxt_text) error!\n");
            break;
        }

        if (CMID_ECHO_TEXT != xio_msgctxt_text.io_cmid)
        {
            printf("CMID_ECHO_TEXT[%d] != xio_msgctxt_text.io_cmid[%d]\n",
                   CMID_ECHO_TEXT, xio_msgctxt_text.io_cmid);
            break;
        }

        // 延时时间差
        x_tmtick_t time_delay =
            std::chrono::duration_cast< x_tmtick_t >(
                x_clock_t::now() -
                x_clock_t::time_point(
                    x_clock_t::duration(*(x_int64_t *)xio_msgctxt_text.io_dptr)));

        xit_tmcount += time_delay.count();
        xit_rscount += 1;

        // 输出 echo应答 信息
        std::string xstr((x_char_t *)xio_msgctxt_text.io_dptr + sizeof(x_int64_t), xio_msgctxt_text.io_size - sizeof(x_int64_t));
        printf("[delay time: %4d us, average: %lf us] ECHO => %s\n",
               (x_int32_t)time_delay.count(),
               (x_lfloat_t)xit_tmcount / (x_lfloat_t)xit_rscount,
               xstr.c_str());

        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    //======================================

    sockfd_close(xfdt_sockfd);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////

/**********************************************************/
/**
 * @brief 发送 IO 消息。
 */
x_int32_t send_io_msgctxt(x_sockfd_t xfdt_sockfd, x_io_msgctxt_t & xio_msgctxt)
{
    //======================================
    // 设置发送缓存信息

    x_uchar_t  xct_dptr[TEXT_LEN_4K];
    x_uint32_t xut_size = IO_HDSIZE + xio_msgctxt.io_size;

    io_set_context(xct_dptr, TEXT_LEN_4K, &xio_msgctxt);

    //======================================
    // 发送数据

    x_int32_t xit_nbytes = send(xfdt_sockfd, (x_char_t *)xct_dptr, (x_int32_t)xut_size, 0);
    if (-1 == xit_nbytes)
    {
        printf("send(xfdt_sockfd[%d], ..., (x_int32_t)xut_size[%d], 0) return -1\n",
               xfdt_sockfd, xut_size);
        return -1;
    }

    //======================================

    return xit_nbytes;
}

/**********************************************************/
/**
 * @brief 接收 IO 消息。
 */
x_int32_t recv_io_msgctxt(x_sockfd_t xfdt_sockfd, x_io_msgctxt_t & xio_msgctxt)
{
    //======================================
    // 接收数据

    static x_uchar_t xct_dptr[TEXT_LEN_4K];

    memset(&xio_msgctxt, 0, sizeof(x_io_msgctxt_t));

    x_int32_t xit_nbytes = recv(xfdt_sockfd, (x_char_t *)xct_dptr, TEXT_LEN_4K, 0);
    if (-1 == xit_nbytes)
    {
        printf("recv(xfdt_sockfd[%d], ...) return -1\n", xfdt_sockfd);
        return -1;
    }

    //======================================
    // 提取 IO 消息

    if (0 != io_get_context(xct_dptr, xit_nbytes, &xio_msgctxt))
    {
        return -1;
    }

    //======================================

    return xit_nbytes;
}

/**********************************************************/
/**
 * @brief 返回套接字当前操作失败的错误码。
 */
x_int32_t sockfd_lasterror(void)
{
#ifdef _MSC_VER
	return (x_int32_t)WSAGetLastError();
#else // !_MSC_VER
	return errno;
#endif // _MSC_VER
}

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
	x_int32_t   xit_ecode   = 0;
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
		skaddr_host.sin_port   = htons(xwt_port);
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
		tmval.tv_sec  = (xut_tmout / 1000);
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
