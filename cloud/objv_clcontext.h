//
//  objv_clcontext.h
//  objv
//
//  Created by zhang hailong on 14-2-10.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_clcontext_h
#define objv_objv_clcontext_h


#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_clchannel.h"
#include "objv_cloud.h"
#include "objv_array.h"
#include "objv_dispatch.h"
#include "objv_hash_map.h"
    
    typedef struct _CLServiceContext {
        CLContext base;
        CLServiceContainer * READONLY container;
    } CLServiceContext;
    
    OBJV_KEY_DEC(CLServiceContext)
    
    OBJV_CLASS_DEC(CLServiceContext)
    
    void CLServiceContextSetContainer(CLServiceContext * ctx,CLServiceContainer * container);
                                      
    typedef struct _CLChannelContextSendTask {
        CLTask * task;
        objv_class_t * taskType;
        struct _CLChannelContextSendTask * next;
    } CLChannelContextSendTask;
    
    typedef struct _CLChannelContext {
        CLServiceContext base;
        objv_dispatch_queue_t * READONLY queue;
        CLChannelContextSendTask * READONLY beginTask;
        CLChannelContextSendTask * READONLY endTask;
        objv_mutex_t mutex;
    } CLChannelContext;
    
    OBJV_KEY_DEC(CLChannelContext);
    
    OBJV_CLASS_DEC(CLChannelContext)
    
    void CLChannelContextSetConfig(CLChannelContext * ctx,objv_object_t * config);
    
    void CLChannelContextAddChannel(CLChannelContext * ctx,CLChannel * channel);
    
    void CLChannelContextDequeueSendTask(CLChannelContext * ctx, CLTask ** task,objv_class_t ** taskType);
    
    
#ifdef __cplusplus
}
#endif


#endif
