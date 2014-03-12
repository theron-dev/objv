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
#include "objv_url.h"
    
    typedef enum _CLChannelMode {
        CLChannelModeNone = 0
        ,CLChannelModeRead = 1 << 0
        ,CLChannelModeWrite = 1 << 1
    } CLChannelMode;
    
    typedef struct _CLChannel{
        objv_object_t base;
        objv_channel_t * READONLY oChannel;
        objv_string_t * READONLY domain;
        objv_url_t * READONLY url;
        
        objv_timeinval_t READONLY tickTimeinval;
        
        objv_timeinval_t READONLY idleTimeinval;
        
        objv_timeinval_t heartbeatTimeinval; // 心跳时间
        
        CLChannelMode mode;
        
    } CLChannel;
    
    OBJV_KEY_DEC(CLChannel)
    
    OBJV_CLASS_DEC(CLChannel)
    
    OBJV_KEY_DEC(connect)
    
    OBJV_KEY_DEC(disconnect)
    
    OBJV_KEY_DEC(readTask)
    
    OBJV_KEY_DEC(postTask)
    
    OBJV_KEY_DEC(tick)
    
    typedef OBJVChannelStatus ( * CLChannelMethodConnect)(objv_class_t * clazz,CLChannel * channel,objv_timeinval_t timeout);
    
    typedef OBJVChannelStatus ( * CLChannelMethodDisconnect)(objv_class_t * clazz,CLChannel * channel);
    
    typedef OBJVChannelStatus ( * CLChannelMethodReadTask)(objv_class_t * clazz,CLChannel * channel
                                                           ,CLTask ** task,objv_class_t ** taskType,objv_string_t ** target,objv_timeinval_t timeout);
    
    typedef void ( * CLChannelMethodPostTask)(objv_class_t * clazz,CLChannel * channel,CLTask * task,objv_class_t * taskType,objv_string_t * target);
    
    typedef OBJVChannelStatus ( * CLChannelMethodTick)(objv_class_t * clazz,CLChannel * channel,objv_timeinval_t timeout);
    
    typedef void ( * CLChannelMethodSetChannel)(objv_class_t * clazz,CLChannel * channel,objv_channel_t * oChannel);
    
    objv_object_t * CLChannelInit(objv_class_t * clazz,CLChannel * clChannel, objv_channel_t * channel);
    
    OBJVChannelStatus CLChannelConnect(objv_class_t * clazz,CLChannel * channel,objv_timeinval_t timeout);
    
    OBJVChannelStatus CLChannelDisconnect(objv_class_t * clazz, CLChannel * channel);
    
    OBJVChannelStatus CLChannelReadTask(objv_class_t * clazz,CLChannel * channel,CLTask ** task,objv_class_t ** taskType,objv_string_t ** target,objv_timeinval_t timeout);
    
    void CLChannelPostTask(objv_class_t * clazz,CLChannel * channel,CLTask * task,objv_class_t * taskType,objv_string_t * target);
    
    OBJVChannelStatus CLChannelTick(objv_class_t * clazz,CLChannel * channel,objv_timeinval_t timeout);
    
    void CLChannelSetChannel(CLChannel * channel,objv_channel_t * oChannel);
    
    void CLChannelSetDomain(CLChannel * channel,objv_string_t * domain);
    
    void CLChannelSetURL(CLChannel * channel,objv_url_t * url);
    
#ifdef __cplusplus
}
#endif



#endif
