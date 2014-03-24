//
//  objv_exception.h
//  objv
//
//  Created by zhang hailong on 14-2-6.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_exception_h
#define objv_objv_exception_h

#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv.h"
#include "objv_string.h"
    
    OBJV_KEY_DEC(Exception)
    
    typedef struct _objv_exception_t {
        objv_object_t base;
        int READONLY code;
        objv_string_t * READONLY message;
        objv_string_t * READONLY callSymbols;
    } objv_exception_t;
   
    OBJV_CLASS_DEC(Exception)
    
    objv_exception_t * objv_exception_alloc(objv_zone_t * zone,int code,const char * format,...);
    
    objv_exception_t * objv_exception_allocv(objv_zone_t * zone,int code,const char * format,va_list ap);
    
    objv_exception_t * objv_exception_new(objv_zone_t * zone,int code,const char * format,...);
    
    objv_exception_t * objv_exception_newv(objv_zone_t * zone,int code,const char * format,va_list ap);
    
#ifdef __cplusplus
}
#endif



#endif
