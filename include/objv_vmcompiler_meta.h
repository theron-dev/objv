//
//  objv_vmcompiler_meta.h
//  objv
//
//  Created by zhang hailong on 14-2-7.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_vmcompiler_meta_h
#define objv_objv_vmcompiler_meta_h

#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_vmcompiler.h"
    
    vmCompilerClassMeta * vmCompilerClassSource(objv_string_t * source,objv_array_t * errors);
    
#ifdef __cplusplus
}
#endif



#endif
