/**
 * @file    xmempool.h
 * <pre>
 * Copyright (c) 2015, Gaaagaa All rights reserved.
 *
 * 文件名称：xmempool.h
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

#ifndef __XMEMPOOL_H__
#define __XMEMPOOL_H__

#include <set>
#include <map>

////////////////////////////////////////////////////////////////////////////////

#if defined(_WIN64) || defined(__x84_64__)
    #ifndef __x64__
    #define __x64__ 1
    #endif // __x64__
#endif // defined(_WIN64) || defined(__x84_64__)

#ifdef __x64__
typedef unsigned long long msize_t;
#else // !__x64__
typedef unsigned int       msize_t;
#endif // __x64__

typedef unsigned char *    mblock_t;
typedef void *             mhandle_t;
typedef unsigned long long mtime_t;

////////////////////////////////////////////////////////////////////////////////
// x_mempool_t

/**
 * @class x_mempool_t
 * @brief 内存管理类（内存池）。
 */
class x_mempool_t
{
    // common data types
public:
    /**
     * @enum  emConstValue
     * @brief 对象内部使用到的常量值。
     */
    typedef enum emConstValue
    {
        ECV_ALLOC_SMALLER  = 1 * 1024,     ///< 较小的内存块分配粒度
        ECV_ALLOC_BIGGER   = 4 * 1024,     ///< 较大的内存块分配粒度
    } emConstValue;

protected:
    /** [ 内存块缓存，内存块大小 ] */
    typedef std::map< mblock_t, msize_t >   mapblock_t;

    /** [ 内存块大小，[内存块缓存，内存块回收的时间点] ] */
    typedef std::multimap< msize_t, std::pair< mblock_t, mtime_t > >   mmapblock_t;

    /** [ 内存块缓存 ] */
    typedef std::set< mblock_t > msetblock_t;

    /**
     * @class x_memlocker_t
     * @brief 数据锁操作句柄封装类。
     */
    class x_memlocker_t
    {
        // constructor/destructor
    public:
        x_memlocker_t(void);
        ~x_memlocker_t(void);

        // operator
    public:
        operator mhandle_t(void)       { return m_handle_lock; }
        operator mhandle_t(void) const { return m_handle_lock; }

        // data members
    protected:
        mhandle_t  m_handle_lock;   ///< 锁操作句柄
    };

    // constructor/destructor
public:
    x_mempool_t(void);
    ~x_mempool_t(void);

    // commom invoking
public:
    /**********************************************************/
    /**
     * @brief 按给定的（内存块）长度值，计算对齐后的数值。
     */
    static inline msize_t align_size(msize_t mst_size)
    {
        msize_t mst_xpart = (mst_size > ECV_ALLOC_BIGGER) ? ECV_ALLOC_BIGGER : ECV_ALLOC_SMALLER;
        return (mst_size + mst_xpart - 1) / mst_xpart * mst_xpart;
    }

    // public interfaces
public:
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
    mblock_t alloc(msize_t mst_size);

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
    mblock_t ralloc(mblock_t mbt_dptr, msize_t mst_size);

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
    mblock_t zalloc(msize_t mst_memb_nums, msize_t mst_memb_size);

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
    bool recyc(mblock_t mbt_dptr);

    /**********************************************************/
    /**
     * @brief 返回当前池总共分配的内存大小。
     */
    inline msize_t alloc_size(void) const { return m_st_alloc_size; }

    /**********************************************************/
    /**
     * @brief 返回当前池回收到已分配的内存大小。
     */
    inline msize_t recyc_size(void) const { return m_st_recyc_size; }

    /**********************************************************/
    /**
     * @brief 在回收队列中，释放掉超时未使用到的那些内存块（降低系统内存占用）。
     *
     * @param [in ] mtt_timeout : 超时时间（单位按 秒 计）；
     *                            若设置为 0，则可以释放掉所有处于回收状态的内存块。
     */
    void release_timeout_memblock(mtime_t mtt_timeout);

    /**********************************************************/
    /**
     * @brief 释放池空间（释放掉所有申请的内存块）。
     */
    void release_pool(void);

    // inner invoking methods
protected:
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
    mblock_t pop_recyc_block(msize_t mst_size);

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
    bool push_recyc_block(mblock_t memblock);

    // class data
protected:
    mutable x_memlocker_t  m_xmt_locker;    ///< 访问控制锁
    mapblock_t             m_map_alloc;     ///< 创建的内存块映射表
    msetblock_t            m_mset_recyc;    ///< 回收的内存块集合表（用于防止重复回收相同的内存块）
    mmapblock_t            m_mmap_recyc;    ///< 回收的内存块映射表（明确内存块的大小和回收时间点信息）
    msize_t                m_st_alloc_size; ///< 总共申请的内存大小
    msize_t                m_st_recyc_size; ///< 未使用的（处于回收状态）内存大小
};

////////////////////////////////////////////////////////////////////////////////

#endif // __XMEMPOOL_H__
