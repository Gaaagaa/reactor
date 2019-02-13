/**
 * @file    xlog_handler.h
 * <pre>
 * Copyright (c) 2018, Gaaagaa All rights reserved.
 * 
 * 文件名称：xlog_handler.h
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

#ifndef __XLOG_HANDLER_H__
#define __XLOG_HANDLER_H__

////////////////////////////////////////////////////////////////////////////////
// x_log_handler_t

/**
 * @class x_log_handler_t
 * @brief 日志输出的操作类（单例模式调用）。
 */
class x_log_handler_t
{
    // common data types
public:
    /**
     * @enum  emOutputType
     * @brief 日志输出格式类型的枚举值。
     */
    typedef enum emOutputType
    {
        EMOT_INFO   = 0x0010,     ///< 常规信息
        EMOT_DEBUG  = 0x0020,     ///< 调试信息
        EMOT_ERROR  = 0x0030,     ///< 错误信息
        EMOT_WARN   = 0x0040,     ///< 告警信息
    } emOutputType;

    // common invoking
public:
    /**********************************************************/
    /**
     * @brief x_log_handler_t 对象的单例调用接口。
     */
    static x_log_handler_t & instance(void);

    // constructor/destructor
private:
    explicit x_log_handler_t(void);
    ~x_log_handler_t(void);

    x_log_handler_t(const x_log_handler_t & xobject);
    x_log_handler_t & operator=(const x_log_handler_t & xobject);

    // public interfaces
public:
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
    x_int32_t open(x_cstring_t xszt_conf_file, x_pvoid_t xpvt_reserved);

    /**********************************************************/
    /**
     * @brief 关闭日志输出模块。
     */
    x_void_t close(void);

    /**********************************************************/
    /**
     * @brief 判断日志输出模块是否已经打开。
     */
    inline x_bool_t is_open(void) const { return (X_NULL != m_xht_handler); }

    /**********************************************************/
    /**
     * @brief 输出日志信息。
     * 
     * @param [in ] xot_property : 输出信息的属性（参看 emOutputType 枚举值）。
     * @param [in ] xszt_format  : 输出信息的格式。
     * @param [in ] ...          : 输出信息的相关参数列表。
     */
    x_void_t output(emOutputType xot_property, x_cstring_t xszt_format, ...);

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
    x_void_t output_ex(emOutputType xot_property,
                       x_cstring_t xszt_filename,
                       x_int32_t   xit_codeline,
                       x_cstring_t xszt_function,
                       x_cstring_t xszt_format,
                       ...);

    // data members
private:
    x_handle_t   m_xht_handler;    ///< 控制句柄对象
};

////////////////////////////////////////////////////////////////////////////////
// x_log_tconv_t

/** 定义 x_log_tconv_t 对象最大容纳的缓存数量 */
#ifndef LOG_TCONV_MAX_SIZE
#define LOG_TCONV_MAX_SIZE 8
#endif // LOG_TCONV_MAX_SIZE

/**
 * @class x_log_tconv_t
 * @brief 负责日志输出文本所使用的临时操作缓存。
 */
class x_log_tconv_t
{
	// constructor/destructor
public:
	x_log_tconv_t(void);
	~x_log_tconv_t(void);

    // common invoking
public:
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
	static x_handle_t open_handle(x_int32_t xit_max_tconv);

	/**********************************************************/
	/**
	 * @brief 关闭字符串缓存操作句柄。
	 */
	static x_void_t close_handle(x_handle_t xht_tconv);

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
	static x_string_t alloc_buffer(x_handle_t xht_tconv, x_uint32_t xut_size);

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
    static x_cstring_t hex_dump(x_handle_t xht_tconv, const x_uchar_t * xct_dptr, x_uint32_t xut_size);

	// operator handle
public:
	operator x_handle_t(void)
	{
		if (X_NULL == m_xht_tconv)
			m_xht_tconv = open_handle(LOG_TCONV_MAX_SIZE);
		return m_xht_tconv;
	}

	// data members
protected:
	x_handle_t   m_xht_tconv;  ///< 操作句柄
};

////////////////////////////////////////////////////////////////////////////////

/** 用于宏输出接口内 字符串申请缓存 */
#define LOG_BUF(xut_size)            x_log_tconv_t::alloc_buffer((x_handle_t)(xht_tconv), (xut_size))

/** 用于宏输出接口内 生成十六进制格式字符串 */
#define LOG_HEX(xct_dptr, xut_size)  x_log_tconv_t::hex((x_handle_t)(xht_tconv), (xct_dptr), (xut_size))

/** 使用标准设备输出信息 */
#define STD_TRACE(xszt_format, ...)               \
        do                                        \
        {                                         \
            x_log_tconv_t xht_tconv;              \
            printf((xszt_format), ##__VA_ARGS__); \
            printf("\n");                         \
        } while (0)                               \

////////////////////////////////////////////////////////////////////////////////

/** 是否在 LOGH_OUTPUT() 宏内嵌入 标准设备输出信息 */
#ifndef ENABLE_LOG_EMBED_STD
#define ENABLE_LOG_EMBED_STD 0
#endif // ENABLE_LOG_EMBED_STD

#if ENABLE_LOG_EMBED_STD

/** LOGH_OUTPUT() 宏内嵌入的 标准设备输出信息 */
#define LOGH_STD(xszt_format, ...)                \
        do                                        \
        {                                         \
            x_log_tconv_t xht_tconv;              \
            printf("[%s, %d, %s]",                \
               __FILE__, __LINE__, __FUNCTION__); \
            printf((xszt_format), ##__VA_ARGS__); \
            printf("\n");                         \
        } while (0)                               \

#else // !ENABLE_LOG_EMBED_STD

/** LOGH_OUTPUT() 宏内嵌入的 标准设备输出信息 */
#define LOGH_STD(xszt_format, ...)

#endif // ENABLE_LOG_EMBED_STD

//====================================================================

/** 是否启用日志输出宏 */
#ifndef ENABLE_LOG_OUTPUT
#if ((defined _DEBUG) || (defined DEBUG))
#define ENABLE_LOG_OUTPUT 1
#else // ((defined _DEBUG) || (defined DEBUG))
#define ENABLE_LOG_OUTPUT 0
#endif // #if ((defined _DEBUG) || (defined DEBUG))
#endif // ENABLE_LOG_OUTPUT

#if ENABLE_LOG_OUTPUT

/** 打开日志输出模块 */
#define LOGH_OPEN(xszt_conf_file, xpvt_reserved)  x_log_handler_t::instance().open((xszt_conf_file), (xpvt_reserved))

/** 关闭日志输出模块 */
#define LOGH_CLOSE()             do { x_log_handler_t::instance().close(); } while (0)

/** 是否启用扩展的日志输出接口 */
#ifndef ENABLE_LOG_OUTPUTEX
#define ENABLE_LOG_OUTPUTEX 1
#endif // ENABLE_LOG_OUTPUTEX

#if ENABLE_LOG_OUTPUTEX

/** 输出日志信息 */
#define LOGH_OUTPUT(xot_property, xszt_format, ...)         \
        do                                                  \
        {                                                   \
            LOGH_STD(xszt_format, ##__VA_ARGS__);           \
            x_log_tconv_t xht_tconv;                        \
            x_log_handler_t::instance().output_ex(          \
                                            xot_property,   \
                                            __FILE__,       \
                                            __LINE__,       \
                                            __FUNCTION__,   \
                                            (xszt_format),  \
                                            ##__VA_ARGS__); \
        } while (0)                                         \

#else // !ENABLE_LOG_OUTPUTEX

/** 输出日志信息 */
#define LOGH_OUTPUT(xot_property, xszt_format, ...)         \
        do                                                  \
        {                                                   \
            LOGH_STD(xszt_format, ##__VA_ARGS__);           \
            x_log_tconv_t xht_tconv;                        \
            x_log_handler_t::instance().output(             \
                                            xot_property,   \
                                            (xszt_format),  \
                                            ##__VA_ARGS__); \
        } while (0)                                         \

#endif // ENABLE_LOG_OUTPUTEX

/** 输出常规信息 */
#define LOGI(xszt_format, ...)   LOGH_OUTPUT(x_log_handler_t::EMOT_INFO , (xszt_format), ##__VA_ARGS__)

/** 输出调试信息 */
#define LOGD(xszt_format, ...)   LOGH_OUTPUT(x_log_handler_t::EMOT_DEBUG, (xszt_format), ##__VA_ARGS__)

/** 输出出错信息 */
#define LOGE(xszt_format, ...)   LOGH_OUTPUT(x_log_handler_t::EMOT_ERROR, (xszt_format), ##__VA_ARGS__)

/** 输出警告信息 */
#define LOGW(xszt_format, ...)   LOGH_OUTPUT(x_log_handler_t::EMOT_WARN , (xszt_format), ##__VA_ARGS__)

#else // !ENABLE_LOG_OUTPUT

/** 打开日志输出模块 */
#define LOGH_OPEN(xszt_conf_file, xpvt_reserved)    0

/** 关闭日志输出模块 */
#define LOGH_CLOSE()

/** 输出日志信息 */
#define LOGH_OUTPUT(xot_property, xszt_format, ...)

/** 输出常规信息 */
#define LOGI(xszt_format, ...)

/** 输出调试信息 */
#define LOGD(xszt_format, ...)

/** 输出出错信息 */
#define LOGE(xszt_format, ...)

/** 输出警告信息 */
#define LOGW(xszt_format, ...)

#endif // ENABLE_LOG_OUTPUT

//====================================================================

#ifdef ENABLE_LOG_FUNC
#define LOGI_FUNC()              LOGI("invoking")
#else // !ENABLE_LOG_FUNC
#define LOGI_FUNC()
#endif // ENABLE_LOG_FUNC

////////////////////////////////////////////////////////////////////////////////

#endif // __XLOG_HANDLER_H__
