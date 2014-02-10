//
//  objv_dispatch.c
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

OBJV_KEY_IMP(DispatchTask)
OBJV_KEY_IMP(Dispatch)
OBJV_KEY_IMP(run)

static void objv_dispatch_task_method_run (objv_class_t * clazz, objv_object_t * object){
    
    
}

OBJV_CLASS_METHOD_IMP_BEGIN(DispatchTask)

OBJV_CLASS_METHOD_IMP(run, "v()", objv_dispatch_task_method_run)

OBJV_CLASS_METHOD_IMP_END(DispatchTask)

OBJV_CLASS_IMP_M(DispatchTask, OBJV_CLASS(Object), objv_dispatch_task_t)


void objv_dispatch_task_run(objv_class_t * clazz,objv_dispatch_task_t * task){
    
    if(task){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(run))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            (* (objv_dispatch_task_method_run_t) method->impl)(c, (objv_object_t *)task);
        }
        
    }
    
}


static void objv_dispatch_method_dealloc (objv_class_t * clazz, objv_object_t * object){
    
    objv_dispatch_t * dispatch = (objv_dispatch_t *) object;
    
    objv_mutex_lock(& dispatch->mutex);
    
    objv_object_release((objv_object_t *) dispatch->tasks);
    
    objv_mutex_unlock(& dispatch->mutex);
    
    objv_mutex_destroy(& dispatch->mutex);
    
    objv_waiter_destroy(& dispatch->waiter);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
}

static objv_object_t * objv_dispatch_method_init (objv_class_t * clazz, objv_object_t * object,va_list ap){
    
    if(clazz->superClass){
        object = objv_object_initv(clazz->superClass, object, ap);
    }
    
    if(object){
        objv_dispatch_t * dispatch = (objv_dispatch_t *) object;
        dispatch->name = va_arg(ap, const char *);
        dispatch->tasks = objv_array_alloc(object->zone, 20);
        
        objv_mutex_init(& dispatch->mutex);
        objv_waiter_init(& dispatch->waiter);
    }
    
    return object;
}

OBJV_CLASS_METHOD_IMP_BEGIN(Dispatch)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", objv_dispatch_method_dealloc)

OBJV_CLASS_METHOD_IMP(init, "@(*)", objv_dispatch_method_init)

OBJV_CLASS_METHOD_IMP_END(Dispatch)

OBJV_CLASS_IMP_M(Dispatch, OBJV_CLASS(Object), objv_dispatch_t)


objv_dispatch_t * objv_dispatch_alloc(objv_zone_t * zone,const char * name){
    return (objv_dispatch_t *) objv_object_alloc(zone
                                                 , OBJV_CLASS(Dispatch),name);
}

int objv_dispatch_run(objv_dispatch_t * dispatch,objv_timeinval_t timeout){
    
    objv_dispatch_task_t * task;
    int c = 0,i;
    objv_timeinval_t s = objv_timestamp(),t;
    
    do {
    
        objv_mutex_lock(& dispatch->mutex);
        
        for(i = 0;i<dispatch->tasks->length;i++){
            task = (objv_dispatch_task_t *) objv_array_objectAt(dispatch->tasks, i);
            if(task->delay == 0.0 || objv_timestamp() - task->start >= task->delay){
                break;
            }
        }
        
        if(i < dispatch->tasks->length){
            objv_object_retain((objv_object_t *) task);
            objv_array_removeAt(dispatch->tasks, i);
        }
        else{
            task = NULL;
        }
        
        objv_mutex_unlock(& dispatch->mutex);

        if(task){
            
            dispatch->idleTimeinval = 0;
            
            if(! task->canceled){
                objv_dispatch_task_run(task->base.isa, task);
            }
            
            objv_object_release((objv_object_t *) task);
            
            c ++;
            
        }
        else if(timeout == 0.0 || (t = objv_timestamp() - s) >= timeout){
            
            dispatch->idleTimeinval = 0;
            
            break;
        }
        else{
        
            dispatch->idleTimeinval += timeout - t;
            
            objv_waiter_lock(& dispatch->waiter);
            
            int rs = objv_waiter_wait_timeout(& dispatch->waiter, timeout - t);
            
            objv_waiter_unlock(& dispatch->waiter);
            
            if(ETIMEDOUT == rs){
                break;
            }
            else if(timeout == 0.0 || (t = objv_timestamp() - s) >= timeout){
                break;
            }
        }
        
    } while(1);
    
    return c;
}

void objv_dispatch_addTask(objv_dispatch_t * dispatch,objv_dispatch_task_t * task){
    
    if(dispatch && task){
        
        task->start = objv_timestamp();
        task->canceled = objv_false;
        
        dispatch->idleTimeinval = 0;
        
        objv_mutex_lock(& dispatch->mutex);
        
        objv_array_add(dispatch->tasks, (objv_object_t *) task);
        
        objv_mutex_unlock(& dispatch->mutex);

        objv_waiter_lock(& dispatch->waiter);
        
        objv_waiter_join(& dispatch->waiter);
        
        objv_waiter_unlock(& dispatch->waiter);
    }
}

void objv_dispatch_cancelTask(objv_dispatch_t * dispatch,objv_dispatch_task_t * task){
    
    if(dispatch && task){
        
        objv_mutex_lock(& dispatch->mutex);
        
        task->canceled = objv_true;
        
        objv_array_remove(dispatch->tasks, (objv_object_t *)task);
        
        objv_mutex_unlock(& dispatch->mutex);

    }
}

void objv_dispatch_cancelAllTasks(objv_dispatch_t * dispatch){
    
    if(dispatch ){
        
        objv_mutex_lock(& dispatch->mutex);
        
        objv_dispatch_task_t * task;
        
        for(int i=0;i<dispatch->tasks->length;i++){
            task = (objv_dispatch_task_t *) objv_array_objectAt(dispatch->tasks, i);
            task->canceled = objv_true;
        }
        
        objv_array_clear(dispatch->tasks);
        
        objv_mutex_unlock(& dispatch->mutex);
        
    }
    
}

int objv_dispatch_tasks_count(objv_dispatch_t * dispatch){
    
    if(dispatch ){
        
        int length = 0;
        
        objv_mutex_lock(& dispatch->mutex);
        
        length = dispatch->tasks->length ;
        
        objv_mutex_unlock(& dispatch->mutex);
        
        return length;
        
    }
    
    return 0;
}

static objv_dispatch_t * main_dispatch = NULL;
static objv_thread_key_t dispatch_key = 0;

static void dispatch_key_dealloc(void * dispatch){
    
    objv_object_release((objv_object_t *) dispatch);
    
}

objv_dispatch_t * objv_dispatch_get_current(){
    
    if(dispatch_key == 0){
        objv_thread_key_create(& dispatch_key, dispatch_key_dealloc);
    }
    
    objv_dispatch_t * dispatch = objv_thread_key_value(dispatch_key);
  
    if(dispatch == NULL){
        dispatch = objv_dispatch_alloc(NULL, NULL);
        objv_thread_key_setValue(dispatch_key, dispatch);
    }
   
    return dispatch;
}

objv_dispatch_t * objv_dispatch_get_main(){
    return main_dispatch;
}

void objv_dispatch_set_main(objv_dispatch_t * dispatch){
    
    objv_object_retain((objv_object_t *) dispatch);
    objv_object_release((objv_object_t *) main_dispatch);
    
    main_dispatch = dispatch;
}

