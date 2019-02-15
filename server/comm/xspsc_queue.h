/**
 * @file    xspsc_queue.h
 * <pre>
 * Copyright (c) 2018, Gaaagaa All rights reserved.
 * 
 * 文件名称：xspsc_queue.h
 * 创建日期：2018年12月17日
 * 文件标识：
 * 文件摘要：实现线程安全的 单生产者/单消费者（single producer/single consumer） FIFO 队列。
 * 
 * 特别声明：x_spsc_queue_t 的设计，主要参考了 zeromq 的 yqueue_t 模板类的数据存储理念。
 * 特别鸣谢：zeromq 开源项目，Lee哥 。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2018年12月17日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#ifndef __XSPSC_QUEUE_H__
#define __XSPSC_QUEUE_H__

#include <memory>
#include <atomic>

////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////
// x_spsc_queue_t

/**
 * @class x_spsc_queue_t
 * @brief 实现线程安全的 单生产者/单消费者（single producer/single consumer） FIFO 队列。
 * 
 * @param [in ] _Ty    : 队列存储的元素类型。
 * @param [in ] _En    : 队列中的存储块可容纳元素的数量。
 * @param [in ] _Alloc : 元素分配器。
 */
template< typename _Ty, size_t _En, typename _Alloc = std::allocator< _Ty > >
class x_spsc_queue_t : protected _Alloc
{
    static_assert(_En >= 4, "_En size value must be greater than or equal to 4!");

    // common data types
public:
    using x_element_t = _Ty;

private:
    /**
     * @struct x_chunk_t
     * @brief  存储元素节点的连续内存块结构体。
     */
    typedef struct x_chunk_t
    {
        x_element_t  array[_En];  ///< 当前内存块中的元素节点数组
        x_chunk_t  * prev_ptr;    ///< 指向前一内存块节点
        x_chunk_t  * next_ptr;    ///< 指向后一内存块节点
    } x_chunk_t;

#ifdef _MSC_VER
    using ssize_t = std::intptr_t;
#endif // _MSC_VER

    using x_atomic_ptr_t  = std::atomic< x_chunk_t * >;
    using x_atomic_size_t = std::atomic< size_t >;
    using x_allocator_t   = _Alloc;

    // constructor/destructor
public:
    explicit x_spsc_queue_t(void)
        : m_chk_begin_ptr(nullptr)
        , m_xst_begin_pos(0)
        , m_chk_end_ptr(nullptr)
        , m_xst_end_pos(0)
        , m_chk_back_ptr(nullptr)
        , m_xst_back_pos(0)
        , m_xst_queue_size(0)
        , m_chk_swap_ptr(nullptr)
    {
        m_chk_begin_ptr = m_chk_end_ptr = alloc_chunk();
    }

    ~x_spsc_queue_t(void)
    {
        x_chunk_t * xchunk_ptr = nullptr;

        while (size() > 0)
            pop();

        while (true)
        {
            if (m_chk_begin_ptr == m_chk_end_ptr)
            {
                free_chunk(m_chk_begin_ptr);
                break;
            }

            xchunk_ptr = m_chk_begin_ptr;
            m_chk_begin_ptr = m_chk_begin_ptr->next_ptr;
            if (nullptr != xchunk_ptr)
                free_chunk(xchunk_ptr);
        }

        xchunk_ptr = m_chk_swap_ptr.exchange(nullptr);
        if (nullptr != xchunk_ptr)
            free_chunk(xchunk_ptr);
    }

    x_spsc_queue_t(x_spsc_queue_t && xobject) = delete;
    x_spsc_queue_t(const x_spsc_queue_t & xobject) = delete;
    x_spsc_queue_t & operator=(const x_spsc_queue_t & xobject) = delete;

    // public interfaces
public:
    /**********************************************************/
    /**
     * @brief 当前队列中的元素数量。
     */
    inline size_t size(void) const
    {
        return m_xst_queue_size;
    }

    /**********************************************************/
    /**
     * @brief 判断队列是否为空。
     */
    inline bool empty(void) const
    {
        return (0 == size());
    }

    /**********************************************************/
    /**
     * @brief 向队列尾端压入一个元素。
     */
    void push(const x_element_t & xemt_value)
    {
        x_allocator_t::construct(&m_chk_end_ptr->array[m_xst_end_pos], xemt_value);

        m_chk_back_ptr = m_chk_end_ptr;
        m_xst_back_pos = m_xst_end_pos;

        m_xst_queue_size.fetch_add(1);
        move_end_pos();
    }

    /**********************************************************/
    /**
     * @brief 向队列尾端压入一个元素。
     */
    void push(x_element_t && xemt_value)
    {
        x_allocator_t::construct(&m_chk_end_ptr->array[m_xst_end_pos],
                                 std::forward< x_element_t >(xemt_value));

        m_chk_back_ptr = m_chk_end_ptr;
        m_xst_back_pos = m_xst_end_pos;

        m_xst_queue_size.fetch_add(1);
        move_end_pos();
    }

    /**********************************************************/
    /**
     * @brief 从队列前端弹出一个元素。
     */
    void pop(void)
    {
        if (empty())
            return;
        m_xst_queue_size.fetch_sub(1);
        x_allocator_t::destroy(&m_chk_begin_ptr->array[m_xst_begin_pos]);
        move_begin_pos();
    }

    /**********************************************************/
    /**
     * @brief 返回队列首个元素。
     */
    inline x_element_t & front(void)
    {
        XASSERT(!empty());
        return m_chk_begin_ptr->array[m_xst_begin_pos];
    }

    /**********************************************************/
    /**
     * @brief 返回队列首个元素。
     */
    inline const x_element_t & front(void) const
    {
        XASSERT(!empty());
        return m_chk_begin_ptr->array[m_xst_begin_pos];
    }

    /**********************************************************/
    /**
     * @brief 返回队列末端元素。
     */
    inline x_element_t & back(void)
    {
        XASSERT(!empty());
        return m_chk_back_ptr->array[m_xst_back_pos];
    }

    /**********************************************************/
    /**
     * @brief 返回队列末端元素。
     */
    inline const x_element_t & back(void) const
    {
        XASSERT(!empty());
        return m_chk_back_ptr->array[m_xst_back_pos];
    }

    // internal invoking
private:
    /**********************************************************/
    /**
     * @brief 申请一个存储元素节点的内存块。
     */
    x_chunk_t * alloc_chunk(void)
    {
        constexpr size_t const xchunk_count =
            (sizeof(x_chunk_t) + sizeof(x_element_t) - 1) / sizeof(x_element_t);

        x_chunk_t * xchunk_ptr =
            reinterpret_cast< x_chunk_t * >(x_allocator_t::allocate(xchunk_count));
        XASSERT(nullptr != xchunk_ptr);
        if (nullptr != xchunk_ptr)
        {
            xchunk_ptr->prev_ptr = nullptr;
            xchunk_ptr->next_ptr = nullptr;
        }
        return xchunk_ptr;
    }

    /**********************************************************/
    /**
     * @brief 释放一个存储元素节点的内存块。
     */
    void free_chunk(x_chunk_t * xchunk_ptr)
    {
        constexpr size_t const xchunk_count =
            (sizeof(x_chunk_t) + sizeof(x_element_t) - 1) / sizeof(x_element_t);

        if (nullptr != xchunk_ptr)
        {
            x_allocator_t::deallocate(reinterpret_cast< x_element_t * >(xchunk_ptr), xchunk_count);
        }
    }

    /**********************************************************/
    /**
     * @brief 将起始端位置向后移（该接口仅由 pop() 接口调用）。
     */
    void move_begin_pos(void)
    {
        if (++m_xst_begin_pos == _En)
        {
            x_chunk_t * xchunk_ptr = m_chk_begin_ptr;
            m_chk_begin_ptr = m_chk_begin_ptr->next_ptr;
            XASSERT(nullptr != m_chk_begin_ptr);
            m_chk_begin_ptr->prev_ptr = nullptr;
            m_xst_begin_pos = 0;

            xchunk_ptr = m_chk_swap_ptr.exchange(xchunk_ptr);
            if (nullptr != xchunk_ptr)
                free_chunk(xchunk_ptr);
        }
    }

    /**********************************************************/
    /**
     * @brief 将结束端位置向后移（该接口仅由 push() 接口调用）。
     */
    void move_end_pos(void)
    {
        if (++m_xst_end_pos == _En)
        {
            x_chunk_t * xchunk_ptr = m_chk_swap_ptr.exchange(nullptr);
            if (nullptr != xchunk_ptr)
            {
                m_chk_end_ptr->next_ptr = xchunk_ptr;
                xchunk_ptr->prev_ptr = m_chk_end_ptr;
            }
            else
            {
                m_chk_end_ptr->next_ptr = alloc_chunk();
                m_chk_end_ptr->next_ptr->prev_ptr = m_chk_end_ptr;
            }

            m_chk_end_ptr = m_chk_end_ptr->next_ptr;
            m_xst_end_pos = 0;
        }
    }

    // data members
protected:
    x_chunk_t       * m_chk_begin_ptr;  ///< 内存块链表的起始块
    ssize_t           m_xst_begin_pos;  ///< 队列中的首个元素位置
    x_chunk_t       * m_chk_end_ptr;    ///< 内存块链表的结束块
    ssize_t           m_xst_end_pos;    ///< 队列中的元素结束位置
    x_chunk_t       * m_chk_back_ptr;   ///< 内存块链表的结尾块
    ssize_t           m_xst_back_pos;   ///< 队列中的结尾元素位置
    x_atomic_size_t   m_xst_queue_size; ///< 队列中的有效元素数量
    x_atomic_ptr_t    m_chk_swap_ptr;   ///< 用于保存临时内存块（备用缓存块）
};

////////////////////////////////////////////////////////////////////////////////

#endif // __XSPSC_QUEUE_H__
