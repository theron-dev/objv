//
//  objv_clchannel_http.h
//  objv
//
//  Created by zhang hailong on 14-2-27.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_clchannel_http_h
#define objv_objv_clchannel_http_h


#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_clchannel.h"
#include "objv_http.h"
    
    typedef enum _CLHttpChannelContentType {
        CLHttpChannelContentTypeNone = 0,
        CLHttpChannelContentTypeChunked = 1<<0,
    } CLHttpChannelContentType;
    
    typedef struct _CLHttpChannelPostTask {
        CLTask * task;
        objv_class_t * taskType;
        objv_string_t * target;
        struct _CLHttpChannelPostTask * next;
    } CLHttpChannelPostTask;
    
    typedef struct _CLHttpChannel{
        CLChannel base;
        OBJVHttpRequest httpRequest;
        CLHttpChannelPostTask * beginTask;
        CLHttpChannelPostTask * endTask;
        objv_mutex_t tasks_mutex;
        
        struct {
            objv_mbuf_t mbuf;
            objv_mbuf_t data;
            size_t dataLength;
            size_t off;
            int state;
        } read;
        
        struct {
            objv_mbuf_t mbuf;
            size_t off;
            int state;
        } write;
        
        CLHttpChannelContentType contentType;
    } CLHttpChannel;
    
    OBJV_KEY_DEC(CLHttpChannel)
    
    OBJV_CLASS_DEC(CLHttpChannel)
    
    
    OBJVChannelStatus CLHttpChannelUnpackageTask(objv_zone_t * zone, CLTask ** task,objv_class_t ** taskType,objv_string_t ** target,objv_mbuf_t * data,CLHttpChannelContentType contentType);
    
    OBJVChannelStatus CLHttpChannelPackageTask(objv_zone_t * zone, CLTask * task,objv_class_t * taskType,objv_string_t * target,objv_mbuf_t * mbuf,CLHttpChannelContentType contentType);

        
#ifdef __cplusplus
}
#endif



#endif
