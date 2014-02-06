//
//  objv_tokenizer.h
//  objv
//
//  Created by zhang hailong on 14-2-6.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_tokenizer_h
#define objv_objv_tokenizer_h


#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_array.h"
    
#define HTOKENIZER_MAX_OPSIZE   8
    
    typedef struct _objv_tokenizer_location_t{
        char * p;
        unsigned int line;
        unsigned int index;
    } objv_tokenizer_location_t;
    
    typedef struct _objv_tokenizer_range_t{
        objv_tokenizer_location_t begin;
        objv_tokenizer_location_t end;
    } objv_tokenizer_range_t;
    
    typedef struct _objv_tokenizer_t{
        objv_object_t base;
        const char * READONLY ofString;
        objv_tokenizer_range_t READONLY range;
        objv_array_t * READONLY childs;
        struct _objv_tokenizer_t * READONLY parent;
        objv_object_t * READONLY userInfo;
    } objv_tokenizer_t;
    
    objv_tokenizer_t * objv_tokenizer_alloc( const char * source);
    
    objv_tokenizer_t * objv_tokenizer_new( const char * source);
    
    void objv_tokenizer_child_add(objv_tokenizer_t * tokenizer,objv_tokenizer_t * token);
    
    void objv_tokenizer_child_remove(objv_tokenizer_t * tokenizer,objv_tokenizer_t * token);
    
    void objv_tokenizer_user_info_set(objv_tokenizer_t * tokenizer,objv_object_t * userInfo);
    
    objv_boolean_t objv_tokenizer_equal_string(objv_tokenizer_t * tokenizer,const char * string);
    
    typedef struct _objv_tokenizer_scanf_context_t{
        objv_tokenizer_location_t begin;
        objv_tokenizer_location_t prev;
        int s;
        char op[HTOKENIZER_MAX_OPSIZE];
        char beginChar[8];     // ( [ {
        char endChar[8];       // ) ] }
        objv_tokenizer_t * focusTokenizer;
    } objv_tokenizer_scanf_context_t;
    
    typedef objv_boolean_t (* objv_tokenizer_scanf_t)(objv_tokenizer_t * tokenizer,objv_tokenizer_location_t * locaiton,objv_tokenizer_t * token,objv_tokenizer_scanf_context_t * ctx);
    
    objv_boolean_t objv_tokenizer_scanf(objv_tokenizer_t * tokenizer,objv_tokenizer_scanf_t scanf,objv_tokenizer_scanf_context_t * ctx,objv_tokenizer_location_t * location);
    
    OBJV_KEY_DEC(Tokenizer)
    
    typedef struct _objv_tokenizer_comment_t{
        objv_tokenizer_t base;
    } objv_tokenizer_comment_t;
    
    OBJV_KEY_DEC(TokenizerComment)
    
    objv_boolean_t objv_tokenizer_comment_scanf(objv_tokenizer_t * tokenizer,objv_tokenizer_location_t * locaiton,objv_tokenizer_t * token,objv_tokenizer_scanf_context_t * ctx);
    
    typedef struct _objv_tokenizer_string_t{
        objv_tokenizer_t base;
    } objv_tokenizer_string_t;
    
    OBJV_KEY_DEC(TokenizerString)
    
    objv_boolean_t objv_tokenizer_string_scanf(objv_tokenizer_t * tokenizer,objv_tokenizer_location_t * locaiton,objv_tokenizer_t * token,objv_tokenizer_scanf_context_t * ctx);
    
    typedef struct _objv_tokenizer_value_t{
        objv_tokenizer_t base;
    } objv_tokenizer_value_t;
    
    OBJV_KEY_DEC(TokenizerValue)
    
    objv_boolean_t objv_tokenizer_value_scanf(objv_tokenizer_t * tokenizer,objv_tokenizer_location_t * locaiton,objv_tokenizer_t * token,objv_tokenizer_scanf_context_t * ctx);
    
    typedef struct _objv_tokenizer_name_t{
        objv_tokenizer_t base;
    } objv_tokenizer_name_t;
  
    OBJV_KEY_DEC(TokenizerName)
    
    objv_boolean_t objv_tokenizer_name_scanf(objv_tokenizer_t * tokenizer,objv_tokenizer_location_t * locaiton,objv_tokenizer_t * token,objv_tokenizer_scanf_context_t * ctx);
    
    typedef struct _objv_tokenizer_operator_t{
        objv_tokenizer_t base;
    } objv_tokenizer_operator_t;
    
    OBJV_KEY_DEC(TokenizerOperator)
   
    objv_boolean_t objv_tokenizer_operator_scanf(objv_tokenizer_t * tokenizer,objv_tokenizer_location_t * locaiton,objv_tokenizer_t * token,objv_tokenizer_scanf_context_t * ctx);
    
    typedef struct _objv_tokenizer_group_t{
        objv_tokenizer_t base;
    } objv_tokenizer_group_t;
    
    OBJV_KEY_DEC(TokenizerGroup)
    
    objv_boolean_t objv_tokenizer_group_scanf(objv_tokenizer_t * tokenizer,objv_tokenizer_location_t * locaiton,objv_tokenizer_t * token,objv_tokenizer_scanf_context_t * ctx);
    
    
    typedef struct _objv_tokenizer_combi_t{
        objv_tokenizer_t base;
    } objv_tokenizer_combi_t;
    
    OBJV_KEY_DEC(TokenizerCombi)
    
    objv_tokenizer_combi_t * objv_tokenizer_comib(objv_tokenizer_t * tokenizer,int index,int length);
    
    void objv_tokenizer_print(objv_tokenizer_t * tokenizer,int level);
    
    
#ifdef __cplusplus
}
#endif


#endif
