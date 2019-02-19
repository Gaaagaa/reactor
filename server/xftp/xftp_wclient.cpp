/**
 * @file    xftp_wclient.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * 文件名称：xftp_wclient.cpp
 * 创建日期：2019年02月14日
 * 文件标识：
 * 文件摘要：为 xftp 的客户端连接提供基本操作的业务层工作对象。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年02月14日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#include "xcomm.h"
#include "xftp_wclient.h"
#include "xftp_server.h"

#include <json/json.h>
#include <sstream>

////////////////////////////////////////////////////////////////////////////////
// x_ftp_wclient_t

//====================================================================

// 
// x_ftp_wclient_t : constructor/destructor
// 

x_ftp_wclient_t::x_ftp_wclient_t(x_handle_t xht_manager, x_sockfd_t xfdt_sockfd)
    : x_super_t(xht_manager, xfdt_sockfd)
{

}

x_ftp_wclient_t::~x_ftp_wclient_t(void)
{

}

//====================================================================

// 
// x_ftp_wclient_t : overrides
// 

/**********************************************************/
/**
 * @brief 处理 “接收 IO 请求消息” 的事件。
 */
x_int32_t x_ftp_wclient_t::io_event_requested(x_tcp_io_message_t & xio_message)
{
    x_int32_t xit_error = 0;

    x_io_msgctxt_t xio_msgctxt;
    if (IOCTX_ERR_OK == io_context_rinfo(xio_message.data(), xio_message.rlen(), &xio_msgctxt))
    {
        switch (xio_msgctxt.io_cmid)
        {
        case CMID_WCLI_LOGIN : xit_error = iocmd_login(xio_msgctxt.io_seqn, xio_msgctxt.io_dptr, xio_msgctxt.io_size); break;
        case CMID_WCLI_HBEAT : xit_error = iocmd_hbeat(xio_msgctxt.io_seqn, xio_msgctxt.io_dptr, xio_msgctxt.io_size); break;
        case CMID_WCLI_FLIST : xit_error = iocmd_flist(xio_msgctxt.io_seqn, xio_msgctxt.io_dptr, xio_msgctxt.io_size); break;

        default:
            break;
        }
    }

    return xit_error;
}

/**********************************************************/
/**
 * @brief 处理 “完成 IO 应答消息” 的事件。
 */
x_int32_t x_ftp_wclient_t::io_event_responsed(x_tcp_io_message_t & xio_message)
{
    return 0;
}

/**********************************************************/
/**
 * @brief 处理 “IO 通道对象被销毁” 的事件。
 */
x_int32_t x_ftp_wclient_t::io_event_destroyed(void)
{
    return 0;
}

//====================================================================

// 
// x_ftp_wclient_t : internal invoking
// 

/**********************************************************/
/**
 * @brief 处理 IO 请求命令：登录。
 */
x_int32_t x_ftp_wclient_t::iocmd_login(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size)
{
    if (xut_size > 0)
        m_xstr_name = std::move(std::string((x_char_t *)xct_dptr, xut_size));
    else
        m_xstr_name = "{unknow}";

    x_io_msgctxt_t xio_msgctxt;
    xio_msgctxt.io_seqn = xut_seqn;
    xio_msgctxt.io_cmid = CMID_WCLI_LOGIN;
    xio_msgctxt.io_size = 0;
    xio_msgctxt.io_dptr = X_NULL;

    post_res_xmsg(xio_msgctxt);

    return 0;
}

/**********************************************************/
/**
 * @brief 处理 IO 请求命令：心跳。
 */
x_int32_t x_ftp_wclient_t::iocmd_hbeat(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size)
{
    x_io_msgctxt_t xio_msgctxt;
    xio_msgctxt.io_seqn = xut_seqn;
    xio_msgctxt.io_cmid = CMID_WCLI_HBEAT;
    xio_msgctxt.io_size = 0;
    xio_msgctxt.io_dptr = X_NULL;

    post_res_xmsg(xio_msgctxt);

    return 0;
}

/**********************************************************/
/**
 * @brief 处理 IO 请求命令：获取文件列表。
 */
x_int32_t x_ftp_wclient_t::iocmd_flist(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size)
{
    do
    {
        //======================================
        // 文件列表

        x_ftp_server_t::x_list_file_t xlst_filenames;
        x_ftp_server_t::instance().get_file_list(xlst_filenames, ECV_GET_MAX_FILES);

        if (xlst_filenames.empty())
        {
            LOGW("x_ftp_server_t::get_file_list(xlst_filenames, ...) return empty list!");
            break;
        }

        //======================================
        // 构造应答操作的 Json 字符串

        Json::Value j_list(Json::ValueType::arrayValue);
        for (x_ftp_server_t::x_list_file_t::iterator itlst = xlst_filenames.begin();
             itlst != xlst_filenames.end();
             ++itlst)
        {
            Json::Value j_file;
            j_file["file"] = itlst->first;
            j_file["size"] = itlst->second;

            j_list.append(j_file);
        }

        Json::StreamWriterBuilder j_builder;
        j_builder.settings_["indentation"            ] = "";
        j_builder.settings_["enableYAMLCompatibility"] = false;

        std::unique_ptr< Json::StreamWriter > j_writer(j_builder.newStreamWriter());

        std::ostringstream xstr_ostr;
        j_writer->write(j_list, &xstr_ostr);

        std::string xstr_jlist = xstr_ostr.str();
        if (xstr_jlist.size() >= 0x0000FFFF)
        {
            LOGW("xstr_jlist.size()[%d] >= 0x0000FFFF, it's too long!",
                 (x_int32_t)xstr_jlist.size());
            xstr_jlist = "";
        }

        //======================================
        // 投递应答信息

        x_io_msgctxt_t xio_msgctxt;
        xio_msgctxt.io_seqn = xut_seqn;
        xio_msgctxt.io_cmid = CMID_WCLI_FLIST;
        xio_msgctxt.io_size = (x_uint32_t)xstr_jlist.size();
        xio_msgctxt.io_dptr = (x_uchar_t *)const_cast< x_char_t * >(xstr_jlist.c_str());

        post_res_xmsg(xio_msgctxt);

        //======================================

    } while (0);

    return 0;
}
