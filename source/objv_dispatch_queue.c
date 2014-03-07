//
//  objv_dispatch_queue.c
//  objv
//
//  Created by zhang hailong on 14-1-30.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY


#include "objv_os.h"
#include "objv.h"
#include "objv_dispatch.h"
#include "objv_hash_map.h"
#include "objv_autorelease.h"
#include "objv_log.h"

#define DEFAULT_MAX_THREAD_COUNT    20

OBJV_KEY_IMP(DispatchQueue)


static void objv_dispatch_queue_method_dealloc (objv_class_t * clazz
                                                , objv_object_t * object){
    
    objv_dispatch_queue_t * queue = (objv_dispatch_queue_t *) object;
    
    objv_mutex_lock(& queue->mutex);
    
    for(int i=0;i<queue->dispatchs->length;i++){
        objv_dispatch_cancelAllTasks((objv_dispatch_t *) objv_array_objectAt(queue->dispatchs, i));
    }
    
    objv_object_release((objv_object_t *) queue->dispatchs);
    objv_object_release((objv_object_t *) queue->tasks);
    
    objv_mutex_unlock(& queue->mutex);
    objv_mutex_destroy(& queue->mutex);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
}

static objv_object_t * objv_dispatch_queue_method_init (objv_class_t * clazz
                                                , objv_object_t * object,va_list ap){
    
    if(clazz->superClass){
        object = objv_object_initv(clazz->superClass, object, ap);
    }
    
    if(object){
    
        objv_dispatch_queue_t * queue = (objv_dispatch_queue_t *) object;
        queue->name = va_arg(ap, const char *);
        queue->maxThreadCount = va_arg(ap, unsigned int);
        
        queue->dispatchs = objv_array_alloc(object->zone, 20);
        queue->tasks = objv_array_alloc(object->zone, 20);
        
        objv_mutex_init(& queue->mutex);
    }
    
    return object;
}

OBJV_CLASS_METHOD_IMP_BEGIN(DispatchQueue)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", objv_dispatch_queue_method_dealloc)

OBJV_CLASS_METHOD_IMP(init, "@(*)", objv_dispatch_queue_method_init)

OBJV_CLASS_METHOD_IMP_END(DispatchQueue)

OBJV_CLASS_IMP_M(DispatchQueue, OBJV_CLASS(Object), objv_dispatch_queue_t)


objv_dispatch_queue_t * objv_dispatch_queue_alloc(objv_zone_t * zone,const char * name,unsigned int maxThreadCount){
    return (objv_dispatch_queue_t *) objv_object_alloc(zone, OBJV_CLASS(DispatchQueue),name,maxThreadCount,NULL);
}

typedef struct _objv_dispatch_queue_thread_userInfo {
    objv_zone_t * zone;
    objv_dispatch_queue_t * queue;
    objv_dispatch_task_t * task;
} objv_dispatch_queue_thread_userInfo;

static void * objv_dispatch_queue_thread(void * userInfo){
    
    objv_dispatch_queue_thread_userInfo * u = (objv_dispatch_queue_thread_userInfo *) userInfo;
    
    objv_autorelease_pool_push();
    
    objv_dispatch_queue_t * queue = u->queue;
    
    objv_dispatch_t * dispatch = objv_dispatch_get_current();
    
    objv_mutex_lock(& queue->mutex);
    
    objv_array_add(queue->dispatchs, (objv_object_t *) dispatch);
    
    if(u->task){
       
        objv_dispatch_addTask(dispatch, u->task);
        
        objv_array_remove(queue->tasks, (objv_object_t *) u->task);
        
    }
    
    objv_mutex_unlock(& queue->mutex);
 
    while (dispatch->idleTimeinval < 20 || objv_dispatch_tasks_count(dispatch) > 0) {
        
        objv_autorelease_pool_push();
        
        objv_dispatch_run(dispatch,0.05);
        
        objv_autorelease_pool_pop();
    }
    
    objv_mutex_lock(& queue->mutex);
    
    objv_array_remove(queue->dispatchs, (objv_object_t *) dispatch);
    
    objv_mutex_unlock(& queue->mutex);
    
    objv_object_release((objv_object_t *) u->queue);
    
    objv_zone_free(u->zone, u);
    
    objv_autorelease_pool_pop();
    
    objv_thread_detach(objv_thread_self());
    
    objv_thread_exit(NULL);
    
    return NULL;
}

void objv_dispatch_queue_addTask(objv_dispatch_queue_t * queue
                                 ,objv_dispatch_task_t * task){
    
    if(queue && task){

        if((queue->maxThreadCount == 0
               || queue->dispatchs->length < queue->maxThreadCount)){
            
            objv_dispatch_queue_thread_userInfo * t = (objv_dispatch_queue_thread_userInfo *) objv_zone_malloc(queue->base.zone, sizeof(objv_dispatch_queue_thread_userInfo));
            
            objv_zone_memzero(queue->base.zone, t, sizeof(objv_dispatch_queue_thread_userInfo));
            
            t->zone = queue->base.zone;
            t->queue = (objv_dispatch_queue_t *) objv_object_retain((objv_object_t *)queue);
            t->task = task;
            
            objv_mutex_lock(& queue->mutex);
            
        
            if( objv_thread_create(objv_dispatch_queue_thread, t) ) {
                objv_array_add(queue->tasks, (objv_object_t *) task);
            }
            else{
                
                objv_log("objv_dispatch_queue_addTask objv_thread_create error\n");
                
                objv_dispatch_t * disp = NULL;
                
                if(queue->index >= queue->dispatchs->length){
                    queue->index = 0;
                }
                
                if(queue->index < queue->dispatchs->length){
                    disp = (objv_dispatch_t *) objv_array_objectAt(queue->dispatchs, queue->index ++);
                    objv_dispatch_addTask(disp, task);
                }
                
            }
            
            objv_mutex_unlock(& queue->mutex);
            
        }
        else {
            
            objv_mutex_lock(& queue->mutex);
            
            objv_dispatch_t * disp = NULL;
            
            if(queue->index >= queue->dispatchs->length){
                queue->index = 0;
            }
            
            if(queue->index < queue->dispatchs->length){
                disp = (objv_dispatch_t *) objv_array_objectAt(queue->dispatchs, queue->index ++);
                objv_dispatch_addTask(disp, task);
            }
            else{
                
                objv_dispatch_queue_thread_userInfo * t = (objv_dispatch_queue_thread_userInfo *) objv_zone_malloc(queue->base.zone, sizeof(objv_dispatch_queue_thread_userInfo));
                
                objv_zone_memzero(queue->base.zone, t, sizeof(objv_dispatch_queue_thread_userInfo));
                
                t->zone = queue->base.zone;
                t->queue = (objv_dispatch_queue_t *) objv_object_retain((objv_object_t *)queue);
                t->task = task;
                
                if( objv_thread_create(objv_dispatch_queue_thread, t) ) {
                    objv_array_add(queue->tasks, (objv_object_t *) task);
                }
                else{
                    objv_log("objv_dispatch_queue_addTask objv_thread_create error\n");
                }
            }
            
            objv_mutex_unlock(& queue->mutex);
        }

    }
}

void objv_dispatch_queue_cancelTask(objv_dispatch_queue_t * queue
                                    ,objv_dispatch_task_t * task){
    if(queue && task){
        
        objv_dispatch_t * p;
        
        objv_mutex_lock(& queue->mutex);
        
        for(int i=0;i<queue->dispatchs->length;i++){
            
            p = (objv_dispatch_t *) objv_array_objectAt(queue->dispatchs, i);
            
            objv_dispatch_cancelTask(p, task);
        
        }

        objv_array_remove(queue->tasks,(objv_object_t *) task);
        
        objv_mutex_unlock(& queue->mutex);
        
    }
}

void objv_dispatch_queue_cancelAllTasks(objv_dispatch_queue_t * queue){
    
    if(queue){
        
        objv_dispatch_t * p;
        objv_dispatch_task_t * task;
        
        objv_mutex_lock(& queue->mutex);
        
        for(int i=0;i<queue->dispatchs->length;i++){
            
            p = (objv_dispatch_t *) objv_array_objectAt(queue->dispatchs, i);
            
            objv_dispatch_cancelAllTasks(p);
            
        }
        
        for(int i=0;i<queue->tasks->length;i++){
            
            task = (objv_dispatch_task_t *) objv_array_objectAt(queue->tasks, i);
            
            task->canceled = objv_true;
            
        }
        
        objv_array_clear(queue->tasks);
        
        objv_mutex_unlock(& queue->mutex);
    }
    
}

objv_dispatch_queue_t * objv_dispatch_queue_default(){
    
    static objv_dispatch_queue_t * queue = NULL;
    
    if(queue == NULL){
        queue = objv_dispatch_queue_alloc(NULL, "default", DEFAULT_MAX_THREAD_COUNT);
    }
    
    return queue;
}
