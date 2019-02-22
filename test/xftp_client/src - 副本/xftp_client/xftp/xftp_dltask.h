/**
 * @file    xftp_dltask.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * �ļ����ƣ�xftp_dltask.h
 * �������ڣ�2019��02��21��
 * �ļ���ʶ��
 * �ļ�ժҪ���ļ����ص�����������ࡣ
 * 
 * ��ǰ�汾��1.0.0.0
 * ��    �ߣ�
 * ������ڣ�2019��02��21��
 * �汾ժҪ��
 * 
 * ȡ���汾��
 * ԭ����  ��
 * ������ڣ�
 * �汾ժҪ��
 * </pre>
 */

#ifndef __XFTP_DLTASK_H__
#define __XFTP_DLTASK_H__

#include <mutex>
#include <atomic>
#include <list>

#include "xmempool.h"
#include "xthreadpool.h"
#include "xftp_msgctxt.h"

////////////////////////////////////////////////////////////////////////////////
// x_ftp_dltask_t

/**
 * @class x_ftp_dltask_t
 * @brief �ļ����ص�����������ࡣ
 */
class x_ftp_dltask_t : public x_task_t
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
        ECV_FILE_CHUNK_SIZE = 32 * 1024,   ///< �ļ���Ĵ�С
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

    using x_seqno_t       = std::atomic< x_uint16_t >;
    using x_lock_t        = std::mutex;
    using x_iomsg_queue_t = std::list< x_io_msghead_t * >;

    /**********************************************************/
    /**
     * @brief ���õ��ڴ�ض���
     */
    static inline x_mempool_t & mempool(void) { return _S_mempool; }

private:
    static x_mempool_t _S_mempool;  ///< ���õ��ڴ�ض���

    // constructor/destructor
public:
    x_ftp_dltask_t(void);
    virtual ~x_ftp_dltask_t(void);

    // overrides
public:
    /**********************************************************/
    /**
     * @brief �������ִ�����̵ĳ���ӿڡ�
     */
    virtual void run(x_running_checker_t * xchecker_ptr);

    /**********************************************************/
    /**
     * @brief �ж� ������� �Ƿ����
     */
    virtual bool is_suspend(void) const;

    /**********************************************************/
    /**
     * @brief ���������������б�ʶ��
     */
    virtual void set_running_flag(bool xrunning_flag);

    /**********************************************************/
    /**
     * @brief ��ȡ��������ɾ������
     */
    virtual const x_task_deleter_t * get_deleter(void) const;

    // public interfaces
public:


    // data members
private:
    std::string  m_xstr_host;   ///< XFTP ���������� IP
    x_uint16_t   m_xut_port;    ///< XFTP ���������� �˿ں�
    std::string  m_xstr_fpath;  ///< �����ص��ļ����ش洢·��
    x_int64_t    m_xit_fsize;   ///< �ļ��ܴ�С

    x_sockfd_t   m_xfdt_sockfd; ///< ���������׽���
};

////////////////////////////////////////////////////////////////////////////////

#endif // __XFTP_DLTASK_H__
