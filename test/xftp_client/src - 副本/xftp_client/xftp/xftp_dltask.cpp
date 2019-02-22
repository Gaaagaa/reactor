/**
 * @file    xftp_dltask.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 * 
 * �ļ����ƣ�xftp_dltask.cpp
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

#include "stdafx.h"
#include "xftp_dltask.h"

////////////////////////////////////////////////////////////////////////////////
// x_ftp_dltask_t

//====================================================================

// 
// x_ftp_dltask_t : constructor/destructor
// 

x_ftp_dltask_t::x_ftp_dltask_t(void)
{

}

x_ftp_dltask_t::~x_ftp_dltask_t(void)
{

}

//====================================================================

// 
// x_ftp_dltask_t : overrides
// 

/**********************************************************/
/**
 * @brief �������ִ�����̵ĳ���ӿڡ�
 */
void x_ftp_dltask_t::run(x_running_checker_t * xchecker_ptr)
{
    while (xchecker_ptr->is_enable_running())
    {

    }
}

/**********************************************************/
/**
 * @brief �ж� ������� �Ƿ����
 * @note  ����������ڹ���״̬�������߳���ȡ����ʱ���������ö���
 */
bool x_ftp_dltask_t::is_suspend(void) const
{
    return false;
}

/**********************************************************/
/**
 * @brief ���������������б�ʶ��
 */
void x_ftp_dltask_t::set_running_flag(bool xrunning_flag)
{

}

/**********************************************************/
/**
 * @brief ��ȡ��������ɾ������
 */
const x_task_deleter_t * x_ftp_dltask_t::get_deleter(void) const
{
    return x_task_t::get_deleter();
}

