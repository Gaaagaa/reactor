/**
 * @file    xftp_download.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * �ļ����ƣ�xftp_download.cpp
 * �������ڣ�2019��02��22��
 * �ļ���ʶ��
 * �ļ�ժҪ��XFTP �Ŀͻ����ļ����ع����ࡣ
 * 
 * ��ǰ�汾��1.0.0.0
 * ��    �ߣ�
 * ������ڣ�2019��02��22��
 * �汾ժҪ��
 * 
 * ȡ���汾��
 * ԭ����  ��
 * ������ڣ�
 * �汾ժҪ��
 * </pre>
 */

#include "stdafx.h"
#include "xftp_download.h"

////////////////////////////////////////////////////////////////////////////////
// x_ftp_download_t

//====================================================================

// 
// x_ftp_download_t : constructor/destructor
// 

x_ftp_download_t::x_ftp_download_t(void)
    : m_xbt_login(X_TRUE)
    , m_xstr_fpath("")
    , m_xit_fsize(0)
    , m_xbt_pause(X_FALSE)
    , m_xht_fstream(INVALID_HANDLE_VALUE)
{

}

x_ftp_download_t::~x_ftp_download_t(void)
{
    if (is_startup())
    {
        logout();
    }
}

//====================================================================

// 
// x_ftp_download_t : overrides
// 

/**********************************************************/
/**
 * @brief Ͷ���������ݰ���Ϣ��
 */
x_void_t x_ftp_download_t::send_heartbeat(void)
{
    x_io_msgctxt_t xio_msgctxt;
    xio_msgctxt.io_seqn = 0;
    xio_msgctxt.io_cmid = CMID_DLOAD_HBEAT;
    xio_msgctxt.io_size = 0;
    xio_msgctxt.io_dptr = X_NULL;

    post_req_xmsg(xio_msgctxt, X_TRUE);
}

/**********************************************************/
/**
 * @brief �յ� IO ��Ϣ��֪ͨ�ӿڡ�
 */
x_void_t x_ftp_download_t::io_recved_msgctxt(const x_io_msgctxt_t & xio_msgctxt)
{
    switch (xio_msgctxt.io_cmid)
    {
    case CMID_DLOAD_LOGIN: iocmd_login(xio_msgctxt.io_seqn, xio_msgctxt.io_dptr, xio_msgctxt.io_size); break;
    case CMID_DLOAD_HBEAT: iocmd_hbeat(xio_msgctxt.io_seqn, xio_msgctxt.io_dptr, xio_msgctxt.io_size); break;
    case CMID_DLOAD_CHUNK: iocmd_chunk(xio_msgctxt.io_seqn, xio_msgctxt.io_dptr, xio_msgctxt.io_size); break;
    case CMID_DLOAD_PAUSE: iocmd_pause(xio_msgctxt.io_seqn, xio_msgctxt.io_dptr, xio_msgctxt.io_size); break;

    default:
        break;
    }
}

/**********************************************************/
/**
 * @brief ��� IO ��Ϣ���͵�֪ͨ�ӿڡ�
 */
x_void_t x_ftp_download_t::io_sended_msgctxt(const x_io_msgctxt_t & xio_msgctxt)
{

}

/**********************************************************/
/**
 * @brief IO �������������֪ͨ�ӿڡ�
 */
x_void_t x_ftp_download_t::io_error_notify(x_int32_t xit_error, x_bool_t xbt_iosend)
{
    if (0 != xit_error)
    {
        x_msg_handler_t::instance().post_msg(EM_EST_CLIENT,
                                             XMKEY_DLOAD_IOERR,
                                             X_NULL,
                                             sizeof(x_int32_t),
                                             (x_uchar_t *)&xit_error);
    }
}

//====================================================================

// 
// x_ftp_download_t : public interfaces
// 

/**********************************************************/
/**
 * @brief ��¼������
 *
 * @param [in ] xszt_host  : Ŀ������������������ IP ��ַ���Ķ�ʽ IP ��ַ�ַ�������
 * @param [in ] xwt_port   : Ŀ������������������ �˿ںš�
 * @param [in ] xszt_fpath : Ҫ���ص��ļ�·������
 * @param [in ] xit_fsize  : Ҫ���ص��ļ���С��
 *
 * @return x_int32_t
 *         - �ɹ������� 0��
 *         - ʧ�ܣ����� �����롣
 */
x_int32_t x_ftp_download_t::login(x_cstring_t xszt_host, x_uint16_t xwt_port, x_cstring_t xszt_fpath, x_int64_t xit_fsize)
{
    x_int32_t xit_error = -1;

    do
    {
        //======================================

        logout();
        m_xbt_login = X_FALSE;

        if ((xit_fsize <= 0) || (X_NULL == xszt_fpath) || ('\0' == *xszt_fpath))
        {
            break;
        }

        //======================================
        // ��������

        enable_auto_heartbeat(X_FALSE);

        xit_error = x_ftp_cliworker_t::startup(xszt_host, xwt_port);
        if (0 != xit_error)
        {
            break;
        }

        //======================================

        m_xbt_login  = X_FALSE;
        m_xstr_fpath = xszt_fpath;
        m_xit_fsize  = xit_fsize;
        m_xbt_pause  = X_FALSE;

        //======================================
        // �ύ��¼��֤����

        x_cstring_t xszt_fname = file_base_name(xszt_fpath);
        x_uint32_t  xut_length = (x_uint32_t)strlen(xszt_fname);

        x_io_msgctxt_t xio_msgctxt;
        xio_msgctxt.io_seqn = 0;
        xio_msgctxt.io_cmid = CMID_DLOAD_LOGIN;
        xio_msgctxt.io_size = xut_length;
        xio_msgctxt.io_dptr = (x_uchar_t *)xszt_fname;

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
x_void_t x_ftp_download_t::logout(void)
{
    m_xbt_login  = X_FALSE;
    m_xstr_fpath = "";
    m_xit_fsize  = 0;
    m_xbt_pause  = X_FALSE;

    shutdown();

    if (INVALID_HANDLE_VALUE != m_xht_fstream)
    {
        CloseHandle(m_xht_fstream);
        m_xht_fstream = INVALID_HANDLE_VALUE;
    }
}

//====================================================================

// 
// x_ftp_download_t : internal invoking
// 

/**********************************************************/
/**
 * @brief Ͷ���ļ������������
 *
 * @param [in ] xit_offset  : �ļ���ƫ��λ�á�
 * @param [in ] xut_chksize : �ļ����С��
 * @param [in ] xbt_pause   : ��ͣ��ʶ��
 *
 * @return x_int32_t
 *         - �ɹ������� 0��
 *         - ʧ�ܣ����� �����롣
 */
x_int32_t x_ftp_download_t::post_req_chunk(x_int64_t xit_offset, x_uint32_t xut_chksize, x_bool_t xbt_pause)
{
    // [�ļ���ȡƫ��λ�� 8byte + ������ļ��鳤�� 4byte + ��ͣ��ʶ 4byte]
    x_uint32_t xut_io_size = sizeof(x_int64_t) + sizeof(x_uint32_t) + sizeof(x_uint32_t);
    x_uchar_t  xct_io_dptr[sizeof(x_int64_t) + sizeof(x_uint32_t) + sizeof(x_uint32_t)];

    x_uchar_t * xct_vptr = xct_io_dptr;

    // �ļ���ȡƫ��λ��
    *(x_int64_t *)(xct_vptr) = (x_int64_t)vx_htonll(xit_offset);
    xct_vptr += sizeof(x_int64_t);

    // ������ļ��鳤��
    *(x_uint32_t *)(xct_vptr) = (x_uint32_t)vx_htonl(xut_chksize);
    xct_vptr += sizeof(x_uint32_t);

    // ��ͣ��ʶ
    *(x_uint32_t *)(xct_vptr) = (x_uint32_t)vx_htonl(xbt_pause);

    // IO ������������Ϣ
    x_io_msgctxt_t xio_msgctxt;
    xio_msgctxt.io_seqn = 0;
    xio_msgctxt.io_cmid = CMID_DLOAD_CHUNK;
    xio_msgctxt.io_size = xut_io_size;
    xio_msgctxt.io_dptr = xct_io_dptr;

    return post_req_xmsg(xio_msgctxt, X_TRUE);
}

//====================================================================

// 
// x_ftp_download_t : io event handlers
// 

/**********************************************************/
/**
 * @brief ���� IO Ӧ����������������ӡ�
 */
x_int32_t x_ftp_download_t::iocmd_login(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size)
{
    x_int32_t xit_error = -1;

    do 
    {
        //======================================
        // Ӧ��Ĵ�������֤

        if (sizeof(x_int32_t) == xut_size)
        {
            xit_error = (x_int32_t)vx_ntohl(*(x_int32_t *)xct_dptr);
        }

        if (0 != xit_error)
        {
            break;
        }

        //======================================
        // �򿪱����ļ������ڽ����ļ�������

        if (!MkFilePathDirA(m_xstr_fpath.c_str()))
        {
            xit_error = -1;
            break;
        }

        XASSERT(INVALID_HANDLE_VALUE == m_xht_fstream);
        m_xht_fstream = CreateFileA(m_xstr_fpath.c_str(),
                                    GENERIC_WRITE,
                                    FILE_SHARE_WRITE | FILE_SHARE_READ,
                                    NULL,
                                    CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);
        if (INVALID_HANDLE_VALUE == m_xht_fstream)
        {
            xit_error = -1;
            break;
        }

        //======================================
        // Ͷ����������

        post_req_chunk(0LL, ECV_NOR_CHUNK_SIZE, X_FALSE);

        //======================================

        m_xbt_login = X_TRUE;
        enable_auto_heartbeat(X_TRUE);
        xit_error = 0;

        //======================================
    } while (0);

    if (0 != xit_error)
    {
        io_error_notify(xit_error, X_FALSE);
    }

    return 0;
}

/**********************************************************/
/**
 * @brief ���� IO Ӧ�����������
 */
x_int32_t x_ftp_download_t::iocmd_hbeat(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size)
{
    return 0;
}

/**********************************************************/
/**
 * @brief ���� IO Ӧ����������ļ��顣
 */
x_int32_t x_ftp_download_t::iocmd_chunk(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size)
{
    x_int32_t xit_error = -1;
    x_bool_t  xbt_write = X_FALSE;

    do 
    {
        //======================================
        // �������ж�

        if ((INVALID_HANDLE_VALUE == m_xht_fstream) || (X_NULL == xct_dptr) || (xut_size < sizeof(x_int32_t)))
        {
            xit_error = -1;
            break;
        }

        if (sizeof(x_int32_t) == xut_size)
        {
            xit_error = (x_int32_t)vx_ntohl(*(x_int32_t *)xct_dptr);
            break;
        }

        //======================================
        // �ļ�����Ϣ����

        if ((xut_size < (sizeof(x_int64_t) + sizeof(x_uint32_t))))
        {
            xit_error = -1;
            break;
        }

        x_uchar_t * xct_vptr = xct_dptr;

        x_int64_t  xit_offset = (x_int64_t )vx_ntohll(*(x_int64_t  *)xct_vptr); xct_vptr += sizeof(x_int64_t );
        x_uint32_t xut_lchunk = (x_uint32_t)vx_ntohl (*(x_uint32_t *)xct_vptr); xct_vptr += sizeof(x_uint32_t);

        //======================================
        // д���ļ�������

        LARGE_INTEGER xoffset;
        xoffset.QuadPart = xit_offset;
        if (!SetFilePointerEx(m_xht_fstream, xoffset, X_NULL, FILE_BEGIN))
        {
            xit_error = -1;
            break;
        }

        xit_error = 0;

        x_ulong_t xut_count = 0;
        while (xut_count < xut_lchunk)
        {
            x_ulong_t xut_wbytes = 0;
            if (!WriteFile(m_xht_fstream, xct_vptr + xut_count, xut_lchunk - xut_count, &xut_wbytes, NULL))
            {
                xit_error = -1;
                break;
            }

            xut_count += xut_wbytes;
            if (0 == xut_wbytes)
                break;
        }

        if (xut_count != xut_lchunk)
        {
            xit_error = -1;
            break;
        }

        if (0 != xit_error)
        {
            break;
        }

        xbt_write = X_TRUE;

        //======================================
        xit_error = 0;
    } while (0);

    if ((0 != xit_error) || !xbt_write)
    {
        io_error_notify(xit_error, X_FALSE);
    }

    return 0;
}

/**********************************************************/
/**
 * @brief ���� IO Ӧ�������ͣ��
 */
x_int32_t x_ftp_download_t::iocmd_pause(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size)
{
    if (sizeof(x_uint32_t) == xut_size)
    {
        m_xbt_pause = (0 != vx_ntohl(*(x_ulong_t *)xct_dptr)) ? X_TRUE : X_FALSE;

        x_msg_handler_t::instance().post_msg(
            EM_EST_CLIENT, XMKEY_DLOAD_PAUSE, X_NULL, sizeof(x_bool_t), (x_uchar_t *)&m_xbt_pause);
    }

    return 0;
}
