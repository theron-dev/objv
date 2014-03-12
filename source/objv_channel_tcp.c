//
//  objv_channel_tcp.c
//  objv
//
//  Created by zhang hailong on 14-2-10.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv.h"
#include "objv_channel_tcp.h"
#include "objv_log.h"

OBJV_KEY_IMP(TCPChannel)

static OBJVChannelStatus objv_channel_tcp_canWrite(objv_class_t * clazz,objv_channel_tcp_t * channel,objv_timeinval_t timeout);

static OBJVChannelStatus objv_channel_tcp_disconnect(objv_class_t * clazz,objv_channel_tcp_t * channel);

static OBJVChannelStatus objv_channel_tcp_connect(objv_class_t * clazz, objv_channel_tcp_t * channel,objv_timeinval_t timeout){

    OBJVChannelStatus state = OBJVChannelStatusNone;
    
    if(channel->handler){
        state = objv_channel_tcp_canWrite(clazz,channel,timeout);
        if(state == OBJVChannelStatusError){
            objv_channel_tcp_disconnect(clazz,channel);
            state = OBJVChannelStatusNone;
        }
        else{
            channel->base.connected = objv_true;
            state = OBJVChannelStatusOK;
        }
    }
    
    if(! channel->handler && channel->host){
        {
            objv_channel_handler_t sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            struct sockaddr_in addr ;
            int res ;
#ifdef O_NONBLOCK
            int fl;
#endif
            if(sock == -1){
                state = OBJVChannelStatusError;
            }
            else{
                memset(&addr, 0, sizeof(struct sockaddr_in));
                addr.sin_family = AF_INET;
                addr.sin_port = 0;
                addr.sin_addr.s_addr = INADDR_ANY;
                
                res = bind(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
                
                if(res != 0){
                    objv_channel_setExceptionFormat((objv_channel_t *) channel,0, "%s:%d socket bind error",channel->host->UTF8String,channel->port);
                    closesocket(sock);
                    state = OBJVChannelStatusError;
                }
                else{
                    
#ifdef O_NONBLOCK
                    fl =  fcntl(sock, F_GETFL) ;
                    fcntl(sock, F_SETFL, fl | O_NONBLOCK);
#endif
                    addr.sin_port = htons(channel->port);
                    addr.sin_addr = objv_os_resolv(channel->host->UTF8String);
                    
                    res = connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
                    
                    if(res == -1){
                        if(errno == EINPROGRESS || errno ==EINTR){
                            channel->handler = sock;
                            state = OBJVChannelStatusNone;
                        }
                        else{
                            closesocket(sock);
                            objv_channel_setExceptionFormat((objv_channel_t *) channel,0, "errno: %d\n",errno);
                            state = OBJVChannelStatusError;
                        }
                    }
                    else{
                        channel->handler = sock;
                        state = res == 0 ? OBJVChannelStatusNone : OBJVChannelStatusOK;
                    }
                }
            }
            
            if(state == OBJVChannelStatusOK){
                channel->base.connected = objv_true;
            }
        }
    }
    
    return state;

}

static OBJVChannelStatus objv_channel_tcp_canRead(objv_class_t * clazz,objv_channel_tcp_t * channel,objv_timeinval_t timeout){
    
    if(channel->handler){
        
        OBJVChannelStatus state = OBJVChannelStatusNone;
        int res;
        fd_set rds;
        struct timeval timeo = {(int)timeout, (int)((timeout - (int)timeout) * 1000000)};
        FD_ZERO(&rds);
        
        FD_SET(channel->handler, &rds);
        
        res = select(channel->handler + 1, &rds, NULL, NULL, &timeo);
        
        if(res == 0){
            
        }
        else if(res == -1){
            if(errno == EINTR){
                state = OBJVChannelStatusNone;
            }
            else{
                state = OBJVChannelStatusError;
            }
        }
        else{
            if(FD_ISSET(channel->handler, &rds)){
                state = OBJVChannelStatusOK;
            }
        }
        
        return state;
    }
    
    return OBJVChannelStatusError;
}

static ssize_t objv_channel_tcp_read(objv_class_t * clazz,objv_channel_tcp_t * channel,void * data,size_t length){
    
    if(channel->handler){
        return objv_os_socket_read(channel->handler,data,length);
    }
    
    return OBJVChannelStatusError;
}

static OBJVChannelStatus objv_channel_tcp_canWrite(objv_class_t * clazz,objv_channel_tcp_t * channel,objv_timeinval_t timeout){
    
    if(channel->handler){
        
        OBJVChannelStatus state = OBJVChannelStatusNone;
        int res;
        fd_set wds;
        struct timeval timeo = {(int)timeout, (int)((timeout - (int)timeout) * 1000000)};
        FD_ZERO(&wds);
        
        FD_SET(channel->handler, &wds);
        
        res = select(channel->handler + 1, NULL, &wds, NULL, &timeo);
        
        if(res == 0){
            
        }
        else if(res == -1){
            if(errno == EINTR){
                state = OBJVChannelStatusNone;
            }
            else{
                state = OBJVChannelStatusError;
            }
        }
        else{
            if(FD_ISSET(channel->handler, &wds)){
                state = OBJVChannelStatusOK;
            }
        }
        
        return state;

    }
    
    return OBJVChannelStatusError;
}

static ssize_t objv_channel_tcp_write(objv_class_t * clazz,objv_channel_tcp_t * channel,void * data,size_t length){
    
    if(channel->handler){
        return objv_os_socket_write(channel->handler,data,length);
    }
    
    return OBJVChannelStatusError;
}

static OBJVChannelStatus objv_channel_tcp_disconnect(objv_class_t * clazz,objv_channel_tcp_t * channel){
    
    if(channel->handler){
        objv_os_socket_close(channel->handler);
        channel->handler = 0;
        channel->base.connected = objv_false;
    }
    
    if(channel->host){
        return OBJVChannelStatusNone;
    }
    
    return OBJVChannelStatusError;
}

static objv_object_t * objv_channel_tcp_init(objv_class_t * clazz,objv_object_t * object,va_list ap){
    
    if(clazz->superClass){
        object = objv_object_initv(clazz->superClass, object, ap);
    }
    
    if(object){
    
        objv_channel_tcp_t * channel = (objv_channel_tcp_t *) object;
        int argc = va_arg(ap, int);
        
        if(argc == 1){
            channel->handler = va_arg(ap, objv_channel_handler_t);
            channel->base.connected = objv_true;
        }
        else if(argc == 2){
            channel->host = (objv_string_t *) objv_object_retain(va_arg(ap, objv_object_t *));
            channel->port = va_arg(ap, int);
        }
    }
    
    return object;
}

static void objv_channel_tcp_dealloc(objv_class_t * clazz,objv_object_t * object){
    
    objv_channel_tcp_t * channel = (objv_channel_tcp_t *) object;
    
    if(channel->handler){
        objv_channel_tcp_disconnect(clazz,channel);
    }
    
    objv_object_release((objv_object_t *)channel->host);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
}

OBJV_CLASS_METHOD_IMP_BEGIN(TCPChannel)

OBJV_CLASS_METHOD_IMP(connect, "b(d)", objv_channel_tcp_connect)

OBJV_CLASS_METHOD_IMP(canRead, "i(d)", objv_channel_tcp_canRead)

OBJV_CLASS_METHOD_IMP(read, "i(*,i,d)", objv_channel_tcp_read)

OBJV_CLASS_METHOD_IMP(canWrite, "i(d)", objv_channel_tcp_canWrite)

OBJV_CLASS_METHOD_IMP(write, "i(*,i,d)", objv_channel_tcp_write)

OBJV_CLASS_METHOD_IMP(disconnect, "v()", objv_channel_tcp_disconnect)

OBJV_CLASS_METHOD_IMP(init, "@(*)", objv_channel_tcp_init)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", objv_channel_tcp_dealloc)

OBJV_CLASS_METHOD_IMP_END(TCPChannel)

OBJV_CLASS_IMP_M(TCPChannel, OBJV_CLASS(Channel), objv_channel_tcp_t)


objv_channel_tcp_t * objv_channel_tcp_allocWithHandle(objv_zone_t * zone,objv_channel_handler_t handler){
    return (objv_channel_tcp_t *) objv_object_alloc(zone, OBJV_CLASS(TCPChannel), 1,handler);
}

objv_channel_tcp_t * objv_channel_tcp_allocWithHost(objv_zone_t * zone,objv_string_t * host,int port){
    return (objv_channel_tcp_t *) objv_object_alloc(zone, OBJV_CLASS(TCPChannel), 2,host,port);
}

int objv_channel_tcp_getPort(objv_channel_tcp_t * tcp){
    if(tcp->handler){
        struct sockaddr_in addr;
        socklen_t socklen = sizeof(struct sockaddr_in);
        getsockname(tcp->handler, (struct sockaddr *) & addr, &socklen);
        return ntohs(addr.sin_port);
    }
    return tcp->port;
}

