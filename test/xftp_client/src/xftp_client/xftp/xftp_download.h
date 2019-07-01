/**
 * @file    xftp_download.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * �ļ����ƣ�xftp_download.h
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

#ifndef __XFTP_DOWNLOAD_H__
#define __XFTP_DOWNLOAD_H__

#include "xftp_cliworker.h"
#include <fstream>

////////////////////////////////////////////////////////////////////////////////
// x_ftp_download_t

/**
 * @class x_ftp_download_t
 * @brief XFTP �Ŀͻ����ļ����ع����ࡣ
 */
class x_ftp_download_t : public x_ftp_cliworker_t
{
    // common data types
public:
    /**
     * @enum  emConstValue
     * @brief ��ص�ö�ٳ���ֵ��
     */
    typedef enum emConstValue
    {
        ECV_NOR_CHUNK_SIZE = 32 * 1024,  ///< IO �������ļ��鳣�泤��
        ECV_MAX_CHUNK_SIZE = 60 * 1024,  ///< IO �������ļ�����󳤶�
    } emConstValue;

    /**
     * @enum  emIoContextCmid
     * @brief IO ���������� ID ��
     */
    typedef enum emIoContextCmid
    {
        CMID_DLOAD_LOGIN = 0x0030,  ///< ��¼
        CMID_DLOAD_HBEAT = 0x2000,  ///< ����
        CMID_DLOAD_CHUNK = 0x2010,  ///< �����ļ���
        CMID_DLOAD_PAUSE = 0x2020,  ///< ��ͣ����
    } emIoContextCmid;

    // constructor/destructor
public:
    x_ftp_download_t(void);
    virtual ~x_ftp_download_t(void);

    // overrides
public:
    /**********************************************************/
    /**
     * @brief �������͡�
     */
    virtual x_uint16_t ctype(void) const
    {
        return CMID_DLOAD_LOGIN;
    }

protected:
    /**********************************************************/
    /**
     * @brief Ͷ���������ݰ���Ϣ��
     */
    virtual x_void_t send_heartbeat(void) override;

    /**********************************************************/
    /**
     * @brief �յ� IO ��Ϣ��֪ͨ�ӿڡ�
     */
    virtual x_void_t io_recved_msgctxt(const x_io_msgctxt_t & xio_msgctxt) override;

    /**********************************************************/
    /**
     * @brief ��� IO ��Ϣ���͵�֪ͨ�ӿڡ�
     */
    virtual x_void_t io_sended_msgctxt(const x_io_msgctxt_t & xio_msgctxt) override;

    /**********************************************************/
    /**
     * @brief IO �������������֪ͨ�ӿڡ�
     */
    virtual x_void_t io_error_notify(x_int32_t xit_error, x_bool_t xbt_iosend) override;

    // public interfaces
public:
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
    x_int32_t login(x_cstring_t xszt_host, x_uint16_t xwt_port, x_cstring_t xszt_fpath, x_int64_t xit_fsize);

    /**********************************************************/
    /**
     * @brief �ǳ�������
     */
    x_void_t logout(void);

    /**********************************************************/
    /**
     * @brief �Ƿ��Ѿ���¼��
     */
    inline x_bool_t is_login(void) const { return (is_startup() && m_xbt_login); }

    // internal invoking
protected:
    /**********************************************************/
    /**
     * @brief Ͷ���ļ������������
     *
     * @param [in ] xit_offset  : �ļ���ƫ��λ�á�
     * @param [in ] xit_chksize : �ļ����С��
     *
     * @return x_int32_t
     *         - �ɹ������� 0��
     *         - ʧ�ܣ����� �����롣
     */
    x_int32_t post_req_chunk(x_int64_t xit_offset, x_int64_t xit_chksize);

    // io event handlers
protected:
    /**********************************************************/
    /**
     * @brief ���� IO Ӧ����������������ӡ�
     */
    x_int32_t iocmd_login(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size);

    /**********************************************************/
    /**
     * @brief ���� IO Ӧ�����������
     */
    x_int32_t iocmd_hbeat(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size);

    /**********************************************************/
    /**
     * @brief ���� IO Ӧ����������ļ��顣
     */
    x_int32_t iocmd_chunk(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size);

    /**********************************************************/
    /**
     * @brief ���� IO Ӧ�������ͣ��
     */
    x_int32_t iocmd_pause(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size);

    // data members
protected:
    x_bool_t      m_xbt_login;    ///< ��ʶ��¼�ɹ����������ӽ����ɹ���

    std::string   m_xstr_fpath;   ///< �ļ�·����
    x_int64_t     m_xit_fsize;    ///< �ļ���С
    x_bool_t      m_xbt_pause;    ///< ��ͣ��ʶ

    std::ofstream m_xht_fstream;  ///< �ļ�д��������
};

////////////////////////////////////////////////////////////////////////////////

#define XMKEY_DLOAD_IOERR   ((x_uint32_t)(x_ftp_download_t::CMID_DLOAD_LOGIN << 16) | (x_uint32_t)x_ftp_download_t::CMID_DLOAD_LOGIN)
#define XMKEY_DLOAD_CHUNK   ((x_uint32_t)(x_ftp_download_t::CMID_DLOAD_LOGIN << 16) | (x_uint32_t)x_ftp_download_t::CMID_DLOAD_CHUNK)
#define XMKEY_DLOAD_PAUSE   ((x_uint32_t)(x_ftp_download_t::CMID_DLOAD_LOGIN << 16) | (x_uint32_t)x_ftp_download_t::CMID_DLOAD_PAUSE)

////////////////////////////////////////////////////////////////////////////////

#endif // __XFTP_DOWNLOAD_H__

