//
//  objv_vm.h
//  objv
//
//  Created by zhang hailong on 14-2-7.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_vm_h
#define objv_objv_vm_h


#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_vermin.h"
#include "objv_array.h"
#include "objv_vmcompiler_binary.h"
    
    vm_boolean_t vmContextLoadClassSource(vmContext * ctx,const char * className,objv_string_t * source,objv_array_t * errors);
    
    vmVariant vmRun(vmContext * ctx,objv_string_t * source,objv_array_t * errors);
    
    vm_boolean_t vmCompilerBinaryLoadClassSource(vmCompilerBinary * binary,const char * className,objv_string_t * source,objv_array_t * errors);
    
    vm_boolean_t vmCompilerBinaryLoadProject(vmCompilerBinary * binary,const char * project);
    
#ifdef __cplusplus
}
#endif


#endif
