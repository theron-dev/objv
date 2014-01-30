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

OBJV_KEY_IMP(DispatchQueue)


static void objv_dispatch_queue_method_dealloc (objv_class_t * clazz
                                                , objv_object_t * object){
    
    objv_dispatch_queue_t * dispatch = (objv_dispatch_queue_t *) object;
    
    for(int i=0;i<dispatch->dispatchs->length;i++){
        objv_dispatch_cancelAllTasks((objv_dispatch_t *) objv_array_objectAt(dispatch->dispatchs, i));
    }
    
    objv_object_release((objv_object_t *) dispatch->dispatchs);
    
    objv_mutex_lock(& dispatch->tasks_mutex);
    
    objv_object_release((objv_object_t *) dispatch->tasks);
    
    objv_mutex_unlock(& dispatch->tasks_mutex);

    objv_mutex_destroy(& dispatch->tasks_mutex);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
}

static objv_method_t objv_dispatch_queue_methods[] = {
    {OBJV_KEY(dealloc),"v()",(objv_method_impl_t)objv_dispatch_queue_method_dealloc}
};

objv_class_t objv_dispatch_queue_class = {OBJV_KEY(DispatchQueue),& objv_object_class
    ,objv_dispatch_queue_methods,sizeof(objv_dispatch_queue_methods) / sizeof(objv_method_t)
    ,NULL,0
    ,sizeof(objv_dispatch_queue_t)
    ,NULL,0,0};


objv_dispatch_queue_t * objv_dispatch_queue_alloc(objv_zone_t * zone,const char * name,unsigned int maxThreadCount){
    
    objv_dispatch_queue_t * dispatch = (objv_dispatch_queue_t *) objv_object_alloc(zone, &objv_dispatch_queue_class);
    
    objv_mutex_init(& dispatch->tasks_mutex);
    
    dispatch->tasks = objv_array_alloc(zone, 20);
    dispatch->dispatchs = objv_array_alloc(zone, 20);
    dispatch->threadCount = 0;
    dispatch->maxThreadCount = maxThreadCount;
    
    return dispatch;
}

OBJV_KEY_DEC(DispatchQueueTask)

OBJV_KEY_IMP(DispatchQueueTask)

typedef struct _objv_dispatch_queue_task_t{
    objv_dispatch_task_t base;
    objv_dispatch_queue_t * queue;
    objv_dispatch_task_t * task;
    objv_dispatch_t * dispatch;
    objv_boolean_t removed;
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
    
    if(task->removed){
        
        
        objv_array_remove(queue->dispatchs, (objv_object_t *) task->dispatch);
        
        queue->threadCount --;
        
        objv_mutex_lock(&task->dispatch->mutex);
        
        for(int i=0;i<task->dispatch->tasks->length;i++){
            
            objv_dispatch_queue_addTask(queue,(objv_dispatch_task_t *)objv_array_objectAt(task->dispatch->tasks, i));
            
        }
        
        objv_mutex_unlock(&task->dispatch->mutex);
        
        objv_object_release(object);
        
    }
    else{
        objv_array_add(queue->dispatchs, (objv_object_t *) task->dispatch);
    }
}


static objv_method_t objv_dispatch_queue_task_methods[] = {
    {OBJV_KEY(dealloc),"v()",(objv_method_impl_t) objv_dispatch_queue_task_method_dealloc}
    ,{OBJV_KEY(run),"v()",(objv_method_impl_t) objv_dispatch_queue_task_method_run}
};

objv_class_t objv_dispatch_queue_task_class = {OBJV_KEY(DispatchQueueTask)
    ,& objv_dispatch_task_class
    ,objv_dispatch_queue_task_methods
    ,sizeof(objv_dispatch_queue_task_methods) / sizeof(objv_method_t)
    ,NULL,0
    ,sizeof(objv_dispatch_queue_task_t)
    ,NULL,0,0};


static void * objv_dispatch_queue_thread(void * userInfo){
    
    objv_dispatch_queue_task_t * task = (objv_dispatch_queue_task_t *) userInfo;
    
    objv_dispatch_t * dispatch = objv_dispatch_get_current();
    
    task->dispatch = (objv_dispatch_t *) objv_object_retain((objv_object_t *) dispatch);
    
    objv_dispatch_addTask(dispatch, task->task);
    
    objv_dispatch_addTask(objv_dispatch_get_main(), (objv_dispatch_task_t *) task);
    
    objv_timeinval_t keepAlive = 0,t;
    
    while (keepAlive < 20) {
        
        t = objv_dispatch_run(dispatch);
        
        if(t < 0.02){
            usleep(200);
            keepAlive += 0.2;
        }
        else{
            keepAlive = 0;
        }
    }
    
    task->removed = objv_true;
    
    objv_dispatch_addTask(objv_dispatch_get_main(), (objv_dispatch_task_t *) task);
    
    return NULL;
}

void objv_dispatch_queue_addTask(objv_dispatch_queue_t * dispatch
                                 ,objv_dispatch_task_t * task){
    
    if(dispatch && task){
        
        if(dispatch->maxThreadCount == 0
           || dispatch->threadCount < dispatch->maxThreadCount){
            
            dispatch->threadCount ++;
            
            objv_dispatch_queue_task_t * t = (objv_dispatch_queue_task_t *) objv_object_alloc(dispatch->base.zone, &objv_dispatch_queue_task_class);
            
            t->queue = (objv_dispatch_queue_t *) objv_object_retain((objv_object_t *)dispatch);
            t->task = (objv_dispatch_task_t *) objv_object_retain((objv_object_t *) task);
            
            objv_thread_create(objv_dispatch_queue_thread, t);
        
        }
        else{
            
            objv_dispatch_t * disp = NULL, * p;
            int length = 0;
            
            for(int i=0;i<dispatch->dispatchs->length;i++){
                
                p = (objv_dispatch_t *) objv_array_objectAt(dispatch->dispatchs, i);
                
                if(disp == NULL){
                    disp = p;
                    length = objv_dispatch_tasks_count(disp);
                }
                else if(objv_dispatch_tasks_count(p) < length){
                    disp = p;
                    length = objv_dispatch_tasks_count(p);
                }
            }
            
            if(disp){
                
                objv_dispatch_addTask(disp, task);
                
            }
            else{
                
                objv_mutex_lock(& dispatch->tasks_mutex);
                
                objv_array_add(dispatch->tasks,(objv_object_t *)task);
                
                objv_mutex_unlock(& dispatch->tasks_mutex);

            }
        }
    }
}

void objv_dispatch_queue_cancelTask(objv_dispatch_queue_t * dispatch
                                    ,objv_dispatch_task_t * task){
    if(dispatch && task){
        
        objv_mutex_lock(& dispatch->tasks_mutex);
        
        objv_array_remove(dispatch->tasks,(objv_object_t *)task);
        
        objv_mutex_unlock(& dispatch->tasks_mutex);
        
        objv_dispatch_t * p;
        
        for(int i=0;i<dispatch->dispatchs->length;i++){
            
            p = (objv_dispatch_t *) objv_array_objectAt(dispatch->dispatchs, i);
            
            objv_dispatch_cancelTask(p, task);
        
        }
        
    }
}

void objv_dispatch_queue_cancelAllTasks(objv_dispatch_queue_t * dispatch){
    
    if(dispatch){
        
        objv_mutex_lock(& dispatch->tasks_mutex);
        
        objv_array_clear(dispatch->tasks);
        
        objv_mutex_unlock(& dispatch->tasks_mutex);
      
        objv_dispatch_t * p;
        
        for(int i=0;i<dispatch->dispatchs->length;i++){
            
            p = (objv_dispatch_t *) objv_array_objectAt(dispatch->dispatchs, i);
            
            objv_dispatch_cancelAllTasks(p);
            
        }
        
    }
    
}
