/**
 * @file    xftp_cliworker.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xftp_cliworker.h
 * 创建日期：2019年02月22日
 * 文件标识：
 * 文件摘要：XFTP 的客户端网络连接工作类（定义的接口类）。
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

#ifndef __XFTP_CLIWORKER_H__
#define __XFTP_CLIWORKER_H__

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <list>

#include "xmempool.h"
#include "xftp_msgctxt.h"

////////////////////////////////////////////////////////////////////////////////
// x_ftp_cliworker_t

/**
 * @class x_ftp_cliworker_t
 * @brief XFTP 的客户端网络连接工作类（定义的接口类）。
 */
class x_ftp_cliworker_t
{
    // common data types
public:
    /**
     * @enum  emConstValue
     * @brief 枚举常量值。
     */
    typedef enum emConstValue
    {
        ECV_CONNECT_TIMEOUT = 3000,        ///< 默认连接操作的超时时间
        ECV_MAXWAIT_TIMEOUT = 3000,        ///< 最大等待超时时间
        ECV_HEARTBEAT_TIME  = 5000,        ///< 默认使用的心跳间隔时间
    } emConstValue;

    using x_seqno_t       = std::atomic< x_uint16_t >;
    using x_locker_t      = std::mutex;
    using x_sync_notify_t = std::condition_variable;
    using x_thread_t      = std::thread;
    using x_iomsg_queue_t = std::list< x_io_msghead_t * >;

    /**********************************************************/
    /**
     * @brief 共用的内存池对象。
     */
    static inline x_mempool_t & mempool(void) { return _S_mempool; }

private:
    static x_mempool_t _S_mempool;  ///< 共用的内存池对象

    // constructor/destructor
public:
    x_ftp_cliworker_t(void);
    virtual ~x_ftp_cliworker_t(void);

    // extensible interfaces
public:
    /**********************************************************/
    /**
     * @brief 连接类型。
     */
    virtual x_uint16_t ctype(void) const = 0;

protected:
    /**********************************************************/
    /**
     * @brief 投递心跳数据包信息。
     */
    virtual x_void_t send_heartbeat(void);

    /**********************************************************/
    /**
     * @brief 收到 IO 消息的通知接口。
     */
    virtual x_void_t io_recved_msgctxt(const x_io_msgctxt_t & xio_msgctxt);

    /**********************************************************/
    /**
     * @brief 完成 IO 消息发送的通知接口。
     */
    virtual x_void_t io_sended_msgctxt(const x_io_msgctxt_t & xio_msgctxt);

    /**********************************************************/
    /**
     * @brief IO 操作产生错误的通知接口。
     */
    virtual x_void_t io_error_notify(x_int32_t xit_error, x_bool_t xbt_iosend);

    // public interfaces
public:
    /**********************************************************/
    /**
     * @brief 启动工作对象。
     *
     * @param [in ] xszt_host : 目标主机（服务器）的 IP 地址（四段式 IP 地址字符串）。
     * @param [in ] xwt_port  : 目标主机（服务器）的 端口号。
     *
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t startup(x_cstring_t xszt_host, x_uint16_t xwt_port);

    /**********************************************************/
    /**
     * @brief 关闭工作对象。
     */
    x_void_t shutdown(void);

    /**********************************************************/
    /**
     * @brief 是否已经启动。
     */
    inline x_bool_t is_startup(void) const { return m_xbt_running; }

    /**********************************************************/
    /**
     * @brief 获取连接的心跳间隔时间（毫秒级 时间）。
     */
    inline x_uint32_t get_heartbeat_time(x_void_t) const { return m_xut_heart_time; }

    /**********************************************************/
    /**
     * @brief 设置连接的心跳间隔时间（毫秒级 时间）。
     */
    inline x_void_t set_heartbeat_time(x_uint32_t xut_hearttime) { m_xut_heart_time = xut_hearttime; }

    /**********************************************************/
    /**
     * @brief 是否允许自动提交心跳包。
     */
    inline x_bool_t is_enable_auto_heartbeat(x_void_t) const
    {
        return X_FALSE;
        //return m_xbt_heart_enable;
    }

    /**********************************************************/
    /**
     * @brief 启用/禁用 自动提交心跳包。
     */
    inline x_void_t enable_auto_heartbeat(x_bool_t xbt_enable) { m_xbt_heart_enable = xbt_enable; }

    /**********************************************************/
    /**
     * @brief 投递网络请求 IO 消息数据包。
     */
    x_int32_t post_req_xmsg(x_io_msgctxt_t & xio_msgctxt, x_bool_t xbt_seqn);

    // internal invoking
protected:
    /**********************************************************/
    /**
     * @brief 数据发送操作的工作线程的入口函数。
     */
    x_void_t thread_send_run(void);

    /**********************************************************/
    /**
     * @brief 数据接收操作的工作线程的入口函数。
     */
    x_void_t thread_recv_run(void);

    /**********************************************************/
    /**
     * @brief 接收到网络的数据 IO 流。
     *
     * @param [in    ] xct_io_dptr : 网络数据流缓存。
     * @param [in,out] xut_io_dlen : 数据流缓存长度（入参，表示有效的数据流字节数；回参，表示处理到的字节数）。
     *
     * @return x_int32_t
     *         - 返回 -1，表示该连接对象的套接字失效，将进行关闭操作；
     *         - 返回  0，表示该连接对象的套接字有效，将继续保活操作。
     */
    x_int32_t io_recved(x_uchar_t * const xct_io_dptr, x_uint32_t & xut_io_dlen);

    // data members
protected:
    x_sockfd_t       m_xfdt_sockfd;      ///< 套接字描述符
    x_bool_t         m_xbt_running;      ///< 工作线程的运行标识
    x_thread_t       m_xthd_send;        ///< 数据发送的工作线程
    x_thread_t       m_xthd_recv;        ///< 数据接收的工作线程

    x_bool_t         m_xbt_heart_enable; ///< 标识是否允许提交心跳包
    x_uint32_t       m_xut_heart_time;   ///< 心跳间隔时间

    x_seqno_t        m_xut_seqno;        ///< IO 消息的流水号生成器
    x_locker_t       m_xlock_mqreq;      ///< 请求操作的 IO 消息队列的同步操作锁
    x_sync_notify_t  m_xnotify_mqreq;    ///< 请求操作的 IO 消息队列的同步通知变量（条件变量）
    x_iomsg_queue_t  m_xmqueue_req;      ///< 请求操作的 IO 消息队列
};

////////////////////////////////////////////////////////////////////////////////

#endif // __XFTP_CLIWORKER_H__
