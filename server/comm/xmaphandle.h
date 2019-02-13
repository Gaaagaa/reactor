/**
 * @file    xmaptable.h
 * <pre>
 * Copyright (c) 2015, Gaaagaa All rights reserved.
 * 
 * 文件名称：xmaptable.h
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

#ifndef __XMAPHANDLE_H__
#define __XMAPHANDLE_H__

#include "xtypes.h"

////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

////////////////////////////////////////////////////////////////////////////////

#define MAPTBL_INVALID_ITEMKEY  ((x_size_t)-1)
#define MAPTBL_INVALID_HANDLER  ((x_handle_t)-1)
#define MAPTBL_TIMEOUT_INFINIT  ((x_size_t)0xFFFFFFFF)

/**
 * @enum  maptbl_error_table
 * @brief 映射表操作的错误码表枚举值。
 */
typedef enum maptbl_error_table
{
    MAPTBL_ERR_SUCCESS  = 0x00000000,   ///< 操作成功
    MAPTBL_ERR_HANDLE   = 0x00000100,   ///< 映射表的操作句柄无效
    MAPTBL_ERR_TIMEOUT  = 0x00000110,   ///< 操作超时
    MAPTBL_ERR_NOTFOUND = 0x00000120,   ///< 索引键未找到
    MAPTBL_ERR_OPERATOR = 0x00000130,   ///< 操作无效
    MAPTBL_ERR_OUTOFMEM = 0x00000140,   ///< 内存申请失败
    MAPTBL_ERR_NITEMKEY = 0x00000150,   ///< 索引键值为 MAPTBL_INVALID_ITEMKEY
    MAPTBL_ERR_NHANDLER = 0x00000160,   ///< 设置映射的句柄参数为 MAPTBL_INVALID_HANDLER
    MAPTBL_ERR_VRELATED = 0x00000170,   ///< 索引键已关联了句柄参数
} maptbl_error_table;

/**
 * @brief 映射表子项遍历操作的回调函数类型。
 * 
 * @param [in    ] xht_maptbl  : 映射表的操作句柄。
 * @param [in    ] xst_itemkey : 子项索引键。
 * @param [in,out] xht_handler : 子项关联的映射句柄（可进行回参设置，若回参为 MAPTBL_INVALID_HANDLER，则删除子项）。
 * @param [in    ] xht_context : 回调所设置的上下文标识句柄。
 * 
 * @return x_bool_t
 *         - 返回 X_TRUE ，继续遍历操作；
 *         - 返回 X_FALSE，则终止遍历操作。
 */
typedef x_bool_t (* maptbl_trav_callback)(x_handle_t   xht_maptbl,
                                          x_size_t     xst_itemkey,
                                          x_handle_t * xht_handler,
                                          x_handle_t   xht_context);

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
x_handle_t maptbl_create(x_size_t xst_capacity);

/**********************************************************/
/**
 * @brief 销毁 映射表 对象。
 * 
 * @param [in ] xht_maptbl : 映射表的操作句柄（即 maptbl_create() 的返回值）。
 * 
 */
x_void_t maptbl_destroy(x_handle_t xht_maptbl);

/**********************************************************/
/**
 * @brief 读取映射表中所映射的子项数量。
 * 
 * @param [in ] xht_maptbl : 映射表的操作句柄（即 maptbl_create() 的返回值）。
 * 
 * @return x_size_t
 *         - 返回 子项数量。
 */
x_size_t maptbl_count(x_handle_t xht_maptbl);

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
x_int32_t maptbl_lock(x_handle_t xht_maptbl, x_size_t xst_itemkey, x_handle_t * xht_handler, x_size_t xst_timeout);

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
x_int32_t maptbl_unlock(x_handle_t xht_maptbl, x_size_t xst_itemkey);

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
x_int32_t maptbl_insert(x_handle_t xht_maptbl, x_size_t xst_itemkey, x_handle_t xht_handler, x_size_t xst_timeout);

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
x_int32_t maptbl_update(x_handle_t xht_maptbl, x_size_t xst_itemkey, x_handle_t xht_handler, x_size_t xst_timeout);

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
x_int32_t maptbl_query(x_handle_t xht_maptbl, x_size_t xst_itemkey, x_handle_t * xht_handler, x_size_t xst_timeout);

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
x_int32_t maptbl_delete(x_handle_t xht_maptbl, x_size_t xst_itemkey, x_size_t xst_timeout);

/**********************************************************/
/**
 * @brief 清除映射表中所有子项。
 * 
 * @param [in ] xht_maptbl  : 映射表的操作句柄（即 maptbl_create() 的返回值）。
 * 
 * @return x_int32_t
 *         - 返回 错误码值（参看 maptbl_error_table 枚举值）。
 */
x_int32_t maptbl_cleanup(x_handle_t xht_maptbl);

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
x_size_t maptbl_first(x_handle_t xht_maptbl, x_size_t xst_timeout);

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
x_size_t maptbl_next(x_handle_t xht_maptbl, x_size_t xst_itemkey, x_size_t xst_timeout);

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
x_size_t maptbl_trav(x_handle_t xht_maptbl, x_size_t xst_nextkey, x_size_t xst_timeout, maptbl_trav_callback xfunc_ptr, x_handle_t xht_context);

////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}; // extern "C"
#endif // __cplusplus

////////////////////////////////////////////////////////////////////////////////

#endif // __XMAPHANDLE_H__
