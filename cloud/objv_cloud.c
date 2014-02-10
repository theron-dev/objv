//
//  objv_cloud.c
//  objv
//
//  Created by zhang hailong on 14-2-9.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv.h"
#include "objv_cloud.h"
#include "objv_value.h"
#include "objv_log.h"

OBJV_KEY_IMP(CLContext)

static void CLContextMethodDealloc(objv_class_t * clazz,objv_object_t * object){
    
    CLContext * ctx = (CLContext *) object,* c;
    
    int i;
    
    if(ctx->childs){
    
        for (i=0; i<ctx->childs->length; i ++) {
            
            c = (CLContext * ) objv_array_objectAt(ctx->childs, i);
            
            c->parent = NULL;
            
        }
        
    }
    
    objv_object_release((objv_object_t *) ctx->childs);
    objv_object_release((objv_object_t *) ctx->config);
    objv_object_release((objv_object_t *) ctx->domain);
    objv_object_release((objv_object_t *) ctx->dispatch);
 
    if(clazz->superClass){
        
        objv_object_dealloc(clazz->superClass, object);
        
    }
}

static objv_object_t * CLContextMethodInit(objv_class_t * clazz,objv_object_t * object,va_list ap){
    
    if(clazz->superClass){
        object = objv_object_initv(clazz->superClass, object, ap);
    }
    
    if(object){
        
        CLContext * ctx = (CLContext *) object;
        ctx->dispatch = (objv_dispatch_t *) objv_object_retain((objv_object_t *) objv_dispatch_get_current());
    }
    
    return object;
}

static void CLContextMethodSetConfig(objv_class_t * clazz,CLContext * ctx,objv_object_t * config){
    
    
    if(ctx->config != config){
        
        objv_object_retain(config);
        objv_object_release(ctx->config);
        
        ctx->config = config;
        
    }
    
}



OBJV_CLASS_METHOD_IMP_BEGIN(CLContext)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", CLContextMethodDealloc)

OBJV_CLASS_METHOD_IMP(init, "@(*)", CLContextMethodInit)

OBJV_CLASS_METHOD_IMP(setConfig, "v(@)", CLContextMethodSetConfig)

OBJV_CLASS_METHOD_IMP_END(CLContext)

OBJV_CLASS_IMP_M(CLContext, OBJV_CLASS(Object), CLContext)

typedef enum _CLContextDispatchTaskInvoke{
    CLContextDispatchTaskInvokeSendTask,CLContextDispatchTaskInvokeHandleTask
} CLContextDispatchTaskInvoke;

typedef struct _CLContextDispatchTask{
    objv_dispatch_task_t base;
    CLContext * context;
    objv_class_t * taskType;
    CLTask * task;
    CLContextDispatchTaskInvoke invoke;
} CLContextDispatchTask;

OBJV_KEY_DEC(CLContextDispatchTask)


static void CLContextDispatchTaskMethodDealloc(objv_class_t * clazz,objv_object_t * object){
    
    CLContextDispatchTask * dispatchTask = (CLContextDispatchTask *) object;
    
    objv_object_release((objv_object_t *) dispatchTask->task);
    objv_object_release((objv_object_t *) dispatchTask->context);
    
    if(clazz->superClass){
        
        objv_object_dealloc(clazz->superClass, object);
        
    }
    
}

static void CLContextDispatchTaskMethodRun (objv_class_t * clazz,objv_object_t * object){
    
    CLContextDispatchTask * dispatchTask = (CLContextDispatchTask *) object;
    
    switch (dispatchTask->invoke) {
        case CLContextDispatchTaskInvokeSendTask:
        
            CLContextSendTask(dispatchTask->context, dispatchTask->taskType, dispatchTask->task);
            
            break;
        case CLContextDispatchTaskInvokeHandleTask:
            
            CLContextHandleTask(dispatchTask->context, dispatchTask->taskType, dispatchTask->task);
            
            break;
        default:
            break;
    }
    
}

static objv_object_t * CLContextDispatchTaskMethodInit(objv_class_t * clazz,objv_object_t * object,va_list ap){
    
    if(clazz->superClass){
        object = objv_object_initv(clazz->superClass, object, ap);
    }
    
    if(object){
        
        CLContextDispatchTask * dispatchTask = (CLContextDispatchTask *) object;
        CLContext * context = va_arg(ap, CLContext *);
        objv_class_t * taskType = context ? va_arg(ap, objv_class_t *): NULL;
        CLTask * task = taskType ? va_arg(ap, CLTask *) : NULL;
        CLContextDispatchTaskInvoke invoke = task ? va_arg(ap, CLContextDispatchTaskInvoke) : CLContextDispatchTaskInvokeSendTask;
        
        if(taskType && task){
            
            dispatchTask->taskType = taskType;
            dispatchTask->task = (CLTask *) objv_object_retain((objv_object_t *) task);
            dispatchTask->invoke = invoke;
        }
        
    }
    
    return object;
}


OBJV_CLASS_METHOD_IMP_BEGIN(CLContextDispatchTask)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", CLContextDispatchTaskMethodDealloc)

OBJV_CLASS_METHOD_IMP(init, "@(*)", CLContextDispatchTaskMethodInit)

OBJV_CLASS_METHOD_IMP(run, "v()", CLContextDispatchTaskMethodRun)

OBJV_CLASS_METHOD_IMP_END(CLContextDispatchTask)

OBJV_CLASS_IMP_M(CLContextDispatchTask, OBJV_CLASS(DispatchTask), CLContextDispatchTask)

void CLContextHandleTask(CLContext * context, objv_class_t * taskType, CLTask * task){
    if(context && taskType && task){
        
        if(context->dispatch == objv_dispatch_get_current()){
            
            objv_class_t * c = context->base.isa;
            
            objv_method_t * method = NULL;
            
            while(c && (method = objv_class_getMethod(c, OBJV_KEY(handleTask))) == NULL){
                
                c = c->superClass;
            }
            
            if(method){
                (* (CLContextHandleTaskFun) method->impl)(c,context,taskType,task);
            }
        }
        else{
            CLContextDispatchTask * dispatchTask = (CLContextDispatchTask *) objv_object_alloc(context->base.zone, OBJV_CLASS(DispatchTask),context,taskType,task,CLContextDispatchTaskInvokeHandleTask,NULL);
            
            objv_dispatch_addTask(context->dispatch, (objv_dispatch_task_t *) dispatchTask);
            
            objv_object_release((objv_object_t *) dispatchTask);
        }
    }
}

static void _CLContextSendTask(CLContext * context, objv_class_t * taskType, CLTask * task){
    
    objv_class_t * c = context->base.isa;
    
    objv_method_t * method = NULL;
    
    while(c && (method = objv_class_getMethod(c, OBJV_KEY(sendTask))) == NULL){
        
        c = c->superClass;
    }
    
    if(method){
        (* (CLContextSendTaskFun) method->impl)(c,context,taskType,task);
    }
    
}

void CLContextSendTask(CLContext * context, objv_class_t * taskType, CLTask * task){
    if(context && taskType && task){
        
        if(context->dispatch == objv_dispatch_get_current()){
            
            if(task->target == NULL){
                task->target = (objv_string_t *) objv_object_retain((objv_object_t *) context->domain);
            }
            
            if(task->source == NULL){
                task->identifier = ++ context->identifier;
                task->source = (CLContext *) objv_object_retain( (objv_object_t *) context);
            }
            
            if( objv_string_hasPrefix(task->target->UTF8String, context->domain->UTF8String) ){
                {
                    const char * lp;
                    int i;
                    CLContext * ctx;
                    
                    if((lp = objv_string_hasSuffix(task->target->UTF8String, ".*"))){
                        
                        if(objv_string_hasPrefixTo(context->domain->UTF8String,task->target->UTF8String, lp)){
                            _CLContextSendTask(context,taskType,task);
                        }
                        
                        for(i=0;i<context->childs->length;i++){
                            ctx = (CLContext *) objv_array_objectAt(context->childs, i);
                            if(objv_string_hasPrefixTo(ctx->domain->UTF8String,task->target->UTF8String, lp)){
                                CLContextSendTask(ctx,taskType,task);
                            }
                        }
                        
                    }
                    else{
                        
                        if(strcmp(task->target->UTF8String, context->domain->UTF8String) == 0){
                            _CLContextSendTask(context,taskType,task);
                        }
                        else if(context->childs){
                        
                            for(i=0;i<context->childs->length;i++){
                                ctx = (CLContext *) objv_array_objectAt(context->childs, i);
                                if(strcmp(task->target->UTF8String, ctx->domain->UTF8String) == 0){
                                    CLContextSendTask(ctx,taskType,task);
                                    break;
                                }
                            }
                            
                        }
                    }
                }

            }
            else if( context->parent ){
                CLContextSendTask( context->parent , taskType, task);
            }
            
        }
        else{
            CLContextDispatchTask * dispatchTask = (CLContextDispatchTask *) objv_object_alloc(context->base.zone, OBJV_CLASS(DispatchTask),context,taskType,task,CLContextDispatchTaskInvokeSendTask,NULL);
            
            objv_dispatch_addTask(context->dispatch, (objv_dispatch_task_t *) dispatchTask);
            
            objv_object_release((objv_object_t *) dispatchTask);
        }
        
    }
}

void CLContextAddChild(CLContext * context, CLContext * child){
    if(context && child && child->parent == NULL){
        
        child->parent = context;
        
        if(context->childs == NULL){
            context->childs = objv_array_alloc(context->base.zone, 4);
        }
        
        objv_array_add(context->childs, (objv_object_t *) child);
        
    }
}

OBJV_KEY_IMP(setConfig)

void CLContextSetConfig(objv_class_t * clazz, CLContext * context,objv_object_t * config){
    
    if(clazz && context){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(setConfig))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            (* (CLContextSetConfigFun) method->impl)(c,context,config);
        }
        
    }
}

OBJV_KEY_IMP(CLTask)

static void CLTaskMethodDealloc(objv_class_t * clazz,objv_object_t * object){
    
    CLTask * task = (CLTask *) object;
    
    objv_object_retain((objv_object_t *) task->target);
    objv_object_retain((objv_object_t *) task->source);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
}


OBJV_CLASS_METHOD_IMP_BEGIN(CLTask)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", CLTaskMethodDealloc)

OBJV_CLASS_METHOD_IMP_END(CLTask)

OBJV_CLASS_IMP_M(CLTask, OBJV_CLASS(Object), CLTask)


OBJV_KEY_IMP(CLService)

static void CLServiceMethodDealloc(objv_class_t * clazz,objv_object_t * object){
    
    CLService * service = (CLService *) object;
    
    objv_object_release((objv_object_t *) service->config);
    
    objv_hash_map_dealloc(service->taskTypes);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
}

static void CLServiceMethodHandleTask(objv_class_t * clazz,CLService * service,objv_class_t * taskType,CLTask * task){
    
}

static objv_object_t * CLServiceMethodInit(objv_class_t * clazz,objv_object_t * object,va_list ap){
    
    if(clazz->superClass){
        object = objv_object_initv(clazz->superClass, object, ap);
    }
    
    if(object){
        
        CLService * srv = (CLService *) object;
        
        srv->taskTypes = objv_hash_map_alloc(4, objv_hash_map_hash_code_ptr, objv_map_compare_any);
        
    }
    
    return object;
}

OBJV_CLASS_METHOD_IMP_BEGIN(CLService)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", CLServiceMethodDealloc)

OBJV_CLASS_METHOD_IMP(handleTask, "v(*,@)", CLServiceMethodHandleTask)

OBJV_CLASS_METHOD_IMP(init, "@()", CLServiceMethodInit)

OBJV_CLASS_METHOD_IMP_END(CLService)

OBJV_CLASS_IMP_M(CLService, OBJV_CLASS(Object), CLService)


void CLServiceSetConfig(CLService * service, objv_object_t * config){
    if(service && config){
        objv_zone_t * zone = service->base.zone;
        objv_array_t * taskTypes = (objv_array_t *) objv_object_objectForKey(config->isa, config, (objv_object_t *)objv_string_new_nocopy(zone, "taskTypes"));
        objv_string_t * string;
        objv_class_t * taskType;
        
        if(taskTypes && objv_object_isKindOfClass((objv_object_t *)taskTypes, OBJV_CLASS(Array))){
            for(int i=0;i<taskTypes->length;i++){
                string = objv_object_stringValue(objv_array_objectAt(taskTypes, i), NULL);
                if(string){
                    taskType = objv_class(objv_key( string->UTF8String ));
                    if(taskType){
                        objv_hash_map_put(service->taskTypes, taskType, taskType);
                    }
                    else{
                        objv_log("Error: Not found taskType %s\n",string->UTF8String);
                    }
                }
            }
        }
        
        service->inherit = objv_object_booleanValueForKey(config, (objv_object_t *) objv_string_new_nocopy(zone, "inherit"), objv_false);
    }
}

void CLServiceAddTaskType(CLService * service,objv_class_t * taskType){
    if(service && taskType){
        objv_hash_map_put(service->taskTypes, taskType, taskType);
    }
}

objv_boolean_t CLServiceHasTaskType(CLService * service,objv_class_t * taskType){
    
    if(service && taskType){
        
        int i;
        objv_class_t * clazz;
        
        if(service->inherit){
            
            for(i=0;i<service->taskTypes->length;i++){
                
                clazz = (objv_class_t *) objv_hash_map_keyAt(service->taskTypes, i);
                
                if(objv_class_isKindOfClass(clazz, taskType)){
                    return objv_true;
                }
                
            }
            
        }
        else{
            return objv_hash_map_get(service->taskTypes, taskType) != NULL;
        }
        
    }
    
    return objv_false;
    
}

objv_boolean_t CLServiceHandleTask(CLService * service,CLContext * ctx,objv_class_t * taskType,CLTask * task){
    if(service && ctx && taskType && task){
        
        objv_class_t * c = service->base.isa;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(handleTask))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (CLServicetHandleTaskFun) method->impl)(c,service,ctx,taskType,task);
        }
    }
    return objv_false;
}

OBJV_KEY_IMP(CLServiceContainer)

static void CLServiceContainerMethodDealloc(objv_class_t * clazz,objv_object_t * object){
    
    CLServiceContainer * container = (CLServiceContainer *) object;
    
    objv_object_release((objv_object_t *) container->config);
    objv_object_release((objv_object_t *) container->services);
    
    if(clazz->superClass){
        
        objv_object_dealloc(clazz->superClass, object);
    }

}

static objv_object_t * CLServiceContainerMethodInit(objv_class_t * clazz,objv_object_t * object,va_list ap){
    
    if(clazz->superClass){
        object = objv_object_initv(clazz, object, ap);
    }
    
    if(object){
        CLServiceContainer * container = (CLServiceContainer *) object;
        container->services = objv_array_alloc(object->zone, 4);
    }
    
    return object;
}

OBJV_CLASS_METHOD_IMP_BEGIN(CLServiceContainer)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", CLServiceContainerMethodDealloc)

OBJV_CLASS_METHOD_IMP(init, "@(*)", CLServiceContainerMethodInit)

OBJV_CLASS_METHOD_IMP_END(CLServiceContainer)

OBJV_CLASS_IMP_M(CLServiceContainer, OBJV_CLASS(Object), CLServiceContainer)

void CLServiceContainerAddService(CLServiceContainer * container,CLService * service){
    if(container && service){
        objv_array_add(container->services, (objv_object_t *) service);
    }
}

void CLServiceContainerSetConfig(CLServiceContainer * container,objv_object_t * config){
    if(container && config){
        objv_zone_t * zone = container->base.zone;
        objv_array_t * services = (objv_array_t *) objv_object_objectForKey(config->isa, config, (objv_object_t *) objv_string_new_nocopy(zone, "services"));
        int i;
        objv_object_t * cfg;
        objv_string_t * string;
        objv_class_t * clazz;
        CLService * service;
        
        objv_object_retain(config);
        objv_object_release(container->config);
        
        container->config = config;
        
        if(services && objv_object_isKindOfClass((objv_object_t *)services, OBJV_CLASS(Array))){
            
            for(i=0;i<services->length;i++){
                
                cfg = objv_array_objectAt(services, i);
                
                string = objv_object_stringValueForKey(cfg, (objv_object_t *) objv_string_new_nocopy(zone, "class"), NULL);
                
                if(string){
                    clazz = objv_class(objv_key(string->UTF8String));
                    if(clazz){
                        
                        service = (CLService *) objv_object_alloc(zone, clazz,NULL);
                        
                        if(service){
                            
                            CLServiceSetConfig(service, cfg);
                            
                            objv_array_add(container->services, (objv_object_t *) service);
                            
                            objv_object_release((objv_object_t *) service);
                        }
                    }
                    else{
                        objv_log("Error: Not Found Service %s\n",string->UTF8String);
                    }
                }
                
            }
            
        }
        
    }
}

void CLServiceContainerHandleTask(CLServiceContainer * container,CLContext * ctx,objv_class_t * taskType,CLTask * task){
    if(container && taskType && task){
        
        int i;
        CLService * srv;
        
        for(i=0;i<container->services->length;i++){
            srv = (CLService *) objv_array_objectAt(container->services, i);
            if(CLServiceHasTaskType(srv, taskType) && CLServiceHandleTask(srv, ctx, taskType, task)){
                break;
            }
        }
        
    }
}

