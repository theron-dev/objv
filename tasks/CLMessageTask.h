//
//  CLMessageTask.h
//  objv
//
//  Created by zhang hailong on 14-3-11.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_CLMessageTask_h
#define objv_CLMessageTask_h



#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_cloud.h"
    
    typedef struct _CLMessageTask {
        CLTask base;
        unsigned long long messageId;
        unsigned long long uid;
        unsigned long long tuid;
        unsigned long long mettingId;
        objv_timeinval_t timestamp;
    } CLMessageTask;
    
    OBJV_CLASS_DEC(CLMessageTask)
    
    OBJV_KEY_DEC(CLMessageTask)
    OBJV_KEY_DEC(timestamp)
    OBJV_KEY_DEC(messageId)
    OBJV_KEY_DEC(uid)
    OBJV_KEY_DEC(tuid)
    OBJV_KEY_DEC(mettingId)
    
    typedef struct _CLMessageImageTask {
        CLMessageTask base;
        unsigned int width;
        unsigned int height;
    } CLMessageImageTask;
    
    OBJV_CLASS_DEC(CLMessageImageTask)
    
    OBJV_KEY_DEC(CLMessageImageTask)
    OBJV_KEY_DEC(width)
    OBJV_KEY_DEC(height)
    
    typedef struct _CLMessageAudioTask {
        CLMessageTask base;
        double duration;
    } CLMessageAudioTask;
    
    OBJV_CLASS_DEC(CLMessageAudioTask)
    
    OBJV_KEY_DEC(CLMessageAudioTask)
    OBJV_KEY_DEC(duration)

    
    typedef struct _CLMessageVideoTask {
        CLMessageTask base;
        size_t offset;
        size_t length;
        double duration;
    } CLMessageVideoTask;
    
    OBJV_CLASS_DEC(CLMessageVideoTask)
    
    OBJV_KEY_DEC(CLMessageVideoTask)
    
    OBJV_KEY_DEC(offset)
    OBJV_KEY_DEC(length)
    OBJV_KEY_DEC(duration)
    
#ifdef __cplusplus
}
#endif



#endif
