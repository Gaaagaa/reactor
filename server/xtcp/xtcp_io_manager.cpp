/**
 * @file    xtcp_io_manager.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xtcp_io_manager.cpp
 * 创建日期：2019年01月16日
 * 文件标识：
 * 文件摘要：辅助 x_tcp_io_server_t 对象，进行 套接字 和 IO句柄对象(x_tcp_io_handler_t)
 *          进行映射、消息投递 等工作的管理类。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年01月16日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#include "xcomm.h"
#include "xtcp_io_manager.h"
#include "xmaphandle.h"
#include "xtcp_io_server.h"
#include "xtcp_io_holder.h"

////////////////////////////////////////////////////////////////////////////////
// x_tcp_io_manager_t

//====================================================================

// 
// x_tcp_io_manager_t : constructor/destructor
// 

x_tcp_io_manager_t::x_tcp_io_manager_t(x_handle_t xht_tcpserver)
    : m_xht_tcpserver(xht_tcpserver)
    , m_xfunc_iocbk(X_NULL)
    , m_xht_cbk_ctxt(X_NULL)
{
    XVERIFY(X_NULL != (m_xht_mapsockfd = maptbl_create(ECV_MAPSOCK_CAPACITY)));
}

x_tcp_io_manager_t::~x_tcp_io_manager_t(void)
{
    maptbl_destroy(m_xht_mapsockfd);
    m_xht_mapsockfd = X_NULL;
}

//====================================================================

// 
// x_tcp_io_manager_t : public interfaces
// 

/**********************************************************/
/**
 * @brief 启动 IO 管理模块。
 * 
 * @param [in ] xut_threads : 业务处理的工作线程的数量（若为 0，将取 hardware_concurrency() 返回值的 2倍 + 1）。
 * 
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t x_tcp_io_manager_t::start(x_uint32_t xut_threads)
{
    x_int32_t xit_error = -1;

    do
    {
        //======================================

        if (is_start())
        {
            stop();
        }

        //======================================
        // 启动业务处理的线程池

        if (!m_xthreadpool.startup(xut_threads))
        {
            LOGE("m_xthreadpool.startup(xut_threads[%d]) return false!", xut_threads);
            xit_error = -1;
            break;
        }

        //======================================

        xit_error = 0;
    } while (0);

    if (0 != xit_error)
    {
        stop();
    }

    return xit_error;
}

/**********************************************************/
/**
 * @brief 停止 IO 管理模块。
 */
x_void_t x_tcp_io_manager_t::stop(void)
{
    m_xthreadpool.shutdown();
    m_xthreadpool.cleanup_task();
    cleanup();

    x_tcp_io_task_t::taskpool().free_extra();
    x_tcp_io_message_t::xmsg_mempool().release_timeout_memblock(0);
}

/**********************************************************/
/**
 * @brief 注册套接字的 写就绪 事件。
 */
x_int32_t x_tcp_io_manager_t::register_pollout(x_sockfd_t xfdt_sockfd)
{
    XASSERT(X_NULL != m_xht_tcpserver);
    x_tcp_io_server_t * xserver_ptr = (x_tcp_io_server_t *)m_xht_tcpserver;

    return xserver_ptr->set_pollout(xfdt_sockfd, X_TRUE);
}

/**********************************************************/
/**
 * @brief 移除所管理的套接字。
 */
x_int32_t x_tcp_io_manager_t::remove_io_holder(x_sockfd_t xfdt_sockfd)
{
    XASSERT(X_NULL != m_xht_tcpserver);
    x_tcp_io_server_t * xserver_ptr = (x_tcp_io_server_t *)m_xht_tcpserver;

    return xserver_ptr->io_handle_close(-1, xfdt_sockfd);
}

/**********************************************************/
/**
 * @brief 更新套接字的保活时间戳。
 */
x_int32_t x_tcp_io_manager_t::update_io_kpalive(x_sockfd_t xfdt_sockfd, x_handle_t xht_channel)
{
    XASSERT(X_NULL != m_xht_tcpserver);
    x_tcp_io_server_t * xhserver_ptr = (x_tcp_io_server_t *)m_xht_tcpserver;

    return xhserver_ptr->post_ioalive_event(x_tcp_io_keepalive_t::EIOA_UPDATE,
                                            xfdt_sockfd,
                                            xht_channel,
                                            get_time_tick());
}

/**********************************************************/
/**
 * @brief 返回当前管理的 IO 句柄对象(x_tcp_io_handler_t)。
 */
x_uint32_t x_tcp_io_manager_t::count(void) const
{
    return (x_uint32_t)maptbl_count(m_xht_mapsockfd);
}

/**********************************************************/
/**
 * @brief 提交 IO 处理的任务对象。
 */
x_void_t x_tcp_io_manager_t::submit_io_task(x_task_ptr_t xtask_ptr)
{
    if (m_xthreadpool.is_startup())
    {
        m_xthreadpool.submit_task(xtask_ptr);
    }
}

/**********************************************************/
/**
 * @brief 回调方式创建业务层工作对象。
 */
x_int32_t x_tcp_io_manager_t::create_io_channel(x_tcp_io_create_args_t & xio_create_args)
{
    if (X_NULL != m_xfunc_iocbk)
    {
        return m_xfunc_iocbk(xio_create_args.xfdt_sockfd,
                             (x_handle_t)&xio_create_args,
                             EIO_ECBK_CREATE,
                             m_xht_cbk_ctxt);
    }

    return -1;
}

//====================================================================

// 
// x_tcp_io_manager_t : io event
// 

/**********************************************************/
/**
 * @brief 处理 接收到套接字连接事件 的操作接口。
 * 
 * @param [in ] xfdt_sockfd : 触发该事件的套接字描述符。
 * 
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t x_tcp_io_manager_t::io_event_accept(x_sockfd_t xfdt_sockfd)
{
    x_int32_t  xit_error   = -1;
    x_handle_t xht_handler = X_NULL;

    do 
    {
        //======================================
        // 进行锁测试，若成功，则表示映射表中已经存在
        // 对应套接字映射节点，这情况不应该发生

        xit_error = maptbl_lock(m_xht_mapsockfd, (x_size_t)xfdt_sockfd, &xht_handler, MAPTBL_TIMEOUT_INFINIT);
        if (MAPTBL_ERR_SUCCESS == xit_error)
        {
            LOGE("maptbl_lock(m_xht_mapsockfd, (x_size_t)xfdt_sockfd[%d], ...) OK，xfdt_sockfd already exists in the map table!",
                 xfdt_sockfd);
            maptbl_unlock(m_xht_mapsockfd, (x_size_t)xfdt_sockfd);
            XASSERT(X_FALSE);
            xit_error = -1;
            break;
        }

        //======================================
        // 回调通知，主要目的是要验证 该套接字 是否可接受后续服务

        if (X_NULL != m_xfunc_iocbk)
        {
            xit_error = m_xfunc_iocbk(xfdt_sockfd, X_NULL, EIO_ECBK_ACCEPT, m_xht_cbk_ctxt);
            if (0 != xit_error)
            {
                LOGE("m_xfunc_iocbk(xfdt_sockfd[%d], X_NULL, EIO_ECBK_ACCEPT, m_xht_cbk_ctxt) return error : %d",
                     xfdt_sockfd, xit_error);
                break;
            }
        }

        //======================================
        // 新增 x_tcp_io_creator_t 对象，并加入到映射表中
        // 等待完成 x_tcp_io_channel_t 对象的创建后，
        // 再将映射对象改成 x_tcp_io_holder_t 对象

        xht_handler = (x_handle_t)(new x_tcp_io_creator_t());
        if (X_NULL == xht_handler)
        {
            LOGE("(x_handle_t)(new x_tcp_io_creator_t()) return X_NULL");
            xit_error = -1;
            XASSERT(X_FALSE);
            break;
        }

        xit_error = maptbl_insert(m_xht_mapsockfd, (x_size_t)xfdt_sockfd, xht_handler, MAPTBL_TIMEOUT_INFINIT);
        if (MAPTBL_ERR_SUCCESS != xit_error)
        {
            LOGE("maptbl_insert(m_xht_mapsockfd, (x_size_t)xfdt_sockfd[%d], ...) return error : %d",
                 xfdt_sockfd, xit_error);
            delete (x_tcp_io_handler_t *)xht_handler;
            xht_handler = X_NULL;
            break;
        }

        //======================================
        xit_error = 0;
    } while (0);

    return xit_error;
}

/**********************************************************/
/**
 * @brief 处理 套接字关闭事件 的操作接口。
 * 
 * @param [in ] xfdt_sockfd : 触发该事件的套接字描述符。
 * 
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t x_tcp_io_manager_t::io_event_close(x_sockfd_t xfdt_sockfd)
{
    x_int32_t  xit_error   = -1;
    x_handle_t xht_handler = X_NULL;

    //======================================

    do
    {
        xit_error = maptbl_lock(m_xht_mapsockfd, (x_size_t)xfdt_sockfd, &xht_handler, MAPTBL_TIMEOUT_INFINIT);
        if (MAPTBL_ERR_SUCCESS != xit_error)
        {
            LOGE("maptbl_lock(m_xht_mapsockfd, (x_size_t)xfdt_sockfd[%d], ...) return error : %d",
                 xfdt_sockfd, xit_error);
            break;
        }

        if (X_NULL != xht_handler)
        {
            delete (x_tcp_io_handler_t *)xht_handler;
            xht_handler = X_NULL;
        }

        maptbl_delete(m_xht_mapsockfd, (x_size_t)xfdt_sockfd, 0);
        maptbl_unlock(m_xht_mapsockfd, (x_size_t)xfdt_sockfd);
    } while (0);

    //======================================

    return xit_error;
}

/**********************************************************/
/**
 * @brief 处理 套接字可读事件 的操作接口。
 * 
 * @param [in ] xfdt_sockfd : 触发该事件的套接字描述符。
 * 
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t x_tcp_io_manager_t::io_event_read(x_sockfd_t xfdt_sockfd)
{
    x_int32_t  xit_error   = -1;
    x_handle_t xht_handler = X_NULL;

    x_tcp_io_handler_t * xio_handler = X_NULL;
    x_tcp_io_creator_t * xio_creator = X_NULL;

    //======================================

    x_bool_t xbt_lock = X_FALSE;

    do
    {
        xit_error = maptbl_lock(m_xht_mapsockfd, (x_size_t)xfdt_sockfd, &xht_handler, MAPTBL_TIMEOUT_INFINIT);
        if (MAPTBL_ERR_SUCCESS != xit_error)
        {
            LOGE("maptbl_lock(m_xht_mapsockfd, (x_size_t)xfdt_sockfd[%d], ...) return error : %d",
                 xfdt_sockfd, xit_error);
            break;
        }

        xbt_lock = X_TRUE;

        if (X_NULL == xht_handler)
        {
            LOGW("(X_NULL == xht_handler) xfdt_sockfd : %d", xfdt_sockfd);
            xit_error = -1;
            break;
        }

        xio_handler = (x_tcp_io_handler_t *)xht_handler;

        //======================================
        // x_tcp_io_holder_t 对象的 io_reading() 处理流程

        // 已经创建了 x_tcp_io_channnel_t 对象，
        // 则进行 读事件 的投递操作
        if (x_tcp_io_handler_t::EIO_HTYPE_HOLDER == xio_handler->htype())
        {
            xit_error = xio_handler->io_reading((x_handle_t)this, xfdt_sockfd);
            if (0 != xit_error)
            {
                LOGE("xio_handler->io_reading(..., xfdt_sockfd[%d]) return error : %d",
                        xfdt_sockfd, xit_error);
            }

            break;
        }

        //======================================
        // x_tcp_io_creator_t 对象的 io_reading() 处理流程
        // 未创建 x_tcp_io_channnel_t 对象，则尝试 创建操作

        if (x_tcp_io_handler_t::EIO_HTYPE_CREATOR != xio_handler->htype())
        {
            LOGW("x_tcp_io_handler_t::EIO_HTYPE_CREATOR[%d] != xio_handler->htype()[%d]",
                 x_tcp_io_handler_t::EIO_HTYPE_CREATOR, xio_handler->htype());
            xit_error = -1;
            break;
        }

        xio_creator = (x_tcp_io_creator_t *)xht_handler;
        xit_error = xio_creator->io_reading((x_handle_t)this, xfdt_sockfd);
        if (0 != xit_error)
        {
            LOGE("xio_creator->io_reading(..., xfdt_sockfd[%d]) return error : %d",
                 xfdt_sockfd, xit_error);
            break;
        }

        // 若成功创建了业务层的 x_tcp_io_channel_t 对象，
        // 则将新建的对象更新至映射表中，之后删除 xio_creator 对象
        if (nullptr != xio_creator->get_io_channel())
        {
            xht_handler = (x_handle_t)(new x_tcp_io_holder_t(xio_creator->get_io_channel()));
            XASSERT(X_NULL != xht_handler);

            XVERIFY(MAPTBL_ERR_SUCCESS ==
                maptbl_update(m_xht_mapsockfd, (x_size_t)xfdt_sockfd, xht_handler, 0));

            delete xio_creator;
            xio_creator = X_NULL;
        }

        //======================================
        xit_error = 0;
    } while (0);

    if (xbt_lock)
    {
        maptbl_unlock(m_xht_mapsockfd, (x_size_t)xfdt_sockfd);
    }

    //======================================

    return xit_error;
}

/**********************************************************/
/**
 * @brief 处理 套接字可写事件 的操作接口。
 * 
 * @param [in ] xfdt_sockfd : 触发该事件的套接字描述符。
 * 
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t x_tcp_io_manager_t::io_event_write(x_sockfd_t xfdt_sockfd)
{
    x_int32_t  xit_error   = -1;
    x_handle_t xht_handler = X_NULL;

    //======================================

    do
    {
        xit_error = maptbl_lock(m_xht_mapsockfd, (x_size_t)xfdt_sockfd, &xht_handler, MAPTBL_TIMEOUT_INFINIT);
        if (MAPTBL_ERR_SUCCESS != xit_error)
        {
            LOGE("maptbl_lock(m_xht_mapsockfd, (x_size_t)xfdt_sockfd[%d], ...) return error : %d",
                 xfdt_sockfd, xit_error);
            break;
        }

        if (X_NULL != xht_handler)
        {
            xit_error = ((x_tcp_io_handler_t *)xht_handler)->io_writing((x_handle_t)this, xfdt_sockfd);
            if (0 != xit_error)
            {
                LOGE("xht_handler->io_writing((x_handle_t)this, xfdt_sockfd[%d]) return error : %d",
                     xfdt_sockfd, xit_error);
            }
        }

        maptbl_unlock(m_xht_mapsockfd, (x_size_t)xfdt_sockfd);
    } while (0);

    //======================================

    return xit_error;
}

/**********************************************************/
/**
 * @brief 处理 套接字定时巡检事件 的操作接口。
 * 
 * @param [in ] xfdt_sockfd : 触发该事件的套接字描述符。
 * 
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t x_tcp_io_manager_t::io_event_verify(x_sockfd_t xfdt_sockfd)
{
    x_int32_t  xit_error   = -1;
    x_handle_t xht_handler = X_NULL;

    //======================================

    do
    {
        xit_error = maptbl_lock(m_xht_mapsockfd, (x_size_t)xfdt_sockfd, &xht_handler, MAPTBL_TIMEOUT_INFINIT);
        if (MAPTBL_ERR_SUCCESS != xit_error)
        {
            LOGE("maptbl_lock(m_xht_mapsockfd, (x_size_t)xfdt_sockfd[%d], ...) return error : %d",
                 xfdt_sockfd, xit_error);
            break;
        }

        if (X_NULL != xht_handler)
        {
            xit_error = ((x_tcp_io_handler_t *)xht_handler)->io_verify((x_handle_t)this, xfdt_sockfd);
            if (0 != xit_error)
            {
                LOGE("xht_handler->io_verify((x_handle_t)this, xfdt_sockfd[%d]) return error : %d",
                     xfdt_sockfd, xit_error);
            }
        }

        maptbl_unlock(m_xht_mapsockfd, (x_size_t)xfdt_sockfd);
    } while (0);

    //======================================

    return xit_error;
}

//====================================================================

// 
// x_tcp_io_manager_t : internal invoking
// 

/**********************************************************/
/**
 * @brief 套接字映射表遍历清除操作的回调函数接口。
 *
 * @param [in    ] xht_maptbl  : 映射表的操作句柄。
 * @param [in    ] xfdt_sockfd : 套接字。
 * @param [in,out] xht_handler : 子项关联的映射句柄（可进行回参设置）。
 *
 * @return x_bool_t
 *         - 返回 X_TRUE ，继续遍历操作；
 *         - 返回 X_FALSE，则终止遍历操作。
 */
x_bool_t x_tcp_io_manager_t::mapsockfd_cleanup(x_handle_t xht_maptbl, x_sockfd_t xfdt_sockfd, x_handle_t * xht_handler)
{
    x_tcp_io_handler_t * xio_handler = (x_tcp_io_handler_t *)(*xht_handler);
    if (X_NULL != xio_handler)
    {
        delete xio_handler;
        xio_handler = X_NULL;
        *xht_handler = X_NULL;
    }

    sockfd_close(xfdt_sockfd);

    return X_TRUE;
}

/**********************************************************/
/**
 * @brief 强制清理掉所有的 IO句柄对象(x_tcp_io_handler_t)。
 */
x_void_t x_tcp_io_manager_t::cleanup(void)
{
    maptbl_trav(
        m_xht_mapsockfd,
        0,
        MAPTBL_TIMEOUT_INFINIT,
        [](x_handle_t xht_maptbl, x_size_t xst_itemkey, x_handle_t * xht_handler, x_handle_t xht_context) -> x_bool_t
        {
            return ((x_tcp_io_manager_t *)xht_context)->mapsockfd_cleanup(xht_maptbl, (x_sockfd_t)xst_itemkey, xht_handler);
        },
        (x_handle_t)this);

    maptbl_cleanup(m_xht_mapsockfd);
}

