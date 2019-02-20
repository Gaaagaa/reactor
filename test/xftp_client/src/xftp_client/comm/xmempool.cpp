/**
 * @file    xmempool.cpp
 * <pre>
 * Copyright (c) 2015, Gaaagaa All rights reserved.
 *
 * 文件名称：xmempool.cpp
 * 创建日期：2015年04月24日
 * 文件标识：
 * 文件摘要：内存池管理，实现多类型的内存块（不同内存块大小的）内存池。
 *
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2015年04月24日
 * 版本摘要：
 *
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#include "xmempool.h"

#include <string.h>
#include <time.h>
#include <malloc.h>
#include <assert.h>

////////////////////////////////////////////////////////////////////////////////

#if 0

#ifndef XLOCK_IMPLEMENTATION
#define XLOCK_IMPLEMENTATION

#ifdef _MSC_VER
#include <Windows.h>
#define XLOCK_TYPE            CRITICAL_SECTION
#define XLOCK_INIT(xlock)     InitializeCriticalSection((xlock))
#define XLOCK_FREE(xlock)     DeleteCriticalSection((xlock))
#define XLOCK_ENTER(xlock)    EnterCriticalSection((xlock))
#define XLOCK_LEAVE(xlock)    LeaveCriticalSection((xlock))
#else // !_MSC_VER
#include <pthread.h>
#define XLOCK_TYPE            pthread_mutex_t
#define XLOCK_INIT(xlock)     pthread_mutex_init((xlock), NULL)
#define XLOCK_FREE(xlock)     pthread_mutex_destroy((xlock))
#define XLOCK_ENTER(xlock)    pthread_mutex_lock((xlock))
#define XLOCK_LEAVE(xlock)    pthread_mutex_unlock((xlock))
#endif // _MSC_VER

#endif // XLOCK_IMPLEMENTATION

/**
 * @class x_memlock_t
 * @brief 数据操作锁。
 */
class x_memlock_t
{
    // constructor/destructor
public:
    x_memlock_t(void)  { XLOCK_INIT(&m_xlock); }
    ~x_memlock_t(void) { XLOCK_FREE(&m_xlock); }

    // public interfaces
public:
    /**********************************************************/
    /**
     * @brief 加锁操作。
     */
    inline void lock(void)   { XLOCK_ENTER(&m_xlock); }

    /**********************************************************/
    /**
     * @brief 解锁操作。
     */
    inline void unlock(void) { XLOCK_LEAVE(&m_xlock); }

    // data members
protected:
    XLOCK_TYPE   m_xlock;   ///< 锁
};

#else

#include <mutex>

/**
 * @class x_memlock_t
 * @brief 数据操作锁。
 */
class x_memlock_t : public std::mutex
{

};

#endif

/**
* @class x_memautolock_t
* @brief 实现自动锁操作（利用 构造/析构 函数，调用锁对象的 加锁/解锁 操作）。
*/
class x_memautolock_t
{
    // constructor/destructor
public:
    x_memautolock_t(x_memlock_t * xlock_ptr)
    {
        m_xlock_ptr = xlock_ptr;
        m_xlock_ptr->lock();
    }

    ~x_memautolock_t(void)
    {
        m_xlock_ptr->unlock();
    }

    // class data
protected:
    x_memlock_t * m_xlock_ptr;  ///< 自动锁的目标操作对象
};

////////////////////////////////////////////////////////////////////////////////
// x_mempool_t::x_memlocker_t

//====================================================================

// 
// x_mempool_t::x_memlocker_t : constructor/destructor
// 

x_mempool_t::x_memlocker_t::x_memlocker_t(void)
{
    m_handle_lock = (mhandle_t)(new x_memlock_t());
}

x_mempool_t::x_memlocker_t::~x_memlocker_t(void)
{
    if (NULL != m_handle_lock)
    {
        delete ((x_memlock_t *)m_handle_lock);
        m_handle_lock = NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////
// x_mempool_t

//====================================================================

// 
// x_mempool_t : commom invoking
// 

//====================================================================

// 
// x_mempool_t : constructor/destructor
// 

x_mempool_t::x_mempool_t(void)
{
    m_st_alloc_size = 0;
    m_st_recyc_size = 0;
}

x_mempool_t::~x_mempool_t(void)
{
    release_pool();
}

//====================================================================

// 
// x_mempool_t : public interfaces
// 

/**********************************************************/
/**
 * @brief 申请内存块。
 *
 * @param [in ] mst_size : 申请的内存块长度。
 *
 * @return mblock_t
 *         - 成功，返回 内存块地址；
 *         - 失败，返回 NULL。
 */
mblock_t x_mempool_t::alloc(msize_t mst_size)
{
    mblock_t memblock = NULL;

    if ((mst_size <= 0) || (mst_size > 0x7FFFFFFF))
    {
        return NULL;
    }

    x_memautolock_t xautolock((x_memlock_t *)(mhandle_t)m_xmt_locker);

    msize_t mst_xsize = align_size(mst_size);

    do 
    {
        memblock = pop_recyc_block(mst_xsize);
        if (NULL != memblock)
            break;

        memblock = (mblock_t)malloc(mst_xsize);
        if (NULL == memblock)
            break;

        m_map_alloc.insert(std::pair< mblock_t, msize_t >(memblock, mst_xsize));
        m_st_alloc_size += mst_xsize;

    } while (0);

    return memblock;
}

/**********************************************************/
/**
 * @brief 扩展内存块空间。
 *
 * @param [in ] mbt_dptr : 操作的内存块（该内存块必须是本对象所管理）。
 * @param [in ] mst_size : 扩展的空间大小。
 *
 * @return mblock_t
 *         - 成功，返回 内存块地址；
 *         - 失败，返回 NULL。
 */
mblock_t x_mempool_t::ralloc(mblock_t mbt_dptr, msize_t mst_size)
{
    mblock_t memblock = NULL;

    if ((mst_size <= 0) || (mst_size > 0x7FFFFFFF))
    {
        return NULL;
    }

    x_memautolock_t xautolock((x_memlock_t *)(mhandle_t)m_xmt_locker);

    msize_t mst_xsize = align_size(mst_size);

    do 
    {
        mapblock_t::iterator itfind = m_map_alloc.find(mbt_dptr);
        if (itfind == m_map_alloc.end())
        {
            break;
        }

        if (itfind->second >= mst_xsize)
        {
            memblock = mbt_dptr;
            break;
        }

        memblock = pop_recyc_block(mst_xsize);
        if (NULL == memblock)
            memblock = (mblock_t)malloc(mst_xsize);
        if (NULL == memblock)
            break;

        memcpy(memblock, mbt_dptr, itfind->second);
        push_recyc_block(mbt_dptr);

        m_map_alloc.insert(std::pair< mblock_t, msize_t >(memblock, mst_xsize));
        m_st_alloc_size += mst_xsize;

    } while (0);

    return memblock;
}

/**********************************************************/
/**
 * @brief （按内存单元方式）申请内存块，并执行清零操作。
 *
 * @param [in ] mst_memb_nums : 内存单元数量。
 * @param [in ] mst_memb_size : 内存单元大小。
 *
 * @return mblock_t
 *         - 成功，返回 内存块地址；
 *         - 失败，返回 NULL。
 */
mblock_t x_mempool_t::zalloc(msize_t mst_memb_nums, msize_t mst_memb_size)
{
    msize_t mst_zalloc_size = mst_memb_nums * mst_memb_size;
    mblock_t memblock = NULL;

    if ((mst_zalloc_size <= 0) || (mst_zalloc_size > 0x7FFFFFFF))
    {
        return NULL;
    }

    x_memautolock_t xautolock((x_memlock_t *)(mhandle_t)m_xmt_locker);

    msize_t mst_xsize = align_size(mst_zalloc_size);

    do 
    {
        memblock = pop_recyc_block(mst_xsize);
        if (NULL != memblock)
        {
            memset(memblock, 0, mst_xsize);
            break;
        }

        memblock = (mblock_t)calloc(mst_xsize, 1);
        if (NULL == memblock)
            break;

        m_map_alloc.insert(std::pair< mblock_t, msize_t >(memblock, mst_xsize));
        m_st_alloc_size += mst_xsize;

    } while (0);

    return memblock;
}

/**********************************************************/
/**
 * @brief 回收内存块。
 *
 * @param [in ] mbt_dptr : 内存块地址。
 *
 * @return bool
 *         - 成功，返回 true；
 *         - 失败，返回 false，表示所要回收的内存块并非池管理的内存块。
 */
bool x_mempool_t::recyc(mblock_t mbt_dptr)
{
    x_memautolock_t xautolock((x_memlock_t *)(mhandle_t)m_xmt_locker);
    return push_recyc_block(mbt_dptr);
}

/**********************************************************/
/**
 * @brief 在回收队列中，释放掉超时未使用到的那些内存块（降低系统内存占用）。
 *
 * @param [in ] mtt_timeout : 超时时间（单位按 秒 计）；
 *                            若设置为 0，则可以释放掉所有处于回收状态的内存块。
 */
void x_mempool_t::release_timeout_memblock(mtime_t mtt_timeout)
{
    x_memautolock_t xautolock((x_memlock_t *)(mhandle_t)m_xmt_locker);

    mtime_t mtt_now = (mtime_t)time(NULL);

    mmapblock_t::iterator itmmap = m_mmap_recyc.begin();
    for (; itmmap != m_mmap_recyc.end(); )
    {
        if (mtt_now >= (mtt_timeout + itmmap->second.second))
        {
            m_st_alloc_size -= itmmap->first;
            m_st_recyc_size -= itmmap->first;

            mblock_t mbt_dptr = itmmap->second.first;
            m_mset_recyc.erase(mbt_dptr);
            m_mmap_recyc.erase(itmmap++);
            m_map_alloc.erase(mbt_dptr);

            if (NULL != mbt_dptr)
            {
                free(mbt_dptr);
            }
        }
        else
        {
            ++itmmap;
        }
    }
}

/**********************************************************/
/**
 * @brief 释放池空间（释放掉所有申请的内存块）。
 */
void x_mempool_t::release_pool(void)
{
    x_memautolock_t xautolock((x_memlock_t *)(mhandle_t)m_xmt_locker);

    mapblock_t::iterator itmap = m_map_alloc.begin();
    for (; itmap != m_map_alloc.end(); ++itmap)
    {
        mblock_t mbt_dptr = itmap->first;

#ifdef MP_DEBUG_VIEW
        m_mset_recyc.erase(mbt_dptr);
#endif // MP_DEBUG_VIEW

        if (NULL != mbt_dptr)
        {
            free(mbt_dptr);
        }
    }

#ifdef MP_DEBUG_VIEW
    msetblock_t::iterator itmset = m_mset_recyc.begin();
    for (; itmset != m_mset_recyc.end(); ++itmset)
    {
        TRACE("memory leak in address : %08X\n", (msize_t)*itmset);
    }
#endif // MP_DEBUG_VIEW

    m_map_alloc.clear();
    m_mset_recyc.clear();
    m_mmap_recyc.clear();

    m_st_alloc_size = 0;
    m_st_recyc_size = 0;
}

//====================================================================

// 
// x_mempool_t : inner invoking methods
// 

/**********************************************************/
/**
 * @brief 按指定的大小从回收的内存块中取出内存块。
 *
 * @param [in ] mst_size : 指定的内存块大小。
 *
 * @return mblock_t
 *         - 成功，返回 内存块 地址；
 *         - 失败，返回 NULL。
 */
mblock_t x_mempool_t::pop_recyc_block(msize_t mst_size)
{
    mblock_t mbt_dptr = NULL;

    do 
    {
        if (m_mmap_recyc.empty())
            break;

        mmapblock_t::iterator itfind = m_mmap_recyc.find(mst_size);
        if (itfind == m_mmap_recyc.end())
            break;

        mbt_dptr = itfind->second.first;

        m_mset_recyc.erase(mbt_dptr);
        m_mmap_recyc.erase(itfind);

        m_st_recyc_size -= mst_size;

    } while (0);

    return mbt_dptr;
}

/**********************************************************/
/**
 * @brief 将内存块压入回收的内存块管理队列中。
 *
 * @param [in ] memblock : 回收的内存块。
 *
 * @return bool
 *         - 成功，返回 true；
 *         - 失败，返回 false，表示所要回收的内存块并非池管理的内存块。
 */
bool x_mempool_t::push_recyc_block(mblock_t mbt_dptr)
{
    if (NULL == mbt_dptr)
    {
        return false;
    }

    // 确认是否为申请过的内存块
    mapblock_t::iterator itfind = m_map_alloc.find(mbt_dptr);
    if (itfind == m_map_alloc.end())
    {
        return false;
    }

    // 若内存块未在回收集合表中，则可执行回收操作（防止重复回收相同的内存块）
    if (m_mset_recyc.find(mbt_dptr) == m_mset_recyc.end())
    {
        m_mset_recyc.insert(mbt_dptr);
        m_mmap_recyc.insert(
            std::pair< msize_t, mmapblock_t::mapped_type >(
                itfind->second,
                mmapblock_t::mapped_type(mbt_dptr, (mtime_t)time(NULL))));
        m_st_recyc_size += itfind->second;
    }

    return true;
}
