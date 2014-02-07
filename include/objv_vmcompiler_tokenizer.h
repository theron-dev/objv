//
//  objv_vermin_compiler_tokenizer.h
//  objv
//
//  Created by zhang hailong on 14-2-7.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_vmcompiler_tokenizer_h
#define objv_objv_vmcompiler_tokenizer_h

#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_tokenizer.h"
#include "objv_vmcompiler.h"
    
    objv_tokenizer_t * vm_compiler_tokenizer(objv_tokenizer_t * tokenizer,objv_array_t * errors);
    
    void vm_compiler_tokenizer_comment_clear(objv_tokenizer_t * tokenizer);
    
    void vm_compiler_tokenizer_optimization(objv_tokenizer_t * tokenizer);
    
    vm_int32_t vm_compiler_tokenizer_operator_index_of(objv_tokenizer_t * tokenizer,const char * op,vm_int32_t index);
    
    vm_int32_t vm_compiler_tokenizer_group_index_of(objv_tokenizer_t * tokenizer,char beginChar,vm_int32_t index);
    
#ifdef __cplusplus
}
#endif


#endif
