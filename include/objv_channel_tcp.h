//
//  objv_channel_tcp.h
//  objv
//
//  Created by zhang hailong on 14-2-10.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_channel_tcp_h
#define objv_objv_channel_tcp_h


#ifdef __cplusplus
extern "C" {
#endif
    
    #include "objv_channel.h"
    #include "objv_string.h"
    
    typedef objv_os_socket_t objv_channel_handler_t;

    typedef struct _objv_channel_tcp_t {
        objv_channel_t base;
        objv_channel_handler_t READONLY handler;
        objv_string_t * READONLY host;
        int READONLY port;
    } objv_channel_tcp_t;
    
    OBJV_KEY_DEC(TCPChannel)
    
    OBJV_CLASS_DEC(TCPChannel)
    
    objv_channel_tcp_t * objv_channel_tcp_allocWithHandle(objv_zone_t * zone,objv_channel_handler_t handler);
    
    objv_channel_tcp_t * objv_channel_tcp_allocWithHost(objv_zone_t * zone,objv_string_t * host,int port);
    
    int objv_channel_tcp_getPort(objv_channel_tcp_t * tcp);
    
#ifdef __cplusplus
}
#endif


#endif
