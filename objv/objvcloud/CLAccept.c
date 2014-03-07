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
#include "objv_log.h"

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
    
    objv_object_release((objv_object_t *) conn->queue);
    
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
                objv_timeinval_t keepAlive = 0;
                
                if(h){
                    
                    if(OBJVHttpStringEqual(h->value, "chunked", httpChannel->httpRequest.ofString)){
                        httpChannel->contentType |= CLHttpChannelContentTypeChunked;
                    }
                    
                }
                
                h = OBJVHttpRequestGetHeader(&httpChannel->httpRequest, "Keep-Alive");
                
                if(h){
                    
                    {
                        char *p = httpChannel->httpRequest.ofString + h->value.location;
                        
                        p[h->value.length] = 0;
                        
                        keepAlive = atof(p);
                        
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
                        
                        objv_mbuf_format(&httpChannel->write.mbuf, "HTTP/1.1 200 OK\r\nContent-Type: text/task\r\nTransfer-Encoding: chunked\r\n\r\n");
                        
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
                        
                        CLChannelContextSetQueue(context, conn->queue);
                        
                        context->allowRemovedFromParent = objv_true;
                        context->keepAlive = keepAlive;
                        
                        keepAlive = objv_object_doubleValueForKey(conn->ctx->config, (objv_object_t *) objv_string_new_nocopy(zone, "keepAlive"), 20);
                        
                        if(context->keepAlive == 0){
                            context->keepAlive = keepAlive;
                        }
                        else if(context->keepAlive > keepAlive){
                            context->keepAlive = keepAlive;
                        }
                        
                        CLContextSetDomain((CLContext *) context, objv_string_new(zone, p));
                        
                        CLChannelSetURL((CLChannel *) conn->httpChannel, objv_url_newWithFormat(zone, "http://%s:%d/channel/%s",inet_ntoa(conn->from.sin_addr)
                                                                                      ,ntohs(conn->from.sin_port),p));
                        
                        CLChannelContextAddChannel(context, (CLChannel *) httpChannel);
                        
                        CLContextAddChild(conn->ctx, (CLContext *) context);
                        
                        status = OBJVChannelStatusOK;
                        
                        break;
                    }
                    
                }
                else if(OBJVHttpStringHasPrefix(httpChannel->httpRequest.path, "/info", httpChannel->httpRequest.ofString)){
                
                    {
                        CLAcceptContextInfoTask * task = (CLAcceptContextInfoTask *) objv_object_alloc(zone, OBJV_CLASS(CLAcceptContextInfoTask),NULL);
                        
                        objv_mbuf_clear(&httpChannel->write.mbuf);
                        
                        objv_mbuf_format(&httpChannel->write.mbuf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nTransfer-Encoding: chunked\r\n\r\n");
                   
                        task->ctx = (CLContext *) objv_object_retain((objv_object_t *) conn->ctx);
                        task->httpChannel = (CLHttpChannel *) objv_object_retain((objv_object_t *) conn->httpChannel);
                   
                        objv_dispatch_addTask(task->ctx->dispatch, (objv_dispatch_task_t *) task);
                        
                        objv_object_release((objv_object_t *) task);
                        
                        status = OBJVChannelStatusOK;
                        
                        break;
                    }
                    
                }
                else{
                    
                    objv_mbuf_clear(&httpChannel->write.mbuf);
                    
                    objv_mbuf_format(&httpChannel->write.mbuf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nTransfer-Encoding: chunked\r\n\r\n0\r\n\r\n");
                    
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

void CLAcceptSetConnectQueue(CLAccept * accept,objv_dispatch_queue_t * queue){
    if(accept && accept->connectQueue != queue){
        objv_object_retain((objv_object_t *) queue);
        objv_object_release((objv_object_t *) accept->connectQueue);
        accept->connectQueue = queue;
    }
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
                conn->queue = (objv_dispatch_queue_t *) objv_object_retain((objv_object_t *) ac->connectQueue);
                
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


OBJV_KEY_IMP(CLAcceptContextInfoTask)

static void CLAcceptContextInfoTaskDealloc(objv_class_t * clazz,objv_object_t * object){
    
    CLAcceptContextInfoTask * task = (CLAcceptContextInfoTask *) object;
    
    objv_object_release((objv_object_t *) task->ctx);
    
    objv_object_release((objv_object_t *) task->httpChannel);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
}

static void CLAcceptContextInfoTaskRunContextInfo(CLContext * ctx, objv_mbuf_t * mbuf){

    CLContext * child;
    CLChannelContext * channelContext;
    CLChannel * channel;
    int i;
    
    objv_mbuf_format(mbuf, "domain: %s\r\n", ctx->domain ? ctx->domain->UTF8String : "");
    
    objv_mbuf_format(mbuf, "identifier: %llu\r\n", ctx->identifier);
    
    if(ctx->parent){
        
        objv_mbuf_format(mbuf, "parent: %s\r\n",ctx->parent->domain ? ctx->parent->domain->UTF8String : "");
        
    }
    
    if(objv_object_isKindOfClass((objv_object_t *) ctx, OBJV_CLASS(CLChannelContext))){
        
        channelContext = (CLChannelContext *) ctx;
        
        objv_mutex_lock(& channelContext->channels_mutex);
        
        if(channelContext->channels){
            
            objv_mbuf_format(mbuf, "channels: \r\n");
        
            for(i=0;i<channelContext->channels->length;i++){
                channel = (CLChannel *) objv_array_objectAt(channelContext->channels, i);
                objv_mbuf_format(mbuf, "url: %s\r\n", channel->url && channel->url->absoluteString ? channel->url->absoluteString->UTF8String : "");
            }
            
            objv_mbuf_append(mbuf, "\r\n", 2);
        }
        
        objv_mutex_unlock(& channelContext->channels_mutex);
        
    }
    
    if(ctx->childs){
        
        objv_mbuf_format(mbuf, "childs: \r\n");
        
        for (int i=0; i<ctx->childs->length; i++) {
            
            child = (CLContext *) objv_array_objectAt(ctx->childs, i);

            CLAcceptContextInfoTaskRunContextInfo(child,mbuf);
        }
        
        objv_mbuf_append(mbuf, "\r\n", 2);
    }
    
    objv_mbuf_append(mbuf, "\r\n", 2);
    
}

static void CLAcceptContextInfoTaskRun(objv_class_t * clazz,objv_object_t * object){
   
    CLAcceptContextInfoTask * task = (CLAcceptContextInfoTask *) object;
    CLHttpChannel * httpChannel = task->httpChannel;
    objv_mbuf_t * mbuf = & httpChannel->write.mbuf;
    objv_channel_t * oChannel = httpChannel->base.oChannel;
    CLContext * ctx = task->ctx;

    ssize_t off = mbuf->length;
    
    objv_mbuf_extend(mbuf, off + 32);
    
    objv_mbuf_format(mbuf, "%08lx\r\n", 0);
    
    CLAcceptContextInfoTaskRunContextInfo(ctx, mbuf);

    snprintf((char *) mbuf->data + off, 10,"%08lx\r\n",mbuf->length - 10 - off);
    
    * ((char *) mbuf->data + off + 9) = '\n';
    
    objv_mbuf_format(mbuf, "\r\n0\r\n\r\n");
    
    httpChannel->write.off = 0;
    
    ssize_t len;
    OBJVChannelStatus status;
    
    while (1) {
        
        if(httpChannel->write.mbuf.length - httpChannel->write.off == 0){
            httpChannel->write.state = 0;
            break;
        }
        
        status = objv_channel_canWrite(oChannel->base.isa, oChannel, 0.02);
        
        if(status != OBJVChannelStatusOK){
            break;
        }
        
        len = objv_channel_write(oChannel->base.isa, oChannel,(char *) httpChannel->write.mbuf.data + httpChannel->write.off, httpChannel->write.mbuf.length - httpChannel->write.off);
        
        if(len >0){
            
            httpChannel->write.off += len;
            
        }
        else{
            status = (OBJVChannelStatus) len;
            break;
        }
    }
    
    
}

OBJV_CLASS_METHOD_IMP_BEGIN(CLAcceptContextInfoTask)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", CLAcceptContextInfoTaskDealloc)

OBJV_CLASS_METHOD_IMP(run, "v()", CLAcceptContextInfoTaskRun)

OBJV_CLASS_METHOD_IMP_END(CLAcceptContextInfoTask)

OBJV_CLASS_IMP_M(CLAcceptContextInfoTask, OBJV_CLASS(DispatchTask), CLAcceptContextInfoTask)

