//
//  CLMonitorService.c
//  objv
//
//  Created by zhang hailong on 14-3-10.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#include "objv_os.h"
#include "objv.h"
#include "objv_cloud.h"
#include "CLMonitorService.h"
#include "objv_mail.h"
#include "objv_value.h"
#include "objv_autorelease.h"

OBJV_KEY_IMP(CLMonitorService)

static void CLMonitorServiceDealloc(objv_class_t * clazz, objv_object_t * object) {

    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
}

static objv_boolean_t CLMonitorServicetHandleTask (objv_class_t * clazz,CLService * service,CLContext * ctx,objv_class_t * taskType,CLTask * task){
    
    if(OBJV_CLASS(CLContextChildChangedTask) == taskType){
        
        CLContextChildChangedTask * t = (CLContextChildChangedTask *) task;
        objv_zone_t * zone = service->base.zone;
        objv_object_t * cfg = service->config;
        objv_object_t * mail = objv_object_objectValueForKey(cfg,(objv_object_t *) objv_string_new_nocopy(zone, "mail"), NULL);
        objv_string_t * smtp = objv_object_stringValueForKey(mail, (objv_object_t *) objv_string_new_nocopy(zone, "smtp"), NULL);
        objv_string_t * to = objv_object_stringValueForKey(mail, (objv_object_t *) objv_string_new_nocopy(zone, "to"), NULL);
        
        if(mail && smtp && to){
            {
                
                objv_mail_t * m = (objv_mail_t *) objv_object_new(zone, OBJV_CLASS(Mail),NULL);
                
                objv_mail_setSmtp(m, objv_url_new(zone, smtp->UTF8String)
                                  , objv_object_stringValueForKey(mail, (objv_object_t *) objv_string_new_nocopy(zone, "user"), NULL)
                                  , objv_object_stringValueForKey(mail, (objv_object_t *) objv_string_new_nocopy(zone, "password"), NULL));
                
                objv_string_t * title = NULL;
                
                if(strcmp(t->state->UTF8String, "add") == 0){
                    title = objv_string_new_format(zone, "服务上线 %s",t->domain->UTF8String);
                }
                else if(strcmp(t->state->UTF8String, "remove") == 0){
                    title = objv_string_new_format(zone, "服务下线 %s",t->domain->UTF8String);
                }
                else {
                    title = objv_string_new_format(zone, "服务[%s] %s",t->state->UTF8String,t->domain->UTF8String);
                }
                
                objv_mail_send(m, to, title, title, 0.2);
                
            }
        }
        
        
    }
    
    return objv_false;
}

static void CLMonitorServicetInitialize (struct _objv_class_t * clazz){
    objv_class_initialize(OBJV_CLASS(CLContextChildChangedTask));
}


OBJV_CLASS_METHOD_IMP_BEGIN(CLMonitorService)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", CLMonitorServiceDealloc)

OBJV_CLASS_METHOD_IMP(handleTask, "b(@,*,@)", CLMonitorServicetHandleTask)

OBJV_CLASS_METHOD_IMP_END(CLMonitorService)

OBJV_CLASS_IMP_M_I(CLMonitorService, OBJV_CLASS(CLService), CLMonitorService,CLMonitorServicetInitialize)
