/**
 * @file    xmsg_subscriber.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xmsg_subscriber.h
 * 创建日期：2019年02月14日
 * 文件标识：
 * 文件摘要：消息订阅者的消息分派接口模板类。
 * 特别鸣谢：Lee哥
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

#ifndef __XMSG_SUBSCRIBER_H__
#define __XMSG_SUBSCRIBER_H__

#include <map>
#include <atomic>
#include <mutex>

////////////////////////////////////////////////////////////////////////////////
// x_msg_subscriber_t

/**
 * @class x_msg_subscriber_t< _Ty, _Ky, _Args... >
 * @brief 消息订阅者的消息分派接口模板类。
 * 
 * @param[in ] _Ty      : 订阅者的派生类。
 * @param[in ] _Ky      : 消息分派操作的 索引键 类型。
 * @param[in ] _Args... : 消息分派操作的 接口函数 参数列表。
 */
template< class _Ty, typename _Ky, typename... _Args >
class x_msg_subscriber_t
{
    // common data types
public:
    using x_func_t     = void (_Ty::*)(_Args...);
    using x_mkey_t     = _Ky;
    using x_typeid_t   = size_t;
    using x_mapfunc_t  = std::map< x_mkey_t, x_func_t >;
    using x_this_t     = x_msg_subscriber_t< _Ty, _Ky, _Args... > *;
    using x_mseqid_t   = std::atomic< x_typeid_t >;
    using x_mlocker_t  = std::mutex;
    using x_autolock_t = std::lock_guard< x_mlocker_t >;
    using x_mapthis_t  = std::map< x_typeid_t, x_this_t >;

    /** 广播操作使用的标识 ID */
    static constexpr const x_typeid_t XID_BROADCAST = 0;

    // common invoking
public:
    /**********************************************************/
    /**
     * @brief 消息分派操作的入口函数。
     * 
     * @param [in ] xmkey    : 消息分派的 索引键。
     * @param [in ] xid : 分派操作的目标对象标识 ID（为 XID_BROADCAST 时，则广播通知）。
     * @param [in ] xargs... : 分派操作的参数列表。
     */
    static void dispatch_entry(x_mkey_t xmkey, x_typeid_t xid, _Args ... xargs)
    {
        typename x_mapthis_t::iterator itmap;

        x_autolock_t xautolock(_S_xmt_locker);

        if (XID_BROADCAST == xid)
        {
            for (itmap = _S_map_object.begin(); itmap != _S_map_object.end(); ++itmap)
            {
                itmap->second->dispatch(xmkey, xargs...);
            }
        }
        else
        {
            itmap = _S_map_object.find(xid);
            if (itmap != _S_map_object.end())
            {
                itmap->second->dispatch(xmkey, xargs...);
            }
        }
    }

    /**********************************************************/
    /**
     * @brief 将订阅者对象加入消息分派的目标操作对象表。
     */
    static bool jointo_dispatch(x_this_t xobject_ptr)
    {
        if (nullptr != xobject_ptr)
        {
            x_autolock_t xautolock(_S_xmt_locker);
            if (_S_map_object.find(xobject_ptr->xid()) == _S_map_object.end())
            {
                _S_map_object.insert(std::make_pair(xobject_ptr->xid(), xobject_ptr));
                return true;
            }
        }

        return false;
    }

    /**********************************************************/
    /**
     * @brief 将订阅者对象移出消息分派的目标操作对象表。
     */
    static void remove_dispatch(x_this_t xobject_ptr)
    {
        if (nullptr != xobject_ptr)
        {
            x_autolock_t xautolock(_S_xmt_locker);
            _S_map_object.erase(xobject_ptr->xid());
        }
    }

private:
    static x_mseqid_t  _S_xid_mseqid;   ///< 订阅者对象ID生成器
    static x_mlocker_t _S_xmt_locker;   ///< 订阅者对象映射表的访问操作锁
    static x_mapthis_t _S_map_object;   ///< 订阅者对象映射表

    // constructor/destructor
public:
    x_msg_subscriber_t(void)
    {
        m_xid_suber = _S_xid_mseqid.fetch_add(1);
    }

    ~x_msg_subscriber_t(void)
    {

    }

    // public interfaces
public:
    /**********************************************************/
    /**
     * @brief 对象标识 ID。
     */
    inline x_typeid_t xid(void) const { return m_xid_suber; }

    /**********************************************************/
    /**
     * @brief 消息分派操作。
     */
    void dispatch(x_mkey_t xmkey, _Args ... xargs)
    {
        typename x_mapfunc_t::iterator itfind = m_map_mfunc.find(xmkey);
        if (itfind != m_map_mfunc.end())
        {
            (reinterpret_cast< _Ty * >(this)->*(itfind->second))(xargs...);
        }
    }

    /**********************************************************/
    /**
     * @brief 注册消息处理接口。
     * 
     * @param [in ] xmkey : 消息处理的 索引键。
     * @param [in ] xfunc : 消息处理的 接口。
     * 
     * @return bool
     *         - 成功，返回 true；
     *         - 失败，返回 false。
     */
    bool register_mkey(x_mkey_t xmkey, x_func_t xfunc)
    {
        typename x_mapfunc_t::iterator itfind = m_map_mfunc.find(xmkey);
        if (itfind == m_map_mfunc.end())
        {
            m_map_mfunc.insert(std::make_pair(xmkey, xfunc));
            return true;
        }

        return false;
    }

    /**********************************************************/
    /**
     * @brief 移除消息处理接口。
     */
    void unregister_mkey(x_mkey_t xmkey)
    {
        m_map_mfunc.erase(xmkey);
    }

    /**********************************************************/
    /**
     * @brief 清除所有的消息处理接口。
     */
    void clear_mkey_map(void)
    {
        m_map_mfunc.clear();
    }

    // data members
private:
    x_typeid_t   m_xid_suber;   ///< 对象标识 ID
    x_mapfunc_t  m_map_mfunc;   ///< 消息处理接口的映射表
};

//====================================================================

#ifndef SELECTANY
#if defined(_MSC_VER)
#define SELECTANY   __declspec(selectany)
#else // !defined(_MSC_VER)
#define SELECTANY   __attribute__((weak))
#endif // defined(_MSC_VER)
#endif // SELECTANY

/** 订阅者对象ID生成器 */
template< class _Ty, typename _Ky, typename... _Args >
SELECTANY typename x_msg_subscriber_t< _Ty, _Ky, _Args... >::x_mseqid_t
                   x_msg_subscriber_t< _Ty, _Ky, _Args... >::_S_xid_mseqid(1);

/** 订阅者对象映射表的访问操作锁 */
template< class _Ty, typename _Ky, typename... _Args >
SELECTANY typename x_msg_subscriber_t< _Ty, _Ky, _Args... >::x_mlocker_t
                   x_msg_subscriber_t< _Ty, _Ky, _Args... >::_S_xmt_locker;

/** 订阅者对象映射表 */
template< class _Ty, typename _Ky, typename... _Args >
SELECTANY typename x_msg_subscriber_t< _Ty, _Ky, _Args... >::x_mapthis_t
                   x_msg_subscriber_t< _Ty, _Ky, _Args... >::_S_map_object;

////////////////////////////////////////////////////////////////////////////////

#endif // __XMSG_SUBSCRIBER_H__
