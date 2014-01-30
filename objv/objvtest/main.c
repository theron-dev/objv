//
//  main.c
//  objvtest
//
//  Created by zhang hailong on 14-1-30.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#include <stdio.h>

#include "objv_os.h"
#include "objv.h"
#include "objv_dispatch.h"
#include "objv_string.h"
#include "objv_value.h"
#include "objv_log.h"
#include "objv_autorelease.h"

OBJV_KEY_DEC(ConsoleTask)

OBJV_KEY_IMP(ConsoleTask)


static void ConsoleTaskMethodAction (objv_class_t * clazz, objv_object_t * object){
    
    objv_dispatch_delay_task_t * task = (objv_dispatch_delay_task_t *) object;
    
    objv_string_t * s = objv_string_new(object->zone, "ok");
    
    objv_log("\n%s\n",s->UTF8String);
    
    task->start = 0;
    
    objv_dispatch_addTask(objv_dispatch_get_current(), (objv_dispatch_task_t *) object);
    
}


static objv_method_t ConsoleTaskMethods[] = {
    {OBJV_KEY(action),"v()",(objv_method_impl_t)ConsoleTaskMethodAction}
};

objv_class_t ConsoleTaskClass = { OBJV_KEY(ConsoleTask),& objv_dispatch_delay_task_class
    ,ConsoleTaskMethods,sizeof(ConsoleTaskMethods) / sizeof(objv_method_t)
    ,NULL,0
    ,sizeof(objv_dispatch_delay_task_t)
    ,NULL,0,0};


int main(int argc, const char * argv[])
{

    objv_dispatch_set_main(objv_dispatch_get_current());
    
    objv_dispatch_delay_task_t * task = (objv_dispatch_delay_task_t *) objv_object_alloc(NULL, & ConsoleTaskClass);
    
    task->delay = 0.3;
    
    objv_dispatch_addTask(objv_dispatch_get_current(), (objv_dispatch_task_t *) task);
    
    objv_object_release((objv_object_t *) task);
    
    objv_timeinval_t t;

    while (1) {
        
        objv_autorelease_pool_t * pool = objv_autorelease_pool_alloc(NULL);
        
        t = objv_dispatch_run(objv_dispatch_get_current());
        
        objv_object_release((objv_object_t *)pool);
        
        if(t < 0.02){
            
            usleep(200);
        }
        
    }

    // insert code here...
    printf("Hello, World!\n");
    return 0;
}

