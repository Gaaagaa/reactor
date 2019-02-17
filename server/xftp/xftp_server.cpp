/**
 * @file    xftp_server.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xftp_server.cpp
 * 创建日期：2019年01月21日
 * 文件标识：
 * 文件摘要：
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年01月21日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#include "xcomm.h"
#include "xconfig.h"

#include "xftp_server.h"
#include "xftp_msgctxt.h"

#include "xftp_echo.h"
#include "xftp_query.h"

#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

////////////////////////////////////////////////////////////////////////////////

using x_workconf_t = x_tcp_io_server_t::x_workconf_t;

/** 全局的工作配置参数 */
static x_workconf_t _S_xwct_config;

/** 用于监听操作的套接字 */
static x_sockfd_t _S_xfdt_listen = X_INVALID_SOCKFD;

/** 文件存储的目录 */
static x_char_t _S_xszt_files_dir[TEXT_LEN_PATH] = { 0 };

/**********************************************************/
/**
 * @brief 初始化工作的文件存储目录。
 */
static x_int32_t init_files_dir(void)
{
    x_config_t & xconfig = x_config_t::instance();

    xconfig.read_str("xftp", "file_dir", _S_xszt_files_dir, TEXT_LEN_PATH, "");

    // 不能为空
    x_int32_t xit_len = (x_int32_t)strlen(_S_xszt_files_dir);
    if (xit_len <= 0)
    {
        STD_TRACE("file_dir is empty!");
        return -1;
    }

    // 判断是否为目录
    struct stat xstat_buf;
    if (0 != stat(_S_xszt_files_dir, &xstat_buf))
    {
        STD_TRACE("stat(_S_xszt_files_dir[%s], &xstat_buf) error : %d",
                  _S_xszt_files_dir, errno);
        return ((0 == errno) ? -1 : errno);
    }

    if (!S_ISDIR(xstat_buf.st_mode))
    {
        STD_TRACE("_S_xszt_files_dir[%s] is not a directory!", _S_xszt_files_dir);
        return -1;
    }

    // 确认字符串为 "/" 结尾
    if ('/' != _S_xszt_files_dir[xit_len - 1])
    {
        if ('\\' == _S_xszt_files_dir[xit_len - 1])
            _S_xszt_files_dir[xit_len - 1] = '/';
        else
            _S_xszt_files_dir[xit_len] = '/';
    }

    // 具备 读写 访问权限
    if (0 != access(_S_xszt_files_dir, R_OK | W_OK))
    {
        STD_TRACE("access(_S_xszt_files_dir[%s], R_OK | W_OK) error : %d",
                  _S_xszt_files_dir, errno);
        return ((0 == errno) ? -1 : errno);
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// x_ftp_server_t

//====================================================================

// 
// x_ftp_server_t : common invoking
// 

/**********************************************************/
/**
 * @brief x_master_t 在基本组件初始化完成后，额外进行数据初始化操作所使用的回调接口。
 * 
 * @param [in ] xht_context : 回调的上下文句柄。
 * 
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t x_ftp_server_t::init_extra_callback(x_handle_t xht_context)
{
    x_int32_t xit_error = 0;

    //======================================
    // 读取相关配置参数

    x_config_t & xconfig = x_config_t::instance();

    xconfig.read_str("server", "host", _S_xwct_config.xszt_host, TEXT_LEN_64, "");
    _S_xwct_config.xut_port             = xconfig.read_int("server", "port"            , 10086);
    _S_xwct_config.xut_epoll_maxsockfds = xconfig.read_int("server", "epoll_maxsockfds", 4096 );
    _S_xwct_config.xut_epoll_waitevents = xconfig.read_int("server", "epoll_waitevents", 256  );
    _S_xwct_config.xut_ioman_threads    = xconfig.read_int("server", "ioman_threads"   , 4    );
    _S_xwct_config.xut_tmout_kpalive    = xconfig.read_int("server", "tmout_kpalive"   , 8 * 60 * 1000);
    _S_xwct_config.xut_tmout_baleful    = xconfig.read_int("server", "tmout_baleful"   , 4 * 60 * 1000);
    _S_xwct_config.xut_tmout_mverify    = xconfig.read_int("server", "tmout_mverify"   , 4 * 60 * 1000);

    //======================================
    // 文件存储的目录

    xit_error = init_files_dir();
    if (0 != xit_error)
    {
        return xit_error;
    }

    //======================================
    // 创建程序监听操作的套接字

    _S_xfdt_listen = create_listen_sockfd(_S_xwct_config.xszt_host, _S_xwct_config.xut_port);
    if (X_INVALID_SOCKFD == _S_xfdt_listen)
    {
        return ((0 == errno) ? -1 : errno);
    }

    //======================================

    return xit_error;
}

/**********************************************************/
/**
 * @brief x_ftp_server_t 对象的单例调用接口。
 */
x_ftp_server_t & x_ftp_server_t::instance(void)
{
    static x_ftp_server_t _S_instance;
    return _S_instance;
}

/**********************************************************/
/**
 * @brief 套接字 IO 事件回调通知接口。
 * 
 * @param [in ] xfdt_sockfd : 套接字描述符。
 * @param [in ] xht_optargs : 回调至业务层的操作参数。
 * @param [in ] xut_ioevent : 回调的 IO 事件（参看 emIoEventCallback 枚举值）。
 * @param [in ] xht_context : 回调的上下文标识信息。
 * 
 * @return x_int32_t
 *         - 返回事件操作状态码（或错误码）。
 */
x_int32_t x_ftp_server_t::xio_event_callback(x_sockfd_t xfdt_sockfd,
                                             x_handle_t xht_optargs,
                                             x_uint32_t xut_ioevent,
                                             x_handle_t xht_context)
{
    x_ftp_server_t * xthis_ptr = (x_ftp_server_t *)xht_context;
    return xthis_ptr->xio_event(xfdt_sockfd, xht_optargs, xut_ioevent);
}

//====================================================================

// 
// x_ftp_server_t : constructor/destructor
// 

x_ftp_server_t::x_ftp_server_t(void)
{
    x_tcp_io_server_t::set_io_event_callback(
        &x_ftp_server_t::xio_event_callback, (x_handle_t)this);
}

x_ftp_server_t::~x_ftp_server_t(void)
{

}

//====================================================================

// 
// x_ftp_server_t : public interfaces
// 

/**********************************************************/
/**
 * @brief 启动 XTFP 网络服务。
 */
x_int32_t x_ftp_server_t::startup(void)
{
    x_int32_t xit_error = -1;

    do
    {
        //======================================

        if (is_startup())
        {
            xit_error = 0;
            break;
        }

        //======================================
        // 注册相关的业务层工作对象

#define REGISTER_IOTYPE(name)   XVERIFY(register_iotype(name::ECV_CONNECTION_TYPE, &name::create))

        REGISTER_IOTYPE(x_ftp_echo_t);
        REGISTER_IOTYPE(x_ftp_query_t);

#undef  REGISTER_IOTYPE

        //======================================

        xit_error = x_tcp_io_server_t::startup(_S_xwct_config, _S_xfdt_listen);
        if (0 != xit_error)
        {
            LOGE("x_tcp_io_server_t::startup(_S_xwct_config, _S_xfdt_listen[%d]) return error : %d",
                 _S_xfdt_listen, xit_error);
            break;
        }

        //======================================
        xit_error = 0;
    } while (0);

    if (0 != xit_error)
    {
        shutdown();
    }

    return xit_error;
}

/**********************************************************/
/**
 * @brief 关闭 XTFP 网络服务。
 */
x_void_t x_ftp_server_t::shutdown(void)
{
    x_tcp_io_server_t::shutdown();
    m_xmap_fcreate.clear();
}

/**********************************************************/
/**
 * @brief 注册业务层工作对象的创建接口。
 * 
 * @param [in ] xut_iotype   : 业务层工作对象 的 类型标识。
 * @param [in ] xfunc_create : 业务层工作对象 的 创建接口。
 * 
 * @return x_bool_t
 *         - 成功，返回 X_TRUE；
 *         - 失败，返回 X_FALSE。
 */
x_bool_t x_ftp_server_t::register_iotype(x_uint16_t xut_iotype, x_func_create_t xfunc_create)
{
    x_map_func_create_t::iterator itfind = m_xmap_fcreate.find(xut_iotype);
    if (itfind == m_xmap_fcreate.end())
    {
        m_xmap_fcreate.insert(std::make_pair(xut_iotype, xfunc_create));
        return X_TRUE;
    }

    return X_FALSE;
}

/**********************************************************/
/**
 * @brief 移除业务层工作对象的创建接口。
 */
x_void_t x_ftp_server_t::unregister_iotype(x_uint16_t xut_iotype)
{
    m_xmap_fcreate.erase(xut_iotype);
}

/**********************************************************/
/**
 * @brief 获取文件列表。
 * 
 * @param [out] xlst_files    : 操作成功返回的文件列表。
 * @param [in ] xut_max_files : 获取文件的最大数量。
 * 
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t x_ftp_server_t::get_file_list(x_list_file_t & xlst_files,
                                        x_uint32_t xut_max_files)
{
    struct stat     xstat_buf;
    struct dirent * xdirent_ptr = X_NULL;
    x_uint32_t      xut_count   = 0;

    if (xut_max_files <= 0)
    {
        return 0;
    }

    // 打开目录
    DIR * xdir_ptr = opendir(_S_xszt_files_dir);
    if (X_NULL == xdir_ptr)
    {
        LOGE("opendir(_S_xszt_files_dir[%s]) return X_NULL, errno : %d",
             _S_xszt_files_dir, errno);
        return ((0 == errno) ? -1 : errno);
    }

    while (X_NULL != (xdirent_ptr = readdir(xdir_ptr)))
    {
        //======================================
        // 文件路径

        std::string xstr_filename;
        xstr_filename  = _S_xszt_files_dir;
        xstr_filename += xdirent_ptr->d_name;

        //======================================
        // 只获取 常规文件，且具备 可读写 权限

        if (0 != stat(xstr_filename.c_str(), &xstat_buf))
        {
            LOGE("stat(xstr_filename.c_str()[%s], ...) errno : %d",
                 xstr_filename.c_str(), errno);
            continue;
        }

        if (!S_ISREG(xstat_buf.st_mode))
        {
            continue;
        }

        if (0 != access(xstr_filename.c_str(), R_OK | W_OK))
        {
            LOGE("access(xstr_filename.c_str()[%s], ...) errno : %d",
                 xstr_filename.c_str(), errno);
            continue;
        }

        //======================================

        xstr_filename = xdirent_ptr->d_name;
        if (!xstr_filename.empty())
        {
            xlst_files.push_back(std::make_pair(xstr_filename, (x_uint32_t)xstat_buf.st_size));
        }

        if (++xut_count >= xut_max_files)
        {
            break;
        }

        //======================================
    }

    if (X_NULL != xdir_ptr)
    {
        closedir(xdir_ptr);
        xdir_ptr = X_NULL;
    }

    return 0;
}

//====================================================================

// 
// x_ftp_server_t : internal invoking
// 

/**********************************************************/
/**
 * @brief 套接字 IO 事件回调通知接口。
 * 
 * @param [in ] xfdt_sockfd : 套接字描述符。
 * @param [in ] xht_optargs : 回调至业务层的操作参数。
 * @param [in ] xut_ioevent : 回调的 IO 事件（参看 emIoEventCallback 枚举值）。
 * 
 * @return x_int32_t
 *         - 返回事件操作状态码（或错误码）。
 */
x_int32_t x_ftp_server_t::xio_event(x_sockfd_t xfdt_sockfd,
                                    x_handle_t xht_optargs,
                                    x_uint32_t xut_ioevent)
{
    x_int32_t xit_error = 0;

    switch (xut_ioevent)
    {
    case EIO_ECBK_ACCEPT : xit_error = xio_event_accept(xfdt_sockfd, xht_optargs); break;
    case EIO_ECBK_CREATE : xit_error = xio_event_create(xfdt_sockfd, xht_optargs); break;

    default:
        break;
    }

    return xit_error;
}

/**********************************************************/
/**
 * @brief 处理 “套接字接收连接” 的事件回调操作（该接口仅由 xio_event() 调用）。
 */
x_int32_t x_ftp_server_t::xio_event_accept(x_sockfd_t xfdt_sockfd, x_handle_t xht_optargs)
{
    return 0;
}

/**********************************************************/
/**
 * @brief 处理 “建立业务层工作对象” 的事件回调操作（该接口仅由 xio_event() 调用）。
 */
x_int32_t x_ftp_server_t::xio_event_create(x_sockfd_t xfdt_sockfd, x_handle_t xht_optargs)
{
    x_int32_t xit_error = 0;

    x_tcp_io_create_args_t & xcreate_args = *(x_tcp_io_create_args_t *)xht_optargs;

    x_tcp_io_message_t & xio_message  = *(x_tcp_io_message_t *)xcreate_args.xht_message;
    x_io_msgctxt_t xio_msgctxt;

    x_map_func_create_t::iterator itfind;

    do
    {
        //======================================
        // 确定首个 IO 消息数据包的有效性，
        // 然后提取出来，作为创建业务层工作对象的参数

        xit_error = io_get_context(xio_message.data(), xio_message.rlen(), &xio_msgctxt);
        if (IOCTX_ERR_OK != xit_error)
        {
            if (IOCTX_ERR_PART == xit_error)
            {
                xit_error = 0;
                xcreate_args.xht_channel = X_NULL;
            }
            else
            {
                LOGE("io_get_context(..., xio_message.rlen()[%d], &xio_msgctxt) return error : %d",
                     xio_message.rlen(), xit_error);
                xit_error = -1;
            }

            break;
        }

        //======================================
        // 创建业务层工作对象

        itfind = m_xmap_fcreate.find(xio_msgctxt.io_cmid);
        if (itfind == m_xmap_fcreate.end())
        {
            LOGE("This type[%d] of connection is not supported!", xio_msgctxt.io_cmid);
            xit_error = -1;
            break;
        }

        xit_error = itfind->second(xcreate_args.xht_manager,
                                   xcreate_args.xfdt_sockfd,
                                   (x_handle_t)&xio_msgctxt,
                                   xcreate_args.xht_channel);
        if (0 != xit_error)
        {
            LOGE("itfind->second(..., xfdt_sockfd[%s:%d], xio_msgctxt[io_cmid: %d, io_size: %d], ...) return error : %d",
                 sockfd_remote_ip(xcreate_args.xfdt_sockfd, LOG_BUF(64), 64),
                 sockfd_remote_port(xcreate_args.xfdt_sockfd),
                 xio_msgctxt.io_cmid,
                 xio_msgctxt.io_size,
                 xit_error);
            break;
        }

        //======================================
        xit_error = 0;
    } while (0);

    return xit_error;
}
