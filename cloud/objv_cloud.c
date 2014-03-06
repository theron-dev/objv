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
OBJV_KEY_IMP(handleTask)
OBJV_KEY_IMP(sendTask)

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
    objv_object_release((objv_object_t *) ctx->childsTree);
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
        
        CLContextSetDomain(ctx, objv_object_stringValueForKey(config, (objv_object_t *) objv_string_new_nocopy(ctx->base.zone, "domain"), NULL));
    }
    
}



OBJV_CLASS_METHOD_IMP_BEGIN(CLContext)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", CLContextMethodDealloc)

OBJV_CLASS_METHOD_IMP(init, "@(*)", CLContextMethodInit)

OBJV_CLASS_METHOD_IMP(setConfig, "v(@)", CLContextMethodSetConfig)

OBJV_CLASS_METHOD_IMP_END(CLContext)

OBJV_CLASS_IMP_M(CLContext, OBJV_CLASS(Object), CLContext)

typedef enum _CLContextDispatchTaskInvoke{
    CLContextDispatchTaskInvokeSendTask,CLContextDispatchTaskInvokeHandleTask,CLContextDispatchTaskInvokeAddChild,CLContextDispatchTaskInvokeRemoveChild
} CLContextDispatchTaskInvoke;

typedef struct _CLContextDispatchTask{
    objv_dispatch_task_t base;
    CLContext * context;
    objv_class_t * taskType;
    CLTask * task;
    objv_string_t * target;
    CLContext * child;
    CLContextDispatchTaskInvoke invoke;
} CLContextDispatchTask;

OBJV_KEY_DEC(CLContextDispatchTask)


static void CLContextDispatchTaskMethodDealloc(objv_class_t * clazz,objv_object_t * object){
    
    CLContextDispatchTask * dispatchTask = (CLContextDispatchTask *) object;
    
    objv_object_release((objv_object_t *) dispatchTask->task);
    objv_object_release((objv_object_t *) dispatchTask->context);
    objv_object_release((objv_object_t *) dispatchTask->child);
    objv_object_release((objv_object_t *) dispatchTask->target);

    if(clazz->superClass){
        
        objv_object_dealloc(clazz->superClass, object);
        
    }
    
}


static void CLContextDispatchTaskMethodRun (objv_class_t * clazz,objv_object_t * object){
    
    CLContextDispatchTask * dispatchTask = (CLContextDispatchTask *) object;
    
    switch (dispatchTask->invoke) {
        case CLContextDispatchTaskInvokeSendTask:
        
            CLContextSendTask(dispatchTask->context, dispatchTask->taskType, dispatchTask->task,dispatchTask->target);
            
            break;
        case CLContextDispatchTaskInvokeHandleTask:
            
            CLContextHandleTask(dispatchTask->context, dispatchTask->taskType, dispatchTask->task);
            
            break;
        case CLContextDispatchTaskInvokeAddChild:
            
            CLContextAddChild(dispatchTask->context,dispatchTask->child);
            
            break;
        case CLContextDispatchTaskInvokeRemoveChild:
            
            CLContextRemoveChild(dispatchTask->context,dispatchTask->child);
            
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
        objv_class_t * taskType = va_arg(ap, objv_class_t *);
        CLTask * task = va_arg(ap, CLTask *);
        objv_string_t * target = va_arg(ap, objv_string_t *);
        CLContext * child =  va_arg(ap, CLContext *) ;
        CLContextDispatchTaskInvoke invoke =  va_arg(ap, CLContextDispatchTaskInvoke);
        
        dispatchTask->taskType = taskType;
        dispatchTask->task = (CLTask *) objv_object_retain((objv_object_t *) task);
        dispatchTask->target = (objv_string_t *) objv_object_retain((objv_object_t *) target);
        dispatchTask->invoke = invoke;
        dispatchTask->child = (CLContext *) objv_object_retain((objv_object_t *) child);
        dispatchTask->context = (CLContext *) objv_object_retain((objv_object_t *) context);
        
    }
    
    return object;
}

OBJV_KEY_IMP(CLContextDispatchTask)

OBJV_CLASS_METHOD_IMP_BEGIN(CLContextDispatchTask)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", CLContextDispatchTaskMethodDealloc)

OBJV_CLASS_METHOD_IMP(init, "@(*)", CLContextDispatchTaskMethodInit)

OBJV_CLASS_METHOD_IMP(run, "v()", CLContextDispatchTaskMethodRun)

OBJV_CLASS_METHOD_IMP_END(CLContextDispatchTask)

OBJV_CLASS_IMP_M(CLContextDispatchTask, OBJV_CLASS(DispatchTask), CLContextDispatchTask)

static CLContextDispatchTask * CLContextDispatchTaskAlloc(objv_zone_t * zone,CLContext * context
                                                          ,objv_class_t * taskType,CLTask * task,objv_string_t * target,CLContext * child,CLContextDispatchTaskInvoke invoke){
    return (CLContextDispatchTask *) objv_object_alloc(zone, OBJV_CLASS(CLContextDispatchTask),context,taskType,task,target,child,invoke,NULL);
}

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
            
            CLContextDispatchTask * dispatchTask = CLContextDispatchTaskAlloc(context->base.zone,context,taskType,task,NULL,NULL,CLContextDispatchTaskInvokeHandleTask);
            
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

typedef struct _CLContextSendTaskToChildCallbackContext {
    objv_class_t * taskType;
    CLTask * task;
} CLContextSendTaskToChildCallbackContext;

static void CLContextSendTaskToChildCallback(objv_actree_t * actree,objv_object_t * key,objv_object_t * value,void * context){
    CLContextSendTaskToChildCallbackContext * c = (CLContextSendTaskToChildCallbackContext *) context;
    CLContext * ctx = (CLContext *) value;
    _CLContextSendTask(ctx, c->taskType, c->task);
}

void CLContextSendTask(CLContext * context, objv_class_t * taskType, CLTask * task,objv_string_t * target){
    if(context && taskType && task ){
        
        if(context->dispatch == objv_dispatch_get_current()){
            
            if(task->source == NULL){
                task->identifier = ++ context->identifier;
                task->source = (CLContext *) objv_object_retain( (objv_object_t *) context);
            }
            
            if(target == NULL){
                target = context->domain;
            }
            
            const char *p;
            
            if( (p = objv_string_hasPrefix(target->UTF8String, context->domain->UTF8String) ) ){
                {
                    const char * lp;
                    CLContext * ctx;
                    objv_zone_t * zone = context->base.zone;
                    
                    if((lp = objv_string_hasSuffix(target->UTF8String, ".*"))){
                        
                        if(objv_string_hasPrefixTo(context->domain->UTF8String,target->UTF8String, lp)){
                            _CLContextSendTask(context,taskType,task);
                        }
                        
                        if(context->childsTree){
                            {
        
                                objv_array_t * keys = objv_string_split_UTF8String(zone,p, ".");
                                
                                CLContextSendTaskToChildCallbackContext c = {taskType,task};
                                
                                if(keys && keys->length >0 ){
                                    objv_array_replaceAt(keys, (objv_object_t *) context->domain, 0);
                                }
                                
                                objv_array_removeLast(keys);
                                
                                objv_actree_echo(context->childsTree, CLContextSendTaskToChildCallback, & c);
                                
                            }
                        }
                        
                    }
                    else{
                        
                        if(strcmp(target->UTF8String, context->domain->UTF8String) == 0){
                            _CLContextSendTask(context,taskType,task);
                        }
                        else if(context->childsTree){
                        
                            {
                                objv_array_t * keys = objv_string_split_UTF8String(zone,p, ".");
                                
                                if(keys && keys->length >0 ){
                                    objv_array_replaceAt(keys, (objv_object_t *) context->domain, 0);
                                }
                                
                                ctx = (CLContext *) objv_actree_value(context->childsTree, keys);
                                
                                if(ctx){
                                    _CLContextSendTask(ctx,taskType,task);
                                }
                            }
                            
                        }
                    }
                }

            }
            else if( context->parent ){
                _CLContextSendTask( context->parent , taskType, task);
            }
            
        }
        else{
            CLContextDispatchTask * dispatchTask = (CLContextDispatchTask *) CLContextDispatchTaskAlloc(context->base.zone, context,taskType,task,target,NULL,CLContextDispatchTaskInvokeSendTask);
            
            objv_dispatch_addTask(context->dispatch, (objv_dispatch_task_t *) dispatchTask);
            
            objv_object_release((objv_object_t *) dispatchTask);
        }
        
    }
}

void CLContextAddChild(CLContext * context, CLContext * child){
    const char * p;
    
    if(context && context->domain && child && child->domain && child->parent == NULL
       && (p = objv_string_hasPrefix(child->domain->UTF8String, context->domain->UTF8String))){
        
        if(context->dispatch == objv_dispatch_get_current()){
            
            objv_zone_t * zone = context->base.zone;
            objv_boolean_t setdomain = objv_false;
            
            child->parent = context;
            
            if(context->childs == NULL){
                context->childs = objv_array_alloc(zone, 4);
            }
            
            if(context->childsTree == NULL){
                context->childsTree = objv_actree_alloc(zone, (objv_object_t *)context->domain, NULL);
            }
            
            objv_array_add(context->childs, (objv_object_t *) child);

            objv_array_t * keys = objv_string_split_UTF8String(zone, p, ".");
            
            if(keys && keys->length > 1){
                
                objv_string_t * key = (objv_string_t *) objv_array_objectAt(keys, 0);
                
                if(key->length == 0){
                    
                    objv_array_replaceAt(keys, (objv_object_t *) context->domain, 0);
                
                }
                
                key = (objv_string_t *) objv_array_objectAt(keys, keys->length - 1);
                
                if(strcmp(key->UTF8String, "*") == 0){
                    
                    {
                        char uuid[128];
                        
                        snprintf(uuid, sizeof(uuid), "%lx", rand() + time(NULL));
                        
                        key = objv_string_new(zone, uuid);
                        
                        objv_array_replaceAt(keys, (objv_object_t *) key, keys->length - 1);
                        
                        while(objv_actree_value(context->childsTree, keys)){
                            
                            snprintf(uuid, sizeof(uuid),"%lx", rand() + time(NULL));
                            
                            key = objv_string_new(zone, uuid);
                            
                            objv_array_replaceAt(keys, (objv_object_t *) key, keys->length - 1);
                            
                        }
                        
                        CLContextSetDomain(child, objv_array_joinString(keys, "."));
                        
                        setdomain = objv_true;
                        
                    }
                    
                }
                
                objv_actree_setValue(context->childsTree, keys, (objv_object_t *) child);
                
                if(setdomain){
                    {
                        CLDomainSetTask * task = CLDomainSetTaskAlloc(zone, child->domain);
                        
                        CLContextSendTask(context, OBJV_CLASS(CLDomainSetTask), (CLTask *) task, child->domain);
                        
                        objv_object_release((objv_object_t *) task);
                    }
                }
            }
            
        }
        else{
            
            CLContextDispatchTask * dispatchTask = (CLContextDispatchTask *) CLContextDispatchTaskAlloc(context->base.zone,context,NULL,NULL,NULL,child,CLContextDispatchTaskInvokeAddChild);
            
            objv_dispatch_addTask(context->dispatch, (objv_dispatch_task_t *) dispatchTask);
            
            objv_object_release((objv_object_t *) dispatchTask);
        }
    }
}

void CLContextRemoveChild(CLContext * context,CLContext * child){
   
    const char * p;
    
    if(context && child && child->parent == context
       && (p = objv_string_hasPrefix(child->domain->UTF8String, context->domain->UTF8String))){
        
        if(context->dispatch == objv_dispatch_get_current()){
            
            objv_zone_t * zone = context->base.zone;

            objv_array_t * keys = objv_string_split_UTF8String(zone, p, ".");
            
            if(keys && keys->length > 1){
                
                objv_string_t * key = (objv_string_t *) objv_array_objectAt(keys, 0);
                
                if(key->length == 0){
                    
                    objv_array_replaceAt(keys, (objv_object_t *) context->domain, 0);
                    
                    objv_actree_remove(context->childsTree, keys);
                    
                }
                
            }
            
            objv_array_remove(context->childs, (objv_object_t *) child);
            
        }
        else{
            
            CLContextDispatchTask * dispatchTask = (CLContextDispatchTask *) CLContextDispatchTaskAlloc(context->base.zone,context,NULL,NULL,NULL,child,CLContextDispatchTaskInvokeRemoveChild);
            
            objv_dispatch_addTask(context->dispatch, (objv_dispatch_task_t *) dispatchTask);
            
            objv_object_release((objv_object_t *) dispatchTask);
        }
    }
}

void CLContextSetDomain(CLContext * context, objv_string_t * domain){
    if(context && context->domain != domain){
        objv_object_retain((objv_object_t *) domain);
        objv_object_release((objv_object_t *) context->domain);
        context->domain = domain;
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
    
    objv_object_release((objv_object_t *) task->source);
    objv_object_release((objv_object_t *) task->contentType);
    
    objv_mbuf_destroy(& task->content);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
}


OBJV_CLASS_METHOD_IMP_BEGIN(CLTask)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", CLTaskMethodDealloc)

OBJV_CLASS_METHOD_IMP_END(CLTask)

OBJV_CLASS_IMP_M(CLTask, OBJV_CLASS(Object), CLTask)

OBJV_KEY_IMP(CLDomainSetTask)


static void CLDomainSetTaskMethodDealloc(objv_class_t * clazz,objv_object_t * object){
    
    CLDomainSetTask * task = (CLDomainSetTask *) object;
    
    objv_object_release((objv_object_t *) task->domain);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
}

static objv_object_t * CLDomainSetTaskMethodDomain(objv_class_t * clazz,objv_object_t * object){
    CLDomainSetTask * task = (CLDomainSetTask *) object;
    return (objv_object_t *) task->domain;
}

static void CLDomainSetTaskMethodSetDomain(objv_class_t * clazz,objv_object_t * object,objv_object_t * value){
    CLDomainSetTask * task = (CLDomainSetTask *) object;
    objv_string_t * s = objv_object_stringValue(value, NULL);
    if(s != task->domain){
        objv_object_retain((objv_object_t *) s);
        objv_object_release((objv_object_t *) task->domain);
        task->domain = s;
    }
}

OBJV_CLASS_METHOD_IMP_BEGIN(CLDomainSetTask)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", CLDomainSetTaskMethodDealloc)

OBJV_CLASS_METHOD_IMP_END(CLDomainSetTask)

OBJV_CLASS_PROPERTY_IMP_BEGIN(CLDomainSetTask)

OBJV_CLASS_PROPERTY_IMP(domain, object, CLDomainSetTaskMethodDomain, CLDomainSetTaskMethodSetDomain, objv_true)

OBJV_CLASS_PROPERTY_IMP_END(CLDomainSetTask)

OBJV_KEY_IMP(domain)

OBJV_CLASS_IMP_P_M(CLDomainSetTask, OBJV_CLASS(CLTask), CLDomainSetTask)

CLDomainSetTask * CLDomainSetTaskAlloc(objv_zone_t * zone,objv_string_t * domain){
    CLDomainSetTask * task = (CLDomainSetTask *) objv_object_alloc(zone, OBJV_CLASS(CLDomainSetTask),NULL);
    task->domain = (objv_string_t *) objv_object_retain((objv_object_t *) domain);
    return task;
}

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
        object = objv_object_initv(clazz->superClass, object, ap);
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

