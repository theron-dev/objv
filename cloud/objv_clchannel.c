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


static objv_object_t * CLChannelMethodInit(objv_class_t * clazz,objv_object_t * object,va_list ap){
    
    if(clazz->superClass){
        object = objv_object_initv(clazz->superClass, object, ap);
    }
    
    if(object){
        
        CLChannel * channel = (CLChannel *) object;
        
        objv_channel_t * c = va_arg(ap, objv_channel_t *);
        
        if(c){
            
            channel->channel = (objv_channel_t *) objv_object_retain((objv_object_t *) c);

            objv_mbuf_init(& channel->read.data,128);
            objv_mbuf_init(& channel->write.data,128);
            
            channel->heartbeatTimeinval = 6;
        }
        else{
            objv_object_release(object);
            return NULL;
        }
    }
    
    return object;
}

static void CLChannelMethodDealloc(objv_class_t * clazz,objv_object_t * object){
    
    CLChannel * channel = (CLChannel *) object;

    objv_object_release((objv_object_t *) channel->task);
    objv_object_release((objv_object_t *) channel->channel);
    
    objv_mbuf_destroy(& channel->read.data);
    objv_mbuf_destroy(& channel->write.data);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
    
}


OBJV_CLASS_METHOD_IMP_BEGIN(CLChannel)

OBJV_CLASS_METHOD_IMP(init, "@(*)", CLChannelMethodInit)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", CLChannelMethodDealloc)

OBJV_CLASS_METHOD_IMP_END(CLChannel)

OBJV_CLASS_IMP_M(CLChannel, OBJV_CLASS(Object), CLChannel)

objv_boolean_t CLChannelConnectBlockValidate(CLChannelConnectBlock * block){
    unsigned char * p = (unsigned char *) block;
    size_t len = sizeof(CLChannelConnectBlock) - sizeof(unsigned int);
    unsigned int verify = 0;
    
    while (p && len > 0) {
        verify += * p;
        len --;
        p ++;
    }
    return verify == block->verify;
}

void CLChannelConnectBlockSignature(CLChannelConnectBlock * block){
    unsigned char * p = (unsigned char *) block;
    size_t len = sizeof(CLChannelConnectBlock) - sizeof(unsigned int);
    unsigned int verify = 0;
    
    while (p && len > 0) {
        verify += * p;
        len --;
        p ++;
    }
    
    block->verify = verify;
}

void CLChannelRequestBlockSetLength(CLChannelRequestBlock * block,size_t length){
    snprintf(block->length, sizeof(block->length),"%lx",length);
}

size_t CLChannelRequestBlockLength(CLChannelRequestBlock * block){
    size_t l = 0;
    sscanf(block->length, "%lx",& l);
    return l;
}

CLChannel * CLChannelAlloc(objv_zone_t * zone,objv_channel_t * channel){
    return (CLChannel *) objv_object_alloc(zone, OBJV_CLASS(CLChannel),channel,NULL);
}

OBJVChannelStatus CLChannelConnect(CLChannel * channel,objv_timeinval_t timeout){
    
    if(channel){
        
        if(channel->connected){
            return OBJVChannelStatusOK;
        }
        else{
            
            OBJVChannelStatus status = objv_channel_connect(channel->channel->base.isa, channel->channel, timeout);
            
            while(status != OBJVChannelStatusError){
                
                status = objv_channel_canRead(channel->channel->base.isa, channel->channel, timeout);
                
                if(status == OBJVChannelStatusOK){
                    
                    objv_mbuf_extend(& channel->read.data, sizeof(CLChannelConnectBlock));
                    
                    objv_mbuf_clear(& channel->read.data);
                    
                    if(objv_channel_read(channel->channel->base.isa, channel->channel, channel->read.data.data, sizeof(CLChannelConnectBlock)) == sizeof(CLChannelConnectBlock)){
                        
                        if(CLChannelConnectBlockValidate((CLChannelConnectBlock *) channel->read.data.data)){
                            
                            memcpy(& channel->block ,channel->read.data.data,sizeof(CLChannelConnectBlock));
                            
                            channel->connected = objv_true;
                            
                            status = OBJVChannelStatusOK;
                            
                            break;
                        }
                        else{
                            status = OBJVChannelStatusError;
                        }
                    }
                    else{
                        status = OBJVChannelStatusError;
                    }
                }

            }
            
            return status;
            
        }
    }
    
    return OBJVChannelStatusError;
}

OBJVChannelStatus CLChannelReadTask(CLChannel * channel,CLTask ** task,objv_class_t ** taskType,objv_timeinval_t timeout){
    
    if(channel){
        
        OBJVChannelStatus status = OBJVChannelStatusOK;
        ssize_t length;
        
        if(channel->read.state == 0){
            
            if(channel->read.data.length < sizeof(CLChannelRequestBlock)){
                
                status = objv_channel_canRead(channel->channel->base.isa, channel->channel, timeout);
                
                if(status == OBJVChannelStatusOK){
                    
                    objv_mbuf_extend(& channel->read.data, sizeof(CLChannelRequestBlock));
                    
                    length = objv_channel_read(channel->channel->base.isa, channel->channel, (char *) channel->read.data.data + channel->read.data.length, sizeof(CLChannelRequestBlock) - channel->read.data.length);
                    
                    if(length < 0){
                        return OBJVChannelStatusError;
                    }
                    
                    channel->read.data.length += length;
                    
                }
                else{
                    return status;
                }
            }
            
            if(channel->read.data.length == sizeof(CLChannelRequestBlock)){
                
                {
                    CLChannelRequestBlock * block = (CLChannelRequestBlock *) channel->read.data.data ;
                    
                    channel->read.dataLength = CLChannelRequestBlockLength(block);

                    objv_mbuf_clear(& channel->read.data);
                    
                    if(channel->read.dataLength >0){
                        objv_mbuf_extend(& channel->read.data, channel->read.dataLength + 1);
                        channel->read.state = 1;
                    }
                }
                
                status = OBJVChannelStatusOK;
            }
            
        }
        
        if(channel->read.state == 1){
            
            if(channel->read.data.length < channel->read.dataLength){
                
                status = objv_channel_canRead(channel->channel->base.isa, channel->channel, timeout);
                
                if(status == OBJVChannelStatusOK){
                    
                    length = objv_channel_read(channel->channel->base.isa, channel->channel, (char *) channel->read.data.data + channel->read.data.length, channel->read.dataLength - channel->read.data.length);
                    
                    if(length < 0){
                        return OBJVChannelStatusError;
                    }
                    
                    channel->read.data.length += length;
                    
                }
                else{
                    return status;
                }
                
            }
            
            if(channel->read.data.length == channel->read.dataLength){
                
                {
                    objv_object_t * object;
                    objv_string_t * stringValue;
                    objv_class_t * tType;
                    objv_class_t * tClass;
                    CLTask * t;
                    
                    objv_zone_t * zone = channel->base.zone;
                    
                    object = objv_json_decode(channel->base.zone, objv_mbuf_str(& channel->read.data));
                    
                    while(object){
                        
                        stringValue = objv_object_stringValueForKey(object
                                                            , (objv_object_t *) objv_string_new(zone, "taskType"),NULL);
                        
                        if(stringValue){
                            tType = objv_class(objv_key(stringValue->UTF8String));
                            if(!taskType){
                                objv_log("Error: not found taskType %s\n",stringValue->UTF8String);
                                break;
                            }
                            
                            if(!objv_class_isKindOfClass(tType, OBJV_CLASS(CLTask))){
                                objv_log("Error: taskType %s not kindOfClass CLTask\n",stringValue->UTF8String);
                                break;
                            }
                        }
                        else{
                            break;
                        }
                        
                        stringValue = objv_object_stringValueForKey(object
                                                                 , (objv_object_t *) objv_string_new(zone, "taskClass"),NULL);
                        
                        if(stringValue){
                            tClass = objv_class(objv_key(stringValue->UTF8String));
                            if(!tClass){
                                objv_log("Error: not found taskClass %s\n",stringValue->UTF8String);
                                break;
                            }
                            
                            if(!objv_class_isKindOfClass(tClass, OBJV_CLASS(CLTask))){
                                objv_log("Error: taskClass %s not kindOfClass CLTask\n",stringValue->UTF8String);
                                break;
                            }
                        }
                        else{
                            tClass = tType;
                        }
                        
                        t = (CLTask * ) objv_object_new(zone, tClass,NULL);
                        
                        if(! t ){
                            break;
                        }

                        {
                            objv_hash_map_t * keys = objv_hash_map_alloc(16, objv_hash_map_hash_code_key, objv_map_compare_key);
                            objv_class_t * c = tClass;
                            objv_property_t * prop;
                            unsigned int propCount;
                            
                            while(c){
                                
                                prop = c->propertys;
                                propCount = c->propertyCount;
                                
                                while(prop && propCount > 0){
                                    
                                    if(prop->serialization && prop->setter && objv_hash_map_get(keys, prop->name) == NULL){
                                        
                                        if(prop->type == & objv_type_int){
                                            objv_property_setIntValue(c, object, prop, objv_object_intValueForKey(object, (objv_object_t *)objv_string_new_nocopy(zone, prop->name->name), 0));
                                        }
                                        else if(prop->type == & objv_type_uint){
                                            objv_property_setUintValue(c, object, prop, objv_object_uintValueForKey(object, (objv_object_t *)objv_string_new_nocopy(zone, prop->name->name), 0));
                                        }
                                        else if(prop->type == & objv_type_long){
                                            objv_property_setLongValue(c, object, prop, objv_object_longValueForKey(object, (objv_object_t *)objv_string_new_nocopy(zone, prop->name->name), 0));
                                        }
                                        else if(prop->type == & objv_type_ulong){
                                            objv_property_setUlongValue(c, object, prop, objv_object_ulongValueForKey(object, (objv_object_t *)objv_string_new_nocopy(zone, prop->name->name), 0));
                                        }
                                        else if(prop->type == & objv_type_longLong){
                                            objv_property_setLongLongValue(c, object, prop, objv_object_longLongValueForKey(object, (objv_object_t *)objv_string_new_nocopy(zone, prop->name->name), 0));
                                        }
                                        else if(prop->type == & objv_type_ulongLong){
                                            objv_property_setUlongLongValue(c, object, prop, objv_object_ulongLongValueForKey(object, (objv_object_t *)objv_string_new_nocopy(zone, prop->name->name), 0));
                                        }
                                        else if(prop->type == & objv_type_float){
                                            objv_property_setFloatValue(c, object, prop, objv_object_floatValueForKey(object, (objv_object_t *) objv_string_new_nocopy(zone, prop->name->name), 0));
                                        }
                                        else if(prop->type == & objv_type_double){
                                            objv_property_setDoubleValue(c, object, prop, objv_object_doubleValueForKey(object, (objv_object_t *) objv_string_new_nocopy(zone, prop->name->name), 0));
                                        }
                                        else if(prop->type == & objv_type_boolean){
                                            objv_property_setBooleanValue(c, object, prop, objv_object_booleanValueForKey(object, (objv_object_t *) objv_string_new_nocopy(zone, prop->name->name), 0));
                                        }
                                        else if(prop->type == & objv_type_object){
                                            objv_property_setObjectValue(c, object, prop, objv_object_objectValueForKey(object, (objv_object_t *) objv_string_new_nocopy(zone, prop->name->name), NULL));
                                        }

                                        objv_hash_map_put(keys, prop->name,prop);
                                    }
                                    
                                    propCount --;
                                    prop ++;
                                }
                                
                                c = c->superClass;
                            }
                            
                            
                            objv_hash_map_dealloc(keys);
                            
                            if(task){
                                * task = t;
                            }
                            if(taskType){
                                * taskType = tType;
                            }
                        }
                        
                        break;
                    }
                }
                
                
                status = OBJVChannelStatusOK;
            }
        }
        
    }
    
    return OBJVChannelStatusError;
}

void CLChannelSetTask(CLChannel * channel,CLTask * task,objv_class_t * taskType){
    if(channel && task && taskType){
        
        objv_object_retain((objv_object_t *) task);
        objv_object_release((objv_object_t *) channel->task);
        
        channel->task = task;
        channel->taskType = taskType;
    }
}

OBJVChannelStatus CLChannelTick(CLChannel * channel,objv_timeinval_t timeout){
    
    if(channel){
        
        objv_zone_t * zone = channel->base.zone;
        OBJVChannelStatus status = OBJVChannelStatusOK;
        ssize_t length;
        
        if(channel->tickTimeinval == 0){
            channel->tickTimeinval = objv_timestamp();
        }
        
        if(channel->write.state == 0){
            
            if(channel->task && channel->taskType){
                
                {
                    
                    objv_dictionary_t * object = objv_dictionary_alloc(zone, 16);
                    objv_hash_map_t * keys = objv_hash_map_alloc(64, objv_hash_map_hash_code_key, objv_map_compare_key);
                    objv_key_t * taskType = objv_key("taskType");
                    objv_key_t * taskClass = objv_key("taskClass");
                    objv_class_t * c = channel->task->base.isa;
                    objv_property_t * prop;
                    unsigned int propCount;
                    objv_object_t * v;
                    CLChannelRequestBlock * block;
                    
                    objv_dictionary_setValue(object, (objv_object_t *) objv_string_new_nocopy(zone, taskType->name), (objv_object_t *) objv_string_new_nocopy(zone, channel->taskType->name->name));
                    
                    objv_dictionary_setValue(object, (objv_object_t *) objv_string_new_nocopy(zone, taskClass->name), (objv_object_t *) objv_string_new_nocopy(zone, channel->task->base.isa->name->name));
                    
                    
                    objv_hash_map_put(keys, taskType, taskType);
                    objv_hash_map_put(keys, taskClass, taskClass);
                    
                    while(c){
                        
                        prop = c->propertys;
                        propCount = c->propertyCount;
                        
                        while (propCount > 0 && prop) {
                            
                            if(prop->serialization && prop->getter && objv_hash_map_get(keys, prop->name) == NULL){
                                
                                v = objv_property_objectValue(c, (objv_object_t *) channel->task, prop, NULL);
                                
                                objv_dictionary_setValue(object, (objv_object_t *) objv_string_new_nocopy(zone, prop->name->name), v);
                                
                                objv_hash_map_put(keys, prop->name,prop);
                            }
                            
                            propCount --;
                            prop ++;
                        }
                        
                        c = c->superClass;
                    }
                    
                    objv_mbuf_clear(& channel->write.data);
                    
                    objv_mbuf_extend(& channel->write.data, sizeof(CLChannelRequestBlock));
                    
                    channel->write.data.length = sizeof(CLChannelRequestBlock);
                    
                    objv_json_encode_mbuf(zone, (objv_object_t *) object, & channel->write.data, objv_false);
                    
                    block = (CLChannelRequestBlock *) channel->write.data.data;
                    
                    CLChannelRequestBlockSetLength(block, channel->write.data.length - sizeof(CLChannelRequestBlock));
                    
                    objv_object_release((objv_object_t *) object);
                    
                    objv_hash_map_dealloc(keys);
                    
                    channel->write.state = 1;
                    channel->write.writeLength = 0;
                    
                    objv_object_release((objv_object_t *) channel->task);
                    
                    channel->task = NULL;
                    channel->taskType = NULL;

                }
                
                channel->idleTimeinval = 0;
            }
            else{
                channel->idleTimeinval += objv_timestamp() - channel->tickTimeinval;
            }
        }
        
        if(channel->write.state == 1){
            
            if(channel->write.writeLength < channel->write.data.length){
                
                while((status = objv_channel_canWrite(channel->channel->base.isa, channel->channel, timeout)) == OBJVChannelStatusOK){
                    
                    length = objv_channel_write(channel->channel->base.isa, channel->channel, (char *) channel->write.data.data + channel->write.writeLength, channel->write.data.length - channel->write.writeLength);
                    
                    if(length < 0){
                        return OBJVChannelStatusError;
                    }
                    
                    channel->write.writeLength += length;
                    
                    if(channel->write.writeLength >= channel->write.data.length){
                        break;
                    }
                }
                
            }
            
            if(channel->write.writeLength >= channel->write.data.length){
                
                channel->write.writeLength = 0;
                objv_mbuf_clear(& channel->write.data);
                
                channel->write.state = 0;
            }
            
            channel->idleTimeinval = 0;
        }
        
        if(channel->idleTimeinval > channel->heartbeatTimeinval){
            
            if((status = objv_channel_canWrite(channel->channel->base.isa, channel->channel, timeout)) == OBJVChannelStatusOK){
                
                {
                    CLChannelRequestBlock block;
                    CLChannelRequestBlockSetLength(& block, 0);
                    
                    if(objv_channel_write(channel->channel->base.isa, channel->channel, & block, sizeof(CLChannelRequestBlock)) != sizeof(CLChannelRequestBlock)){
                        return OBJVChannelStatusError;
                    }
                }
                
            }
            
        }
        
        return status;
    }
    
    return OBJVChannelStatusError;
}

