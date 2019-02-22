/**
 * @file    xftp_client.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xftp_client.h
 * 创建日期：2019年02月18日
 * 文件标识：
 * 文件摘要：xftp 的客户端连接类。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年02月18日
 * 版本摘要：
 * 
 * 取代版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#ifndef __XFTP_CLIENT_H__
#define __XFTP_CLIENT_H__

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <list>

#include "xmempool.h"
#include "xftp_msgctxt.h"

////////////////////////////////////////////////////////////////////////////////
// x_ftp_client_t

/**
 * @class x_ftp_client_t
 * @brief xftp 的客户端连接类。
 */
class x_ftp_client_t
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

    /**
     * @enum  emIoContextCmid
     * @brief IO 操作的命令 ID 表。
     */
    typedef enum emIoContextCmid
    {
        CMID_WCLI_LOGIN  = 0x0020,  ///< 网络连接类型（建立网络连接）
        CMID_WCLI_HBEAT  = 0x2000,  ///< 心跳
        CMID_WCLI_FLIST  = 0x3010,  ///< 获取文件列表
    } emIoContextCmid;

    using x_seqno_t       = std::atomic< x_uint16_t >;
    using x_locker_t      = std::mutex;
    using x_sync_notify_t = std::condition_variable;
    using x_thread_t      = std::thread;
    using x_iomsg_queue_t = std::list< x_io_msghead_t * >;

    /** 文件列表[ 文件名，文件大小 ] */
    using x_list_file_t   = std::list< std::pair< std::string, x_int64_t > >;

    /**********************************************************/
    /**
     * @brief 共用的内存池对象。
     */
    static inline x_mempool_t & mempool(void) { return _S_mempool; }

private:
    static x_mempool_t _S_mempool;  ///< 共用的内存池对象

    // constructor/destructor
public:
    x_ftp_client_t(void);
    virtual ~x_ftp_client_t(void);

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
     * @brief 是否已经登录。
     */
    inline x_bool_t is_login(void) const { return (is_startup() && m_xbt_login); }

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
    inline x_bool_t is_enable_auto_heartbeat(x_void_t) const { return m_xbt_heart_enable; }

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

    /**********************************************************/
    /**
     * @brief 从缓存中读取文件列表。
     */
    x_void_t get_cache_flist(x_list_file_t & xlst_files);

public:

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
     * @brief 投递心跳数据包信息。
     */
    x_void_t send_heartbeat(void);

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

    /**********************************************************/
    /**
     * @brief 分派应答 IO 消息，进行对应的处理操作。
     */
    x_void_t dispatch_io_msgctxt(const x_io_msgctxt_t & xio_msgctxt);

    /**********************************************************/
    /**
     * @brief 处理 IO 应答命令：建立网络连接。
     */
    x_int32_t iocmd_login(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size);

    /**********************************************************/
    /**
     * @brief 处理 IO 应答命令：心跳。
     */
    x_int32_t iocmd_hbeat(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size);

    /**********************************************************/
    /**
     * @brief 处理 IO 应答命令：获取文件列表。
     */
    x_int32_t iocmd_flist(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size);

    // data members
protected:
    x_sockfd_t       m_xfdt_sockfd;      ///< 套接字描述符
    x_bool_t         m_xbt_login;        ///< 标识登录成功（网络连接建立成功）
    x_bool_t         m_xbt_running;      ///< 工作线程的运行标识
    x_thread_t       m_xthd_send;        ///< 数据发送的工作线程
    x_thread_t       m_xthd_recv;        ///< 数据接收的工作线程

    x_bool_t         m_xbt_heart_enable; ///< 标识是否允许提交心跳包
    x_uint32_t       m_xut_heart_time;   ///< 心跳间隔时间

    x_seqno_t        m_xut_seqno;        ///< IO 消息的流水号生成器
    x_locker_t       m_xlock_mqreq;      ///< 请求操作的 IO 消息队列的同步操作锁
    x_sync_notify_t  m_xnotify_mqreq;    ///< 请求操作的 IO 消息队列的同步通知变量（条件变量）
    x_iomsg_queue_t  m_xmqueue_req;      ///< 请求操作的 IO 消息队列

    x_locker_t       m_xlock_flist;       ///< m_lst_flist_cache 的同步访问锁
    x_list_file_t    m_lst_flist_cache;  ///< 缓存的文件列表
};

////////////////////////////////////////////////////////////////////////////////

#define XMKEY_WCLI_FLIST   ((x_uint32_t)(x_ftp_client_t::CMID_WCLI_LOGIN << 16) | (x_uint32_t)x_ftp_client_t::CMID_WCLI_FLIST)

////////////////////////////////////////////////////////////////////////////////

#endif // __XFTP_CLIENT_H__
