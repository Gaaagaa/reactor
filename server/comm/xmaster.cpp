/**
 * @file    xmaster.cpp
 * <pre>
 * Copyright (c) 2018, Gaaagaa All rights reserved.
 * 
 * 文件名称：xmaster.cpp
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

#include "xcomm.h"
#include "xmaster.h"
#include "xworker.h"
#include "xconfig.h"

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

////////////////////////////////////////////////////////////////////////////////

/**
 * @struct x_cmdline_args_t
 * @brief  命令行参数列表的结构体描述信息。
 */
typedef struct x_cmdline_args_t
{
    x_bool_t     xbt_in_frgnd;  ///< 是否在终端中运行程序
    std::string  xstr_app_name; ///< 程序名称
    std::string  xstr_log_file; ///< 日志输出配置的文件路径
    std::string  xstr_rcf_file; ///< 外部配置参数的文件路径
} x_cmdline_args_t;

/**********************************************************/
/**
 * @brief 输出程序的 命令行 使用信息。
 */
static x_void_t usage(x_cstring_t xszt_name)
{
    STD_TRACE("usage: %s [ -h | -v | -d | -l log.conf | -r run.conf ]\n", xszt_name);
    STD_TRACE("\t-h          : usage information"              );
    STD_TRACE("\t-v          : version information"            );
    STD_TRACE("\t-d          : Run in the foreground"          );
    STD_TRACE("\t-l log.conf : Specify a log config file"      );
    STD_TRACE("\t-r run.conf : Specify a parameter config file");

    STD_TRACE("\ncurrent version : %s", get_version_text());
}

/**********************************************************/
/**
 * @brief 从初始的命令行参数列表中解析出参数元组对象。
 * 
 * @param [in ] xit_argc : 初始化参数列表中的参数数量。
 * @param [in ] xct_argv : 初始化参数列表。
 * @param [out] xargs    : 操作成功返回的参数元组对象。
 * 
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
static x_int32_t make_cmdline_args(x_int32_t xit_argc, x_char_t * xct_argv[], x_cmdline_args_t & xargs)
{
    x_int32_t xit_error = 0;
    x_char_t  xct_iter  = EOF;

    //======================================
    // 设置默认值

    if (xit_argc <= 0)
    {
        usage("<NULL>");
        return -1;
    }

    xargs.xbt_in_frgnd  = X_FALSE;
    xargs.xstr_app_name = std::string(file_base_name(xct_argv[0]));
    xargs.xstr_log_file = std::string("");
    xargs.xstr_rcf_file = std::string("");

    //======================================
    // 解析参数

    while ((xct_iter = getopt(xit_argc, xct_argv, "hvdl:r:")) != EOF)
    {
        switch(xct_iter)
        {
        case 'h':
        case 'v':
            usage(xargs.xstr_app_name.c_str());
            xit_error = -1;
            break;

        case 'd':
            xargs.xbt_in_frgnd = X_TRUE;
            break;

        case 'l':
            if (X_NULL != optarg)
            {
                xargs.xstr_log_file = std::string(optarg);
            }
            break;

        case 'r':
            if (X_NULL != optarg)
            {
                xargs.xstr_rcf_file = std::string(optarg);
            }
            break;

        default:
            break;
        }

        if (0 != xit_error)
        {
            break;
        }
    }

    //======================================
    // 参数有效检查

    if (0 == xit_error)
    {
        // 必须指定 日志配置文件 和 参数配置文件
        if (xargs.xstr_log_file.empty() || xargs.xstr_rcf_file.empty())
        {
            usage(xargs.xstr_app_name.c_str());
            xit_error = -1;
        }
    }

    //======================================

    return xit_error;
}

////////////////////////////////////////////////////////////////////////////////
// x_master_t

//====================================================================

// 
// x_master_t : common invoking
// 

/**********************************************************/
/**
 * @brief 控制进程（主进程）的信号通知处理接口。
 */
x_void_t x_master_t::sig_handler(x_int32_t xit_signo)
{
    switch (xit_signo)
    {
    case SIGINT :
    case SIGQUIT:
    case SIGABRT:
    case SIGTERM:
        {
#if ((defined _DEBUG) || (defined DEBUG))
            ::_exit(0);
#else // !((defined _DEBUG) || (defined DEBUG))
            x_master_t::instance().post_msg(MSGID_SIGQUIT, sizeof(x_int32_t), &xit_signo);
#endif // ((defined _DEBUG) || (defined DEBUG))
        }
        break;

    case SIGCHLD:
        while (X_TRUE)
        {
            x_int32_t xit_stat = 0;
            x_ssize_t xst_wpid = waitpid(-1, &xit_stat, WNOHANG);
            if (xst_wpid <= 0)
            {
                break;
            }

            x_master_t::instance().post_msg(MSGID_SIGCHLD, sizeof(x_ssize_t), &xst_wpid);
        }
        break;

    default:
        break;
    }
}

/**********************************************************/
/**
 * @brief x_master_t 对象的单例调用接口。
 */
x_master_t & x_master_t::instance(void)
{
    static x_master_t _S_instance;
    return _S_instance;
}

//====================================================================

// 
// x_master_t : constructor/destructor
// 

x_master_t::x_master_t(void)
    : m_xit_srfd(-1)
    , m_xst_mpid(0)
    , m_xbt_running(X_FALSE)
    , m_xbt_pullworker(X_TRUE)
    , m_xht_worker(X_NULL)
{

}

x_master_t::~x_master_t(void)
{

}

//====================================================================

// 
// x_master_t : public interfaces
// 

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
x_int32_t x_master_t::startup(x_int32_t xit_argc, x_char_t * xct_argv[])
{
    x_int32_t xit_error = -1;

    do
    {
        //======================================
        // 程序的基本组件初始化操作

        xit_error = init_comm(xit_argc, xct_argv);
        if (0 != xit_error)
        {
            break;
        }

        //======================================
        // 创建工作进程（子进程）

        m_xbt_pullworker = (x_bool_t)x_config_t::instance().read_int("system", "enable_pullworker", X_TRUE);
#if ((defined _DEBUG) || (defined DEBUG))
        m_xht_worker = (x_handle_t)x_worker_t::create_test_worker();
#else // !((defined _DEBUG) || (defined DEBUG))
        xit_error = startup_worker(x_config_t::instance().read_int("system", "workers", 4));
        if (xit_error <= 0)
        {
            break;
        }        
#endif // ((defined _DEBUG) || (defined DEBUG))

        //======================================
        // 进入主进程的初始化流程

        // 主进程 PID 标识值
        m_xst_mpid = getpid();

        // 设置可运行标识
        m_xbt_running = X_TRUE;

        // 初始化消息通知的相关操作
        init_msg_handler();

        // 捕获特定信号进行处理
        if (!setup_signal())
        {
            LOGE("setup_signal() failed!");
            xit_error = -1;
            break;
        }

        //======================================
        xit_error = 0;
    } while (0);

    return xit_error;
}

/**********************************************************/
/**
 * @brief 运行操作（执行程序运行流程的工作）。
 */
x_int32_t x_master_t::run(void)
{
    x_int32_t xit_exitcode = 0;

RE_RUN:
    if (X_NULL != m_xht_worker)
    {
        // 子进程分支
        xit_exitcode = ((x_worker_t *)m_xht_worker)->run();

        // 删除子进程对象
        x_worker_t::destroy_worker((x_worker_t *)m_xht_worker, X_FALSE);
        m_xht_worker = X_NULL;
    }
    else
    {
        // 主进程分支
        while (m_xbt_running)
        {
            if (x_msg_handler_t::instance().queue_size() > 0)
                x_msg_handler_t::instance().msg_dispatch();

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        // 若切换到了子进程分支，则重新调用
        if (X_NULL != m_xht_worker)
            goto RE_RUN;
    }

    return xit_exitcode;
}

/**********************************************************/
/**
 * @brief 关闭操作（执行程序退出清理的工作）。
 */
x_void_t x_master_t::shutdown(void)
{
    cleanup(X_FALSE);

    // 关闭事件通知模块
    x_msg_handler_t::instance().close();

    // 关闭配置信息模块
    x_config_t::instance().close();

    // 关闭日志
    LOGH_CLOSE();
}

//====================================================================

// 
// x_master_t : internal invoking
// 

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
x_int32_t x_master_t::init_comm(x_int32_t xit_argc, x_char_t * xct_argv[])
{
    x_int32_t xit_error = -1;

    x_cmdline_args_t xargs;

    do
    {
        //======================================
        // 解析传入的命令行参数

        xit_error = make_cmdline_args(xit_argc, xct_argv, xargs);
        if (0 != xit_error)
        {
            break;
        }

        //======================================
        // 基本组件初始化

        // 确保主进程单例运行
        if (!ensure_singleton(xargs.xstr_app_name.c_str()))
        {
            STD_TRACE("ensure_singleton([%s]) failed!", xargs.xstr_app_name.c_str());
            xit_error = -1;
            break;
        }

        // 日志初始化
        xit_error = LOGH_OPEN(xargs.xstr_log_file.c_str(), X_NULL);
        if (0 != xit_error)
        {
            STD_TRACE("LOGH_OPEN([%s], X_NULL) return error code : %d", xargs.xstr_log_file.c_str(), xit_error);
            break;
        }

        // 加载配置参数信息
        xit_error = x_config_t::instance().open(xargs.xstr_rcf_file.c_str(), X_NULL);
        if (0 != xit_error)
        {
            LOGE("x_config_t::instance().open(%s, X_NULL) return error code : %d", xargs.xstr_rcf_file.c_str(), xit_error);
            STD_TRACE("x_config_t::instance().open(%s, X_NULL) return error code : %d", xargs.xstr_rcf_file.c_str(), xit_error);
            break;
        }

        // 打开事件通知模块
        xit_error = x_msg_handler_t::instance().open();
        if (0 != xit_error)
        {
            LOGE("x_msg_handler_t::instance().open() return error code : %d", xit_error);
            STD_TRACE("x_msg_handler_t::instance().open() return error code : %d", xit_error);
            break;
        }

        //======================================
        // 转换成守护进程

        if (!xargs.xbt_in_frgnd)
        {
            xit_error = daemon(X_FALSE, X_FALSE);
            if (-1 == xit_error)
            {
                LOGE("daemon(X_FALSE, X_FALSE) failed! errno : %d", errno);
                break;
            }

            // 转换成守护进程后，进程 ID 有所变更，
            // 需要重新对进程重新加锁，以确保主进程单例运行
            if (!ensure_singleton(xargs.xstr_app_name.c_str()))
            {
                LOGE("ensure_singleton([%s]) failed!", xargs.xstr_app_name.c_str());
                xit_error = -1;
                break;
            }
        }

        //======================================
        xit_error = 0;
    } while (0);

    return xit_error;
}

/**********************************************************/
/**
 * @brief 清理 主进程 相关的数据。
 * 
 * @param [in ] xbt_worker_invoking : 是否为工作进程（子进程）调用该接口。
 */
x_void_t x_master_t::cleanup(x_bool_t xbt_worker_invoking)
{
    if (m_xit_srfd >= 0)
    {
        close(m_xit_srfd);
    }

    m_xit_srfd    = -1;
    m_xst_mpid    = 0;
    m_xbt_running = X_FALSE;
    reset_signal();

    // 仅作 对象 的资源删除操作
    for (xmap_worker::iterator itmap = m_xmap_worker.begin(); itmap != m_xmap_worker.end(); ++itmap)
    {
        if (X_NULL != itmap->second)
        {
            x_worker_t::destroy_worker((x_worker_t *)itmap->second, !xbt_worker_invoking);
            itmap->second = X_NULL;
        }
    }

    m_xmap_worker.clear();

    // 清除掉所有通知事件
    x_msg_handler_t::instance().cleanup();
    reset_msg_handler();
}

/**********************************************************/
/**
 * @brief 主进程启动时的加锁操作，保证主进程的单实例运行。
 */
x_bool_t x_master_t::ensure_singleton(x_cstring_t xszt_key)
{
    x_char_t xszt_fname[TEXT_LEN_PATH] = { 0 };

    if (m_xit_srfd >= 0)
    {
        close(m_xit_srfd);
        m_xit_srfd = -1;
    }

    snprintf(xszt_fname, TEXT_LEN_PATH, "/tmp/[C9A81411-ECBE-4F6F-9AFF-1BF460B135DA][%s].pid", xszt_key);

    m_xit_srfd = singleton_run(xszt_fname);
    if (m_xit_srfd < 0)
    {
        return X_FALSE;
    }

    return X_TRUE;
}

/**********************************************************/
/**
 * @brief 设置（安装）系统相关的通知信号。
 */
x_bool_t x_master_t::setup_signal(void)
{
    x_int32_t xit_error = 0;

    do
    {
        if (SIG_ERR == signal(SIGINT , &x_master_t::sig_handler))
        {
            xit_error = errno;
            LOGE("signal(SIGINT , &x_master_t::sig_handler) return error code : %d", xit_error);
            break;
        }

        if (SIG_ERR == signal(SIGQUIT, &x_master_t::sig_handler))
        {
            xit_error = errno;
            LOGE("signal(SIGQUIT, &x_master_t::sig_handler) return error code : %d", xit_error);
            break;
        }

        if (SIG_ERR == signal(SIGABRT, &x_master_t::sig_handler))
        {
            xit_error = errno;
            LOGE("signal(SIGABRT, &x_master_t::sig_handler) return error code : %d", xit_error);
            break;
        }

        if (SIG_ERR == signal(SIGTERM, &x_master_t::sig_handler))
        {
            xit_error = errno;
            LOGE("signal(SIGTERM, &x_master_t::sig_handler) return error code : %d", xit_error);
            break;
        }

        if (SIG_ERR == signal(SIGCHLD, &x_master_t::sig_handler))
        {
            xit_error = errno;
            LOGE("signal(SIGCHLD, &x_master_t::sig_handler) return error code : %d", xit_error);
            break;
        }

    } while (0);

    if (0 != xit_error)
    {
        reset_signal();
    }

    return (0 == xit_error);
}

/**********************************************************/
/**
 * @brief 重置（移除）系统相关的通知信号。
 */
x_void_t x_master_t::reset_signal(void)
{
    signal(SIGINT , SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGABRT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);
}

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
x_int32_t x_master_t::startup_worker(x_size_t xst_nums)
{
    for (x_int32_t xit_iter = 0; xit_iter < (x_int32_t)xst_nums; ++xit_iter)
    {
        x_worker_t * xworker_ptr = x_worker_t::create_worker();

        // 若失败，则跳过
        if (X_NULL == xworker_ptr)
        {
            continue;
        }

        // 若为子进程，则终止循环
        if (0 == xworker_ptr->get_pid())
        {
            // 设置子进程调用接口
            m_xht_worker = (x_handle_t)xworker_ptr;

            // 清理 x_master_t 原有的数据
            cleanup(X_TRUE);
            break;
        }

        // 主进程：将子进程工作对象加入映射表进行管理
        m_xmap_worker.insert(std::make_pair(xworker_ptr->get_pid(), (x_handle_t)xworker_ptr));
    }

    // m_xht_worker 不为 X_NULL，表示此处为子进程分支
    if (X_NULL != m_xht_worker)
    {
        LOGI("startup worker process : (tid, pid) => (%d, %d)", get_tid(), get_pid());
        return 0;
    }

    return (x_int32_t)m_xmap_worker.size();
}

/**********************************************************/
/**
 * @brief 清除子进程对象。
 */
x_void_t x_master_t::erase_worker(x_ssize_t xst_wpid)
{
    xmap_worker::iterator itfind = m_xmap_worker.find(xst_wpid);
    if (itfind != m_xmap_worker.end())
    {
        x_worker_t::destroy_worker((x_worker_t *)itfind->second, X_FALSE);
        m_xmap_worker.erase(itfind);
    }
}

/**********************************************************/
/**
 * @brief 拉起一个子进程对象。
 */
x_bool_t x_master_t::pull_worker(void)
{
    if (!m_xbt_running || !m_xbt_pullworker)
    {
        return X_FALSE;
    }

    x_worker_t * xworker_ptr = x_worker_t::create_worker();

    // 若失败，则跳过
    if (X_NULL == xworker_ptr)
    {
        return X_FALSE;
    }

    if (0 == xworker_ptr->get_pid())
    {
        // 设置子进程调用接口
        m_xht_worker = (x_handle_t)xworker_ptr;

        // 清理 x_master_t 原有的数据
        cleanup(X_TRUE);
    }
    else
    {
        // 主进程：将子进程工作对象加入映射表进行管理
        m_xmap_worker.insert(std::make_pair(xworker_ptr->get_pid(), (x_handle_t)xworker_ptr));
    }

    return X_TRUE;
}

//====================================================================

// 
// x_master_t : msg handlers
// 

/**********************************************************/
/**
 * @brief 初始化消息通知的相关操作。
 */
x_int32_t x_master_t::init_msg_handler(void)
{
    XVERIFY(__obser_type::register_mkey(MSGID_SIGQUIT, &x_master_t::on_msg_sigquit));
    XVERIFY(__obser_type::register_mkey(MSGID_SIGCHLD, &x_master_t::on_msg_sigchld));
    XVERIFY(__obser_type::jointo_dispatch(this));
    XVERIFY(__obser_type::register_msg_diapatch());

    return 0;
}

/**********************************************************/
/**
 * @brief 重置（反初始化）消息通知的相关操作。
 */
x_void_t x_master_t::reset_msg_handler(void)
{
    __obser_type::reset_dispatch();
    __obser_type::unregister_msg_diapatch();
}

/**********************************************************/
/**
 * @brief 处理 退出信号 的消息。
 */
void x_master_t::on_msg_sigquit(x_uint32_t xut_size, x_pvoid_t xpvt_dptr)
{
    XASSERT(sizeof(x_int32_t) == xut_size);
    LOGI("on_msg_sigquit() signo : %d", *(x_int32_t *)xpvt_dptr);

    // 设置退出标识，迫使 run() 接口内的消息循环结束
    m_xbt_running = X_FALSE;
}

/**********************************************************/
/**
 * @brief 处理 工作进程结束 的消息。
 */
void x_master_t::on_msg_sigchld(x_uint32_t xut_size, x_pvoid_t xpvt_dptr)
{
    XASSERT(sizeof(x_ssize_t) == xut_size);

    LOGE("on_msg_sigchld() worker pid : %lld", *(x_ssize_t *)xpvt_dptr);

    erase_worker(*(x_ssize_t *)xpvt_dptr);
    pull_worker();
}
