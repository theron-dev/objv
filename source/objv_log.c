//
//  objv_log.c
//  objv
//
//  Created by zhang hailong on 14-1-30.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv_log.h"
#include "objv_mbuf.h"

static int _objv_log_stdout = STDOUT_FILENO;

void objv_log(const char * format,...){
    
    va_list va;
    
    va_start(va, format);
    
    objv_vlog(format, va);
    
    va_end(va);
    
}

void objv_vlog(const char * format,va_list va){
    objv_mbuf_t mbuf;
    time_t now = time(NULL);
    struct tm * nowmt = gmtime(& now);
    
    objv_mbuf_init(& mbuf, 128);
    
    objv_mbuf_append(& mbuf, "[",1);
    
    mbuf.length += strftime((char *) mbuf.data + mbuf.length,mbuf.size - mbuf.length,"%x %X",nowmt);
    
    objv_mbuf_append(& mbuf, "] ",2);
    
    objv_mbuf_formatv(& mbuf, format, va);
    
    objv_mbuf_append(& mbuf, "\n",2);
    
    write(_objv_log_stdout, mbuf.data, mbuf.length);
    
    objv_mbuf_destroy(& mbuf);
    
}

void objv_log_stdout(int fno){
    _objv_log_stdout = fno;
}
