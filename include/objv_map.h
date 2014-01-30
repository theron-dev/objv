//
//  objv_map.h
//  objv
//
//  Created by zhang hailong on 14-1-30.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_map_h
#define objv_objv_map_h

#ifdef __cplusplus
extern "C" {
#endif
	

    typedef struct _objv_map_t{
        unsigned int READONLY length;
    } objv_map_t;
	
	typedef int (*objv_map_compare_t)(void * key1,void * key2);
	
    int objv_map_compare_string(void * key1 ,void * key2);
	
	int objv_map_compare_any(void * key1 ,void * key2);
	
	objv_map_t * objv_map_alloc(int capacity,objv_map_compare_t compare);
	
	void objv_map_dealloc(objv_map_t * map);
	
	void * objv_map_put(objv_map_t *  map,void * key,void * value);
	
	void * objv_map_get(objv_map_t *  map,void * key);
	
	void * objv_map_remove(objv_map_t *  map,void * key);
		
    void objv_map_clear(objv_map_t * map);
    
	void * objv_map_keyAt(objv_map_t * map,int index);
    
    void * objv_map_valueAt(objv_map_t * map, int index);
    
    void objv_map_setValue(objv_map_t * map,int index,void * value);
    
#ifdef __cplusplus
}
#endif

#endif
