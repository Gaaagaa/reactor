/**
 * @file    xevent_handler.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xevent_handler.h
 * 创建日期：2019年01月01日
 * 文件标识：
 * 文件摘要：定义 异步事件通知管理的控制对象 和 相关的观察者接口类。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年01月01日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#ifndef __XEVENT_HANDLER_H__
#define __XEVENT_HANDLER_H__

#include "xevent_notifier.h"

////////////////////////////////////////////////////////////////////////////////
// x_event_handler_t

/**
 * @class x_event_handler_t
 * @brief 异步事件通知管理的控制对象。
 */
class x_event_handler_t : public x_event_notifier_t
{
    // common invoking
public:
    /**********************************************************/
    /**
     * @brief x_event_handler_t 对象的单例调用接口。
     */
    static x_event_handler_t & instance(void);

    // constructor/destructor
private:
    explicit x_event_handler_t(void);
    virtual ~x_event_handler_t(void);

    x_event_handler_t(const x_event_handler_t & xobject);
    x_event_handler_t & operator=(const x_event_handler_t & xobject);

    // public interfaces
public:
    /**********************************************************/
    /**
     * @brief 打开 异步事件通知管理模块。
     */
    x_int32_t open(void);

    /**********************************************************/
    /**
     * @brief 关闭 异步事件通知管理模块。
     */
    x_void_t close(void);

    /**********************************************************/
    /**
     * @brief 判断 异步事件通知管理模块 是否已经打开。
     */
    inline x_bool_t is_open(void) const { return is_start(); }

};

////////////////////////////////////////////////////////////////////////////////
// x_event_observer_t

#include "xobserver.h"

/**
 * @class x_event_observer_t< _Ty, _Ey >
 * @brief 事件观察者的模板类。
 * @note  特化事件通知接口的格式为 func(x_uint32_t xut_size, x_pvoid_t xpvt_dptr)。
 * @param [in ] _Ty : 观察者的派生类。
 * @param [in ] _Ey : 观察的事件类型。
 */
template< class _Ty, x_uint32_t _Ey >
class x_event_observer_t : public x_observer_t< _Ty, x_uint32_t, x_uint32_t, x_pvoid_t >
{
    // common data types
public:
    using __super_type = x_observer_t< _Ty, x_uint32_t, x_uint32_t, x_pvoid_t >;
    using __obser_type = x_event_observer_t< _Ty, _Ey >;

    using x_this_t     = typename __super_type::x_this_t;
    using x_typeid_t   = typename __super_type::x_typeid_t;

    // common invoking
public:
    /**********************************************************/
    /**
     * @brief 注册事件通知。
     */
    static x_bool_t register_event_notify(void)
    {
        return x_event_handler_t::instance().register_notify_type(
                            _Ey, &x_event_observer_t< _Ty, _Ey >::notify_entry_ex);
    }

    /**********************************************************/
    /**
     * @brief 移除事件通知。
     */
    static x_void_t unregister_event_notify(void)
    {
        return x_event_handler_t::instance().unregister_notify_type(_Ey);
    }

	/**********************************************************/
	/**
	 * @brief 事件通知接口。
	 * 
	 * @param [in ] xut_evtid : 事件标识 ID。
	 * @param [in ] xht_ectxt : 事件上下文描述。
	 * @param [in ] xut_esize : 通知参数（事件数据体大小）。
	 * @param [in ] xct_edptr : 通知参数（事件数据体缓存）。
	 * 
	 * @return x_int32_t
	 *         - 操作状态值。
	 */
	static x_int32_t notify_entry_ex(x_uint32_t  xut_evtid,
                                     x_handle_t  xht_ectxt,
                                     x_uint32_t  xut_esize,
                                     x_uchar_t * xct_edptr)
    {
        __super_type::notify_entry(xut_evtid, (x_typeid_t)xht_ectxt, xut_esize, (x_pvoid_t)xct_edptr);
        return 0;
    }

    /**********************************************************/
    /**
     * @brief 事件类型。
     */
    static inline x_uint32_t etype(void) { return _Ey; }

	/**********************************************************/
	/**
	 * @brief 申请操作的通知事件。
	 * 
	 * @param [in ] xut_evtid : 通知事件 ID。
     * @param [in ] xst_objid : 通知的目标对象标识 ID（为 0 时，表示事件进行广播通知）。
	 * @param [in ] xut_size  : 数据体大小。
	 * @param [out] xpvt_dptr : 事件数据体缓存。
	 * 
	 * @return x_handle_t
	 *         - 成功，返回 事件操作句柄；
	 *         - 失败，返回 X_NULL。
	 */
	static x_handle_t alloc_event(x_uint32_t  xut_evtid,
                                  x_size_t    xst_objid,
                                  x_uint32_t  xut_size ,
                                  x_pvoid_t & xpvt_dptr)
    {
#ifdef _MSC_VER
#pragma warning(disable : 4312)
#endif // _MSC_VER

        x_async_event_t * xevent_ptr =
            x_event_handler_t::instance().alloc_event(
                _Ey, xut_evtid, reinterpret_cast< x_handle_t >(xst_objid), xut_size);

#ifdef _MSC_VER
#pragma warning(default : 4312)
#endif // _MSC_VER

        if (X_NULL == xevent_ptr)
        {
            return X_NULL;
        }

        if (xut_size > 0)
            xpvt_dptr = (x_pvoid_t *)(xevent_ptr->xct_edptr);
        else
            xpvt_dptr = X_NULL;

        return (x_handle_t)xevent_ptr;
    }

	/**********************************************************/
	/**
	 * @brief 投递通知事件。
	 * 
	 * @param [in ] xut_evtid : 通知事件 ID。
     * @param [in ] xst_objid : 通知的目标对象标识 ID（为 0 时，表示事件进行广播通知）。
	 * @param [in ] xut_size  : 数据体大小。
	 * @param [in ] xpvt_dptr : 事件数据体缓存。
	 * 
	 * @return x_int32_t
	 *         - 成功，返回 0；
	 *         - 失败，返回 错误码。
	 */
	static x_int32_t post_event(x_uint32_t xut_evtid,
                                x_size_t   xst_objid,
                                x_uint32_t xut_size ,
                                x_pvoid_t  xpvt_dptr)
    {
        x_pvoid_t  xpvt_fptr = X_NULL;
        x_handle_t xht_event = alloc_event(xut_evtid, xst_objid, xut_size, xpvt_fptr);
        if (X_NULL == xht_event)
        {
            return -1;
        }

        if ((X_NULL != xpvt_fptr) && (xut_size > 0) && (X_NULL != xpvt_dptr))
        {
            memcpy(xpvt_fptr, xpvt_dptr, xut_size);
        }

        return post_event(xht_event);
    }

	/**********************************************************/
	/**
	 * @brief 投递通知事件。
	 * 
	 * @param [in ] xht_event : 事件句柄。
	 * 
	 * @return x_int32_t
	 *         - 成功，返回 0；
	 *         - 失败，返回 错误码。
	 */
	static x_int32_t post_event(x_handle_t xht_event)
    {
        return x_event_handler_t::instance().post_event((x_async_event_t *)xht_event);
    }

	/**********************************************************/
	/**
	 * @brief 回收通知事件。
	 */
	static x_void_t recyc_event(x_handle_t xht_event)
    {
        x_event_handler_t::instance().recyc_event((x_async_event_t *)xht_event);
    }

    // constructor/destructor
public:
    x_event_observer_t(void)
    {

    }

    ~x_event_observer_t(void)
    {

    }

    // public interfaces
public:
    /**********************************************************/
    /**
     * @brief 重置（清理）对象所有通知操作的相关连接。
     */
    x_void_t reset_notify(void)
    {
        __super_type::remove_notify(reinterpret_cast< x_this_t >(this));
        __super_type::clear_mkey_map();
    }

	/**********************************************************/
	/**
	 * @brief 申请操作的通知事件。
	 * 
	 * @param [in ] xut_evtid : 通知事件 ID。
	 * @param [in ] xut_size  : 数据体大小。
	 * @param [out] xpvt_dptr : 事件数据体缓存。
	 * 
	 * @return x_handle_t
	 *         - 成功，返回 事件操作句柄；
	 *         - 失败，返回 X_NULL。
	 */
	inline x_handle_t alloc_event(x_uint32_t xut_evtid, x_uint32_t xut_size, x_pvoid_t & xpvt_dptr)
    {
        return alloc_event(xut_evtid, (x_size_t)__super_type::xid(), xut_size, xpvt_dptr);
    }

	/**********************************************************/
	/**
	 * @brief 投递通知事件。
	 * 
	 * @param [in ] xut_evtid : 通知事件 ID。
	 * @param [in ] xut_size  : 数据体大小。
	 * @param [in ] xpvt_dptr : 事件数据体缓存。
	 * 
	 * @return x_int32_t
	 *         - 成功，返回 0；
	 *         - 失败，返回 错误码。
	 */
	inline x_int32_t post_event(x_uint32_t xut_evtid, x_uint32_t xut_size, x_pvoid_t xpvt_dptr)
    {
        return post_event(xut_evtid, (x_size_t)__super_type::xid(), xut_size, xpvt_dptr);
    }
};

////////////////////////////////////////////////////////////////////////////////

#endif // __XEVENT_HANDLER_H__
