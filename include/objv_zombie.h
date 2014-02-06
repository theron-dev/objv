//
//  objv_zombie.h
//  objv
//
//  Created by zhang hailong on 14-2-4.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_zombie_h
#define objv_objv_zombie_h

#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv.h"
#include "objv_os.h"
    
    typedef struct _objv_zombie_block_t {
        size_t size;
        const char * file;
        int line;
        size_t index;
    } objv_zombie_block_t;
    
    typedef struct _objv_zombie_t{
        objv_zone_t zone;
        objv_mutex_t READONLY mutex;
        objv_zombie_block_t ** READONLY blocks;
        size_t READONLY size;
        size_t READONLY length;
        size_t READONLY memsize;
    } objv_zombie_t;
    
    void objv_zombie_init(objv_zombie_t * zombie,size_t capacity);
    
    void objv_zombie_destroy(objv_zombie_t * zombie);
    
    void objv_zombie_print(objv_zombie_t * zombie);
    
#ifdef __cplusplus
}
#endif



#endif
