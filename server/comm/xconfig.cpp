/**
 * @file    xconfig.cpp
 * <pre>
 * Copyright (c) 2018, Gaaagaa All rights reserved.
 * 
 * 文件名称：xconfig.cpp
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

#include "xcomm.h"
#include "xconfig.h"

#include <fstream>
#include <json/json.h>

////////////////////////////////////////////////////////////////////////////////
// x_config_t

//====================================================================

// 
// x_config_t : common invoking
// 

/**********************************************************/
/**
 * @brief x_config_t 对象的单例调用接口。
 */
x_config_t & x_config_t::instance(void)
{
    static x_config_t _S_instance;
    return _S_instance;
}

//====================================================================

// 
// x_config_t : constructor/destructor
// 

x_config_t::x_config_t(void)
    : m_xht_handler(X_NULL)
{
    memset(m_xszt_fpath, 0, TEXT_LEN_4K);
}

x_config_t::~x_config_t(void)
{
    close();
}

//====================================================================

// 
// x_config_t : public interfaces
// 

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
x_int32_t x_config_t::open(x_cstring_t xszt_conf_file, x_pvoid_t xpvt_reserved)
{
    x_int32_t xit_error = -1;

    Json::Value * j_value_ptr = X_NULL;

    do
    {
        //======================================

        if (is_open())
        {
            close();
        }

        // 文件路径
        if (X_NULL == realpath(xszt_conf_file, m_xszt_fpath))
        {
            if (ENOENT != errno)
            {
                xit_error = errno;
                break;
            }
        }

        // Json 数据存储根节点
        j_value_ptr = new Json::Value();
        if (X_NULL == j_value_ptr)
        {
            xit_error = -1;
            break;
        }

        //======================================
        // 从文件读取 JSON 数据

        std::ifstream json_ifstream(m_xszt_fpath, std::ios::in | std::ios::binary);
        if (json_ifstream.is_open())
        {
            try
            {
                json_ifstream >> *j_value_ptr;
            }
            catch (...)
            {
                STD_TRACE("json file type error : %s", m_xszt_fpath);
                xit_error = -1;
                break;
            }
        }

        m_xht_handler = (x_handle_t)j_value_ptr;
        j_value_ptr = X_NULL;

        //======================================

        xit_error = 0;
    } while (0);

    if (X_NULL != j_value_ptr)
    {
        delete j_value_ptr;
        j_value_ptr = X_NULL;
    }

    return xit_error;
}

/**********************************************************/
/**
 * @brief 关闭 外部配置参数管理模块。
 */
x_void_t x_config_t::close(void)
{
    memset(m_xszt_fpath, 0, TEXT_LEN_4K);

    if (X_NULL != m_xht_handler)
    {
        delete (Json::Value *)m_xht_handler;
        m_xht_handler = X_NULL;
    }
}

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
x_int32_t x_config_t::read_int(x_cstring_t xszt_section, x_cstring_t xszt_keyname, x_int32_t xit_default)
{
    do
    {
        if (X_NULL == m_xht_handler)
        {
            break;
        }

        Json::Value & j_value = *(Json::Value *)m_xht_handler;
        if (!j_value.isMember(xszt_section))
        {
            break;
        }

        Json::Value & j_section = j_value[xszt_section];
        if (!j_section.isMember(xszt_keyname))
        {
            break;
        }

        Json::Value & j_key = j_section[xszt_keyname];
        xit_default = j_key.asInt();
    } while (0);

    return xit_default;
}

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
x_bool_t x_config_t::write_int(x_cstring_t xszt_section, x_cstring_t xszt_keyname, x_int32_t xit_value)
{
    x_bool_t xbt_ok = X_FALSE;

    do
    {
        //======================================

        if (X_NULL == m_xht_handler)
        {
            break;
        }

        Json::Value & j_value = *(Json::Value *)m_xht_handler;
        j_value[xszt_section][xszt_keyname] = xit_value;

        //======================================
        // 向文件写入 JSON 数据

        std::ofstream json_ofstream(m_xszt_fpath, std::ios::out | std::ios::trunc);
        if (json_ofstream.is_open())
        {
            try { json_ofstream << j_value; } catch (...) { }
        }

        //======================================

    } while (0);

    return xbt_ok;
}

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
x_cstring_t x_config_t::read_str(x_cstring_t xszt_section,
                                 x_cstring_t xszt_keyname,
                                 x_string_t  xszt_value,
                                 x_uint32_t  xut_size,
                                 x_cstring_t xszt_default)
{
    x_cstring_t xszt_string = xszt_default;

    do
    {
        if ((X_NULL == m_xht_handler) || (X_NULL == xszt_value) || (xut_size <= 0))
        {
            break;
        }

        Json::Value & j_value = *(Json::Value *)m_xht_handler;
        if (!j_value.isMember(xszt_section))
        {
            break;
        }

        Json::Value & j_section = j_value[xszt_section];
        if (!j_section.isMember(xszt_keyname))
        {
            break;
        }

        Json::Value & j_key = j_section[xszt_keyname];

        std::string xstr_value = j_key.asString();
        strncpy(xszt_value, xstr_value.c_str(), xut_size);

        xszt_string = xszt_value;
    } while (0);

    return xszt_string;
}

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
std::string x_config_t::read_str(x_cstring_t xszt_section, x_cstring_t xszt_keyname, x_cstring_t xszt_default)
{
    std::string xstr_value = (X_NULL != xszt_default) ? xszt_default : "";

    do
    {
        if (X_NULL == m_xht_handler)
        {
            break;
        }

        Json::Value & j_value = *(Json::Value *)m_xht_handler;
        if (!j_value.isMember(xszt_section))
        {
            break;
        }

        Json::Value & j_section = j_value[xszt_section];
        if (!j_section.isMember(xszt_keyname))
        {
            break;
        }

        Json::Value & j_key = j_section[xszt_keyname];
        xstr_value = j_key.asString();
    } while (0);

    return xstr_value;
}

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
x_bool_t x_config_t::write_str(x_cstring_t xszt_section, x_cstring_t xszt_keyname, x_cstring_t xszt_value)
{
    x_bool_t xbt_ok = X_FALSE;

    do
    {
        //======================================

        if (X_NULL == m_xht_handler)
        {
            break;
        }

        Json::Value & j_value = *(Json::Value *)m_xht_handler;
        j_value[xszt_section][xszt_keyname] = (X_NULL != xszt_value) ? xszt_value : "";

        //======================================
        // 向文件写入 JSON 数据

        std::ofstream json_ofstream(m_xszt_fpath, std::ios::out | std::ios::trunc);
        if (json_ofstream.is_open())
        {
            try { json_ofstream << j_value; } catch (...) { }
        }

        //======================================

    } while (0);

    return xbt_ok;
}

