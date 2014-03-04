//
//  objv_actree.h
//  objv
//
//  Created by zhang hailong on 14-2-28.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_actree_h
#define objv_objv_actree_h

#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_dictionary.h"
 
    OBJV_KEY_DEC(ACTree)
    
    OBJV_CLASS_DEC(ACTree)
    
    typedef struct _objv_actree_t {
        objv_object_t base;
        objv_object_t * READONLY key;
        objv_object_t * READONLY value;
        objv_dictionary_t * READONLY childs;
    } objv_actree_t;
    
    objv_actree_t * objv_actree_alloc(objv_zone_t * zone,objv_object_t * key, objv_object_t * value);
    
    objv_actree_t * objv_actree_new(objv_zone_t * zone,objv_object_t * key, objv_object_t * value);
    
    void objv_actree_setValue(objv_actree_t * actree,objv_array_t * keys,objv_object_t * value);
    
    objv_object_t * objv_actree_value(objv_actree_t * actree,objv_array_t * keys);
    
    void objv_actree_remove(objv_actree_t * actree,objv_array_t * keys);
    
    objv_actree_t * objv_actree_find(objv_actree_t * actree,objv_array_t * keys);
    
    typedef void ( * objv_actree_echo_callback_t) (objv_actree_t * actree,objv_object_t * key ,objv_object_t * value,void * context);
    
    void objv_actree_echo(objv_actree_t * actree,objv_actree_echo_callback_t callback,void * context);
    
#ifdef __cplusplus
}
#endif



#endif
