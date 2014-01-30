//
//  objv_autorelease.h
//  objv
//
//  Created by zhang hailong on 14-1-30.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_autorelease_h
#define objv_objv_autorelease_h

#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv.h"
    
    OBJV_KEY_DEC(AutoreleaseTask)
    OBJV_KEY_DEC(AutoreleasePool)
    
    typedef struct _objv_autorelease_pool_t {
        objv_object_t base;
        objv_object_t ** objects;
        int size;
        int length;
        struct _objv_autorelease_pool_t * parent;
    } objv_autorelease_pool_t;
    
    extern objv_class_t objv_autorelease_pool_class;
    
    objv_autorelease_pool_t * objv_autorelease_pool_alloc(objv_zone_t * zone);
   
    objv_object_t * objv_object_autorelease(objv_object_t * object);
    
#ifdef __cplusplus
}
#endif


#endif
