/**
 * @file    xconfig.h
 * <pre>
 * Copyright (c) 2018, Gaaagaa All rights reserved.
 * 
 * 文件名称：xconfig.h
 * 创建日期：2018年12月30日
 * 文件标识：
 * 文件摘要：程序的配置参数读写类（单例模式调用）。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2018年12月30日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#ifndef __XCONFIG_H__
#define __XCONFIG_H__

////////////////////////////////////////////////////////////////////////////////
// x_config_t

/**
 * @class x_config_t
 * @brief 程序的配置参数读写类（单例模式调用）。
 */
class x_config_t
{
    // common invoking
public:
    /**********************************************************/
    /**
     * @brief x_config_t 对象的单例调用接口。
     */
    static x_config_t & instance(void);

    // constructor/destructor
private:
    explicit x_config_t(void);
    ~x_config_t(void);

    x_config_t(const x_config_t & xobject);
    x_config_t & operator=(const x_config_t & xobject);

    // public interfaces
public:
    /**********************************************************/
    /**
     * @brief 打开 外部配置参数管理模块。
     * 
     * @param [in ] xszt_conf_file : 外部参数配置文件的文件路径。
     * @param [in ] xpvt_reserved  : 保留参数（可以设置为 X_NULL）。
     * 
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t open(x_cstring_t xszt_conf_file, x_pvoid_t xpvt_reserved);

    /**********************************************************/
    /**
     * @brief 关闭 外部配置参数管理模块。
     */
    x_void_t close(void);

    /**********************************************************/
    /**
     * @brief 判断 外部配置参数管理模块 是否已经打开。
     */
    inline x_bool_t is_open(void) const { return (X_NULL != m_xht_handler); }

    /**********************************************************/
    /**
     * @brief 读取整数值。
     * 
     * @param [in ] xszt_section : 分节名称。
     * @param [in ] xszt_keyname : 索引键名称。
     * @param [in ] xit_default  : 读取失败时，返回的默认值。
     * 
     * @return x_int32_t
     *         - 成功，返回 存储的整数值；
     *         - 失败，返回 默认的整数值（xit_default）。
     */
    x_int32_t read_int(x_cstring_t xszt_section, x_cstring_t xszt_keyname, x_int32_t xit_default);

    /**********************************************************/
    /**
     * @brief 写入整数值。
     * 
     * @param [in ] xszt_section : 分节名称。
     * @param [in ] xszt_keyname : 索引键名称。
     * @param [in ] xit_value    : 待写入的整数值。
     * 
     * @return x_bool_t
     *         - 成功，返回 X_TRUE ；
     *         - 失败，返回 X_FALSE。
     */
    x_bool_t write_int(x_cstring_t xszt_section, x_cstring_t xszt_keyname, x_int32_t xit_value);

    /**********************************************************/
    /**
     * @brief 读取字符串。
     * 
     * @param [in ] xszt_section : 分节名称。
     * @param [in ] xszt_keyname : 索引键名称。
     * @param [in ] xszt_value   : 读取字符串的数据接收缓存。
     * @param [in ] xut_size     : 读取字符串的数据接收缓存大小。
     * @param [in ] xszt_default : 读取失败时，返回的默认值。
     * 
     * @return x_cstring_t
     *         - 成功，返回 存储的字符串（xszt_value 的地址）；
     *         - 失败，返回 默认的字符串（xszt_default 的地址）。
     */
    x_cstring_t read_str(x_cstring_t xszt_section,
                         x_cstring_t xszt_keyname,
                         x_string_t  xszt_value,
                         x_uint32_t  xut_size,
                         x_cstring_t xszt_default);

    /**********************************************************/
    /**
     * @brief 读取字符串。
     * 
     * @param [in ] xszt_section : 分节名称。
     * @param [in ] xszt_keyname : 索引键名称。
     * @param [in ] xszt_default : 读取失败时，返回的默认值。
     * 
     * @return std::string
     *         - 成功，返回 存储的字符串；
     *         - 失败，返回 默认的字符串（xszt_default）。
     */
    std::string read_str(x_cstring_t xszt_section, x_cstring_t xszt_keyname, x_cstring_t xszt_default);

    /**********************************************************/
    /**
     * @brief 写入字符串。
     * 
     * @param [in ] xszt_section : 分节名称。
     * @param [in ] xszt_keyname : 索引键名称。
     * @param [in ] xszt_value   : 待写入的字符串。
     * 
     * @return x_bool_t
     *         - 成功，返回 X_TRUE ；
     *         - 失败，返回 X_FALSE。
     */
    x_bool_t write_str(x_cstring_t xszt_section, x_cstring_t xszt_keyname, x_cstring_t xszt_value);

    // data members
private:
    x_char_t     m_xszt_fpath[TEXT_LEN_4K]; ///< 存储配置文件的绝对路径
    x_handle_t   m_xht_handler;             ///< 控制句柄对象
};

////////////////////////////////////////////////////////////////////////////////

#endif // __XCONFIG_H__
