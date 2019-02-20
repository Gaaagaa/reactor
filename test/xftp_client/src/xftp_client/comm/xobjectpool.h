/**
 * @file    xobjectpool.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xobjectpool.h
 * 创建日期：2019年02月09日
 * 文件标识：
 * 文件摘要：对象池模板类。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年02月09日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#ifndef __XOBJECTPOOL_H__
#define __XOBJECTPOOL_H__

#include <list>
#include <set>
#include <memory>
#include <mutex>

////////////////////////////////////////////////////////////////////////////////
// x_objectpool_t

/**
 * @class x_objectpool_t< _Ty, _ObjAllocator< _Ty > >
 * @brief 对象池模板类。
 */
template< typename _Ty, typename _Alloc = std::allocator< _Ty > >
class x_objectpool_t : protected _Alloc
{
    // common data types
public:
    using x_object_t     = _Ty;
    using x_object_ptr_t = x_object_t *;
    using x_allocator_t  = _Alloc;

private:
    using x_locker_t     = std::mutex;
    using x_autolock_t   = std::lock_guard< x_locker_t >;
    using x_obj_queue_t  = typename std::list< x_object_ptr_t >;
    using x_iterator_t   = typename std::list< x_object_ptr_t >::iterator;

    // constructor/destructor
public:
    x_objectpool_t(void)
    {

    }

    ~x_objectpool_t(void)
    {
        free_extra();
    }

    x_objectpool_t(x_objectpool_t && xobject) = delete;
    x_objectpool_t & operator=(x_objectpool_t && xobject) = delete;
    x_objectpool_t(const x_objectpool_t & xobject) = delete;
    x_objectpool_t & operator=(const x_objectpool_t & xobject) = delete;

    // public interfaces
public:
    /**********************************************************/
    /**
     * @brief 从对象池中申请对象。
     * 
     * @param [in ] xargs... : 对象的初始化（构造函数）参数列表。
     * 
     * @return x_object_ptr_t
     *         - 成功，返回 指向申请到的对象指针；
     *         - 失败，返回 nullptr 。
     */
    template< typename... _Args >
    x_object_ptr_t alloc(_Args && ... xargs)
    {
        x_object_ptr_t xobj_ptr = alloc_obj_mem();
        if (nullptr != xobj_ptr)
        {
            x_allocator_t::construct(xobj_ptr, std::forward< _Args >(xargs)...);
        }

        return xobj_ptr;
    }

    /**********************************************************/
    /**
     * @brief 将对象回收至对象池中。
     */
    bool recyc(x_object_ptr_t xobj_ptr)
    {
        if (nullptr != xobj_ptr)
        {
            x_autolock_t xautolock(m_sync_lock);

            x_allocator_t::destroy(xobj_ptr);
            m_que_recyc.push_back(xobj_ptr);

            return true;
        }

        return false;
    }

    /**********************************************************/
    /**
     * @brief 返回对象池缓存的对象数量。
     */
    inline size_t size(void) const
    {
        return m_que_recyc.size();
    }

    /**********************************************************/
    /**
     * @brief 释放掉所有已经回收的对象。
     */
    void free_extra(void)
    {
        x_autolock_t xautolock(m_sync_lock);

        x_iterator_t   xobj_itr;
        x_object_ptr_t xobj_ptr = nullptr;

        //======================================

        for (xobj_itr = m_que_recyc.begin(); xobj_itr != m_que_recyc.end(); ++xobj_itr)
        {
            xobj_ptr = *xobj_itr;

            if (nullptr != xobj_ptr)
            {
                x_allocator_t::deallocate(xobj_ptr, 1);
                xobj_ptr = nullptr;
            }
        }

        m_que_recyc.clear();

        //======================================
    }

    // internal invoking
private:
    /**********************************************************/
    /**
     * @brief 申请对象缓存。
     */
    x_object_ptr_t alloc_obj_mem(void)
    {
        x_object_ptr_t xobj_ptr = nullptr;

        x_autolock_t xautolock(m_sync_lock);

        if (m_que_recyc.empty())
        {
            xobj_ptr = x_allocator_t::allocate(1);
        }
        else
        {
            xobj_ptr = m_que_recyc.front();
            m_que_recyc.pop_front();
        }

        return xobj_ptr;
    }

    // data members
protected:
    x_locker_t     m_sync_lock;  ///< 数据同步锁
    x_obj_queue_t  m_que_recyc;  ///< 回收的对象集合
};

////////////////////////////////////////////////////////////////////////////////
// x_objectpool_ex_t

/**
 * @class x_objectpool_ex_t< _Ty, _ObjAllocator< _Ty > >
 * @brief 对象池模板类（扩展版本）。
 */
template< typename _Ty, typename _Alloc = std::allocator< _Ty > >
class x_objectpool_ex_t : protected _Alloc
{
    // common data types
public:
    using x_object_t     = _Ty;
    using x_object_ptr_t = x_object_t *;
    using x_allocator_t  = _Alloc;

private:
    using x_locker_t     = std::mutex;
    using x_autolock_t   = std::lock_guard< x_locker_t >;
    using x_obj_queue_t  = typename std::set< x_object_ptr_t >;
    using x_iterator_t   = typename std::set< x_object_ptr_t >::iterator;

    // constructor/destructor
public:
    x_objectpool_ex_t(void)
    {

    }

    ~x_objectpool_ex_t(void)
    {
        clear();
    }

    x_objectpool_ex_t(x_objectpool_ex_t && xobject) = delete;
    x_objectpool_ex_t & operator=(x_objectpool_ex_t && xobject) = delete;
    x_objectpool_ex_t(const x_objectpool_ex_t & xobject) = delete;
    x_objectpool_ex_t & operator=(const x_objectpool_ex_t & xobject) = delete;

    // public interfaces
public:
    /**********************************************************/
    /**
     * @brief 从对象池中申请对象。
     * 
     * @param [in ] xargs... : 对象的初始化（构造函数）参数列表。
     * 
     * @return x_object_ptr_t
     *         - 成功，返回 指向申请到的对象指针；
     *         - 失败，返回 nullptr 。
     */
    template< typename... _Args >
    x_object_ptr_t alloc(_Args && ... xargs)
    {
        x_object_ptr_t xobj_ptr = alloc_obj_mem();
        if (nullptr != xobj_ptr)
        {
            x_allocator_t::construct(xobj_ptr, std::forward< _Args >(xargs)...);
        }

        return xobj_ptr;
    }

    /**********************************************************/
    /**
     * @brief 将对象回收至对象池中。
     */
    bool recyc(x_object_ptr_t xobj_ptr)
    {
        if (nullptr != xobj_ptr)
        {
            x_autolock_t xautolock(m_sync_lock);

            if (m_que_alloc.find(xobj_ptr) == m_que_alloc.end())
            {
                return false;
            }

            x_allocator_t::destroy(xobj_ptr);
            m_que_recyc.insert(xobj_ptr);

            return true;
        }

        return false;
    }

    /**********************************************************/
    /**
     * @brief 判断指定对象是否为本对象池所分配出去的对象。
     */
    bool is_in_pool(x_object_ptr_t xobj_ptr)
    {
        if (nullptr != xobj_ptr)
        {
            x_autolock_t xautolock(m_sync_lock);
            return (m_que_alloc.find(xobj_ptr) != m_que_alloc.end());
        }

        return false;
    }

    /**********************************************************/
    /**
     * @brief 判断对象的有效性（属于本对象池分配，且未被回收）。
     */
    bool is_valid(x_object_ptr_t xobj_ptr)
    {
        if (nullptr != xobj_ptr)
        {
            x_autolock_t xautolock(m_sync_lock);
            return ((m_que_alloc.find(xobj_ptr) != m_que_alloc.end()) &&
                    (m_que_recyc.find(xobj_ptr) == m_que_recyc.end()));
        }

        return false;
    }

    /**********************************************************/
    /**
     * @brief 判断对象是否已经回收至池中。
     */
    bool is_recyc(x_object_ptr_t xobj_ptr)
    {
        if (nullptr != xobj_ptr)
        {
            x_autolock_t xautolock(m_sync_lock);
            return ((m_que_alloc.find(xobj_ptr) != m_que_alloc.end()) &&
                    (m_que_recyc.find(xobj_ptr) != m_que_recyc.end()));
        }

        return false;
    }

    /**********************************************************/
    /**
     * @brief 统计当前分配出去的对象数量。
     */
    inline size_t count(void) const
    {
        return (m_que_alloc.size() - m_que_recyc.size());
    }

    /**********************************************************/
    /**
     * @brief 返回对象池总共分配的对象数量。
     */
    inline size_t size(void) const
    {
        return m_que_alloc.size();
    }

    /**********************************************************/
    /**
     * @brief 释放掉所有已经回收的对象。
     */
    void free_extra(void)
    {
        x_autolock_t xautolock(m_sync_lock);

        x_iterator_t   xobj_itr;
        x_object_ptr_t xobj_ptr = nullptr;

        for (xobj_itr = m_que_recyc.begin(); xobj_itr != m_que_recyc.end(); ++xobj_itr)
        {
            xobj_ptr = *xobj_itr;
            m_que_alloc.erase(xobj_ptr);

            if (nullptr != xobj_ptr)
            {
                x_allocator_t::deallocate(xobj_ptr, 1);
                xobj_ptr = nullptr;
            }
        }

        m_que_recyc.clear();
    }

    /**********************************************************/
    /**
     * @brief 清空对象池。
     */
    void clear(void)
    {
        x_autolock_t xautolock(m_sync_lock);

        x_iterator_t   xobj_itr;
        x_object_ptr_t xobj_ptr = nullptr;

        //======================================

        for (xobj_itr = m_que_recyc.begin(); xobj_itr != m_que_recyc.end(); ++xobj_itr)
        {
            xobj_ptr = *xobj_itr;
            m_que_alloc.erase(xobj_ptr);

            if (nullptr != xobj_ptr)
            {
                x_allocator_t::deallocate(xobj_ptr, 1);
                xobj_ptr = nullptr;
            }
        }

        m_que_recyc.clear();

        //======================================

        for (xobj_itr = m_que_alloc.begin(); xobj_itr != m_que_alloc.end(); ++xobj_itr)
        {
            xobj_ptr = *xobj_itr;
            if (nullptr != xobj_ptr)
            {
                x_allocator_t::destroy(xobj_ptr);
                x_allocator_t::deallocate(xobj_ptr, 1);
                xobj_ptr = nullptr;
            }
        }

        m_que_alloc.clear();

        //======================================
    }

    // internal invoking
private:
    /**********************************************************/
    /**
     * @brief 申请对象缓存。
     */
    x_object_ptr_t alloc_obj_mem(void)
    {
        x_object_ptr_t xobj_ptr = nullptr;

        x_autolock_t xautolock(m_sync_lock);

        if (m_que_recyc.empty())
        {
            xobj_ptr = x_allocator_t::allocate(1);
            m_que_alloc.insert(xobj_ptr);
        }
        else
        {
            xobj_ptr = *m_que_recyc.begin();
            m_que_recyc.erase(m_que_recyc.begin());
        }

        return xobj_ptr;
    }

    // data members
protected:
    x_locker_t     m_sync_lock;  ///< 数据同步锁
    x_obj_queue_t  m_que_alloc;  ///< 所申请的对象集合
    x_obj_queue_t  m_que_recyc;  ///< 回收的对象集合
};

////////////////////////////////////////////////////////////////////////////////

#endif // __XOBJECTPOOL_H__
