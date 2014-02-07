//
//  objv_vmcompiler_binary.c
//  objv
//
//  Created by zhang hailong on 14-2-7.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//


#include "objv_os.h"
#include "objv.h"
#include "objv_vmcompiler_binary.h"
#include "objv_log.h"
#include "md5.h"

#define UNIQUE_KEY_OFFSET  10

OBJV_KEY_IMP(vmCompilerBinary)

static void vmCompilerBinaryMethodDealloc(objv_class_t * clazz,objv_object_t * object){
    
    vmCompilerBinary * binary = (vmCompilerBinary *) object;
    
    objv_object_release((objv_object_t *) binary->classMetas);
    
    objv_object_release((objv_object_t *) binary->operatorMetas);
    
    objv_object_release((objv_object_t *) binary->resources);
    
    objv_mbuf_destroy(& binary->uniqueKeys);
    
    if(clazz->superClass){
        
        objv_object_dealloc(clazz->superClass, object);
        
    }
    
}

static objv_object_t * vmCompilerBinaryMethodInit(objv_class_t * clazz,objv_object_t * object,va_list ap){

    
    if(clazz->superClass){
        object = objv_object_initv(clazz->superClass, object, ap);
    }
    
    if(object){
        
        vmCompilerBinary * binary = (vmCompilerBinary *) object;
        
        binary->classMetas = objv_array_alloc(object->zone,4);
        binary->operatorMetas = objv_array_alloc(object->zone,4);
        binary->resources = objv_dictionary_alloc(object->zone,4);
        
        objv_mbuf_init(& binary->uniqueKeys,128);
        objv_mbuf_init(& binary->binary,128);
    }
    
    return object;
}

static objv_method_t vmCompilerBinaryMethods[] = {
    {OBJV_KEY(init),"@(*)",(objv_method_impl_t)vmCompilerBinaryMethodInit}
    ,{OBJV_KEY(dealloc),"v()",(objv_method_impl_t)vmCompilerBinaryMethodDealloc}
};

objv_class_t vmCompilerBinaryClass = {OBJV_KEY(vmCompilerBinary),& objv_object_class
    ,vmCompilerBinaryMethods,sizeof(vmCompilerBinaryMethods) / sizeof(objv_method_t)
    ,NULL,0
    ,sizeof(vmCompilerBinary)
    ,NULL,0,0};



static vmMetaOffset vmCompilerBinaryUniqueKey(vmCompilerBinary * binary,const char * key,vm_int32_t length,vm_boolean_t isNilString){
    vm_int32_t i;
    vmMetaOffset offset = 0;
    char * p = binary->uniqueKeys.data;
    char e = 0;
    size_t len;
    
    if(key == NULL || (!isNilString && length == 0)){
        return 0;
    }
    
    for(i=0;i<binary->uniqueKeyCount;i++){
        len = strlen(p);
        if(len == length && strncmp(p, key, length) ==0){
            return (vmMetaOffset)(p - (char *)binary->uniqueKeys.data + UNIQUE_KEY_OFFSET);
        }
        p = p + len +1;
    }
    offset = (vmMetaOffset) (binary->uniqueKeys.length + UNIQUE_KEY_OFFSET);
    objv_mbuf_append(&binary->uniqueKeys, (void *) key, length);
    objv_mbuf_append(&binary->uniqueKeys, (void *) key, length);
    objv_mbuf_append(&binary->uniqueKeys, &e,1);
    binary->uniqueKeyCount ++;
    return offset;
}

static vmMetaOffset vmCompilerBinaryAddOperatorMeta(vmCompilerBinary * binary,vmCompilerClassMeta * classMeta,vmCompilerMetaOperator *op){
    vm_int32_t i,c;
    vmCompilerMeta * meta;
    objv_zone_t * zone = binary->base.zone;
    
    memcpy(op->binary.classUDID, classMeta->binary.UDID, sizeof(classMeta->binary.UDID));
    op->binary.offset = binary->operatorOffset;
    op->binary.uniqueKey = vmCompilerBinaryUniqueKey(binary,op->uniqueKey.location.p,op->uniqueKey.length,vm_false);
    
    objv_array_add(binary->operatorMetas,(objv_object_t *)op);
    
    c = op->metas->length;
    
    op->binary.length = sizeof(vmMetaOperator) + sizeof(vmMeta) * c;
    
    binary->operatorOffset += op->binary.length;
    
    for(i=0;i<c;i++){
        meta = (vmCompilerMeta *)objv_array_objectAt(op->metas,i);
        if(meta->type & vmMetaTypeOperator){
            meta->binary.valueOffset = vmCompilerBinaryAddOperatorMeta(binary, classMeta,meta->operatorMeta);
        }
        else if(meta->type & vmMetaTypeString){
            {
                objv_mbuf_t buf ;
                char * p = meta->stringValue.location.p;
                vm_int32_t len = meta->stringValue.length;
                objv_string_t * v = NULL;
                
                objv_mbuf_init(& buf, 20);
                
                if( *p == '"' || *p =='\''){
                    p ++;
                    len -= 2;
                }
                
                while(p && len >0){
                    
                    if(p[0] == '\\'){
                        if(p[1] == '\\'){
                            objv_mbuf_append(&buf, (char *)"\\", 1);
                        }
                        else if(p[1] == 'n'){
                            objv_mbuf_append(&buf, (char *)"\n", 1);
                        }
                        else if(p[1] == 'r'){
                            objv_mbuf_append(&buf, (char *)"\r", 1);
                        }
                        else if(p[1] == 't'){
                            objv_mbuf_append(&buf, (char *)"\t", 1);
                        }
                        else if(p[1] == '\''){
                            objv_mbuf_append(&buf, (char *)"'", 1);
                        }
                        else if(p[1] == '"'){
                            objv_mbuf_append(&buf, (char *)"\"", 1);
                        }
                        else{
                            objv_mbuf_append(&buf,p + 1,1);
                        }
                        p++;
                    }
                    else{
                        objv_mbuf_append(&buf,p,1);
                    }
                    
                    len --;
                    p ++;
                }
                
                if(* meta->stringValue.location.p == '\''){
                    if(buf.length ==1){
                        meta->type = vmMetaTypeInt32;
                        meta->int32Value = * (unsigned char *) buf.data;
                    }
                    else{
                        v = (objv_string_t *) objv_dictionary_value(binary->resources, (objv_object_t *)objv_string_new(zone, objv_mbuf_str(& buf)));
                    }
                }
                
                if(meta->type & vmMetaTypeString){
                    if(v){
                        meta->binary.valueOffset = vmCompilerBinaryUniqueKey(binary,v->UTF8String , (vm_int32_t) v->length,vm_true);
                    }
                    else{
                        meta->binary.valueOffset = vmCompilerBinaryUniqueKey(binary,objv_mbuf_str(&buf) , (vm_int32_t )buf.length,vm_true);
                    }
                }
                
                objv_mbuf_destroy(&buf);
            }
            
        }
        else if(meta->type & vmMetaTypeObject){
            meta->binary.valueOffset = vmCompilerBinaryUniqueKey(binary,meta->objectKey.location.p,meta->objectKey.length,vm_false);
        }
        else if(meta->type & vmMetaTypeArg){
            meta->binary.valueOffset = vmCompilerBinaryUniqueKey(binary,meta->objectKey.location.p,meta->objectKey.length,vm_false);
        }
    }
    return op->binary.offset;
}

vm_boolean_t vmCompilerBinaryAddClass(vmCompilerBinary * binary,vmCompilerClassMeta * classMeta,const char * className){
    vm_int32_t i,c;
    
    classMeta->binary.className = vmCompilerBinaryUniqueKey(binary,className,(vm_int32_t) strlen(className),vm_true);
    
    if(classMeta->superClass.length == 0){
        classMeta->binary.superClass = vmCompilerBinaryUniqueKey(binary,"Object",6,vm_true);
    }
    else{
        classMeta->binary.superClass = vmCompilerBinaryUniqueKey(binary, classMeta->superClass.location.p,classMeta->superClass.length,vm_true);
    }
    
    classMeta->binary.offset = binary->classOffset;
    
    objv_array_add(binary->classMetas,(objv_object_t *)classMeta);
    
    c = classMeta->propertys->length;
    
    for(i=0;i<c;i++){
        vmCompilerBinaryAddOperatorMeta(binary,classMeta,(vmCompilerMetaOperator *)objv_array_objectAt(classMeta->propertys,i));
    }
    
    c = classMeta->functions->length;
    
    for(i=0;i<c;i++){
        vmCompilerBinaryAddOperatorMeta(binary,classMeta,(vmCompilerMetaOperator *)objv_array_objectAt(classMeta->functions,i));
    }
    
    binary->classOffset += sizeof(vmMetaClass) + classMeta->propertys->length * sizeof(vmMetaOffset) + classMeta->functions->length * sizeof(vmMetaOffset);
    
    return vm_true;
}

vm_boolean_t vmCompilerBinaryAddStringResource(vmCompilerBinary * binary,const char * key,const char * string){
    objv_string_t * okey = objv_string_new(binary->base.zone, key);
    objv_object_t * v = objv_dictionary_value(binary->resources, (objv_object_t *)okey);
    if(v == NULL){
        v = (objv_object_t *) objv_string_new(binary->base.zone, string);
        objv_dictionary_setValue(binary->resources, (objv_object_t *) okey, v);
    }
    return vm_true;
}

vmMetaBinary * vmCompilerBinaryBytes(vmCompilerBinary * binary){
    vm_int32_t i,c,j,length;
    vmMetaOffset uniqueKeyOffset = 0;
    vmMetaOffset operatorOffset = 0;
    vmMetaOffset offset = 0,t;
    vmMetaOffset classOffset = 0;
    vmCompilerClassMeta * classMeta;
    vmCompilerMetaOperator *op;
    vmCompilerMeta * meta;
    vmMetaBinary * bytes;
    vmMetaOperator rOperator;
    vmMeta rMeta;
    vmMetaClass rClassMeta;
    vmMetaClassBinary bClassMeta;
    char *p;
    md5_state_t md5;
    
    length = vmCompilerBinaryLength(binary);
    
    objv_mbuf_extend(& binary->binary ,length);
    
    bytes = (vmMetaBinary *) binary->binary.data;
    
    bytes->classCount = binary->classMetas->length;
    bytes->uniqueKeyCount = binary->uniqueKeyCount;
    bytes->length = length;
    
    vmMetaBinarySignature(bytes);
    
    uniqueKeyOffset = (vmMetaOffset) (length - binary->uniqueKeys.length) ;
    
    memcpy((char *)bytes + uniqueKeyOffset, binary->uniqueKeys.data, binary->uniqueKeys.length);
    
    operatorOffset = uniqueKeyOffset - binary->operatorOffset;
    
    c = binary->operatorMetas->length;
    
    offset = operatorOffset;
    
    for(i=0;i<c;i++){
        op = (vmCompilerMetaOperator *)objv_array_objectAt(binary->operatorMetas, i);
        memset(&rOperator, 0, sizeof(vmMetaOperator));
        rOperator.uniqueKey = op->binary.uniqueKey ? op->binary.uniqueKey + uniqueKeyOffset - UNIQUE_KEY_OFFSET : 0;
        rOperator.type = op->type;
        rOperator.metaCount = op->metas->length;
        memcpy((char *)bytes + offset, &rOperator, sizeof(vmMetaOperator));
        offset += sizeof(vmMetaOperator);
        for(j=0;j<rOperator.metaCount;j++){
            meta = (vmCompilerMeta *)objv_array_objectAt(op->metas, j);
            memset(&rMeta, 0, sizeof(vmMeta));
            rMeta.type = meta->type;
            if(rMeta.type & vmMetaTypeBoolean){
                rMeta.booleanValue = meta->booleanValue;
            }
            else if(rMeta.type & vmMetaTypeInt32){
                rMeta.int32Value = meta->int32Value;
            }
            else if(rMeta.type & vmMetaTypeInt64){
                rMeta.int64Value = meta->int64Value;
            }
            else if(rMeta.type & vmMetaTypeDouble){
                rMeta.doubleValue = meta->doubleValue;
            }
            else if(rMeta.type & vmMetaTypeString){
                rMeta.stringKey = meta->binary.valueOffset ? uniqueKeyOffset + meta->binary.valueOffset - UNIQUE_KEY_OFFSET : 0;
            }
            else if(rMeta.type & vmMetaTypeObject){
                rMeta.objectKey = meta->binary.valueOffset ? uniqueKeyOffset + meta->binary.valueOffset - UNIQUE_KEY_OFFSET : 0;
            }
            else if(rMeta.type & vmMetaTypeOperator){
                rMeta.operatorOffset = meta->binary.valueOffset ? operatorOffset + meta->binary.valueOffset: 0;
            }
            else if(rMeta.type & vmMetaTypeArg){
                rMeta.objectKey = meta->binary.valueOffset ? uniqueKeyOffset + meta->binary.valueOffset - UNIQUE_KEY_OFFSET : 0;
            }
            memcpy((char *)bytes + offset, &rMeta, sizeof(vmMeta));
            offset += sizeof(vmMeta);
        }
        
    }
    
    classOffset = operatorOffset - binary->classOffset;
    offset = classOffset;
    
    c = binary->classMetas->length;
    
    for(i=0;i<c;i++){
        classMeta = (vmCompilerClassMeta *) objv_array_objectAt(binary->classMetas, i);
        memset(&rClassMeta, 0, sizeof(vmMetaClass));
        rClassMeta.superClass = classMeta->binary.superClass ? classMeta->binary.superClass + uniqueKeyOffset - UNIQUE_KEY_OFFSET : 0;
        rClassMeta.offset = offset;
        rClassMeta.propertyCount = classMeta->propertys->length;
        rClassMeta.functionCount = classMeta->functions->length;
        memcpy((char *)bytes + offset, &rClassMeta, sizeof(vmMetaClass));
        offset += sizeof(vmMetaClass);
        
        for(j=0;j<rClassMeta.propertyCount;j++){
            op = (vmCompilerMetaOperator *) objv_array_objectAt(classMeta->propertys,j);
            t = operatorOffset + op->binary.offset;
            memcpy((char *)bytes + offset, &t, sizeof(vmMetaOffset));
            offset += sizeof(vmMetaOffset);
        }
        
        for(j=0;j<rClassMeta.functionCount;j++){
            op = (vmCompilerMetaOperator *) objv_array_objectAt(classMeta->functions,j);
            t = operatorOffset + op->binary.offset;
            memcpy((char *)bytes + offset, &t, sizeof(vmMetaOffset));
            offset += sizeof(vmMetaOffset);
        }
    }
    
    offset = sizeof(vmMetaBinary);
    
    for(i=0;i<c;i++){
        classMeta = (vmCompilerClassMeta *) objv_array_objectAt(binary->classMetas, i);
        memset(&bClassMeta, 0, sizeof(vmMetaClassBinary));
        bClassMeta.uniqueKey = classMeta->binary.className ? classMeta->binary.className + uniqueKeyOffset - UNIQUE_KEY_OFFSET : 0;
        bClassMeta.metaOffset = classOffset + classMeta->binary.offset;
        memcpy((char *)bytes + offset, &bClassMeta, sizeof(vmMetaClassBinary));
        offset += sizeof(vmMetaClassBinary);
    }
    
    p = (char *) binary->uniqueKeys.data;
    
    for(i=0;i<binary->uniqueKeyCount;i++){
        t = (vmMetaOffset)(uniqueKeyOffset + p - (char *)binary->uniqueKeys.data);
        memcpy((char *)bytes + offset, &t, sizeof(vmMetaOffset));
        offset += sizeof(vmMetaOffset);
        p += strlen(p) +1;
    }
    
    md5_init(&md5);
    
    if(length > sizeof(vmMetaBinary)){
        md5_append(&md5, (unsigned char *) bytes + sizeof(vmMetaBinary), length - sizeof(vmMetaBinary));
    }
    
    md5_finish(&md5, bytes->UDID);
    
    return bytes;
}

vm_int32_t vmCompilerBinaryLength(vmCompilerBinary * binary){
    
    vm_int32_t length = sizeof(vmMetaBinary);
    
    length += sizeof(vmMetaClassBinary) * binary->classMetas->length;
    
    length += sizeof(vmMetaOffset) * binary->uniqueKeyCount;
    
    length += binary->classOffset;
    
    length += binary->operatorBinaryOffset;
    
    length += binary->operatorOffset;
    
    length += binary->uniqueKeys.length;
    
    return length;
}

