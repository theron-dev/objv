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
    
    void objv_autorelease_pool_push();
    
    void objv_autorelease_pool_pop();
   
    objv_object_t * objv_object_new(objv_zone_t * zone,objv_class_t * clazz,...);
    
    objv_object_t * objv_object_newv(objv_zone_t * zone,objv_class_t * clazz,va_list ap);
    
    
    objv_object_t * objv_object_autorelease(objv_object_t * object);
    
#ifdef __cplusplus
}
#endif


#endif
