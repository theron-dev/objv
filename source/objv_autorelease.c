//
//  objv_autorelease.c
//  objv
//
//  Created by zhang hailong on 14-1-30.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv.h"
#include "objv_autorelease.h"
#include "objv_dispatch.h"
#include "objv_log.h"

OBJV_KEY_DEC(AutoreleaseTask)
OBJV_KEY_DEC(AutoreleasePool)

typedef struct _objv_autorelease_pool_t {
    objv_object_t base;
    objv_object_t ** objects;
    int size;
    int length;
    struct _objv_autorelease_pool_t * parent;
} objv_autorelease_pool_t;

OBJV_KEY_IMP(AutoreleaseTask)
OBJV_KEY_IMP(AutoreleasePool)

static objv_thread_key_t autorelease_pool_key = 0;

static void objv_autorelease_pool_methods_dealloc(objv_class_t * clazz, objv_object_t * obj){
    
    objv_autorelease_pool_t * pool = (objv_autorelease_pool_t *) obj;
   
    objv_object_t ** objects = NULL, ** p;
    int size = 0,c;
    
    while(pool->length > 0){
        
        if(pool->length > size){
            size = pool->length;
            if(objects){
                objects = (objv_object_t **) objv_zone_realloc(obj->zone, objects, size * sizeof(objv_object_t *));
            }
            else{
                objects = (objv_object_t **) objv_zone_malloc(obj->zone, size * sizeof(objv_object_t *));
            }
        }
        
        memcpy(objects , pool->objects, pool->length * sizeof(objv_object_t *));
        
        p = objects;
        c = pool->length;
        
        pool->length = 0;
        
        while (c > 0 && p) {
            
            objv_object_release( * p);
            
            c --;
            p ++;
        }
    
    }
    
    if(objects){
        objv_zone_free(obj->zone, objects);
    }
    
    if(pool->objects){
        objv_zone_free(obj->zone, pool->objects);
    }
    
    objv_thread_key_setValue(autorelease_pool_key, pool->parent);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass,obj);
    }
}


static objv_method_t objv_autorelease_pool_methods[] = {
    {OBJV_KEY(dealloc),"v()",(objv_method_impl_t)objv_autorelease_pool_methods_dealloc}
};

static objv_class_t objv_autorelease_pool_class = {OBJV_KEY(AutoreleasePool),& objv_Object_class
    ,objv_autorelease_pool_methods,sizeof(objv_autorelease_pool_methods) / sizeof(objv_method_t)
    ,NULL,0
    ,sizeof(objv_autorelease_pool_t)
    ,NULL,0};



static void autorelease_pool_key_dealloc(void * pool){
    
}

objv_autorelease_pool_t * objv_autorelease_pool_get_current(){
    
    if(autorelease_pool_key == 0){
        
        objv_thread_key_create(& autorelease_pool_key, autorelease_pool_key_dealloc);
        
    }
    
    return objv_thread_key_value(autorelease_pool_key);
}


void objv_autorelease_pool_push(){
    
    objv_autorelease_pool_t * pool = (objv_autorelease_pool_t *) objv_object_alloc(NULL, &objv_autorelease_pool_class);
    
    pool->parent = objv_autorelease_pool_get_current();
    
    objv_thread_key_setValue(autorelease_pool_key, pool);
    
}

void objv_autorelease_pool_pop(){
    objv_object_release((objv_object_t *)objv_autorelease_pool_get_current());
}

objv_object_t * objv_object_new(objv_zone_t * zone,objv_class_t * clazz,...){
    
    objv_object_t * object;
    
    va_list ap;
    
    va_start(ap, clazz);
    
    object = objv_object_autorelease(objv_object_allocv(zone, clazz,ap));
    
    va_end(ap);
    
    return object;
}

objv_object_t * objv_object_newv(objv_zone_t * zone,objv_class_t * clazz,va_list ap){
    return objv_object_autorelease(objv_object_allocv(zone, clazz,ap));
}


objv_object_t * objv_object_autorelease(objv_object_t * object){
    if(object){
        
        objv_autorelease_pool_t * pool = objv_autorelease_pool_get_current();
        
        if(pool){
            
            if(pool->length + 1 > pool->size){
                pool->size += 20;
                if(pool->objects){
                    pool->objects = objv_zone_realloc(NULL, pool->objects,pool->size * sizeof(objv_object_t *));
                }
                else{
                    pool->objects = objv_zone_malloc(NULL, pool->size * sizeof(objv_object_t *));
                }
            }
            
            pool->objects[pool->length ++] = object;
            
        }
        else{
            objv_log("\nnot found autorelease pool\n");
        }
        
    }
    return object;
}
