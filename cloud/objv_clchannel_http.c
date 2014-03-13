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
#include "objv_log.h"
#include "objv_crypt.h"


OBJV_KEY_IMP(CLHttpChannel)

static objv_object_t * CLHttpChannelMethodInit(objv_class_t * clazz,objv_object_t * object,va_list ap){
    
    if(clazz->superClass){
        object = objv_object_initv(clazz->superClass, object, ap);
    }
    
    if(object){
        
        CLHttpChannel * channel = (CLHttpChannel *) object;
        
        OBJVHTTPRequestInit( &channel->httpRequest );
        OBJVHTTPResponseInit( &channel->httpResponse );
        
        objv_mbuf_init(& channel->read.mbuf, 512);
        objv_mbuf_init(& channel->read.data, 512);
        objv_mbuf_init(& channel->read.decode, 512);
        
        objv_mbuf_init(& channel->write.mbuf, 512);
        objv_mbuf_init(& channel->write.encode, 512);
        
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
    OBJVHTTPResponseDestroy( &channel->httpResponse );
    
    objv_mutex_lock(& channel->tasks_mutex);
    
    CLHttpChannelPostTask * t;
    
    while(channel->beginTask){
        
        t = channel->beginTask;
        
        objv_object_release((objv_object_t *) t->task);
        objv_object_release((objv_object_t *) t->target);
        
        objv_zone_free(zone, t);
        
        channel->beginTask = t->next;
    }
    
    channel->beginTask = channel->endTask = NULL;
    
    objv_mutex_unlock(& channel->tasks_mutex);
    
    objv_mutex_destroy(& channel->tasks_mutex);
    
    objv_mbuf_destroy( & channel->read.mbuf);
    objv_mbuf_destroy( & channel->read.data);
    objv_mbuf_destroy( & channel->read.decode);
    objv_mbuf_destroy( & channel->write.mbuf);
    objv_mbuf_destroy( & channel->write.encode);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
    
}

static OBJVChannelStatus CLHttpChannelMethodConnect(objv_class_t * clazz,CLChannel * channel,objv_timeinval_t timeout){
    
    CLHttpChannel * httpChannel = (CLHttpChannel *) channel;
    
    OBJVChannelStatus status = OBJVChannelStatusError;
    
    ssize_t len;
    
    if(channel->oChannel){
        status = objv_channel_connect(channel->oChannel->base.isa, channel->oChannel, timeout);
    }
    
    if(status ==OBJVChannelStatusOK){
        
        if(channel->mode == CLChannelModeNone){
            
            objv_mbuf_clear(& httpChannel->write.mbuf);
            
            objv_mbuf_format(& httpChannel->write.mbuf, "POST %s",channel->url->path->UTF8String);
            
            if(channel->url->query){
                objv_mbuf_format(& httpChannel->write.mbuf, "?%s",channel->url->query->UTF8String);
            }
            
            if(channel->url->token){
                objv_mbuf_format(& httpChannel->write.mbuf, "#",channel->url->token->UTF8String);
            }
            
            objv_mbuf_format(& httpChannel->write.mbuf, " 1.1\r\n");
            
            objv_mbuf_format(& httpChannel->write.mbuf, "Content-Type: text/task\r\nTransfer-Encoding: chunked\r\nAccept-Encoding: gzip\r\n\r\n");
            
            while(1){
                
                status = objv_channel_canWrite(channel->oChannel->base.isa, channel->oChannel, timeout);
                
                if(status == OBJVChannelStatusError){
                    break;
                }
                else if(status == OBJVChannelStatusOK){
                    
                    len = objv_channel_write(channel->oChannel->base.isa, channel->oChannel
                                             , (char *) httpChannel->write.mbuf.data + httpChannel->write.off ,httpChannel->write.mbuf.length - httpChannel->write.off);
                    
                    if(len == 0){
                        continue;
                    }
                    else if(len < 0){
                        status = OBJVChannelStatusError;
                        break;
                    }
                    
                    httpChannel->write.off += len;
                    
                    if(httpChannel->write.off == httpChannel->write.mbuf.length){
                        
                        objv_log("\n%s\n",objv_mbuf_str(& httpChannel->write.mbuf));
                        
                        httpChannel->contentType = CLHttpChannelContentTypeChunked | CLHttpChannelContentTypeGzip;
                        httpChannel->read.state = -1;
                        channel->mode = CLChannelModeRead | CLChannelModeWrite;
                        
                        objv_mbuf_clear(& httpChannel->write.mbuf);
                        httpChannel->write.off = 0;
                        
                        status = OBJVChannelStatusOK;
                        
                        break;
                    }
                }
                
            }
            
        }
        
    }
    
    return status;
}

static OBJVChannelStatus CLHttpChannelMethodDisconnect(objv_class_t * clazz,CLChannel * channel){
    
    OBJVChannelStatus status = OBJVChannelStatusError;
    CLHttpChannel * httpChannel = (CLHttpChannel *) channel;
    
    if(channel->oChannel){
        status = objv_channel_disconnect(channel->oChannel->base.isa, channel->oChannel);
    }
    
    if(status == OBJVChannelStatusNone){
        
        objv_mbuf_clear( & httpChannel->read.mbuf);
        objv_mbuf_clear( & httpChannel->read.data);
        objv_mbuf_clear( & httpChannel->write.mbuf);
        httpChannel->read.dataLength = httpChannel->read.off = 0;
        httpChannel->read.state = 0;
        httpChannel->write.off = 0;
        httpChannel->write.state = 0;
        
        httpChannel->contentType = CLHttpChannelContentTypeNone;
        channel->mode = CLChannelModeNone;
        
        OBJVHTTPRequestReset(& httpChannel->httpRequest);
        OBJVHTTPResponseReset(& httpChannel->httpResponse);

    }
    
    return status;
}


OBJVChannelStatus CLHttpChannelUnpackageTask(objv_zone_t * zone, CLTask ** task,objv_class_t ** taskType,objv_string_t ** target,objv_mbuf_t * data,objv_mbuf_t * decode,CLHttpChannelContentType contentType){
    
    if(contentType & CLHttpChannelContentTypeChunked){
        
        OBJVHttpRequest request;
        OBJVHttpHeader * h;
        objv_class_t * tType = NULL;
        objv_class_t * tClass = NULL;
        CLTask * t;
        size_t length;
        char * p = data->data;
        
        if(contentType & CLHttpChannelContentTypeGzip){
            
            objv_mbuf_clear(decode);
            
            objv_gzip_decode(data->data, data->length, decode);
            
            data = decode;
            
            p = data->data;
        }
        
        OBJVChannelStatus status = OBJVChannelStatusNone;
        
        OBJVHTTPRequestInit(& request);
        
        request.state.state = OBJVHttpRequestStateHeaderKey;
        
        if(OBJVHTTPRequestRead(& request, 0,  data->length, data->data) == OBJVHttpRequestStateOK){
            
            status = OBJVChannelStatusOK;
            
            while(1) {
            
                h = OBJVHttpHeadersGetHeader(& request.headers, "taskType",request.ofString);
                
                if(! h){
                    break;
                }
                
                p[h->value.location + h->value.length] = 0;
                
                tType = objv_class(objv_key( p + h->value.location));
            
                if(!tType){
                    objv_log("\nNot Found taskType %s\n",p + h->value.location);
                    break;
                }
                
                if(!objv_class_isKindOfClass(tType, OBJV_CLASS(CLTask))){
                    objv_log("\ntaskType %s not implements CLTask\n",tType->name->name);
                    break;
                }
                
                h = OBJVHttpHeadersGetHeader(& request.headers, "taskClass",request.ofString);
                
                if(! h){
                    break;
                }
                
                p[h->value.location + h->value.length] = 0;
                
                tClass = objv_class(objv_key( p + h->value.location));
                
                if(!tClass){
                    objv_log("\nNot Found taskClass %s\n",p + h->value.location);
                    break;
                }
                
                if(!objv_class_isKindOfClass(tClass, OBJV_CLASS(CLTask))){
                    objv_log("\ntaskClass %s not implements CLTask\n",tClass->name->name);
                    break;
                }
                
                t = (CLTask *) objv_object_new(zone, tClass,NULL);
                
                h = OBJVHttpHeadersGetHeader(& request.headers, "identifier",request.ofString);
                
                if(! h){
                    break;
                }
                
                p[h->value.location + h->value.length] = 0;
                
                t->identifier = atoll(p + h->value.location);
                
                h = OBJVHttpHeadersGetHeader(& request.headers, "replyIdentifier",request.ofString);
                
                if(! h){
                    break;
                }
                
                p[h->value.location + h->value.length] = 0;
                
                t->replyIdentifier = atoll(p + h->value.location);
                
                h = OBJVHttpHeadersGetHeader(& request.headers, "source",request.ofString);
                
                if(! h){
                    break;
                }
                
                p[h->value.location + h->value.length] = 0;
                
                t->source = objv_string_alloc(zone, p + h->value.location);
                
                h = OBJVHttpHeadersGetHeader(& request.headers, "target",request.ofString);
                
                if(h && target){
                    
                    p[h->value.location + h->value.length] = 0;
                    * target = objv_string_new(zone, p + h->value.location);
                    
                }
                
                h = OBJVHttpHeadersGetHeader(& request.headers, "Content-Type",request.ofString);
                
                if(h){
                    
                    t->contentType = objv_string_alloc_with_length(zone, p + h->value.location , h->value.length);
                    
                    h = OBJVHttpHeadersGetHeader(& request.headers, "Content-Length",request.ofString);
                    
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

OBJVChannelStatus CLHttpChannelPackageTask(objv_zone_t * zone, CLTask * task,objv_class_t * taskType,objv_string_t * target,objv_mbuf_t * mbuf,objv_mbuf_t * encode,CLHttpChannelContentType contentType){
    
    if( contentType & CLHttpChannelContentTypeChunked ){
        
        
        size_t off = mbuf->length;
        
        objv_mbuf_extend(mbuf, mbuf->length + 10);
        
        objv_mbuf_format(mbuf, "%08lx\r\n", 0);
        
        objv_mbuf_clear(encode);
        
        objv_mbuf_format(encode, "taskType: %s\r\n",taskType->name->name);
        objv_mbuf_format(encode, "taskClass: %s\r\n",task->base.isa->name->name);
        objv_mbuf_format(encode, "identifier: %lld\r\n",task->identifier);
        objv_mbuf_format(encode, "replyIdentifier: %lld\r\n",task->replyIdentifier);
        objv_mbuf_format(encode, "source: %s\r\n",task->source->UTF8String);
        
        if(target){
            objv_mbuf_format(encode, "target: %s\r\n",target->UTF8String);
        }
        
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
                        objv_mbuf_format(encode, "%s: ",prop->name->name);
                        objv_url_encode_mbuf(zone, v->UTF8String, encode);
                        objv_mbuf_append(encode, "\r\n", 2);
                    }
                    
                    propCount --;
                    prop ++;
                }
                
                
                clazz = clazz->superClass;
            }
        }
        
        if(task->contentType){
            
            objv_mbuf_format(encode, "Content-Type: %s\r\n", task->contentType->UTF8String);
            objv_mbuf_format(encode, "Content-Length: %lu\r\n",task->content.length);
            
            objv_mbuf_append(encode, "\r\n", 2);
            
            objv_mbuf_append(encode, task->content.data, task->content.length);
            
        }
        else{
            objv_mbuf_append(encode, "\r\n", 2);
        }
        
        if(contentType & CLHttpChannelContentTypeGzip){
            
            objv_gzip_encode(encode->data, encode->length, mbuf);
            
        }
        else{
            objv_mbuf_append( mbuf, encode->data, encode->length);
        }
        
        snprintf((char *) mbuf->data + off, 10,"%08lx\r\n",mbuf->length - 10 - off);
        
        * ((char *) mbuf->data + off + 9) = '\n';
        
        objv_mbuf_append(mbuf, "\r\n", 2);
    
        return OBJVChannelStatusNone;
    }
    
    return OBJVChannelStatusError;
}


static OBJVChannelStatus CLHttpChannelMethodReadTask(objv_class_t * clazz,CLChannel * channel,CLTask ** task,objv_class_t ** taskType,objv_string_t ** target,objv_timeinval_t timeout){
    
    
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
            
            if(httpChannel->read.state == -1){
                
                while(1){
                    
                    OBJVHTTPResponseRead(& httpChannel->httpResponse, httpChannel->read.off, httpChannel->read.mbuf.length - httpChannel->read.off, httpChannel->read.mbuf.data);
                 
                    httpChannel->read.off = httpChannel->httpResponse.length;
                    
                    if(httpChannel->httpResponse.state.state == OBJVHttpRequestStateFail){
                        status = OBJVChannelStatusError;
                        break;
                    }
                    
                    if(httpChannel->httpResponse.state.state == OBJVHttpRequestStateOK){
                        status = OBJVChannelStatusOK;
                        httpChannel->read.off = httpChannel->httpResponse.length;
                        break;
                    }
                    
                    objv_mbuf_extend(& httpChannel->read.mbuf, httpChannel->read.mbuf.length + 512);
                    
                    status = objv_channel_canRead(oChannel->base.isa, oChannel, timeout);
                    
                    if(status == OBJVChannelStatusError){
                        return status;
                    }
                    else if(status == OBJVChannelStatusNone){
                        continue;
                    }
                    
                    len = objv_channel_read(oChannel->base.isa, oChannel, (char *) httpChannel->read.mbuf.data + httpChannel->read.mbuf.length
                                            , httpChannel->read.mbuf.size - httpChannel->read.mbuf.length);
                    
                    if(len < 0){
                        return OBJVChannelStatusError;
                    }
                    else if(len == 0){
                        continue;
                    }
                    
                }
                
                if(httpChannel->httpResponse.state.state == OBJVHttpRequestStateOK){
                    status = OBJVChannelStatusOK;
                    httpChannel->read.state = 0;
                    httpChannel->read.dataLength = 0;
                    break;
                }
                else{
                    status = OBJVChannelStatusError;
                    break;
                }
            }
            else if(httpChannel->read.state == 0){
            
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
                    memcpy(httpChannel->read.data.data, httpChannel->read.mbuf.data + httpChannel->read.off, len);
                    httpChannel->read.off += len;
                    httpChannel->read.data.length += len;
                }
                
                if(httpChannel->read.dataLength == httpChannel->read.data.length){
                    
                    status = CLHttpChannelUnpackageTask(zone,task,taskType,target, & httpChannel->read.data,& httpChannel->read.decode,httpChannel->contentType);
                    
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
                    httpChannel->read.dataLength = 0;
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

static void CLHttpChannelMethodPostTask(objv_class_t * clazz,CLChannel * channel,CLTask * task,objv_class_t * taskType,objv_string_t * target){
    
    CLHttpChannel * httpChannel = (CLHttpChannel *) channel;
    
    if( (httpChannel->base.mode & CLChannelModeWrite) && task && taskType ){
        
        objv_zone_t * zone = httpChannel->base.base.zone;
        
        CLHttpChannelPostTask * t = (CLHttpChannelPostTask *) objv_zone_malloc(zone, sizeof(CLHttpChannelPostTask));
        
        objv_zone_memzero(zone, t, sizeof(CLHttpChannelPostTask));
        
        t->task = (CLTask *) objv_object_retain((objv_object_t *) task);
        t->taskType = taskType;
        t->target = (objv_string_t *) objv_object_retain((objv_object_t *) target);
        
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
                
                CLHttpChannelPackageTask(zone,task->task,task->taskType,task->target,& httpChannel->write.mbuf,& httpChannel->write.encode, CLHttpChannelContentTypeChunked | CLHttpChannelContentTypeGzip);
                
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
#ifdef DEBUG    
                    objv_log("\n%s\n",objv_mbuf_str(& httpChannel->write.mbuf));
#endif
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
            
            if(status == OBJVChannelStatusNone){
                status = OBJVChannelStatusOK;
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


