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
        void (* retain)(struct _objv_zone_t *zone, void *ptr);
        void (* release)(struct _objv_zone_t *zone, void *ptr);
    } objv_zone_t;
    
    objv_zone_t * objv_zone_default();
    
    void objv_zone_default_set(objv_zone_t * zone);
    
    
    void * objv_zone_malloc(objv_zone_t * zone,size_t size);
    
    void objv_zone_free(objv_zone_t * zone,void * ptr);
    
    void * objv_zone_realloc(objv_zone_t * zone,void * ptr,size_t size);

    void objv_zone_memzero(objv_zone_t * zone,void * ptr,size_t size);
    
    void objv_zone_retain(objv_zone_t * zone,void * ptr);
    
    void objv_zone_release(objv_zone_t * zone,void * ptr);
    
    typedef const char objv_key_t;
    
#define OBJV_KEY_DEC(key)
#define OBJV_KEY_IMP(key)
#define OBJV_KEY(key)   #key
    
    enum {
        objv_type_void,         //v
        objv_type_int,          //i
        objv_type_uint,         //I
        objv_type_long,         //l
        objv_type_ulong,        //L
        objv_type_longLong,     //q
        objv_type_ulongLong,    //Q
        objv_type_float,        //f
        objv_type_double,       //d
        objv_type_boolean,      //b
        objv_type_ptr,          //*
        objv_type_object,       //@
        objv_type_bytes,       //B
    };
    
#define OBJV_TYPE(type) objv_type_##type
    
    typedef unsigned int objv_type_t;
    
    typedef void * objv_method_impl_t;
    
    typedef struct _objv_method_t {
        objv_key_t * READONLY name;
        const char * READONLY types;
        objv_method_impl_t READONLY impl;
    } objv_method_t;
    
    typedef struct _objv_property_t {
        objv_key_t * READONLY name;
        objv_type_t READONLY type;
        objv_method_impl_t * READONLY getter;
        objv_method_impl_t * READONLY setter;
        objv_boolean_t serialization;
    } objv_property_t;
    
    struct _objv_hash_map_t;
    
    typedef struct _objv_class_t {
        objv_key_t * READONLY name;
        struct _objv_class_t * READONLY superClass;
        objv_method_t * READONLY methods;
        unsigned int READONLY methodCount;
        objv_property_t * READONLY propertys;
        unsigned int READONLY propertyCount;
        size_t READONLY size;

        void (* initialize) (struct _objv_class_t * clazz);
        
        objv_boolean_t READONLY initialized;
        
        struct _objv_hash_map_t * methodsMap;
        struct _objv_hash_map_t * propertysMap;
    } objv_class_t;
    
    objv_method_t * objv_class_getMethod(objv_class_t * clazz,objv_key_t * name);
    objv_method_t * objv_class_getMethodOfClass(objv_class_t * clazz,objv_key_t * name,objv_class_t ** ofClass);
    objv_property_t * objv_class_getProperty(objv_class_t * clazz,objv_key_t * name);
    objv_property_t * objv_class_getPropertyOfClass(objv_class_t * clazz,objv_key_t * name,objv_class_t ** ofClass);
    objv_boolean_t objv_class_isKindOfClass(objv_class_t * clazz,objv_class_t * ofClass);
    
    struct _objv_object_t;
    
    typedef struct _objv_object_weak_t {
        struct _objv_object_t ** READONLY object;
        struct _objv_object_weak_t * READONLY next;
    } objv_object_weak_t;
    
    typedef struct _objv_object_t {
        objv_class_t * READONLY isa;
        objv_zone_t * READONLY zone;
        objv_mutex_t READONLY mutex;
        int READONLY retainCount;
        objv_object_weak_t * READONLY weak;
    } objv_object_t;
   
    objv_class_t * objv_class(objv_key_t * className);

    void objv_class_initialize(objv_class_t * clazz);

    void objv_class_reg(objv_class_t * clazz);
    
    typedef objv_object_t * (* objv_object_method_init_t) (objv_class_t * clazz, objv_object_t * object,va_list ap);
    
    objv_object_t * objv_object_init(objv_class_t * clazz,objv_object_t * object,...);
    
    objv_object_t * objv_object_initv(objv_class_t * clazz,objv_object_t * object,va_list ap);
    
    objv_object_t * objv_object_allocv(objv_zone_t * zone,objv_class_t * clazz,va_list ap);
    
    objv_object_t * objv_object_alloc(objv_zone_t * zone,objv_class_t * clazz,...);
    
    objv_object_t * objv_object_alloc_exertv(objv_zone_t * zone,objv_class_t * clazz,size_t exert,va_list ap);
    
    objv_object_t * objv_object_alloc_exert(objv_zone_t * zone,objv_class_t * clazz,size_t exert,...);

    objv_object_t * objv_object_retain(objv_object_t * object);

    void objv_object_release(objv_object_t * object);
    
    objv_object_t * objv_object_weak(objv_object_t * object, objv_object_t ** toObject);
    
    void objv_object_unweak(objv_object_t * object, objv_object_t ** toObject);
    
    objv_boolean_t objv_object_isKindOfClass(objv_object_t * object,objv_class_t * ofClass);
    
    extern objv_class_t objv_Object_class;
    
    typedef void (* objv_object_method_dealloc_t) (objv_class_t * clazz, objv_object_t * object);
    
    void objv_object_dealloc(objv_class_t * clazz, objv_object_t * object);
    
    typedef long (* objv_object_method_hashCode_t) (objv_class_t * clazz, objv_object_t * object);
    
    long objv_object_hashCode(objv_class_t * clazz,objv_object_t * object);
    
    typedef objv_boolean_t (* objv_object_method_equal_t) (objv_class_t * clazz, objv_object_t * object,objv_object_t * value);
    
    objv_boolean_t objv_object_equal(objv_class_t * clazz,objv_object_t * object,objv_object_t * value);
    
    typedef objv_object_t * ( * objv_object_method_copy_t) (objv_class_t * clazz, objv_object_t * object);
    
    objv_object_t * objv_object_copy(objv_class_t * clazz,objv_object_t * object);

#define OBJV_CLASS(clazz)   (&objv_##clazz##_class)
    
#define OBJV_CLASS_DEC(clazz) extern objv_class_t objv_##clazz##_class;
    
#define OBJV_CLASS_METHOD_IMP_BEGIN(clazz) static objv_method_t objv_##clazz##_methods[] = {
    
#define OBJV_CLASS_METHOD_IMP(name,type,impl) {(#name),(type),(objv_method_impl_t)(impl)},
    
#define OBJV_CLASS_METHOD_IMP_END(clazz) };
    
#define OBJV_CLASS_PROPERTY_IMP_BEGIN(clazz) static objv_property_t objv_##clazz##_propertys[] = {
    
#define OBJV_CLASS_PROPERTY_IMP(name,type,getter,setter,serialization) {(#name),objv_type_##type,(objv_method_impl_t)(getter),(objv_method_impl_t)(setter),(serialization)},
    
#define OBJV_CLASS_PROPERTY_IMP_END(clazz) };
    
    
#define OBJV_CLASS_IMP(clazz,superClazz,object) objv_class_t objv_##clazz##_class = {(#clazz),(superClazz),NULL,0,NULL,0,sizeof(object),NULL,0};
  
#define OBJV_CLASS_IMP_P(clazz,superClazz,object) objv_class_t objv_##clazz##_class = {(#clazz),(superClazz),NULL,0,objv_##clazz##_propertys,sizeof(objv_##clazz##_propertys) / sizeof(objv_property_t),sizeof(object),NULL,0};

#define OBJV_CLASS_IMP_P_M(clazz,superClazz,object) objv_class_t objv_##clazz##_class = {(#clazz),(superClazz), objv_##clazz##_methods,sizeof( objv_##clazz##_methods) / sizeof(objv_method_t),objv_##clazz##_propertys,sizeof(objv_##clazz##_propertys) / sizeof(objv_property_t),sizeof(object),NULL,0};
    
#define OBJV_CLASS_IMP_M(clazz,superClazz,object) objv_class_t objv_##clazz##_class = {(#clazz),(superClazz), objv_##clazz##_methods,sizeof( objv_##clazz##_methods) / sizeof(objv_method_t),NULL,0,sizeof(object),NULL,0};

#define OBJV_CLASS_IMP_I(clazz,superClazz,object,initialize) objv_class_t objv_##clazz##_class = {(#clazz),(superClazz),NULL,0,NULL,0,sizeof(object),(initialize),0};
    
#define OBJV_CLASS_IMP_P_I(clazz,superClazz,object,initialize) objv_class_t objv_##clazz##_class = {(#clazz),(superClazz),NULL,0,objv_##clazz##_propertys,sizeof(objv_##clazz##_propertys) / sizeof(objv_property_t),sizeof(object),(initialize),0};

#define OBJV_CLASS_IMP_M_I(clazz,superClazz,object,initialize) objv_class_t objv_##clazz##_class = {(#clazz),(superClazz), objv_##clazz##_methods,sizeof( objv_##clazz##_methods) / sizeof(objv_method_t),NULL,0,sizeof(object),(initialize),0};

#define OBJV_CLASS_IMP_P_M_I(clazz,superClazz,object,initialize) objv_class_t objv_##clazz##_class = {(#clazz),(superClazz), objv_##clazz##_methods,sizeof( objv_##clazz##_methods) / sizeof(objv_method_t),objv_##clazz##_propertys,sizeof(objv_##clazz##_propertys) / sizeof(objv_property_t),sizeof(object),(initialize),0};
    
#ifdef __cplusplus
}
#endif

#endif
