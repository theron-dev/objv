//
//  objv_gc.c
//  objv
//
//  Created by zhang hailong on 14-2-8.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv_gc.h"

static void * objv_zone_gc_malloc(struct _objv_zone_t *zone, size_t size,const char * file,int line){
    
    objv_gc_t * gc = (objv_gc_t *) zone;
    
    objv_gc_block_t * block = ( objv_gc_block_t *) malloc(size + sizeof(objv_gc_block_t));
    
    memset(block, 0, sizeof(objv_gc_block_t));
   
    block->size = size;

    objv_mutex_lock(& gc->mutex);
    
    if(gc->blocks){
        block->next = gc->blocks;
        gc->blocks = block;
    }
    else{
        gc->blocks = block;
    }
    
    gc->mallocsize += size;
    
    objv_mutex_unlock(& gc->mutex);
    
    return block + 1;
}

static void objv_zone_gc_free(struct _objv_zone_t * zone, void *ptr){
    
    objv_gc_t * gc = (objv_gc_t *) zone;
    
    objv_gc_block_t * block = (objv_gc_block_t *) ptr - 1;
    
    objv_mutex_lock(& gc->mutex);
    
    assert(block->size);
    
    block->size = 0;
    
    gc->freesize += block->size;
    
    objv_mutex_unlock(& gc->mutex);
}

static void * objv_zone_gc_realloc(struct _objv_zone_t *zone, void *ptr, size_t size,const char * file,int line){
    
    objv_gc_t * gc = (objv_gc_t *) zone;
    
    objv_gc_block_t * block = (objv_gc_block_t *) ptr - 1;
    
    objv_gc_block_t * reblock;
    
    objv_mutex_lock(& gc->mutex);
    
    reblock = (objv_gc_block_t *) malloc(size + sizeof(objv_gc_block_t));
    
    memcpy(reblock, block, sizeof(objv_gc_block_t) + block->size);
    
    block->size = 0;
    
    if(gc->blocks){
        block->next = gc->blocks;
        gc->blocks = reblock;
    }
    else{
        gc->blocks = reblock;
    }
    
    objv_mutex_unlock(& gc->mutex);
    
    return reblock + 1;
}

static void objv_zone_gc_memzero(struct _objv_zone_t *zone, void *ptr, size_t size){
    memset(ptr, 0, size);
}

void objv_gc_init(objv_gc_t * gc){
    
    memset(gc,0,sizeof(objv_gc_t));
    
    objv_mutex_init(& gc->mutex);
    gc->zone.name = "gc";
    gc->zone.malloc = objv_zone_gc_malloc;
    gc->zone.free = objv_zone_gc_free;
    gc->zone.realloc = objv_zone_gc_realloc;
    gc->zone.memzero = objv_zone_gc_memzero;
}

void objv_gc_destroy(objv_gc_t * gc){
    
    objv_mutex_lock(& gc->mutex);
    
    objv_gc_block_t * block = gc->blocks,*t;
    
    while (block) {
        t = block;
        block = block->next;
        free(t);
    }

    objv_mutex_unlock(& gc->mutex);

    objv_mutex_destroy(& gc->mutex);
    
    memset(gc,0,sizeof(objv_gc_t));
}

void objv_gc_cleanup(objv_gc_t * gc){
    
    objv_mutex_lock(& gc->mutex);
    
    objv_gc_block_t * block = gc->blocks,* pblock = NULL;
    
    while (block) {
    
        if(block->size == 0){
            if(pblock){
                pblock->next = block->next;
                free(block);
                block = pblock->next;
            }
            else{
                gc->blocks = block->next;
                block = gc->blocks;
            }
        }
        else{
            block = block->next;
        }
    }
    
    gc->mallocsize -= gc->freesize;
    gc->freesize = 0;
    
    objv_mutex_unlock(& gc->mutex);
}

