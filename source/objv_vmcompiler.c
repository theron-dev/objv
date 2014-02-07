//
//  objv_vermin_compiler.c
//  objv
//
//  Created by zhang hailong on 14-2-7.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv.h"
#include "objv_vmcompiler.h"
#include "objv_autorelease.h"
#include "objv_mbuf.h"
#include "objv_log.h"

OBJV_KEY_IMP(vmCompilerMetaOperator)

static void vmCompilerMetaOperatorMethodDealloc(objv_class_t * clazz,objv_object_t * object){
    
    vmCompilerMetaOperator * op = (vmCompilerMetaOperator *) object;

    objv_object_release((objv_object_t *) op->metas);
    
}

static objv_object_t * vmCompilerMetaOperatorMethodCopy(objv_class_t * clazz,objv_object_t * object){
    
    vmCompilerMetaOperator * obj = (vmCompilerMetaOperator *) object;
    vmCompilerMetaOperator * op = vmCompilerMetaOperatorNew(object->zone, obj->type, obj->uniqueKey.location, obj->uniqueKey.length);
    vmCompilerMeta * meta;
    for(int i=0;i< obj->metas->length;i++){
        meta = (vmCompilerMeta *) objv_array_objectAt(obj->metas, i);
        vmCompilerMetaOperatorAddCompilerMeta(op, (vmCompilerMeta *) objv_object_copy(meta->base.isa,(objv_object_t *) meta));
    }
    
    return (objv_object_t *) op;
}

static objv_object_t * vmCompilerMetaOperatorMethodInit(objv_class_t * clazz,objv_object_t * object,va_list ap){
    
    if(clazz->superClass){
        object = objv_object_initv(clazz->superClass,object,ap);
    }
    
    if(object){
        vmCompilerMetaOperator * op = (vmCompilerMetaOperator *) object;
        op->metas = objv_array_alloc(object->zone,4);
        op->type = va_arg(ap,vmOperatorType);
        op->uniqueKey.location = va_arg(ap,objv_tokenizer_location_t);
        op->uniqueKey.length = va_arg(ap,vm_uint32_t);
    }
    
    return object;
}

static objv_method_t vmCompilerMetaOperatorMethods[] = {
    {OBJV_KEY(dealloc),"v()",(objv_method_impl_t)vmCompilerMetaOperatorMethodDealloc}
    ,{OBJV_KEY(copy),"@()",(objv_method_impl_t)vmCompilerMetaOperatorMethodCopy}
    ,{OBJV_KEY(init),"@(*)",(objv_method_impl_t)vmCompilerMetaOperatorMethodInit}
};

objv_class_t vmCompilerMetaOperatorClass = {OBJV_KEY(vmCompilerMetaOperator),& objv_object_class
    ,vmCompilerMetaOperatorMethods,sizeof(vmCompilerMetaOperatorMethods) / sizeof(objv_method_t)
    ,NULL,0
    ,sizeof(vmCompilerMetaOperator)
    ,NULL,0,0};


vmCompilerMetaOperator * vmCompilerMetaOperatorNew(objv_zone_t * zone, vmOperatorType type,objv_tokenizer_location_t location,vm_uint32_t length){
    return (vmCompilerMetaOperator *) objv_object_new(zone,& vmCompilerMetaOperatorClass,type,location, length);
}

void vmCompilerMetaOperatorAddCompilerMeta(vmCompilerMetaOperator * op,vmCompilerMeta * compilerMeta){
    objv_array_add(op->metas, (objv_object_t *) compilerMeta);
}

OBJV_KEY_IMP(vmCompilerMeta)

static void vmCompilerMetaMethodDealloc(objv_class_t * clazz,objv_object_t * object){
    
    vmCompilerMeta * meta = (vmCompilerMeta *) object;
    
    if(meta->type & vmMetaTypeOperator){
        objv_object_release((objv_object_t *)meta->operatorMeta);
    }
}

static objv_object_t * vmCompilerMetaMethodCopy(objv_class_t * clazz,objv_object_t * object){
    
    vmCompilerMeta * meta = (vmCompilerMeta *) object;

    switch (meta->type) {
        case vmMetaTypeInt32:
            return (objv_object_t *) vmCompilerMetaNewWithInt32(object->zone,meta->int32Value);
            break;
        case vmMetaTypeInt64:
            return (objv_object_t *) vmCompilerMetaNewWithInt64(object->zone,meta->int64Value);
            break;
        case vmMetaTypeDouble:
            return (objv_object_t *) vmCompilerMetaNewWithDouble(object->zone,meta->doubleValue);
            break;
        case vmMetaTypeBoolean:
            return (objv_object_t *) vmCompilerMetaNewWithBoolean(object->zone,meta->booleanValue);
            break;
        case vmMetaTypeString:
            return (objv_object_t *) vmCompilerMetaNewWithString(object->zone,meta->stringValue.location,meta->stringValue.length);
            break;
        case vmMetaTypeObject:
            return (objv_object_t *) vmCompilerMetaNewWithObjectKey(object->zone,meta->objectKey.location,meta->objectKey.length);
            break;
        case vmMetaTypeOperator:
            return (objv_object_t *) vmCompilerMetaNewWithOperator(object->zone,meta->operatorMeta);
            break;
        case vmMetaTypeArg:
            return (objv_object_t *) vmCompilerMetaNewWithArg(object->zone,meta->stringValue.location,meta->stringValue.length);
        default:
            return (objv_object_t *) vmCompilerMetaNew(object->zone);
            break;
    }
    
}

static objv_object_t * vmCompilerMetaMethodInit(objv_class_t * clazz,objv_object_t * object,va_list ap){
    
    if(clazz->superClass){
        object = objv_object_initv(clazz->superClass,object,ap);
    }
    
    if(object){
        
        vmCompilerMeta * meta = (vmCompilerMeta *) object;
        meta->type = va_arg(ap,vmMetaType);
        
        switch (meta->type) {
            case vmMetaTypeInt32:
                meta->int32Value = va_arg(ap,vm_int32_t);
                break;
            case vmMetaTypeInt64:
                meta->int64Value = va_arg(ap,vm_int64_t);
                break;
            case vmMetaTypeDouble:
                meta->doubleValue = va_arg(ap,vm_double_t);
                break;
            case vmMetaTypeBoolean:
                meta->booleanValue = va_arg(ap,vm_boolean_t);
                break;
            case vmMetaTypeString:
                meta->stringValue.location = va_arg(ap,objv_tokenizer_location_t);
                meta->stringValue.length = va_arg(ap,vm_uint32_t);
                break;
            case vmMetaTypeObject:
                meta->objectKey.location = va_arg(ap,objv_tokenizer_location_t);
                meta->objectKey.length = va_arg(ap,vm_uint32_t);
                break;
            case vmMetaTypeOperator:
                meta->operatorMeta = (vmCompilerMetaOperator * ) objv_object_retain( va_arg(ap,objv_object_t *));
                break;
            case vmMetaTypeArg:
                meta->stringValue.location = va_arg(ap,objv_tokenizer_location_t);
                meta->stringValue.length = va_arg(ap,vm_uint32_t);
            default:
                break;
        }
        
    }
    
    return object;
}

static objv_method_t vmCompilerMetaMethods[] = {
    {OBJV_KEY(dealloc),"v()",(objv_method_impl_t)vmCompilerMetaMethodDealloc}
    ,{OBJV_KEY(copy),"@()",(objv_method_impl_t)vmCompilerMetaMethodCopy}
    ,{OBJV_KEY(init),"@(*)",(objv_method_impl_t)vmCompilerMetaMethodInit}
};

objv_class_t vmCompilerMetaClass = {OBJV_KEY(vmCompilerMeta),& objv_object_class
    ,vmCompilerMetaMethods,sizeof(vmCompilerMetaMethods) / sizeof(objv_method_t)
    ,NULL,0
    ,sizeof(vmCompilerMeta)
    ,NULL,0,0};



vmCompilerMeta * vmCompilerMetaNew(objv_zone_t * zone){
    return (vmCompilerMeta *) objv_object_new(zone,&vmCompilerMetaClass, vmMetaTypeVoid);
}


vmCompilerMeta * vmCompilerMetaNewWithInt32(objv_zone_t * zone,vm_int32_t value){
    return (vmCompilerMeta *) objv_object_new(zone,&vmCompilerMetaClass, vmMetaTypeInt32,value);
}

vmCompilerMeta * vmCompilerMetaNewWithInt64(objv_zone_t * zone,vm_int64_t value){
    return (vmCompilerMeta *) objv_object_new(zone,&vmCompilerMetaClass, vmMetaTypeInt64,value);
}

vmCompilerMeta * vmCompilerMetaNewWithBoolean(objv_zone_t * zone,vm_boolean_t value){
    return (vmCompilerMeta *) objv_object_new(zone,&vmCompilerMetaClass, vmMetaTypeBoolean,value);
}

vmCompilerMeta * vmCompilerMetaNewWithDouble(objv_zone_t * zone,vm_double_t value){
    return (vmCompilerMeta *) objv_object_new(zone,&vmCompilerMetaClass, vmMetaTypeDouble,value);
}

vmCompilerMeta * vmCompilerMetaNewWithString(objv_zone_t * zone,objv_tokenizer_location_t location,vm_uint32_t length){
    return (vmCompilerMeta *) objv_object_new(zone,&vmCompilerMetaClass, vmMetaTypeString,location,length);
}

vmCompilerMeta * vmCompilerMetaNewWithObjectKey(objv_zone_t * zone,objv_tokenizer_location_t location,vm_uint32_t length){
    return (vmCompilerMeta *) objv_object_new(zone,&vmCompilerMetaClass, vmMetaTypeObject,location,length);
}

vmCompilerMeta * vmCompilerMetaNewWithOperator(objv_zone_t * zone,vmCompilerMetaOperator * op){
    return (vmCompilerMeta *) objv_object_new(zone,&vmCompilerMetaClass, vmMetaTypeOperator,op);
}


vmCompilerMeta * vmCompilerMetaNewWithNumberString(objv_zone_t * zone,objv_tokenizer_location_t location,vm_uint32_t length){
    vmCompilerMeta * meta = NULL;
    objv_mbuf_t buf;
    vm_double_t doubleValue ;
    vm_int64_t int64Value;
    vm_int32_t int32Value;
    char * value;
    
    objv_mbuf_init( & buf, 64);
    
    objv_mbuf_append(& buf, location.p, length);
    
    value = (char *)objv_mbuf_str(& buf);
    
    doubleValue = atof(value);
    int64Value = (vm_int64_t) doubleValue;
    int32Value = (vm_int32_t) doubleValue;
    
    if(value && value[0] == '0' && value[1] == 'x'){
        sscanf(value, "%llx",&int64Value);
        int32Value = (vm_int32_t) int64Value;
        if(int32Value == int64Value){
            meta = vmCompilerMetaNewWithInt32(zone,int32Value);
        }
        else{
            meta = vmCompilerMetaNewWithInt64(zone,int64Value);
        }
    }
    else if(value && value[0] == '0' && value[1] != '.'){
        sscanf(value, "%llo",&int64Value);
        int32Value = (vm_int32_t) int64Value;
        if(int32Value == int64Value){
            meta = vmCompilerMetaNewWithInt32(zone,int32Value);
        }
        else{
            meta = vmCompilerMetaNewWithInt64(zone,int64Value);
        }
    }
    else if(objv_string_indexOf(value, ".") || objv_string_indexOf(value, "e")){
        meta = vmCompilerMetaNewWithDouble(zone,doubleValue);
    }
    else if(doubleValue == (vm_double_t) int64Value){
        if(int32Value == int64Value){
            meta = vmCompilerMetaNewWithInt32(zone,int32Value);
        }
        else{
            meta = vmCompilerMetaNewWithInt64(zone,int64Value);
        }
    }
    else{
        meta = vmCompilerMetaNewWithDouble(zone,doubleValue);
    }
  
    objv_mbuf_destroy(& buf);
    
    return meta;
}

vmCompilerMeta * vmCompilerMetaNewWithArg(objv_zone_t * zone,objv_tokenizer_location_t location,vm_uint32_t length){
    return (vmCompilerMeta *) objv_object_new(zone,&vmCompilerMetaClass, vmMetaTypeArg,location,length);
}



OBJV_KEY_IMP(vmCompilerClassMeta)

static void vmCompilerClassMetaMethodDealloc(objv_class_t * clazz,objv_object_t * object){
    
    vmCompilerClassMeta * meta = (vmCompilerClassMeta *) object;
    
    objv_object_release((objv_object_t *) meta->propertys);
    objv_object_release((objv_object_t *) meta->functions);
    
}

static objv_object_t * vmCompilerClassMetaMethodCopy(objv_class_t * clazz,objv_object_t * object){
    
    vmCompilerClassMeta * rs = vmCompilerClassMetaNew(object->zone);
    vmCompilerClassMeta * meta = (vmCompilerClassMeta *) object;
    
    int i;
    objv_object_t * o;
    
    for(i=0;i<meta->propertys->length;i++){
        o = objv_array_objectAt(meta->propertys, i);
        objv_array_add(rs->propertys, objv_object_copy(o->isa, o));
    }
    
    for(i=0;i<meta->functions->length;i++){
        o = objv_array_objectAt(meta->functions, i);
        objv_array_add(rs->functions, objv_object_copy(o->isa, o));
    }
    
    return (objv_object_t *) rs;
}

static objv_object_t * vmCompilerClassMetaMethodInit(objv_class_t * clazz,objv_object_t * object,va_list ap){
    
    if(clazz->superClass){
        object = objv_object_initv(clazz->superClass,object,ap);
    }
    
    if(object){
        
        vmCompilerClassMeta * meta = (vmCompilerClassMeta *) object;
        
        meta->propertys = objv_array_alloc(object->zone, 4);
        meta->functions = objv_array_alloc(object->zone, 4);
        
    }
    
    return object;
}

static objv_method_t vmCompilerClassMetaMethods[] = {
    {OBJV_KEY(dealloc),"v()",(objv_method_impl_t)vmCompilerClassMetaMethodDealloc}
    ,{OBJV_KEY(copy),"@()",(objv_method_impl_t)vmCompilerClassMetaMethodCopy}
    ,{OBJV_KEY(init),"@(*)",(objv_method_impl_t)vmCompilerClassMetaMethodInit}
};

objv_class_t vmCompilerClassMetaClass = {OBJV_KEY(vmCompilerClassMeta),& objv_object_class
    ,vmCompilerClassMetaMethods,sizeof(vmCompilerClassMetaMethods) / sizeof(objv_method_t)
    ,NULL,0
    ,sizeof(vmCompilerClassMeta)
    ,NULL,0,0};


vmCompilerClassMeta * vmCompilerClassMetaNew(objv_zone_t * zone){
    return (vmCompilerClassMeta *) objv_object_new(zone,&vmCompilerClassMetaClass);
}

void vmCompilerClassAddProperty(vmCompilerClassMeta * classMeta,vmCompilerMetaOperator * op){
    objv_array_add(classMeta->propertys, (objv_object_t *) op);
}

void vmCompilerClassAddFunction(vmCompilerClassMeta * classMeta,vmCompilerMetaOperator * op){
    objv_array_add(classMeta->functions, (objv_object_t *) op);
}


OBJV_KEY_IMP(vmCompilerError)

static void vmCompilerErrorMethodDealloc(objv_class_t * clazz,objv_object_t * object){
    
    vmCompilerError * error = (vmCompilerError *) object;
    
    objv_mbuf_destroy(& error->error);
    
}

static objv_object_t * vmCompilerErrorMethodInit(objv_class_t * clazz,objv_object_t * object,va_list ap){
    
    if(clazz->superClass){
        object = objv_object_initv(clazz->superClass,object,ap);
    }
    
    if(object){
        
        vmCompilerError * meta = (vmCompilerError *) object;
        
        meta->location = va_arg(ap, objv_tokenizer_location_t);
        
        const char * format = va_arg(ap, const char *);
        va_list * va = va_arg(ap, va_list * );
        
        objv_mbuf_init(& meta->error, 64);
        
        if(format){
            objv_mbuf_formatv(& meta->error, format, * va);
        }
    }
    
    return object;
}

static objv_method_t vmCompilerErrorMethods[] = {
    {OBJV_KEY(dealloc),"v()",(objv_method_impl_t)vmCompilerErrorMethodDealloc}
    ,{OBJV_KEY(init),"@(*)",(objv_method_impl_t)vmCompilerErrorMethodInit}
};

objv_class_t vmCompilerErrorClass = {OBJV_KEY(vmCompilerError),& objv_object_class
    ,vmCompilerErrorMethods,sizeof(vmCompilerErrorMethods) / sizeof(objv_method_t)
    ,NULL,0
    ,sizeof(vmCompilerError)
    ,NULL,0,0};


vmCompilerError * vmCompilerErrorNew(objv_zone_t * zone, objv_tokenizer_location_t location,const char * format,...){
    
    va_list ap;
    vmCompilerError * e;
    va_start(ap, format);
    
    e = vmCompilerErrorNewV(zone,location,format,ap);
    
    va_end(ap);
    
    return e;
}

vmCompilerError * vmCompilerErrorNewV(objv_zone_t * zone,objv_tokenizer_location_t location,const char * format,va_list ap){
    return (vmCompilerError *) objv_object_new(zone, & vmCompilerErrorClass, location,format,& ap);
}


static void vmCompilerStringLog(vmCompilerString string){
    vm_uint32_t l = string.length;
    char * p = string.location.p;
    
    while(l >0){
        objv_log("%c",*p);
        l --;
        p++;
    }
}

void vmCompilerMetaOperatorLog(vmCompilerMetaOperator * metaOperator){
    vmCompilerStringLog(metaOperator->uniqueKey);
    objv_log("\n");
}

void vmCompilerClassMetaLog(vmCompilerClassMeta * classMeta){
    vm_uint32_t i;
    vmCompilerMetaOperator * op;
    objv_log("extends ");
    
    vmCompilerStringLog(classMeta->superClass);
    
    objv_log("\n");
    
    objv_log("propertys:\n");
    
    for(i=0;i<classMeta->propertys->length;i++){
        op = (vmCompilerMetaOperator *) objv_array_objectAt(classMeta->propertys, i);
        vmCompilerMetaOperatorLog(op);
    }
    
    objv_log("functions:\n");

    for(i=0;i<classMeta->functions->length;i++){
        op = (vmCompilerMetaOperator *) objv_array_objectAt(classMeta->functions, i);
        vmCompilerMetaOperatorLog(op);
    }
}

void vmCompilerErrorSet(objv_array_t * errors,objv_tokenizer_location_t location,const char * format,...){
    
    vmCompilerError * e;
    va_list ap;
    
    va_start(ap, format);
    
    e = vmCompilerErrorNewV(errors->base.zone,location,format,ap);
    
    va_end(ap);
    
    objv_array_add(errors, (objv_object_t *) e);
}

