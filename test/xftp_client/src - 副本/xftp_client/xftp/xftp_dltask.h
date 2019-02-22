/**
 * @file    xftp_dltask.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xftp_dltask.h
 * 创建日期：2019年02月21日
 * 文件标识：
 * 文件摘要：文件下载的任务对象工作类。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年02月21日
 * 版本摘要：
 * 
 * 取代版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#ifndef __XFTP_DLTASK_H__
#define __XFTP_DLTASK_H__

#include <mutex>
#include <atomic>
#include <list>

#include "xmempool.h"
#include "xthreadpool.h"
#include "xftp_msgctxt.h"

////////////////////////////////////////////////////////////////////////////////
// x_ftp_dltask_t

/**
 * @class x_ftp_dltask_t
 * @brief 文件下载的任务对象工作类。
 */
class x_ftp_dltask_t : public x_task_t
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
        ECV_FILE_CHUNK_SIZE = 32 * 1024,   ///< 文件块的大小
    } emConstValue;

    /**
     * @enum  emIoContextCmid
     * @brief IO 操作的命令 ID 表。
     */
    typedef enum emIoContextCmid
    {
        CMID_DLOAD_LOGIN = 0x0030,  ///< 登录
        CMID_DLOAD_HBEAT = 0x2000,  ///< 心跳
        CMID_DLOAD_CHUNK = 0x2010,  ///< 下载文件块
        CMID_DLOAD_PAUSE = 0x2020,  ///< 暂停下载
    } emIoContextCmid;

    using x_seqno_t       = std::atomic< x_uint16_t >;
    using x_lock_t        = std::mutex;
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
    x_ftp_dltask_t(void);
    virtual ~x_ftp_dltask_t(void);

    // overrides
public:
    /**********************************************************/
    /**
     * @brief 任务对象执行流程的抽象接口。
     */
    virtual void run(x_running_checker_t * xchecker_ptr);

    /**********************************************************/
    /**
     * @brief 判断 任务对象 是否挂起。
     */
    virtual bool is_suspend(void) const;

    /**********************************************************/
    /**
     * @brief 设置任务对象的运行标识。
     */
    virtual void set_running_flag(bool xrunning_flag);

    /**********************************************************/
    /**
     * @brief 获取任务对象的删除器。
     */
    virtual const x_task_deleter_t * get_deleter(void) const;

    // public interfaces
public:


    // data members
private:
    std::string  m_xstr_host;   ///< XFTP 服务器主机 IP
    x_uint16_t   m_xut_port;    ///< XFTP 服务器主机 端口号
    std::string  m_xstr_fpath;  ///< 待下载的文件本地存储路径
    x_int64_t    m_xit_fsize;   ///< 文件总大小

    x_sockfd_t   m_xfdt_sockfd; ///< 网络连接套接字
};

////////////////////////////////////////////////////////////////////////////////

#endif // __XFTP_DLTASK_H__
