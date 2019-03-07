/**
 * @file    xtcp_io_keepalive.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xtcp_io_keepalive.cpp
 * 创建日期：2019年01月22日
 * 文件标识：
 * 文件摘要：实现 IO 存活检测（保活机制）的工具类。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年01月22日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#include "xcomm.h"
#include "xtcp_io_keepalive.h"

////////////////////////////////////////////////////////////////////////////////
// x_tcp_io_keepalive_t

//====================================================================

// 
// x_tcp_io_keepalive_t : common invoking
// 

/**********************************************************/
/**
 * @brief 当前时间戳（校正后的数值）。
 */
x_tcp_io_keepalive_t::x_timestamp_t x_tcp_io_keepalive_t::redefine_time_tick(void)
{
    return (x_timestamp_t)(get_time_tick() * (x_timestamp_t)ECV_TMSTAMP_MAXPLUS);
}

//====================================================================

// 
// x_tcp_io_keepalive_t : constructor/destructor
// 

x_tcp_io_keepalive_t::x_tcp_io_keepalive_t(x_func_ioalive_t xfunc_ptr, x_handle_t xht_context)
    : m_xfunc_ioalive(xfunc_ptr)
    , m_xht_iocontext(xht_context)
    , m_xut_tmout_kpalive(ECV_TIMEOUT_KPALIVE * (x_timestamp_t)ECV_TMSTAMP_MAXPLUS)
    , m_xut_tmout_baleful(ECV_TIMEOUT_BALEFUL * (x_timestamp_t)ECV_TMSTAMP_MAXPLUS)
    , m_xbt_running(X_FALSE)
{

}

x_tcp_io_keepalive_t::~x_tcp_io_keepalive_t(void)
{

}

//====================================================================

// 
// x_tcp_io_keepalive_t : public interfaces
// 

/**********************************************************/
/**
 * @brief 启动工作流程。
 * 
 * @param [in ] xut_tmout_kpalive : 检测存活的超时时间（单位 毫秒）。
 * @param [in ] xut_tmout_baleful : 检测恶意连接的超时时间（单位 毫秒）。
 * @param [in ] xut_tmout_mverify : 定时巡检的超时时间（单位 毫秒）。
 * 
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t x_tcp_io_keepalive_t::start(x_uint64_t xut_tmout_kpalive,
                                      x_uint64_t xut_tmout_baleful,
                                      x_uint64_t xut_tmout_mverify)
{
    x_int32_t xit_error = -1;

    do
    {
        //======================================

        if (is_start())
        {
            stop();
        }

        m_xut_tmout_kpalive = limit_bound(xut_tmout_kpalive, 20000, ECV_TIMEOUT_KPALIVE) * ECV_TMSTAMP_MAXPLUS;
        m_xut_tmout_baleful = limit_bound(xut_tmout_baleful, 20000, ECV_TIMEOUT_BALEFUL) * ECV_TMSTAMP_MAXPLUS;
        m_xut_tmout_mverify = limit_lower(xut_tmout_mverify,        ECV_TIMEOUT_MVERIFY);

        //======================================

        try
        {
            m_xbt_running = X_TRUE;
            m_xio_thread  = x_thread_t([this](void) -> x_void_t { thread_run(); });
        }
        catch (...)
        {
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
 * @brief 停止工作流程。
 */
x_void_t x_tcp_io_keepalive_t::stop(void)
{
    m_xbt_running = X_FALSE;

    if (m_xio_thread.joinable())
    {
        m_xio_thread.join();
    }

    m_xlkt_lstevt.lock();

    m_xlst_event.clear();
    m_xmap_ndesc.clear();
    m_xmap_kalive.clear();

    m_xlkt_lstevt.unlock();
}

/**********************************************************/
/**
 * @brief 投递控制事件（参看 emIoAliveEventType 描述的事件类型），请求更新 IO 节点描述信息。
 * 
 * @param [in ] xevt_optype : 事件类型（参看 emIoAliveEventType）。
 * @param [in ] xkey_sockfd : 存活检测的套接字描述符（索引键）。
 * @param [in ] xioc_handle : 套接字描述符所关联的工作句柄。
 * @param [in ] xtms_kalive : 最新的活动（保活）时间戳。
 * 
 * @return x_int32_t
 *         - 返回 0，表示投递成功；
 *         - 返回 -1，表示投递失败（工作流程未启动）。
 */
x_int32_t x_tcp_io_keepalive_t::post_event(x_eventtype_t xevt_optype,
                                           x_iokeytype_t xkey_sockfd,
                                           x_iochannel_t xioc_handle,
                                           x_timestamp_t xtms_kalive)
{
    if (!is_start())
    {
        return -1;
    }

    m_xlkt_lstevt.lock();

    x_event_t xevent;
    xevent.xevt_optype = xevt_optype;
    xevent.xkey_sockfd = xkey_sockfd;
    xevent.xioc_handle = xioc_handle;
    xevent.xtms_kalive = xtms_kalive;

    m_xlst_event.push_back(xevent);

    m_xlkt_lstevt.unlock();

    return 0;
}

//====================================================================

// 
// x_tcp_io_keepalive_t : internal invoking
// 

/**********************************************************/
/**
 * @brief 将 [ 时间戳，IO 节点索引键 ] 插入到目标映射表。
 * 
 * @param [out   ] xmap_tskey  : 操作的目标映射表（时间戳 到 IO 节点索引键 的映射表）。
 * @param [in,out] xtst_value  : 入参，具体的时间戳值；回参，矫正过的时间戳值。
 * @param [in    ] xkey_sockfd : IO 节点的索引键值。
 * 
 * @return x_bool_t
 *         - 成功，返回 X_TRUE;
 *         - 失败，返回 X_FALSE。
 */
x_bool_t x_tcp_io_keepalive_t::insert_tskey(x_map_tskey_t & xmap_tskey,
                                            x_timestamp_t & xtst_value,
                                            x_iokeytype_t xkey_sockfd)
{
    xtst_value *= (x_timestamp_t)ECV_TMSTAMP_MAXPLUS;

    for (x_timestamp_t xtst_adjust = 1;
         xtst_adjust < (x_timestamp_t)ECV_TMSTAMP_MAXPLUS;
         ++xtst_adjust)
    {
        if (xmap_tskey.find(xtst_value) == xmap_tskey.end())
        {
            xmap_tskey.insert(std::make_pair(xtst_value, xkey_sockfd));
            return X_TRUE;
        }

        xtst_value += xtst_adjust;
    }

    return X_FALSE;
}

/**********************************************************/
/**
 * @brief 同步更新 IO 节点描述信息的映射表 中的信息。
 */
x_void_t x_tcp_io_keepalive_t::update_xmap_ndesc(void)
{
    x_lst_event_t xlst_event;
    m_xlkt_lstevt.lock();
    m_xlst_event.swap(xlst_event);
    m_xlkt_lstevt.unlock();

    for (x_lst_event_t::iterator itlst = xlst_event.begin(); itlst != xlst_event.end(); ++itlst)
    {
        x_event_t & xevent = *itlst;

        x_map_ndesc_t::iterator itfind = m_xmap_ndesc.find(xevent.xkey_sockfd);
        switch (xevent.xevt_optype)
        {
        case EIOA_JOINTO:
        case EIOA_UPDATE:
            if (itfind != m_xmap_ndesc.end())
            {
                m_xmap_kalive.erase(itfind->second.xtms_kalive);
                XVERIFY(insert_tskey(m_xmap_kalive, xevent.xtms_kalive, itfind->second.xkey_sockfd));
                itfind->second.xioc_handle = xevent.xioc_handle;
                itfind->second.xtms_kalive = xevent.xtms_kalive;
            }
            else
            {
                x_io_ndesc_t & xndesc = m_xmap_ndesc[xevent.xkey_sockfd];
                xndesc.xkey_sockfd = xevent.xkey_sockfd;
                xndesc.xioc_handle = xevent.xioc_handle;
                xndesc.xtms_kalive = xevent.xtms_kalive;

                XVERIFY(insert_tskey(m_xmap_kalive, xndesc.xtms_kalive, xndesc.xkey_sockfd));
            }
            break;

        case EIOA_REMOVE:
            if (itfind != m_xmap_ndesc.end())
            {
                m_xmap_kalive.erase(itfind->second.xtms_kalive);
                m_xmap_ndesc.erase(itfind);
            }
            break;

        default:
            break;
        }
    }
}

/**********************************************************/
/**
 * @brief 执行存活检测流程。
 */
x_void_t x_tcp_io_keepalive_t::keepalive_proc(void)
{
    x_timestamp_t xtst_now = 0;
    x_timestamp_t xtst_end = 0;

    x_map_ndesc_t::iterator itmap_ndesc;
    x_map_tskey_t::iterator itmap_tskey;

    using x_vector_ts_t = std::vector< x_timestamp_t >;

    x_vector_ts_t xvec_erase;

    //======================================

    for (itmap_tskey = m_xmap_kalive.begin(); itmap_tskey != m_xmap_kalive.end(); ++itmap_tskey)
    {
        itmap_ndesc = m_xmap_ndesc.find(itmap_tskey->second);
        XVERIFY(itmap_ndesc != m_xmap_ndesc.end());
        XASSERT(itmap_tskey->first == itmap_ndesc->second.xtms_kalive);

        xtst_now = redefine_time_tick();

        // 若关联了工作句柄，则判断存活超时；否则，判断恶意连接超时
        if (X_NULL != itmap_ndesc->second.xioc_handle)
            xtst_end = itmap_tskey->first + m_xut_tmout_kpalive;
        else
            xtst_end = itmap_tskey->first + m_xut_tmout_baleful;

        // 未超时
        if (xtst_end > xtst_now)
        {
            break;
        }

        LOGW("xfdt_sockfd[%s:%d] <%s> timeout!",
             sockfd_remote_ip(itmap_ndesc->second.xkey_sockfd, LOG_BUF(64), 64),
             sockfd_remote_port(itmap_ndesc->second.xkey_sockfd),
             (X_NULL != itmap_ndesc->second.xioc_handle) ? "keepalive" : "baleful");

        // 关闭超时的套接字
        if (X_NULL != m_xfunc_ioalive)
        {
            m_xfunc_ioalive(itmap_ndesc->second.xkey_sockfd, EIO_AEC_TIMEOUT, m_xht_iocontext);
        }

        // 加入移除操作的队列
        xvec_erase.push_back(itmap_tskey->first);
    }

    //======================================

    for (x_vector_ts_t::iterator itvec = xvec_erase.begin(); itvec != xvec_erase.end(); ++itvec)
    {
        m_xmap_kalive.erase(*itvec);
    }

    //======================================
}

/**********************************************************/
/**
 * @brief 工作线程运行的入口函数。
 */
x_void_t x_tcp_io_keepalive_t::thread_run(void)
{
    using x_time_clock_t  = std::chrono::system_clock;
    using x_time_point_t  = x_time_clock_t::time_point;
    using x_millisecond_t = std::chrono::milliseconds;

    x_time_point_t xtime_bgn = x_time_clock_t::now();
    x_time_point_t xtime_end = xtime_bgn + x_millisecond_t(m_xut_tmout_mverify);
    x_time_point_t xtime_tmp;

    while (m_xbt_running)
    {
        //======================================

        // 同步更新相关数据
        update_xmap_ndesc();

        // IO 存活检测
        keepalive_proc();

        //======================================
        // 回调巡检

        if (X_NULL != m_xfunc_ioalive)
        {
            xtime_tmp = x_time_clock_t::now();
            if ((xtime_tmp <= xtime_bgn) || (xtime_tmp >= xtime_end))
            {
                m_xfunc_ioalive(X_INVALID_SOCKFD, EIO_AEC_MVERIFY, m_xht_iocontext);

                xtime_bgn = x_time_clock_t::now();
                xtime_end = xtime_bgn + x_millisecond_t(m_xut_tmout_mverify);
            }
        }

        //======================================
        // 线程暂停操作

        xtime_tmp = x_time_clock_t::now() + x_millisecond_t(50);
        while (m_xbt_running && m_xlst_event.empty() && (x_time_clock_t::now() < xtime_tmp))
        {
            std::this_thread::sleep_for(x_millisecond_t(1));
        }

        //======================================
    }
}
