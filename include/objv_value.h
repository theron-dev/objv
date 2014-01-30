//
//  objv_value.h
//  objv
//
//  Created by zhang hailong on 14-1-30.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_object_h
#define objv_objv_object_h

#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_string.h"
    
    OBJV_KEY_DEC(stringValue)
    OBJV_KEY_DEC(intValue)
    OBJV_KEY_DEC(uintValue)
    OBJV_KEY_DEC(longValue)
    OBJV_KEY_DEC(ulongValue)
    OBJV_KEY_DEC(longLongValue)
    OBJV_KEY_DEC(ulongLongValue)
    OBJV_KEY_DEC(floatValue)
    OBJV_KEY_DEC(doubleValue)
    OBJV_KEY_DEC(booleanValue)
    
    OBJV_KEY_DEC(Value)
    
    typedef struct _objv_value_t {
        objv_object_t base;
        objv_type_t * READONLY type;
        union {
            int READONLY intValue;
            unsigned int READONLY uintValue;
            long READONLY longValue;
            unsigned long READONLY ulongValue;
            long long READONLY longLongValue;
            unsigned long long READONLY ulongLongValue;
            float READONLY floatValue;
            double READONLY doubleValue;
            objv_boolean_t READONLY booleanValue;
            void * READONLY ptrValue;
        };
        objv_boolean_t READONLY malloc;
    } objv_value_t;
    
    extern objv_class_t objv_value_class;
    
    objv_value_t * objv_value_alloc_intValue(objv_zone_t * zone,int value);
    
    objv_value_t * objv_value_alloc_uintValue(objv_zone_t * zone,unsigned int value);
    
    objv_value_t * objv_value_alloc_longValue(objv_zone_t * zone,long value);
    
    objv_value_t * objv_value_alloc_ulongValue(objv_zone_t * zone,unsigned long value);
    
    objv_value_t * objv_value_alloc_longLongValue(objv_zone_t * zone,long long value);
    
    objv_value_t * objv_value_alloc_ulongLongValue(objv_zone_t * zone,unsigned long long value);
    
    objv_value_t * objv_value_alloc_floatValue(objv_zone_t * zone,float value);
    
    objv_value_t * objv_value_alloc_doubleValue(objv_zone_t * zone,double value);
    
    objv_value_t * objv_value_alloc_booleanValue(objv_zone_t * zone,objv_boolean_t value);
    
    objv_value_t * objv_value_alloc_ptrValue(objv_zone_t * zone,void * value);
    
    objv_value_t * objv_value_alloc_structValue(objv_zone_t * zone,void * value,objv_type_t * type);
    
    
    int objv_value_intValue(objv_value_t * value,int defaultValue);
    
    unsigned int objv_value_uintValue(objv_value_t * value,unsigned int defaultValue);
    
    long objv_value_longValue(objv_value_t * value,long defaultValue);
    
    unsigned long objv_value_ulongValue(objv_value_t * value,unsigned long defaultValue);
    
    long long objv_value_longLongValue(objv_value_t * value,long long defaultValue);
    
    unsigned long long objv_value_ulongLongValue(objv_value_t * value,unsigned long long defaultValue);
    
    float objv_value_floatValue(objv_value_t * value,float defaultValue);
    
    double objv_value_doubleValue(objv_value_t * value,double defaultValue);
    
    objv_boolean_t objv_value_booleanValue(objv_value_t * value,objv_boolean_t defaultValue);
    
    void * objv_value_ptrValue(objv_value_t * value,void * defaultValue);
    
    
    typedef int (* objv_object_property_intValue_t) (objv_class_t * clazz,objv_object_t * object,objv_property_t * property);
    
    typedef unsigned int (* objv_object_property_uintValue_t) (objv_class_t * clazz,objv_object_t * object,objv_property_t * property);
    
    typedef long (* objv_object_property_longValue_t) (objv_class_t * clazz,objv_object_t * object,objv_property_t * property);
    
    typedef unsigned long (* objv_object_property_ulongValue_t) (objv_class_t * clazz,objv_object_t * object,objv_property_t * property);
    
    typedef long long (* objv_object_property_longLongValue_t) (objv_class_t * clazz,objv_object_t * object,objv_property_t * property);
    
    typedef unsigned long long (* objv_object_property_ulongLongValue_t) (objv_class_t * clazz,objv_object_t * object,objv_property_t * property);
    
    typedef float (* objv_object_property_floatValue_t) (objv_class_t * clazz,objv_object_t * object,objv_property_t * property);
    
    typedef double (* objv_object_property_doubleValue_t) (objv_class_t * clazz,objv_object_t * object,objv_property_t * property);
    
    typedef objv_boolean_t (* objv_object_property_booleanValue_t) (objv_class_t * clazz,objv_object_t * object,objv_property_t * property);
    
    typedef objv_string_t * (* objv_object_property_stringValue_t) (objv_class_t * clazz,objv_object_t * object,objv_property_t * property);
    
    
    int objv_object_intValue(objv_object_t * object,int defaultValue);
    
    unsigned int objv_object_uintValue(objv_object_t * object,unsigned int defaultValue);
    
    long objv_object_longValue(objv_object_t * object,long defaultValue);
    
    unsigned long objv_object_ulongValue(objv_object_t * object,unsigned long defaultValue);
    
    long long objv_object_longLongValue(objv_object_t * object,long long defaultValue);
    
    unsigned long long objv_object_ulongLongValue(objv_object_t * object,unsigned long long defaultValue);
    
    float objv_object_floatValue(objv_object_t * object,float defaultValue);
    
    double objv_object_doubleValue(objv_object_t * object,double defaultValue);
    
    objv_boolean_t objv_object_booleanValue(objv_object_t * object,objv_boolean_t defaultValue);
    
    objv_string_t * objv_object_stringValue(objv_object_t * object,objv_string_t * defaultValue);
    
    
#ifdef __cplusplus
}
#endif


#endif
