//
//  objv_zone.c
//  objv
//
//  Created by zhang hailong on 14-1-29.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#include "objv_os.h"
#include "objv.h"
#include "objv_log.h"

static void * objv_zone_default_malloc(struct _objv_zone_t *zone, size_t size){
    return malloc(size);
}

static void objv_zone_default_free(struct _objv_zone_t *zone, void *ptr){
    free(ptr);
}

static void * objv_zone_default_realloc(struct _objv_zone_t *zone, void *ptr, size_t size){
    
    return realloc(ptr,size);
}

static void objv_zone_default_memzero(struct _objv_zone_t *zone, void *ptr, size_t size){
    memset(ptr, 0, size);
}


static objv_zone_t _objv_zone_default = {
    "default",
    objv_zone_default_malloc,
    objv_zone_default_free,
    objv_zone_default_realloc,
    objv_zone_default_memzero
};

static objv_zone_t * g_objv_zone_default = &_objv_zone_default;

objv_zone_t * objv_zone_default(){
    return g_objv_zone_default;
}

void objv_zone_default_set(objv_zone_t * zone){
    if(zone){
        g_objv_zone_default = zone;
    }
    else{
        g_objv_zone_default = &_objv_zone_default;
    }
}

void * objv_zone_malloc(objv_zone_t * zone,size_t size){
    
    if(zone == NULL){
        zone = objv_zone_default();
    }
    
    return (* zone->malloc)(zone,size);
}

void objv_zone_free(objv_zone_t * zone,void * ptr){
    
    if(zone == NULL){
        zone = objv_zone_default();
    }
    
    (* zone->free)(zone,ptr);
}

void * objv_zone_realloc(objv_zone_t * zone,void * ptr,size_t size){
    
    if(zone == NULL){
        zone = objv_zone_default();
    }
    
    return (* zone->realloc)( zone ,ptr , size);
}


void objv_zone_memzero(objv_zone_t * zone,void * ptr,size_t size){
    
    if(zone == NULL){
        zone = objv_zone_default();
    }
    
    (* zone->memzero)( zone ,ptr , size);
}

void objv_zone_retain(objv_zone_t * zone,void * ptr){
    
    if(zone == NULL){
        zone = objv_zone_default();
    }
    
    if(zone->retain){
        (* zone->retain)( zone ,ptr );
    }
}


void objv_zone_release(objv_zone_t * zone,void * ptr){
    if(zone == NULL){
        zone = objv_zone_default();
    }
    
    if(zone->release){
        (* zone->release)( zone ,ptr);
    }
}
