//
//  objv_dispatch.h
//  objv
//
//  Created by zhang hailong on 14-1-30.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_dispatch_h
#define objv_objv_dispatch_h

#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_os.h"
#include "objv_array.h"
 
    OBJV_KEY_DEC(DispatchTask)
    OBJV_KEY_DEC(Dispatch)
    OBJV_KEY_DEC(run)
    
    typedef struct _objv_dispatch_task_t {
        objv_object_t base;
        objv_boolean_t READONLY canceled;
        objv_timeinval_t READONLY start;
        unsigned int READONLY version;
        objv_timeinval_t delay;
    } objv_dispatch_task_t;

    typedef void ( * objv_dispatch_task_method_run_t) (objv_class_t * clazz,objv_object_t * object);
    
    void objv_dispatch_task_run(objv_class_t * clazz, objv_dispatch_task_t * task);
    
    OBJV_CLASS_DEC(DispatchTask)
    
    typedef struct _objv_dispatch_t {
        objv_object_t base;
        const char * READONLY name;
        objv_array_t * READONLY tasks;
        objv_mutex_t READONLY mutex;
        objv_waiter_t READONLY waiter;
        objv_timeinval_t READONLY idleTimeinval;
        unsigned int READONLY version;
    } objv_dispatch_t;
    
    OBJV_CLASS_DEC(Dispatch)
    
    objv_dispatch_t * objv_dispatch_alloc(objv_zone_t * zone,const char * name);
    
    int objv_dispatch_run(objv_dispatch_t * dispatch,objv_timeinval_t timeout);

    void objv_dispatch_addTask(objv_dispatch_t * dispatch,objv_dispatch_task_t * task);
    
    void objv_dispatch_cancelTask(objv_dispatch_t * dispatch,objv_dispatch_task_t * task);
    
    void objv_dispatch_cancelAllTasks(objv_dispatch_t * dispatch);
    
    int objv_dispatch_tasks_count(objv_dispatch_t * dispatch);
    
    objv_dispatch_t * objv_dispatch_get_current();
    
    objv_dispatch_t * objv_dispatch_get_main();
    
    void objv_dispatch_set_main(objv_dispatch_t * dispatch);
    
    OBJV_KEY_DEC(DispatchQueue)
    
    typedef struct _objv_dispatch_queue_t {
        objv_object_t base;
        const char * name;
        objv_mutex_t READONLY mutex;
        objv_array_t * READONLY dispatchs;
        objv_array_t * READONLY tasks;
        unsigned int READONLY maxThreadCount;
        unsigned int READONLY index;
    } objv_dispatch_queue_t;
    
    OBJV_CLASS_DEC(DispatchQueue)
    
    objv_dispatch_queue_t * objv_dispatch_queue_alloc(objv_zone_t * zone,const char * name,unsigned int maxThreadCount);
    
    void objv_dispatch_queue_addTask(objv_dispatch_queue_t * queue
                                     ,objv_dispatch_task_t * task);
    
    void objv_dispatch_queue_cancelTask(objv_dispatch_queue_t * queue
                                        ,objv_dispatch_task_t * task);
    
    void objv_dispatch_queue_cancelAllTasks(objv_dispatch_queue_t * queue);
    
    objv_dispatch_queue_t * objv_dispatch_queue_default();
    
#ifdef __cplusplus
}
#endif

#endif
