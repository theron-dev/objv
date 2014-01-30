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

    
    OBJV_KEY_DEC(String)
    
    typedef struct _objv_string_t {
        objv_object_t base;
        char * READONLY UTF8String;
        size_t READONLY length;
        objv_boolean_t READONLY copyed;
    } objv_string_t;
    
    
    extern objv_class_t objv_string_class;
    
    objv_string_t * objv_string_alloc(objv_zone_t * zone,const char * UTF8String);
    
    objv_string_t * objv_string_new(objv_zone_t * zone,const char * UTF8String);
    
    objv_string_t * objv_string_alloc_nocopy(objv_zone_t * zone,const char * UTF8String);
    
#ifdef __cplusplus
}
#endif

#endif
