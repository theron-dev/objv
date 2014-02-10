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
    
    typedef unsigned long long CLIdentifier;
    
    typedef struct _CLContext {
        objv_object_t base;
        objv_string_t * READONLY domain;
        CLIdentifier READONLY identifier;
        objv_dispatch_t * READONLY dispatch;
        struct _CLContext * READONLY parent;
        objv_array_t * READONLY childs;
        objv_object_t * READONLY config;
    } CLContext;
    
    OBJV_KEY_DEC(CLContext)
    OBJV_CLASS_DEC(CLContext)
    
    
    typedef struct _CLService {
        objv_object_t base;
        objv_object_t * READONLY config;
        CLContext * READONLY context;
    } CLService;
    
    OBJV_KEY_DEC(CLService)
    OBJV_CLASS_DEC(CLService)
    
    typedef struct _CLTask {
        objv_object_t base;
        CLIdentifier READONLY identifier;
        CLIdentifier READONLY replyIdentifier;
        CLContext * READONLY source;
        objv_string_t * READONLY target;
    } CLTask;
    
    OBJV_KEY_DEC(CLTask)
    OBJV_CLASS_DEC(CLTask)
    
    OBJV_KEY_DEC(handleTask)
    
     typedef void ( * CLServicetHandleTaskFun ) (objv_class_t * clazz,CLService * service,objv_class_t * taskType,CLTask * task);
    typedef void ( * CLContextHandleTaskFun ) (objv_class_t * clazz,CLContext * context,objv_class_t * taskType,CLTask * task);
    
    void CLContextHandleTask(CLContext * context, objv_class_t * taskType, CLTask * task);
    
    void CLContextSendTask(CLContext * context, objv_class_t * taskType, CLTask * task);
    
    void CLContextAddChild(CLContext * context, CLContext * child);
    
    void CLContextSetConfig(CLContext * context,objv_object_t * config);
    
#ifdef __cplusplus
}
#endif



#endif
