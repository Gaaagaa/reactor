/**
 * @file    main.cpp
 * <pre>
 * Copyright (c) 2018, Gaaagaa All rights reserved.
 * 
 * 文件名称：main.cpp
 * 创建日期：2018年12月20日
 * 文件标识：
 * 文件摘要：程序入口函数。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2018年12月20日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#include "xcomm.h"
#include "xmaster.h"
#include "xftp_server.h"

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv[])
{
    x_int32_t  xit_error = -1;
    x_master_t & xmaster = x_master_t::instance();

    xmaster.set_init_callback(&x_ftp_server_t::init_extra_callback, X_NULL);

    xit_error = xmaster.startup(argc, argv);
    if (0 == xit_error)
        xit_error = xmaster.run();
    xmaster.shutdown();

    return xit_error;
}
