//
//  objv_vmcompiler_binary.h
//  objv
//
//  Created by zhang hailong on 14-2-7.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_vmcompiler_binary_h
#define objv_objv_vmcompiler_binary_h

#include "objv_array.h"
#include "objv_dictionary.h"
#include "objv_vermin.h"
#include "objv_vmcompiler.h"
#include "objv_mbuf.h"
#include "objv_dictionary.h"

typedef struct _vmCompilerBinary{
    objv_object_t base;
    objv_array_t * classMetas;
    objv_array_t * operatorMetas;
    objv_mbuf_t uniqueKeys;
    vm_uint32_t uniqueKeyCount;
    vmMetaOffset operatorOffset;
    vmMetaOffset classOffset;
    vmMetaOffset operatorBinaryOffset;
    objv_dictionary_t * resources;
    objv_mbuf_t binary;
} vmCompilerBinary;

OBJV_KEY_DEC(vmCompilerBinary)

extern objv_class_t vmCompilerBinaryClass;

vmCompilerBinary * vmCompilerBinaryAlloc(objv_zone_t * zone);

vm_boolean_t vmCompilerBinaryAddClass(vmCompilerBinary * binary,vmCompilerClassMeta * classMeta,const char * className);

vm_boolean_t vmCompilerBinaryAddStringResource(vmCompilerBinary * binary,const char * key,const char * string);

vm_boolean_t vmCompilerBinaryLength(vmCompilerBinary * binary);

vmMetaBinary * vmCompilerBinaryBytes(vmCompilerBinary * binary);

#endif
