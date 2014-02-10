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

#define DEFAULT_MAX_THREAD_COUNT    20

OBJV_KEY_IMP(DispatchQueue)


static void objv_dispatch_queue_method_dealloc (objv_class_t * clazz
                                                , objv_object_t * object){
    
    objv_dispatch_queue_t * queue = (objv_dispatch_queue_t *) object;
    
    for(int i=0;i<queue->dispatchs->length;i++){
        objv_dispatch_cancelAllTasks((objv_dispatch_t *) objv_array_objectAt(queue->dispatchs, i));
    }
    
    objv_object_release((objv_object_t *) queue->dispatchs);
    
    objv_object_release((objv_object_t *) queue->dispatch);
    
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
        queue->threadCount = 0;
        queue->dispatch = (objv_dispatch_t *) objv_object_retain( (objv_object_t *) objv_dispatch_get_current() );
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

OBJV_KEY_DEC(DispatchQueueTask)

OBJV_KEY_IMP(DispatchQueueTask)

typedef struct _objv_dispatch_queue_task_t{
    objv_dispatch_task_t base;
    objv_dispatch_queue_t * queue;
    objv_dispatch_task_t * task;
    objv_dispatch_t * dispatch;
    objv_boolean_t removed;
    objv_boolean_t canceled;
    objv_boolean_t added;
    objv_boolean_t canceledAll;
} objv_dispatch_queue_task_t;


static void objv_dispatch_queue_task_method_dealloc (objv_class_t * clazz
                                                , objv_object_t * object){
    
    objv_dispatch_queue_task_t * task = (objv_dispatch_queue_task_t *) object;
    
    objv_object_release((objv_object_t *) task->task);
    
    objv_object_release((objv_object_t *) task->queue);
    
    objv_object_release((objv_object_t *) task->dispatch);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
}

static void objv_dispatch_queue_task_method_run (objv_class_t * clazz
                                                     , objv_object_t * object){
    
    objv_dispatch_queue_task_t * task = (objv_dispatch_queue_task_t *) object;
    
    objv_dispatch_queue_t * queue = task->queue;
    
    if(task->added){
        
        objv_dispatch_queue_addTask(queue, task->task);
        
    }
    else if(task->canceled){
        
        objv_dispatch_queue_cancelTask(queue, task->task);
        
    }
    else if(task->canceledAll){
        
        objv_dispatch_queue_cancelAllTasks(queue);
        
    }
    else if(task->removed){
        
        objv_array_remove(queue->dispatchs, (objv_object_t *) task->dispatch);
        
        queue->threadCount --;
        
        objv_mutex_lock(&task->dispatch->mutex);
        
        for(int i=0;i<task->dispatch->tasks->length;i++){
            
            objv_dispatch_queue_addTask(queue,(objv_dispatch_task_t *)objv_array_objectAt(task->dispatch->tasks, i));
            
        }
        
        objv_mutex_unlock(&task->dispatch->mutex);
        
    }
    else if(task->dispatch) {
        objv_array_add(queue->dispatchs, (objv_object_t *) task->dispatch);
    }
}


static objv_method_t objv_dispatch_queue_task_methods[] = {
    {OBJV_KEY(dealloc),"v()",(objv_method_impl_t) objv_dispatch_queue_task_method_dealloc}
    ,{OBJV_KEY(run),"v()",(objv_method_impl_t) objv_dispatch_queue_task_method_run}
};

objv_class_t objv_dispatch_queue_task_class = {OBJV_KEY(DispatchQueueTask)
    ,OBJV_CLASS(DispatchTask)
    ,objv_dispatch_queue_task_methods
    ,sizeof(objv_dispatch_queue_task_methods) / sizeof(objv_method_t)
    ,NULL,0
    ,sizeof(objv_dispatch_queue_task_t)
    ,NULL,0};


static void * objv_dispatch_queue_thread(void * userInfo){
    
    objv_autorelease_pool_push();
    
    objv_dispatch_queue_task_t * task = (objv_dispatch_queue_task_t *) userInfo;
    
    objv_dispatch_queue_t * queue = task->queue;
    
    objv_dispatch_t * dispatch = objv_dispatch_get_current();
    
    task->dispatch = (objv_dispatch_t *) objv_object_retain((objv_object_t *) dispatch);
    
    objv_dispatch_addTask(dispatch, task->task);
    
    objv_object_release((objv_object_t *)task->task);
    
    task->task = NULL;
    
    objv_dispatch_addTask(queue->dispatch, (objv_dispatch_task_t *) task);
 
    while (dispatch->idleTimeinval < 20) {
        
        objv_autorelease_pool_push();
        
        objv_dispatch_run(dispatch,0.02);
        
        objv_autorelease_pool_pop();
    }
    
    task->removed = objv_true;
    
    objv_dispatch_addTask(queue->dispatch, (objv_dispatch_task_t *) task);
  
    objv_object_release((objv_object_t *) task);
    
    objv_autorelease_pool_pop();
    
    return NULL;
}

void objv_dispatch_queue_addTask(objv_dispatch_queue_t * queue
                                 ,objv_dispatch_task_t * task){
    
    if(queue && task){
        
        if(queue->dispatch != objv_dispatch_get_current()){
            
            objv_dispatch_queue_task_t * t = (objv_dispatch_queue_task_t *) objv_object_alloc(queue->base.zone, &objv_dispatch_queue_task_class);
            
            t->queue = (objv_dispatch_queue_t *) objv_object_retain((objv_object_t *)queue);
            t->task = (objv_dispatch_task_t *) objv_object_retain((objv_object_t *) task);
            t->added = objv_true;
            
            objv_dispatch_addTask(queue->dispatch, (objv_dispatch_task_t *)t);
            
            objv_object_release((objv_object_t *) t);
            
            return;
        }
        
        objv_dispatch_t * disp = NULL, * p;
        
        for(int i=0;i<queue->dispatchs->length;i++){
            
            p = (objv_dispatch_t *) objv_array_objectAt(queue->dispatchs, i);
            
            if(disp == NULL){
                disp = p;
            }
            else if(p->idleTimeinval > disp->idleTimeinval){
                disp = p;
            }
        }

        
        if((disp == NULL || disp->idleTimeinval == 0.0)
           && (queue->maxThreadCount == 0
               || queue->threadCount < queue->maxThreadCount)){
            
            queue->threadCount ++;
            
            objv_dispatch_queue_task_t * t = (objv_dispatch_queue_task_t *) objv_object_alloc(queue->base.zone, &objv_dispatch_queue_task_class);
            
            t->queue = (objv_dispatch_queue_t *) objv_object_retain((objv_object_t *)queue);
            t->task = (objv_dispatch_task_t *) objv_object_retain((objv_object_t *) task);
            
            objv_thread_create(objv_dispatch_queue_thread, t);
        
        }
        else if(disp){
            
            objv_dispatch_addTask(disp, task);
        
        }
        else{
            assert(0);
        }
    }
}

void objv_dispatch_queue_cancelTask(objv_dispatch_queue_t * queue
                                    ,objv_dispatch_task_t * task){
    if(queue && task){
        
        if(queue->dispatch != objv_dispatch_get_current()){
            
            objv_dispatch_queue_task_t * t = (objv_dispatch_queue_task_t *) objv_object_alloc(queue->base.zone, &objv_dispatch_queue_task_class);
            
            t->queue = (objv_dispatch_queue_t *) objv_object_retain((objv_object_t *)queue);
            t->task = (objv_dispatch_task_t *) objv_object_retain((objv_object_t *) task);
            t->canceled = objv_true;
            
            objv_dispatch_addTask(queue->dispatch, (objv_dispatch_task_t *)t);
            
            objv_object_release((objv_object_t *) t);
            
            return;
        }
        
        objv_dispatch_t * p;
        
        for(int i=0;i<queue->dispatchs->length;i++){
            
            p = (objv_dispatch_t *) objv_array_objectAt(queue->dispatchs, i);
            
            objv_dispatch_cancelTask(p, task);
        
        }
        
    }
}

void objv_dispatch_queue_cancelAllTasks(objv_dispatch_queue_t * queue){
    
    if(queue){
        
        if(queue->dispatch != objv_dispatch_get_current()){
            
            objv_dispatch_queue_task_t * t = (objv_dispatch_queue_task_t *) objv_object_alloc(queue->base.zone, &objv_dispatch_queue_task_class);
            
            t->queue = (objv_dispatch_queue_t *) objv_object_retain((objv_object_t *)queue);
            t->canceledAll = objv_true;
            
            objv_dispatch_addTask(queue->dispatch, (objv_dispatch_task_t *)t);
            
            objv_object_release((objv_object_t *) t);
            
            return;
        }
      
        objv_dispatch_t * p;
        
        for(int i=0;i<queue->dispatchs->length;i++){
            
            p = (objv_dispatch_t *) objv_array_objectAt(queue->dispatchs, i);
            
            objv_dispatch_cancelAllTasks(p);
            
        }
        
    }
    
}

objv_dispatch_queue_t * objv_dispatch_queue_default(){
    
    static objv_dispatch_queue_t * queue = NULL;
    
    if(queue == NULL){
        queue = objv_dispatch_queue_alloc(NULL, "default", DEFAULT_MAX_THREAD_COUNT);
    }
    
    return queue;
}
