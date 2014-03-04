//
//  CLAccept.c
//  objv
//
//  Created by zhang hailong on 14-2-28.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv.h"
#include "CLAccept.h"
#include "objv_autorelease.h"
#include "objv_clcontext.h"
#include "objv_value.h"

static void CLAcceptSIGNAN(int signo){

}

static void CLAcceptMethodDealloc(objv_class_t * clazz,objv_object_t * object){
    
    CLAccept * accept = (CLAccept *) object;
    
    objv_dispatch_queue_cancelAllTasks(accept->connectQueue);
    
    if(accept->copyed){
        
        if(accept->handler.sock > 0){
            
            objv_mutex_lock(& accept->handler.mutex);
            
            objv_os_socket_close(accept->handler.sock);
            
            objv_mutex_unlock(& accept->handler.mutex);
        }
        
        
        objv_mutex_destroy(& accept->handler.mutex);
        
    }
    
    objv_object_release((objv_object_t *) accept->ctx);
    objv_object_release((objv_object_t *) accept->connectQueue);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
    
}

static objv_object_t * CLAcceptMethodInit(objv_class_t * clazz,objv_object_t * object,va_list ap){
    
    
    if(clazz->superClass){
        object = objv_object_initv(clazz->superClass, object,ap);
    }
    
    if(object){
        
        CLAccept * accept = (CLAccept *) object;
        
        char * s = getenv(CL_ENV_MAX_THREAD_COUNT_KEY);
        
        unsigned int maxThreadCount = 256;
        
        if(s) {
            maxThreadCount = atoi(s);
        }
        
        if(maxThreadCount < 1){
            maxThreadCount = 1;
        }
        
        accept->connectQueue = objv_dispatch_queue_alloc(object->zone, "connnects", maxThreadCount);
        
    }
    
    return object;
    
}

static void CLAcceptMethodRun (objv_class_t * clazz,objv_object_t * object){
    
    CLAccept * ac = (CLAccept *) object;
    CLAcceptConnect * connect = NULL;
    
    OBJVChannelStatus status = CLAcceptGetConnect(ac, 0.02, & connect);
    
    if(status == OBJVChannelStatusOK && connect){
        
        objv_dispatch_queue_addTask(ac->connectQueue, (objv_dispatch_task_t *) connect);
        
    }
    
    if(status != OBJVChannelStatusError){
        
        ac->base.delay = 0.02;
        
        objv_dispatch_addTask(objv_dispatch_get_current(), (objv_dispatch_task_t *) ac);
    }
    
}


OBJV_KEY_IMP(CLAccept)
OBJV_KEY_IMP(CLAcceptConnect)

OBJV_CLASS_METHOD_IMP_BEGIN(CLAccept)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", CLAcceptMethodDealloc)

OBJV_CLASS_METHOD_IMP(init, "@(*)", CLAcceptMethodInit)

OBJV_CLASS_METHOD_IMP(run, "v()", CLAcceptMethodRun)

OBJV_CLASS_METHOD_IMP_END(CLAccept)

OBJV_CLASS_IMP_M(CLAccept, OBJV_CLASS(DispatchTask), CLAccept)


static void CLAcceptConnectMethodDealloc(objv_class_t * clazz,objv_object_t * object){
    
    CLAcceptConnect * conn = (CLAcceptConnect *) object;
    
    objv_object_release((objv_object_t *) conn->channel);

    objv_object_release((objv_object_t *) conn->ctx);
    
    objv_object_release((objv_object_t *) conn->httpChannel);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
    
}

static void CLAcceptConnectMethodRun (objv_class_t * clazz,objv_object_t * object){
    
    CLAcceptConnect * conn = (CLAcceptConnect *) object;
    objv_zone_t * zone = conn->base.base.zone;
    
    OBJVChannelStatus status = OBJVChannelStatusNone;
    
    if(conn->httpChannel == NULL){
        
        conn->httpChannel = (CLHttpChannel *) objv_object_alloc(zone, OBJV_CLASS(CLHttpChannel),NULL);
        
        CLChannelSetChannel((CLChannel *) conn->httpChannel, (objv_channel_t *) conn->channel);
        
    }
    
    CLHttpChannel * httpChannel = conn->httpChannel;
    
    while(1){
        
        if(httpChannel->httpRequest.state.state == OBJVHttpRequestStateNone){
            
            status = objv_channel_canRead(conn->channel->base.base.isa, (objv_channel_t *) conn->channel, 0.02);
            
            if(status != OBJVChannelStatusOK){
                break;
            }
            
            ssize_t len = objv_channel_read(conn->channel->base.base.isa, (objv_channel_t *) conn->channel, httpChannel->read.mbuf.data , httpChannel->read.mbuf.size );
            
            if(len <=0){
                status = (OBJVChannelStatus) len;
                break;
            }
            
            httpChannel->read.mbuf.length = len;
            
            OBJVHTTPRequestRead(& httpChannel->httpRequest, 0
                                , httpChannel->read.mbuf.length, (char *) httpChannel->read.mbuf.data);
        }
        
        if(httpChannel->httpRequest.state.state == OBJVHttpRequestStateOK){
            
            {
                OBJVHttpHeader * h = OBJVHttpRequestGetHeader(&httpChannel->httpRequest, "Transfer-Encoding");
                
                if(h){
                    
                    if(OBJVHttpStringEqual(h->value, "chunked", httpChannel->httpRequest.ofString)){
                        httpChannel->contentType |= CLHttpChannelContentTypeChunked;
                    }
                    
                }
                
                h = OBJVHttpRequestGetHeader(&httpChannel->httpRequest, "Content-Type");
                
                if(h){
                    
                    if(OBJVHttpStringEqual(h->value, "text/json", httpChannel->httpRequest.ofString)){
                        httpChannel->contentType |= CLHttpChannelContentTypeJSON;
                    }
                    
                }
                
                if(OBJVHttpStringHasPrefix(httpChannel->httpRequest.path, "/channel/", httpChannel->httpRequest.ofString)){
                    
                    {
                        char * p = httpChannel->httpRequest.ofString + httpChannel->httpRequest.path.location ;
                        CLChannelContext * context ;
                        
                        p[httpChannel->httpRequest.path.length] = 0;
                        p += 9;
                        
                        httpChannel->base.mode = CLChannelModeNone;
                        
                        if(OBJVHttpStringEqual(httpChannel->httpRequest.method, "GET", httpChannel->httpRequest.ofString)){
                            httpChannel->base.mode |= CLChannelModeWrite;
                        }
                        else if(OBJVHttpStringEqual(httpChannel->httpRequest.method, "POST", httpChannel->httpRequest.ofString)){
                            httpChannel->base.mode |= CLChannelModeWrite | CLChannelModeRead;
                        }
                        else if(OBJVHttpStringEqual(httpChannel->httpRequest.method, "PUT", httpChannel->httpRequest.ofString)){
                            httpChannel->base.mode |= CLChannelModeRead;
                        }
                        
                        objv_mbuf_clear(&httpChannel->write.mbuf);
                        
                        objv_mbuf_format(&httpChannel->write.mbuf, "HTTP/1.1 200 OK\r\nContent-Type: text/json\r\nTransfer-Encoding: chunked\r\n\r\n");
                        
                        status = objv_channel_canWrite(conn->channel->base.base.isa, (objv_channel_t *) conn->channel, 0.02);
                        
                        if(status == OBJVChannelStatusOK){
                            
                            if(objv_channel_write(conn->channel->base.base.isa, (objv_channel_t *) conn->channel, httpChannel->write.mbuf.data, httpChannel->write.mbuf.length) != httpChannel->write.mbuf.length){
                                status = OBJVChannelStatusError;
                                break;
                            }
                            
                        }
                        else{
                            status = OBJVChannelStatusError;
                            break;
                        }
                        
                        
                        objv_mbuf_clear(&httpChannel->write.mbuf);
                        
                        context = (CLChannelContext *) objv_object_new(zone, OBJV_CLASS(CLChannelContext),NULL);
                        
                        context->allowRemovedFromParent = objv_true;
                        
                        CLContextSetDomain((CLContext *) context, objv_string_new(zone, p));
                        
                        CLChannelContextAddChannel(context, (CLChannel *) httpChannel);
                        
                        CLContextAddChild(conn->ctx, (CLContext *) context);
                        
                    }
                    
                }
                else{
                    
                    objv_mbuf_clear(&httpChannel->write.mbuf);
                    
                    objv_mbuf_format(&httpChannel->write.mbuf, "HTTP/1.1 200 OK\r\nContent-Type: text/json\r\nTransfer-Encoding: chunked\r\n\r\n0\r\n\r\n");
                    
                    status = objv_channel_canWrite(conn->channel->base.base.isa, (objv_channel_t *) conn->channel, 0.02);
                    
                    if(status == OBJVChannelStatusOK){
                        
                        if(objv_channel_write(conn->channel->base.base.isa, (objv_channel_t *) conn->channel, httpChannel->write.mbuf.data, httpChannel->write.mbuf.length) != httpChannel->write.mbuf.length){
                            status = OBJVChannelStatusError;
                            break;
                        }
                        
                    }
                    else{
                        status = OBJVChannelStatusError;
                        break;
                    }
                }
                
            }
            
            
            status = OBJVChannelStatusOK;
            break;
        }
        else if(httpChannel->httpRequest.state.state == OBJVHttpRequestStateFail){
            status = OBJVChannelStatusError;
            break;
        }
        else if(httpChannel->read.mbuf.length - httpChannel->httpRequest.length == 0){
            
            objv_mbuf_extend(& httpChannel->read.mbuf, httpChannel->httpRequest.length + 1024);
            
            status = objv_channel_canRead(conn->channel->base.base.isa, (objv_channel_t *) conn->channel, 0.02);
            
            if(status != OBJVChannelStatusOK){
                break;
            }
            
            ssize_t len = objv_channel_read(conn->channel->base.base.isa, (objv_channel_t *) conn->channel
                                            , (char *) httpChannel->read.mbuf.data + httpChannel->httpRequest.length
                                            , httpChannel->read.mbuf.size - httpChannel->httpRequest.length );
            
            if(len <=0){
                status = (OBJVChannelStatus) len;
                break;
            }
        
            httpChannel->read.mbuf.length += len;
            
            OBJVHTTPRequestRead(& httpChannel->httpRequest, httpChannel->httpRequest.length
                                , len, (char *) httpChannel->read.mbuf.data);
        }
        
    }
    
    
    if(status == OBJVChannelStatusNone){
        
        conn->base.delay = 0.002;
        
        objv_dispatch_addTask(objv_dispatch_get_current(), (objv_dispatch_task_t *) conn);
    }
}

OBJV_CLASS_METHOD_IMP_BEGIN(CLAcceptConnect)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", CLAcceptConnectMethodDealloc)

OBJV_CLASS_METHOD_IMP(run, "v()", CLAcceptConnectMethodRun)

OBJV_CLASS_METHOD_IMP_END(CLAcceptConnect)

OBJV_CLASS_IMP_M(CLAcceptConnect, OBJV_CLASS(DispatchTask), CLAcceptConnect)


CLAccept * CLAcceptAlloc(objv_zone_t * zone,int port){
    
    CLAcceptHandler h;
    
    objv_zone_memzero(zone, & h, sizeof(h));
    
    objv_mutex_init(& h.mutex);
    
    {
        int res;
        struct sockaddr_in addr ;
        int fl;
        int fn = 1;
        
        memset(&addr, 0, sizeof(struct sockaddr_in));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;
        
        h.sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        
        if(h.sock <=0 ){
            return NULL;
        }
        
        res = bind(h.sock, (struct sockaddr *) & addr, sizeof(struct sockaddr_in));
        
        if(res != 0){
            closesocket(h.sock);
            return NULL;
        }
        
        res = listen(h.sock, SOMAXCONN);
        
        if(res != 0){
            closesocket(h.sock);
            return NULL;
        }
        
        fl =  fcntl(h.sock, F_GETFL) ;
        fcntl(h.sock, F_SETFL, fl | O_NONBLOCK);
        
        setsockopt(h.sock, SOL_SOCKET, SO_RCVLOWAT, (void *)&fn, sizeof(fn));
        setsockopt(h.sock, SOL_SOCKET, SO_SNDLOWAT, (void *)&fn, sizeof(fn));
        
    }
    
    signal(SIGPIPE, CLAcceptSIGNAN);
    signal(SIGTTOU, CLAcceptSIGNAN);
    signal(ETIMEDOUT, CLAcceptSIGNAN);
    
    CLAccept * ac = CLAcceptAllocWithHandler (zone,& h);
    
    ac->copyed = objv_true;
    
    return ac;
}


CLAccept * CLAcceptAllocWithHandler(objv_zone_t * zone,CLAcceptHandler * handler){
    CLAccept * ac = (CLAccept *) objv_object_alloc(zone, OBJV_CLASS(CLAccept),NULL);
    ac->handler = * handler;
    
    {
        struct sockaddr_in addr ;
        socklen_t socklen = sizeof(struct sockaddr_in);
        getsockname(handler->sock, (struct sockaddr *) & addr, &socklen);
        ac->port = ntohs(addr.sin_port);
    }
    
    return ac;
}

OBJVChannelStatus CLAcceptGetConnect(CLAccept * ac,objv_timeinval_t timeout,CLAcceptConnect ** connenct ){
    if(ac){
        
        objv_os_socket_t client = 0;
        fd_set rds;
        int res;
        int fl;
        int fn = 1;
        struct sockaddr_in addr;
        socklen_t socklen = sizeof(addr);
        
        struct timeval timeo = {(int)timeout, (timeout - (int) timeout) * 1000000};
        
        objv_mutex_lock(& ac->handler.mutex);
        
        FD_ZERO(&rds);
        
        FD_SET(ac->handler.sock, &rds);
        
        res = select(ac->handler.sock + 1, &rds, NULL, NULL, &timeo);
        
        if(res == 0){
            
        }
        else if(res == -1){
            if(errno == EINTR){
                
            }
            else{
                client = -1;
            }
        }
        else{
            if(FD_ISSET(ac->handler.sock, &rds)){
                
                client = accept(ac->handler.sock, (struct sockaddr *) & addr,& socklen);
                
                if(client != -1){
                    fl =  fcntl(client, F_GETFL) ;
                    fcntl(client, F_SETFL, fl | O_NONBLOCK);
                    setsockopt(ac->handler.sock, SOL_SOCKET, SO_RCVLOWAT, (void *)&fn, sizeof(fn));
                    setsockopt(ac->handler.sock, SOL_SOCKET, SO_SNDLOWAT, (void *)&fn, sizeof(fn));
                }
            }
        }
        
        objv_mutex_unlock(& ac->handler.mutex);
        
        if(client >0 ){
            
            {
                CLAcceptConnect * conn = (CLAcceptConnect *) objv_object_new(ac->base.base.zone,OBJV_CLASS(CLAcceptConnect),NULL);
                conn->channel = objv_channel_tcp_allocWithHandle(ac->base.base.zone, client);
                conn->from = addr;
                conn->ctx = (CLContext *) objv_object_retain((objv_object_t *) ac->ctx);
                
                * connenct = conn;
            }
            
            return OBJVChannelStatusOK;
        }
        else if(client < 0){
            return OBJVChannelStatusError;
        }

        return OBJVChannelStatusNone;
    }
    return OBJVChannelStatusError;
}

