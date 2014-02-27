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
    
    typedef struct _CLChannelContext {
        CLServiceContext base;
        objv_dispatch_queue_t * READONLY queue;
        objv_array_t * READONLY channels;
        objv_mutex_t READONLY channels_mutex;
        unsigned int READONLY channel_index;
    } CLChannelContext;
    
    OBJV_KEY_DEC(CLChannelContext);
    
    OBJV_CLASS_DEC(CLChannelContext)
    
    void CLChannelContextSetConfig(CLChannelContext * ctx,objv_object_t * config);
    
    void CLChannelContextAddChannel(CLChannelContext * ctx,CLChannel * channel);
    
    void CLChannelContextRemoveChannel(CLChannelContext * ctx,CLChannel * channel);
    
#ifdef __cplusplus
}
#endif


#endif
