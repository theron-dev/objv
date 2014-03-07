//
//  objv_data.c
//  objv
//
//  Created by zhang hailong on 14-3-7.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY


#include "objv_os.h"
#include "objv.h"
#include "objv_data.h"
#include "objv_autorelease.h"

OBJV_KEY_IMP(Data)

static void objv_data_method_dealloc(objv_class_t * clazz,objv_object_t * object ){

    if(clazz->superClass){
        
        objv_object_dealloc(clazz->superClass,object);
    }
}


static objv_object_t * objv_data_method_init(objv_class_t * clazz,objv_object_t * object,va_list ap){
    
    if(clazz->superClass){
        object = objv_object_initv(clazz->superClass,object,ap);
    }
    
    if(object){
        
        objv_data_t * data = (objv_data_t *) object;
        void * bytes = va_arg(ap,void *);
        unsigned int length = va_arg(ap,unsigned int);
        objv_boolean_t copyed = va_arg(ap,objv_boolean_t);
        
        if(copyed){
            
            data->bytes = objv_zone_malloc(object->zone,length);
            data->length = length;
            data->copyed = copyed;
            
            memcpy(data->bytes,bytes,length);
        }
        else{
            data->bytes = bytes;
            data->length = length;
            data->copyed = copyed;
        }
    }
    
    return object;
    
}


OBJV_CLASS_METHOD_IMP_BEGIN(Data)

OBJV_CLASS_METHOD_IMP(dealloc,"v()",objv_data_method_dealloc)

OBJV_CLASS_METHOD_IMP(init,"@(*)",objv_data_method_init)

OBJV_CLASS_METHOD_IMP_END(Data)

OBJV_CLASS_IMP_M(Data,OBJV_CLASS(Object),objv_data_t)

objv_data_t * objv_data_alloc(objv_zone_t * zone,void * bytes,unsigned int length){
    return (objv_data_t *) objv_object_alloc(zone, OBJV_CLASS(Data),bytes,length,objv_true,NULL);
}

objv_data_t * objv_data_alloc_nocopy(objv_zone_t * zone,void * bytes,unsigned int length){
    return (objv_data_t *) objv_object_alloc(zone, OBJV_CLASS(Data),bytes,length,objv_false,NULL);
}

objv_data_t * objv_data_new(objv_zone_t * zone,void * bytes,unsigned int length){
    return (objv_data_t *) objv_object_autorelease((objv_object_t *) objv_data_alloc(zone,bytes,length));
}

objv_data_t * objv_data_new_nocopy(objv_zone_t * zone,void * bytes,unsigned int length){
    return (objv_data_t *) objv_object_autorelease((objv_object_t *) objv_data_alloc_nocopy(zone,bytes,length));
}

