//
//  objv_channel.c
//  objv
//
//  Created by zhang hailong on 14-2-10.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv.h"
#include "objv_channel.h"


OBJV_KEY_IMP(Channel)

OBJV_KEY_IMP(connect)

OBJV_KEY_IMP(canRead)

OBJV_KEY_IMP(read)

OBJV_KEY_IMP(canWrite)

OBJV_KEY_IMP(write)

OBJV_KEY_IMP(disconnect)

static void objv_channel_dealloc(objv_class_t * clazz,objv_object_t * object){

    objv_channel_t * channel = (objv_channel_t *) object;
    
    objv_object_release((objv_object_t *) channel->exception);
    
    if(clazz->superClass){
        
        objv_object_dealloc(clazz->superClass, object);
    }
    
}

OBJV_CLASS_METHOD_IMP_BEGIN(Channel)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", objv_channel_dealloc)

OBJV_CLASS_METHOD_IMP_END(Channel)

OBJV_CLASS_IMP_M(Channel, OBJV_CLASS(Object), objv_channel_t)


OBJVChannelStatus objv_channel_connect(objv_class_t * clazz,objv_channel_t * channel,objv_timeinval_t timeout){

    if(channel && ! channel->connected){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(connect))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_channel_connect_t) method->impl)(c,channel,timeout);
        }
        
        return channel->connected ? OBJVChannelStatusOK : OBJVChannelStatusError;
    }
    
    return channel && channel->connected ? OBJVChannelStatusOK : OBJVChannelStatusError;
}

OBJVChannelStatus objv_channel_canRead(objv_class_t * clazz,objv_channel_t * channel,objv_timeinval_t timeout){
    
    if(channel && channel->connected){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(canRead))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_channel_canRead_t) method->impl)(c,channel,timeout);
        }
        
        return OBJVChannelStatusError;
    }
    
    return OBJVChannelStatusError;
}


ssize_t objv_channel_read(objv_class_t * clazz,objv_channel_t * channel,void * data,size_t length){
    
    if(channel && channel->connected){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(read))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_channel_read_t) method->impl)(c,channel,data,length);
        }
        
        return OBJVChannelStatusError;
    }
    
    return OBJVChannelStatusError;
}

OBJVChannelStatus objv_channel_canWrite(objv_class_t * clazz,objv_channel_t * channel,objv_timeinval_t timeout){
    
    if(channel && channel->connected){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(canWrite))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_channel_canWrite_t) method->impl)(c,channel,timeout);
        }
        
        return OBJVChannelStatusError;
    }
    
    return OBJVChannelStatusError;
}

ssize_t objv_channel_write(objv_class_t * clazz,objv_channel_t * channel,void * data,size_t length){
    
    if(channel && channel->connected){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(write))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_channel_write_t) method->impl)(c,channel,data,length);
        }
        
        return OBJVChannelStatusError;
    }
    
    return OBJVChannelStatusError;
}

void objv_channel_disconnect(objv_class_t * clazz,objv_channel_t * channel){
    
    if(channel && channel->connected){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(connect))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            (* (objv_channel_disconnect_t) method->impl)(c,channel);
        }

    }

}

void objv_channel_setException(objv_channel_t * channel,objv_exception_t * exception){
    if(channel && channel->exception != exception){
        objv_object_retain((objv_object_t *) exception);
        objv_object_release((objv_object_t *) channel->exception);
        
        channel->exception = exception;
    }
}

void objv_channel_setExceptionFormat(objv_channel_t * channel,int code,const char * format,...){
    if(channel){
        va_list ap;
        
        va_start(ap, format);
        
        objv_channel_setException(channel, objv_exception_newv(channel->base.zone, code, format, ap));
        
        va_end(ap);
    }
}
