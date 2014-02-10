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

static void CLContextMethodHandleTask(objv_class_t * clazz,CLContext * context,objv_class_t * taskType,CLTask * task){
    
}

OBJV_CLASS_METHOD_IMP_BEGIN(CLContext)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", CLContextMethodDealloc)

OBJV_CLASS_METHOD_IMP(init, "@(*)", CLContextMethodInit)

OBJV_CLASS_METHOD_IMP(handleTask, "v(*,@)", CLContextMethodHandleTask)

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

void CLContextSendTask(CLContext * context, objv_class_t * taskType, CLTask * task){
    if(context && taskType && task){
        
        if(context->dispatch == objv_dispatch_get_current()){
            
            if(task->target == NULL){
                task->target = (objv_string_t *) objv_object_retain((objv_object_t *) context->domain);
            }
            
            task->identifier = ++ context->identifier;
            task->source = (CLContext *) objv_object_retain( (objv_object_t *) context);
            
            if( objv_string_hasPrefix(task->target->UTF8String, context->domain->UTF8String) ){
                {
                    const char * lp;
                    int i;
                    CLContext * ctx;
                    
                    if((lp = objv_string_hasSuffix(task->target->UTF8String, ".*"))){
                        
                        if(objv_string_hasPrefixTo(context->domain->UTF8String,task->target->UTF8String, lp)){
                            CLContextHandleTask(context,taskType,task);
                        }
                        
                        for(i=0;i<context->childs->length;i++){
                            ctx = (CLContext *) objv_array_objectAt(context->childs, i);
                            if(objv_string_hasPrefixTo(ctx->domain->UTF8String,task->target->UTF8String, lp)){
                                CLContextHandleTask(ctx,taskType,task);
                            }
                        }
                        
                    }
                    else{
                        
                        if(strcmp(task->target->UTF8String, context->domain->UTF8String) == 0){
                            CLContextHandleTask(context,taskType,task);
                        }
                        else if(context->childs){
                        
                            for(i=0;i<context->childs->length;i++){
                                ctx = (CLContext *) objv_array_objectAt(context->childs, i);
                                if(strcmp(task->target->UTF8String, ctx->domain->UTF8String) == 0){
                                    CLContextHandleTask(ctx,taskType,task);
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

void CLContextSetConfig(CLContext * context,objv_object_t * config){
    if(context && context->config != config){
        
        objv_object_retain(config);
        objv_object_release(context->config);
        
        context->config = config;
        
        objv_object_release((objv_object_t *) context->domain);
        
        if(config){
            context->domain = (objv_string_t *) objv_object_objectForKey(config->isa, config, (objv_object_t *) objv_string_new(context->base.zone, "domain"));
            objv_object_retain((objv_object_t *) context->domain);
        }
        else{
            context->domain = NULL;
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
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
}

static void CLServiceMethodHandleTask(objv_class_t * clazz,CLService * service,objv_class_t * taskType,CLTask * task){
    
}

OBJV_CLASS_METHOD_IMP_BEGIN(CLService)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", CLServiceMethodDealloc)

OBJV_CLASS_METHOD_IMP(handleTask, "v(*,@)", CLServiceMethodHandleTask)

OBJV_CLASS_METHOD_IMP_END(CLService)

OBJV_CLASS_IMP_M(CLService, OBJV_CLASS(Object), CLService)
