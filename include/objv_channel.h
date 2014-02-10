//
//  objv_channel.h
//  objv
//
//  Created by zhang hailong on 14-2-10.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_channel_h
#define objv_objv_channel_h


#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_exception.h"
 
    typedef enum _OBJVChannelStatus{
        OBJVChannelStatusNone = 0,
        OBJVChannelStatusOK = 1,
        OBJVChannelStatusError = -1,
    } OBJVChannelStatus;
    
    typedef struct _objv_channel_t {
        objv_object_t base;
        objv_boolean_t READONLY connected;
        objv_exception_t * READONLY exception;
    } objv_channel_t;
    
    OBJV_KEY_DEC(Channel)
    
    OBJV_CLASS_DEC(Channel)
    
    OBJV_KEY_DEC(connect)
    
    OBJV_KEY_DEC(canRead)
    
    OBJV_KEY_DEC(read)
    
    OBJV_KEY_DEC(canWrite)
    
    OBJV_KEY_DEC(write)
    
    OBJV_KEY_DEC(disconnect)
    
    typedef OBJVChannelStatus (* objv_channel_connect_t )(objv_class_t * clazz,objv_channel_t * channel,objv_timeinval_t timeout);
    
    OBJVChannelStatus objv_channel_connect(objv_class_t * clazz, objv_channel_t * channel,objv_timeinval_t timeout);
    
    
    typedef OBJVChannelStatus (* objv_channel_canRead_t) (objv_class_t * clazz,objv_channel_t * channel,objv_timeinval_t timeout);
    
    OBJVChannelStatus objv_channel_canRead(objv_class_t * clazz,objv_channel_t * channel,objv_timeinval_t timeout);
    
    
    typedef ssize_t (* objv_channel_read_t) (objv_class_t * clazz,objv_channel_t * channel,void * data,size_t length);
    
    ssize_t objv_channel_read(objv_class_t * clazz,objv_channel_t * channel,void * data,size_t length);
    
    
    typedef OBJVChannelStatus (* objv_channel_canWrite_t) (objv_class_t * clazz,objv_channel_t * channel,objv_timeinval_t timeout);
    
    OBJVChannelStatus objv_channel_canWrite(objv_class_t * clazz,objv_channel_t * channel,objv_timeinval_t timeout);
    
    
    typedef ssize_t (* objv_channel_write_t) (objv_class_t * clazz,objv_channel_t * channel,void * data,size_t length);
    
    ssize_t objv_channel_write(objv_class_t * clazz,objv_channel_t * channel,void * data,size_t length);
    
    typedef void (* objv_channel_disconnect_t) (objv_class_t * clazz,objv_channel_t * channel);
    
    void objv_channel_disconnect(objv_class_t * clazz,objv_channel_t * channel);
    
    void objv_channel_setException(objv_channel_t * channel,objv_exception_t * exception);
    
    void objv_channel_setExceptionFormat(objv_channel_t * channel,int code,const char * format,...);
    
#ifdef __cplusplus
}
#endif



#endif
