/**
 * @file    xmsg_handler.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xmsg_handler.h
 * 创建日期：2019年02月14日
 * 文件标识：
 * 文件摘要：定义 消息通知管理的控制对象 和 相关的订阅者接口类。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年02月14日
 * 版本摘要：
 * 
 * 取代版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#ifndef __XMSG_HANDLER_H__
#define __XMSG_HANDLER_H__

#include "xmsg_publisher.h"

////////////////////////////////////////////////////////////////////////////////
// x_msg_handler_t

/**
 * @class x_msg_handler_t
 * @brief 消息通知管理的控制对象（单例模式调用）。
 */
class x_msg_handler_t : public x_msg_publisher_t
{
    // common invoking
public:
    /**********************************************************/
    /**
     * @brief x_msg_handler_t 对象的单例调用接口。
     */
    static x_msg_handler_t & instance(void);

    // constructor/destructor
private:
    explicit x_msg_handler_t(void);
    virtual ~x_msg_handler_t(void);

    x_msg_handler_t(const x_msg_handler_t & xobject) = delete;
    x_msg_handler_t & operator=(const x_msg_handler_t & xobject) = delete;

    // public interfaces
public:
    /**********************************************************/
    /**
     * @brief 打开 消息通知管理模块。
     */
    x_int32_t open(void);

    /**********************************************************/
    /**
     * @brief 关闭 消息通知管理模块。
     */
    x_void_t close(void);

    /**********************************************************/
    /**
     * @brief 判断 消息通知管理模块 是否已经打开。
     */
    inline x_bool_t is_open(void) const { return is_start(); }

};

////////////////////////////////////////////////////////////////////////////////
// x_spec_subscriber_t

#include "xmsg_subscriber.h"

/**
 * @class x_spec_subscriber_t< _Ty, _Mt >
 * @brief 消息订阅者的模板类。
 * @note  特化消息分派接口的格式为 memfunc(x_uint32_t xut_size, x_pvoid_t xpvt_dptr)。
 * @param [in ] _Ty : 订阅者的派生类。
 * @param [in ] _Mt : 订阅的消息类型。
 */
template< class _Ty, x_uint32_t _Mt >
class x_spec_subscriber_t : public x_msg_subscriber_t< _Ty, x_uint32_t, x_uint32_t, x_pvoid_t >
{
    // common data types
public:
    using __super_type = x_msg_subscriber_t< _Ty, x_uint32_t, x_uint32_t, x_pvoid_t >;
    using __subscriber = x_spec_subscriber_t< _Ty, _Mt >;

    using x_this_t     = typename __super_type::x_this_t;
    using x_typeid_t   = typename __super_type::x_typeid_t;

private:
    using x_msgctxt_t  = x_msg_publisher_t::x_msgctxt_t;

    // common invoking
public:
    /**********************************************************/
    /**
     * @brief 注册消息订阅的分派接口。
     */
    static x_bool_t register_msg_diapatch(void)
    {
        return x_msg_handler_t::instance().register_subscribe_type(
                            _Mt, &x_spec_subscriber_t< _Ty, _Mt >::dispatch_entry_ex);
    }

    /**********************************************************/
    /**
     * @brief 移除消息订阅的分派接口。
     */
    static x_void_t unregister_msg_diapatch(void)
    {
        return x_msg_handler_t::instance().unregister_subscribe_type(_Mt);
    }

    /**********************************************************/
    /**
     * @brief 消息分派操作接口。
     *
     * @param [in ] xut_msgid : 消息标识 ID。
     * @param [in ] xht_mctxt : 消息上下文描述句柄。
     * @param [in ] xut_msize : 消息数据体大小。
     * @param [in ] xct_mdptr : 消息数据体缓存。
     *
     * @return x_int32_t
     *         - 操作状态值（未定义）。
     */
    static x_int32_t dispatch_entry_ex(x_uint32_t  xut_msgid,
                                       x_handle_t  xht_mctxt,
                                       x_uint32_t  xut_msize,
                                       x_uchar_t * xct_mdptr)
    {
        __super_type::dispatch_entry(xut_msgid, (x_typeid_t)xht_mctxt, xut_msize, (x_pvoid_t)xct_mdptr);
        return 0;
    }

    /**********************************************************/
    /**
     * @brief 订阅者类型。
     */
    static inline x_uint32_t stype(void) { return _Mt; }

	/**********************************************************/
	/**
	 * @brief 申请操作的消息对象。
	 * 
	 * @param [in ] xut_msgid : 消息标识 ID。
     * @param [in ] xst_objid : 目标对象标识 ID（为 0 时，表示消息进行广播通知）。
	 * @param [in ] xut_msize : 消息数据体大小。
	 * @param [out] xpvt_dptr : 消息数据体缓存。
	 * 
	 * @return x_handle_t
	 *         - 成功，返回 消息对象操作句柄；
	 *         - 失败，返回 X_NULL。
	 */
	static x_handle_t alloc_msg(x_uint32_t  xut_msgid,
                                x_size_t    xst_objid,
                                x_uint32_t  xut_msize,
                                x_pvoid_t & xpvt_dptr)
    {
#ifdef _MSC_VER
#pragma warning(disable : 4312)
#endif // _MSC_VER

        x_msgctxt_t * xmsg_ptr =
            x_msg_handler_t::instance().alloc_msg(
                _Mt, xut_msgid, reinterpret_cast< x_handle_t >(xst_objid), xut_msize);

#ifdef _MSC_VER
#pragma warning(default : 4312)
#endif // _MSC_VER

        if (X_NULL == xmsg_ptr)
        {
            return X_NULL;
        }

        if (xut_msize > 0)
            xpvt_dptr = (x_pvoid_t)(xmsg_ptr->xct_mdptr);
        else
            xpvt_dptr = X_NULL;

        return (x_handle_t)xmsg_ptr;
    }

	/**********************************************************/
	/**
	 * @brief 投递消息。
	 * 
     * @param [in ] xut_msgid : 消息标识 ID。
     * @param [in ] xst_objid : 目标对象标识 ID（为 0 时，表示消息进行广播通知）。
     * @param [in ] xut_msize : 消息数据体大小。
     * @param [out] xpvt_dptr : 消息数据体缓存。
	 * 
	 * @return x_int32_t
	 *         - 成功，返回 0；
	 *         - 失败，返回 错误码。
	 */
    static x_int32_t post_msg(x_uint32_t xut_msgid,
                              x_size_t   xst_objid,
                              x_uint32_t xut_msize,
                              x_pvoid_t  xpvt_dptr)
    {
        x_pvoid_t  xpvt_rptr = X_NULL;
        x_handle_t xht_msg = alloc_msg(xut_msgid, xst_objid, xut_msize, xpvt_rptr);
        if (X_NULL == xht_msg)
        {
            return -1;
        }

        if ((X_NULL != xpvt_rptr) && (xut_msize > 0) && (X_NULL != xpvt_dptr))
        {
            memcpy(xpvt_rptr, xpvt_dptr, xut_msize);
        }

        return post_msg(xht_msg);
    }

	/**********************************************************/
	/**
	 * @brief 投递消息。
	 */
	static x_int32_t post_msg(x_handle_t xht_msg)
    {
        return x_msg_handler_t::instance().post_msg((x_msgctxt_t *)xht_msg);
    }

	/**********************************************************/
	/**
	 * @brief 回收消息。
	 */
	static x_void_t recyc_event(x_handle_t xht_msg)
    {
        x_msg_handler_t::instance().recyc_msg((x_msgctxt_t *)xht_msg);
    }

    // constructor/destructor
public:
    x_spec_subscriber_t(void)
    {

    }

    ~x_spec_subscriber_t(void)
    {

    }

    // public interfaces
public:
    /**********************************************************/
    /**
     * @brief 重置（清理）对象所有消息分派操作的相关连接。
     */
    x_void_t reset_dispatch(void)
    {
        __super_type::remove_dispatch(reinterpret_cast< x_this_t >(this));
        __super_type::clear_mkey_map();
    }

	/**********************************************************/
	/**
	 * @brief 申请操作的消息。
	 * 
	 * @param [in ] xut_msgid : 消息标识 ID。
	 * @param [in ] xut_msize : 消息数据体大小。
	 * @param [out] xpvt_dptr : 消息数据体缓存。
	 * 
	 * @return x_handle_t
	 *         - 成功，返回 消息对象操作句柄；
	 *         - 失败，返回 X_NULL。
	 */
	inline x_handle_t alloc_msg(x_uint32_t xut_msgid, x_uint32_t xut_msize, x_pvoid_t & xpvt_dptr)
    {
        return alloc_msg(xut_msgid, (x_size_t)__super_type::xid(), xut_msize, xpvt_dptr);
    }

	/**********************************************************/
	/**
	 * @brief 投递消息。
	 * 
     * @param [in ] xut_msgid : 消息标识 ID。
     * @param [in ] xut_msize : 消息数据体大小。
     * @param [out] xpvt_dptr : 消息数据体缓存。
	 * 
	 * @return x_int32_t
	 *         - 成功，返回 0；
	 *         - 失败，返回 错误码。
	 */
	inline x_int32_t post_msg(x_uint32_t xut_msgid, x_uint32_t xut_msize, x_pvoid_t xpvt_dptr)
    {
        return post_msg(xut_msgid, (x_size_t)__super_type::xid(), xut_msize, xpvt_dptr);
    }
};

////////////////////////////////////////////////////////////////////////////////

/**
 * @enum  emMsgSubscriberType
 * @brief 定义所有消息订阅者类型的枚举值。
 */
typedef enum emMsgSubscriberType
{
    EM_EST_MASTER    = 0x00000100,    ///< master 消息订阅者对象接收的消息类型
} emMsgSubscriberType;

////////////////////////////////////////////////////////////////////////////////

#endif // __XMSG_HANDLER_H__
