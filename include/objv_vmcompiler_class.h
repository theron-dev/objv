//
//  objv_vmcompiler_class.h
//  objv
//
//  Created by zhang hailong on 14-2-7.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_vmcompiler_class_h
#define objv_objv_vmcompiler_class_h


#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_tokenizer.h"
#include "objv_vmcompiler.h"
    
    vm_boolean_t vm_compiler_class(objv_tokenizer_t * tokenizer,objv_array_t * errors);
    
#ifdef __cplusplus
}
#endif


#endif
