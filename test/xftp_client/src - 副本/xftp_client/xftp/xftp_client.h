/**
 * @file    xftp_client.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * �ļ����ƣ�xftp_client.h
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

#ifndef __XFTP_CLIENT_H__
#define __XFTP_CLIENT_H__

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <list>

#include "xmempool.h"
#include "xftp_msgctxt.h"

////////////////////////////////////////////////////////////////////////////////
// x_ftp_client_t

/**
 * @class x_ftp_client_t
 * @brief xftp �Ŀͻ��������ࡣ
 */
class x_ftp_client_t
{
    // common data types
public:
    /**
    * @enum  emConstValue
    * @brief ö�ٳ���ֵ��
    */
    typedef enum emConstValue
    {
        ECV_CONNECT_TIMEOUT = 3000,        ///< Ĭ�����Ӳ����ĳ�ʱʱ��
        ECV_MAXWAIT_TIMEOUT = 3000,        ///< ���ȴ���ʱʱ��
        ECV_HEARTBEAT_TIME  = 5000,        ///< Ĭ��ʹ�õ��������ʱ��
    } emConstValue;

    /**
     * @enum  emIoContextCmid
     * @brief IO ���������� ID ��
     */
    typedef enum emIoContextCmid
    {
        CMID_WCLI_LOGIN  = 0x0020,  ///< �����������ͣ������������ӣ�
        CMID_WCLI_HBEAT  = 0x2000,  ///< ����
        CMID_WCLI_FLIST  = 0x3010,  ///< ��ȡ�ļ��б�
    } emIoContextCmid;

    using x_seqno_t       = std::atomic< x_uint16_t >;
    using x_locker_t      = std::mutex;
    using x_sync_notify_t = std::condition_variable;
    using x_thread_t      = std::thread;
    using x_iomsg_queue_t = std::list< x_io_msghead_t * >;

    /** �ļ��б�[ �ļ������ļ���С ] */
    using x_list_file_t   = std::list< std::pair< std::string, x_int64_t > >;

    /**********************************************************/
    /**
     * @brief ���õ��ڴ�ض���
     */
    static inline x_mempool_t & mempool(void) { return _S_mempool; }

private:
    static x_mempool_t _S_mempool;  ///< ���õ��ڴ�ض���

    // constructor/destructor
public:
    x_ftp_client_t(void);
    virtual ~x_ftp_client_t(void);

    // public interfaces
public:
    /**********************************************************/
    /**
     * @brief ������������
     *
     * @param [in ] xszt_host : Ŀ������������������ IP ��ַ���Ķ�ʽ IP ��ַ�ַ�������
     * @param [in ] xwt_port  : Ŀ������������������ �˿ںš�
     *
     * @return x_int32_t
     *         - �ɹ������� 0��
     *         - ʧ�ܣ����� �����롣
     */
    x_int32_t startup(x_cstring_t xszt_host, x_uint16_t xwt_port);

    /**********************************************************/
    /**
     * @brief �رչ�������
     */
    x_void_t shutdown(void);

    /**********************************************************/
    /**
     * @brief �Ƿ��Ѿ�������
     */
    inline x_bool_t is_startup(void) const { return m_xbt_running; }

    /**********************************************************/
    /**
     * @brief �Ƿ��Ѿ���¼��
     */
    inline x_bool_t is_login(void) const { return (is_startup() && m_xbt_login); }

    /**********************************************************/
    /**
     * @brief ��ȡ���ӵ��������ʱ�䣨���뼶 ʱ�䣩��
     */
    inline x_uint32_t get_heartbeat_time(x_void_t) const { return m_xut_heart_time; }

    /**********************************************************/
    /**
     * @brief �������ӵ��������ʱ�䣨���뼶 ʱ�䣩��
     */
    inline x_void_t set_heartbeat_time(x_uint32_t xut_hearttime) { m_xut_heart_time = xut_hearttime; }

    /**********************************************************/
    /**
     * @brief �Ƿ������Զ��ύ��������
     */
    inline x_bool_t is_enable_auto_heartbeat(x_void_t) const { return m_xbt_heart_enable; }

    /**********************************************************/
    /**
     * @brief ����/���� �Զ��ύ��������
     */
    inline x_void_t enable_auto_heartbeat(x_bool_t xbt_enable) { m_xbt_heart_enable = xbt_enable; }

    /**********************************************************/
    /**
     * @brief Ͷ���������� IO ��Ϣ���ݰ���
     */
    x_int32_t post_req_xmsg(x_io_msgctxt_t & xio_msgctxt, x_bool_t xbt_seqn);

    /**********************************************************/
    /**
     * @brief �ӻ����ж�ȡ�ļ��б�
     */
    x_void_t get_cache_flist(x_list_file_t & xlst_files);

public:

    // internal invoking
protected:
    /**********************************************************/
    /**
     * @brief ���ݷ��Ͳ����Ĺ����̵߳���ں�����
     */
    x_void_t thread_send_run(void);

    /**********************************************************/
    /**
     * @brief ���ݽ��ղ����Ĺ����̵߳���ں�����
     */
    x_void_t thread_recv_run(void);

    /**********************************************************/
    /**
     * @brief Ͷ���������ݰ���Ϣ��
     */
    x_void_t send_heartbeat(void);

    /**********************************************************/
    /**
     * @brief ���յ���������� IO ����
     *
     * @param [in    ] xct_io_dptr : �������������档
     * @param [in,out] xut_io_dlen : ���������泤�ȣ���Σ���ʾ��Ч���������ֽ������زΣ���ʾ�������ֽ�������
     *
     * @return x_int32_t
     *         - ���� -1����ʾ�����Ӷ�����׽���ʧЧ�������йرղ�����
     *         - ����  0����ʾ�����Ӷ�����׽�����Ч�����������������
     */
    x_int32_t io_recved(x_uchar_t * const xct_io_dptr, x_uint32_t & xut_io_dlen);

    /**********************************************************/
    /**
     * @brief ����Ӧ�� IO ��Ϣ�����ж�Ӧ�Ĵ��������
     */
    x_void_t dispatch_io_msgctxt(const x_io_msgctxt_t & xio_msgctxt);

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
     * @brief ���� IO Ӧ�������ȡ�ļ��б�
     */
    x_int32_t iocmd_flist(x_uint16_t xut_seqn, x_uchar_t * xct_dptr, x_uint32_t xut_size);

    // data members
protected:
    x_sockfd_t       m_xfdt_sockfd;      ///< �׽���������
    x_bool_t         m_xbt_login;        ///< ��ʶ��¼�ɹ����������ӽ����ɹ���
    x_bool_t         m_xbt_running;      ///< �����̵߳����б�ʶ
    x_thread_t       m_xthd_send;        ///< ���ݷ��͵Ĺ����߳�
    x_thread_t       m_xthd_recv;        ///< ���ݽ��յĹ����߳�

    x_bool_t         m_xbt_heart_enable; ///< ��ʶ�Ƿ������ύ������
    x_uint32_t       m_xut_heart_time;   ///< �������ʱ��

    x_seqno_t        m_xut_seqno;        ///< IO ��Ϣ����ˮ��������
    x_locker_t       m_xlock_mqreq;      ///< ��������� IO ��Ϣ���е�ͬ��������
    x_sync_notify_t  m_xnotify_mqreq;    ///< ��������� IO ��Ϣ���е�ͬ��֪ͨ����������������
    x_iomsg_queue_t  m_xmqueue_req;      ///< ��������� IO ��Ϣ����

    x_locker_t       m_xlock_flist;       ///< m_lst_flist_cache ��ͬ��������
    x_list_file_t    m_lst_flist_cache;  ///< ������ļ��б�
};

////////////////////////////////////////////////////////////////////////////////

#define XMKEY_WCLI_FLIST   ((x_uint32_t)(x_ftp_client_t::CMID_WCLI_LOGIN << 16) | (x_uint32_t)x_ftp_client_t::CMID_WCLI_FLIST)

////////////////////////////////////////////////////////////////////////////////

#endif // __XFTP_CLIENT_H__
