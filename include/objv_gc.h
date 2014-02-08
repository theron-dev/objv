//
//  objv_gc.h
//  objv
//
//  Created by zhang hailong on 14-2-8.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_gc_h
#define objv_objv_gc_h


#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv.h"
#include "objv_os.h"
    
    typedef struct _objv_gc_block_t {
        size_t size;
        struct _objv_gc_block_t * next;
    } objv_gc_block_t;
    
    typedef struct _objv_gc_t{
        objv_zone_t zone;
        objv_mutex_t READONLY mutex;
        objv_gc_block_t * READONLY blocks;
        size_t READONLY mallocsize;
        size_t READONLY freesize;
    } objv_gc_t;
    
    void objv_gc_init(objv_gc_t * gc);
    
    void objv_gc_destroy(objv_gc_t * gc);
    
    void objv_gc_cleanup(objv_gc_t * gc);
    
#ifdef __cplusplus
}
#endif



#endif
