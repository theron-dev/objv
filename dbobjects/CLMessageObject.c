//
//  CLMessageObject.c
//  objv
//
//  Created by zhang hailong on 14-3-11.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv.h"
#include "CLMessageObject.h"


OBJV_KEY_IMP(CLMessageObject)

OBJV_KEY_IMP(uid)
OBJV_KEY_IMP(fuid)
OBJV_KEY_IMP(mettingId)
OBJV_KEY_IMP(contentType)
OBJV_KEY_IMP(body)
OBJV_KEY_IMP(resourceURI)
OBJV_KEY_IMP(width)
OBJV_KEY_IMP(height)
OBJV_KEY_IMP(duration)

static void CLMessageObjectMethodDealloc(objv_class_t * clazz, objv_object_t * object) {

    CLMessageObject * dataObject = (CLMessageObject *) object;
    
    objv_object_release((objv_object_t *) dataObject->contentType);
    objv_object_release((objv_object_t *) dataObject->body);
    objv_object_release((objv_object_t *) dataObject->resourceURI);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
}

static unsigned long long CLMessageObjectGetUid(objv_class_t * clazz, CLMessageObject * dataObject){
    return dataObject->uid;
}

static void CLMessageObjectSetUid(objv_class_t * clazz, CLMessageObject * dataObject,unsigned long long uid){
    dataObject->uid = uid;
}

static unsigned long long CLMessageObjectGetFuid(objv_class_t * clazz, CLMessageObject * dataObject){
    return dataObject->fuid;
}

static void CLMessageObjectSetFuid(objv_class_t * clazz, CLMessageObject * dataObject,unsigned long long fuid){
    dataObject->fuid = fuid;
}

static unsigned long long CLMessageObjectGetMettingId(objv_class_t * clazz, CLMessageObject * dataObject){
    return dataObject->mettingId;
}

static void CLMessageObjectSetMettingId(objv_class_t * clazz, CLMessageObject * dataObject,unsigned long long mettingId){
    dataObject->mettingId = mettingId;
}

static objv_string_t * CLMessageObjectGetContentType(objv_class_t * clazz, CLMessageObject * dataObject){
    return dataObject->contentType;
}

static void CLMessageObjectSetContentType(objv_class_t * clazz, CLMessageObject * dataObject,objv_string_t * contentType){
    if(dataObject->contentType != contentType){
        objv_object_retain((objv_object_t *) contentType);
        objv_object_release((objv_object_t *) dataObject->contentType);
        dataObject->contentType = contentType;
    }
}

static objv_string_t * CLMessageObjectGetBody(objv_class_t * clazz, CLMessageObject * dataObject){
    return dataObject->body;
}

static void CLMessageObjectSetBody(objv_class_t * clazz, CLMessageObject * dataObject,objv_string_t * body){
    if(dataObject->body != body){
        objv_object_retain((objv_object_t *) body);
        objv_object_release((objv_object_t *) dataObject->body);
        dataObject->body = body;
    }
}

static objv_string_t * CLMessageObjectGetResourceURI(objv_class_t * clazz, CLMessageObject * dataObject){
    return dataObject->resourceURI;
}

static void CLMessageObjectSetResourceURI(objv_class_t * clazz, CLMessageObject * dataObject,objv_string_t * resourceURI){
    if(dataObject->resourceURI != resourceURI){
        objv_object_retain((objv_object_t *) resourceURI);
        objv_object_release((objv_object_t *) dataObject->resourceURI);
        dataObject->resourceURI = resourceURI;
    }
}

static unsigned int CLMessageObjectGetWidth(objv_class_t * clazz, CLMessageObject * dataObject){
    return dataObject->width;
}

static void CLMessageObjectSetWidth(objv_class_t * clazz, CLMessageObject * dataObject,unsigned int width){
    dataObject->width = width;
}

static unsigned int CLMessageObjectGetHeight(objv_class_t * clazz, CLMessageObject * dataObject){
    return dataObject->height;
}

static void CLMessageObjectSetHeight(objv_class_t * clazz, CLMessageObject * dataObject,unsigned int height){
    dataObject->height = height;
}

static double CLMessageObjectGetDuration(objv_class_t * clazz, CLMessageObject * dataObject){
    return dataObject->duration;
}

static void CLMessageObjectSetDuration(objv_class_t * clazz, CLMessageObject * dataObject,double duration){
    dataObject->duration = duration;
}

static unsigned long CLMessageObjectGetLength(objv_class_t * clazz, CLMessageObject * dataObject){
    return dataObject->length;
}

static void CLMessageObjectSetLength(objv_class_t * clazz, CLMessageObject * dataObject,unsigned long length){
    dataObject->length = length;
}

static int CLMessageObjectGetState(objv_class_t * clazz, CLMessageObject * dataObject){
    return dataObject->state;
}

static void CLMessageObjectSetState(objv_class_t * clazz, CLMessageObject * dataObject,int state){
    dataObject->state = state;
}

static double CLMessageObjectGetTimestamp(objv_class_t * clazz, CLMessageObject * dataObject){
    return dataObject->timestamp;
}

static void CLMessageObjectSetTimestamp(objv_class_t * clazz, CLMessageObject * dataObject,double timestamp){
    dataObject->timestamp = timestamp;
}


OBJV_CLASS_METHOD_IMP_BEGIN(CLMessageObject)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", CLMessageObjectMethodDealloc)

OBJV_CLASS_METHOD_IMP_END(CLMessageObject)



OBJV_CLASS_PROPERTY_IMP_BEGIN(CLMessageObject)

OBJV_CLASS_PROPERTY_IMP(uid, ulongLong, CLMessageObjectGetUid, CLMessageObjectSetUid, objv_true)

OBJV_CLASS_PROPERTY_IMP(fuid, ulongLong, CLMessageObjectGetFuid, CLMessageObjectSetFuid, objv_true)

OBJV_CLASS_PROPERTY_IMP(mettingId, ulongLong, CLMessageObjectGetMettingId, CLMessageObjectSetMettingId, objv_true)

OBJV_CLASS_PROPERTY_IMP(contentType, object, CLMessageObjectGetContentType, CLMessageObjectSetContentType, objv_true)

OBJV_CLASS_PROPERTY_IMP(body, object, CLMessageObjectGetBody, CLMessageObjectSetBody, objv_true)

OBJV_CLASS_PROPERTY_IMP(resourceURI, object, CLMessageObjectGetResourceURI, CLMessageObjectSetResourceURI, objv_true)

OBJV_CLASS_PROPERTY_IMP(width, uint, CLMessageObjectGetWidth, CLMessageObjectSetWidth, objv_true)

OBJV_CLASS_PROPERTY_IMP(height, uint, CLMessageObjectGetHeight, CLMessageObjectSetHeight, objv_true)

OBJV_CLASS_PROPERTY_IMP(duration, double, CLMessageObjectGetDuration, CLMessageObjectSetDuration, objv_true)

OBJV_CLASS_PROPERTY_IMP(length, ulong, CLMessageObjectGetLength, CLMessageObjectSetLength, objv_true)

OBJV_CLASS_PROPERTY_IMP(state, int, CLMessageObjectGetState, CLMessageObjectSetState, objv_true)

OBJV_CLASS_PROPERTY_IMP(timestamp, int, CLMessageObjectGetTimestamp, CLMessageObjectSetTimestamp, objv_true)


OBJV_CLASS_PROPERTY_IMP_END(CLMessageObject)


OBJV_CLASS_IMP_P_M(CLMessageObject, OBJV_CLASS(DBObject), CLMessageObject)


