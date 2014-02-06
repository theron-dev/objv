//
//  objv_vermin.h
//  objv
//
//  Created by zhang hailong on 14-2-4.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_vermin_h
#define objv_objv_vermin_h


#ifdef __cplusplus
extern "C" {
#endif
    
#define vmVersion  "1.0.0"
#define vmHeadTag     "__objv_vm__"
    
#include "objv.h"
#include "objv_array.h"
#include "objv_hash_map.h"
#include "objv_string.h"
    
    typedef int vm_int32_t;
    typedef unsigned int vm_uint32_t;
    typedef long long vm_int64_t;
    typedef double vm_double_t;
    typedef int vm_boolean_t;
    typedef const char * vm_string_t;
    
    extern vm_boolean_t vm_true;
    extern vm_boolean_t vm_false;
    
    enum {
        vmVariantTypeVoid = 0
        ,vmVariantTypeInt32 = 1<<0
        ,vmVariantTypeInt64 = 1<<1
        ,vmVariantTypeBoolean = 1<<2
        ,vmVariantTypeDouble = 1<<3
        ,vmVariantTypeObject = 1<<4
        ,vmVariantTypeString = 1<<5
        ,vmVariantTypeThrowable = 1<<6
        ,vmVariantTypeWeak = 1<<7
    };
    
    typedef vm_uint32_t vmVariantType;
    
    typedef struct _vmVariant{
        vmVariantType type;
        union{
            vm_int32_t int32Value;
            vm_int64_t int64Value;
            vm_boolean_t booleanValue;
            vm_double_t doubleValue;
            vm_string_t stringValue;
            objv_object_t * objectValue;
        };
        objv_class_t * ofClass;
    } vmVariant;
    
    
    enum{
        vmOperatorTypeNone
        
        ,vmOperatorTypeNew             // new
        ,vmOperatorTypeFunction        // ()
        
        ,vmOperatorTypeInvoke      // {}
        ,vmOperatorTypeProperty   // .　[]
        
        ,vmOperatorTypeInc           // a++
        ,vmOperatorTypeDec           // a--
        ,vmOperatorTypeBeforeInc     // ++a
        ,vmOperatorTypeBeforeDec     // --a
        ,vmOperatorTypeAdd           // +
        ,vmOperatorTypeSub           // -
        ,vmOperatorTypeMultiplication// *
        ,vmOperatorTypeDivision      // /
        ,vmOperatorTypeModulus       // %
        ,vmOperatorTypeBitwiseAnd    // &
        ,vmOperatorTypeBitwiseOr     // |
        ,vmOperatorTypeBitwiseExclusiveOr        // ^
        ,vmOperatorTypeBitwiseComplement         // ~
        ,vmOperatorTypeBitwiseShiftLeft          // <<
        ,vmOperatorTypeBitwiseShiftRight         // >>
        ,vmOperatorTypeNot           // !
        ,vmOperatorTypeAnd           // &&
        ,vmOperatorTypeOr            // ||
        ,vmOperatorTypeGreater       // >
        ,vmOperatorTypeGreaterEqual  // >=
        ,vmOperatorTypeLess          // <
        ,vmOperatorTypeLessEqual     // <=
        ,vmOperatorTypeEqual         // ==
        ,vmOperatorTypeAbsEqual      // ===
        ,vmOperatorTypeNotEqual      // !=
        ,vmOperatorTypeAbsNotEqual   // !==
        ,vmOperatorTypeAssign        // =
        ,vmOperatorTypeIfElse        // ? :
        ,vmOperatorTypeAntiNumber    // -
        
        ,vmOperatorTypeIfElseIfElse  // if() {}  else if() {}  else {}
        ,vmOperatorTypeFor           // for
        ,vmOperatorTypeWhile         // while
        ,vmOperatorTypeForeach       // for in
        
        ,vmOperatorTypeTryCatchFinally // try
        
        ,vmOperatorTypeThrow         // throw
        
        ,vmOperatorTypeVar           // var
        
        ,vmOperatorTypeIs            // is
        
        ,vmOperatorTypeVarWeak       // weak
        
    };
    
    typedef vm_uint32_t vmOperatorType;
    
    enum{
        vmMetaTypeVoid = vmVariantTypeVoid
        ,vmMetaTypeInt32 = vmVariantTypeInt32
        ,vmMetaTypeInt64 = vmVariantTypeInt64
        ,vmMetaTypeBoolean = vmVariantTypeBoolean
        ,vmMetaTypeDouble = vmVariantTypeDouble
        ,vmMetaTypeObject = vmVariantTypeObject
        ,vmMetaTypeString = vmVariantTypeString
        ,vmMetaTypeThrowable = vmVariantTypeThrowable
        ,vmMetaTypeWeak = vmVariantTypeWeak
        
        ,vmMetaTypeReturn   = 1<<8
        ,vmMetaTypeOperator = 1<<9
        ,vmMetaTypeBreak    = 1<<10
        ,vmMetaTypeContinue  = 1 <<11
        ,vmMetaTypeArg   = 1 << 12
        ,vmMetaTypeCatch = 1 << 13
        ,vmMetaTypeFinally = 1 << 14
    };
    
    typedef vm_uint32_t vmMetaType;
    
    typedef vm_uint32_t vmMetaOffset;
   
    typedef struct _vmMeta{
        vmMetaType type;
        union{
            vm_int32_t int32Value;
            vm_int64_t int64Value;
            vm_boolean_t booleanValue;
            vm_double_t doubleValue;
            vmMetaOffset stringKey;
            vmMetaOffset objectKey;
            vmMetaOffset operatorOffset;
        };
    } vmMeta;
    
    typedef struct _vmMetaOperator{
        vmOperatorType type;
        vmMetaOffset uniqueKey;
        vm_uint32_t metaCount;
    } vmMetaOperator;
    
    typedef struct _vmMetaClass{
        vmMetaOffset offset;
        vmMetaOffset uniqueKey;
        vmMetaOffset superClass;
        vm_uint32_t propertyCount;
        vm_uint32_t functionCount;
    } vmMetaClass;
    
    typedef struct _vmMetaBinary {
        struct {
            char tag[16];
            char version[16];
            char high;
            char low;
        } head;
        vm_uint32_t classCount;
        vm_uint32_t uniqueKeyCount;
        unsigned char UDID[16];
        vm_uint32_t length;
        vm_uint32_t verify;
    } vmMetaBinary;
    
    objv_boolean_t vmMetaBinaryValidate(vmMetaBinary * binary);
    
    void vmMetaBinarySignature(vmMetaBinary * binary);
    
    struct _vmContext;
    
    typedef struct _vmClass{
        objv_class_t base;
        struct _vmContext * READONLY ctx;
        vmMetaClass * READONLY vmClass;
        objv_hash_map_t * READONLY propertys;
        objv_hash_map_t * READONLY functions;
    } vmClass;
    
    
    typedef struct _vmBinary {
        vmMetaBinary * binary;
        objv_boolean_t copyed;
    } vmBinary;
    
    typedef struct _vmContextScope {
        objv_hash_map_t * variants;
    } vmContextScope;
    
    typedef struct _vmContext {
        objv_object_t base;
        struct {
            vmBinary * binarys;
            vm_uint32_t size;
            vm_uint32_t length;
        } binarys;
        objv_hash_map_t * classMap;
        objv_hash_map_t * keyMap;
        struct {
            objv_key_t * init;
            objv_key_t * destroy;
            objv_key_t * thisKey;
            objv_key_t * superKey;
            objv_key_t * argumentsKey;
        } keys;
        struct {
            vmContextScope * scope;
            vm_uint32_t size;
            vm_uint32_t length;
        } scopes;
    } vmContext;
    
    OBJV_KEY_DEC(vmContext)
    OBJV_KEY_DEC(invoke)
    OBJV_KEY_DEC(property);
    OBJV_KEY_DEC(setProperty);
    OBJV_KEY_DEC(initWithContext);
    
    extern objv_class_t vmContextClass;
    
    typedef struct _vmObject {
        objv_object_t base;
        vmContext * READONLY ctx;
    } vmObject;
    
    typedef vmVariant (* vmObjectInitWithContextFun) (objv_class_t * clazz, objv_object_t * object,vmContext * context,objv_array_t * arguments);
    
    typedef vmVariant (* vmObjectMethodInvokeFun) (objv_class_t * clazz,objv_object_t * object, objv_key_t * name,objv_array_t * arguments);
    
    typedef vmVariant (* vmObjectMethodPropertyFun) (objv_class_t * clazz,objv_object_t * object, objv_key_t * name);
    
    typedef vmVariant (* vmObjectMethodSetPropertyFun) (objv_class_t * clazz,objv_object_t * object, objv_key_t * name,vmVariant value);
    
    
    vmContext * vmContextAlloc(objv_zone_t * zone);
    
    void vmContextScopePush(vmContext * ctx);
    
    void vmContextScopePop(vmContext * ctx);
    
    void vmContextSetVariant(vmContext * ctx,objv_key_t * key, vmVariant variant);
    
    vmVariant vmContextVariant(vmContext * ctx,objv_key_t * key);
    
    objv_boolean_t vmContextLoadBinary(vmContext * ctx,vmMetaBinary * binary,objv_boolean_t copy);
    
    objv_class_t * vmContextGetClass(vmContext * ctx, objv_key_t * key);
    
    objv_key_t * vmContextKey(vmContext * ctx, const char * key);
    
    vmVariant vmObjectNew(vmContext * ctx,objv_class_t * clazz,objv_array_t * arguments);
    
    vmVariant vmObjectInvoke(objv_class_t * clazz, objv_object_t * object,objv_key_t * key, objv_array_t * arguments);
    
    vmVariant vmObjectGetProperty(objv_class_t * clazz,objv_object_t * object, objv_key_t * key );
    
    vmVariant vmObjectSetProperty(objv_class_t * clazz,objv_object_t * object, objv_key_t * key , vmVariant value);
    
    vmVariant vmObjectOperatorExecute(vmContext * ctx,objv_class_t * clazz,objv_object_t * object, void * binary,vmMetaOperator * op);
    
    vmVariant vmObjectOperatorFunction(vmContext * ctx,objv_class_t * clazz,objv_object_t * object, void * binary,vmMetaOperator * op,objv_array_t * arguments);
    
    objv_boolean_t vmIsClass(objv_class_t * clazz);
    
    vmVariant vmVariantToObject(objv_zone_t * zone, vmVariant var);
    
    vm_int64_t vmVariantToInt64(vmVariant var);
    
    vm_int32_t vmVariantToInt32(vmVariant var);
    
    objv_string_t * vmVariantToString(objv_zone_t * zone,vmVariant var);
    
    void vmVariantToStringBuffer(vmVariant var,objv_mbuf_t * mbuf);
    
    vm_boolean_t vmVariantToBoolean(vmVariant var);
    
    vm_double_t vmVariantToDouble(vmVariant var);

    vmVariant vmObjectToVariant(objv_object_t * object);

#ifdef __cplusplus
}
#endif



#endif
