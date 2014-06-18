//
//  element.h
//  objv
//
//  Created by zhang hailong on 14-6-18.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_element_h
#define objv_element_h


#ifdef __cplusplus
extern "C" {
#endif
	
#include "objv.h"
#include "objv_dictionary.h"
#include "objv_array.h"
    
    struct _Document;
    
    typedef struct _Element {
        objv_object_t base;
        objv_string_t * READONLY ns;
        objv_string_t * READONLY name;
        objv_string_t * READONLY text;
        objv_dictionary_t * READONLY attributes;
        objv_array_t * READONLY childs;
        struct _Element * READONLY parent;
        struct _Document * READONLY document;
    } Element;

    
#ifdef __cplusplus
}
#endif


#endif
