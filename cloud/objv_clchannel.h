//
//  objv_clchannel.h
//  objv
//
//  Created by zhang hailong on 14-2-10.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_clchannel_h
#define objv_objv_clchannel_h


#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_channel.h"
#include "objv_cloud.h"
#include "objv_array.h"  
#include "objv_string.h"
    
    
    typedef struct _CLChannel{
        objv_object_t base;
        objv_boolean_t READONLY connected;
        objv_channel_t * READONLY oChannel;
        objv_string_t * READONLY domain;
        
        objv_timeinval_t READONLY tickTimeinval;
        
        objv_timeinval_t READONLY idleTimeinval;
        
        objv_timeinval_t heartbeatTimeinval; // 心跳时间
        
    } CLChannel;
    
    OBJV_KEY_DEC(CLChannel)
    
    OBJV_CLASS_DEC(CLChannel)
    
    OBJV_KEY_DEC(connect)
    
    OBJV_KEY_DEC(readTask)
    
    OBJV_KEY_DEC(postTask)
    
    OBJV_KEY_DEC(tick)
    
    typedef OBJVChannelStatus ( * CLChannelMethodConnect)(objv_class_t * clazz,CLChannel * channel,objv_timeinval_t timeout);
    
    typedef OBJVChannelStatus ( * CLChannelMethodReadTask)(objv_class_t * clazz,CLChannel * channel,CLTask ** task,objv_class_t ** taskType,objv_timeinval_t timeout);
    
    typedef void ( * CLChannelMethodPostTask)(objv_class_t * clazz,CLChannel * channel,CLTask * task,objv_class_t * taskType);
    
    typedef OBJVChannelStatus ( * CLChannelMethodTick)(objv_class_t * clazz,CLChannel * channel,objv_timeinval_t timeout);
    
    typedef void ( * CLChannelMethodSetChannel)(objv_class_t * clazz,CLChannel * channel,objv_channel_t * oChannel);
    
    objv_object_t * CLChannelInit(objv_class_t * clazz,CLChannel * clChannel, objv_channel_t * channel);
    
    OBJVChannelStatus CLChannelConnect(objv_class_t * clazz,CLChannel * channel,objv_timeinval_t timeout);
    
    OBJVChannelStatus CLChannelReadTask(objv_class_t * clazz,CLChannel * channel,CLTask ** task,objv_class_t ** taskType,objv_timeinval_t timeout);
    
    void CLChannelPostTask(objv_class_t * clazz,CLChannel * channel,CLTask * task,objv_class_t * taskType);
    
    OBJVChannelStatus CLChannelTick(objv_class_t * clazz,CLChannel * channel,objv_timeinval_t timeout);
    
    void CLChannelSetChannel(CLChannel * channel,objv_channel_t * oChannel);
    
    void CLChannelSetDomain(CLChannel * channel,objv_string_t * domain);
    
#ifdef __cplusplus
}
#endif



#endif
