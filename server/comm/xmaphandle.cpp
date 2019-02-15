/**
 * @file    xmaphandle.cpp
 * <pre>
 * Copyright (c) 2015, Gaaagaa All rights reserved.
 * 
 * 文件名称：xmaphandle.cpp
 * 创建日期：2015年04月10日
 * 文件标识：
 * 文件摘要：句柄映射表（支持多线程并发访问操作的 哈希映射表，即 分段锁哈希表）。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2015年04月10日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#include "xmaphandle.h"

#ifdef _MSC_VER
#include <windows.h>
#else // !_MSC_VER
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>

#define gettid() syscall(__NR_gettid)

#endif // _MSC_VER

#ifndef ENABLE_XASSERT
#if ((defined _DEBUG) || (defined DEBUG))
#define ENABLE_XASSERT 1
#else // !((defined _DEBUG) || (defined DEBUG))
#define ENABLE_XASSERT 0
#endif // ((defined _DEBUG) || (defined DEBUG))
#endif // ENABLE_XASSERT

#ifndef XASSERT
#if ENABLE_XASSERT
#include <cassert>
#define XASSERT(xptr)    assert(xptr)
#else // !ENABLE_XASSERT
#define XASSERT(xptr)
#endif // ENABLE_XASSERT
#endif // XASSERT

#ifndef ENABLE_MAPTBL_VALID
#define ENABLE_MAPTBL_VALID  0
#endif // ENABLE_MAPTBL_VALID

////////////////////////////////////////////////////////////////////////////////

//====================================================================

// 
// common data types
// 

#define MAPTBL_ITEM_GSTEP_SIZE  4        ///< 子项中映射的句柄队列的增长大小
#define MAPTBL_ITEM_DEPTH_SIZE  32768    ///< 子项中映射的句柄队列的最大深度

/**
 * @struct maptbl_pair
 * @brief  映射表的子项映射对信息。
 */
typedef struct maptbl_pair
{
    x_size_t             xst_itemkey;  ///< 子项索引键
    x_handle_t           xht_handler;  ///< 子项映射句柄
    struct maptbl_pair * xpt_nextptr;  ///< 指向下一个 maptbl_pair
} maptbl_pair;

/**
 * @struct maptbl_item
 * @brief  映射表的子项描述信息。
 */
typedef struct maptbl_item
{
    x_uint32_t    xut_spin_lock;  ///< 旋转锁标识
    maptbl_pair * xht_item_dptr;  ///< 子项中映射的句柄队列
} maptbl_item;

/**
 * @struct maptbl_table
 * @brief  映射表的内部数据描述信息。
 */
typedef struct maptbl_table
{
    x_uint32_t    xut_this_size;   ///< 用于检测操作，其值固定为 sizeof(maptbl_table)
    x_uint32_t    xut_itemcount;   ///< 映射表中当前映射的子项数量
    x_size_t      xst_capacity;    ///< 映射表的索引目录容量
    maptbl_item * xmap_item_dptr;  ///< 子项索引目录队列
} maptbl_table;

//====================================================================

// 
// inner invoking
// 

/**********************************************************/
/**
 * @brief 系统时钟（单位为 ms 毫秒，1970年1月1日到现在的时间）。
 */
static x_uint64_t maptbl_time_tick64(void)
{
#ifdef _MSC_VER
    // 1601 ~ 1970 年之间的时间 百纳秒数
    const x_uint64_t NS100_1970 = 116444736000000000ULL;

    FILETIME       xfilev_time;
    ULARGE_INTEGER xvalue_time;

    GetSystemTimeAsFileTime(&xfilev_time);
    xvalue_time.LowPart  = xfilev_time.dwLowDateTime;
    xvalue_time.HighPart = xfilev_time.dwHighDateTime;

    return ((xvalue_time.QuadPart - NS100_1970) / 10000ULL);
#else // !_MSC_VER
    struct timeval tmval;
    gettimeofday(&tmval, X_NULL);

    return (x_uint64_t)(tmval.tv_sec * 1000ULL + tmval.tv_usec / 1000ULL);
#endif // _MSC_VER
}

/**********************************************************/
/**
 * @brief 对当前线程执行 Sleep() 操作，时间单位为 毫秒。
 */
static inline x_void_t maptbl_msleep(x_uint32_t xut_mseconds)
{
#ifdef _MSC_VER
    Sleep(xut_mseconds);
#else // !_MSC_VER
    usleep(xut_mseconds * 1000);
#endif // _MSC_VER
}

/**********************************************************/
/**
 * @brief 获取当前线程 ID 值。
 */
static inline x_uint32_t maptbl_tid(void)
{
#ifdef _MSC_VER
    return (x_uint32_t)GetCurrentThreadId();
#else // !_MSC_VER
    return (x_uint32_t)gettid();
#endif // _MSC_VER
}

/**********************************************************/
/**
 * @brief 变量与指定值比较相等则赋新值的原子操作。
 * 
 * @param [in,out] xut_dest     : 目标操作的变量值。
 * @param [in    ] xut_exchange : 比较成功后所赋的新值。
 * @param [in    ] xut_compare  : 指定比较的数值。
 * 
 * @return x_uint32_t
 *         - 返回 xut_dest 的原始值。
 */
static inline x_uint32_t maptbl_atomic_cmpxchg(x_uint32_t * xut_dest, x_uint32_t xut_exchange, x_uint32_t xut_compare)
{
#ifdef _MSC_VER
    return _InterlockedCompareExchange((volatile x_uint32_t *)xut_dest, xut_exchange, xut_compare);
#else // !_MSC_VER
    return __sync_val_compare_and_swap(xut_dest, xut_compare, xut_exchange);
#endif // _MSC_VER
}

/**********************************************************/
/**
 * @brief 变量与指定值执行加法操作的原子操作。
 * 
 * @param [in,out] xut_dest  : 目标操作的变量值。
 * @param [in    ] xut_value : 执行加法操作的指定值。
 * 
 * @return x_uint32_t
 *         - 返回 xut_dest 的原始值。
 */
static inline x_uint32_t maptbl_atomic_add(x_uint32_t * xut_dest, x_uint32_t xut_value)
{
#ifdef _MSC_VER
    return _InterlockedExchangeAdd((volatile x_uint32_t *)xut_dest, xut_value);
#else // !_MSC_VER
    return __sync_fetch_and_add(xut_dest, xut_value);
#endif // _MSC_VER
}

/**********************************************************/
/**
 * @brief 变量与指定值执行减法操作的原子操作。
 * 
 * @param [in,out] xut_dest  : 目标操作的变量值。
 * @param [in    ] xut_value : 执行减法操作的指定值。
 * 
 * @return x_uint32_t
 *         - 返回 xut_dest 的原始值。
 */
static inline x_uint32_t maptbl_atomic_sub(x_uint32_t * xut_dest, x_uint32_t xut_value)
{
#ifdef _MSC_VER
    return _InterlockedExchangeAdd((volatile x_uint32_t *)xut_dest, ~xut_value + 1);
#else // !_MSC_VER
    return __sync_fetch_and_sub(xut_dest, xut_value);
#endif // _MSC_VER
}

/**********************************************************/
/**
 * @brief 校验 maptbl_table 指针对象的有效性。
 * 
 * @param [in ] xmap_hptr : maptbl_table 对象。
 * 
 * @return x_bool_t
 *         - 返回 X_TRUE，对象校验有效；
 *         - 返回 X_FALSE，对象校验无效。
 */
static inline x_bool_t maptbl_valid(maptbl_table * xmap_hptr)
{
    if (X_NULL == xmap_hptr)
    {
        return X_FALSE;
    }

    if (sizeof(maptbl_table) != xmap_hptr->xut_this_size)
    {
        return X_FALSE;
    }

    if ((xmap_hptr->xst_capacity <= 0) || (X_NULL == xmap_hptr->xmap_item_dptr))
    {
        return X_FALSE;
    }

    return X_TRUE;
}

/**********************************************************/
/**
 * @brief 对映射表指定的目录索引号进行锁定操作（与 maptbl_unlock_index() 实现对应操作）。
 * 
 * @param [in ] xmap_hptr   : 映射表对象。
 * @param [in ] xst_index   : 指定锁定的目录索引号。
 * @param [in ] xitem_dptr  : 锁定操作成功返回的子项。
 * @param [in ] xst_timeout : 操作超时时间（单位为 ms 毫秒）。。
 * 
 * @return x_int32_t
 *         - 返回 0，操作成功；
 *         - 返回 -1，操作超时。
 */
static x_int32_t maptbl_lock_index(maptbl_table * xmap_hptr, x_size_t xst_index, maptbl_item ** xitem_dptr, x_size_t xst_timeout)
{
    x_uint64_t xst_vtm = maptbl_time_tick64();
    x_uint64_t xst_etm = xst_vtm + (x_uint64_t)xst_timeout;
    x_uint32_t xut_tid = maptbl_tid() + 1;

    maptbl_item * xmap_iptr = xmap_hptr->xmap_item_dptr + xst_index;

    while (xst_vtm <= xst_etm)
    {
        if (0 == maptbl_atomic_cmpxchg(&xmap_iptr->xut_spin_lock, xut_tid, 0))
        {
            *xitem_dptr = xmap_iptr;
            return 0;
        }

        maptbl_msleep(1);
        xst_vtm = maptbl_time_tick64();
    }

    return -1;
}

/**********************************************************/
/**
 * @brief 对映射表指定的目录索引号进行锁定状态检测。
 * 
 * @param [in ] xmap_hptr  : 映射表对象。
 * @param [in ] xst_index  : 指定锁定的目录索引号。
 * @param [in ] xitem_dptr : 锁定操作成功返回的子项（若入参为 X_NULL，则忽略该操作）。
 * 
 * @return x_int32_t
 *         - 返回  0，表示当前线程对该索引号处于占用状态；
 *         - 返回 -1，表示当前线程对该索引号处于未占用状态。
 */
static x_int32_t maptbl_trylock_index(maptbl_table * xmap_hptr, x_size_t xst_index, maptbl_item ** xitem_dptr)
{
    x_uint32_t xut_tid = maptbl_tid() + 1;

    maptbl_item * xmap_iptr = xmap_hptr->xmap_item_dptr + xst_index;

    if (xut_tid == maptbl_atomic_cmpxchg(&xmap_iptr->xut_spin_lock, xut_tid, xut_tid))
    {
        *xitem_dptr = xmap_iptr;
        return 0;
    }

    return -1;
}

/**********************************************************/
/**
 * @brief 对映射表指定的目录索引号进行解锁操作（与 maptbl_lock_index() 实现对应操作）。
 * 
 * @param [in ] xmap_hptr : 映射表对象。
 * @param [in ] xst_index : 指定解锁的目录索引号。
 * 
 * @return x_int32_t
 *         - 返回 0，操作成功；
 *         - 返回 -1，非法操作。
 */
static x_int32_t maptbl_unlock_index(maptbl_table * xmap_hptr, x_size_t xst_index)
{
    x_uint32_t xut_tid = maptbl_tid() + 1;

    maptbl_item * xmap_iptr = xmap_hptr->xmap_item_dptr + xst_index;

    if (xut_tid != maptbl_atomic_cmpxchg(&xmap_iptr->xut_spin_lock, 0, xut_tid))
    {
        return -1;
    }

    return 0;
}

/**********************************************************/
/**
 * @brief 向 子项映射对的存储链表（结尾）插入 子项映射对。
 * 
 * @param [in ] xmap_iptr   : 子项映射对的向量集描述信息。
 * @param [in ] xst_itemkey : 子项映射对的索引键。
 * @param [in ] xht_handler : 子项映射对的映射句柄。
 * 
 * @return x_bool_t
 *         - 插入失败，返回 X_FALSE；
 *         - 插入成功，返回 X_TRUE。
 */
static x_bool_t maptbl_pair_insert(maptbl_item * xmap_iptr, x_size_t xst_itemkey, x_handle_t xht_handler)
{
    maptbl_pair * xht_hptr = X_NULL;
    maptbl_pair * xht_nptr = xmap_iptr->xht_item_dptr;

    XASSERT(MAPTBL_INVALID_ITEMKEY != xst_itemkey);
    XASSERT(MAPTBL_INVALID_HANDLER != xht_handler);

    xht_hptr = (maptbl_pair *)malloc(sizeof(maptbl_pair));
    if (X_NULL == xht_hptr)
    {
        XASSERT(X_FALSE);
        return X_FALSE;
    }

    xht_hptr->xst_itemkey = xst_itemkey;
    xht_hptr->xht_handler = xht_handler;
    xht_hptr->xpt_nextptr = X_NULL;

    if (X_NULL == xmap_iptr->xht_item_dptr)
    {
        xmap_iptr->xht_item_dptr = xht_hptr;
    }
    else
    {
        while (X_NULL != xht_nptr)
        {
            if (X_NULL == xht_nptr->xpt_nextptr)
            {
                xht_nptr->xpt_nextptr = xht_hptr;
                break;
            }
            xht_nptr = xht_nptr->xpt_nextptr;
        }
    }

    return X_TRUE;
}

/**********************************************************/
/**
 * @brief 按给定的子项索引键，查找 子项映射对。
 * 
 * @param [in ] xmap_iptr   : 子项映射对的存储链表描述信息。
 * @param [in ] xst_itemkey : 给定的子项索引键。
 * 
 * @return maptbl_pair *
 *         - 查找失败，返回 X_NULL；
 *         - 查找成功，返回 子项映射对。
 */
static maptbl_pair * maptbl_pair_find(maptbl_item * xmap_iptr, x_size_t xst_itemkey)
{
    maptbl_pair * xht_nptr = xmap_iptr->xht_item_dptr;

    while (X_NULL != xht_nptr)
    {
        if (xst_itemkey == xht_nptr->xst_itemkey)
            return xht_nptr;
        xht_nptr = xht_nptr->xpt_nextptr;
    }

    return X_NULL;
}

/**********************************************************/
/**
 * @brief 按给定的子项索引键，删除 子项映射对。
 * 
 * @param [in ] xmap_iptr   : 子项映射对的存储链表。
 * @param [in ] xst_itemkey : 给定的子项索引键。
 * 
 * @return x_bool_t
 *         - 删除失败，返回 X_FALSE；
 *         - 删除成功，返回 X_TRUE。
 */
static x_bool_t maptbl_pair_erase(maptbl_item * xmap_iptr, x_size_t xst_itemkey)
{
    maptbl_pair * xht_hptr = X_NULL;
    maptbl_pair * xht_nptr = xmap_iptr->xht_item_dptr;

    while (X_NULL != xht_nptr)
    {
        if (xst_itemkey == xht_nptr->xst_itemkey)
        {
            if (X_NULL == xht_hptr)
                xmap_iptr->xht_item_dptr = xht_nptr->xpt_nextptr;
            else
                xht_hptr->xpt_nextptr = xht_nptr->xpt_nextptr;
            free(xht_nptr);
            return X_TRUE;
        }

        xht_hptr = xht_nptr;
        xht_nptr = xht_nptr->xpt_nextptr;
    }

    return X_FALSE;
}

/**********************************************************/
/**
 * @brief 对 子项映射对的存储链表 执行遍历操作。
 * 
 * @param [in ] xmap_iptr   : 子项映射对的存储链表。
 * @param [out] xbt_keepcbk : 标识是否可继续执行后续遍历回调操作。
 * @param [in ] xht_maptbl  : 映射表的操作句柄。
 * @param [in ] xfunc_ptr   : 遍历操作的回调函数接口。
 * @param [in ] xht_context : 遍历操作的回调上下文标识信息。
 * 
 * @return x_size_t
 *         - 返回 执行回调子项的数量。
 */
static x_size_t maptbl_pair_trav(maptbl_item * xmap_iptr,
                                 x_bool_t * xbt_keepcbk,
                                 x_handle_t xht_maptbl,
                                 maptbl_trav_callback xfunc_ptr,
                                 x_handle_t xht_context)
{
    maptbl_pair  * xht_hptr  = X_NULL;
    maptbl_pair  * xht_nptr  = xmap_iptr->xht_item_dptr;
    maptbl_table * xmap_hptr = (maptbl_table *)xht_maptbl;
    x_size_t       xst_count = 0;

    *xbt_keepcbk = X_TRUE;

    while ((X_NULL != xht_nptr) && *xbt_keepcbk)
    {
        xst_count += 1;
        *xbt_keepcbk = xfunc_ptr(xht_maptbl, xht_nptr->xst_itemkey, &xht_nptr->xht_handler, xht_context);

        if (MAPTBL_INVALID_HANDLER == xht_nptr->xht_handler)
        {
            if (X_NULL == xht_hptr)
            {
                xmap_iptr->xht_item_dptr = xht_nptr->xpt_nextptr;
                free(xht_nptr);
                xht_nptr = xmap_iptr->xht_item_dptr;
            }
            else
            {
                xht_hptr->xpt_nextptr = xht_nptr->xpt_nextptr;
                free(xht_nptr);
                xht_nptr = xht_hptr->xpt_nextptr;
            }

            maptbl_atomic_sub(&xmap_hptr->xut_itemcount, 1);
        }
        else
        {
            xht_hptr = xht_nptr;
            xht_nptr = xht_nptr->xpt_nextptr;
        }
    }

    return xst_count;
}

/**********************************************************/
/**
 * @brief 对 子项映射对的存储链表 执行遍历操作。
 * 
 * @param [in ] xmap_iptr   : 子项映射对的存储链表。
 * @param [in ] xst_nextkey : 以此键值的基准节点的下一节点作为遍历操作的起始节点（若为 MAPTBL_INVALID_ITEMKEY，则从起始节点开始遍历操作）。
 * @param [out] xbt_keepcbk : 标识是否可继续执行后续遍历回调操作。
 * @param [in ] xht_maptbl  : 映射表的操作句柄。
 * @param [in ] xfunc_ptr   : 遍历操作的回调函数接口。
 * @param [in ] xht_context : 遍历操作的回调上下文标识信息。
 * 
 * @return x_size_t
 *         - 返回 执行回调子项的数量。
 */
static x_size_t maptbl_pair_trav_next(maptbl_item * xmap_iptr,
                                      x_size_t xst_nextkey,
                                      x_bool_t * xbt_keepcbk,
                                      x_handle_t xht_maptbl,
                                      maptbl_trav_callback xfunc_ptr,
                                      x_handle_t xht_context)
{
    maptbl_pair  * xht_hptr  = X_NULL;
    maptbl_pair  * xht_nptr  = xmap_iptr->xht_item_dptr;
    maptbl_table * xmap_hptr = (maptbl_table *)xht_maptbl;
    x_size_t       xst_count = 0;

    if (MAPTBL_INVALID_ITEMKEY != xst_nextkey)
    {
        while (X_NULL != xht_nptr)
        {
            if (xst_nextkey == xht_nptr->xst_itemkey)
                break;
            xht_hptr = xht_nptr;
            xht_nptr = xht_nptr->xpt_nextptr;
        }
    }

    *xbt_keepcbk = X_TRUE;

    while ((X_NULL != xht_nptr) && *xbt_keepcbk)
    {
        xst_count += 1;
        *xbt_keepcbk = xfunc_ptr(xht_maptbl, xht_nptr->xst_itemkey, &xht_nptr->xht_handler, xht_context);

        if (MAPTBL_INVALID_HANDLER == xht_nptr->xht_handler)
        {
            if (X_NULL != xht_hptr)
            {
                xht_hptr->xpt_nextptr = xht_nptr->xpt_nextptr;
                free(xht_nptr);
                xht_nptr = xht_hptr->xpt_nextptr;
            }
            else
            {
                xmap_iptr->xht_item_dptr = xht_nptr->xpt_nextptr;
                free(xht_nptr);
                xht_nptr = xmap_iptr->xht_item_dptr;
            }

            maptbl_atomic_sub(&xmap_hptr->xut_itemcount, 1);
        }
        else
        {
            xht_hptr = xht_nptr;
            xht_nptr = xht_nptr->xpt_nextptr;
        }
    }

    return xst_count;
}

/**********************************************************/
/**
 * @brief 清除 子项映射对的存储链表 中的所有子项节点。
 */
static x_void_t maptbl_pair_clear(maptbl_item * xmap_iptr)
{
    maptbl_pair * xht_hptr = X_NULL;
    maptbl_pair * xht_nptr = xmap_iptr->xht_item_dptr;

    while (X_NULL != xht_nptr)
    {
        xht_hptr = xht_nptr->xpt_nextptr;
        free(xht_nptr);
        xht_nptr = xht_hptr;
    }

    xmap_iptr->xht_item_dptr = X_NULL;
}

//====================================================================

// 
// public interfaces
// 

#if ENABLE_MAPTBL_VALID

#define MAPTBL_XVOID

#define MAPTBL_VALID(xmap_hptr, xreturn)  \
    do                                    \
    {                                     \
        if (!maptbl_valid(xmap_hptr))     \
        {                                 \
            return xreturn;               \
        }                                 \
    } while (0)                           \

#else // !ENABLE_MAPTBL_VALID

#define MAPTBL_VALID(xmap_hptr, xreturn)

#endif // ENABLE_MAPTBL_VALID

/**********************************************************/
/**
 * @brief 创建 映射表 对象。
 * 
 * @param [in ] xst_capacity : 映射表的索引容量（这并非 子项容纳数量 上限）。
 * 
 * @return x_handle_t
 *         - 成功，返回 映射表的操作句柄；
 *         - 失败，返回 X_NULL。
 */
x_handle_t maptbl_create(x_size_t xst_capacity)
{
    maptbl_table * xmap_hptr = X_NULL;

    if (xst_capacity <= 0)
    {
        return X_NULL;
    }

    xmap_hptr = (maptbl_table *)malloc(sizeof(maptbl_table));
    if (X_NULL == xmap_hptr)
    {
        return X_NULL;
    }

    xmap_hptr->xut_this_size  = sizeof(maptbl_table);
    xmap_hptr->xut_itemcount  = 0;
    xmap_hptr->xst_capacity   = xst_capacity;
    xmap_hptr->xmap_item_dptr = (maptbl_item *)calloc(xst_capacity, sizeof(maptbl_item));
    if (X_NULL == xmap_hptr->xmap_item_dptr)
    {
        free(xmap_hptr);
        xmap_hptr = X_NULL;
    }

    return (x_handle_t)xmap_hptr;
}

/**********************************************************/
/**
 * @brief 销毁 映射表 对象。
 * 
 * @param [in ] xht_maptbl : 映射表的操作句柄（即 maptbl_create() 的返回值）。
 * 
 */
x_void_t maptbl_destroy(x_handle_t xht_maptbl)
{
    x_size_t       xst_iter  = 0;
    maptbl_item  * xmap_iptr = X_NULL;
    maptbl_table * xmap_hptr = (maptbl_table *)xht_maptbl;

    MAPTBL_VALID(xmap_hptr, MAPTBL_XVOID);

    xmap_hptr->xut_this_size = 0;

    for (xmap_iptr = xmap_hptr->xmap_item_dptr; xst_iter < xmap_hptr->xst_capacity; ++xst_iter, ++xmap_iptr)
    {
        if (X_NULL != xmap_iptr->xht_item_dptr)
        {
            maptbl_pair_clear(xmap_iptr);
            xmap_iptr->xht_item_dptr = X_NULL;
        }
    }

    free(xmap_hptr->xmap_item_dptr);
    free(xmap_hptr);
    xmap_hptr = X_NULL;
}

/**********************************************************/
/**
 * @brief 读取映射表中所映射的子项数量。
 * 
 * @param [in ] xht_maptbl : 映射表的操作句柄（即 maptbl_create() 的返回值）。
 * 
 * @return x_size_t
 *         - 返回 子项数量。
 */
x_size_t maptbl_count(x_handle_t xht_maptbl)
{
    maptbl_table * xmap_hptr = (maptbl_table *)xht_maptbl;
    MAPTBL_VALID(xmap_hptr, 0);

    return xmap_hptr->xut_itemcount;
}

/**********************************************************/
/**
 * @brief 锁定操作子项（与 maptbl_unlock() 作为对应操作）。
 * 
 * @param [in ] xht_maptbl  : 映射表的操作句柄（即 maptbl_create() 的返回值）。
 * @param [in ] xst_itemkey : 子项索引键。
 * @param [out] xht_handler : 操作成功返回关联的映射句柄（入参为 X_NULL 时，忽略返回操作）。
 * @param [in ] xst_timeout : 操作超时时间（单位为 ms 毫秒）。
 * 
 * @return x_int32_t
 *         - 返回 错误码值（参看 maptbl_error_table 枚举值）。
 */
x_int32_t maptbl_lock(x_handle_t xht_maptbl, x_size_t xst_itemkey, x_handle_t * xht_handler, x_size_t xst_timeout)
{
    maptbl_table * xmap_hptr = (maptbl_table *)xht_maptbl;
    maptbl_item  * xmap_iptr = X_NULL;
    maptbl_pair  * xpair_ptr = X_NULL;

    x_size_t xst_index = 0;

    //======================================

    if (MAPTBL_INVALID_ITEMKEY == xst_itemkey)
    {
        return MAPTBL_ERR_NITEMKEY;
    }

    MAPTBL_VALID(xmap_hptr, MAPTBL_ERR_HANDLE);

    //======================================

    xst_index = xst_itemkey % xmap_hptr->xst_capacity;
    if (0 != maptbl_lock_index(xmap_hptr, xst_index, &xmap_iptr, xst_timeout))
    {
        return MAPTBL_ERR_TIMEOUT;
    }

    xpair_ptr = maptbl_pair_find(xmap_iptr, xst_itemkey);
    if (X_NULL == xpair_ptr)
    {
        maptbl_unlock_index(xmap_hptr, xst_index);
        return MAPTBL_ERR_NOTFOUND;
    }

    if (X_NULL != xht_handler)
    {
        *xht_handler = xpair_ptr->xht_handler;
    }

    //======================================

    return MAPTBL_ERR_SUCCESS;
}

/**********************************************************/
/**
 * @brief 解锁操作子项（与 maptbl_lock() 作为对应操作）。
 * 
 * @param [in ] xht_maptbl  : 映射表的操作句柄（即 maptbl_create() 的返回值）。
 * @param [in ] xst_itemkey : 子项索引键。
 * 
 * @return x_int32_t
 *         - 返回 错误码值（参看 maptbl_error_table 枚举值）。
 */
x_int32_t maptbl_unlock(x_handle_t xht_maptbl, x_size_t xst_itemkey)
{
    maptbl_table * xmap_hptr = (maptbl_table *)xht_maptbl;

    if (MAPTBL_INVALID_ITEMKEY == xst_itemkey)
    {
        return MAPTBL_ERR_NITEMKEY;
    }

    MAPTBL_VALID(xmap_hptr, MAPTBL_ERR_HANDLE);

    if (0 != maptbl_unlock_index(xmap_hptr, xst_itemkey % xmap_hptr->xst_capacity))
    {
        return MAPTBL_ERR_OPERATOR;
    }

    return MAPTBL_ERR_SUCCESS;
}

// 锁操作的宏实现方式
#define MAPTBL_TRYLOCK_INDEX(xbt_vlock, xmap_hptr, xst_index, xmap_iptr, xst_timeout)       \
    if (xbt_vlock)                                                                          \
    {                                                                                       \
        if (0 != maptbl_lock_index((xmap_hptr), (xst_index), &(xmap_iptr), (xst_timeout)))  \
            return MAPTBL_ERR_TIMEOUT;                                                      \
    }                                                                                       \
    else                                                                                    \
    {                                                                                       \
        if (0 != maptbl_trylock_index((xmap_hptr), (xst_index), &(xmap_iptr)))              \
            return MAPTBL_ERR_OPERATOR;                                                     \
    }                                                                                       \

// 解锁操作的宏实现
#define MAPTBL_UNLOCK_INDEX(xbt_vlock, xmap_hptr, xst_index)  { if (xbt_vlock) maptbl_unlock_index((xmap_hptr), (xst_index)); }

/**********************************************************/
/**
 * @brief 向映射表插入子项。
 * 
 * @param [in ] xht_maptbl  : 映射表的操作句柄（即 maptbl_create() 的返回值）。
 * @param [in ] xst_itemkey : 子项索引键。
 * @param [in ] xht_handler : 子项关联的映射句柄。
 * @param [in ] xst_timeout : 操作超时时间（单位为 ms 毫秒），若为 0 值时，则不进行锁保护操作。
 * 
 * @return x_int32_t
 *         - 返回 错误码值（参看 maptbl_error_table 枚举值）。
 */
x_int32_t maptbl_insert(x_handle_t xht_maptbl, x_size_t xst_itemkey, x_handle_t xht_handler, x_size_t xst_timeout)
{
    x_size_t       xst_index = 0;
    maptbl_table * xmap_hptr = (maptbl_table *)xht_maptbl;
    maptbl_item  * xmap_iptr = X_NULL;
    maptbl_pair  * xpair_ptr = X_NULL;
    x_bool_t       xbt_vlock = (0 != xst_timeout);

    //======================================

    if (MAPTBL_INVALID_ITEMKEY == xst_itemkey)
    {
        return MAPTBL_ERR_NITEMKEY;
    }

    if (MAPTBL_INVALID_HANDLER == xht_handler)
    {
        return MAPTBL_ERR_NHANDLER;
    }

    MAPTBL_VALID(xmap_hptr, MAPTBL_ERR_HANDLE);

    //======================================

    xst_index = xst_itemkey % xmap_hptr->xst_capacity;
    MAPTBL_TRYLOCK_INDEX(xbt_vlock, xmap_hptr, xst_index, xmap_iptr, xst_timeout);

    xpair_ptr = maptbl_pair_find(xmap_iptr, xst_itemkey);
    if (X_NULL != xpair_ptr)
    {
        MAPTBL_UNLOCK_INDEX(xbt_vlock, xmap_hptr, xst_index);
        return MAPTBL_ERR_VRELATED;
    }

    //======================================

    if (!maptbl_pair_insert(xmap_iptr, xst_itemkey, xht_handler))
    {
        MAPTBL_UNLOCK_INDEX(xbt_vlock, xmap_hptr, xst_index);
        return MAPTBL_ERR_OUTOFMEM;
    }

    maptbl_atomic_add(&xmap_hptr->xut_itemcount, 1);
    MAPTBL_UNLOCK_INDEX(xbt_vlock, xmap_hptr, xst_index);

    //======================================

    return MAPTBL_ERR_SUCCESS;
}

/**********************************************************/
/**
 * @brief 更新映射表子项的映射句柄。
 * 
 * @param [in ] xht_maptbl  : 映射表的操作句柄（即 maptbl_create() 的返回值）。
 * @param [in ] xst_itemkey : 子项索引键。
 * @param [in ] xht_handler : 更新的映射句柄。
 * @param [in ] xst_timeout : 操作超时时间（单位为 ms 毫秒），若为 0 值时，则不进行锁保护操作。
 * 
 * @return x_int32_t
 *         - 返回 错误码值（参看 maptbl_error_table 枚举值）。
 */
x_int32_t maptbl_update(x_handle_t xht_maptbl, x_size_t xst_itemkey, x_handle_t xht_handler, x_size_t xst_timeout)
{
    x_size_t       xst_index = 0;
    maptbl_table * xmap_hptr = (maptbl_table *)xht_maptbl;
    maptbl_item  * xmap_iptr = X_NULL;
    maptbl_pair  * xpair_ptr = X_NULL;
    x_bool_t       xbt_vlock = (0 != xst_timeout);

    //======================================

    if (MAPTBL_INVALID_ITEMKEY == xst_itemkey)
    {
        return MAPTBL_ERR_NITEMKEY;
    }

    if (MAPTBL_INVALID_HANDLER == xht_handler)
    {
        return MAPTBL_ERR_NHANDLER;
    }

    MAPTBL_VALID(xmap_hptr, MAPTBL_ERR_HANDLE);

    //======================================

    xst_index = xst_itemkey % xmap_hptr->xst_capacity;
    MAPTBL_TRYLOCK_INDEX(xbt_vlock, xmap_hptr, xst_index, xmap_iptr, xst_timeout);

    xpair_ptr = maptbl_pair_find(xmap_iptr, xst_itemkey);
    if (X_NULL == xpair_ptr)
    {
        MAPTBL_UNLOCK_INDEX(xbt_vlock, xmap_hptr, xst_index);
        return MAPTBL_ERR_NOTFOUND;
    }

    xpair_ptr->xht_handler = xht_handler;
    MAPTBL_UNLOCK_INDEX(xbt_vlock, xmap_hptr, xst_index);

    //======================================

    return MAPTBL_ERR_SUCCESS;
}

/**********************************************************/
/**
 * @brief 查找（获取）映射表子项的映射句柄。
 * 
 * @note
 * <pre>
 *   一般情况下，不建议使用该接口，若需要获取映射句柄，
 *   可使用 maptbl_lock()/maptbl_unlock() 完成该功能。
 * </pre>
 * 
 * @param [in ] xht_maptbl  : 映射表的操作句柄（即 maptbl_create() 的返回值）。
 * @param [in ] xst_itemkey : 子项索引键。
 * @param [out] xht_handler : 操作成功返回所对应的映射句柄。
 * @param [in ] xst_timeout : 操作超时时间（单位为 ms 毫秒），若为 0 值时，则不进行锁保护操作。
 * 
 * @return x_int32_t
 *         - 返回 错误码值（参看 maptbl_error_table 枚举值）。
 */
x_int32_t maptbl_query(x_handle_t xht_maptbl, x_size_t xst_itemkey, x_handle_t * xht_handler, x_size_t xst_timeout)
{
    x_size_t       xst_index = 0;
    maptbl_table * xmap_hptr = (maptbl_table *)xht_maptbl;
    maptbl_item  * xmap_iptr = X_NULL;
    maptbl_pair  * xpair_ptr = X_NULL;
    x_bool_t       xbt_vlock = (0 != xst_timeout);

    //======================================

    if (MAPTBL_INVALID_ITEMKEY == xst_itemkey)
    {
        return MAPTBL_ERR_NITEMKEY;
    }

    if (X_NULL == xht_handler)
    {
        return MAPTBL_ERR_NHANDLER;
    }

    MAPTBL_VALID(xmap_hptr, MAPTBL_ERR_HANDLE);

    //======================================

    xst_index = xst_itemkey % xmap_hptr->xst_capacity;
    MAPTBL_TRYLOCK_INDEX(xbt_vlock, xmap_hptr, xst_index, xmap_iptr, xst_timeout);

    xpair_ptr = maptbl_pair_find(xmap_iptr, xst_itemkey);
    if (X_NULL == xpair_ptr)
    {
        MAPTBL_UNLOCK_INDEX(xbt_vlock, xmap_hptr, xst_index);
        return MAPTBL_ERR_NOTFOUND;
    }

    *xht_handler = xpair_ptr->xht_handler;
    MAPTBL_UNLOCK_INDEX(xbt_vlock, xmap_hptr, xst_index);

    //======================================

    return MAPTBL_ERR_SUCCESS;
}

/**********************************************************/
/**
 * @brief 从映射表中删除子项。
 * 
 * @param [in ] xht_maptbl  : 映射表的操作句柄（即 maptbl_create() 的返回值）。
 * @param [in ] xst_itemkey : 子项索引键。
 * @param [in ] xst_timeout : 操作超时时间（单位为 ms 毫秒），若为 0 值时，则不进行锁保护操作。
 * 
 * @return x_int32_t
 *         - 返回 错误码值（参看 maptbl_error_table 枚举值）。
 */
x_int32_t maptbl_delete(x_handle_t xht_maptbl, x_size_t xst_itemkey, x_size_t xst_timeout)
{
    x_size_t       xst_index = 0;
    maptbl_table * xmap_hptr = (maptbl_table *)xht_maptbl;
    maptbl_item  * xmap_iptr = X_NULL;
    x_bool_t       xbt_vlock = (0 != xst_timeout);

    //======================================

    if (MAPTBL_INVALID_ITEMKEY == xst_itemkey)
    {
        return MAPTBL_ERR_NITEMKEY;
    }

    MAPTBL_VALID(xmap_hptr, MAPTBL_ERR_HANDLE);

    //======================================

    xst_index = xst_itemkey % xmap_hptr->xst_capacity;
    MAPTBL_TRYLOCK_INDEX(xbt_vlock, xmap_hptr, xst_index, xmap_iptr, xst_timeout);

    //======================================

    if (!maptbl_pair_erase(xmap_iptr, xst_itemkey))
    {
        MAPTBL_UNLOCK_INDEX(xbt_vlock, xmap_hptr, xst_index);
        return MAPTBL_ERR_NOTFOUND;
    }

    maptbl_atomic_sub(&xmap_hptr->xut_itemcount, 1);

    MAPTBL_UNLOCK_INDEX(xbt_vlock, xmap_hptr, xst_index);

    //======================================

    return MAPTBL_ERR_SUCCESS;
}

/**********************************************************/
/**
 * @brief 清除映射表中所有子项。
 * 
 * @param [in ] xht_maptbl  : 映射表的操作句柄（即 maptbl_create() 的返回值）。
 * 
 * @return x_int32_t
 *         - 返回 错误码值（参看 maptbl_error_table 枚举值）。
 */
x_int32_t maptbl_cleanup(x_handle_t xht_maptbl)
{
    x_size_t       xst_iter  = 0;
    maptbl_item  * xmap_iptr = X_NULL;
    maptbl_table * xmap_hptr = (maptbl_table *)xht_maptbl;

    MAPTBL_VALID(xmap_hptr, MAPTBL_ERR_HANDLE);

    for (xmap_iptr = xmap_hptr->xmap_item_dptr; xst_iter < xmap_hptr->xst_capacity; ++xst_iter, ++xmap_iptr)
    {
        XASSERT(0 == maptbl_atomic_cmpxchg(&xmap_iptr->xut_spin_lock, 0, 0));

        if (X_NULL != xmap_iptr->xht_item_dptr)
        {
            maptbl_pair_clear(xmap_iptr);
            xmap_iptr->xht_item_dptr = X_NULL;
        }

        xmap_iptr->xut_spin_lock = 0;
    }

    xmap_hptr->xut_itemcount = 0;

    return MAPTBL_ERR_SUCCESS;
}

// 锁操作的宏实现方式
#define MAPTBL_TRYLOCK_ITER(xbt_vlock, xmap_hptr, xst_iter, xmap_iptr, xst_timeout, xrt_value)  \
    do                                                                                          \
    {                                                                                           \
        if (xbt_vlock)                                                                          \
        {                                                                                       \
            if (0 != maptbl_lock_index((xmap_hptr), (xst_iter), &(xmap_iptr), (xst_timeout)))   \
                return (xrt_value);                                                             \
        }                                                                                       \
        else                                                                                    \
        {                                                                                       \
            if (0 != maptbl_trylock_index((xmap_hptr), (xst_iter), &(xmap_iptr)))               \
                return (xrt_value);                                                             \
        }                                                                                       \
    } while (0)                                                                                 \

// 解锁操作的宏实现
#define MAPTBL_UNLOCK_ITER(xbt_vlock, xmap_hptr, xst_iter)  \
    do                                                      \
    {                                                       \
        if (xbt_vlock)                                      \
            maptbl_unlock_index((xmap_hptr), (xst_iter));   \
    } while (0)                                             \

/**********************************************************/
/**
 * @brief 读取映射表中首个映射子项的索引键值。
 * 
 * @param [in ] xht_maptbl  : 映射表的操作句柄（即 maptbl_create() 的返回值）。
 * @param [in ] xst_timeout : 操作超时时间（单位为 ms 毫秒），若为 0 值时，则不进行锁保护操作。
 * 
 * @return x_size_t
 *         - 返回 首个子项索引键值；
 *         - 返回 MAPTBL_INVALID_ITEMKEY，则表示为无效索引键值。
 */
x_size_t maptbl_first(x_handle_t xht_maptbl, x_size_t xst_timeout)
{
    maptbl_table * xmap_hptr = (maptbl_table *)xht_maptbl;
    maptbl_item  * xmap_iptr = X_NULL;
    x_bool_t       xbt_vlock = (0 != xst_timeout);

    x_size_t xst_iter = 0;
    x_size_t xst_ikey = 0;

    //======================================

    MAPTBL_VALID(xmap_hptr, MAPTBL_INVALID_ITEMKEY);

    //======================================

    for (xst_iter = 0; xst_iter < xmap_hptr->xst_capacity; ++xst_iter)
    {
        MAPTBL_TRYLOCK_ITER(xbt_vlock, xmap_hptr, xst_iter, xmap_iptr, xst_timeout, MAPTBL_INVALID_ITEMKEY);

        if (X_NULL != xmap_iptr->xht_item_dptr)
        {
            xst_ikey = xmap_iptr->xht_item_dptr->xst_itemkey;
            MAPTBL_UNLOCK_ITER(xbt_vlock, xmap_hptr, xst_iter);
            return xst_ikey;
        }

        MAPTBL_UNLOCK_ITER(xbt_vlock, xmap_hptr, xst_iter);
    }

    //======================================

    return MAPTBL_INVALID_ITEMKEY;
}

/**********************************************************/
/**
 * @brief 读取映射表中下一个映射子项的索引键值。
 * 
 * @param [in ] xht_maptbl  : 映射表的操作句柄（即 maptbl_create() 的返回值）。
 * @param [in ] xst_itemkey : 指定检索操作的起始索引键值。
 * @param [in ] xst_timeout : 操作超时时间（单位为 ms 毫秒），若为 0 值时，则不进行锁保护操作。
 * 
 * @return x_size_t
 *         - 返回 下一个子项索引键值；
 *         - 返回 MAPTBL_INVALID_ITEMKEY，则表示为无效索引键值。
 */
x_size_t maptbl_next(x_handle_t xht_maptbl, x_size_t xst_itemkey, x_size_t xst_timeout)
{
    maptbl_table * xmap_hptr = (maptbl_table *)xht_maptbl;
    maptbl_item  * xmap_iptr = X_NULL;
    maptbl_pair  * xpair_ptr = X_NULL;
    x_bool_t       xbt_vlock = (0 != xst_timeout);

    x_size_t xst_iter = 0;

    //======================================

    if (MAPTBL_INVALID_ITEMKEY == xst_itemkey)
    {
        return MAPTBL_INVALID_ITEMKEY;
    }

    MAPTBL_VALID(xmap_hptr, MAPTBL_INVALID_ITEMKEY);

    //======================================

    xst_iter = xst_itemkey % xmap_hptr->xst_capacity;

    MAPTBL_TRYLOCK_ITER(xbt_vlock, xmap_hptr, xst_iter, xmap_iptr, xst_timeout, MAPTBL_INVALID_ITEMKEY);

    xpair_ptr = maptbl_pair_find(xmap_iptr, xst_itemkey);
    if (X_NULL == xpair_ptr)
    {
        MAPTBL_UNLOCK_ITER(xbt_vlock, xmap_hptr, xst_iter);
        return MAPTBL_INVALID_ITEMKEY;
    }

    if (X_NULL != xpair_ptr->xpt_nextptr)
    {
        xst_itemkey = xpair_ptr->xpt_nextptr->xst_itemkey;
        MAPTBL_UNLOCK_ITER(xbt_vlock, xmap_hptr, xst_iter);
        return xst_itemkey;
    }

    MAPTBL_UNLOCK_ITER(xbt_vlock, xmap_hptr, xst_iter);

    //======================================

    for (xst_iter += 1; xst_iter < xmap_hptr->xst_capacity; ++xst_iter)
    {
        MAPTBL_TRYLOCK_ITER(xbt_vlock, xmap_hptr, xst_iter, xmap_iptr, xst_timeout, MAPTBL_INVALID_ITEMKEY);

        if (X_NULL != xmap_iptr->xht_item_dptr)
        {
            xst_itemkey = xmap_iptr->xht_item_dptr->xst_itemkey;
            MAPTBL_UNLOCK_ITER(xbt_vlock, xmap_hptr, xst_iter);
            return xst_itemkey;
        }

        MAPTBL_UNLOCK_ITER(xbt_vlock, xmap_hptr, xst_iter);
    }

    //======================================

    return MAPTBL_INVALID_ITEMKEY;
}

/**********************************************************/
/**
 * @brief 对映射表所有子项执行遍历操作。
 * 
 * @param [in ] xht_maptbl  : 映射表的操作句柄（即 maptbl_create() 的返回值）。
 * @param [in ] xst_nextkey : 以此键值的基准节点的下一节点作为遍历操作的起始节点（若为 MAPTBL_INVALID_ITEMKEY，则从起始节点开始遍历操作）。
 * @param [in ] xst_timeout : 操作超时时间（单位为 ms 毫秒），若为 0 值时，则不进行锁保护操作。
 * @param [in ] xfunc_ptr   : 遍历操作的回调函数接口。
 * @param [in ] xht_context : 遍历操作的回调上下文标识信息。
 * 
 * @return x_size_t
 *         - 返回 执行回调子项的数量。
 */
x_size_t maptbl_trav(x_handle_t xht_maptbl, x_size_t xst_nextkey, x_size_t xst_timeout, maptbl_trav_callback xfunc_ptr, x_handle_t xht_context)
{
    maptbl_table * xmap_hptr = (maptbl_table *)xht_maptbl;
    maptbl_item  * xmap_iptr = X_NULL;
    x_bool_t       xbt_vlock = (0 != xst_timeout);

    x_size_t   xst_iter  = 0;
    x_size_t   xst_count = 0;
    x_bool_t   xbt_keepc = X_TRUE;

    //======================================

    if (X_NULL == xfunc_ptr)
    {
        return 0;
    }

    MAPTBL_VALID(xmap_hptr, 0);

    if (xmap_hptr->xut_itemcount <= 0)
    {
        return 0;
    }

    //======================================

    if (MAPTBL_INVALID_ITEMKEY == xst_nextkey)
        xst_iter = 0;
    else
        xst_iter = xst_nextkey % xmap_hptr->xst_capacity;

    MAPTBL_TRYLOCK_ITER(xbt_vlock, xmap_hptr, xst_iter, xmap_iptr, xst_timeout, xst_count);

    xst_count += maptbl_pair_trav_next(xmap_iptr, xst_nextkey, &xbt_keepc, xht_maptbl, xfunc_ptr, xht_context);
    if (!xbt_keepc)
    {
        MAPTBL_UNLOCK_ITER(xbt_vlock, xmap_hptr, xst_iter);
        return xst_count;
    }

    MAPTBL_UNLOCK_ITER(xbt_vlock, xmap_hptr, xst_iter);

    //======================================

    for (xst_iter += 1; xst_iter < xmap_hptr->xst_capacity; ++xst_iter)
    {
        MAPTBL_TRYLOCK_ITER(xbt_vlock, xmap_hptr, xst_iter, xmap_iptr, xst_timeout, xst_count);

        if (X_NULL == xmap_iptr->xht_item_dptr)
        {
            MAPTBL_UNLOCK_ITER(xbt_vlock, xmap_hptr, xst_iter);
            continue;
        }

        xst_count += maptbl_pair_trav(xmap_iptr, &xbt_keepc, xht_maptbl, xfunc_ptr, xht_context);

        if (!xbt_keepc)
        {
            MAPTBL_UNLOCK_ITER(xbt_vlock, xmap_hptr, xst_iter);
            return xst_count;
        }

        MAPTBL_UNLOCK_ITER(xbt_vlock, xmap_hptr, xst_iter);
    }

    //======================================

    return xst_count;
}
