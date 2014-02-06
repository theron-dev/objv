//
//  objv_hash_map.h
//  objv
//
//  Created by zhang hailong on 14-1-30.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_hash_map_h
#define objv_objv_hash_map_h


#ifdef __cplusplus
extern "C" {
#endif
	
#include "objv_map.h"
    
    typedef struct _objv_hash_map_t{
        unsigned int READONLY length;
    } objv_hash_map_t;
    
    typedef long (*objv_hash_map_hash_code_t)(void * key);
	
	long objv_hash_map_hash_code_string(void * key );
	
	long objv_hash_map_hash_code_ptr(void * key);
    
    long objv_hash_map_hash_code_key(void * key);
    
	objv_hash_map_t * objv_hash_map_alloc(int capacity,objv_hash_map_hash_code_t hash_code ,objv_map_compare_t compare);
	
	void objv_hash_map_dealloc(objv_hash_map_t * map);
	
	void * objv_hash_map_put(objv_hash_map_t *  map,void * key,void * value);
	
	void * objv_hash_map_get(objv_hash_map_t *  map,void * key);
	
	void * objv_hash_map_remove(objv_hash_map_t *  map,void * key);
    
    void objv_hash_map_clear(objv_hash_map_t * map);
    
    void * objv_hash_map_keyAt(objv_hash_map_t * map,int index);
    
    void * objv_hash_map_valueAt(objv_hash_map_t * map,int index);
    
#ifdef __cplusplus
}
#endif

#endif
