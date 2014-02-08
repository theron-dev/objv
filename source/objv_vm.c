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

vm_boolean_t vmContextLoadClassSource(vmContext * ctx,const char * className,objv_string_t * source,objv_array_t * errors){
    objv_zone_t * zone = ctx->base.zone;
    vmCompilerClassMeta * classMeta = vmCompilerClassSource(source, errors);
    vmCompilerBinary * binary ;
    vmMetaBinary * b;
    vm_boolean_t rs = vm_false;
    
    if(classMeta){
        
        binary = vmCompilerBinaryAlloc(zone);
        
        vmCompilerBinaryAddClass(binary, classMeta, className);
        
        b = vmCompilerBinaryBytes(binary);
        
        rs = vmContextLoadBinary(ctx, b, vm_true);
        
        objv_object_release((objv_object_t *) binary);
        
    }
    return rs;
}

vmVariant vmRun(vmContext * ctx,objv_string_t * source,objv_array_t * errors){
    
    char className[128];
    vmVariant v = {vmVariantTypeVoid,0};
    objv_class_t * clazz;
    objv_object_t * object;
    
    sprintf(className,"unname_%ld_%d",time(NULL),rand());
    
    if(vmContextLoadClassSource(ctx,className,source,errors)){
       
        clazz = vmContextGetClass(ctx, vmContextKey(ctx, className));
        
        if(clazz){
            
            vmContextScopePush(ctx);
            
            v = vmObjectNew(ctx, clazz, NULL);
            
            if(v.type & vmVariantTypeObject && v.objectValue){
                object = v.objectValue;
                
                v = vmObjectInvoke(clazz, object, vmContextKey(ctx, "main"), NULL);
                
            }
            
            vmContextScopePop(ctx);
        }
        
    }
    
    return v;
    
}