//
//  objv.h
//  objv
//
//  Created by zhang hailong on 14-1-29.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_h
#define objv_objv_h

#include "objv_types.h"

#ifdef __cplusplus
extern "C" {
#endif

    
  
    typedef struct _objv_zone_t {
        const char * READONLY name;
        void *(* malloc)(struct _objv_zone_t *zone, size_t size);
        void (*free)(struct _objv_zone_t *zone, void *ptr);
        void *(*realloc)(struct _objv_zone_t *zone, void *ptr, size_t size);
        void (* memzero)(struct _objv_zone_t *zone, void *ptr, size_t size);
    } objv_zone_t;
    
    objv_zone_t * objv_zone_default();
    
    void * objv_zone_malloc(objv_zone_t * zone,size_t size);
    
    void objv_zone_free(objv_zone_t * zone,void * ptr);
    
    void * objv_zone_realloc(objv_zone_t * zone,void * ptr,size_t size);
    
    void objv_zone_memzero(objv_zone_t * zone,void * ptr,size_t size);
    
    typedef enum {
        objv_key_type_static = 0,objv_key_type_dynamic = 1
    } objv_key_type;
    
    typedef struct _objv_key_t {
        const char * name;
        objv_key_type type;
    } objv_key_t;
   
#define OBJV_KEY_DEC(name)  extern objv_key_t __objv_key_##name;
#define OBJV_KEY_IMP(name)  objv_key_t __objv_key_##name = {#name,objv_key_type_static};
#define OBJV_KEY(name)      (& __objv_key_##name )
    
    OBJV_KEY_DEC(dealloc)
    OBJV_KEY_DEC(retainCount)
    OBJV_KEY_DEC(equal)
    OBJV_KEY_DEC(hashCode)
    OBJV_KEY_DEC(Object)
    
    objv_boolean_t objv_key_equal(objv_key_t * key1,objv_key_t * key2);
    
    objv_key_t * objv_key(const char * key);

    typedef struct _objv_type_t {
        const char * READONLY name;
        size_t READONLY size;
    } objv_type_t;
    
    extern objv_type_t objv_type_int;       //i
    extern objv_type_t objv_type_uint;      //I
    extern objv_type_t objv_type_long;      //l
    extern objv_type_t objv_type_ulong;     //L
    extern objv_type_t objv_type_longLong;  //q
    extern objv_type_t objv_type_ulongLong; //Q
    extern objv_type_t objv_type_float;     //f
    extern objv_type_t objv_type_double;    //d
    extern objv_type_t objv_type_void;      //v
    extern objv_type_t objv_type_boolean;   //b
    extern objv_type_t objv_type_ptr;       //*
    extern objv_type_t objv_type_object;    //@
    
#define OBJV_TYPE_DEC(S)        extern objv_type_t objv_type_##S;
#define OBJV_TYPE_IMP(S)        objv_type_t objv_type_##S = {#S,sizeof(S)};
#define OBJV_TYPE(S)            ( & objv_type_##S )
    
    typedef void * objv_method_impl_t;
    
    typedef struct _objv_method_t {
        objv_key_t * READONLY name;
        const char * READONLY types;
        objv_method_impl_t READONLY impl;
    } objv_method_t;
    
    typedef struct _objv_property_t {
        objv_key_t * READONLY name;
        objv_type_t * READONLY type;
        objv_method_t * READONLY getter;
        objv_method_t * READONLY setter;
    } objv_property_t;
    
    typedef struct _objv_class_t {
        objv_key_t * READONLY name;
        struct _objv_class_t * READONLY superClass;
        objv_method_t * READONLY methods;
        unsigned int READONLY methodCount;
        objv_property_t * READONLY propertys;
        unsigned int READONLY propertyCount;
        size_t READONLY size;
        void (* initialize) (struct _objv_class_t * clazz);
        
        size_t READONLY offset;
        objv_boolean_t READONLY initialized;
        
    } objv_class_t;
    
    objv_method_t * objv_class_getMethod(objv_class_t * clazz,objv_key_t * name);
    objv_property_t * objv_class_getProperty(objv_class_t * clazz,objv_key_t * name);
    objv_boolean_t objv_class_isKindOfClass(objv_class_t * clazz,objv_class_t * ofClass);
    
    typedef struct _objv_object_t {
        objv_class_t * READONLY isa;
        objv_zone_t * READONLY zone;
        objv_mutex_t READONLY mutex;
        int READONLY retainCount;
    } objv_object_t;
   
    objv_class_t * objv_class(const char * className);

    void objv_class_initialize(objv_class_t * clazz);
    
    objv_object_t * objv_object_alloc(objv_zone_t * zone,objv_class_t * clazz);
    
    objv_object_t * objv_object_retain(objv_object_t * object);
    
    void objv_object_release(objv_object_t * object);
    
    void objv_object_lock(objv_object_t * object);
    
    void objv_object_unlock(objv_object_t * object);
    
    objv_boolean_t objv_object_isKindOfClass(objv_object_t * object,objv_class_t * ofClass);
    
    
    extern objv_class_t objv_object_class;
    
    typedef void (* objv_object_method_dealloc_t) (objv_class_t * clazz, objv_object_t * object);
    
    void objv_object_dealloc(objv_class_t * clazz, objv_object_t * object);
    
    typedef long (* objv_object_method_hashCode_t) (objv_class_t * clazz, objv_object_t * object);
    
    long objv_object_hashCode(objv_class_t * clazz,objv_object_t * object);
    
    typedef objv_boolean_t (* objv_object_method_equal_t) (objv_class_t * clazz, objv_object_t * object,objv_object_t * value);
    
    objv_boolean_t objv_object_equal(objv_class_t * clazz,objv_object_t * object,objv_object_t * value);
    

#ifdef __cplusplus
}
#endif

#endif
