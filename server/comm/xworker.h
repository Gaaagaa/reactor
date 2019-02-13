/**
 * @file    xworker.h
 * <pre>
 * Copyright (c) 2018, Gaaagaa All rights reserved.
 * 
 * 文件名称：xworker.h
 * 创建日期：2018年12月20日
 * 文件标识：
 * 文件摘要：程序工作进程的业务处理类。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2018年12月20日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#ifndef __XWORKER_H__
#define __XWORKER_H__

////////////////////////////////////////////////////////////////////////////////
// x_worker_t

/**
 * @class x_worker_t
 * @brief 程序工作进程的业务处理类。
 */
class x_worker_t
{
    // common invoking
public:
    /**********************************************************/
    /**
     * @brief 创建工作进程对象。
     */
    static x_worker_t * create_worker(void);

#if ((defined _DEBUG) || (defined DEBUG))
    /**********************************************************/
    /**
     * @brief 创建测试的工作进程对象。
     */
    static x_worker_t * create_test_worker(void);
#endif // ((defined _DEBUG) || (defined DEBUG))

    /**********************************************************/
    /**
     * @brief 销毁工作进程对象。
     * 
     * @param[in ] xobject_ptr : 工作进程对象。
     * @param[in ] xbt_kill    : 是否立即杀掉进程。
     */
    static x_void_t destroy_worker(x_worker_t * xobject_ptr, x_bool_t xbt_kill);

    // constructor/destructor
protected:
    explicit x_worker_t(x_ssize_t xst_wpid);
    ~x_worker_t(void);

private:
    x_worker_t(const x_worker_t & xobject);
    x_worker_t & operator=(const x_worker_t & xobject);

    // public interfaces
public:
    /**********************************************************/
    /**
     * @brief 返回 工作进程的 PID 。
     */
    inline x_ssize_t get_pid(void) const { return m_xst_wpid; }

    /**********************************************************/
    /**
     * @brief 工作进程的执行流程接口。
     */
    x_int32_t run(void);

    // internal invoking
protected:


    // data members
protected:
    x_ssize_t    m_xst_wpid;          ///< 工作进程的 PID 标识值
};

////////////////////////////////////////////////////////////////////////////////

#endif // __XWORKER_H__
