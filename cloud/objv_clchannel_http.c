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
#include "objv_json.h"
#include "objv_value.h"
#include "objv_autorelease.h"
#include "objv_url.h"

OBJV_KEY_IMP(CLHttpChannel)

static objv_object_t * CLHttpChannelMethodInit(objv_class_t * clazz,objv_object_t * object,va_list ap){
    
    if(clazz->superClass){
        object = objv_object_initv(clazz->superClass, object, ap);
    }
    
    if(object){
        
        CLHttpChannel * channel = (CLHttpChannel *) object;
        
        OBJVHTTPRequestInit( &channel->httpRequest );
        
        objv_mbuf_init(& channel->read.mbuf, 128);
        objv_mbuf_init(& channel->read.data, 128);
        
        objv_mbuf_init(& channel->write.mbuf, 128);
        
        objv_mutex_init( & channel->tasks_mutex);
        
    }
    
    return object;
}

static void CLHttpChannelMethodDealloc(objv_class_t * clazz,objv_object_t * object){
    
    CLHttpChannel * channel = (CLHttpChannel *) object;
 
    objv_zone_t * zone = channel->base.base.zone;
    
    if(channel->base.oChannel && channel->base.mode & CLChannelModeWrite){
        
        objv_mbuf_clear(& channel->write.mbuf);
        
        objv_mbuf_append(& channel->write.mbuf, "0\r\n\r\n", 5);
        
        {
            OBJVChannelStatus status = objv_channel_canWrite(channel->base.oChannel->base.isa, channel->base.oChannel, 0.02);
            if(status == OBJVChannelStatusOK){
                objv_channel_write(channel->base.oChannel->base.isa, channel->base.oChannel, channel->write.mbuf.data, channel->write.mbuf.length);
            }
        }
        
    }

    
    OBJVHTTPRequestDestroy( &channel->httpRequest );
    
    objv_mutex_lock(& channel->tasks_mutex);
    
    CLHttpChannelPostTask * t;
    
    while(channel->beginTask){
        
        t = channel->beginTask;
        
        objv_object_release((objv_object_t *) t->task);
        
        objv_zone_free(zone, t);
        
        channel->beginTask = t->next;
    }
    
    channel->beginTask = channel->endTask = NULL;
    
    objv_mutex_unlock(& channel->tasks_mutex);
    
    objv_mutex_destroy(& channel->tasks_mutex);
    
    objv_mbuf_destroy( & channel->read.mbuf);
    objv_mbuf_destroy( & channel->read.data);
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

static OBJVChannelStatus CLHttpChannelMethodDisconnect(objv_class_t * clazz,CLChannel * channel){
    
    if(channel->oChannel){
        return objv_channel_disconnect(channel->oChannel->base.isa, channel->oChannel);
    }
    
    return OBJVChannelStatusError;
}


OBJVChannelStatus CLHttpChannelUnpackageTask(objv_zone_t * zone, CLTask ** task,objv_class_t ** taskType,objv_mbuf_t * data,CLHttpChannelContentType contentType){
    
    if(contentType & CLHttpChannelContentTypeChunked){
        
        OBJVHttpRequest request;
        OBJVHttpHeader * h;
        objv_class_t * tType = NULL;
        objv_class_t * tClass = NULL;
        CLTask * t;
        size_t length;
        char * p = data->data;
        
        OBJVChannelStatus status = OBJVChannelStatusNone;
        
        OBJVHTTPRequestInit(& request);
        
        request.state.state = OBJVHttpRequestStateHeaderKey;
        
        if(OBJVHTTPRequestRead(& request, 0,  data->length, data->data) == OBJVHttpRequestStateOK){
            
            while(1) {
            
                h = OBJVHttpRequestGetHeader(& request, "taskType");
                
                if(! h){
                    break;
                }
                
                p[h->value.location + h->value.length] = 0;
                
                tType = objv_class(objv_key( p + h->value.location));
            
                if(!tType){
                    break;
                }
                
                if(!objv_class_isKindOfClass(tType, OBJV_CLASS(CLTask))){
                    break;
                }
                
                h = OBJVHttpRequestGetHeader(& request, "taskClass");
                
                if(! h){
                    break;
                }
                
                p[h->value.location + h->value.length] = 0;
                
                tClass = objv_class(objv_key( p + h->value.location));
                
                if(!tClass){
                    break;
                }
                
                if(!objv_class_isKindOfClass(tClass, OBJV_CLASS(CLTask))){
                    break;
                }
                
                t = (CLTask *) objv_object_new(zone, tClass,NULL);
                
                h = OBJVHttpRequestGetHeader(& request, "identifier");
                
                if(! h){
                    break;
                }
                
                p[h->value.location + h->value.length] = 0;
                
                t->identifier = atoll(p + h->value.location);
                
                h = OBJVHttpRequestGetHeader(& request, "replyIdentifier");
                
                if(! h){
                    break;
                }
                
                p[h->value.location + h->value.length] = 0;
                
                t->replyIdentifier = atoll(p + h->value.location);
                
                h = OBJVHttpRequestGetHeader(& request, "Content-Type");
                
                if(h){
                    
                    t->contentType = objv_string_alloc_with_length(zone, p + h->value.location , h->value.length);
                    
                    h = OBJVHttpRequestGetHeader(& request, "Content-Length");
                    
                    if(h){
                        
                        p[h->value.location + h->value.length] = 0;
                        
                        length = atol(p + h->value.location);
                        
                        if(length > data->length - request.length){
                            length = data->length - request.length;
                        }
                        
                        objv_mbuf_init(& t->content, length);
                        
                        objv_mbuf_append(& t->content, (char *) data->data + request.length, length);
                        
                    }
                    
                }
                
                {
                    
                    objv_key_t * key;
                    objv_property_t * prop;
                    objv_class_t * ofClass;
                    objv_string_t * v;
                    h = request.headers.data;
                    length = request.headers.length;
                    
                    while (h && length >0) {
                        
                        p[h->key.location + h->key.length] = 0;
                        p[h->value.location + h->value.length] = 0;
                        
                        key = objv_key(p + h->key.location);
                        
                        prop = objv_class_getPropertyOfClass(tClass, key, & ofClass);
                        
                        if(prop && prop->setter && prop->serialization){
                            
                            v = objv_string_new(zone, p + h->value.location);
                            
                            objv_property_setObjectValue(ofClass, (objv_object_t *) t, prop, (objv_object_t *) v);
                            
                        }
                        
                        
                        
                        h ++;
                        length --;
                    }
                }

                if(taskType){
                    * taskType = tType;
                }
                if(task){
                    * task = t;
                }
                
                status = OBJVChannelStatusOK;
                
                break;
            }

        }
        
        OBJVHTTPRequestDestroy(& request);
        
        return status;
    }
    
    return OBJVChannelStatusError;
}

OBJVChannelStatus CLHttpChannelPackageTask(objv_zone_t * zone, CLTask * task,objv_class_t * taskType,objv_mbuf_t * mbuf,CLHttpChannelContentType contentType){
    
    if( contentType & CLHttpChannelContentTypeChunked ){
        
        
        size_t off = mbuf->length;
        
        objv_mbuf_extend(mbuf, mbuf->length + 10);
        
        objv_mbuf_format(mbuf, "%08lx\r\n", 0);
        
        objv_mbuf_format(mbuf, "taskType: %s\r\n",taskType->name->name);
        objv_mbuf_format(mbuf, "taskClass: %s\r\n",task->base.isa->name->name);
        objv_mbuf_format(mbuf, "identifier: %lld\r\n",task->identifier);
        objv_mbuf_format(mbuf, "replyIdentifier: %lld\r\n",task->replyIdentifier);
        
        {
            objv_class_t * clazz = task->base.isa;
            objv_property_t * prop;
            objv_string_t * v;
            unsigned int propCount;
            
            while(clazz){
                
                prop = clazz->propertys;
                propCount = clazz->propertyCount;
                
                while (prop && propCount >0) {

                    v = objv_property_stringValue(clazz, (objv_object_t *) task, prop, NULL);
                    
                    if(v){
                        objv_mbuf_format(mbuf, "%s: ",prop->name->name);
                        objv_url_encode_mbuf(zone, v->UTF8String, mbuf);
                        objv_mbuf_append(mbuf, "\r\n", 2);
                    }
                    
                    propCount --;
                    prop ++;
                }
                
                
                clazz = clazz->superClass;
            }
        }
        
        if(task->contentType){
            
            objv_mbuf_format(mbuf, "Content-Type: %s\r\n", task->contentType->UTF8String);
            objv_mbuf_format(mbuf, "Content-Length: %lu\r\n",task->content.length);
            
            objv_mbuf_append(mbuf, "\r\n", 2);
            
            objv_mbuf_append(mbuf, task->content.data, task->content.length);
            
        }
        else{
            objv_mbuf_append(mbuf, "\r\n", 2);
        }
        
        snprintf((char *) mbuf->data + off, 10,"%08lx\r\n",mbuf->length - 10 - off);
        
        * ((char *) mbuf->data + off + 9) = '\n';
        
        objv_mbuf_append(mbuf, "\r\n", 2);
    
        return OBJVChannelStatusNone;
    }
    
    return OBJVChannelStatusError;
}


static OBJVChannelStatus CLHttpChannelMethodReadTask(objv_class_t * clazz,CLChannel * channel,CLTask ** task,objv_class_t ** taskType,objv_timeinval_t timeout){
    
    
    CLHttpChannel * httpChannel = (CLHttpChannel *) channel;
    objv_channel_t * oChannel = httpChannel->base.oChannel;
    
    if(( httpChannel->base.mode & CLChannelModeRead ) && ( httpChannel->contentType & CLHttpChannelContentTypeChunked)){
    
        OBJVChannelStatus status = OBJVChannelStatusNone;
        ssize_t len;
        char * p ;
        objv_zone_t * zone = channel->base.zone;
        
        while(1){
            
            if(httpChannel->read.mbuf.length - httpChannel->read.off == 0){
                
                objv_mbuf_extend(& httpChannel->read.mbuf, 128);
                
                status = objv_channel_canRead(oChannel->base.isa, oChannel, timeout);
                
                if(status != OBJVChannelStatusOK){
                    return status;
                }
                
                len = objv_channel_read(oChannel->base.isa, oChannel, httpChannel->read.mbuf.data, httpChannel->read.mbuf.size);
                
                if(len <= 0){
                    return (OBJVChannelStatus) len;
                }
                
                httpChannel->read.mbuf.length = len;
                httpChannel->read.off = 0;
            }
            
            if(httpChannel->read.mbuf.length - httpChannel->read.off == 0){
                return status;
            }
            
            p = (char *) httpChannel->read.mbuf.data + httpChannel->read.off;
            
            if(httpChannel->read.state == 0){
            
                if(*p == '\r'){
                    
                }
                else if(*p == '\n'){
                    httpChannel->read.state = 1;
                    objv_mbuf_clear(& httpChannel->read.data);
                }
                else if(*p >='0' && *p <='9'){
                    httpChannel->read.dataLength = (httpChannel->read.dataLength << 4) + (*p - '0');
                }
                else if(*p >='a' && *p <='f'){
                    httpChannel->read.dataLength = (httpChannel->read.dataLength << 4) + (*p - 'a' + 10);
                }
                else if(*p >='A' && *p <='F'){
                    httpChannel->read.dataLength = (httpChannel->read.dataLength << 4) + (*p - 'A' + 10);
                }
                else{
                    return OBJVChannelStatusError;
                }
                
            }
            else if (httpChannel->read.state == 1){
                
                if(httpChannel->read.dataLength > httpChannel->read.data.length){
                    len = MIN(httpChannel->read.dataLength - httpChannel->read.data.length, httpChannel->read.mbuf.length - httpChannel->read.off);
                    memcpy(httpChannel->read.data.data, httpChannel->read.mbuf.data, len);
                    httpChannel->read.off += len;
                    httpChannel->read.data.length += len;
                }
                
                if(httpChannel->read.dataLength == httpChannel->read.data.length){
                    
                    status = CLHttpChannelUnpackageTask(zone,task,taskType, & httpChannel->read.data,httpChannel->contentType);
                    
                    if(status != OBJVChannelStatusOK){
                        return status;
                    }
                    
                    httpChannel->read.state = 2;
                    
                    break;
                }
                
                continue;
                
            }
            else if(httpChannel->read.state == 2){
                if(*p == '\r'){
                    
                }
                else if(*p == '\n'){
                    httpChannel->read.state = 0;
                }
                else{
                    status = OBJVChannelStatusError;
                    break;
                }
            }
            else {
                assert(0);
            }
            
            httpChannel->read.off ++;
        }
        
        return status;
    }
    
    return OBJVChannelStatusError;
}

static void CLHttpChannelMethodPostTask(objv_class_t * clazz,CLChannel * channel,CLTask * task,objv_class_t * taskType){
    
    CLHttpChannel * httpChannel = (CLHttpChannel *) channel;
    
    if( (httpChannel->base.mode & CLChannelModeWrite) && task && taskType ){
        
        objv_zone_t * zone = httpChannel->base.base.zone;
        
        CLHttpChannelPostTask * t = (CLHttpChannelPostTask *) objv_zone_malloc(zone, sizeof(CLHttpChannelPostTask));
        
        objv_zone_memzero(zone, t, sizeof(CLHttpChannelPostTask));
        
        t->task = (CLTask *) objv_object_retain((objv_object_t *) task);
        t->taskType = taskType;
        
        objv_mutex_lock(& httpChannel->tasks_mutex);

        if(httpChannel->beginTask == NULL){
            httpChannel->beginTask = httpChannel->endTask = t;
        }
        else{
            httpChannel->endTask->next = t;
            httpChannel->endTask = t;
        }
        
        objv_mutex_unlock(& httpChannel->tasks_mutex);

    }
    
}

static OBJVChannelStatus CLHttpChannelMethodTick(objv_class_t * clazz,CLChannel * channel,objv_timeinval_t timeout){
    
    CLHttpChannel * httpChannel = (CLHttpChannel *) channel;
    objv_channel_t * oChannel = httpChannel->base.oChannel;
    
    if( httpChannel->base.mode & CLChannelModeWrite ){
        
        OBJVChannelStatus status = OBJVChannelStatusNone;
        
        if(httpChannel->write.state == 0){
            
            objv_zone_t * zone = channel->base.zone;
            
            CLHttpChannelPostTask * task = NULL;
            
            objv_mutex_lock(& httpChannel->tasks_mutex);
            
            task = httpChannel->beginTask;
            
            if(task){
                httpChannel->beginTask = task->next;
            }
            
            if(task == httpChannel->endTask){
                httpChannel->endTask = NULL;
            }
            
            objv_mutex_unlock(& httpChannel->tasks_mutex);
            
            if(task){
            
                objv_mbuf_clear( & httpChannel->write.mbuf);
                
                CLHttpChannelPackageTask(zone,task->task,task->taskType,& httpChannel->write.mbuf, CLHttpChannelContentTypeChunked);
                
                httpChannel->write.off = 0;
                httpChannel->write.state = 1;
                
                
                objv_object_release((objv_object_t *) task->task);
                
                objv_zone_free(zone, task);
            }
            
        }
        
        if(httpChannel->write.state == 1){
            
            
            ssize_t len;
            
            while (1) {
                
                if(httpChannel->write.mbuf.length - httpChannel->write.off == 0){
                    httpChannel->write.state = 0;
                    break;
                }
                
                status = objv_channel_canWrite(oChannel->base.isa, oChannel, timeout);
                
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
            
            return status;
        }
        
        return status;
    }
    
    return OBJVChannelStatusError;
}


OBJV_CLASS_METHOD_IMP_BEGIN(CLHttpChannel)

OBJV_CLASS_METHOD_IMP(init, "@(*)", CLHttpChannelMethodInit)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", CLHttpChannelMethodDealloc)

OBJV_CLASS_METHOD_IMP(connect, "i(d)", CLHttpChannelMethodConnect)

OBJV_CLASS_METHOD_IMP(disconnect, "i()", CLHttpChannelMethodDisconnect)

OBJV_CLASS_METHOD_IMP(readTask, "i(*,*,d)", CLHttpChannelMethodReadTask)

OBJV_CLASS_METHOD_IMP(postTask, "v(*,*)", CLHttpChannelMethodPostTask)

OBJV_CLASS_METHOD_IMP(tick, "i(d)", CLHttpChannelMethodTick)

OBJV_CLASS_METHOD_IMP_END(CLChannel)

OBJV_CLASS_IMP_M(CLHttpChannel, OBJV_CLASS(CLChannel), CLHttpChannel)


