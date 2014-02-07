//
//  objv_vmcompiler_invokes.c
//  objv
//
//  Created by zhang hailong on 14-2-7.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//


#include "objv_os.h"
#include "objv.h"
#include "objv_vmcompiler_invokes.h"
#include "objv_vmcompiler_expression.h"
#include "objv_vmcompiler_tokenizer.h"

static vm_boolean_t vm_compiler_invoke_var(objv_tokenizer_t * tokenizer,vm_int32_t index,vm_int32_t length,objv_array_t * errors){
    vm_int32_t focusIndex,s = 0,c = length + index;
    objv_tokenizer_t * token ;
    
    s = 0;
    
    if(index >= c){
        vmCompilerErrorSet(errors,tokenizer->range.begin,"invoke var");
        return vm_false;
    }
    
    while(index < c){
        token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index);
        switch (s) {
            case 0:
            {
                if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_name_class)){
                    s = 1;
                }
                else{
                    vmCompilerErrorSet(errors,token->range.begin,"invoke var");
                    return vm_false;
                }
                
            }
                break;
            case 1:
                
                if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_operator_class)){
                    if(* token->range.begin.p == '='){
                        s = 2;
                        focusIndex = index + 1;
                    }
                    else if(* token->range.begin.p == ','){
                        s = 0;
                    }
                    else{
                        vmCompilerErrorSet(errors,token->range.begin,"invoke var");
                        return vm_false;
                    }
                }
                else{
                    vmCompilerErrorSet(errors,token->range.begin,"invoke var");
                    return vm_false;
                }
                break;
            case 2:
                
                if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_operator_class) && * token->range.begin.p == ','){
                    if(index - focusIndex ==0){
                        vmCompilerErrorSet(errors,token->range.begin,"invoke var");
                        return vm_false;
                    }
                    else if(index - focusIndex == 1){
                        if(! vm_compiler_expression((objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 0), errors)){
                            return vm_false;
                        }
                        s = 0;
                    }
                    else{
                        if(! vm_compiler_expression((objv_tokenizer_t *) objv_tokenizer_comib(tokenizer, focusIndex,index - focusIndex), errors)){
                            return vm_false;
                        }
                        
                        c = c - (index - focusIndex) + 1;
                        index = focusIndex + 1;
                        s = 0;
                    }
                }
                
                break;
                
            default:
                break;
        }
        
        index ++;
    }
    
    if(s == 2){
        if(index - focusIndex ==0){
            vmCompilerErrorSet(errors,token->range.begin,"invoke var");
            return vm_false;
        }
        else if(index - focusIndex == 1){
            if(! vm_compiler_expression((objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 0), errors)){
                return vm_false;
            }
        }
        else{
            if(! vm_compiler_expression((objv_tokenizer_t *) objv_tokenizer_comib(tokenizer, focusIndex,index - focusIndex), errors)){
                return vm_false;
            }
        }
    }
    
    return vm_true;
}

static vm_boolean_t vm_compiler_invoke_if(objv_tokenizer_t * tokenizer,vm_int32_t index,vm_int32_t length,objv_array_t * errors){
    
    
    objv_tokenizer_t * token ;
    if(length >0){
        token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index);
        if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_group_class) && * token->range.begin.p == '('){
            
            if(! vm_compiler_expression(token, errors)){
                vmCompilerErrorSet(errors,token->range.begin,"invoke if");
                return vm_false;
            }
            
            if(length > 1){
                token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index + 1);
                if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_group_class) && * token->range.begin.p == '{'){
                    if(! vm_compiler_invokes((objv_tokenizer_t *) token, errors)){
                        vmCompilerErrorSet(errors,token->range.begin,"invoke if");
                        return vm_false;
                    }
                }
                else{
                    if(length - 1 == 1){
                        if(! vm_compiler_expression((objv_tokenizer_t *) token, errors)){
                            vmCompilerErrorSet(errors,token->range.begin,"invoke if");
                            return vm_false;
                        }
                    }
                    else{
                        if(! vm_compiler_expression((objv_tokenizer_t *) objv_tokenizer_comib(tokenizer, index + 1,length - 1), errors)){
                            vmCompilerErrorSet(errors,token->range.begin,"invoke if");
                            return vm_false;
                        }
                    }
                }
            }
        }
        else{
            vmCompilerErrorSet(errors,token->range.begin,"invoke if");
            return vm_false;
        }
    }
    else{
        vmCompilerErrorSet(errors,tokenizer->range.begin,"invoke if");
        return vm_false;
    }
    
    return vm_true;
}

static vm_boolean_t vm_compiler_invoke_else(objv_tokenizer_t * tokenizer,vm_int32_t index,vm_int32_t length,objv_array_t * errors){
    
    objv_tokenizer_t * token ;
    
    if(length > 0){
        token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index);
        if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_name_class)){
            if(objv_tokenizer_equal_string(token,"if")){
                if(! vm_compiler_invoke_if(tokenizer,2, length -2,errors)){
                    return vm_false;
                }
            }
            else{
                vmCompilerErrorSet(errors,token->range.begin,"invoke else if");
                return vm_false;
            }
        }
        else if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_group_class) && * token->range.begin.p == '{'){
            if(! vm_compiler_invokes((objv_tokenizer_t *) token, errors)){
                return vm_false;
            }
        }
        else{
            if(length ==1){
                if(! vm_compiler_expression((objv_tokenizer_t *) token, errors)){
                    return vm_false;
                }
            }
            else{
                if(! vm_compiler_expression((objv_tokenizer_t *) objv_tokenizer_comib(tokenizer, index,length), errors)){
                    return vm_false;
                }
            }
        }
    }
    else{
        vmCompilerErrorSet(errors,tokenizer->range.begin,"invoke else");
        return vm_false;
    }
    
    return vm_true;
}

static vm_boolean_t vm_compiler_invoke_for(objv_tokenizer_t * tokenizer,vm_int32_t index,vm_int32_t length,objv_array_t * errors){
    
    objv_tokenizer_t * token ,* t ;
    vm_int32_t i,c,i1,i2;
    
    if(length > 0){
        token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index);
        if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_group_class) && * token->range.begin.p == '('){
            
            c = token->childs->length;
            
            i = vm_compiler_tokenizer_operator_index_of(token, "in", 0);
            
            if(i == -1){
                
                i1 = vm_compiler_tokenizer_operator_index_of(token, ";", 0);
                
                if(i1 == -1){
                    vmCompilerErrorSet(errors,token->range.begin,"invoke for");
                    return vm_false;
                }
                else if(i1 == 1){
                    if( ! vm_compiler_expression((objv_tokenizer_t *) objv_array_objectAt(token->childs, 0), errors)){
                        return vm_false;
                    }
                }
                else if(i1 > 1){
                    
                    t = (objv_tokenizer_t *) objv_array_objectAt(token->childs, 0);
                    
                    if(objv_object_isKindOfClass((objv_object_t *) t, & objv_tokenizer_name_class)
                       && objv_tokenizer_equal_string(t, "var")){
                        if(! vm_compiler_expression((objv_tokenizer_t *) objv_tokenizer_comib(token, 1, i1 -1), errors)){
                            return vm_false;
                        }
                        c = c - i1 + 2;
                        i1 = 2;
                    }
                    else{
                        if(! vm_compiler_expression((objv_tokenizer_t *) objv_tokenizer_comib(token, 0, i1), errors)){
                            return vm_false;
                        }
                        c = c - i1 +1;
                        i1 = 1;
                    }
                }
                
                i2 = vm_compiler_tokenizer_operator_index_of(token, ";", i1 +1);
                
                if(i2 == -1){
                    vmCompilerErrorSet(errors,token->range.begin,"invoke for");
                    return vm_false;
                }
                else if(i2 - i1 == 2){
                    if( ! vm_compiler_expression((objv_tokenizer_t *) objv_array_objectAt(token->childs, i1 +1), errors)){
                        return vm_false;
                    }
                }
                else if(i2 - i1 > 2){
                    if(! vm_compiler_expression((objv_tokenizer_t *) objv_tokenizer_comib(token, i1 + 1, i2 - i1 - 1), errors)){
                        return vm_false;
                    }
                    c = c - (i2 - i1 -1) + 1;
                    i2 = i1 +2;
                }
                
                if(c - i2 == 2){
                    if( ! vm_compiler_expression((objv_tokenizer_t *) objv_array_objectAt(token->childs, i2 +1), errors)){
                        return vm_false;
                    }
                }
                else if(c - i2 > 2){
                    if(! vm_compiler_expression((objv_tokenizer_t *) objv_tokenizer_comib(token, i2 + 1, c - i2 - 1), errors)){
                        return vm_false;
                    }
                }
                
            }
            else if(i == 0){
                vmCompilerErrorSet(errors,token->range.begin,"invoke for");
                return vm_false;
            }
            else if(i == 1){
                objv_tokenizer_log(token, 5);
                t = (objv_tokenizer_t *) objv_array_objectAt(token->childs, 0);
            }
            else if(i == 2){
                t = (objv_tokenizer_t *) objv_array_objectAt(token->childs, 0);
                if(objv_object_isKindOfClass((objv_object_t *) t, & objv_tokenizer_name_class)
                   && objv_tokenizer_equal_string(t, "var")){
                    
                    t = (objv_tokenizer_t *) objv_array_objectAt(token->childs, 1);
                    
                }
                else{
                    vmCompilerErrorSet(errors,t->range.begin,"invoke for");
                    return vm_false;
                }
            }
            else{
                vmCompilerErrorSet(errors,token->range.begin,"invoke for");
                return vm_false;
            }
            
            if(i == 1 || i ==2){
                
                if(objv_object_isKindOfClass((objv_object_t *) t, & objv_tokenizer_name_class)){
                    
                    c = c - i - 1;
                    
                    if(c <=0 ){
                        vmCompilerErrorSet(errors,t->range.begin,"invoke for");
                        return vm_false;
                    }
                    else if(c == 1){
                        if( ! vm_compiler_expression((objv_tokenizer_t *) objv_array_objectAt(token->childs, i + 1), errors)){
                            return vm_false;
                        }
                    }
                    else{
                        if(! vm_compiler_expression((objv_tokenizer_t *) objv_tokenizer_comib(token, i + 1, c), errors)){
                            return vm_false;
                        }
                    }
                    
                }
                else{
                    vmCompilerErrorSet(errors,t->range.begin,"invoke for");
                    return vm_false;
                }
            }
            
            if(length == 2){
                token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index + 1);
                if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_group_class)){
                    if(* token->range.begin.p == '{'){
                        if(! vm_compiler_invokes(token, errors)){
                            return vm_false;
                        }
                    }
                    else{
                        if(! vm_compiler_expression(token, errors)){
                            return vm_false;
                        }
                    }
                }
                else{
                    if(! vm_compiler_expression(token, errors)){
                        return vm_false;
                    }
                }
            }
            else if(length > 2){
                if(! vm_compiler_expression((objv_tokenizer_t *) objv_tokenizer_comib(tokenizer, index + 1,length -1), errors)){
                    return vm_false;
                }
            }
            else{
                vmCompilerErrorSet(errors,token->range.begin,"invoke for");
                return vm_false;
            }
        }
        else{
            vmCompilerErrorSet(errors,token->range.begin,"invoke for");
            return vm_false;
        }
    }
    else{
        vmCompilerErrorSet(errors,tokenizer->range.begin,"invoke for");
        return vm_false;
    }
    
    return vm_true;
}

static vm_boolean_t vm_compiler_invoke_while(objv_tokenizer_t * tokenizer,vm_int32_t index,vm_int32_t length,objv_array_t * errors){
    
    objv_tokenizer_t * token;
    
    if(length >0){
        
        token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index);
        
        if(objv_object_isKindOfClass((objv_object_t *)token, & objv_tokenizer_group_class)
           && * token->range.begin.p == '('){
            
            if( ! vm_compiler_expression(token, errors) ){
                
                return vm_false;
            }
            
            if(length == 2){
                token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index + 1);
                if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_group_class)){
                    if(* token->range.begin.p == '{'){
                        if(! vm_compiler_invokes(token, errors)){
                            return vm_false;
                        }
                    }
                    else{
                        if(! vm_compiler_expression(token, errors)){
                            return vm_false;
                        }
                    }
                }
                else{
                    if(! vm_compiler_expression(token, errors)){
                        return vm_false;
                    }
                }
            }
            else if(length > 2){
                if(! vm_compiler_expression((objv_tokenizer_t *) objv_tokenizer_comib(tokenizer, index + 1,length -1), errors)){
                    return vm_false;
                }
            }
            else{
                vmCompilerErrorSet(errors,token->range.begin,"invoke while");
                return vm_false;
            }
            
        }
        else{
            vmCompilerErrorSet(errors,token->range.begin,"invoke while");
            return vm_false;
        }
        
    }
    else{
        vmCompilerErrorSet(errors,tokenizer->range.begin,"invoke while");
        return vm_false;
    }
    
    return vm_true;
}

static vm_boolean_t vm_compiler_invoke_try(objv_tokenizer_t * tokenizer,vm_int32_t index,vm_int32_t length,objv_array_t * errors){
    
    objv_tokenizer_t * token;
    
    if(length == 1){
        
        token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index);
        
        if(objv_object_isKindOfClass((objv_object_t *)token, & objv_tokenizer_group_class)
           && * token->range.begin.p == '{'){
            
            if(! vm_compiler_invokes(token, errors)){
                return vm_false;
            }
            
        }
        else{
            vmCompilerErrorSet(errors,tokenizer->range.begin,"invoke try");
            return vm_false;
        }
    }
    else{
        vmCompilerErrorSet(errors,tokenizer->range.begin,"invoke try");
        return vm_false;
    }
    
    return vm_true;
}

static vm_boolean_t vm_compiler_invoke_catch(objv_tokenizer_t * tokenizer,vm_int32_t index,vm_int32_t length,objv_array_t * errors){
    
    objv_tokenizer_t * token,*t;
    vm_int32_t c;
    
    if(length == 2){
        
        token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index);
        
        if(objv_object_isKindOfClass((objv_object_t *)token, & objv_tokenizer_group_class)
           && * token->range.begin.p == '('){
            
            c = token->childs->length;
            
            if(c == 1){
                
                t = (objv_tokenizer_t *) objv_array_objectAt(token->childs, 0);
                if(!objv_object_isKindOfClass((objv_object_t *)t, & objv_tokenizer_name_class)){
                    vmCompilerErrorSet(errors,t->range.begin,"invoke catch");
                    objv_tokenizer_log(token, 100);
                    return vm_false;
                }
                
            }
            
            if(c == 2){
                t = (objv_tokenizer_t *) objv_array_objectAt(token->childs, 1);
                if(!objv_object_isKindOfClass((objv_object_t *)t, & objv_tokenizer_name_class)){
                    vmCompilerErrorSet(errors,t->range.begin,"invoke catch");
                    return vm_false;
                }
                
            }
            
            if(c< 1){
                vmCompilerErrorSet(errors,token->range.begin,"invoke catch");
                return vm_false;
            }
            
            token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index + 1);
            
            if(objv_object_isKindOfClass((objv_object_t *)token, & objv_tokenizer_group_class)
               && * token->range.begin.p == '{'){
                if(! vm_compiler_invokes(token, errors)){
                    return vm_false;
                }
            }
            else{
                vmCompilerErrorSet(errors,token->range.begin,"invoke catch");
                return vm_false;
            }
            
        }
        else{
            vmCompilerErrorSet(errors,tokenizer->range.begin,"invoke catch");
            return vm_false;
        }
    }
    else{
        vmCompilerErrorSet(errors,tokenizer->range.begin,"invoke catch");
        return vm_false;
    }
    
    return vm_true;
}

static vm_boolean_t vm_compiler_invoke_finally(objv_tokenizer_t * tokenizer,vm_int32_t index,vm_int32_t length,objv_array_t * errors){
    
    objv_tokenizer_t * token;
    
    if(length == 1){
        
        token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 0);
        
        if(objv_object_isKindOfClass((objv_object_t *)token, & objv_tokenizer_group_class)
           && * token->range.begin.p == '{'){
            
            if(! vm_compiler_invokes(token, errors)){
                return vm_false;
            }
            
        }
        else{
            vmCompilerErrorSet(errors,tokenizer->range.begin,"invoke finally");
            return vm_false;
        }
    }
    else{
        vmCompilerErrorSet(errors,tokenizer->range.begin,"invoke finally");
        return vm_false;
    }
    
    return vm_true;
}

static vm_boolean_t vm_compiler_invoke_throw(objv_tokenizer_t * tokenizer,vm_int32_t index,vm_int32_t length,objv_array_t * errors){
    
    objv_tokenizer_t * token;
    
    if(length > 0){
        
        if(length ==1 ){
            
            token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 0);
            
            if(! vm_compiler_expression(token, errors)){
                return vm_false;
            }
            
        }
        else{
            token = (objv_tokenizer_t *) objv_tokenizer_comib(tokenizer, index, length);
            if(! vm_compiler_expression(token, errors)){
                return vm_false;
            }
        }
    }
    else{
        vmCompilerErrorSet(errors,tokenizer->range.begin,"invoke throw");
        return vm_false;
    }
    
    return vm_true;
}

static vm_boolean_t vm_compiler_invoke_break(objv_tokenizer_t * tokenizer,vm_int32_t index,vm_int32_t length,objv_array_t * errors){
    
    if(length == 0){
        
    }
    else{
        vmCompilerErrorSet(errors,tokenizer->range.begin,"invoke break");
        return vm_false;
    }
    
    return vm_true;
}

static vm_boolean_t vm_compiler_invoke_continue(objv_tokenizer_t * tokenizer,vm_int32_t index,vm_int32_t length,objv_array_t * errors){
    
    if(length == 0){
        
    }
    else{
        vmCompilerErrorSet(errors,tokenizer->range.begin,"invoke continue");
        return vm_false;
    }
    
    return vm_true;
}

static vm_boolean_t vm_compiler_invoke_return(objv_tokenizer_t * tokenizer,vm_int32_t index,vm_int32_t length,objv_array_t * errors){
    
    objv_tokenizer_t * token;
    
    if(length > 0){
        
        if(length ==1 ){
            
            token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 0);
            
            if(! vm_compiler_expression(token, errors)){
                return vm_false;
            }
            
        }
        else{
            token = (objv_tokenizer_t *) objv_tokenizer_comib(tokenizer, index, length);
            if(! vm_compiler_expression(token, errors)){
                return vm_false;
            }
        }
    }
    
    return vm_true;
}

static vm_boolean_t vm_compiler_invoke(objv_tokenizer_t * tokenizer,objv_array_t * errors){
    
    vm_int32_t c = tokenizer->childs->length;
    objv_tokenizer_t * token ;
    
    if(c > 0){
        token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 0);
        
        if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_name_class)){
            
            if(objv_tokenizer_equal_string(token,"var")){
                
                if(! vm_compiler_invoke_var(tokenizer,1, c -1,errors)){
                    return vm_false;
                }
                
            }
            else if(objv_tokenizer_equal_string(token,"if")){
                if(! vm_compiler_invoke_if(tokenizer,1, c -1,errors)){
                    return vm_false;
                }
            }
            else if(objv_tokenizer_equal_string(token,"else")){
                if(! vm_compiler_invoke_else(tokenizer,1, c -1,errors)){
                    return vm_false;
                }
            }
            else if(objv_tokenizer_equal_string(token,"for")){
                
                if(! vm_compiler_invoke_for(tokenizer,1, c -1,errors)){
                    return vm_false;
                }
                
            }
            else if(objv_tokenizer_equal_string(token,"while")){
                if(! vm_compiler_invoke_while(tokenizer,1, c -1,errors)){
                    return vm_false;
                }
            }
            else if(objv_tokenizer_equal_string(token,"try")){
                if(! vm_compiler_invoke_try(tokenizer,1, c -1,errors)){
                    return vm_false;
                }
            }
            else if(objv_tokenizer_equal_string(token,"catch")){
                if(! vm_compiler_invoke_catch(tokenizer,1, c -1,errors)){
                    return vm_false;
                }
            }
            else if(objv_tokenizer_equal_string(token,"finally")){
                if(! vm_compiler_invoke_finally(tokenizer,1, c -1,errors)){
                    return vm_false;
                }
            }
            else if(objv_tokenizer_equal_string(token,"throw")){
                if(! vm_compiler_invoke_throw(tokenizer,1, c -1,errors)){
                    return vm_false;
                }
            }
            else if(objv_tokenizer_equal_string(token,"break")){
                if(! vm_compiler_invoke_break(tokenizer,1, c -1,errors)){
                    return vm_false;
                }
            }
            else if(objv_tokenizer_equal_string(token,"continue")){
                if(! vm_compiler_invoke_continue(tokenizer,1, c -1,errors)){
                    return vm_false;
                }
            }
            else if(objv_tokenizer_equal_string(token,"return")){
                if(! vm_compiler_invoke_return(tokenizer,1, c -1,errors)){
                    return vm_false;
                }
            }
            else{
                if(! vm_compiler_expression(tokenizer, errors)){
                    return vm_false;
                }
            }
        }
        else{
            if(! vm_compiler_expression(tokenizer, errors)){
                return vm_false;
            }
        }
    }
    
    return vm_true;
}

vm_boolean_t vm_compiler_invokes(objv_tokenizer_t * tokenizer,objv_array_t * errors){
    
    vm_int32_t i,c = tokenizer->childs->length,index = 0,length = 0;
    objv_tokenizer_t * token ;
    
    for(i=0;i<c;i++){
        
        token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, i);
        
        if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_operator_class)){
            
            if( * token->range.begin.p == ';'){
                if(length >1){
                    if( !vm_compiler_invoke((objv_tokenizer_t *)objv_tokenizer_comib(tokenizer, index, length),errors)){
                        return vm_false;
                    }
                    i = index + 1;
                    c = c - length +1;
                }
                else if(length == 1){
                    if( !vm_compiler_invoke((objv_tokenizer_t *)(objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index),errors)){
                        return vm_false;
                    }
                }
                length = 0;
                index = 0;
            }
            else{
                if(length == 0){
                    index = i;
                }
                length ++;
            }
        }
        else{
            
            if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_group_class) && * token->range.begin.p == '{'){
                
                if(!vm_compiler_invokes(token,errors)){
                    return vm_false;
                }
                
                if(length == 0){
                    index = i;
                }
                length ++;
                
                if(length >1){
                    if( !vm_compiler_invoke((objv_tokenizer_t *)objv_tokenizer_comib(tokenizer, index, length),errors)){
                        return vm_false;
                    }
                    i = index ;
                    c = c - length +1;
                }
                else if(length == 1){
                    if( !vm_compiler_invoke((objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index),errors)){
                        return vm_false;
                    }
                }
                length = 0;
                index = 0;
            }
            else if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_combi_class)){
                if(length >1){
                    if( !vm_compiler_invoke((objv_tokenizer_t *)objv_tokenizer_comib(tokenizer, index, length),errors)){
                        return vm_false;
                    }
                    i = index;
                    c = c - length +1;
                }
                else if(length == 1){
                    if( !vm_compiler_invoke((objv_tokenizer_t *)(objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index),errors)){
                        return vm_false;
                    }
                }
                length = 1;
                index = i;
            }
            else{
                if(length == 0){
                    index = i;
                }
                length ++;
            }
        }
        
    }
    
    if(length >1){
        if( !vm_compiler_invoke((objv_tokenizer_t *)objv_tokenizer_comib(tokenizer, index, length),errors)){
            return vm_false;
        }
    }
    else if(length == 1){
        if( !vm_compiler_invoke((objv_tokenizer_t *)(objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index),errors)){
            return vm_false;
        }
    }
    
    return vm_true;
}
