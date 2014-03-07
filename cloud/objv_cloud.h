//
//  objv_cloud.h
//  objv
//
//  Created by zhang hailong on 14-2-9.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_cloud_h
#define objv_objv_cloud_h



#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_array.h"
#include "objv_dispatch.h"
#include "objv_string.h"
#include "objv_hash_map.h"
#include "objv_dictionary.h"
#include "objv_actree.h"
    
    typedef unsigned long long CLIdentifier;

    
    typedef struct _CLContext {
        objv_object_t base;
        objv_string_t * READONLY domain;
        CLIdentifier READONLY identifier;
        objv_dispatch_queue_t * READONLY queue;
        struct _CLContext * READONLY parent;
        objv_array_t * READONLY childs;
        objv_object_t * READONLY config;
        objv_actree_t * READONLY childsTree;
        objv_mutex_t READONLY mutex;
    } CLContext;
    
    OBJV_KEY_DEC(CLContext)
    OBJV_CLASS_DEC(CLContext)
    
    typedef struct _CLService {
        objv_object_t base;
        objv_object_t * READONLY config;
        objv_hash_map_t * READONLY taskTypes;
        objv_boolean_t inherit;
    } CLService;
    
    OBJV_KEY_DEC(CLService)
    OBJV_CLASS_DEC(CLService)
    
    typedef struct _CLTask {
        objv_object_t base;
        CLIdentifier READONLY identifier;
        CLIdentifier READONLY replyIdentifier;
        objv_string_t * READONLY contentType;
        objv_mbuf_t READONLY content;
        CLContext * READONLY source;
    } CLTask;
    
    OBJV_KEY_DEC(CLTask)
    OBJV_CLASS_DEC(CLTask)
    
    typedef struct _CLDomainSetTask {
        CLTask base;
        objv_string_t * READONLY domain;
    } CLDomainSetTask;
    
    OBJV_KEY_DEC(CLDomainSetTask)
    OBJV_CLASS_DEC(CLDomainSetTask)
    
    OBJV_KEY_DEC(domain)
    
    CLDomainSetTask * CLDomainSetTaskAlloc(objv_zone_t * zone,objv_string_t * domain);
    
    typedef struct _CLServiceContainer {
        objv_object_t base;
        objv_array_t * READONLY services;
        objv_object_t * READONLY config;
    } CLServiceContainer;
    
    OBJV_KEY_DEC(CLServiceContainer)
    OBJV_CLASS_DEC(CLServiceContainer)
    
    void CLServiceContainerAddService(CLServiceContainer * container,CLService * service);
    
    void CLServiceContainerSetConfig(CLServiceContainer * container,objv_object_t * config);
    
    void CLServiceContainerHandleTask(CLServiceContainer * container,CLContext * ctx, objv_class_t * taskType,CLTask * task);
    
    typedef objv_boolean_t ( * CLServicetHandleTaskFun ) (objv_class_t * clazz,CLService * service,CLContext * ctx,objv_class_t * taskType,CLTask * task);
    
    void CLServiceSetConfig(CLService * service, objv_object_t * config);
    
    void CLServiceAddTaskType(CLService * service,objv_class_t * taskType);
    
    objv_boolean_t CLServiceHasTaskType(CLService * service,objv_class_t * taskType);
    
    objv_boolean_t CLServiceHandleTask(CLService * service,CLContext * ctx,objv_class_t * taskType,CLTask * task);
    
    OBJV_KEY_DEC(handleTask)
    OBJV_KEY_DEC(sendTask)
    
    
    typedef void ( * CLContextHandleTaskFun ) (objv_class_t * clazz,CLContext * context,objv_class_t * taskType,CLTask * task);
    typedef void ( * CLContextSendTaskFun ) (objv_class_t * clazz,CLContext * context,objv_class_t * taskType,CLTask * task);
    
    void CLContextHandleTask(CLContext * context, objv_class_t * taskType, CLTask * task);

    void CLContextSendTask(CLContext * context, objv_class_t * taskType, CLTask * task, objv_string_t * target);
    
    void CLContextAddChild(CLContext * context, CLContext * child);
    
    void CLContextRemoveChild(CLContext * context,CLContext * child);
    
    void CLContextSetDomain(CLContext * context, objv_string_t * domain);

    void CLContextSetQueue(CLContext * ctx,objv_dispatch_queue_t * queue);
    
    OBJV_KEY_DEC(setConfig)
    
    typedef void ( * CLContextSetConfigFun ) (objv_class_t * clazz,CLContext * context,objv_object_t * config);
    
    void CLContextSetConfig(objv_class_t * clazz, CLContext * context,objv_object_t * config);
    
#ifdef __cplusplus
}
#endif



#endif
