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

#include "xtcp_io_channel.h"

////////////////////////////////////////////////////////////////////////////////
// x_tcp_io_handler_t

/**
 * @class x_tcp_io_handler_t
 * @brief IO 句柄的抽象接口类（定义 IO 事件的处理接口）。
 * @note  在 x_tcp_io_manager_t 中，与 套接字描述符 结成映射对。
 */
class x_tcp_io_handler_t
{
    // common data types
public:
    /**
     * @enum  emIoHandleType
     * @brief 定义 x_tcp_io_handler_t 的类型枚举值。
     */
    typedef enum emIoHandleType
    {
        EIO_HTYPE_CREATOR = 0x0010,  ///< x_tcp_io_creator_t 类型
        EIO_HTYPE_HOLDER  = 0x0020,  ///< x_tcp_io_holder_t  类型
    } emIoHandleType;

    // constructor/destructor
public:
    explicit x_tcp_io_handler_t(void) {  }
    virtual ~x_tcp_io_handler_t(void) {  }

    // extensible interfaces
public:
    /**********************************************************/
    /**
     * @brief 返回对象类型标识。
     */
    virtual x_uint32_t htype(void) const = 0;

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
    virtual x_int32_t io_reading(x_handle_t xht_manager, x_sockfd_t xfdt_sockfd)
    {
        return 0;
    }

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
    virtual x_int32_t io_writing(x_handle_t xht_manager, x_sockfd_t xfdt_sockfd)
    {
        return 0;
    }
};

////////////////////////////////////////////////////////////////////////////////
// x_tcp_io_creator_t

/**
 * @class x_tcp_io_creator_t
 * @brief 协助 x_tcp_io_manager_t 创建业务层的 x_tcp_io_channel_t 对象。
 */
class x_tcp_io_creator_t : public x_tcp_io_handler_t
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
    using x_io_cwptr_t = std::weak_ptr<   x_tcp_io_channel_t >;
    using x_io_csptr_t = std::shared_ptr< x_tcp_io_channel_t >;

    // constructor/destructor
public:
    explicit x_tcp_io_creator_t(void);
    virtual ~x_tcp_io_creator_t(void);

    // overrides
public:
    /**********************************************************/
    /**
     * @brief 返回对象类型标识。
     */
    virtual x_uint32_t htype(void) const override
    {
        return EIO_HTYPE_CREATOR;
    }

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
    virtual x_int32_t io_reading(x_handle_t xht_manager, x_sockfd_t xfdt_sockfd) override;

    // public interfaces
public:
    /**********************************************************/
    /**
     * @brief 返回 持有的 IO 通道对象。
     */
    inline x_io_csptr_t get_io_channel(void) const
    {
        return m_xio_csptr;
    }

    // internal invoking
private:
    /**********************************************************/
    /**
     * @brief 尝试接收首个 IO 消息，创建业务层中对应的 x_tcp_io_channel_t 对象。
     * 
     * @param [in ] xht_manager : IO 管理对象（x_tcp_io_manager_t）。
     * @param [in ] xfdt_sockfd : 目标操作的套接字描述符。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t try_create_io_channel(x_handle_t xht_manager, x_sockfd_t xfdt_sockfd);

    // data members
private:
    x_io_csptr_t        m_xio_csptr;   ///< 持有的 IO 通道对象
    x_tcp_io_message_t  m_xmsg_swap;   ///< 用于临时数据交换操作的网络 IO 消息对象
};

////////////////////////////////////////////////////////////////////////////////
// x_tcp_io_holder_t

/**
 * @class x_tcp_io_holder_t
 * @brief 持有 x_tcp_io_channel_t 对象的辅助类，协助 x_tcp_io_manager_t 进行事件投递操作。
 */
class x_tcp_io_holder_t : public x_tcp_io_handler_t
{
    // common data types
private:
    using x_io_cwptr_t = std::weak_ptr<   x_tcp_io_channel_t >;
    using x_io_csptr_t = std::shared_ptr< x_tcp_io_channel_t >;

    // common invoking
public:
    /**********************************************************/
    /**
     * @brief 释放任务对象池中缓存的任务对象。
     */
    static x_void_t release_taskpool(void);

    // constructor/destructor
public:
    explicit x_tcp_io_holder_t(const x_io_csptr_t & xio_csptr);
    virtual ~x_tcp_io_holder_t(void);

    // overrides
public:
    /**********************************************************/
    /**
     * @brief 返回对象类型标识。
     */
    virtual x_uint32_t htype(void) const override
    {
        return EIO_HTYPE_HOLDER;
    }

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
    virtual x_int32_t io_reading(x_handle_t xht_manager, x_sockfd_t xfdt_sockfd) override;

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
    virtual x_int32_t io_writing(x_handle_t xht_manager, x_sockfd_t xfdt_sockfd) override;

    // data members
private:
    x_io_csptr_t m_xio_csptr;   ///< 持有的 IO 通道对象
};

////////////////////////////////////////////////////////////////////////////////

#endif // __XTCP_IO_HOLDER_H__
