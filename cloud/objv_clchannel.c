//
//  objv_clchannel.c
//  objv
//
//  Created by zhang hailong on 14-2-10.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv.h"
#include "objv_cloud.h"
#include "objv_clchannel.h"
#include "objv_json.h"
#include "objv_autorelease.h"
#include "objv_value.h"
#include "objv_log.h"
#include "objv_hash_map.h"
#include "objv_dictionary.h"

OBJV_KEY_IMP(CLChannel)
OBJV_KEY_IMP(readTask)
OBJV_KEY_IMP(postTask)
OBJV_KEY_IMP(tick)
OBJV_KEY_IMP(setChannel)
OBJV_KEY_DEC(disconnect)

static objv_object_t * CLChannelMethodInit(objv_class_t * clazz,objv_object_t * object,va_list ap){
    
    if(clazz->superClass){
        object = objv_object_initv(clazz->superClass, object, ap);
    }
    
    if(object){
        
        CLChannel * channel = (CLChannel *) object;
        
        channel->heartbeatTimeinval = 6;
        
    }
    
    return object;
}

static void CLChannelMethodDealloc(objv_class_t * clazz,objv_object_t * object){
    
    CLChannel * channel = (CLChannel *) object;

    objv_object_release((objv_object_t *) channel->oChannel);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
    
}


OBJV_CLASS_METHOD_IMP_BEGIN(CLChannel)

OBJV_CLASS_METHOD_IMP(init, "@(*)", CLChannelMethodInit)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", CLChannelMethodDealloc)

OBJV_CLASS_METHOD_IMP_END(CLChannel)

OBJV_CLASS_IMP_M(CLChannel, OBJV_CLASS(Object), CLChannel)


CLChannel * CLChannelAlloc(objv_zone_t * zone,objv_class_t * clazz,objv_channel_t * channel){
    return (CLChannel *) objv_object_alloc(zone, clazz,channel,NULL);
}

OBJVChannelStatus CLChannelConnect(objv_class_t * clazz, CLChannel * channel,objv_timeinval_t timeout){
    
    if(clazz && channel){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(connect))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (CLChannelMethodConnect) method->impl)(c,channel,timeout);
        }
        
    }
    
    return OBJVChannelStatusError;
}

OBJVChannelStatus CLChannelDisconnect(objv_class_t * clazz, CLChannel * channel){
    
    if(clazz && channel){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(disconnect))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (CLChannelMethodDisconnect) method->impl)(c,channel);
        }
        
    }
    
    return OBJVChannelStatusError;
}

OBJVChannelStatus CLChannelReadTask(objv_class_t * clazz,CLChannel * channel,CLTask ** task,objv_class_t ** taskType,objv_timeinval_t timeout){
    
    if(clazz && channel){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(readTask))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (CLChannelMethodReadTask) method->impl)(c,channel,task,taskType,timeout);
        }
        
    }
    
    return OBJVChannelStatusError;

}

void CLChannelPostTask(objv_class_t * clazz,CLChannel * channel,CLTask * task,objv_class_t * taskType){
  
    if(clazz && channel){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(postTask))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            (* (CLChannelMethodPostTask) method->impl)(c,channel,task,taskType);
        }
        
    }
    
}

OBJVChannelStatus CLChannelTick(objv_class_t * clazz,CLChannel * channel,objv_timeinval_t timeout){
    
    if(clazz && channel){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(tick))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (CLChannelMethodTick) method->impl)(c,channel,timeout);
        }
        
    }
    
    return OBJVChannelStatusError;
}

void CLChannelSetChannel(CLChannel * channel,objv_channel_t * oChannel){
    
    if(channel && channel->oChannel != oChannel){
        
        objv_object_retain((objv_object_t *) oChannel);
        objv_object_release((objv_object_t *) channel->oChannel);
        
        channel->oChannel = oChannel;
        
    }
    
}

void CLChannelSetDomain(CLChannel * channel,objv_string_t * domain){
    if(channel && channel->domain != domain){
        
        objv_object_retain((objv_object_t *) domain);
        objv_object_release((objv_object_t *) channel->domain);
        
        channel->domain = domain;
        
    }
}

void CLChannelSetURL(CLChannel * channel,objv_url_t * url){
    if(channel && channel->url != url){
        
        objv_object_retain((objv_object_t *) url);
        objv_object_release((objv_object_t *) channel->url);
        
        channel->url = url;
        
    }
}
