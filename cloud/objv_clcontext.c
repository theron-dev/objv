//
//  objv_clcontext.c
//  objv
//
//  Created by zhang hailong on 14-2-10.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv.h"
#include "objv_cloud.h"
#include "objv_clcontext.h"
#include "objv_value.h"
#include "objv_autorelease.h"

OBJV_KEY_IMP(CLServiceContext)

static void CLServiceContextMethodDealloc(objv_class_t * clazz,objv_object_t * object){

    CLServiceContext * ctx = (CLServiceContext *) object;
    
    objv_object_release((objv_object_t *) ctx->container);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
}

static void CLServiceContextMethodHandleTask(objv_class_t * clazz,CLServiceContext * ctx,objv_class_t * taskType,CLTask * task){
    
    if(ctx->container){
        
        CLServiceContainerHandleTask(ctx->container, (CLContext *) ctx, taskType, task);
        
    }
    
}

static void CLServiceContextMethodSendTask(objv_class_t * clazz,CLServiceContext * ctx,objv_class_t * taskType,CLTask * task){
    CLContextHandleTask((CLContext *) ctx, taskType, task);
}

OBJV_CLASS_METHOD_IMP_BEGIN(CLServiceContext)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", CLServiceContextMethodDealloc)

OBJV_CLASS_METHOD_IMP(handleTask, "v(*,@)", CLServiceContextMethodHandleTask)

OBJV_CLASS_METHOD_IMP(sendTask, "v(*,@)", CLServiceContextMethodSendTask)

OBJV_CLASS_METHOD_IMP_END(CLServiceContext)

OBJV_CLASS_IMP_M(CLServiceContext, OBJV_CLASS(CLContext), CLServiceContext)

void CLServiceContextSetContainer(CLServiceContext * ctx,CLServiceContainer * container){
    if(ctx && ctx->container != container){
        objv_object_retain((objv_object_t *) container);
        objv_object_release((objv_object_t *) ctx->container);
        ctx->container = container;
    }
}


OBJV_KEY_IMP(CLChannelContext);


static void CLChannelContextMethodDealloc(objv_class_t * clazz,objv_object_t * object){
    
    CLChannelContext * ctx = (CLChannelContext *) object;

    objv_dispatch_queue_cancelAllTasks(ctx->queue);
    
    objv_mutex_lock(& ctx->channels_mutex);
    
    objv_object_release((objv_object_t *) ctx->channels );
    
    ctx->channels = NULL;
    
    objv_mutex_unlock(& ctx->channels_mutex);
    
    objv_object_release((objv_object_t *) ctx->queue);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
}

static objv_object_t * CLChannelContextMethodInit(objv_class_t * clazz,objv_object_t * object,va_list ap){
    
    if(clazz->superClass){
        object = objv_object_initv(clazz->superClass, object, ap);
    }
    
    if(object){
        
        objv_zone_t * zone = object->zone;
        
        CLChannelContext * ctx = (CLChannelContext *) object;
        
        objv_mutex_init(& ctx->channels_mutex);
        
        objv_mutex_lock(& ctx->channels_mutex);
        
        ctx->channels = objv_array_alloc(zone, 4);
        
        objv_mutex_unlock(& ctx->channels_mutex);
        
        ctx->queue = objv_dispatch_queue_alloc(zone, "CLChannelContextQueue", 16);
        
    }
    
    return object;
}


static void  CLChannelContextMethodSendTaskFun (objv_class_t * clazz,CLChannelContext * ctx,objv_class_t * taskType,CLTask * task){
    
    if(task->source == (CLContext *) ctx){
        CLContextHandleTask((CLContext *) ctx, taskType, task);
    }
    else{
        
        objv_mutex_lock(& ctx->channels_mutex);
        
        if( ctx->channels && ctx->channels->length > 0){
            
            for(int i=0;i<ctx->channels->length;i++){
                CLChannel * channel = (CLChannel *) objv_array_objectAt(ctx->channels, i);
                CLChannelPostTask(channel->base.isa, channel, task, taskType);
            }
            
        }
        
        objv_mutex_unlock(& ctx->channels_mutex);
    }
}

static void CLChannelContextMethodSetConfig(objv_class_t * clazz,CLChannelContext * ctx,objv_object_t * config){
 
    if(clazz->superClass){
        CLContextSetConfig(clazz->superClass, (CLContext *) ctx, config);
    }

    ctx->keepAlive = objv_object_doubleValueForKey(config, (objv_object_t *) objv_string_new_nocopy(ctx->base.base.base.zone, "keepAlive"), 20);
    
}


OBJV_CLASS_METHOD_IMP_BEGIN(CLChannelContext)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", CLChannelContextMethodDealloc)

OBJV_CLASS_METHOD_IMP(init, "@(*)", CLChannelContextMethodInit)

OBJV_CLASS_METHOD_IMP(sendTask, "v(*,@)", CLChannelContextMethodSendTaskFun)

OBJV_CLASS_METHOD_IMP(setConfig, "v(@)", CLChannelContextMethodSetConfig)

OBJV_CLASS_METHOD_IMP_END(CLChannelContext)

OBJV_CLASS_IMP_M(CLChannelContext,OBJV_CLASS(CLContext),CLChannelContext)


typedef struct _CLChannelContextTask {
    objv_dispatch_task_t base;
    CLChannel * channel;
    CLChannelContext * ctx;
    objv_timeinval_t idleTimeinval;
    objv_timeinval_t keepAlive;
} CLChannelContextTask;

OBJV_KEY_DEC(CLChannelContextTask)

OBJV_CLASS_DEC(CLChannelContextTask)

OBJV_KEY_IMP(CLChannelContextTask)

static void CLChannelContextTaskRun(objv_class_t * clazz, CLChannelContextTask * task){
    
    OBJVChannelStatus status = OBJVChannelStatusOK;
    
    task->idleTimeinval += objv_timestamp() - task->base.start;
    
    if((status = CLChannelConnect(task->channel->base.isa, task->channel, 0.02)) == OBJVChannelStatusOK){
        
        if(task->channel->mode & CLChannelModeRead){
            
            CLTask * t = NULL;
            objv_class_t * tType = NULL;
            
            if((status = CLChannelReadTask(task->channel->base.isa,task->channel, & t, & tType, 0.02)) == OBJVChannelStatusOK){
                
                if(t && tType){
                    
                    CLContextHandleTask((CLContext *) task->ctx, tType, t);
                    
                    task->idleTimeinval = 0;
                }
                
            }
            
        }
        
        if(status != OBJVChannelStatusError && (task->channel->mode & CLChannelModeWrite)){
            status = CLChannelTick(task->channel->base.isa,task->channel, 0.02);
        }
     
        if(status == OBJVChannelStatusError){
            status = CLChannelDisconnect(task->channel->base.isa, task->channel);
        }
        else if(status == OBJVChannelStatusOK){
            task->idleTimeinval = 0;
        }
    }
    
    if(status == OBJVChannelStatusError || (task->keepAlive != 0.0 && task->idleTimeinval > task->keepAlive)){
        CLChannelContextRemoveChannel(task->ctx, task->channel);
    }
    else if(task->ctx) {
        task->base.delay = MIN(0.2,task->idleTimeinval);
        objv_dispatch_queue_addTask(task->ctx->queue, (objv_dispatch_task_t *) task);
    }
}

static void CLChannelContextTaskDealloc(objv_class_t * clazz, objv_object_t * object){
    
    CLChannelContextTask * task =(CLChannelContextTask *) object;
    
    objv_object_unweak((objv_object_t *)task->ctx, (objv_object_t **)& task->ctx);
    
    task->ctx = NULL;
    
    objv_object_release((objv_object_t *)task->channel);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
}

static objv_object_t * CLChannelContextTaskInit(objv_class_t * clazz, objv_object_t * object,va_list ap){
    
    if(clazz->superClass){
        object = objv_object_initv(clazz->superClass, object, ap);
    }
    
    if(object){
        
        CLChannelContextTask * task =(CLChannelContextTask *) object;
        
        objv_object_t * ctx = va_arg(ap, objv_object_t *);
        objv_object_t * channel = va_arg(ap, objv_object_t *);
        objv_object_t ** toObject = (objv_object_t **)& task->ctx;
        task->ctx = (CLChannelContext *) objv_object_weak(ctx, toObject);
        task->channel = (CLChannel *) objv_object_retain(channel);
        
    }
    
    return object;
}

OBJV_CLASS_METHOD_IMP_BEGIN(CLChannelContextTask)

OBJV_CLASS_METHOD_IMP(run, "v()", CLChannelContextTaskRun)
OBJV_CLASS_METHOD_IMP(dealloc, "v()", CLChannelContextTaskDealloc)
OBJV_CLASS_METHOD_IMP(init, "@(*)", CLChannelContextTaskInit)

OBJV_CLASS_METHOD_IMP_END(CLChannelContextTask)

OBJV_CLASS_IMP_M(CLChannelContextTask, OBJV_CLASS(DispatchTask), CLChannelContextTask)

void CLChannelContextAddChannel(CLChannelContext * ctx,CLChannel * channel){
    
    if(ctx && channel && ctx->queue){
        
        objv_zone_t * zone = ctx->base.base.base.zone;
        
        objv_mutex_lock(& ctx->channels_mutex);
        
        if(ctx->channels){
            objv_array_add(ctx->channels,(objv_object_t *) channel);
        }
        
        objv_mutex_unlock(& ctx->channels_mutex);

        CLChannelContextTask * task = (CLChannelContextTask *) objv_object_alloc(zone, OBJV_CLASS(CLChannelContextTask),ctx,channel,NULL);
        
        task->keepAlive = ctx->keepAlive;
        
        objv_dispatch_queue_addTask(ctx->queue, (objv_dispatch_task_t *) task);
        
        objv_object_release((objv_object_t *) task);
    }
}

void CLChannelContextRemoveChannel(CLChannelContext * ctx,CLChannel * channel){
    
    if(ctx && channel ){
        
        size_t channelCount = 0;
        
        objv_object_retain((objv_object_t *) ctx);
        
        CLChannelContextWillRemoveChannel(ctx->base.base.base.isa,ctx,channel);
        
        objv_mutex_lock(& ctx->channels_mutex);
        
        if(ctx->channels){
            
            objv_array_remove(ctx->channels,(objv_object_t *) channel);
        
            channelCount = ctx->channels->length;
        
        }
        
        objv_mutex_unlock(& ctx->channels_mutex);

        CLChannelContextDidRemoveChannel(ctx->base.base.base.isa,ctx);
        
        if(channelCount == 0 && ctx->allowRemovedFromParent && ctx->base.base.parent){
            
            CLContextRemoveChild(ctx->base.base.parent, (CLContext *) ctx);
            
        }
        
        objv_object_release((objv_object_t *) ctx);
    }
    
}

OBJV_KEY_IMP(willRemoveChannel)

OBJV_KEY_IMP(didRemoveChannel)

void CLChannelContextWillRemoveChannel(objv_class_t * clazz,CLChannelContext * ctx,CLChannel * channel){
    
    if(clazz && ctx){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(willRemoveChannel))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            (* (CLChannelContextMethodWillRemoveChannel) method->impl)(c,ctx,channel);
        }
    }
}

void CLChannelContextDidRemoveChannel(objv_class_t * clazz,CLChannelContext * ctx){
    
    if(clazz && ctx){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(didRemoveChannel))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            (* (CLChannelContextMethodDidRemoveChannel) method->impl)(c,ctx);
        }
    }
    
}

