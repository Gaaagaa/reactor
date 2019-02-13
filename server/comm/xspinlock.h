/**
 * @file    xspinlock.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xspinlock.h
 * 创建日期：2019年01月22日
 * 文件标识：
 * 文件摘要：简易的旋转锁类。
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

#ifndef __XSPINLOCK_H__
#define __XSPINLOCK_H__

#include <atomic>
#include <thread>

////////////////////////////////////////////////////////////////////////////////
// x_spinlock_t

/**
 * @class x_spinlock_t
 * @brief 简易的旋转锁类。
 */
class x_spinlock_t
{
    // constructor/destructor
public:
    x_spinlock_t(void)  { }
    ~x_spinlock_t(void) { }

    // public interfaces
public:
    /**********************************************************/
    /**
     * @brief 尝试加锁的操作接口。
     */
    bool try_lock(void)
    {
        return !m_xspin_flag.test_and_set(std::memory_order_acquire);
    }

    /**********************************************************/
    /**
     * @brief 加锁操作接口。
     */
    void lock(void)
    {
        while (m_xspin_flag.test_and_set(std::memory_order_acquire))
            std::this_thread::yield();
    }

    /**********************************************************/
    /**
     * @brief 解锁操作接口。
     */
    void unlock(void)
    {
        m_xspin_flag.clear(std::memory_order_release);
    }

    // data members
private:
    std::atomic_flag   m_xspin_flag = ATOMIC_FLAG_INIT;   ///< 旋转标志
};

////////////////////////////////////////////////////////////////////////////////
// x_autospin_t

/**
 * @class x_autospin_t
 * @brief 利用对象的 构造/析构 函数，辅助 x_spinlock_t 对象进行自动加锁解锁操作。
 */
template< class _Ty = x_spinlock_t >
class x_autospin_t
{
    // common data types
public:
    using x_locker_t = _Ty;

    // constructor/destructor
public:
    x_autospin_t(x_locker_t & xspin_lock)
        : m_xspin_lock(xspin_lock)
    {
        m_xspin_lock.lock();
    }

    ~x_autospin_t(void)
    {
        m_xspin_lock.unlock();
    }

    // data members
private:
    x_locker_t & m_xspin_lock;
};

////////////////////////////////////////////////////////////////////////////////

#endif // __XSPINLOCK_H__
