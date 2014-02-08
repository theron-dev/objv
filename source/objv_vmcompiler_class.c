//
//  objv_vmcompiler_class.c
//  objv
//
//  Created by zhang hailong on 14-2-7.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#include "objv_os.h"
#include "objv.h"
#include "objv_vmcompiler_class.h"
#include "objv_vmcompiler_invokes.h"
#include "objv_vmcompiler_expression.h"

static vm_boolean_t vm_compiler_class_extends(objv_tokenizer_t * tokenizer,vm_int32_t index,vm_int32_t length,objv_array_t * errors){
    objv_tokenizer_t * token ;
    
    if(length == 1){
        token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 0);
        if( ! objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_name_class)){
            
            vmCompilerErrorSet(errors,token->range.begin,"class extends");
            
            return vm_false;
        }
    }
    else{
        vmCompilerErrorSet(errors,tokenizer->range.begin,"class extends");
        return vm_false;
    }
    
    return vm_true;
}

static vm_boolean_t vm_compiler_class_property(objv_tokenizer_t * tokenizer,vm_int32_t index,vm_int32_t length,objv_array_t * errors){
    
    objv_tokenizer_t * token ;
    
    if(length >0 && length <= 3){
        
        token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index);
        
        if( objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_group_class)){
            if(* token->range.begin.p == '('){
                
                if(token->childs->length == 1){
                    
                    token = (objv_tokenizer_t *) objv_array_objectAt(token->childs, 0);
                    
                    if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_name_class)
                       && objv_tokenizer_equal_string(token, "weak")){
                        
                    }
                    else{
                        vmCompilerErrorSet(errors,token->range.begin,"class property");
                        
                        return vm_false;
                    }
                    
                }
                else{
                    
                    vmCompilerErrorSet(errors,token->range.begin,"class property");
                    
                    return vm_false;
                }
                
            }
            else{
                vmCompilerErrorSet(errors,token->range.begin,"class property");
                
                return vm_false;
            }
            
            if(length > 1){
                
                token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index +1);
                
                index ++;
                length --;
            }
            else{
                vmCompilerErrorSet(errors,token->range.begin,"class property");
                return vm_false;
            }
            
        }
        
        if( ! objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_name_class)){
            
            vmCompilerErrorSet(errors,token->range.begin,"class property");
            
            return vm_false;
        }
        
        if(length > 1){
            token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index + 1);
            
            if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_operator_class)
               && token->range.begin.p == token->range.end.p && * token->range.begin.p == '='){
                
                if(length > 2){
                    
                    token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index + 2);
                    
                    if(!objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_string_class)
                       && !objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_value_class)){
                        
                        vmCompilerErrorSet(errors,token->range.begin,"class property");
                        
                        return vm_false;
                    }
                    
                }
                else{
                    vmCompilerErrorSet(errors,token->range.begin,"class property");
                    
                    return vm_false;
                }
                
            }
            else{
                vmCompilerErrorSet(errors,token->range.begin,"class property");
                
                return vm_false;
            }
        }
    }
    else{
        vmCompilerErrorSet(errors,tokenizer->range.begin,"class property");
        return vm_false;
    }
    return vm_true;
}

static vm_boolean_t vm_compiler_class_function_arguments(objv_tokenizer_t * tokenizer,objv_array_t * errors){
    
    vm_int32_t s = 0,i,c = tokenizer->childs->length;
    objv_tokenizer_t * token = NULL;
    
    for(i=0;i<c;i++){
        
        token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, i);
        
        if( objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_name_class)){
            
            if(s == 1){
                vmCompilerErrorSet(errors,token->range.begin,"class function arguments");
                return vm_false;
            }
            
            s = 1;
            
        }
        else if( objv_object_isKindOfClass((objv_object_t *) token,& objv_tokenizer_operator_class) && token->range.begin.p == token->range.end.p && * token->range.begin.p == ','){
            
            if(s == 0){
                vmCompilerErrorSet(errors,token->range.begin,"class function arguments");
                return vm_false;
            }
            
            s = 0;
        }
        else{
            vmCompilerErrorSet(errors,token->range.begin,"class function arguments");
            
            return vm_false;
        }
        
    }
    
    return token == NULL || s == 1;
}

static vm_boolean_t vm_compiler_class_function(objv_tokenizer_t * tokenizer,vm_int32_t index,vm_int32_t length,objv_array_t * errors){
    objv_tokenizer_t * token ;
    
    if(length == 3){
        
        token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index);
        
        if( ! objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_name_class)){
            
            vmCompilerErrorSet(errors,token->range.begin,"class function");
            
            return vm_false;
        }
        
        token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index + 1);
        
        if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_group_class) && * token->range.begin.p == '('){
            
            if(! vm_compiler_class_function_arguments(token,errors)){
                return vm_false;
            }
        }
        else{
            vmCompilerErrorSet(errors,token->range.begin,"class function");
            
            return vm_false;
        }
        
        
        token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index + 2);
        
        if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_group_class) && * token->range.begin.p == '{'){
            
            if(! vm_compiler_invokes(token,errors)){
                return vm_false;
            }
            
        }
        else{
            vmCompilerErrorSet(errors,token->range.begin,"class function");
            
            return vm_false;
        }
        
    }
    else{
        vmCompilerErrorSet(errors,tokenizer->range.begin,"class function");
        return vm_false;
    }
    return vm_true;
}

static vm_boolean_t vm_compiler_class_command(objv_tokenizer_t * tokenizer,objv_array_t * errors){
    
    vm_int32_t c = tokenizer->childs->length;
    objv_tokenizer_t * token ;
    
    if(c > 0){
        token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 0);
        
        if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_name_class)){
            
            if(objv_tokenizer_equal_string(token,"extends")){
                
                if(! vm_compiler_class_extends(tokenizer,1, c -1,errors)){
                    return vm_false;
                }
                
            }
            else if(objv_tokenizer_equal_string(token,"var")){
                if(! vm_compiler_class_property(tokenizer,1, c -1,errors)){
                    return vm_false;
                }
            }
            else if(objv_tokenizer_equal_string(token,"function")){
                if(! vm_compiler_class_function(tokenizer,1, c -1,errors)){
                    return vm_false;
                }
            }
            else{
                
                vmCompilerErrorSet(errors,token->range.begin,"class");
                
                return vm_false;
            }
        }
        else{
            vmCompilerErrorSet(errors,token->range.begin,"class");
            return vm_false;
        }
    }
    
    return vm_true;
}

vm_boolean_t vm_compiler_class(objv_tokenizer_t * tokenizer,objv_array_t * errors){
    
    vm_int32_t i,c = tokenizer->childs->length,index = 0,length = 0;
    objv_tokenizer_t * token ;
    
    for(i=0;i<c;i++){
        
        token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, i);
        
        if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_operator_class)){
            
            if( * token->range.begin.p == ';'){
                if(length >1){
                    if( !vm_compiler_class_command((objv_tokenizer_t *)objv_tokenizer_comib(tokenizer, index, length),errors)){
                        return vm_false;
                    }
                    i = index;
                    c = c - length +1;
                }
                else if(length == 1){
                    if( !vm_compiler_class_command((objv_tokenizer_t *)(objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index),errors)){
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
                    if( !vm_compiler_class_command((objv_tokenizer_t *)objv_tokenizer_comib(tokenizer, index, length),errors)){
                        return vm_false;
                    }
                    i = index;
                    c = c - length +1;
                }
                else if(length == 1){
                    if( !vm_compiler_class_command((objv_tokenizer_t *)(objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index),errors)){
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
        
    }
    
    if(length >1){
        if( !vm_compiler_class_command((objv_tokenizer_t *)objv_tokenizer_comib(tokenizer, index, length),errors)){
            return vm_false;
        }
    }
    else if(length == 1){
        if( !vm_compiler_class_command((objv_tokenizer_t *)(objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index),errors)){
            return vm_false;
        }
    }
    
    return vm_true;
    
}