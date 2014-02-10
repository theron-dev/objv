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
    
    typedef struct _CLChannelConnectBlock {
        char protocol[8];
        char version[8];
        char encoding[16];
        char contentType[16];
        unsigned int verify;
    } CLChannelConnectBlock;
    
    objv_boolean_t CLChannelConnectBlockValidate(CLChannelConnectBlock * block);
    
    void CLChannelConnectBlockSignature(CLChannelConnectBlock * block);
    
    typedef struct _CLChannelRequestBlock {
        char length[8];
    } CLChannelRequestBlock;
    
    void CLChannelRequestBlockSetLength(CLChannelRequestBlock * block,size_t length);
    
    size_t CLChannelRequestBlockLength(CLChannelRequestBlock * block);
    
    typedef struct _CLChannel{
        objv_object_t base;
        objv_boolean_t READONLY connected;
        objv_channel_t * READONLY channel;
        CLChannelConnectBlock READONLY block;
        CLTask * task;
        objv_class_t * taskType;
        
        struct {
            objv_mbuf_t data;
            int state;
            size_t dataLength;
        } read;
        
        struct {
            objv_mbuf_t data;
            size_t writeLength;
            int state;
        } write;
        
        objv_timeinval_t READONLY tickTimeinval;
        
        objv_timeinval_t READONLY idleTimeinval;
        
        objv_timeinval_t heartbeatTimeinval; // 心跳时间
        
    } CLChannel;
    
    OBJV_KEY_DEC(CLChannel)
    
    CLChannel * CLChannelAlloc(objv_zone_t * zone,objv_channel_t * channel);
    
    OBJVChannelStatus CLChannelConnect(CLChannel * channel,objv_timeinval_t timeout);
    
    OBJVChannelStatus CLChannelReadTask(CLChannel * channel,CLTask ** task,objv_class_t ** taskType,objv_timeinval_t timeout);
    
    void CLChannelSetTask(CLChannel * channel,CLTask * task,objv_class_t * taskType);
    
    OBJVChannelStatus CLChannelTick(CLChannel * channel,objv_timeinval_t timeout);
    
#ifdef __cplusplus
}
#endif



#endif
