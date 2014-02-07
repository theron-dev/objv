//
//  objv_string.h
//  objv
//
//  Created by zhang hailong on 14-1-30.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_string_h
#define objv_objv_string_h


#ifdef __cplusplus
extern "C" {
#endif

#include "objv_mbuf.h"
    
    OBJV_KEY_DEC(String)
    
    typedef struct _objv_string_t {
        objv_object_t base;
        char * READONLY UTF8String;
        size_t READONLY length;
        objv_boolean_t READONLY copyed;
    } objv_string_t;
    
    
    extern objv_class_t objv_string_class;
    
    objv_string_t * objv_string_alloc(objv_zone_t * zone,const char * UTF8String);
    
    objv_string_t * objv_string_alloc_format(objv_zone_t * zone,const char * format,...);
    
    objv_string_t * objv_string_alloc_formatv(objv_zone_t * zone,const char * format,va_list ap);
    
    objv_string_t * objv_string_new(objv_zone_t * zone,const char * UTF8String);
    
    objv_string_t * objv_string_new_format(objv_zone_t * zone,const char * format,...);
    
    objv_string_t * objv_string_unicode_alloc(objv_zone_t * zone,unsigned short * unicode,size_t length);
    
    objv_string_t * objv_string_unicode_new(objv_zone_t * zone,unsigned short * unicode,size_t length);
    
    objv_string_t * objv_string_alloc_nocopy(objv_zone_t * zone,const char * UTF8String);
    
    size_t objv_unicode_to_utf8(unsigned short * unicode, size_t length, objv_mbuf_t * mbuf);

    const char * objv_string_indexOf(const char * string,const char * substring);
    
#ifdef __cplusplus
}
#endif

#endif
