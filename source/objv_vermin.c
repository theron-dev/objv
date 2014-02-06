//
//  objv_vermin.c
//  objv
//
//  Created by zhang hailong on 14-2-4.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv_vermin.h"
#include "objv_autorelease.h"
#include "objv_log.h"
#include "objv_value.h"


OBJV_KEY_DEC(vmObjectIterator)
OBJV_KEY_IMP(vmObjectIterator)

typedef struct _vmObjectIterator {
    objv_iterator_t base;
    vmObject * object;
    objv_array_t * keys;
    vm_uint32_t index;
} vmObjectIterator;


static objv_object_t * vmObjectIteratorMethodsNext(objv_class_t * clazz, objv_object_t * obj){
    vmObjectIterator * iterator = (vmObjectIterator *) obj;
    return objv_array_objectAt(iterator->keys, iterator->index ++);
}

static objv_object_t * vmObjectIteratorMethodsAlloc (objv_class_t * clazz, objv_object_t * object,va_list ap){
    
    vmObjectIterator * iterator = (vmObjectIterator *) object;
    
    objv_hash_map_t * keys = objv_hash_map_alloc(64, objv_hash_map_hash_code_key, objv_map_compare_key);
    
    objv_class_t * c = clazz ;
    
    vmClass * vmclass;
    
    vm_uint32_t i;
    
    char * binary ;
    
    vmMetaOperator * op;
    
    objv_key_t * key;
    
    objv_string_t * skey;
    
    iterator->object = (vmObject *) objv_object_retain((objv_object_t *) va_arg(ap, objv_object_t *));
    iterator->keys = objv_array_alloc(iterator->object->base.zone, 32);
    
    while(vmIsClass(c)){
        
        vmclass = (vmClass *) c;
        
        binary = (char *) vmclass->vmClass - vmclass->vmClass->offset;
        
        if(vmclass->propertys){
            for(i=0;i<vmclass->propertys->length;i++){
                op = (vmMetaOperator *) objv_hash_map_valueAt(vmclass->propertys, i);
                key = vmContextKey(iterator->object->ctx, binary + op->uniqueKey);
                
                if(objv_hash_map_get(keys, key) == NULL){
                    
                    skey = objv_string_alloc_nocopy(iterator->object->base.zone, key->name);
                    
                    objv_array_add(iterator->keys, (objv_object_t *) skey);
                    
                    objv_object_release((objv_object_t *) skey);
                    
                    objv_hash_map_put(keys, key, key);
                }
            }
        }
        
        c = c->superClass;
        
    }
    
    
    objv_hash_map_dealloc(keys);
    
    return object;
}

static void vmObjectIteratorMethodsDealloc (objv_class_t * clazz, objv_object_t * object){
    
    vmObjectIterator * iterator = (vmObjectIterator *) object;
    
    objv_object_release( ( objv_object_t *) iterator->object );
    objv_object_release( ( objv_object_t *) iterator->keys );
    
    if(clazz->superClass){
        
        objv_object_dealloc(clazz->superClass, object);
        
    }
}

static objv_method_t vmObjectIteratorMethods[] = {
    {OBJV_KEY(next),"@()",(objv_method_impl_t)vmObjectIteratorMethodsNext}
    ,{OBJV_KEY(alloc),"@(@)",(objv_method_impl_t)vmObjectIteratorMethodsAlloc}
    ,{OBJV_KEY(dealloc),"v()",(objv_method_impl_t)vmObjectIteratorMethodsDealloc}
};


static objv_class_t vmObjectIteratorClass = {OBJV_KEY(vmObjectIterator),& objv_iterator_class
    ,vmObjectIteratorMethods,sizeof(vmObjectIteratorMethods) / sizeof(objv_method_t)
    ,NULL,0
    ,sizeof(vmObjectIterator)
    ,NULL,0,0};



OBJV_KEY_IMP(vmContext)
OBJV_KEY_IMP(invoke)
OBJV_KEY_IMP(property);
OBJV_KEY_IMP(setProperty);
OBJV_KEY_IMP(initWithContext);

typedef union _BitsSign{
    struct{
        char high;
        char low;
    } bits;
    unsigned short int16;
} BitsSign;

void vmMetaBinarySignature(vmMetaBinary * binary){
   
    BitsSign bits;
    unsigned char * p = (unsigned char *) binary;
    vm_uint32_t len = sizeof(vmMetaBinary) - sizeof(vm_uint32_t);
    vm_uint32_t verify = 0;
    
    bits.int16 = 0x0100;
    
    strncpy(binary->head.tag, vmHeadTag, sizeof(binary->head.tag));
    strncpy(binary->head.version, vmVersion, sizeof(binary->head.version));
    
    binary->head.high = bits.bits.high;
    binary->head.low = bits.bits.low;
    
    
    while (p && len > 0) {
        verify += * p;
        len --;
        p ++;
    }
    
    binary->verify = verify;
    
}

objv_boolean_t vmMetaBinaryValidate(vmMetaBinary * binary){
    
    BitsSign bits;
    unsigned char * p = (unsigned char *) binary;
    vm_uint32_t len = sizeof(vmMetaBinary) - sizeof(vm_uint32_t);
    vm_uint32_t verify = 0;
    
    bits.int16 = 0x0100;
    
    while (p && len > 0) {
        verify += * p;
        len --;
        p ++;
    }
    
    return binary && binary->verify == verify && binary->length >= sizeof(vmMetaBinary)
                && strncmp(binary->head.tag, vmHeadTag, sizeof(binary->head.tag)) == 0
                && strncmp(binary->head.version, vmVersion, sizeof(binary->head.version)) == 0
                && binary->head.high == bits.bits.high && binary->head.low == bits.bits.low;
    
    
}

static void vmContextDealloc(objv_class_t * clazz, objv_object_t * obj){
    
    vmContext * ctx = (vmContext *) obj;
    
    vmBinary * binary;
    objv_key_t * key;
    vmClass * c;
    int i,n;
    vmContextScope * scope ;
    vmVariant * var;
    
    for(i=0;i<ctx->scopes.length;i++){
        
        scope = ctx->scopes.scope + i;
        
        if(scope->variants){
            for(n=0; n< scope->variants->length;n++){
                var = objv_hash_map_valueAt(scope->variants, n);
                if((var->type & vmVariantTypeObject) && var->objectValue && ! (var->type & vmVariantTypeWeak)){
                    objv_object_release((objv_object_t *) var->objectValue);
                }
                objv_zone_free(ctx->base.zone, var);
            }
            
            objv_hash_map_dealloc(scope->variants);
        }
        
    }
    
    if(ctx->scopes.scope){
        objv_zone_free(ctx->base.zone, ctx->scopes.scope);
    }
    
    for(i=0;i<ctx->keyMap->length;i++){
        
        key = (objv_key_t *) objv_hash_map_valueAt(ctx->keyMap, i);
        
        objv_zone_free(obj->zone, key);
        
    }
    
    for(i=0;i<ctx->binarys.length;i++){
        
        binary = ctx->binarys.binarys + i;
        
        if(binary->copyed){
            objv_zone_free(obj->zone, binary->binary);
        }
        
    }
    
    for(i=0;i<ctx->classMap->length;i++){
        c = (vmClass *) objv_hash_map_valueAt(ctx->classMap, i);
        if(c->functions){
            objv_hash_map_dealloc(c->functions);
        }
        if(c->propertys){
            objv_hash_map_dealloc(c->propertys);
        }
        objv_zone_free(obj->zone, c);
    }
    
    objv_hash_map_dealloc(ctx->classMap);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass,obj);
    }
}


static objv_method_t vmContextMethods[] = {
    {OBJV_KEY(dealloc),"v()",(objv_method_impl_t)vmContextDealloc}
};

objv_class_t vmContextClass = {OBJV_KEY(vmContext),& objv_object_class
    ,vmContextMethods,sizeof(vmContextMethods) / sizeof(objv_method_t)
    ,NULL,0
    ,sizeof(vmContext)
    ,NULL,0,0};


vmContext * vmContextAlloc(objv_zone_t * zone){
    
    vmContext * ctx = (vmContext *) objv_object_alloc(zone, & vmContextClass);
    
    ctx->classMap = objv_hash_map_alloc(20, objv_hash_map_hash_code_ptr, objv_map_compare_any);
    ctx->keyMap = objv_hash_map_alloc(20, objv_hash_map_hash_code_string, objv_map_compare_string);
    
    ctx->keys.init = vmContextKey(ctx, "init");
    ctx->keys.destroy = vmContextKey(ctx, "destroy");
    ctx->keys.thisKey = vmContextKey(ctx, "this");
    ctx->keys.superKey = vmContextKey(ctx, "super");
    ctx->keys.argumentsKey = vmContextKey(ctx, "arguments");
    
    vmContextScopePush(ctx);
    
    return ctx;
}

void vmContextScopePush(vmContext * ctx){
    if(ctx){
        
        if(ctx->scopes.length +1 > ctx->scopes.size){
            ctx->scopes.size += 20;
            if(ctx->scopes.scope){
                ctx->scopes.scope = objv_zone_realloc(ctx->base.zone, ctx->scopes.scope, sizeof(vmContextScope) * ctx->scopes.size);
            }
            else{
                ctx->scopes.scope = objv_zone_malloc(ctx->base.zone, sizeof(vmContextScope) * ctx->scopes.size);
            }
        }
        
        objv_zone_memzero(ctx->base.zone, ctx->scopes.scope + ctx->scopes.length ++, sizeof(vmContextScope));
        
    }
}

void vmContextScopePop(vmContext * ctx){
    if(ctx){
        
        assert(ctx->scopes.length >0);
        
        vmContextScope * scope = ctx->scopes.scope + ctx->scopes.length -1;
        
        int n;
        vmVariant * var;
        
        if(scope->variants){
            for(n=0; n< scope->variants->length;n++){
                var = objv_hash_map_valueAt(scope->variants, n);
                if((var->type & vmVariantTypeObject) && var->objectValue && ! (var->type & vmVariantTypeWeak)){
                    objv_object_release((objv_object_t *) var->objectValue);
                }
                objv_zone_free(ctx->base.zone, var);
            }
            
            objv_hash_map_dealloc(scope->variants);
        }
        
        ctx->scopes.length --;
        
    }
}

void vmContextSetVariant(vmContext * ctx,objv_key_t * key, vmVariant variant){
    if(ctx && key){
        
        assert(ctx->scopes.length >0);
        
        vmContextScope * scope = ctx->scopes.scope + ctx->scopes.length -1;
        
        vmVariant * v = (vmVariant *) objv_hash_map_get(scope->variants, key);
        
        if((variant.type & vmVariantTypeObject) && variant.objectValue && ! (variant.type & vmVariantTypeWeak)){
            objv_object_retain(variant.objectValue);
        }
        
        if(v){
            
            if((v->type & vmVariantTypeObject) && v->objectValue && ! (v->type & vmVariantTypeWeak)){
                objv_object_release((objv_object_t *) v->objectValue);
            }
            
            if(variant.type == vmVariantTypeVoid){
                objv_hash_map_remove(scope->variants, key);
                objv_zone_free(ctx->base.zone, v);
            }
        }
        else {
            
            if(variant.type != vmVariantTypeVoid){
                
                v = (vmVariant *) objv_zone_malloc(ctx->base.zone, sizeof(vmVariant));
                * v = variant;
                
                if(scope->variants == NULL){
                    scope->variants = objv_hash_map_alloc(16, objv_hash_map_hash_code_ptr, objv_map_compare_any);
                }
                
                objv_hash_map_put(scope->variants, key, v);
            }
            
        }
        
    }
}

vmVariant vmContextVariant(vmContext * ctx,objv_key_t * key){
    vmVariant v = {vmVariantTypeVoid,0};
    if(ctx && key){
        
        assert(ctx->scopes.length >0);
        
        vm_int32_t index = ctx->scopes.length -1;
        vmContextScope * scope;
        vmVariant * p;
        
        while(index >= 0){
            
            scope = ctx->scopes.scope + index;
            
            p = (vmVariant *) objv_hash_map_get(scope->variants, key);
            
            if(p){
                
                v = * p;
                
                break;
            }
            
            index --;
        }
    }
    return v;
}

static void vmObjectClassDealloc (struct _objv_class_t * clazz, objv_object_t * object){
    
    vmObject * obj = (vmObject *) object;
    vmClass * vmclass = (vmClass *) clazz;
    vmVariant * p = (vmVariant *) (obj + 1);
    int c = vmclass->vmClass->propertyCount;
    
    while (p && c >0) {
        
        if((p->type & vmVariantTypeObject) && p->objectValue && ! (p->type & vmVariantTypeWeak)){
            
            objv_object_release(p->objectValue);
            
        }
        
        c -- ;
        p ++;
    }
    
    if(clazz->superClass){
        
        objv_object_dealloc(clazz->superClass, object);
        
    }
    
}

static vmVariant vmObjectClassInvoke (struct _objv_class_t * clazz, objv_object_t * object
                                      ,objv_key_t * key,objv_array_t * arguments){
    
    vmVariant v = {vmVariantTypeVoid,0};
    
    vmClass * vmclass = (vmClass *) clazz;
    vmObject * obj = (vmObject *) object;
    
    vmMetaOperator * op = (vmMetaOperator *) objv_hash_map_get(vmclass->functions, key);
    
    if(op){
        vmObjectOperatorFunction(obj->ctx, clazz, object, (char *) vmclass->vmClass - vmclass->vmClass->offset, op, arguments);
    }
    else if(clazz->superClass){
        v = vmObjectInvoke(clazz->superClass, object, key, arguments);
    }
    
    return v;
}

static vmVariant vmObjectClassProperty (struct _objv_class_t * clazz, objv_object_t * object,objv_key_t * key){
    
    vmObject * obj = (vmObject *) object;
    vmClass * vmclass = (vmClass *) clazz;
    vmMetaOperator * bp = (vmMetaOperator *) (vmclass->vmClass + 1);
    vmMetaOperator * op = objv_hash_map_get(vmclass->propertys, key);
    vmVariant * p = (vmVariant *) (obj + 1);
    
    if(op){
        return p[(op - bp) / sizeof(vmMetaOperator)];
    }
    
    return vmObjectGetProperty(clazz->superClass, object, key);
}

static vmVariant vmObjectClassSetProperty (struct _objv_class_t * clazz, objv_object_t * object
                                           ,objv_key_t * key,vmVariant value){
    
    vmObject * obj = (vmObject *) object;
    vmClass * vmclass = (vmClass *) clazz;
    vmMetaOperator * bp = (vmMetaOperator *) (vmclass->vmClass + 1);
    vmMetaOperator * op = objv_hash_map_get(vmclass->propertys, key);
    vmVariant v = {vmVariantTypeVoid,0};
    vmVariant * p = (vmVariant *) (obj + 1);
    
    if(op){
        
        p = p + ((op - bp) / sizeof(vmMetaOperator));
        * p = value;
    }
    else{
        v = vmObjectSetProperty(clazz->superClass, object, key, value);
    }
    
    return v;
}

static void vmClassInitialize (struct _objv_class_t * clazz){
    
    vmClass * vmclass = (vmClass *) clazz;
    
    char * p = (char *) vmclass->vmClass - vmclass->vmClass->offset;
    char * pKey;
    vmMetaOperator * op;
    objv_key_t * key;
    vm_uint32_t c;
    
    if(vmclass->vmClass->propertyCount){
        
        vmclass->propertys = objv_hash_map_alloc(vmclass->vmClass->propertyCount, objv_hash_map_hash_code_ptr, objv_map_compare_any);
        
        op = (vmMetaOperator *)(vmclass->vmClass + 1);
        
        c = vmclass->vmClass->propertyCount;
        
        while(op && c >0){
            
            pKey = p + op->uniqueKey;
            
            key = vmContextKey(vmclass->ctx, pKey);
            
            objv_hash_map_put(vmclass->propertys, key, op);
            
            c --;
            op ++;
        }
        
    }
    
    if(vmclass->vmClass->functionCount){
        
        vmclass->functions = objv_hash_map_alloc(vmclass->vmClass->functionCount, objv_hash_map_hash_code_ptr, objv_map_compare_any);
        
        op = (vmMetaOperator *)(vmclass->vmClass + 1) + vmclass->vmClass->propertyCount;
        
        c = vmclass->vmClass->functionCount;
        
        while(op && c >0){
            
            pKey = p + op->uniqueKey;
            
            key = vmContextKey(vmclass->ctx, pKey);
            
            objv_hash_map_put(vmclass->functions, key, op);
            
            c --;
            op ++;
        }

    }
    
}

static objv_iterator_t * vmObjectClassIterator (objv_class_t * clazz,objv_object_t * obj){
    return (objv_iterator_t *) objv_object_autorelease((objv_object_t *) objv_object_alloc(obj->zone, &vmObjectIteratorClass,obj) );
}


static objv_method_t vmClassMethods[] = {
    {OBJV_KEY(dealloc),"v()",(objv_method_impl_t)vmObjectClassDealloc}
    ,{OBJV_KEY(invoke),"v(*,*,I)",(objv_method_impl_t)vmObjectClassInvoke}
    ,{OBJV_KEY(property),"v(*)",(objv_method_impl_t)vmObjectClassProperty}
    ,{OBJV_KEY(setProperty),"v(*,{})",(objv_method_impl_t)vmObjectClassSetProperty}
    ,{OBJV_KEY(iterator),"@()",(objv_method_impl_t)vmObjectClassIterator}
};

objv_boolean_t vmContextLoadBinary(vmContext * ctx,vmMetaBinary * binary,objv_boolean_t copy){
    
    if(ctx && binary && vmMetaBinaryValidate(binary)){
        
        vmBinary * b;
        vmMetaClass * mClass;
        vm_uint32_t c;
        char * pKey;
        objv_key_t * key;
        vmClass * clazz, * superClass;
        
        if(ctx->binarys.length + 1 > ctx->binarys.size){
            ctx->binarys.size += 16;
            if(ctx->binarys.binarys){
                ctx->binarys.binarys = (vmBinary *) objv_zone_realloc(ctx->base.zone, ctx->binarys.binarys, ctx->binarys.size * sizeof(vmBinary));
            }
            else{
                ctx->binarys.binarys = (vmBinary *) objv_zone_malloc(ctx->base.zone,  ctx->binarys.size * sizeof(vmBinary));
            }
        }
        
        b = ctx->binarys.binarys + ctx->binarys.length ++;
        
        if(copy){
            b->binary = (vmMetaBinary *) objv_zone_malloc(ctx->base.zone, binary->length);
            memcpy(b->binary, binary, binary->length);
            b->copyed = objv_true;
        }
        else{
            b->binary = binary;
            b->copyed = objv_false;
        }
        
        mClass = (vmMetaClass *) (b->binary + 1);
        
        c = b->binary->classCount;
        
        while(c >0 && mClass){
            
            pKey = ((char *) b->binary + mClass->uniqueKey);
            
            key = vmContextKey(ctx, pKey);
            
            clazz = objv_zone_malloc(ctx->base.zone, sizeof(vmClass));
            
            objv_zone_memzero(ctx->base.zone, clazz, sizeof(vmClass));
            
            clazz->base.name = key;
            clazz->base.size = sizeof(vmClass) + mClass->propertyCount * sizeof(vmVariant);
            clazz->base.initialize = vmClassInitialize;
            clazz->base.methods = vmClassMethods;
            clazz->base.methodCount = sizeof(vmClassMethods) / sizeof(objv_method_t);
            clazz->vmClass = mClass;
            clazz->ctx = ctx;
            
            if(mClass->superClass){
                
                pKey = ((char *) b->binary + mClass->superClass);
                key = vmContextKey(ctx, pKey);
                superClass = (vmClass *) objv_hash_map_get(ctx->classMap, key);
                
                if(superClass){
                    clazz->base.superClass = (objv_class_t *) superClass;
                }
                else{
                    clazz->base.superClass = objv_class(key);
                }
                
                if(clazz->base.superClass == NULL){
                    clazz->base.superClass = & objv_object_class;
                }
                else{
                    objv_log("\nNot Found Class %s\n",pKey);
                }
            }
            else{
                clazz->base.superClass = & objv_object_class;
            }
            
            c -- ;
            mClass ++;
        }
        
        
        return objv_true;
    }
    
    return objv_false;
}

objv_class_t * vmContextGetClass(vmContext * ctx, objv_key_t * key){
    if(ctx && key){
        objv_class_t * clazz = (objv_class_t *) objv_hash_map_get(ctx->classMap, key);
        if(clazz == NULL){
            clazz = objv_class(key);
        }
        return clazz;
    }
    return NULL;
}

objv_key_t * vmContextKey(vmContext * ctx, const char * key){
    
    if(ctx && key){
        objv_key_t * k = (objv_key_t *) objv_hash_map_get(ctx->keyMap, (void * ) key);
        if(k == NULL){
            k = (objv_key_t *) objv_zone_malloc(ctx->base.zone, sizeof(objv_key_t));
            k->name = key;
            k->type = objv_key_type_dynamic;
            objv_hash_map_put(ctx->keyMap, (void *) k->name, k);
        }
        return k;
    }
    
    return NULL;
}


vmVariant vmObjectNew(vmContext * ctx,objv_class_t * clazz,objv_array_t * arguments){
    
    vmVariant v = {vmVariantTypeVoid,0};
    
    if(ctx && clazz){
        
        vmObject * object = (vmObject *) objv_object_new(ctx->base.zone, (objv_class_t *) clazz);
        
        object->ctx = (vmContext *) objv_object_retain( (objv_object_t *) ctx);
        
        objv_class_t * c = (objv_class_t *) clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(invoke))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            v = (* (vmObjectMethodInvokeFun) method->impl)(c,(objv_object_t *) object,ctx->keys.init, arguments);
        }
        
    }
    
    return v;
}

vmVariant vmObjectInvoke(objv_class_t * clazz,objv_object_t * object,objv_key_t * key,objv_array_t * arguments){
    
    vmVariant v = {vmVariantTypeVoid,0};
    
    if(object && key){
        
        objv_class_t * c = (objv_class_t *) clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(invoke))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            v = (* (vmObjectMethodInvokeFun) method->impl)(c,(objv_object_t *) object,key, arguments);
        }
        
    }
    
    return v;
}

vmVariant vmObjectGetProperty(objv_class_t * clazz,objv_object_t * object, objv_key_t * key ){
    
    vmVariant v = {vmVariantTypeVoid,0};
    
    if(object && key){
        
        objv_class_t * c = (objv_class_t *) clazz;
        
        objv_method_t * method = NULL;
        
        objv_property_t * prop;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(property))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            v = (* (vmObjectMethodPropertyFun) method->impl)(c,(objv_object_t *) object,key);
        }
        
        else{
            
            c = (objv_class_t *) clazz;
            
            while(c && (prop = objv_class_getProperty(c, key)) == NULL){
                
                c = c->superClass;
            }
            
            if(prop){
                if(prop->type == & objv_type_int){
                    v.type = vmVariantTypeInt32;
                    v.int32Value = objv_property_intValue(c, object, prop, 0);
                }
                else if(prop->type == & objv_type_uint){
                    v.type = vmVariantTypeInt32;
                    v.int32Value = objv_property_uintValue(c, object, prop, 0);
                }
                else if(prop->type == & objv_type_long){
                    v.type = vmVariantTypeInt64;
                    v.int64Value = objv_property_longValue(c, object, prop, 0);
                }
                else if(prop->type == & objv_type_ulong){
                    v.type = vmVariantTypeInt64;
                    v.int64Value = objv_property_ulongValue(c, object, prop, 0);
                }
                else if(prop->type == & objv_type_longLong){
                    v.type = vmVariantTypeInt64;
                    v.int64Value = objv_property_longLongValue(c, object, prop, 0);
                }
                else if(prop->type == & objv_type_ulongLong){
                    v.type = vmVariantTypeInt64;
                    v.int64Value = objv_property_ulongLongValue(c, object, prop, 0);
                }
                else if(prop->type == & objv_type_float){
                    v.type = vmVariantTypeDouble;
                    v.doubleValue = objv_property_floatValue(c, object, prop, 0);
                }
                else if(prop->type == & objv_type_double){
                    v.type = vmVariantTypeDouble;
                    v.doubleValue = objv_property_doubleValue(c, object, prop, 0);
                }
                else if(prop->type == & objv_type_boolean){
                    v.type = vmVariantTypeBoolean;
                    v.booleanValue = objv_property_booleanValue(c, object, prop, 0);
                }
                else if(prop->type == & objv_type_object){
                    v.type = vmVariantTypeObject;
                    v.objectValue = objv_property_objectValue(c, object, prop, NULL);
                }
            }
        }
        
    }
    
    return v;
    
}

vmVariant vmObjectSetProperty(objv_class_t * clazz,objv_object_t * object, objv_key_t * key , vmVariant value){
    
    vmVariant v = {vmVariantTypeVoid,0};
    
    if(object && key){
        
        objv_class_t * c = (objv_class_t *) clazz;
        
        objv_method_t * method = NULL;
        
        objv_property_t * prop;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(setProperty))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            v = (* (vmObjectMethodSetPropertyFun) method->impl)(c,(objv_object_t *) object,key,value);
        }
        else{
            
            c = (objv_class_t *) clazz;
            
            while(c && (prop = objv_class_getProperty(c, key)) == NULL){
                
                c = c->superClass;
            }
            
            if(prop){
                if(prop->type == & objv_type_int){
                    objv_property_setIntValue(c, object, prop, vmVariantToInt32(value));
                }
                else if(prop->type == & objv_type_uint){
                    objv_property_setUintValue(c, object, prop, vmVariantToInt32(value));
                }
                else if(prop->type == & objv_type_long){
                    objv_property_setLongValue(c, object, prop, vmVariantToInt32(value));
                }
                else if(prop->type == & objv_type_ulong){
                    objv_property_setUlongValue(c, object, prop, vmVariantToInt32(value));
                }
                else if(prop->type == & objv_type_longLong){
                    objv_property_setLongLongValue(c, object, prop, vmVariantToInt64(value));
                }
                else if(prop->type == & objv_type_ulongLong){
                    objv_property_setUlongLongValue(c, object, prop, vmVariantToInt64(value));
                }
                else if(prop->type == & objv_type_float){
                    objv_property_setFloatValue(c, object, prop, vmVariantToDouble(value));
                }
                else if(prop->type == & objv_type_double){
                    objv_property_setDoubleValue(c, object, prop, vmVariantToDouble(value));
                }
                else if(prop->type == & objv_type_boolean){
                    objv_property_setBooleanValue(c, object, prop, vmVariantToBoolean(value));
                }
                else if(prop->type == & objv_type_object){
                    objv_property_setObjectValue(c, object, prop, vmVariantToObject(object->zone, value).objectValue);
                }
            }
        }
    }
    
    return v;
    
}

objv_boolean_t vmIsClass(objv_class_t * clazz){
    return clazz && clazz->initialize == vmClassInitialize;
}


vmVariant vmVariantToObject(objv_zone_t * zone,vmVariant var){
    vmVariant rs = {vmVariantTypeObject,0};
    if(var.type == vmVariantTypeInt32){
        rs.objectValue = objv_object_autorelease((objv_object_t *)objv_value_alloc_intValue(zone, var.int32Value));
    }
    else if(var.type == vmVariantTypeInt64){
        rs.objectValue = objv_object_autorelease((objv_object_t *)objv_value_alloc_longLongValue(zone, var.int64Value));
    }
    else if(var.type == vmVariantTypeDouble){
        rs.objectValue = objv_object_autorelease((objv_object_t *)objv_value_alloc_doubleValue(zone, var.doubleValue));
    }
    else if(var.type == vmVariantTypeBoolean){
        rs.objectValue = objv_object_autorelease((objv_object_t *)objv_value_alloc_booleanValue(zone, var.booleanValue));
    }
    else if(var.type == vmVariantTypeString && var.stringValue){
        rs.objectValue = objv_object_autorelease((objv_object_t *)objv_string_alloc(zone, var.stringValue));
    }
    else if((var.type & vmVariantTypeObject) && var.objectValue){
        rs.objectValue = var.objectValue;
        rs.ofClass = var.ofClass;
    }
    return rs;
}

vm_int64_t vmVariantToInt64(vmVariant var){
    if(var.type == vmVariantTypeInt32){
        return var.int32Value;
    }
    else if(var.type == vmVariantTypeInt64){
        return var.int64Value;
    }
    else if(var.type == vmVariantTypeDouble){
        return var.doubleValue;
    }
    else if(var.type == vmVariantTypeBoolean){
        return var.booleanValue;
    }
    else if(var.type == vmVariantTypeString && var.stringValue){
        return atoll(var.stringValue);
    }
    else if((var.type & vmVariantTypeObject) && var.objectValue){
        return objv_object_longLongValue(var.objectValue, 0);
    }
    return 0;
}

vm_int32_t vmVariantToInt32(vmVariant var){
    if(var.type == vmVariantTypeInt32){
        return var.int32Value;
    }
    else if(var.type == vmVariantTypeInt64){
        return (vm_int32_t) var.int64Value;
    }
    else if(var.type == vmVariantTypeDouble){
        return var.doubleValue;
    }
    else if(var.type == vmVariantTypeBoolean){
        return var.booleanValue;
    }
    else if(var.type == vmVariantTypeString && var.stringValue){
        return atoi(var.stringValue);
    }
    else if((var.type & vmVariantTypeObject) && var.objectValue){
        return objv_object_intValue(var.objectValue, 0);
    }
    return 0;
}

objv_string_t * vmVariantToString(objv_zone_t * zone, vmVariant var){
    if(var.type == vmVariantTypeInt32){
        return (objv_string_t *) objv_object_autorelease((objv_object_t *)objv_string_alloc_format(zone, "%d",var.int32Value));
    }
    else if(var.type == vmVariantTypeInt64){
        return (objv_string_t *) objv_object_autorelease((objv_object_t *)objv_string_alloc_format(zone, "%lld",var.int64Value));
    }
    else if(var.type == vmVariantTypeDouble){
        return (objv_string_t *) objv_object_autorelease((objv_object_t *)objv_string_alloc_format(zone, "%f",var.doubleValue));
    }
    else if(var.type == vmVariantTypeBoolean){
        if(var.booleanValue){
            return objv_string_new(zone, "true");
        }
        return objv_string_new(zone, "false");
    }
    else if(var.type == vmVariantTypeString && var.stringValue){
        return (objv_string_t *) objv_object_autorelease((objv_object_t *)objv_string_alloc(zone, var.stringValue));
    }
    else if((var.type & vmVariantTypeObject) && var.objectValue){
        return objv_object_stringValue(var.objectValue, NULL);
    }
    return NULL;
}

void vmVariantToStringBuffer(vmVariant var,objv_mbuf_t * mbuf){
    if(var.type == vmVariantTypeInt32){
        objv_mbuf_format(mbuf, "%d",var.int32Value);
    }
    else if(var.type == vmVariantTypeInt64){
        objv_mbuf_format(mbuf, "%lld",var.int64Value);
    }
    else if(var.type == vmVariantTypeDouble){
        objv_mbuf_format(mbuf, "%f",var.doubleValue);
    }
    else if(var.type == vmVariantTypeBoolean){
        if(var.booleanValue){
            objv_mbuf_format(mbuf, "true");
        }
        else{
            objv_mbuf_format(mbuf, "false");
        }
    }
    else if(var.type == vmVariantTypeString && var.stringValue){
        objv_mbuf_append(mbuf, (void *)var.stringValue,strlen(var.stringValue));
    }
    else if((var.type & vmVariantTypeObject) && var.objectValue){
        {
            objv_string_t * s = objv_object_stringValue(var.objectValue, NULL);
            if(s){
                objv_mbuf_append(mbuf, (void *) s->UTF8String, s->length);
            }
        }
    }
}

vm_boolean_t vmVariantToBoolean(vmVariant var){
    
    if(var.type & vmVariantTypeDouble){
        return var.doubleValue ? vm_true : vm_false;
    }
    else if(var.type & vmVariantTypeString){
        return var.stringValue ? vm_true : vm_false;
    }
    else if(var.type & vmVariantTypeObject){
        if(var.objectValue){
            return vm_true;
        }
        else{
            return vm_false;
        }
    }
    else if(var.type & vmVariantTypeInt32){
        return var.int32Value ? vm_true : vm_false;
    }
    else if(var.type & vmVariantTypeInt64){
        return var.int64Value ? vm_true : vm_false;
    }
    else if(var.type & vmVariantTypeBoolean){
        return var.booleanValue ? vm_true : vm_false;
    }
    
    return vm_false;
}

vm_double_t vmVariantToDouble(vmVariant var){
    
    if(var.type & vmVariantTypeDouble){
        return var.doubleValue;
    }
    else if(var.type & vmVariantTypeString){
        return var.stringValue ? atof(var.stringValue) : 0;
    }
    else if(var.type & vmVariantTypeObject){
        return objv_object_doubleValue(var.objectValue, 0.0);
    }
    else if(var.type & vmVariantTypeInt32){
        return var.int32Value;
    }
    else if(var.type & vmVariantTypeInt64){
        return var.int64Value;
    }
    else if(var.type & vmVariantTypeBoolean){
        return var.booleanValue;
    }
    return 0.0;
}

vmVariant vmObjectToVariant(objv_object_t * object){
    vmVariant v = {vmVariantTypeVoid,0};
    
    if(objv_object_isKindOfClass(object, & objv_value_class)){
        {
            objv_value_t * vv = (objv_value_t *) object;
            if(vv->type == & objv_type_int){
                v.type = vmVariantTypeInt32;
                v.int32Value = vv->intValue;
            }
            else if(vv->type == & objv_type_uint){
                v.type = vmVariantTypeInt32;
                v.int32Value = vv->uintValue;
            }
            else if(vv->type == & objv_type_long){
                v.type = vmVariantTypeInt64;
                v.int64Value = vv->longValue;
            }
            else if(vv->type == & objv_type_ulong){
                v.type = vmVariantTypeInt64;
                v.int64Value = vv->ulongValue;
            }
            else if(vv->type == & objv_type_longLong){
                v.type = vmVariantTypeInt64;
                v.int64Value = vv->longLongValue;
            }
            else if(vv->type == & objv_type_ulongLong){
                v.type = vmVariantTypeInt64;
                v.int64Value = vv->ulongLongValue;
            }
            else if(vv->type == & objv_type_boolean){
                v.type = vmVariantTypeBoolean;
                v.booleanValue = vv->booleanValue;
            }
            else if(vv->type == & objv_type_float){
                v.type = vmVariantTypeDouble;
                v.doubleValue = vv->floatValue;
            }
            else if(vv->type == & objv_type_double){
                v.type = vmVariantTypeDouble;
                v.doubleValue = vv->doubleValue;
            }
        }
    }
    else if(object) {
        v.type = vmVariantTypeObject;
        v.objectValue = object;
    }
    
    return v;
}


