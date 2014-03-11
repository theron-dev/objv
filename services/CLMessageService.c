//
//  CLMessageService.c
//  objv
//
//  Created by zhang hailong on 14-3-11.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv.h"
#include "objv_cloud.h"
#include "CLMessageService.h"
#include "objv_value.h"
#include "objv_autorelease.h"
#include "CLMessageTask.h"
#include "objv_db_mysql.h"
#include "CLMessageObject.h"

OBJV_KEY_IMP(CLMessageService)

static void CLMessageServiceDealloc(objv_class_t * clazz, objv_object_t * object) {
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
}

static objv_boolean_t CLMessageServicetHandleTask (objv_class_t * clazz,CLService * service,CLContext * ctx,objv_class_t * taskType,CLTask * task){
    
    if(objv_class_isKindOfClass(taskType,OBJV_CLASS(CLMessageTask))){
        
        objv_zone_t * zone = service->base.zone;
        objv_object_t * cfg = service->config;
        objv_object_t * dbConfig = objv_object_objectValueForKey(cfg, (objv_object_t *) objv_string_new_nocopy(zone, "db"), NULL);
        objv_string_t * url = objv_object_stringValueForKey(dbConfig, (objv_object_t *) objv_string_new_nocopy(zone, "url"), NULL);
        objv_db_t * db = NULL;
        CLMessageTask * messageTask = (CLMessageTask *) task;
        
        if(url && objv_string_hasPrefix(url->UTF8String, "mysql://")){
            
            db = (objv_db_t *) objv_db_mysql_alloc(zone, objv_url_new(zone, url->UTF8String)
                                     , objv_object_stringValueForKey(dbConfig, (objv_object_t *) objv_string_new_nocopy(zone, "user"), NULL)
                                     , objv_object_stringValueForKey(dbConfig, (objv_object_t *) objv_string_new_nocopy(zone, "password"), NULL));
            
            objv_db_object_regClass(db->base.isa, db, OBJV_CLASS(CLMessageObject));
            
        }
        
        if(db) {
        
            CLMessageObject * dataObject = (CLMessageObject *) objv_object_new(zone, OBJV_CLASS(CLMessageObject),NULL);
            
            dataObject->fuid = messageTask->uid;
            dataObject->uid = messageTask->tuid;
            dataObject->mettingId = messageTask->mettingId;
            dataObject->timestamp = messageTask->timestamp;
            dataObject->contentType = (objv_string_t *) objv_object_retain((objv_object_t *) messageTask->base.contentType);
            
            if(objv_object_isKindOfClass( (objv_object_t *) messageTask, OBJV_CLASS(CLMessageImageTask))){
                
            }
            else if(objv_object_isKindOfClass( (objv_object_t *) messageTask, OBJV_CLASS(CLMessageAudioTask))){
                
            }
            else if(objv_object_isKindOfClass( (objv_object_t *) messageTask, OBJV_CLASS(CLMessageVideoTask))){
                
            }
            else {
                dataObject->body = objv_string_alloc(zone, objv_mbuf_str(& messageTask->base.content));
            }
            
            objv_db_object_insert(db->base.isa, db, (objv_db_object_t *) dataObject);
            
            messageTask->messageId = dataObject->base.rowid;
            
        }
        
        CLContextSendTask(ctx, taskType, task, objv_string_new_format(zone, "com.9vteam.im.%llu.*",messageTask->tuid));
        
        objv_object_release((objv_object_t *) db);
        
    }
    
    return objv_false;
}

static void CLMessageServicetInitialize (struct _objv_class_t * clazz){
    objv_class_initialize(OBJV_CLASS(CLMessageImageTask));
    objv_class_initialize(OBJV_CLASS(CLMessageAudioTask));
    objv_class_initialize(OBJV_CLASS(CLMessageVideoTask));
    objv_class_initialize(OBJV_CLASS(CLMessageObject));
}


OBJV_CLASS_METHOD_IMP_BEGIN(CLMessageService)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", CLMessageServiceDealloc)

OBJV_CLASS_METHOD_IMP(handleTask, "b(@,*,@)", CLMessageServicetHandleTask)

OBJV_CLASS_METHOD_IMP_END(CLMessageService)

OBJV_CLASS_IMP_M_I(CLMessageService, OBJV_CLASS(CLService), CLMessageService,CLMessageServicetInitialize)

