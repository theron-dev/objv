//
//  objv_zombie.c
//  objv
//
//  Created by zhang hailong on 14-2-4.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv_zombie.h"
#include "objv_log.h"

#define EXTEND_SIZE 64

static void * objv_zone_zombie_malloc(struct _objv_zone_t *zone, size_t size,const char * file,int line){
    
    objv_zombie_t * zombie = (objv_zombie_t *) zone;
    
    objv_zombie_block_t * block = ( objv_zombie_block_t *) malloc(size + sizeof(objv_zombie_block_t));
    
    memset(block, 0, sizeof(objv_zombie_block_t));
    
    block->file = file;
    block->line = line;
    block->size = size;
    block->retainCount = 1;
    
    objv_mutex_lock(& zombie->mutex);
    
    if(zombie->length + 1 > zombie->size){
        zombie->size += EXTEND_SIZE;
        zombie->blocks = (objv_zombie_block_t ** ) realloc(zombie->blocks,zombie->size * sizeof(objv_zombie_block_t *));
    }
    
    block->index = zombie->length ++;
    
    zombie->blocks[block->index] = block;
    zombie->memsize += size;
    
    objv_mutex_unlock(& zombie->mutex);
    
    return block + 1;
}

static void objv_zone_zombie_free(struct _objv_zone_t *zone, void *ptr){
   
    objv_zombie_t * zombie = (objv_zombie_t *) zone;
    
    objv_zombie_block_t * block = (objv_zombie_block_t *) ptr - 1;
    
    objv_mutex_lock(& zombie->mutex);

    assert(block->size);
    
    block->size = 0;
    
    objv_mutex_unlock(& zombie->mutex);
    
}

static void * objv_zone_zombie_realloc(struct _objv_zone_t *zone, void *ptr, size_t size,const char * file,int line){
    
    objv_zombie_t * zombie = (objv_zombie_t *) zone;
    
    objv_zombie_block_t * block = (objv_zombie_block_t *) ptr - 1;
    
    block = ( objv_zombie_block_t *) realloc(block , size + sizeof(objv_zombie_block_t));
    block->size = size;
    block->file = file;
    block->line = line;
    
    objv_mutex_lock(& zombie->mutex);
    
    zombie->blocks[block->index] = block;
    
    objv_mutex_unlock(& zombie->mutex);
    
    return block + 1;
}

static void objv_zone_zombie_memzero(struct _objv_zone_t *zone, void *ptr, size_t size){
    memset(ptr, 0, size);
}


static void objv_zone_zombie_retain(struct _objv_zone_t *zone, void *ptr,const char * file,int line){
    
    objv_zombie_t * zombie = (objv_zombie_t *) zone;
    
    objv_zombie_block_t * block = (objv_zombie_block_t *) ptr - 1;
    
    objv_mutex_lock(& zombie->mutex);
    
    assert(block->retainCount >0);
    
    block->retainCount ++;
    
    objv_zombie_block_location_t * location = (objv_zombie_block_location_t *) malloc(sizeof(objv_zombie_block_location_t));
    
    memset(location, 0, sizeof(objv_zombie_block_location_t));
    
    location->method = "retain";
    location->file = file;
    location->line = line;
    location->retainCount = block->retainCount;
    
    if(block->begin == NULL){
        block->begin = block->end = location;
    }
    else{
        block->end->next = location;
        block->end = location;
    }
    
    objv_mutex_unlock(& zombie->mutex);
    
}

static void objv_zone_zombie_release(struct _objv_zone_t *zone, void *ptr,const char * file,int line){
    
    objv_zombie_t * zombie = (objv_zombie_t *) zone;
    
    objv_zombie_block_t * block = (objv_zombie_block_t *) ptr - 1;
    
    objv_mutex_lock(& zombie->mutex);
    
    assert(block->retainCount >0);
    
    block->retainCount --;
    
    objv_zombie_block_location_t * location = (objv_zombie_block_location_t *) malloc(sizeof(objv_zombie_block_location_t));
    
    memset(location, 0, sizeof(objv_zombie_block_location_t));
    
    location->method = "release";
    location->file = file;
    location->line = line;
    location->retainCount = block->retainCount;
    
    if(block->begin == NULL){
        block->begin = block->end = location;
    }
    else{
        block->end->next = location;
        block->end = location;
    }
    
    objv_mutex_unlock(& zombie->mutex);
    
}

void objv_zombie_init(objv_zombie_t * zombie,size_t capacity){
    
    memset(zombie,0,sizeof(objv_zombie_t));
    
    objv_mutex_init(& zombie->mutex);
    zombie->size = capacity;
    zombie->blocks = (objv_zombie_block_t ** ) malloc( capacity * sizeof(objv_zombie_block_t *));
    zombie->zone.name = "zombie";
    zombie->zone.malloc = objv_zone_zombie_malloc;
    zombie->zone.free = objv_zone_zombie_free;
    zombie->zone.realloc = objv_zone_zombie_realloc;
    zombie->zone.memzero = objv_zone_zombie_memzero;
    zombie->zone.retain = objv_zone_zombie_retain;
    zombie->zone.release = objv_zone_zombie_release;
}

void objv_zombie_destroy(objv_zombie_t * zombie){
    
    objv_log("\nobjv_zombie_destroy, length:%d, memsize:%d\n",zombie->length,zombie->memsize);

    objv_mutex_lock(& zombie->mutex);
    
    int c = 0;
    
    for(int i=0;i< zombie->length;i++){
        objv_zombie_block_t * block = zombie->blocks[i];
        if(block->size){
            objv_log("%d,%d,(%d)%s\n",i,block->size,block->line,block->file);
            c ++;
        }
        else{
            free(block);
        }
    }
    
    objv_mutex_unlock(& zombie->mutex);
    
    assert(c ==0);
    
    objv_mutex_destroy(& zombie->mutex);
    
    memset(zombie,0,sizeof(objv_zombie_t));
}

void objv_zombie_print(objv_zombie_t * zombie){
    
    objv_log("\n");
    
    objv_mutex_lock(& zombie->mutex);
    
    size_t size = 0;
    
    for(int i=0;i< zombie->length;i++){
        objv_zombie_block_t * block = zombie->blocks[i];
        if(block->size){
            objv_log("%d,(%d)%s\n",block->size,block->line,block->size);
            size += block->size;
        }
    }
    
    objv_mutex_unlock(& zombie->mutex);

    objv_log("malloced size: %d\n",size);
}


