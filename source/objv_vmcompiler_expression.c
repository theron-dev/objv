//
//  objv_vmcompiler_expression.c
//  objv
//
//  Created by zhang hailong on 14-2-7.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//


#include "objv_os.h"
#include "objv.h"
#include "objv_vmcompiler_expression.h"

static vm_boolean_t _vm_compiler_expression(objv_tokenizer_t * tokenizer,objv_array_t * errors);

static vm_boolean_t _vm_compiler_combi_action(objv_tokenizer_t * tokenizer,objv_array_t * errors){
    vm_int32_t i,c = tokenizer->childs->length,index = 0,length = 0;
    objv_tokenizer_t * token ;
    
    for(i=0;i<c;i++){
        
        token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, i);
        
        if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_operator_class)){
            
            if( * token->range.begin.p != '.'){
                if(length >1){
                    objv_tokenizer_comib(tokenizer, index, length);
                    i = index;
                    c = c - length +1;
                }
                length = 0;
                index = 0;
            }
            else{
                if(length == 0){
                    length = 1;
                    index =  i;
                }
                else{
                    length ++;
                }
            }
        }
        else{
            
            if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_group_class)){
                if(* token->range.begin.p == '(' || * token->range.begin.p == '['){
                    if(! _vm_compiler_expression(token,errors)){
                        return vm_false;
                    }
                    
                    if(length == 0){
                        length = 1;
                        index =  i;
                    }
                    else{
                        length ++;
                    }
                }
                else{
                    vmCompilerErrorSet(errors,token->range.begin,"expression action");
                    return vm_false;
                }
            }
            else if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_name_class)){
                if(length == 0){
                    length = 1;
                    index =  i;
                }
                else{
                    length ++;
                }
            }
            else{
                if(length >1){
                    objv_tokenizer_comib(tokenizer, index, length);
                    i = index;
                    c = c - length +1;
                }
                length = 0;
                index = 0;
            }
        }
        
    }
    
    if(length >1){
        objv_tokenizer_comib(tokenizer, index, length);
    }
    
    return vm_true;
}

static vm_boolean_t _vm_compiler_expression_operator_isop(char (*op)[OBJV_TOKENIZER_MAX_OPSIZE],vm_int32_t opCount,char * bp,char * ep){
    char * p = bp,* opp;
    
    while(opCount>0){
        
        p = bp;
        opp =  * op;
        
        while(p <= ep){
            if(*p != *opp){
                break;
            }
            p ++;
            opp ++;
        }
        
        if(p > ep && *opp == '\0'){
            return vm_true;
        }
        
        opCount --;
        op ++;
    }
    
    return vm_false;
}

#define VM_COMPILE_OP_TYPE_ONE_LR   0x01
#define VM_COMPILE_OP_TYPE_ONE_R    0x02
#define VM_COMPILE_OP_TYPE_TOW      0x03
#define VM_COMPILE_OP_TYPE_THREE    0x04

static vm_boolean_t _vm_compiler_expression_operator(objv_tokenizer_t * tokenizer,char (*op)[OBJV_TOKENIZER_MAX_OPSIZE],vm_int32_t opCount,vm_int32_t type,objv_array_t * errors){
    vm_int32_t i,c;
    objv_tokenizer_t * token , *prevToken = NULL,* nextToken = NULL;
    
    c = tokenizer->childs->length;
    
    for(i=0;i<c;i++){
        token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, i);
        
        if(i +1 <c){
            nextToken = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, i + 1);
        }
        else{
            nextToken = NULL;
        }
        
        if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_operator_class)){
            
            if(_vm_compiler_expression_operator_isop(op,opCount,token->range.begin.p,token->range.end.p)){
                
                switch (type) {
                    case VM_COMPILE_OP_TYPE_ONE_LR:
                    {
                        if(prevToken && !objv_object_isKindOfClass((objv_object_t *) prevToken, & objv_tokenizer_operator_class)){
                            prevToken = (objv_tokenizer_t *)objv_tokenizer_comib(tokenizer, i -1, 2);
                            c = c -1;
                            i = i -1;
                            continue;
                        }
                        
                        if(nextToken && !objv_object_isKindOfClass((objv_object_t *) nextToken, & objv_tokenizer_operator_class)){
                            prevToken = (objv_tokenizer_t *)objv_tokenizer_comib(tokenizer, i, 2);
                            c = c -1;
                            continue;
                        }
                        
                        vmCompilerErrorSet(errors,token->range.begin,"expression operator");
                        return vm_false;
                    }
                        break;
                    case VM_COMPILE_OP_TYPE_ONE_R:
                    {
                        if( token->range.begin.p == token->range.end.p && * token->range.begin.p == '-'){
                            if(prevToken && !objv_object_isKindOfClass((objv_object_t *) prevToken, & objv_tokenizer_operator_class)){
                                break;
                            }
                        }
                        if(nextToken && !objv_object_isKindOfClass((objv_object_t *) nextToken, & objv_tokenizer_operator_class)){
                            prevToken = (objv_tokenizer_t *)objv_tokenizer_comib(tokenizer, i, 2);
                            c = c -1;
                            continue;
                        }
                        
                        vmCompilerErrorSet(errors,token->range.begin,"expression operator");
                        return vm_false;
                    }
                        break;
                    case VM_COMPILE_OP_TYPE_TOW:
                    {
                        if(prevToken && nextToken
                           && !objv_object_isKindOfClass((objv_object_t *) prevToken, & objv_tokenizer_operator_class)
                           && !objv_object_isKindOfClass((objv_object_t *) nextToken, & objv_tokenizer_operator_class)){
                            
                            prevToken = (objv_tokenizer_t *)objv_tokenizer_comib(tokenizer, i-1, 3);
                            c = c -2;
                            i = i -1;
                            continue;
                        }
                        
                        vmCompilerErrorSet(errors,token->range.begin,"expression operator");
                        return vm_false;
                    }
                        break;
                    case VM_COMPILE_OP_TYPE_THREE:
                    {
                        if(prevToken && nextToken
                           && !objv_object_isKindOfClass((objv_object_t *) prevToken, & objv_tokenizer_operator_class)
                           && !objv_object_isKindOfClass((objv_object_t *) nextToken, & objv_tokenizer_operator_class)){
                            
                            if(i + 3 <c){
                                prevToken = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, i + 2);
                                nextToken = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, i + 3);
                                
                                if(prevToken && nextToken
                                   && objv_object_isKindOfClass((objv_object_t *) prevToken, & objv_tokenizer_operator_class)
                                   && * prevToken->range.begin.p == ':'
                                   && !objv_object_isKindOfClass((objv_object_t *) nextToken, & objv_tokenizer_operator_class)){
                                    prevToken = (objv_tokenizer_t *)objv_tokenizer_comib(tokenizer, i-1, 5);
                                    c = c -4;
                                    i = i -1;
                                    continue;
                                }
                            }
                        }
                        vmCompilerErrorSet(errors,token->range.begin,"expression operator");
                        return vm_false;
                    }
                        break;
                    default:
                        break;
                }
                
            }
            
        }
        
        prevToken = token;
    }
    
    return vm_true;
}


static vm_boolean_t _vm_compiler_expression(objv_tokenizer_t * tokenizer,objv_array_t * errors){
    
    static char ops[][OBJV_TOKENIZER_MAX_OPSIZE] = {
        "++","--","-",
        "!","~",
        "*","/","%",
        "+","-",
        "<<",">>",
        ">",">=","<","<=",
        "==","!=","===","!==",
        "&",
        "^",
        "|",
        "&&",
        "||",
        
        "?",
        
        "=","/=","*=","%=","+=","-=","<<=",">>=","&=","^=","|=",
        
        "is",
        "new",
    };
    
    
    if(!_vm_compiler_combi_action(tokenizer,errors)){
        return vm_false;
    }
    
    //objv_tokenizer_log(tokenizer);
    
    // new
    if(!_vm_compiler_expression_operator(tokenizer,ops + 38,1
                                         ,VM_COMPILE_OP_TYPE_ONE_R,errors)){
        return vm_false;
    }
    
    // ++ --
    if(!_vm_compiler_expression_operator(tokenizer,ops,2
                                         ,VM_COMPILE_OP_TYPE_ONE_LR,errors)){
        return vm_false;
    }
    
    //objv_tokenizer_log(tokenizer);
    
    // -
    if(!_vm_compiler_expression_operator(tokenizer,ops +2,1
                                         ,VM_COMPILE_OP_TYPE_ONE_R,errors)){
        return vm_false;
    }
    
    //objv_tokenizer_log(tokenizer);
    
    // ! ~
    if(!_vm_compiler_expression_operator(tokenizer,ops +3,2
                                         ,VM_COMPILE_OP_TYPE_ONE_R,errors)){
        return vm_false;
    }
    
    // * / %
    if(!_vm_compiler_expression_operator(tokenizer,ops +5,3
                                         ,VM_COMPILE_OP_TYPE_TOW,errors)){
        return vm_false;
    }
    
    // + -
    if(!_vm_compiler_expression_operator(tokenizer,ops +8,2
                                         ,VM_COMPILE_OP_TYPE_TOW,errors)){
        return vm_false;
    }
    
    // << >>
    if(!_vm_compiler_expression_operator(tokenizer,ops +10,2
                                         ,VM_COMPILE_OP_TYPE_TOW,errors)){
        return vm_false;
    }
    
    // > >= < <=
    if(!_vm_compiler_expression_operator(tokenizer,ops +12,4
                                         ,VM_COMPILE_OP_TYPE_TOW,errors)){
        return vm_false;
    }
    
    // == != === !==
    if(!_vm_compiler_expression_operator(tokenizer,ops +16,4
                                         ,VM_COMPILE_OP_TYPE_TOW,errors)){
        return vm_false;
    }
    
    // &
    if(!_vm_compiler_expression_operator(tokenizer,ops +20,1
                                         ,VM_COMPILE_OP_TYPE_TOW,errors)){
        return vm_false;
    }
    
    // ^
    if(!_vm_compiler_expression_operator(tokenizer,ops +21,1
                                         ,VM_COMPILE_OP_TYPE_TOW,errors)){
        return vm_false;
    }
    
    // |
    if(!_vm_compiler_expression_operator(tokenizer,ops +22,1
                                         ,VM_COMPILE_OP_TYPE_TOW,errors)){
        return vm_false;
    }
    
    // is
    if(!_vm_compiler_expression_operator(tokenizer,ops +37,1
                                         ,VM_COMPILE_OP_TYPE_TOW,errors)){
        return vm_false;
    }
    
    // &&
    if(!_vm_compiler_expression_operator(tokenizer,ops +23,1
                                         ,VM_COMPILE_OP_TYPE_TOW,errors)){
        return vm_false;
    }
    
    // ||
    if(!_vm_compiler_expression_operator(tokenizer,ops +24,1
                                         ,VM_COMPILE_OP_TYPE_TOW,errors)){
        return vm_false;
    }
    
    // ? :
    if(!_vm_compiler_expression_operator(tokenizer,ops +25,1
                                         ,VM_COMPILE_OP_TYPE_THREE,errors)){
        return vm_false;
    }
    
    //  "=","/=","*=","%=","+=","-=","<<=",">>=","&=","^=","|=",
    if(!_vm_compiler_expression_operator(tokenizer,ops +26,11
                                         ,VM_COMPILE_OP_TYPE_TOW,errors)){
        return vm_false;
    }
    
    {
        //objv_tokenizer_log(tokenizer);
        vm_int32_t i,c = tokenizer->childs->length;
        vm_int32_t s = 0;
        objv_tokenizer_t * token = NULL;
        
        for(i=0;i<c;i++){
            token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, i);
            if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_operator_class)){
                if( * token->range.begin.p == ','){
                    if(s == 0){
                        vmCompilerErrorSet(errors,token->range.begin,"expression ,");
                        return vm_false;
                    }
                    s = 0;
                }
                else{
                    vmCompilerErrorSet(errors,token->range.begin,"expression ,");
                    return vm_false;
                }
            }
            else{
                if(s == 1){
                    vmCompilerErrorSet(errors,token->range.begin,"expression ,");
                    return vm_false;
                }
                
                s = 1;
            }
        }
        if(token && s == 0){
            vmCompilerErrorSet(errors,token->range.begin,"");
            return vm_false;
        }
    }
    
    return vm_true;
}

vm_boolean_t vm_compiler_expression(objv_tokenizer_t * tokenizer,objv_array_t * errors){
    return _vm_compiler_expression(tokenizer, errors);
}
