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

typedef struct _objv_autorelease_task_t{
    objv_dispatch_task_t base;
    objv_object_t * object;
} objv_autorelease_task_t;

static void objv_autorelease_task_methods_dealloc(objv_class_t * clazz, objv_object_t * obj){
    
    objv_autorelease_task_t * task = (objv_autorelease_task_t *) obj;
    
    objv_object_release(task->object);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass,obj);
    }
}

static objv_method_t objv_autorelease_task_methods[] = {
    {OBJV_KEY(dealloc),"v()",(objv_method_impl_t)objv_autorelease_task_methods_dealloc}
};


static objv_class_t objv_autorelease_task_class = {OBJV_KEY(AutoreleaseTask),& objv_object_class
    ,objv_autorelease_task_methods,sizeof(objv_autorelease_task_methods) / sizeof(objv_method_t)
    ,NULL,0
    ,sizeof(objv_autorelease_task_t)
    ,NULL,0,0};

objv_autorelease_task_t * objv_autorelease_task_alloc(objv_zone_t * zone,objv_object_t * object){
    
    objv_autorelease_task_t * task = (objv_autorelease_task_t *) objv_object_alloc(zone, &objv_autorelease_task_class);
    
    task->object = object;
    
    return task;
}

static void objv_autorelease_pool_methods_dealloc(objv_class_t * clazz, objv_object_t * obj){
    
    objv_autorelease_pool_t * pool = (objv_autorelease_pool_t *) obj;
    
    objv_autorelease_task_t * task;
    objv_dispatch_t * dispatch = objv_dispatch_get_current();
    
    objv_object_t ** p = pool->objects;
    int c = pool->length;
    
    while (c > 0 && p) {
        
        task = objv_autorelease_task_alloc(NULL,* p);
        
        objv_dispatch_addTask(dispatch, (objv_dispatch_task_t *)task);
        
        objv_object_release((objv_object_t *) task);
        
        c --;
        p ++;
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

static objv_class_t objv_autorelease_pool_class = {OBJV_KEY(AutoreleasePool),& objv_object_class
    ,objv_autorelease_pool_methods,sizeof(objv_autorelease_pool_methods) / sizeof(objv_method_t)
    ,NULL,0
    ,sizeof(objv_autorelease_pool_t)
    ,NULL,0,0};



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
