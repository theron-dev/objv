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
    
    typedef enum _CLChannelContextType {
        CLChannelContextTypeComplicating,   // 多通道并发
        CLChannelContextTypePoll,           // 轮询
    } CLChannelContextType;
    
    typedef struct _CLChannelContext {
        CLServiceContext base;
        objv_array_t * READONLY channels;
        objv_mutex_t READONLY channels_mutex;
        objv_boolean_t allowRemovedFromParent;
        objv_timeinval_t keepAlive;         // >0 空闲时间超过 keepAlive 断开连接， <0 空闲时间超过 - keepAlive 发起 CLKeepAliveTask 保持连接
        CLChannelContextType type;
        unsigned int READONLY index;
    } CLChannelContext;
    
    OBJV_KEY_DEC(CLChannelContext);
    
    OBJV_KEY_DEC(willRemoveChannel);
    
    OBJV_KEY_DEC(didRemoveChannel);
    
    OBJV_CLASS_DEC(CLChannelContext)
    
    void CLChannelContextAddChannel(CLChannelContext * ctx,CLChannel * channel);
    
    void CLChannelContextRemoveChannel(CLChannelContext * ctx,CLChannel * channel);
    
    typedef void ( * CLChannelContextMethodWillRemoveChannel )(objv_class_t * clazz,CLChannelContext * ctx,CLChannel * channel);
    
    void CLChannelContextWillRemoveChannel(objv_class_t * clazz,CLChannelContext * ctx,CLChannel * channel);
    
    typedef void ( * CLChannelContextMethodDidRemoveChannel )(objv_class_t * clazz,CLChannelContext * ctx);
    
    void CLChannelContextDidRemoveChannel(objv_class_t * clazz,CLChannelContext * ctx);
    
#ifdef __cplusplus
}
#endif


#endif
