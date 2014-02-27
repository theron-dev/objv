//
//  objv_clchannel_http.c
//  objv
//
//  Created by zhang hailong on 14-2-27.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv.h"
#include "objv_clchannel_http.h"

OBJV_KEY_IMP(CLHttpChannel)

static objv_object_t * CLHttpChannelMethodInit(objv_class_t * clazz,objv_object_t * object,va_list ap){
    
    if(clazz->superClass){
        object = objv_object_initv(clazz->superClass, object, ap);
    }
    
    if(object){
        
        CLHttpChannel * channel = (CLHttpChannel *) object;
        
        OBJVHTTPRequestReset( &channel->httpRequest );
        
    }
    
    return object;
}

static void CLHttpChannelMethodDealloc(objv_class_t * clazz,objv_object_t * object){
    
    CLHttpChannel * channel = (CLHttpChannel *) object;
 
    OBJVHTTPRequestReset( &channel->httpRequest );
    
    objv_mbuf_destroy( & channel->read.mbuf);
    objv_mbuf_destroy( & channel->write.mbuf);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
    
}

static OBJVChannelStatus CLHttpChannelMethodConnect(objv_class_t * clazz,CLChannel * channel,objv_timeinval_t timeout){
    
    if(channel->connected){
        return OBJVChannelStatusOK;
    }
   
    if(channel->oChannel){
        return objv_channel_connect(channel->oChannel->base.isa, channel->oChannel, timeout);
    }
    
    return OBJVChannelStatusError;
}

static OBJVChannelStatus CLHttpChannelMethodReadTask(objv_class_t * clazz,CLChannel * channel,CLTask ** task,objv_class_t ** taskType,objv_timeinval_t timeout){
    
    CLHttpChannel * httpChannel = (CLHttpChannel *) channel;
    
    if(httpChannel->contentType == CLHttpChannelContentTypeChunked){
    
        if(httpChannel->read.state == 0){
            
            
            
        }
        else if (httpChannel->read.state == 1){
        
        }
        else {
            
        }
        
    }
    
    return OBJVChannelStatusError;
}

static void CLHttpChannelMethodPostTask(objv_class_t * clazz,CLChannel * channel,CLTask * task,objv_class_t * taskType){
    
}

static OBJVChannelStatus CLHttpChannelMethodTick(objv_class_t * clazz,CLChannel * channel,objv_timeinval_t timeout){
    
    return OBJVChannelStatusError;
}


OBJV_CLASS_METHOD_IMP_BEGIN(CLHttpChannel)

OBJV_CLASS_METHOD_IMP(init, "@(*)", CLHttpChannelMethodInit)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", CLHttpChannelMethodDealloc)

OBJV_CLASS_METHOD_IMP(connect, "i(d)", CLHttpChannelMethodConnect)

OBJV_CLASS_METHOD_IMP(readTask, "i(*,*,d)", CLHttpChannelMethodReadTask)

OBJV_CLASS_METHOD_IMP(postTask, "v(*,*)", CLHttpChannelMethodPostTask)

OBJV_CLASS_METHOD_IMP(tick, "i(d)", CLHttpChannelMethodTick)

OBJV_CLASS_METHOD_IMP_END(CLChannel)

OBJV_CLASS_IMP_M(CLHttpChannel, OBJV_CLASS(CLChannel), CLHttpChannel)


