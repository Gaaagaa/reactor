/**
 * @file    xmaster.h
 * <pre>
 * Copyright (c) 2018, Gaaagaa All rights reserved.
 * 
 * 文件名称：xmaster.h
 * 创建日期：2018年12月20日
 * 文件标识：
 * 文件摘要：程序主进程的控制管理类。
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

#ifndef __XMASTER_H__
#define __XMASTER_H__

#include <map>

////////////////////////////////////////////////////////////////////////////////
// x_master_t

/**
 * @class x_master_t
 * @brief 程序主进程的控制管理类。
 */
class x_master_t : public x_event_observer_t< x_master_t, EM_EOT_MASTER >
{
    // common data types
public:
    /**
     * @enum  emConstValue
     * @brief 相关的枚举常量值。
     */
    typedef enum emConstValue
    {

    } emConstValue;

    /**
     * @enum  emEventKey
     * @brief 定义观察者的事件索引键。
     */
    typedef enum emEventKey
    {
        EVENT_KEY_SIGQUIT   = 0x00000100,  ///< 退出操作信号
        EVENT_KEY_SIGCHLD   = 0x00000200,  ///< 工作进程结束
    } emEventKey;

    // common invoking
private:
    /**********************************************************/
    /**
     * @brief 控制进程（主进程）的信号通知处理接口。
     */
    static x_void_t sig_handler(x_int32_t xit_signo);

public:
    /**********************************************************/
    /**
     * @brief x_master_t 对象的单例调用接口。
     */
    static x_master_t & instance(void);

    // constructor/destructor
private:
    explicit x_master_t(void);
    ~x_master_t(void);

    x_master_t(const x_master_t & xobject);
    x_master_t & operator=(const x_master_t & xobject);

    // public interfaces
public:
    /**********************************************************/
    /**
     * @brief 启动操作（执行程序初始化的工作）。
     * 
     * @param [in ] xit_argc : 启动参数列表中的参数数量。
     * @param [in ] xct_argv : 启动参数列表。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t startup(x_int32_t xit_argc, x_char_t * xct_argv[]);

    /**********************************************************/
    /**
     * @brief 运行操作（执行程序运行流程的工作）。
     */
    x_int32_t run(void);

    /**********************************************************/
    /**
     * @brief 关闭操作（执行程序退出清理的工作）。
     */
    x_void_t shutdown(void);

    // internal invoking
protected:
    /**********************************************************/
    /**
     * @brief 程序的基本组件初始化操作接口（由 startup() 接口调用）。
     * 
     * @param [in ] xit_argc : 初始化参数列表中的参数数量。
     * @param [in ] xct_argv : 初始化参数列表。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t init_comm(x_int32_t xit_argc, x_char_t * xct_argv[]);

    /**********************************************************/
    /**
     * @brief 清理 主进程 相关的数据。
     * 
     * @param [in ] xbt_worker_invoking : 是否为工作进程（子进程）调用该接口。
     */
    x_void_t cleanup(x_bool_t xbt_worker_invoking);

    /**********************************************************/
    /**
     * @brief 主进程启动时的加锁操作，保证主进程的单实例运行。
     */
    x_bool_t ensure_singleton(x_cstring_t xszt_key);

    /**********************************************************/
    /**
     * @brief 设置（安装）系统相关的通知信号。
     */
    x_bool_t setup_signal(void);

    /**********************************************************/
    /**
     * @brief 重置（移除）系统相关的通知信号。
     */
    x_void_t reset_signal(void);

    /**********************************************************/
    /**
     * @brief 启动相关的工作进程（接口仅由 startup() 调用）。
     * 
     * @param [in ] xst_nums : 要启动的工作进程（子进程）数量。
     * 
     * @return x_int32_t
     *         - 返回值  < 0，表示操作产生错误；
     *         - 返回值 == 0，在子进程分支；
     *         - 返回值  > 0，在主进程分支，其值表示启动的工作进程（子进程）数量。
     */
    x_int32_t startup_worker(x_size_t xst_nums);

    /**********************************************************/
    /**
     * @brief 清除子进程对象。
     */
    x_void_t erase_worker(x_ssize_t xst_wpid);

    /**********************************************************/
    /**
     * @brief 拉起一个子进程对象。
     */
    x_bool_t pull_worker(void);

    // event handlers
protected:
    /**********************************************************/
    /**
     * @brief 初始化事件通知的相关操作。
     */
    x_int32_t init_event_handler(void);

    /**********************************************************/
    /**
     * @brief 重置（反初始化）事件通知的相关操作。
     */
    x_void_t reset_event_handler(void);

    /**********************************************************/
    /**
     * @brief 处理 退出信号 的事件。
     */
    void on_event_sigquit(x_uint32_t xut_size, x_pvoid_t xpvt_dptr);

    /**********************************************************/
    /**
     * @brief 处理 工作进程结束 的事件。
     */
    void on_event_sigchld(x_uint32_t xut_size, x_pvoid_t xpvt_dptr);

    // data members
protected:
    typedef std::map< x_ssize_t, x_handle_t > xmap_worker;

    x_int32_t     m_xit_srfd;        ///< 主进程单实例运行 所使用的文件锁 对应的 文件描述符
    x_ssize_t     m_xst_mpid;        ///< 主进程的 PID 标识
    x_bool_t      m_xbt_running;     ///< 标识是否可继续运行
    x_bool_t      m_xbt_pullworker;  ///< 子进程退出时，是否可重新拉起子进程继续工作
    xmap_worker   m_xmap_worker;     ///< 主进程管理的子进程对象映射表

    x_handle_t    m_xht_worker;      ///< 子进程的工作对象
};

////////////////////////////////////////////////////////////////////////////////

#endif // __XMASTER_H__
