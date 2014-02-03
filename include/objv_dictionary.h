//
//  objv_dictionary.h
//  objv
//
//  Created by zhang hailong on 14-1-30.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_dictionary_h
#define objv_objv_dictionary_h



#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_array.h"
#include "objv_hash_map.h"
    
    OBJV_KEY_DEC(Dictionary)
    
    typedef struct _objv_dictionary_t {
        objv_object_t base;
        objv_hash_map_t * READONLY map;
    } objv_dictionary_t;
    
    extern objv_class_t objv_dictionary_class;
    
    objv_dictionary_t * objv_dictionary_alloc(objv_zone_t * zone,unsigned int capacity);
    
    objv_dictionary_t * objv_dictionary_new(objv_zone_t * zone,unsigned int capacity);
    
    void objv_dictionary_setValue(objv_dictionary_t * dictionary,objv_object_t * key, objv_object_t * value);
    
    objv_object_t * objv_dictionary_value(objv_dictionary_t * dictionary,objv_object_t * key);
    
    void objv_dictionary_remove(objv_dictionary_t * dictionary,objv_object_t * key);
    
    int objv_dictionary_length(objv_dictionary_t * dictionary);
    
    objv_object_t * objv_dictionary_keyAt(objv_dictionary_t * dictionary,int index);
    
    objv_object_t * objv_dictionary_valueAt(objv_dictionary_t * dictionary,int index);
    
    void objv_dictionary_clear(objv_dictionary_t * dictionary);
    
    
#ifdef __cplusplus
}
#endif



#endif
