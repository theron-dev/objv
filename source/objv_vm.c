//
//  objv_vm.c
//  objv
//
//  Created by zhang hailong on 14-2-7.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#include "objv_os.h"
#include "objv.h"
#include "objv_vermin.h"
#include "objv_vmcompiler.h"
#include "objv_vmcompiler_binary.h"
#include "objv_vmcompiler_meta.h"
#include "objv_autorelease.h"

vmVariant vmRun(vmContext * ctx,objv_string_t * source,objv_array_t * errors){
    
    objv_zone_t * zone = ctx->base.zone;
    vmCompilerBinary * binary ;
    char className[128];
    vmMetaBinary * b;
    vmVariant v = {vmVariantTypeVoid,0};
    objv_class_t * clazz;
    objv_object_t * object;
    vmCompilerClassMeta * classMeta = vmCompilerClassSource(source, errors);
    
    if(classMeta){
        
        objv_autorelease_pool_push();
        
        binary = vmCompilerBinaryAlloc(zone);
        
        sprintf(className,"unname_%ld_%d",time(NULL),rand());
        
        vmCompilerBinaryAddClass(binary, classMeta, className);
        
        b = vmCompilerBinaryBytes(binary);
        
        vmContextLoadBinary(ctx, b, vm_true);
        
        objv_object_release((objv_object_t *) binary);
        
        objv_autorelease_pool_pop();
        
        clazz = vmContextGetClass(ctx, vmContextKey(ctx, className));
        
        if(clazz){
            
            v = vmObjectNew(ctx, clazz, NULL);
            
            if(v.type & vmVariantTypeObject && v.objectValue){
                object = v.objectValue;
                
                v = vmObjectInvoke(clazz, object, vmContextKey(ctx, "main"), NULL);
                
            }
            
        }
        
    }
    
    return v;
    
}