//
//  objv_vermin_compiler.h
//  objv
//
//  Created by zhang hailong on 14-2-7.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_vmcompiler_h
#define objv_objv_vmcompiler_h

#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_value.h"
#include "objv_array.h"
#include "objv_vermin.h"
#include "objv_tokenizer.h"
#include "objv_mbuf.h"
    
    typedef struct _vmCompilerString{
        objv_tokenizer_location_t location;
        vm_uint32_t length;
    } vmCompilerString;
    
    typedef struct _vmCompilerMetaOperator{
        objv_object_t base;
        vmOperatorType type;
        vmCompilerString uniqueKey;
        objv_array_t * metas;
        struct{
            vmMetaOffset uniqueKey;
            vmMetaOffset offset;
            vm_uint32_t length;
            unsigned char classUDID[16];
        } binary;
    } vmCompilerMetaOperator;
    
    OBJV_KEY_DEC(vmCompilerMetaOperator)
   
    extern objv_class_t vmCompilerMetaOperatorClass;
    
    typedef struct _vmCompilerMeta{
        objv_object_t base;
        vmMetaType type;
        union {
            vm_int32_t int32Value;
            vm_int64_t int64Value;
            vm_boolean_t booleanValue;
            vm_double_t doubleValue;
            vmCompilerString stringValue;
            vmCompilerString objectKey;
            vmCompilerMetaOperator * operatorMeta;
        };
        struct{
            vmMetaOffset valueOffset;
        } binary;
    } vmCompilerMeta;
    
    OBJV_KEY_DEC(vmCompilerMeta)
    
    extern objv_class_t vmCompilerMetaClass;

    typedef struct _vmCompilerClassMeta{
        objv_object_t base;
        vmCompilerString superClass;
        objv_array_t * propertys;
        objv_array_t * functions;
        struct{
            vmMetaOffset superClass;
            vmMetaOffset className;
            vmMetaOffset offset;
            unsigned char UDID[16];
        } binary;
    }vmCompilerClassMeta;
    
    OBJV_KEY_DEC(vmCompilerClassMeta)
    
    extern objv_class_t vmCompilerClassMetaClass;

    vmCompilerMetaOperator * vmCompilerMetaOperatorNew(objv_zone_t * zone, vmOperatorType type,objv_tokenizer_location_t location,vm_uint32_t length);
    
    void vmCompilerMetaOperatorAddCompilerMeta(vmCompilerMetaOperator * op,vmCompilerMeta * compilerMeta);
    
    vmCompilerMeta * vmCompilerMetaNew(objv_zone_t * zone);
    
    vmCompilerMeta * vmCompilerMetaNewWithInt32(objv_zone_t * zone,vm_int32_t value);
    
    vmCompilerMeta * vmCompilerMetaNewWithInt64(objv_zone_t * zone,vm_int64_t value);
    
    vmCompilerMeta * vmCompilerMetaNewWithBoolean(objv_zone_t * zone,vm_boolean_t value);
    
    vmCompilerMeta * vmCompilerMetaNewWithDouble(objv_zone_t * zone,vm_double_t value);
    
    vmCompilerMeta * vmCompilerMetaNewWithString(objv_zone_t * zone,objv_tokenizer_location_t location,vm_uint32_t length);
    
    vmCompilerMeta * vmCompilerMetaNewWithObjectKey(objv_zone_t * zone,objv_tokenizer_location_t location,vm_uint32_t length);
    
    vmCompilerMeta * vmCompilerMetaNewWithOperator(objv_zone_t * zone,vmCompilerMetaOperator * op);
    
    vmCompilerMeta * vmCompilerMetaNewWithNumberString(objv_zone_t * zone,objv_tokenizer_location_t location,vm_uint32_t length);
    
    vmCompilerMeta * vmCompilerMetaNewWithArg(objv_zone_t * zone,objv_tokenizer_location_t location,vm_uint32_t length);
    
    vmCompilerClassMeta * vmCompilerClassMetaNew(objv_zone_t * zone);
    
    void vmCompilerClassAddProperty(vmCompilerClassMeta * classMeta,vmCompilerMetaOperator * op);
    
    void vmCompilerClassAddFunction(vmCompilerClassMeta * classMeta,vmCompilerMetaOperator * op);
    
    void vmCompilerClassMetaLog(vmCompilerClassMeta * classMeta);
    
    
    typedef struct _vmCompilerError {
        objv_object_t base;
        objv_tokenizer_location_t location;
        objv_mbuf_t error;
    } vmCompilerError;
    
    OBJV_KEY_DEC(vmCompilerError)
    
    extern objv_class_t vmCompilerErrorClass;
    
    vmCompilerError * vmCompilerErrorNew(objv_zone_t * zone, objv_tokenizer_location_t location,const char * format,...);
    
    vmCompilerError * vmCompilerErrorNewV(objv_zone_t * zone,objv_tokenizer_location_t location,const char * format,va_list ap);
    
    void vmCompilerErrorSet(objv_array_t * errors,objv_tokenizer_location_t location,const char * format,...);
    
    void vmCompilerErrorsLog(objv_array_t * errors);
    
#ifdef __cplusplus
}
#endif



#endif
