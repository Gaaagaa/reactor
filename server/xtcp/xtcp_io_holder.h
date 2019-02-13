/**
 * @file    xtcp_io_holder.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xtcp_io_holder.h
 * 创建日期：2019年02月10日
 * 文件标识：
 * 文件摘要：
 *   1. 定义 x_tcp_io_task_t 线程池的工作任务类；
 *      协助 x_tcp_io_manager_t 进行 IO 事件的执行操作。
 *   2. 定义 x_tcp_io_holder_t 辅助工作类；
 *      其与 套接字描述符 结成映射对，存储于 x_tcp_io_manager_t 中，
 *      协助 x_tcp_io_manager_t 进行 IO 事件的投递操作。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年02月10日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#ifndef __XTCP_IO_HOLDER_H__
#define __XTCP_IO_HOLDER_H__

#include "xthreadpool.h"
#include "xobjectpool.h"
#include "xtcp_io_channel.h"

////////////////////////////////////////////////////////////////////////////////
// x_tcp_io_task_t

/**
 * @class x_tcp_io_task_t
 * @brief 执行业务层的 IO 事件操作的任务对象（驱动 x_tcp_io_channel_t 进行工作）。
 */
class x_tcp_io_task_t : public x_threadpool_t::x_task_t
{
    // common data types
public:
    using x_io_csptr_t = std::shared_ptr< x_tcp_io_channel_t >;
    using x_io_cwptr_t = std::weak_ptr< x_tcp_io_channel_t >;

    /**
     * @class x_task_pool_t
     * @brief 管理 x_tcp_io_task_t 的对象池类。
     */
    class x_taskpool_t : public x_objectpool_t< x_tcp_io_task_t >
                       , protected x_task_deleter_t
    {
        // common data types
    protected:
        using x_deleter_t  = x_task_deleter_t;

        // overrides : x_deleter_t
    protected:
        /**********************************************************/
        /**
         * @brief 对 任务对象 进行资源回收操作（删除操作）。
         */
        virtual void delete_task(x_task_ptr_t xtask_ptr) override
        {
            if (nullptr != xtask_ptr)
            {
                recyc((x_taskpool_t::x_object_ptr_t)xtask_ptr);
            }
        }
    };

    /**
     * @enum  emIoTaskEventType
     * @brief 任务对象的 IO 操作类型枚举值。
     */
    typedef enum emIoTaskEventType
    {
        EIO_TASK_CREATED  = 0x0010,  ///< IO 通道对象的 初建事件
        EIO_TASK_DESTROY  = 0x0020,  ///< IO 通道对象的 销毁事件
        EIO_TASK_READING  = 0x0030,  ///< IO 读事件
        EIO_TASK_WRITING  = 0x0040,  ///< IO 写事件
        EIO_TASK_MSGPUMP  = 0x0050,  ///< IO 消息投递事件
    } emIoTaskEventType;

    // common invoking
public:
    /**********************************************************/
    /**
     * @brief 共用的 任务对象池。
     */
    static inline x_taskpool_t & taskpool(void)
    {
        return _S_comm_taskpool;
    }

    // common data
private:
    static x_taskpool_t _S_comm_taskpool;   ///< 共用的 任务对象池

    // constructor/destructor
public:
    /**********************************************************/
    /**
     * @brief 任务对象的默认构造函数。
     * 
     * @param [in ] xio_cwptr : 目标操作的 x_tcp_io_channel_t 对象。
     * @param [in ] xut_event : 任务对象所要处理事件（参看 emIoTaskEventType 枚举值）。
     * 
     */
    explicit x_tcp_io_task_t(const x_io_cwptr_t & xio_cwptr, x_uint32_t xut_event);
    virtual ~x_tcp_io_task_t(void);

    // overrides
protected:
    /**********************************************************/
    /**
     * @brief 任务对象执行流程的抽象接口。
     */
    virtual void run(x_running_checker_t * xchecker_ptr) override;

    /**********************************************************/
    /**
     * @brief 判断 任务对象 是否挂起。
     */
    virtual bool is_suspend(void) const override;

    /**********************************************************/
    /**
     * @brief 设置任务对象的运行标识。
     */
    virtual void set_running_flag(bool xrunning_flag) override;

    /**********************************************************/
    /**
     * @brief 获取任务对象的删除器。
     */
    virtual const x_task_deleter_t * get_deleter(void) const override;

    // internal invoking
protected:
    /**********************************************************/
    /**
     * @brief 执行写操作的工作流程。
     * 
     * @param [in ] xio_csptr : 目标操作的 IO 通道对象。
     * @param [in ] xit_wmsgs : 可操作的 IO 消息最大数量。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t workflow_write(x_io_csptr_t & xio_csptr, x_int16_t xit_wmsgs);

    /**********************************************************/
    /**
     * @brief 初建 IO 通道对象事件 的处理流程。
     */
    x_int32_t handle_created(void);

    /**********************************************************/
    /**
     * @brief 销毁 IO 通道对象事件 的处理流程。
     */
    x_int32_t handle_destroy(void);

    /**********************************************************/
    /**
     * @brief IO 读事件 的处理流程。
     */
    x_int32_t handle_reading(void);

    /**********************************************************/
    /**
     * @brief IO 写事件 的处理流程。
     */
    x_int32_t handle_writing(void);

    /**********************************************************/
    /**
     * @brief IO 消息投递事件 的处理流程。
     */
    x_int32_t handle_msgpump(void);

    // data members
private:
    x_io_cwptr_t  m_xio_cwptr;  ///< 目标操作的 x_tcp_io_channel_t 对象
    x_io_csptr_t  m_xio_csptr;  ///< 目标操作的 x_tcp_io_channel_t 对象（m_xut_event == EIO_TASK_DESTROY 时有效）
    x_uint32_t    m_xut_event;  ///< 任务对象所要处理事件（参看 emIoTaskEventType 枚举值）

#ifdef _DEBUG
    x_int64_t     m_xit_timev;  ///< 任务提交时的时间戳
#endif // _DEBUG
};

////////////////////////////////////////////////////////////////////////////////
// x_tcp_io_holder_t

/**
 * @class x_tcp_io_holder_t
 * @brief 持有 x_tcp_io_channel_t 对象的辅助类，协助 x_tcp_io_manager_t 进行事件投递操作。
 */
class x_tcp_io_holder_t final
{
    // common data types
public:
    /**
     * @enum  emConstValue
     * @brief 相关的枚举常量值。
     */
    typedef enum emConstValue
    {
        ECV_NIO_MAX_LEN   = 64 * 1024,   ///< IO 消息执行 读/写 操作时，限制最大的数据长度
    } emConstValue;

private:
    using x_io_csptr_t = std::shared_ptr< x_tcp_io_channel_t >;
    using x_io_cwptr_t = std::weak_ptr< x_tcp_io_channel_t >;
    using x_io_task_t  = x_tcp_io_task_t;

    // constructor/destructor
public:
    explicit x_tcp_io_holder_t(void);
    ~x_tcp_io_holder_t(void);

    // public interfaces
public:
    /**********************************************************/
    /**
     * @brief 处理 读操作 事件。
     * 
     * @param [in ] xht_manager : IO 管理对象（x_tcp_io_manager_t）。
     * @param [in ] xfdt_sockfd : 目标操作的套接字描述符。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t io_reading(x_handle_t xht_manager, x_sockfd_t xfdt_sockfd);

    /**********************************************************/
    /**
     * @brief 处理 写操作 事件。
     * 
     * @param [in ] xht_manager : IO 管理对象（x_tcp_io_manager_t）。
     * @param [in ] xfdt_sockfd : 目标操作的套接字描述符。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t io_writing(x_handle_t xht_manager, x_sockfd_t xfdt_sockfd);

    /**********************************************************/
    /**
     * @brief 处理 巡检 事件。
     * 
     * @param [in ] xht_manager : IO 管理对象（x_tcp_io_manager_t）。
     * @param [in ] xfdt_sockfd : 目标操作的套接字描述符。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t io_verify(x_handle_t xht_manager, x_sockfd_t xfdt_sockfd);

    // internal invoking
private:
    /**********************************************************/
    /**
     * @brief 尝试接收首个 IO 消息，创建业务层中对应的 x_tcp_io_channel_t 对象。
     * @note  该接口只为 io_reading() 调用。
     * 
     * @param [in ] xht_manager : IO 管理对象（x_tcp_io_manager_t）。
     * @param [in ] xfdt_sockfd : 目标操作的套接字描述符。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t try_create_io_handle(x_handle_t xht_manager, x_sockfd_t xfdt_sockfd);

    // data members
private:
    x_io_csptr_t        m_xio_csptr;   ///< 持有的 IO 通道对象
    x_tcp_io_message_t  m_xmsg_swap;   ///< 用于临时数据交换操作的网络 IO 消息对象
};

////////////////////////////////////////////////////////////////////////////////

#endif // __XTCP_IO_HOLDER_H__
