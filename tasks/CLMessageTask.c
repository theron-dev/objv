//
//  CLMessageTask.c
//  objv
//
//  Created by zhang hailong on 14-3-11.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#include "objv_os.h"
#include "objv.h"
#include "CLMessageTask.h"


OBJV_KEY_IMP(CLMessageTask)
OBJV_KEY_IMP(timestamp)
OBJV_KEY_IMP(messageId)
OBJV_KEY_IMP(tuid)

static objv_timeinval_t CLMessageTaskTimestamp(objv_class_t * clazz, CLMessageTask * task){
    return task->timestamp;
}

static void CLMessageTaskSetTimestamp(objv_class_t * clazz, CLMessageTask * task,objv_timeinval_t timestamp){
    task->timestamp = timestamp;
}

static unsigned long long CLMessageTaskMessageId(objv_class_t * clazz, CLMessageTask * task){
    return task->messageId;
}

static void CLMessageTaskSetMessageId(objv_class_t * clazz, CLMessageTask * task,unsigned long long messageId){
    task->messageId = messageId;
}

static unsigned long long CLMessageTaskUid(objv_class_t * clazz, CLMessageTask * task){
    return task->uid;
}

static void CLMessageTaskSetUid(objv_class_t * clazz, CLMessageTask * task,unsigned long long uid){
    task->uid = uid;
}

static unsigned long long CLMessageTaskTuid(objv_class_t * clazz, CLMessageTask * task){
    return task->tuid;
}

static void CLMessageTaskSetTuid(objv_class_t * clazz, CLMessageTask * task,unsigned long long tuid){
    task->tuid = tuid;
}

static unsigned long long CLMessageTaskMettingId(objv_class_t * clazz, CLMessageTask * task){
    return task->mettingId;
}

static void CLMessageTaskSetMettingId(objv_class_t * clazz, CLMessageTask * task,unsigned long long mettingId){
    task->mettingId = mettingId;
}

OBJV_CLASS_PROPERTY_IMP_BEGIN(CLMessageTask)

OBJV_CLASS_PROPERTY_IMP(timestamp, double, CLMessageTaskTimestamp, CLMessageTaskSetTimestamp, objv_true)
OBJV_CLASS_PROPERTY_IMP(messageId, ulongLong, CLMessageTaskMessageId, CLMessageTaskSetMessageId, objv_true)
OBJV_CLASS_PROPERTY_IMP(uid, ulongLong, CLMessageTaskUid, CLMessageTaskSetUid, objv_true)
OBJV_CLASS_PROPERTY_IMP(tuid, ulongLong, CLMessageTaskTuid, CLMessageTaskSetTuid, objv_true)
OBJV_CLASS_PROPERTY_IMP(mettingId, ulongLong, CLMessageTaskMettingId, CLMessageTaskSetMettingId, objv_true)

OBJV_CLASS_PROPERTY_IMP_END(CLMessageTask)

OBJV_CLASS_IMP_P(CLMessageTask, OBJV_CLASS(CLTask), CLMessageTask)


OBJV_KEY_IMP(CLMessageImageTask)


static unsigned int CLMessageImageTaskWidth(objv_class_t * clazz,CLMessageImageTask * task){
    return task->width;
}

static void CLMessageImageTaskSetWidth(objv_class_t * clazz,CLMessageImageTask * task, unsigned int width){
    task->width = width;
}

static unsigned int CLMessageImageTaskHeight(objv_class_t * clazz,CLMessageImageTask * task){
    return task->height;
}

static void CLMessageImageTaskSetHeight(objv_class_t * clazz,CLMessageImageTask * task, unsigned int height){
    task->height = height;
}

OBJV_CLASS_PROPERTY_IMP_BEGIN(CLMessageImageTask)

OBJV_CLASS_PROPERTY_IMP(width, uint, CLMessageImageTaskWidth, CLMessageImageTaskSetWidth, objv_true)
OBJV_CLASS_PROPERTY_IMP(height, uint, CLMessageImageTaskHeight, CLMessageImageTaskSetHeight, objv_true)

OBJV_CLASS_PROPERTY_IMP_END(CLMessageImageTask)

OBJV_CLASS_IMP_P(CLMessageImageTask, OBJV_CLASS(CLMessageTask), CLMessageImageTask)



OBJV_KEY_IMP(CLMessageAudioTask)

static double CLMessageAudioTaskDuration(objv_class_t * clazz,CLMessageAudioTask * task){
    return task->duration;
}

static void CLMessageAudioTaskSetDuration(objv_class_t * clazz,CLMessageAudioTask * task,double duration){
    task->duration = duration;
}

OBJV_CLASS_PROPERTY_IMP_BEGIN(CLMessageAudioTask)

OBJV_CLASS_PROPERTY_IMP(duration, double, CLMessageAudioTaskDuration, CLMessageAudioTaskSetDuration, objv_true)

OBJV_CLASS_PROPERTY_IMP_END(CLMessageAudioTask)

OBJV_CLASS_IMP_P(CLMessageAudioTask, OBJV_CLASS(CLMessageTask), CLMessageAudioTask)


OBJV_KEY_IMP(CLMessageVideoTask)
OBJV_KEY_IMP(uuid)
OBJV_KEY_IMP(offset)

static double CLMessageVideoTaskDuration(objv_class_t * clazz,CLMessageVideoTask * task){
    return task->duration;
}

static void CLMessageVideoTaskSetDuration(objv_class_t * clazz,CLMessageVideoTask * task,double duration){
    task->duration = duration;
}


static unsigned long CLMessageVideoTaskOffset(objv_class_t * clazz,CLMessageVideoTask * task){
    return task->offset;
}

static void CLMessageVideoTaskSetOffset(objv_class_t * clazz,CLMessageVideoTask * task,unsigned long offset){
    task->offset = offset;
}


static unsigned long CLMessageVideoTaskLength(objv_class_t * clazz,CLMessageVideoTask * task){
    return task->length;
}

static void CLMessageVideoTaskSetLength(objv_class_t * clazz,CLMessageVideoTask * task,unsigned long length){
    task->length = length;
}


OBJV_CLASS_PROPERTY_IMP_BEGIN(CLMessageVideoTask)

OBJV_CLASS_PROPERTY_IMP(duration, double, CLMessageVideoTaskDuration, CLMessageVideoTaskSetDuration, objv_true)

OBJV_CLASS_PROPERTY_IMP(offset, ulong, CLMessageVideoTaskOffset, CLMessageVideoTaskSetOffset, objv_true)

OBJV_CLASS_PROPERTY_IMP(length, ulong, CLMessageVideoTaskLength, CLMessageVideoTaskSetLength, objv_true)

OBJV_CLASS_PROPERTY_IMP_END(CLMessageVideoTask)

OBJV_CLASS_IMP_P(CLMessageVideoTask, OBJV_CLASS(CLMessageTask), CLMessageVideoTask)




