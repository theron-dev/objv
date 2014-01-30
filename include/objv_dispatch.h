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
    OBJV_KEY_DEC(DispatchDelayTask)
    OBJV_KEY_DEC(Dispatch)
    OBJV_KEY_DEC(run)
    OBJV_KEY_DEC(action)
    
    typedef struct _objv_dispatch_task_t {
        objv_object_t base;
    } objv_dispatch_task_t;

    typedef void ( * objv_dispatch_task_method_run_t) (objv_class_t * clazz,objv_object_t * object);
    
    void objv_dispatch_task_run(objv_class_t * clazz, objv_dispatch_task_t * task);
    
    extern objv_class_t objv_dispatch_task_class;
    
    
    typedef void ( * objv_dispatch_delay_task_method_action_t) (objv_class_t * clazz,objv_object_t * object);
    
    typedef struct _objv_dispatch_delay_task_t {
        objv_dispatch_task_t base;
        objv_timeinval_t start;
        objv_timeinval_t delay;
    } objv_dispatch_delay_task_t;
    
    objv_dispatch_delay_task_t * objv_dispatch_delay_task_alloc(objv_zone_t * zone,objv_timeinval_t delay);
    
    void objv_dispatch_delay_task_action(objv_class_t * clazz,objv_dispatch_delay_task_t * task);
    
    extern objv_class_t objv_dispatch_delay_task_class;
    
    typedef struct _objv_dispatch_t {
        objv_object_t base;
        const char * name;
        objv_array_t * tasks;
        objv_mutex_t mutex;
    } objv_dispatch_t;
    
    extern objv_class_t objv_dispatch_class;
 
    objv_dispatch_t * objv_dispatch_alloc(objv_zone_t * zone,const char * name);
    
    objv_timeinval_t objv_dispatch_run(objv_dispatch_t * dispatch);

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
        objv_array_t * READONLY tasks;
        objv_mutex_t READONLY tasks_mutex;
        objv_array_t * READONLY dispatchs;
        unsigned int READONLY maxThreadCount;
        unsigned int READONLY threadCount;
    } objv_dispatch_queue_t;
    
    extern objv_class_t objv_dispatch_queue_class;
    
    objv_dispatch_queue_t * objv_dispatch_queue_alloc(objv_zone_t * zone,const char * name,unsigned int maxThreadCount);
    
    void objv_dispatch_queue_addTask(objv_dispatch_queue_t * dispatch
                                     ,objv_dispatch_task_t * task);
    
    void objv_dispatch_queue_cancelTask(objv_dispatch_queue_t * dispatch
                                        ,objv_dispatch_task_t * task);
    
    void objv_dispatch_queue_cancelAllTasks(objv_dispatch_queue_t * dispatch);
    
#ifdef __cplusplus
}
#endif

#endif
