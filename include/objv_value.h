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
    
    OBJV_KEY_DEC(objectForKey);
    OBJV_KEY_DEC(setObjectForKey);
    
    OBJV_KEY_DEC(Value)
    
    OBJV_CLASS_DEC(Value)
    
    typedef struct _objv_value_t {
        objv_object_t base;
        objv_type_t READONLY type;
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
            struct {
                void * READONLY bytes;
                size_t READONLY length;
            } bytesValue;
        };
    } objv_value_t;
    
    extern objv_class_t objv_value_class;
    
    objv_value_t * objv_value_alloc_nullValue(objv_zone_t * zone);
    
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
    
    objv_value_t * objv_value_alloc_structValue(objv_zone_t * zone,void * bytes,size_t length);
    
    
    objv_value_t * objv_value_new_nullValue(objv_zone_t * zone);
    
    objv_value_t * objv_value_new_intValue(objv_zone_t * zone,int value);
    
    objv_value_t * objv_value_new_uintValue(objv_zone_t * zone,unsigned int value);
    
    objv_value_t * objv_value_new_longValue(objv_zone_t * zone,long value);
    
    objv_value_t * objv_value_new_ulongValue(objv_zone_t * zone,unsigned long value);
    
    objv_value_t * objv_value_new_longLongValue(objv_zone_t * zone,long long value);
    
    objv_value_t * objv_value_new_ulongLongValue(objv_zone_t * zone,unsigned long long value);
    
    objv_value_t * objv_value_new_floatValue(objv_zone_t * zone,float value);
    
    objv_value_t * objv_value_new_doubleValue(objv_zone_t * zone,double value);
    
    objv_value_t * objv_value_new_booleanValue(objv_zone_t * zone,objv_boolean_t value);
    
    objv_value_t * objv_value_new_ptrValue(objv_zone_t * zone,void * value);
    
    objv_value_t * objv_value_new_structValue(objv_zone_t * zone,void * value,objv_type_t * type);
    
    
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
    
    
    typedef int (* objv_object_property_intValue_t) (objv_class_t * clazz,objv_object_t * object);
    
    typedef unsigned int (* objv_object_property_uintValue_t) (objv_class_t * clazz,objv_object_t * object);
    
    typedef long (* objv_object_property_longValue_t) (objv_class_t * clazz,objv_object_t * object);
    
    typedef unsigned long (* objv_object_property_ulongValue_t) (objv_class_t * clazz,objv_object_t * object);
    
    typedef long long (* objv_object_property_longLongValue_t) (objv_class_t * clazz,objv_object_t * object);
    
    typedef unsigned long long (* objv_object_property_ulongLongValue_t) (objv_class_t * clazz,objv_object_t * object);
    
    typedef float (* objv_object_property_floatValue_t) (objv_class_t * clazz,objv_object_t * object);
    
    typedef double (* objv_object_property_doubleValue_t) (objv_class_t * clazz,objv_object_t * object);
    
    typedef objv_boolean_t (* objv_object_property_booleanValue_t) (objv_class_t * clazz,objv_object_t * object);
    
    typedef objv_string_t * (* objv_object_property_stringValue_t) (objv_class_t * clazz,objv_object_t * object);
    
    typedef objv_object_t * (* objv_object_property_objectValue_t) (objv_class_t * clazz,objv_object_t * object);
    
    
    typedef objv_object_t * (* objv_object_method_objectForKey_t) (objv_class_t * clazz,objv_object_t * object,objv_object_t * key);
    
    typedef void (* objv_object_method_setObjectForKey_t)(objv_class_t * clazz,objv_object_t * object,objv_object_t * key,objv_object_t * value);
    
    objv_object_t * objv_object_objectForKey(objv_class_t * clazz,objv_object_t * object,objv_object_t * key);
    
    void objv_object_setObjectForKey(objv_class_t * clazz,objv_object_t * object,objv_object_t * key,objv_object_t * value);
    
    
    int objv_object_intValueForKey(objv_object_t * object,objv_object_t * key,int defaultValue);
    
    unsigned int objv_object_uintValueForKey(objv_object_t * object,objv_object_t * key,unsigned int defaultValue);
    
    long objv_object_longValueForKey(objv_object_t * object,objv_object_t * key,long defaultValue);
    
    unsigned long objv_object_ulongValueForKey(objv_object_t * object,objv_object_t * key,unsigned long defaultValue);
    
    long long objv_object_longLongValueForKey(objv_object_t * object,objv_object_t * key,long long defaultValue);
    
    unsigned long long objv_object_ulongLongValueForKey(objv_object_t * object,objv_object_t * key,unsigned long long defaultValue);
    
    float objv_object_floatValueForKey(objv_object_t * object,objv_object_t * key,float defaultValue);
    
    double objv_object_doubleValueForKey(objv_object_t * object,objv_object_t * key,double defaultValue);
    
    objv_boolean_t objv_object_booleanValueForKey(objv_object_t * object,objv_object_t * key,objv_boolean_t defaultValue);
    
    objv_string_t * objv_object_stringValueForKey(objv_object_t * object,objv_object_t * key,objv_string_t * defaultValue);
    
    objv_object_t * objv_object_objectValueForKey(objv_object_t * object,objv_object_t * key,objv_object_t * defaultValue);
    
    struct _objv_array_t;
    
    struct _objv_array_t * objv_object_arrayValueForKey(objv_object_t * object,objv_object_t * key);
    
    struct _objv_dictionary_t;
    
    struct _objv_dictionary_t * objv_object_dictionaryValueForKey(objv_object_t * object,objv_object_t * key);
    
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
    
    
    
    int objv_property_intValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property, int defaultValue);
    
    unsigned int objv_property_uintValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,unsigned int defaultValue);
    
    long objv_property_longValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,long defaultValue);
    
    unsigned long objv_property_ulongValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,unsigned long defaultValue);
    
    long long objv_property_longLongValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,long long defaultValue);
    
    unsigned long long objv_property_ulongLongValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,unsigned long long defaultValue);
    
    float objv_property_floatValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,float defaultValue);
    
    double objv_property_doubleValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,double defaultValue);
    
    objv_boolean_t objv_property_booleanValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,objv_boolean_t defaultValue);
    
    objv_string_t * objv_property_stringValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,objv_string_t * defaultValue);

    objv_object_t * objv_property_objectValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,objv_object_t * defaultValue);

    
    typedef void (* objv_object_property_setIntValue_t) (objv_class_t * clazz,objv_object_t * object,int value);
    
    typedef void (* objv_object_property_setUintValue_t) (objv_class_t * clazz,objv_object_t * object,unsigned int value);
    
    typedef void (* objv_object_property_setLongValue_t) (objv_class_t * clazz,objv_object_t * object,long value);
    
    typedef void (* objv_object_property_setUlongValue_t) (objv_class_t * clazz,objv_object_t * object,unsigned long value);
    
    typedef void (* objv_object_property_setLongLongValue_t) (objv_class_t * clazz,objv_object_t * object,long long value);
    
    typedef void (* objv_object_property_setUlongLongValue_t) (objv_class_t * clazz,objv_object_t * object,unsigned long long value);
    
    typedef void (* objv_object_property_setFloatValue_t) (objv_class_t * clazz,objv_object_t * object,float value);
    
    typedef void (* objv_object_property_setDoubleValue_t) (objv_class_t * clazz,objv_object_t * object,double value);
    
    typedef void (* objv_object_property_setBooleanValue_t) (objv_class_t * clazz,objv_object_t * object,objv_boolean_t value);
   
    typedef void (* objv_object_property_setObjectValue_t) (objv_class_t * clazz,objv_object_t * object,objv_object_t * value);
    
    
    void objv_property_setIntValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property, int value);
    
    void objv_property_setUintValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,unsigned int value);
    
    void objv_property_setLongValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,long value);
    
    void objv_property_setUlongValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,unsigned long value);
    
    void objv_property_setLongLongValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,long long value);
    
    void objv_property_setUlongLongValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,unsigned long long value);
    
    void objv_property_setFloatValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,float value);
    
    void objv_property_setDoubleValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,double value);
    
    void objv_property_setBooleanValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,objv_boolean_t value);
    
    void objv_property_setObjectValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,objv_object_t * value);

    
#ifdef __cplusplus
}
#endif


#endif
