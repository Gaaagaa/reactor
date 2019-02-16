/**
 * @file    xtcp_io_channel.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xtcp_io_channel.h
 * 创建日期：2019年01月17日
 * 文件标识：
 * 文件摘要：定义网络 IO 通道对象（业务层工作对象）的抽象接口类。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年01月17日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#ifndef __XTCP_IO_CHANNEL_H__
#define __XTCP_IO_CHANNEL_H__

#include "xspinlock.h"
#include "xspsc_queue.h"
#include "xtcp_io_message.h"

////////////////////////////////////////////////////////////////////////////////

class x_tcp_io_task_t;
class x_tcp_io_creator_t;
class x_tcp_io_holder_t;

////////////////////////////////////////////////////////////////////////////////
// x_tcp_io_channel_t

/**
 * @class x_tcp_io_channel_t
 * @brief 网络 IO 通道对象（业务层工作对象）的抽象接口类。
 */
class x_tcp_io_channel_t
{
    friend x_tcp_io_task_t;
    friend x_tcp_io_creator_t;
    friend x_tcp_io_holder_t;

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

protected:
    /**
     * @enum  emIoHandleStatus
     * @brief 对象工作状态的相关掩码位枚举值。
     */
    typedef enum emIoHandleStatus
    {
        EIO_STATUS_READABLE  = 0x00000001,  ///< 可读
        EIO_STATUS_WRITABLE  = 0x00000002,  ///< 可写
        EIO_STATUS_OCCUPIED  = 0x00000004,  ///< 被（线程池的）工作线程持有（占用）
        EIO_STATUS_WDESTROY  = 0x80000000,  ///< 等待销毁
    } emIoHandleStatus;

    using x_iomsg_t = x_tcp_io_message_t;

    /**
     * @class x_msg_queue_t
     * @brief IO 消息队列。
     */
    class x_msg_queue_t : public x_spsc_queue_t< x_iomsg_t, 64 >
                        , public x_spinlock_t
    {
    };

    using x_mqautolock_t = x_autospin_t< x_msg_queue_t >;

    // constructor/destructor
public:
    explicit x_tcp_io_channel_t(x_handle_t xht_manager, x_sockfd_t xfdt_sockfd);
    virtual ~x_tcp_io_channel_t(void);

    // extensible interfaces : for x_tcp_io_task_t
protected:
    /**********************************************************/
    /**
     * @brief 投递请求操作的 IO 消息。
     * @note  该接口服务于 x_tcp_io_task_t 对象，可重载该接口，实现消息分包等功能。
     * 
     * @param [in,out] xio_message : 投递的 IO 消息。
     * 
     * @return x_int32_t
     *         - 返回值 >=  0，表示操作产生的事件数量，
     *           若重载该接口，进行消息分包操作，则产生的事件数量可能会大于 1。
     *         - 返回值 == -1，表示产生错误，后续则可关闭该业务层工作对象。
     */
    virtual x_int32_t post_req_xmsg(x_tcp_io_message_t & xio_message);

    /**********************************************************/
    /**
     * @brief 拉取应答操作的 IO 消息。
     * @note  该接口服务于 x_tcp_io_task_t 对象，取到的 IO 消息，用于写操作，即应答操作。
     * 
     * @param [out] xio_message : 操作成功返回的 IO 消息。
     * 
     * @return x_int32_t
     *         - 返回值 ==  1，表示操作成功（提取到一个应答的 IO 消息）；
     *         - 返回值 ==  0，表示操作失败（应答消息队列为空）；
     *         - 返回值 == -1，表示操作产生错误。
     */
    virtual x_int32_t pull_res_xmsg(x_tcp_io_message_t & xio_message);

    // extensible interfaces : for the subclass of business layer
protected:
    /**********************************************************/
    /**
     * @brief 运行时的巡检操作接口（可重载该接口，实时判断对象的有效性）。
     * 
     * @return x_int32_t
     *         - 返回 0，表示对象持续有效；
     *         - 返回 其他值（错误码），表示对象失效（之后对象将会转入等待销毁的状态）。
     */
    virtual x_int32_t io_event_runtime_verify(void);

    /**********************************************************/
    /**
     * @brief 处理 “接收 IO 请求消息” 的事件（重载该接口，实现具体业务功能）。
     */
    virtual x_int32_t io_event_requested(x_tcp_io_message_t & xio_message);

    /**********************************************************/
    /**
     * @brief 处理 “完成 IO 应答消息” 的事件（可重载该接口，实现具体的完成通知工作）。
     */
    virtual x_int32_t io_event_responsed(x_tcp_io_message_t & xio_message);

    /**********************************************************/
    /**
     * @brief 处理 “IO 通道对象被销毁” 的事件（可重载该接口，处理相关资源释放/清理工作）。
     */
    virtual x_int32_t io_event_destroyed(void);

    // public interfaces
public:
    /**********************************************************/
    /**
     * @brief 返回 所隶属的 x_tcp_io_manager_t 对象句柄。
     */
    inline x_handle_t get_manager(void) const { return m_xht_manager; }

    /**********************************************************/
    /**
     * @brief 返回 持有的套接字描述符。
     */
    inline x_sockfd_t get_sockfd(void) const { return m_xfdt_sockfd; }

    /**********************************************************/
    /**
     * @brief 是否可读。
     */
    inline x_bool_t is_readable(void) const
    {
        return (0 != (m_xut_status & EIO_STATUS_READABLE));
    }

    /**********************************************************/
    /**
     * @brief 是否可写。
     */
    inline x_bool_t is_writable(void) const
    {
        return (0 != (m_xut_status & EIO_STATUS_WRITABLE));
    }

    /**********************************************************/
    /**
     * @brief 是否被（线程池的）工作线程持有（占用）。
     */
    inline x_bool_t is_occupied(void) const
    {
        return (0 != (m_xut_status & EIO_STATUS_OCCUPIED));
    }

    /**********************************************************/
    /**
     * @brief 是否等待销毁。
     */
    inline x_bool_t is_wdestroy(void) const
    {
        return (0 != (m_xut_status & EIO_STATUS_WDESTROY));
    }

    /**********************************************************/
    /**
     * @brief 请求操作的 IO 消息队列中的消息对象数量。
     */
    inline x_size_t req_queue_size(void) const { return m_xmqueue_req.size(); }

    /**********************************************************/
    /**
     * @brief 应答操作的 IO 消息队列中的消息对象数量。
     */
    inline x_size_t res_queue_size(void) const { return m_xmqueue_res.size(); }

    // internal invoking
protected:
    /**********************************************************/
    /**
     * @brief 将 IO 消息加入到应答队列。
     */
    x_void_t push_res_xmsg(x_iomsg_t && xio_message);

    /**********************************************************/
    /**
     * @brief 将 IO 消息加入到应答队列。
     */
    x_void_t push_res_xmsg(const x_iomsg_t & xio_message);

    /**********************************************************/
    /**
     * @brief 数据清理。
     */
    x_void_t cleanup(void);

    // for x_tcp_io_task_t invoking
private:
    /**********************************************************/
    /**
     * @brief 设置可读状态。
     */
    inline x_void_t set_readable(x_bool_t xbt_readable)
    {
        xbt_readable ? (m_xut_status |=  EIO_STATUS_READABLE) :
                       (m_xut_status &= ~EIO_STATUS_READABLE);
    }

    /**********************************************************/
    /**
     * @brief 设置可写状态。
     */
    inline x_void_t set_writable(x_bool_t xbt_writable)
    {
        xbt_writable ? (m_xut_status |=  EIO_STATUS_WRITABLE) :
                       (m_xut_status &= ~EIO_STATUS_WRITABLE);
    }

    /**********************************************************/
    /**
     * @brief 设置是否被（线程池的）工作线程持有（占用）状态。
     */
    inline x_void_t set_occupied(x_bool_t xbt_occupied)
    {
        xbt_occupied ? (m_xut_status |=  EIO_STATUS_OCCUPIED) :
                       (m_xut_status &= ~EIO_STATUS_OCCUPIED);
    }

    /**********************************************************/
    /**
     * @brief 设置等待销毁状态。
     */
    inline x_void_t set_wdestroy(x_bool_t xbt_wdestroy)
    {
        xbt_wdestroy ? (m_xut_status |=  EIO_STATUS_WDESTROY) :
                       (m_xut_status &= ~EIO_STATUS_WDESTROY);
    }

    /**********************************************************/
    /**
     * @brief 将首个 IO 请求消息转移到 请求队列中。
     * @note  该接口只在创建完对应业务层工作对象后被调用。
     * @see   x_tcp_io_creator_t::try_create_io_channel()
     */
    x_int32_t req_xmsg_first_dump(x_iomsg_t & xio_message);

    /**********************************************************/
    /**
     * @brief 执行 IO 请求消息的读取流程。
     * 
     * @param [out] xit_rmsgs : 操作返回所读取到的 IO 消息对象数量。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t req_xmsg_reading(x_int32_t & xit_rmsgs);

    /**********************************************************/
    /**
     * @brief 为 x_tcp_io_task_t 对象提供的 IO 请求消息的投递（消息泵）操作。
     * 
     * @param [in, out] xit_rmsgs : 入参，投递 IO 请求消息操作的最大数量；回参，实际投递处理的数量。
     * 
     * @return x_int32_t
     *         - 返回 0，表示操作过程中未产生错误；
     *         - 返回 其他值，表示操作过程中产生的错误码。
     */
    x_int32_t req_xmsg_pump(x_int32_t & xit_rmsgs);

    /**********************************************************/
    /**
     * @brief 执行 IO 应答消息的写入流程。
     * 
     * @param [in, out] xit_wmsgs : 入参，写处理 IO 应答消息操作的最大数量；回参，实际写处理的数量。
     * 
     * @return x_int32_t
     *         - 返回 0，表示操作过程中未产生错误；
     *         - 返回 其他值，表示操作过程中产生的错误码。
     */
    x_int32_t res_xmsg_writing(x_int32_t & xit_wmsgs);

    // data members
private:
    x_handle_t      m_xht_manager;   ///< 所隶属的 x_tcp_io_manager_t 对象句柄
    x_sockfd_t      m_xfdt_sockfd;   ///< 持有的套接字描述符
    x_uint32_t      m_xut_status;    ///< 状态标识

    x_iomsg_t       m_xmsg_reading;  ///< 为 x_tcp_io_task_t 提供读操作的 IO 消息缓存对象
    x_iomsg_t       m_xmsg_writing;  ///< 为 x_tcp_io_task_t 提供写操作的 IO 消息缓存对象

protected:
    x_msg_queue_t   m_xmqueue_req;   ///< 请求操作的 IO 消息队列
    x_msg_queue_t   m_xmqueue_res;   ///< 应答操作的 IO 消息队列
};

////////////////////////////////////////////////////////////////////////////////

#endif // __XTCP_IO_CHANNEL_H__
