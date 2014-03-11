//
//  CLMessageObject.h
//  objv
//
//  Created by zhang hailong on 14-3-11.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_CLMessageObject_h
#define objv_CLMessageObject_h


#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_db.h"
    
    enum {
        CLMessageObjectStateNone,CLMessageObjectStateRead
    };
    
    typedef int CLMessageObjectState;
    
    typedef struct _CLMessageObject {
        objv_db_object_t base;
        unsigned long long uid;
        unsigned long long fuid;
        unsigned long long mettingId;
        objv_string_t * contentType;
        objv_string_t * body;
        objv_string_t * resourceURI;
        unsigned int width;
        unsigned int height;
        double duration;
        size_t length;
        CLMessageObjectState state;
        objv_timeinval_t timestamp;
    } CLMessageObject;
    
    
    OBJV_KEY_DEC(CLMessageObject)
    
    OBJV_CLASS_DEC(CLMessageObject)
    
    OBJV_KEY_DEC(uid)
    OBJV_KEY_DEC(fuid)
    OBJV_KEY_DEC(mettingId)
    OBJV_KEY_DEC(contentType)
    OBJV_KEY_DEC(body)
    OBJV_KEY_DEC(resourceURI)
    OBJV_KEY_DEC(width)
    OBJV_KEY_DEC(height)
    OBJV_KEY_DEC(duration)
    OBJV_KEY_DEC(length)
    OBJV_KEY_DEC(state)
    OBJV_KEY_DEC(timestamp)
    
#ifdef __cplusplus
}
#endif



#endif
