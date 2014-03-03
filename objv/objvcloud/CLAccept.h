//
//  CLAccept.h
//  objv
//
//  Created by zhang hailong on 14-2-28.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_CLAccept_h
#define objv_CLAccept_h


#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_cloud.h"
#include "objv_dispatch.h"
#include "objv_channel.h"
#include "objv_http.h"
#include "objv_channel_tcp.h"
#include "objv_clchannel_http.h"
    
#define CL_ENV_MAX_THREAD_COUNT_KEY     "cl-max-thread-count"
#define CL_ENV_CONFIG_KEY               "cl-config"
    
    typedef struct _CLAcceptHandler {
        objv_os_socket_t READONLY sock;
        objv_mutex_t READONLY mutex;
    } CLAcceptHandler;
    
    typedef struct _CLAccept {
        objv_dispatch_task_t base;
        CLContext * ctx;
        int READONLY port;
        CLAcceptHandler READONLY handler;
        objv_boolean_t READONLY copyed;
        objv_dispatch_queue_t * READONLY connectQueue;
    } CLAccept;
    
    OBJV_KEY_DEC(CLAccept)
    
    OBJV_CLASS_DEC(CLAccept)
    
    typedef struct _CLAcceptConnect {
        objv_dispatch_task_t base;
        CLContext * READONLY ctx;
        objv_channel_tcp_t * READONLY channel;
        struct sockaddr_in from;
        CLHttpChannel * httpChannel;
    } CLAcceptConnect;
    
    OBJV_KEY_DEC(CLAcceptConnect)
    
    OBJV_CLASS_DEC(CLAcceptConnect)
    
    CLAccept * CLAcceptAlloc(objv_zone_t * zone,int port);
    
    CLAccept * CLAcceptAllocWithHandler(objv_zone_t * zone,CLAcceptHandler * handler);
    
    OBJVChannelStatus CLAcceptGetConnect(CLAccept * accept,objv_timeinval_t timeout,CLAcceptConnect ** connenct );
    
    
#ifdef __cplusplus
}
#endif


#endif
