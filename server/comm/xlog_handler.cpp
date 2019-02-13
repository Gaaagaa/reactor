/**
 * @file    xlog_handler.cpp
 * <pre>
 * Copyright (c) 2018, Gaaagaa All rights reserved.
 * 
 * 文件名称：xlog_handler.cpp
 * 创建日期：2018年12月28日
 * 文件标识：
 * 文件摘要：日志输出的操作类（单例模式调用）。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2018年12月28日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#include "xcomm.h"
#include "xlog_handler.h"

#include <log4cpp/Category.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/RemoteSyslogAppender.hh>
#include <log4cpp/PropertyConfigurator.hh>

////////////////////////////////////////////////////////////////////////////////

/**********************************************************/
/**
 * @brief 从文件全路径名中，获取其文件名（例如：/Folder/filename.txt 返回 filename.txt）。
 * 
 * @param [in ] xszt_file_path : 文件全路径名。
 * 
 * @return x_cstring_t
 *         - 返回对应的 文件名。
 */
static x_cstring_t xlog_file_base_name(x_cstring_t xszt_file_path)
{
    register x_char_t * xct_iter = (x_char_t *)xszt_file_path;
    register x_char_t * xct_vpos = (x_char_t *)xszt_file_path;

    if (X_NULL == xct_vpos)
    {
        return X_NULL;
    }

    while (*xct_iter)
    {
        if (('\\' == *xct_iter) || ('/' == *xct_iter))
            xct_vpos = xct_iter + 1;
        xct_iter += 1;
    }

    return (x_cstring_t)xct_vpos;
}

////////////////////////////////////////////////////////////////////////////////
// x_log_handler_t

//====================================================================

// 
// x_log_handler_t : common invoking
// 

/**********************************************************/
/**
 * @brief x_log_handler_t 对象的单例调用接口。
 */
x_log_handler_t & x_log_handler_t::instance(void)
{
    static x_log_handler_t _S_instance;
    return _S_instance;
}

//====================================================================

// 
// x_log_handler_t : constructor/destructor
// 

x_log_handler_t::x_log_handler_t(void)
    : m_xht_handler(X_NULL)
{

}

x_log_handler_t::~x_log_handler_t(void)
{
    close();
}

//====================================================================

// 
// x_log_handler_t : public interfaces
// 

/**********************************************************/
/**
 * @brief 打开日志输出模块。
 * 
 * @param [in ] xszt_conf_file : 日志输出的外部配置文件路径。
 * @param [in ] xpvt_reserved  : 保留参数（可以设置为 X_NULL）。
 * 
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t x_log_handler_t::open(x_cstring_t xszt_conf_file, x_pvoid_t xpvt_reserved)
{
    XASSERT(X_NULL == m_xht_handler);

    try
    {
        log4cpp::PropertyConfigurator::configure(xszt_conf_file);
    }
    catch (log4cpp::ConfigureFailure & xinfo_exception)
    {
        printf("Load log config file %s failed with result : %s\n", xszt_conf_file, xinfo_exception.what());
        return -1;
    }

    m_xht_handler = (x_handle_t)&log4cpp::Category::getRoot();

    return 0;
}

/**********************************************************/
/**
 * @brief 关闭日志输出模块。
 */
x_void_t x_log_handler_t::close(void)
{
    if (X_NULL != m_xht_handler)
    {
        try
        {
            log4cpp::Category::shutdownForced();
        }
        catch (...)
        {

        }

        m_xht_handler = X_NULL;
    }
}

/**********************************************************/
/**
 * @brief 输出日志信息。
 * 
 * @param [in ] xot_property : 输出信息的属性（参看 emOutputType 枚举值）。
 * @param [in ] xszt_format  : 输出信息的格式。
 * @param [in ] ...          : 输出信息的相关参数列表。
 */
x_void_t x_log_handler_t::output(emOutputType xot_property, x_cstring_t xszt_format, ...)
{
    if (X_NULL == m_xht_handler)
    {
        return;
    }

    log4cpp::Category * xlog_ptr = (log4cpp::Category *)m_xht_handler;

    va_list xva_args;
    va_start(xva_args, xszt_format);

    try
    {
        switch (xot_property)
        {
        case EMOT_INFO  : xlog_ptr->logva(log4cpp::Priority::INFO , xszt_format, xva_args); break;
        case EMOT_DEBUG : xlog_ptr->logva(log4cpp::Priority::DEBUG, xszt_format, xva_args); break;
        case EMOT_ERROR : xlog_ptr->logva(log4cpp::Priority::ERROR, xszt_format, xva_args); break;
        case EMOT_WARN  : xlog_ptr->logva(log4cpp::Priority::WARN , xszt_format, xva_args); break;

        default:
            break;
        }
    }
    catch(...)
    {

    }

    va_end(xva_args);
}

/**********************************************************/
/**
 * @brief 输出日志信息。
 * 
 * @param [in ] xot_property  : 输出信息的属性（参看 emOutputType 枚举值）。
 * @param [in ] xszt_filename : 代码文件名称。
 * @param [in ] xit_codeline  : 日志输出所在的代码行号。
 * @param [in ] xszt_function : 函数名称。
 * @param [in ] xszt_format   : 输出信息的格式。
 * @param [in ] ...           : 输出信息的相关参数列表。
 */
x_void_t x_log_handler_t::output_ex(emOutputType xot_property,
                                    x_cstring_t xszt_filename,
                                    x_int32_t   xit_codeline,
                                    x_cstring_t xszt_function,
                                    x_cstring_t xszt_format,
                                    ...)
{
    if (X_NULL == m_xht_handler)
    {
        return;
    }

    std::stringstream xstr_ostr;
    xstr_ostr << "["
              << xlog_file_base_name(xszt_filename)
              << ", "
              << xit_codeline
              << ", "
              << xszt_function
              << "] "
              << xszt_format;
    std::string xstr_format = xstr_ostr.str();

    log4cpp::Category * xlog_ptr = (log4cpp::Category *)m_xht_handler;

    va_list xva_args;
    va_start(xva_args, xszt_format);

    try
    {
        switch (xot_property)
        {
        case EMOT_INFO  : xlog_ptr->logva(log4cpp::Priority::INFO , xstr_format.c_str(), xva_args); break;
        case EMOT_DEBUG : xlog_ptr->logva(log4cpp::Priority::DEBUG, xstr_format.c_str(), xva_args); break;
        case EMOT_ERROR : xlog_ptr->logva(log4cpp::Priority::ERROR, xstr_format.c_str(), xva_args); break;
        case EMOT_WARN  : xlog_ptr->logva(log4cpp::Priority::WARN , xstr_format.c_str(), xva_args); break;

        default:
            break;
        }
    }
    catch(...)
    {

    }

    va_end(xva_args);
}

////////////////////////////////////////////////////////////////////////////////
// x_log_tconv_t

//====================================================================

// 
// x_log_tconv_t : common invoking
// 

/**********************************************************/
/**
 * @brief 打开字符串缓存操作句柄。
 * 
 * @param [in ] xit_max_tconv : 最大缓存数量。
 * 
 * @return x_handle_t
 *         - 成功，返回 有效操作句柄；
 *         - 失败，返回 X_NULL。
 */
x_handle_t x_log_tconv_t::open_handle(x_int32_t xit_max_tconv)
{
	if (xit_max_tconv <= 0)
	{
		return X_NULL;
	}

	x_int32_t   xit_size   = (xit_max_tconv + 1) * sizeof(x_pvoid_t);
	x_pvoid_t * xpvt_tconv = (x_pvoid_t *)malloc(xit_size);

	if (X_NULL == xpvt_tconv)
	{
		return X_NULL;
	}

	memset(xpvt_tconv, 0, xit_size - sizeof(x_pvoid_t));

	// 结束标识
	xpvt_tconv[xit_max_tconv] = (x_pvoid_t)-1;

	return (x_handle_t)xpvt_tconv;
}

/**********************************************************/
/**
 * @brief 关闭字符串缓存操作句柄。
 */
x_void_t x_log_tconv_t::close_handle(x_handle_t xht_tconv)
{
	x_string_t  xszt_text  = X_NULL;
	x_int32_t   xit_iter   = 0;
	x_pvoid_t * xpvt_tconv = (x_pvoid_t *)xht_tconv;

	if (X_NULL == xpvt_tconv)
	{
		return;
	}

	for (xit_iter = 0; (x_pvoid_t)-1 != xpvt_tconv[xit_iter]; ++xit_iter)
	{
		xszt_text = (x_string_t)xpvt_tconv[xit_iter];
		if (X_NULL != xszt_text)
		{
			free(xszt_text);
		}

		xpvt_tconv[xit_iter] = X_NULL;
	}

	free(xpvt_tconv);
	xpvt_tconv = X_NULL;
}

/**********************************************************/
/**
 * @brief 申请字符串缓存操作句柄。
 * 
 * @param [in ] xht_tconv : 字符串缓存操作句柄。
 * @param [in ] xut_size  : 申请的缓存大小。
 * 
 * @return x_string_t
 *         - 成功，返回 申请到的缓存地址；
 *         - 失败，返回 X_NULL。
 */
x_string_t x_log_tconv_t::alloc_buffer(x_handle_t xht_tconv, x_uint32_t xut_size)
{
	x_string_t  xszt_text  = X_NULL;
	x_int32_t   xit_iter   = 0;
	x_int32_t   xit_iter_p = -1;
	x_pvoid_t * xpvt_tconv = (x_pvoid_t *)xht_tconv;

	if ((X_NULL == xpvt_tconv) || (xut_size <= 0))
	{
		return X_NULL;
	}

	//======================================
	// 查找可用内存

	xit_iter_p = -1;
	for (xit_iter = 0; (x_pvoid_t)-1 != xpvt_tconv[xit_iter]; ++xit_iter)
	{
		if (X_NULL == (x_string_t)xpvt_tconv[xit_iter])
		{
			xit_iter_p = xit_iter;
			break;
		}
	}

	if (-1 == xit_iter_p)
	{
		XASSERT(X_FALSE);
		return X_NULL;
	}

	//======================================
	// 计算待转换字符数量，并申请内存

	xszt_text = (x_string_t)calloc(xut_size, sizeof(x_char_t));
	if (X_NULL == xszt_text)
	{
		return X_NULL;
	}

	xpvt_tconv[xit_iter_p] = (x_pvoid_t)xszt_text;

	//======================================

	return xszt_text;
}

/**********************************************************/
/**
 * @brief 以十六进制格式打印缓存数据。
 * 
 * @param [in ] xht_tconv : 字符串缓存操作句柄。
 * @param [in ] xct_dptr  : 缓存数据。
 * @param [in ] xut_size  : 缓存数据大小。
 * 
 * @return x_cstring_t
 *         - 成功，返回 输出的字符串；
 *         - 失败，返回 X_NULL。
 */
x_cstring_t x_log_tconv_t::hex_dump(x_handle_t xht_tconv, const x_uchar_t * xct_dptr, x_uint32_t xut_size)
{
    x_string_t  xszt_text  = X_NULL;
    x_int32_t   xit_iter   = 0;
    x_int32_t   xit_iter_p = -1;
    x_pvoid_t * xpvt_tconv = (x_pvoid_t *)xht_tconv;
    x_uchar_t * xct_iter_d = X_NULL;
    x_char_t  * xct_iter_v = X_NULL;

    static x_char_t xszt_hex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

    if ((X_NULL == xpvt_tconv) || (X_NULL == xct_dptr) || (xut_size <= 0))
    {
        return "\0";
    }

    //======================================
    // 查找可用内存

    xit_iter_p = -1;
    for (xit_iter = 0; (x_pvoid_t)-1 != xpvt_tconv[xit_iter]; ++xit_iter)
    {
        if (X_NULL == (x_string_t)xpvt_tconv[xit_iter])
        {
            xit_iter_p = xit_iter;
            break;
        }
    }

    if (-1 == xit_iter_p)
    {
        XASSERT(X_FALSE);
        return "\0";
    }

    //======================================
    // 申请内存

    xszt_text = (x_string_t)calloc((3 * xut_size + 1), sizeof(x_char_t));
    if (X_NULL == xszt_text)
    {
        return "\0";
    }

    xpvt_tconv[xit_iter_p] = (x_pvoid_t)xszt_text;

    //======================================
    // 格式输出操作

#ifdef _MSC_VER
#pragma warning(disable : 4018)
#endif // _MSC_VER

    for (xit_iter = 0, xct_iter_d = (x_uchar_t *)xct_dptr, xct_iter_v = xszt_text;
         xit_iter < (x_int32_t)xut_size;
         ++xit_iter, ++xct_iter_d)
    {
        *xct_iter_v++ = xszt_hex[*xct_iter_d >> 4];
        *xct_iter_v++ = xszt_hex[*xct_iter_d & 0x0F];
        *xct_iter_v++ = ' ';
    }

#ifdef _MSC_VER
#pragma warning(default : 4018)
#endif // _MSC_VER

    *(--xct_iter_v) = '\0';

    //======================================

    return xszt_text;
}

//====================================================================

// 
// x_log_tconv_t : constructor/destructor
// 

x_log_tconv_t::x_log_tconv_t(void)
    : m_xht_tconv(X_NULL)
{

}

x_log_tconv_t::~x_log_tconv_t(void)
{
    if (X_NULL != m_xht_tconv)
    {
        close_handle(m_xht_tconv);
        m_xht_tconv = X_NULL;
    }
}
