/**
 * @file    xftp_client.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * �ļ����ƣ�xftp_client.cpp
 * �������ڣ�2019��02��18��
 * �ļ���ʶ��
 * �ļ�ժҪ��xftp �Ŀͻ��������ࡣ
 * 
 * ��ǰ�汾��1.0.0.0
 * ��    �ߣ�
 * ������ڣ�2019��02��18��
 * �汾ժҪ��
 * 
 * ȡ���汾��
 * ԭ����  ��
 * ������ڣ�
 * �汾ժҪ��
 * </pre>
 */

#include "stdafx.h"
#include "xftp_client.h"

#include <json/json.h>
#include <sstream>

 ////////////////////////////////////////////////////////////////////////////////
 // x_ftp_client_t

//====================================================================

// 
// x_ftp_client_t : constructor/destructor
// 

x_ftp_client_t::x_ftp_client_t(void)
    : m_xbt_login(X_FALSE)
{

}

x_ftp_client_t::~x_ftp_client_t(void)
{
    if (is_startup())
    {
        logout();
    }
}

//====================================================================

// 
// x_ftp_client_t : overrides
// 

/**********************************************************/
/**
 * @brief Ͷ���������ݰ���Ϣ��
 */
x_void_t x_ftp_client_t::send_heartbeat(void)
{
    x_io_msgctxt_t xio_msgctxt;
    xio_msgctxt.io_seqn = 0;
    xio_msgctxt.io_cmid = CMID_WCLI_HBEAT;
    xio_msgctxt.io_size = 0;
    xio_msgctxt.io_dptr = X_NULL;

    post_req_xmsg(xio_msgctxt, X_TRUE);
}

/**********************************************************/
/**
 * @brief �յ� IO ��Ϣ��֪ͨ�ӿڡ�
 */
x_void_t x_ftp_client_t::io_recved_msgctxt(const x_io_msgctxt_t & xio_msgctxt)
{
    switch (xio_msgctxt.io_cmid)
    {
    case CMID_WCLI_LOGIN: iocmd_login(xio_msgctxt.io_seqn, xio_msgctxt.io_dptr, xio_msgctxt.io_size); break;
    case CMID_WCLI_HBEAT: iocmd_hbeat(xio_msgctxt.io_seqn, xio_msgctxt.io_dptr, xio_msgctxt.io_size); break;
    case CMID_WCLI_FLIST: iocmd_flist(xio_msgctxt.io_seqn, xio_msgctxt.io_dptr, xio_msgctxt.io_size); break;

    default:
        break;
    }
}

/**********************************************************/
/**
 * @brief ��� IO ��Ϣ���͵�֪ͨ�ӿڡ�
 */
x_void_t x_ftp_client_t::io_sended_msgctxt(const x_io_msgctxt_t & xio_msgctxt)
{

}

/**********************************************************/
/**
 * @brief IO �������������֪ͨ�ӿڡ�
 */
x_void_t x_ftp_client_t::io_error_notify(x_int32_t xit_error, x_bool_t xbt_iosend)
{
    if (0 != xit_error)
    {
        x_msg_handler_t::instance().post_msg(EM_EST_CLIENT,
                                             XMKEY_WCLI_IOERR,
                                             X_NULL,
                                             sizeof(x_int32_t),
                                             (x_uchar_t *)&xit_error);
    }
}

//====================================================================

// 
// x_ftp_client_t : public interfaces
// 

/**********************************************************/
/**
 * @brief ��¼������
 *
 * @param [in ] xszt_host : Ŀ������������������ IP ��ַ���Ķ�ʽ IP ��ַ�ַ�������
 * @param [in ] xwt_port  : Ŀ������������������ �˿ںš�
 *
 * @return x_int32_t
 *         - �ɹ������� 0��
 *         - ʧ�ܣ����� �����롣
 */
x_int32_t x_ftp_client_t::login(x_cstring_t xszt_host, x_uint16_t xwt_port)
{
    x_int32_t xit_error = -1;

    do
    {
        //======================================

        logout();
        m_xbt_login = X_FALSE;

        //======================================
        // ��������

        enable_auto_heartbeat(X_FALSE);

        xit_error = x_ftp_cliworker_t::startup(xszt_host, xwt_port);
        if (0 != xit_error)
        {
            break;
        }

        //======================================
        // �ύ��¼��֤����

        x_io_msgctxt_t xio_msgctxt;
        xio_msgctxt.io_seqn = 0;
        xio_msgctxt.io_cmid = CMID_WCLI_LOGIN;
        xio_msgctxt.io_size = 0;
        xio_msgctxt.io_dptr = X_NULL;

        post_req_xmsg(xio_msgctxt, X_TRUE);

        //======================================
        xit_error = 0;
    } while (0);

    if (0 != xit_error)
    {
        logout();
    }

    return xit_error;
}

/**********************************************************/
/**
 * @brief �ǳ�������
 */
x_void_t x_ftp_client_t::logout(void)
{
    m_xbt_login = X_FALSE;
    x_ftp_cliworker_t::shutdown();
}

/**********************************************************/
/**
 * @brief �ӻ����ж�ȡ�ļ��б�
 */
x_void_t x_ftp_client_t::get_cache_flist(x_list_file_t & xlst_files)
{
    std::lock_guard< x_locker_t > xautolock(m_xlock_flist);
    xlst_files = m_cache_flist;
}

//====================================================================

// 
// x_ftp_client_t : internal invoking
// 

/**********************************************************/
/**
 * @brief ���� IO Ӧ����������������ӡ�
 */
x_int32_t x_ftp_client_t::iocmd_login(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size)
{
    m_xbt_login = X_TRUE;
    enable_auto_heartbeat(X_TRUE);
    return 0;
}

/**********************************************************/
/**
 * @brief ���� IO Ӧ�����������
 */
x_int32_t x_ftp_client_t::iocmd_hbeat(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size)
{
    return 0;
}

/**********************************************************/
/**
 * @brief ���� IO Ӧ������ı����ݷ��䡣
 */
x_int32_t x_ftp_client_t::iocmd_flist(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size)
{
    do 
    {
        //======================================
        // ����

        Json::Value  j_flist;

        try
        {
            std::istringstream xstr_istr(
                std::string((x_char_t *)xct_dptr, (x_char_t *)xct_dptr + xut_size));
            xstr_istr >> j_flist;
        }
        catch (...)
        {
        	break;
        }

        if (!j_flist.isArray())
        {
            break;
        }

        x_list_file_t xlst_file;

        for (x_uint32_t xut_iter = 0; xut_iter < j_flist.size(); ++xut_iter)
        {
            Json::Value & j_file = j_flist[xut_iter];

            if (j_file.isMember("file") && j_file.isMember("size"))
            {
                xlst_file.push_back(
                    std::make_pair(j_file["file"].asString(), (x_int64_t)j_file["size"].asInt64()));
            }
        }

        //======================================
        // ����

        {
            std::lock_guard< x_locker_t > xautolock(m_xlock_flist);
            m_cache_flist.swap(xlst_file);
        }

        //======================================
        // Ͷ�ݸ���֪ͨ

        x_msg_handler_t::instance().post_msg(EM_EST_CLIENT, XMKEY_WCLI_FLIST, X_NULL, 0, X_NULL);

        //======================================
    } while (0);

    return 0;
}
