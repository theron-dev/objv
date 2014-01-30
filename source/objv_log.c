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

void objv_log(const char * format,...){
    
    va_list va;
    
    va_start(va, format);
    
    objv_vlog(format, va);
    
    va_end(va);
    
}

void objv_vlog(const char * format,va_list va){
    vprintf(format, va);
}
