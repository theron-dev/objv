//
//  objv_vmcompiler_exec.c
//  objv
//
//  Created by zhang hailong on 14-2-7.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//


#include "objv_os.h"
#include "objv.h"
#include "objv_vmcompiler_meta.h"
#include "objv_vmcompiler_class.h"
#include "objv_vmcompiler_tokenizer.h"
#include "objv_tokenizer.h"
#include "objv_vermin.h"

static vmCompilerMeta * vmCompilerExecExpression(objv_tokenizer_t * tokenizer,objv_array_t * errors);

static vmCompilerMeta * vmCompilerExecInvokes(objv_tokenizer_t * tokenizer,objv_array_t * errors);


static vm_boolean_t vmCompilerExecExtends(vmCompilerClassMeta * classMeta,objv_tokenizer_t * tokenizer,objv_array_t * errors){
    objv_tokenizer_t * token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 1);
    
    classMeta->superClass.location = token->range.begin;
    classMeta->superClass.length = (vm_uint32_t) ((token->range.end.p - token->range.begin.p) + 1);
    
    return vm_true;
}

static vm_boolean_t vmCompilerExecProperty(vmCompilerClassMeta * classMeta,objv_tokenizer_t * tokenizer,objv_array_t * errors){
    vm_int32_t index = 1;
    objv_tokenizer_t * token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index);
    vmCompilerMetaOperator * op;
    vmCompilerMeta  * meta;
    
    if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_group_class)){
        index ++;
        token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index);
        
        op = vmCompilerMetaOperatorNew(token->base.zone, vmOperatorTypeVarWeak, token->range.begin, (vm_uint32_t) (token->range.end.p - token->range.begin.p + 1));
    }
    else{
        op = vmCompilerMetaOperatorNew(token->base.zone,vmOperatorTypeVar, token->range.begin, (vm_uint32_t) (token->range.end.p - token->range.begin.p + 1));
    }
    
    vmCompilerClassAddProperty(classMeta, op);
    
    index ++;
    
    if(index < tokenizer->childs->length){
        token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index);
        
        if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_operator_class)
           && token->range.begin.p == token->range.end.p && *token->range.begin.p == '='){
            
            index ++;
            if(index < tokenizer->childs->length){
                token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index);
                if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_value_class)){
                    meta = vmCompilerMetaNewWithNumberString(token->base.zone,token->range.begin, (vm_uint32_t)( token->range.end.p - token->range.begin.p + 1));
                    vmCompilerMetaOperatorAddCompilerMeta(op, meta);
                }
                else if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_string_class)){
                    meta = vmCompilerMetaNewWithString(token->base.zone,token->range.begin, (vm_uint32_t) (token->range.end.p - token->range.begin.p + 1) );
                    vmCompilerMetaOperatorAddCompilerMeta(op, meta);
                }
            }
        }
        
    }
    
    return vm_true;
}

static vmCompilerMeta * vmCompilerExecAction(objv_tokenizer_t * tokenizer,vm_int32_t index,vm_int32_t length,objv_array_t * errors){
    
    vmCompilerMetaOperator * op;
    vmCompilerMeta * meta;
    vm_int32_t i;
    objv_tokenizer_t * token,* nextToken;
    objv_zone_t * zone = tokenizer->base.zone;
    
    if(length >0){
        
        token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index);
        
        meta = vmCompilerExecExpression(token, errors);
        
        if(!meta){
            vmCompilerErrorSet(errors,token->range.begin, "ExecAction");
            return NULL;
        }
        
        for(i=1;i<length;i++){
            
            token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index + i);
            
            if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_operator_class)){
                i ++;
                token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index + i);
            }
            
            if(i + 1 < length){
                nextToken = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index + i + 1);
                if(objv_object_isKindOfClass((objv_object_t *) nextToken, & objv_tokenizer_operator_class)){
                    nextToken = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index + i +2);
                }
            }
            else{
                nextToken = NULL;
            }
            if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_name_class)){
                if(nextToken && objv_object_isKindOfClass((objv_object_t *) nextToken, & objv_tokenizer_group_class) && * nextToken->range.begin.p == '('){
                    op = vmCompilerMetaOperatorNew(zone,vmOperatorTypeFunction, token->range.begin, (vm_uint32_t) ( token->range.end.p - token->range.begin.p + 1));
                    vmCompilerMetaOperatorAddCompilerMeta(op, meta);
                    {
                        vm_int32_t n,c = nextToken->childs->length;
                        for(n=0;n<c;n++){
                            token = (objv_tokenizer_t *) objv_array_objectAt(nextToken->childs, n);
                            if(!objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_operator_class)){
                                meta = vmCompilerExecExpression(token,errors);
                                if(meta){
                                    vmCompilerMetaOperatorAddCompilerMeta(op, meta);
                                }
                                else{
                                    vmCompilerErrorSet(errors,token->range.begin, "ExecAction");
                                    return NULL;
                                }
                            }
                        }
                    }
                    meta = vmCompilerMetaNewWithOperator(zone,op);
                    i ++;
                }
                else {
                    op = vmCompilerMetaOperatorNew(zone,vmOperatorTypeProperty, token->range.begin, (vm_uint32_t) ( token->range.end.p - token->range.begin.p + 1));
                    vmCompilerMetaOperatorAddCompilerMeta(op, meta);
                    meta = vmCompilerMetaNewWithOperator(zone,op);
                }
            }
            else if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_group_class) && * token->range.begin.p == '['){
                
                op = vmCompilerMetaOperatorNew(zone,vmOperatorTypeProperty, token->range.begin, 0);
                
                vmCompilerMetaOperatorAddCompilerMeta(op, meta);
                
                meta = vmCompilerExecExpression(token,errors);
                
                if(meta){
                    
                    vmCompilerMetaOperatorAddCompilerMeta(op, meta);
                    
                    meta = vmCompilerMetaNewWithOperator(zone,op);
                    
                }
                else{
                    vmCompilerErrorSet(errors,token->range.begin, "ExecAction");
                    
                    return NULL;
                }
                
            }
            else{
                
                vmCompilerErrorSet(errors,token->range.begin, "ExecAction");
                
                return NULL;
            }
        }
        
        return meta;
    }
    
    return NULL;
}

#define VM_COMPILE_OP_TYPE_ONE_L    0x01
#define VM_COMPILE_OP_TYPE_ONE_R    0x02
#define VM_COMPILE_OP_TYPE_TOW      0x03
#define VM_COMPILE_OP_TYPE_THREE    0x04

#define VM_COMPILE_OP_TYPE_ONE_LL    0x05
#define VM_COMPILE_OP_TYPE_ONE_RR    0x06

static vmCompilerMeta *  vmCompilerExecOperator(objv_tokenizer_t * tokenizer,const char * op,vmOperatorType opType
                                                , vm_int32_t type,objv_array_t * errors){
    
    vm_int32_t c = tokenizer->childs->length;
    vm_int32_t i = vm_compiler_tokenizer_operator_index_of(tokenizer, op, 0);
    objv_zone_t * zone = tokenizer->base.zone;
    
    if(i != -1){
        
        if(type == VM_COMPILE_OP_TYPE_ONE_LL)
        {
            objv_tokenizer_t * t1,*t2;
            vmCompilerMetaOperator * op;
            vmCompilerMeta * meta;
            vm_int32_t n,nn;
            if(c == 2 && i==1){
                
                t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs,0);
                
                if(objv_object_isKindOfClass((objv_object_t *) t1, & objv_tokenizer_name_class)){
                    op = vmCompilerMetaOperatorNew(zone,opType, t1->range.begin, (vm_uint32_t) ( t1->range.end.p - t1->range.begin.p + 1));
                    
                    return vmCompilerMetaNewWithOperator(zone,op);
                }
                else{
                    
                    nn =  t1->childs->length;
                    
                    n = vm_compiler_tokenizer_operator_index_of(t1, ".", 0);
                    
                    if(n != -1 && nn >2){
                        
                        t2 = (objv_tokenizer_t *) objv_array_last(t1->childs);
                        
                        if(objv_object_isKindOfClass((objv_object_t *) t2, & objv_tokenizer_name_class)){
                            
                            meta = vmCompilerExecAction(t1,0,nn -2,errors);
                            
                            if(meta){
                                
                                op = vmCompilerMetaOperatorNew(zone,opType, t2->range.begin, (vm_uint32_t) ( t2->range.end.p - t2->range.begin.p + 1));
                                
                                vmCompilerMetaOperatorAddCompilerMeta(op, meta);
                                
                                return vmCompilerMetaNewWithOperator(zone,op);
                            }
                            
                        }
                        
                    }
                    
                }
                
            }
            
        }
        
        if(type == VM_COMPILE_OP_TYPE_ONE_RR)
        {
            objv_tokenizer_t * t1,*t2;
            vmCompilerMetaOperator * op;
            vmCompilerMeta * meta;
            vm_int32_t n,nn;
            if(c == 2 && i==0){
                
                t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs,1);
                
                if(objv_object_isKindOfClass((objv_object_t *) t1, & objv_tokenizer_name_class)){
                    
                    op = vmCompilerMetaOperatorNew(zone,opType, t1->range.begin, (vm_uint32_t) ( t1->range.end.p - t1->range.begin.p + 1));
                    
                    return vmCompilerMetaNewWithOperator(zone,op);
                }
                else{
                    
                    nn =  t1->childs->length;
                    
                    n = vm_compiler_tokenizer_operator_index_of(t1, ".", 0);
                    
                    if(n != -1 && nn >2){
                        
                        t2 = (objv_tokenizer_t *) objv_array_last(t1->childs);
                        
                        if(objv_object_isKindOfClass((objv_object_t *) t2, & objv_tokenizer_name_class)){
                            
                            meta = vmCompilerExecAction(t1,0,nn -2,errors);
                            
                            if(meta){
                                
                                op = vmCompilerMetaOperatorNew(zone,opType, t2->range.begin, (vm_uint32_t) ( t2->range.end.p - t2->range.begin.p + 1));
                                
                                vmCompilerMetaOperatorAddCompilerMeta(op, meta);
                                
                                return vmCompilerMetaNewWithOperator(zone,op);
                            }
                            
                        }
                        
                    }
                    
                }
                
            }
            
            
            
        }
        
        if(type == VM_COMPILE_OP_TYPE_ONE_L)
        {
            objv_tokenizer_t * t1;
            vmCompilerMetaOperator * op;
            vmCompilerMeta * meta;
            if(c == 2 && i==1){
                
                t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 0);
                
                if(objv_object_isKindOfClass((objv_object_t *)t1, & objv_tokenizer_name_class)){
                    
                    op = vmCompilerMetaOperatorNew(zone,opType, t1->range.begin, (vm_uint32_t) ( t1->range.end.p - t1->range.begin.p +1));
                    
                    return vmCompilerMetaNewWithOperator(zone,op);
                }
                else{
                    meta = vmCompilerExecExpression(t1,errors);
                    if(meta){
                        op = vmCompilerMetaOperatorNew(zone,opType, tokenizer->range.begin, 0);
                        vmCompilerMetaOperatorAddCompilerMeta(op, meta);
                        
                        return vmCompilerMetaNewWithOperator(zone,op);
                    }
                }
                
            }
        }
        
        if(type == VM_COMPILE_OP_TYPE_ONE_R)
        {
            objv_tokenizer_t * t1;
            vmCompilerMetaOperator * op;
            vmCompilerMeta * meta;
            if(c == 2 && i==0){
                
                t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 1);
                
                if(objv_object_isKindOfClass((objv_object_t *)t1, & objv_tokenizer_name_class)){
                    
                    op = vmCompilerMetaOperatorNew(zone,opType, t1->range.begin, (vm_uint32_t) (t1->range.end.p - t1->range.begin.p +1));
                    
                    return vmCompilerMetaNewWithOperator(zone,op);
                }
                else{
                    meta = vmCompilerExecExpression(t1,errors);
                    if(meta){
                        op = vmCompilerMetaOperatorNew(zone,opType, tokenizer->range.begin, 0);
                        vmCompilerMetaOperatorAddCompilerMeta(op, meta);
                        
                        return vmCompilerMetaNewWithOperator(zone,op);
                    }
                }
                
            }
        }
        
        if(type == VM_COMPILE_OP_TYPE_TOW)
        {
            objv_tokenizer_t * t1,*t2;
            vmCompilerMetaOperator * op;
            vmCompilerMeta * meta;
            if(c == 3 && i==1){
                
                t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 0);
                t2 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 2);
                
                op = vmCompilerMetaOperatorNew(zone,opType, tokenizer->range.begin, 0);
                
                meta = vmCompilerExecExpression(t1,errors);
                if(meta){
                    vmCompilerMetaOperatorAddCompilerMeta(op, meta);
                }
                else{
                    return NULL;
                }
                
                meta = vmCompilerExecExpression(t2,errors);
                
                if(meta){
                    vmCompilerMetaOperatorAddCompilerMeta(op, meta);
                }
                else{
                    return NULL;
                }
                
                return vmCompilerMetaNewWithOperator(zone,op);
                
            }
        }
        
        if(type == VM_COMPILE_OP_TYPE_THREE)
        {
            objv_tokenizer_t * t1,*t2,* t3;
            vmCompilerMetaOperator * op;
            vmCompilerMeta * meta;
            
            if(c == 5 && i==1){
                
                t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 0);
                t2 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 2);
                t3 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 4);
                
                op = vmCompilerMetaOperatorNew(zone,opType, tokenizer->range.begin, 0);
                
                meta = vmCompilerExecExpression(t1,errors);
                if(meta){
                    vmCompilerMetaOperatorAddCompilerMeta(op, meta);
                }
                else{
                    return NULL;
                }
                
                meta = vmCompilerExecExpression(t2,errors);
                
                if(meta){
                    vmCompilerMetaOperatorAddCompilerMeta(op, meta);
                }
                else{
                    return NULL;
                }
                
                meta = vmCompilerExecExpression(t3,errors);
                
                if(meta){
                    vmCompilerMetaOperatorAddCompilerMeta(op, meta);
                }
                else{
                    return NULL;
                }
                
                return vmCompilerMetaNewWithOperator(zone,op);
                
            }
        }
        
    }
    
    return NULL;
}

static vmCompilerMeta * vmCompilerExecExpression(objv_tokenizer_t * tokenizer,objv_array_t * errors);

static vmCompilerMeta * vmCompilerExecExpressionAssign(objv_tokenizer_t * tokenizer,vmCompilerMeta * meta,objv_array_t * errors){
    
    vmCompilerMetaOperator * op;
    vmCompilerMeta * m;
    objv_tokenizer_t * token;
    vm_int32_t i,c;
    objv_zone_t * zone = tokenizer->base.zone;
    
    if(objv_object_isKindOfClass((objv_object_t *)tokenizer, & objv_tokenizer_name_class)){
        op = vmCompilerMetaOperatorNew(zone,vmOperatorTypeAssign,tokenizer->range.begin,(vm_uint32_t) (tokenizer->range.end.p - tokenizer->range.begin.p + 1));
    }
    else{
        
        i = vm_compiler_tokenizer_operator_index_of(tokenizer, ".", 0);
        
        if(i == -1){
            i = vm_compiler_tokenizer_group_index_of(tokenizer, '[', 0);
        }
        
        if(i == -1){
            return NULL;
        }
        
        
        c =  tokenizer->childs->length;
        
        token = (objv_tokenizer_t *) objv_array_last(tokenizer->childs);
        
        if(objv_object_isKindOfClass((objv_object_t *)token, & objv_tokenizer_name_class)){
            
            op = vmCompilerMetaOperatorNew(zone,vmOperatorTypeAssign,token->range.begin,(vm_uint32_t) (tokenizer->range.end.p - token->range.begin.p + 1));
            
            if(c > 2){
                m = vmCompilerExecAction(tokenizer, 0, c - 2, errors);
                if(m){
                    vmCompilerMetaOperatorAddCompilerMeta(op, m);
                }
                else{
                    vmCompilerErrorSet(errors,token->range.begin, "ExecExpressionAssign");
                    return NULL;
                }
            }
            else{
                vmCompilerErrorSet(errors,token->range.begin, "ExecExpressionAssign");
                return NULL;
            }
        }
        else if(objv_object_isKindOfClass((objv_object_t *)token, & objv_tokenizer_group_class)
                && * token->range.begin.p == '['){
            
            op = vmCompilerMetaOperatorNew(zone,vmOperatorTypeAssign,token->range.begin,0);
            
            if(c > 1){
                m = vmCompilerExecAction(tokenizer, 0, c - 1, errors);
                if(m){
                    vmCompilerMetaOperatorAddCompilerMeta(op, m);
                }
                else{
                    vmCompilerErrorSet(errors,token->range.begin, "ExecExpressionAssign");
                    return NULL;
                }
            }
            else{
                vmCompilerErrorSet(errors,token->range.begin, "ExecExpressionAssign");
                return NULL;
            }
            
            m = vmCompilerExecExpression(token,errors);
            
            if(m){
                vmCompilerMetaOperatorAddCompilerMeta(op, m);
            }
            else{
                vmCompilerErrorSet(errors,token->range.begin, "ExecExpressionAssign");
                return NULL;
            }
        }
        else{
            vmCompilerErrorSet(errors,token->range.begin, "ExecExpressionAssign");
            return NULL;
        }
        
    }
    
    vmCompilerMetaOperatorAddCompilerMeta(op, meta);
    
    return vmCompilerMetaNewWithOperator(zone,op);
}


static vmCompilerMeta * vmCompilerExecExpression(objv_tokenizer_t * tokenizer,objv_array_t * errors){
    
    objv_zone_t * zone = tokenizer->base.zone;
    
    if(objv_object_isKindOfClass((objv_object_t *)tokenizer, & objv_tokenizer_name_class)){
        return vmCompilerMetaNewWithObjectKey(zone,tokenizer->range.begin, (vm_uint32_t) ( tokenizer->range.end.p - tokenizer->range.begin.p + 1));
    }
    else if(objv_object_isKindOfClass((objv_object_t *)tokenizer, & objv_tokenizer_string_class)){
        return vmCompilerMetaNewWithString(zone,tokenizer->range.begin, (vm_uint32_t) (tokenizer->range.end.p - tokenizer->range.begin.p + 1));
    }
    else if(objv_object_isKindOfClass((objv_object_t *)tokenizer, & objv_tokenizer_value_class)){
        if(objv_tokenizer_equal_string(tokenizer, "null")){
            return vmCompilerMetaNew(zone);
        }
        if(objv_tokenizer_equal_string(tokenizer, "false")){
            return vmCompilerMetaNewWithBoolean(zone,vm_false);
        }
        if(objv_tokenizer_equal_string(tokenizer, "true")){
            return vmCompilerMetaNewWithBoolean(zone,vm_true);
        }
        return vmCompilerMetaNewWithNumberString(zone,tokenizer->range.begin, (vm_uint32_t) ( tokenizer->range.end.p - tokenizer->range.begin.p + 1));
    }
    else if(objv_object_isKindOfClass((objv_object_t *)tokenizer, & objv_tokenizer_group_class)
            || objv_object_isKindOfClass((objv_object_t *)tokenizer, & objv_tokenizer_combi_class)){
        {
            
            
            vm_int32_t i;
            vm_int32_t c = tokenizer->childs->length;
            
            i = vm_compiler_tokenizer_operator_index_of(tokenizer, ".", 0);
            
            if(i != -1){
                return vmCompilerExecAction(tokenizer, 0, c, errors);
            }
            
            i = vm_compiler_tokenizer_group_index_of(tokenizer, '[', 0);
            
            if(i != -1){
                return vmCompilerExecAction(tokenizer, 0, c, errors);
            }
            
            i = vm_compiler_tokenizer_operator_index_of(tokenizer, "new", 0);
            if(i != -1){
                {
                    objv_tokenizer_t * t1,*t2;
                    vmCompilerMetaOperator * op;
                    vmCompilerMeta * meta;
                    vm_int32_t j,n;
                    
                    if(c == 2 && i ==0){
                        t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 1);
                        t2 = (objv_tokenizer_t *) objv_array_objectAt(t1->childs, 1);
                        t1 = (objv_tokenizer_t *) objv_array_objectAt(t1->childs, 0);
                        if(objv_object_isKindOfClass((objv_object_t *) t1, & objv_tokenizer_name_class)
                           && objv_object_isKindOfClass((objv_object_t *) t2, & objv_tokenizer_group_class)
                           && * t2->range.begin.p == '('){
                            op = vmCompilerMetaOperatorNew(zone,vmOperatorTypeNew, t1->range.begin, (vm_uint32_t) (t1->range.end.p - t1->range.begin.p + 1));
                            n = t2->childs->length;
                            for(j=0;j<n;j++){
                                t1 = (objv_tokenizer_t *) objv_array_objectAt(t2->childs, j);
                                if(!objv_object_isKindOfClass((objv_object_t *)t1, & objv_tokenizer_operator_class)){
                                    meta = vmCompilerExecExpression(t1,errors);
                                    if(meta){
                                        vmCompilerMetaOperatorAddCompilerMeta(op, meta);
                                    }
                                    else{
                                        return NULL;
                                    }
                                }
                            }
                            return vmCompilerMetaNewWithOperator(zone,op);
                        }
                        else{
                            vmCompilerErrorSet(errors,tokenizer->range.begin, "ExecExpression new");
                        }
                    }
                    else{
                        vmCompilerErrorSet(errors,tokenizer->range.begin, "ExecExpression new");
                    }
                }
                return NULL;
            }
            
            i = vm_compiler_tokenizer_operator_index_of(tokenizer, "is", 0);
            
            if(i != -1){
                {
                    objv_tokenizer_t * t1,*t2;
                    vmCompilerMetaOperator * op;
                    vmCompilerMeta * meta;
                    
                    if(c == 3 && i==1){
                        t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 0);
                        t2 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 2);
                        op = vmCompilerMetaOperatorNew(zone,vmOperatorTypeIs, tokenizer->range.begin, 0);
                        
                        meta = vmCompilerExecExpression(t1,errors);
                        
                        if(meta){
                            vmCompilerMetaOperatorAddCompilerMeta(op, meta);
                        }
                        else{
                            return NULL;
                        }
                        
                        meta = vmCompilerExecExpression(t2,errors);
                        
                        if(meta){
                            vmCompilerMetaOperatorAddCompilerMeta(op, meta);
                        }
                        else{
                            return NULL;
                        }
                        
                        return vmCompilerMetaNewWithOperator(zone,op);
                    }
                    else{
                        vmCompilerErrorSet(errors,tokenizer->range.begin, "ExecExpression is");
                    }
                }
                return NULL;
            }
            {
                vmCompilerMeta * meta = NULL;
                
                if((meta = vmCompilerExecOperator(tokenizer, "++", vmOperatorTypeInc, VM_COMPILE_OP_TYPE_ONE_LL, errors))){
                    return meta;
                }
                
                if((meta = vmCompilerExecOperator(tokenizer, "++", vmOperatorTypeBeforeInc, VM_COMPILE_OP_TYPE_ONE_RR, errors))){
                    return meta;
                }
                
                if((meta = vmCompilerExecOperator(tokenizer, "--", vmOperatorTypeDec, VM_COMPILE_OP_TYPE_ONE_LL, errors))){
                    return meta;
                }
                
                if((meta = vmCompilerExecOperator(tokenizer, "--", vmOperatorTypeBeforeDec, VM_COMPILE_OP_TYPE_ONE_RR, errors))){
                    return meta;
                }
                
                if((meta = vmCompilerExecOperator(tokenizer, "-", vmOperatorTypeAntiNumber, VM_COMPILE_OP_TYPE_ONE_R, errors))){
                    return meta;
                }
                
                if((meta = vmCompilerExecOperator(tokenizer, "!", vmOperatorTypeNot, VM_COMPILE_OP_TYPE_ONE_R, errors))){
                    return meta;
                }
                
                if((meta = vmCompilerExecOperator(tokenizer, "~", vmOperatorTypeBitwiseComplement, VM_COMPILE_OP_TYPE_ONE_R, errors))){
                    return meta;
                }
                
                if((meta = vmCompilerExecOperator(tokenizer, "+", vmOperatorTypeAdd, VM_COMPILE_OP_TYPE_TOW, errors))){
                    return meta;
                }
                
                if((meta = vmCompilerExecOperator(tokenizer, "-", vmOperatorTypeSub, VM_COMPILE_OP_TYPE_TOW, errors))){
                    return meta;
                }
                
                if((meta = vmCompilerExecOperator(tokenizer, "*", vmOperatorTypeMultiplication, VM_COMPILE_OP_TYPE_TOW, errors))){
                    return meta;
                }
                
                if((meta = vmCompilerExecOperator(tokenizer, "/", vmOperatorTypeDivision, VM_COMPILE_OP_TYPE_TOW, errors))){
                    return meta;
                }
                
                if((meta = vmCompilerExecOperator(tokenizer, "%", vmOperatorTypeModulus, VM_COMPILE_OP_TYPE_TOW, errors))){
                    return meta;
                }
                
                if((meta = vmCompilerExecOperator(tokenizer, "&", vmOperatorTypeBitwiseAnd, VM_COMPILE_OP_TYPE_TOW, errors))){
                    return meta;
                }
                
                if((meta = vmCompilerExecOperator(tokenizer, "|", vmOperatorTypeBitwiseOr, VM_COMPILE_OP_TYPE_TOW, errors))){
                    return meta;
                }
                
                if((meta = vmCompilerExecOperator(tokenizer, "^", vmOperatorTypeBitwiseExclusiveOr, VM_COMPILE_OP_TYPE_TOW, errors))){
                    return meta;
                }
                
                if((meta = vmCompilerExecOperator(tokenizer, "<<", vmOperatorTypeBitwiseShiftLeft, VM_COMPILE_OP_TYPE_TOW, errors))){
                    return meta;
                }
                
                if((meta = vmCompilerExecOperator(tokenizer, ">>", vmOperatorTypeBitwiseShiftRight, VM_COMPILE_OP_TYPE_TOW, errors))){
                    return meta;
                }
                
                if((meta = vmCompilerExecOperator(tokenizer, "===", vmOperatorTypeAbsEqual, VM_COMPILE_OP_TYPE_TOW, errors))){
                    return meta;
                }
                
                if((meta = vmCompilerExecOperator(tokenizer, "!==", vmOperatorTypeAbsNotEqual, VM_COMPILE_OP_TYPE_TOW, errors))){
                    return meta;
                }
                
                if((meta = vmCompilerExecOperator(tokenizer, "==", vmOperatorTypeEqual, VM_COMPILE_OP_TYPE_TOW, errors))){
                    return meta;
                }
                
                if((meta = vmCompilerExecOperator(tokenizer, "!=", vmOperatorTypeNotEqual, VM_COMPILE_OP_TYPE_TOW, errors))){
                    return meta;
                }
                
                if((meta = vmCompilerExecOperator(tokenizer, ">", vmOperatorTypeGreater, VM_COMPILE_OP_TYPE_TOW, errors))){
                    return meta;
                }
                
                if((meta = vmCompilerExecOperator(tokenizer, ">=", vmOperatorTypeGreaterEqual, VM_COMPILE_OP_TYPE_TOW, errors))){
                    return meta;
                }
                
                if((meta = vmCompilerExecOperator(tokenizer, "<", vmOperatorTypeLess, VM_COMPILE_OP_TYPE_TOW, errors))){
                    return meta;
                }
                
                if((meta = vmCompilerExecOperator(tokenizer, "<=", vmOperatorTypeLessEqual, VM_COMPILE_OP_TYPE_TOW, errors))){
                    return meta;
                }
                
                if((meta = vmCompilerExecOperator(tokenizer, "&&", vmOperatorTypeAnd, VM_COMPILE_OP_TYPE_TOW, errors))){
                    return meta;
                }
                
                if((meta = vmCompilerExecOperator(tokenizer, "||", vmOperatorTypeOr, VM_COMPILE_OP_TYPE_TOW, errors))){
                    return meta;
                }
                
                if((meta = vmCompilerExecOperator(tokenizer, "?", vmOperatorTypeIfElse, VM_COMPILE_OP_TYPE_THREE, errors))){
                    return meta;
                }
            }
            
            i = vm_compiler_tokenizer_operator_index_of(tokenizer, "=", 0);
            
            if(i != -1){
                {
                    if(c == 3 && i ==1){
                        objv_tokenizer_t * t1,*t2;
                        vmCompilerMeta * meta;
                        t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 0);
                        t2 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 2);
                        
                        meta = vmCompilerExecExpression(t2,errors);
                        
                        if(meta){
                            return vmCompilerExecExpressionAssign(t1, meta, errors);
                        }
                        else{
                            vmCompilerErrorSet(errors,tokenizer->range.begin, "ExecExpression =");
                        }
                    }
                    else{
                        vmCompilerErrorSet(errors,tokenizer->range.begin, "ExecExpression =");
                    }
                }
            }
            
            i = vm_compiler_tokenizer_operator_index_of(tokenizer, "+=", 0);
            
            if(i != -1){
                {
                    if(c == 3 && i ==1){
                        objv_tokenizer_t * t1,*t2;
                        vmCompilerMeta * meta;
                        t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 0);
                        t2 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 2);
                        
                        meta = vmCompilerExecOperator(tokenizer, "+=", vmOperatorTypeAdd, VM_COMPILE_OP_TYPE_TOW, errors);
                        
                        if(meta){
                            return vmCompilerExecExpressionAssign(t1, meta, errors);
                        }
                        else{
                            vmCompilerErrorSet(errors,tokenizer->range.begin, "ExecExpression +=");
                        }
                    }
                    else{
                        vmCompilerErrorSet(errors,tokenizer->range.begin, "ExecExpression +=");
                    }
                }
            }
            
            i = vm_compiler_tokenizer_operator_index_of(tokenizer, "-=", 0);
            
            if(i != -1){
                {
                    if(c == 3 && i ==1){
                        objv_tokenizer_t * t1,*t2;
                        vmCompilerMeta * meta;
                        t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 0);
                        t2 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 2);
                        
                        meta = vmCompilerExecOperator(tokenizer, "-=", vmOperatorTypeSub, VM_COMPILE_OP_TYPE_TOW, errors);
                        
                        if(meta){
                            return vmCompilerExecExpressionAssign(t1, meta, errors);
                        }
                        else{
                            vmCompilerErrorSet(errors,tokenizer->range.begin, "ExecExpression -=");
                        }
                    }
                    else{
                        vmCompilerErrorSet(errors,tokenizer->range.begin, "ExecExpression -=");
                    }
                }
            }
            
            i = vm_compiler_tokenizer_operator_index_of(tokenizer, "*=", 0);
            
            if(i != -1){
                {
                    if(c == 3 && i ==1){
                        objv_tokenizer_t * t1,*t2;
                        vmCompilerMeta * meta;
                        t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 0);
                        t2 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 2);
                        
                        meta = vmCompilerExecOperator(tokenizer, "*=", vmOperatorTypeMultiplication, VM_COMPILE_OP_TYPE_TOW, errors);
                        
                        if(meta){
                            return vmCompilerExecExpressionAssign(t1, meta, errors);
                        }
                        else{
                            vmCompilerErrorSet(errors,tokenizer->range.begin, "ExecExpression *=");
                        }
                    }
                    else{
                        vmCompilerErrorSet(errors,tokenizer->range.begin, "ExecExpression *=");
                    }
                }
            }
            
            i = vm_compiler_tokenizer_operator_index_of(tokenizer, "/=", 0);
            
            if(i != -1){
                {
                    if(c == 3 && i ==1){
                        objv_tokenizer_t * t1,*t2;
                        vmCompilerMeta * meta;
                        t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 0);
                        t2 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 2);
                        
                        meta = vmCompilerExecOperator(tokenizer, "/=", vmOperatorTypeDivision, VM_COMPILE_OP_TYPE_TOW, errors);
                        
                        if(meta){
                            return vmCompilerExecExpressionAssign(t1, meta, errors);
                        }
                        else{
                            vmCompilerErrorSet(errors,tokenizer->range.begin, "ExecExpression /=");
                        }
                    }
                    else{
                        vmCompilerErrorSet(errors,tokenizer->range.begin, "ExecExpression /=");
                    }
                }
            }
            
            i = vm_compiler_tokenizer_operator_index_of(tokenizer, "%=", 0);
            
            if(i != -1){
                {
                    if(c == 3 && i ==1){
                        objv_tokenizer_t * t1,*t2;
                        vmCompilerMeta * meta;
                        t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 0);
                        t2 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 2);
                        
                        meta = vmCompilerExecOperator(tokenizer, "%=", vmOperatorTypeMultiplication, VM_COMPILE_OP_TYPE_TOW, errors);
                        
                        if(meta){
                            return vmCompilerExecExpressionAssign(t1, meta, errors);
                        }
                        else{
                            vmCompilerErrorSet(errors,tokenizer->range.begin, "ExecExpression %=");
                        }
                    }
                    else{
                        vmCompilerErrorSet(errors,tokenizer->range.begin, "ExecExpression %=");
                    }
                }
            }
            
            i = vm_compiler_tokenizer_operator_index_of(tokenizer, "%=", 0);
            
            if(i != -1){
                {
                    if(c == 3 && i ==1){
                        objv_tokenizer_t * t1,*t2;
                        vmCompilerMeta * meta;
                        t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 0);
                        t2 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 2);
                        
                        meta = vmCompilerExecOperator(tokenizer, "%=", vmOperatorTypeMultiplication, VM_COMPILE_OP_TYPE_TOW, errors);
                        
                        if(meta){
                            return vmCompilerExecExpressionAssign(t1, meta, errors);
                        }
                        else{
                            vmCompilerErrorSet(errors,tokenizer->range.begin, "ExecExpression %=");
                        }
                    }
                    else{
                        vmCompilerErrorSet(errors,tokenizer->range.begin, "ExecExpression %=");
                    }
                }
            }
            
            i = vm_compiler_tokenizer_operator_index_of(tokenizer, "<<=", 0);
            
            if(i != -1){
                {
                    if(c == 3 && i ==1){
                        objv_tokenizer_t * t1,*t2;
                        vmCompilerMeta * meta;
                        t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 0);
                        t2 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 2);
                        
                        meta = vmCompilerExecOperator(tokenizer, "<<=", vmOperatorTypeBitwiseShiftLeft, VM_COMPILE_OP_TYPE_TOW, errors);
                        
                        if(meta){
                            return vmCompilerExecExpressionAssign(t1, meta, errors);
                        }
                        else{
                            vmCompilerErrorSet(errors,tokenizer->range.begin, "ExecExpression <<=");
                        }
                    }
                    else{
                        vmCompilerErrorSet(errors,tokenizer->range.begin, "ExecExpression <<=");
                    }
                }
            }
            
            i = vm_compiler_tokenizer_operator_index_of(tokenizer, ">>=", 0);
            
            if(i != -1){
                {
                    if(c == 3 && i ==1){
                        objv_tokenizer_t * t1,*t2;
                        vmCompilerMeta * meta;
                        t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 0);
                        t2 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 2);
                        
                        meta = vmCompilerExecOperator(tokenizer, ">>=", vmOperatorTypeBitwiseShiftRight, VM_COMPILE_OP_TYPE_TOW, errors);
                        
                        if(meta){
                            return vmCompilerExecExpressionAssign(t1, meta, errors);
                        }
                        else{
                            vmCompilerErrorSet(errors,tokenizer->range.begin, "ExecExpression >>=");
                        }
                    }
                    else{
                        vmCompilerErrorSet(errors,tokenizer->range.begin, "ExecExpression >>=");
                    }
                }
            }
            
            i = vm_compiler_tokenizer_operator_index_of(tokenizer, "&=", 0);
            
            if(i != -1){
                {
                    if(c == 3 && i ==1){
                        objv_tokenizer_t * t1,*t2;
                        vmCompilerMeta * meta;
                        t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 0);
                        t2 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 2);
                        
                        meta = vmCompilerExecOperator(tokenizer, "&=", vmOperatorTypeBitwiseAnd, VM_COMPILE_OP_TYPE_TOW, errors);
                        
                        if(meta){
                            return vmCompilerExecExpressionAssign(t1, meta, errors);
                        }
                        else{
                            vmCompilerErrorSet(errors,tokenizer->range.begin, "ExecExpression &=");
                        }
                    }
                    else{
                        vmCompilerErrorSet(errors,tokenizer->range.begin, "ExecExpression &=");
                    }
                }
            }
            
            
            i = vm_compiler_tokenizer_operator_index_of(tokenizer, "|=", 0);
            
            if(i != -1){
                {
                    if(c == 3 && i ==1){
                        objv_tokenizer_t * t1,*t2;
                        vmCompilerMeta * meta;
                        t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 0);
                        t2 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 2);
                        
                        meta = vmCompilerExecOperator(tokenizer, "|=", vmOperatorTypeBitwiseOr, VM_COMPILE_OP_TYPE_TOW, errors);
                        
                        if(meta){
                            return vmCompilerExecExpressionAssign(t1, meta, errors);
                        }
                        else{
                            vmCompilerErrorSet(errors,tokenizer->range.begin, "ExecExpression |=");
                        }
                    }
                    else{
                        vmCompilerErrorSet(errors,tokenizer->range.begin, "ExecExpression |=");
                    }
                }
            }
            
            i = vm_compiler_tokenizer_operator_index_of(tokenizer, "^=", 0);
            
            if(i != -1){
                {
                    if(c == 3 && i ==1){
                        objv_tokenizer_t * t1,*t2;
                        vmCompilerMeta * meta;
                        t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 0);
                        t2 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 2);
                        
                        meta = vmCompilerExecOperator(tokenizer, "^=", vmOperatorTypeBitwiseExclusiveOr, VM_COMPILE_OP_TYPE_TOW, errors);
                        
                        if(meta){
                            return vmCompilerExecExpressionAssign(t1, meta, errors);
                        }
                        else{
                            vmCompilerErrorSet(errors,tokenizer->range.begin, "ExecExpression ^=");
                        }
                    }
                    else{
                        vmCompilerErrorSet(errors,tokenizer->range.begin, "ExecExpression ^=");
                    }
                }
            }
            if(((objv_object_isKindOfClass((objv_object_t *) tokenizer, & objv_tokenizer_group_class)
                 && (* tokenizer->range.begin.p == '[' || * tokenizer->range.begin.p == '('))
                || objv_object_isKindOfClass((objv_object_t *) tokenizer, & objv_tokenizer_combi_class))
               && tokenizer->childs->length ==1){
                return vmCompilerExecExpression((objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 0), errors);
            }
            vmCompilerErrorSet(errors,tokenizer->range.begin, "ExecExpression");
        }
    }
    
    return NULL;
}

static vm_boolean_t vmCompilerExecInvokeVar(objv_tokenizer_t * tokenizer,vmCompilerMetaOperator * op,objv_array_t * errors){
    vm_int32_t i,c = tokenizer->childs->length;
    objv_tokenizer_t * token;
    vm_int32_t s = 0;
    vmCompilerMetaOperator *opp = NULL;
    vmCompilerMeta * m;
    objv_zone_t * zone = tokenizer->base.zone;
    
    for(i=1;i<c;i++){
        token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, i);
        switch (s) {
            case 0:
            {
                opp = vmCompilerMetaOperatorNew(zone,vmOperatorTypeVar, token->range.begin, (vm_uint32_t)( token->range.end.p - token->range.begin.p + 1));
                m = vmCompilerMetaNewWithOperator(zone,opp);
                vmCompilerMetaOperatorAddCompilerMeta(op, m);
                
                s = 1;
            }
                break;
            case 1:
            {
                if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_operator_class)){
                    if(* token->range.begin.p  == ','){
                        s = 0;
                        opp = NULL;
                    }
                    else if( * token->range.begin.p == '='){
                        s = 2;
                    }
                    else{
                        vmCompilerErrorSet(errors,token->range.begin, "ExecInvokeVar");
                        return vm_false;
                    }
                }
            }
                break;
            case 2:
                
                if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_operator_class)){
                    if(* token->range.begin.p  == ','){
                        s = 0;
                        opp = NULL;
                    }
                    else{
                        vmCompilerErrorSet(errors,token->range.begin, "ExecInvokeVar");
                        return vm_false;
                    }
                }
                else{
                    m = vmCompilerExecExpression(token,errors);
                    
                    if(m && opp){
                        vmCompilerMetaOperatorAddCompilerMeta(opp, m);
                    }
                    else{
                        vmCompilerErrorSet(errors,token->range.begin, "ExecInvokeVar");
                        return vm_false;
                    }
                }
                break;
            default:
                break;
        }
    }
    
    return vm_true;
}

static vmCompilerMetaOperator * vmCompilerExecInvokeIf(objv_tokenizer_t * tokenizer,objv_array_t * errors){
    vm_int32_t c = tokenizer->childs->length;
    objv_tokenizer_t * t1,*t2;
    vmCompilerMetaOperator *op;
    vmCompilerMeta * meta;
    objv_zone_t * zone =tokenizer->base.zone;
    
    if(c == 3){
        t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 1);
        t2 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 2);
        op = vmCompilerMetaOperatorNew(zone,vmOperatorTypeIfElseIfElse, tokenizer->range.begin, 0);
        
        meta = vmCompilerExecExpression(t1,errors);
        
        if(!meta){
            vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeIf");
            return NULL;
        }
        
        vmCompilerMetaOperatorAddCompilerMeta(op, meta);
        
        if(objv_object_isKindOfClass((objv_object_t *) t2, & objv_tokenizer_group_class) && * t2->range.begin.p == '{'){
            
            meta = vmCompilerExecInvokes(t2,errors);
            
            if(!meta){
                vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeIf");
                return NULL;
            }
        }
        else{
            meta = vmCompilerExecExpression(t2,errors);
            
            if(!meta){
                vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeIf");
                return NULL;
            }
        }
        
        vmCompilerMetaOperatorAddCompilerMeta(op, meta);
        
        return op;
    }
    else{
        vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeIf");
    }
    return NULL;
}

static vmCompilerMetaOperator * vmCompilerExecInvokeFor(objv_tokenizer_t * tokenizer,objv_array_t * errors){
    vm_int32_t i,f;
    vm_int32_t c = tokenizer->childs->length;
    vmCompilerMetaOperator *op;
    vmCompilerMeta * meta;
    objv_tokenizer_t * t1,*t2,*t3;
    objv_zone_t * zone = tokenizer->base.zone;
    
    if(c == 3){
        
        t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 1);
        t2 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 2);
        
        i = vm_compiler_tokenizer_operator_index_of(t1, "in", 0);
        
        if(i == -1){
            
            op = vmCompilerMetaOperatorNew(zone,vmOperatorTypeFor, tokenizer->range.begin, 0);
            
            f = 0;
            
            while((i = vm_compiler_tokenizer_operator_index_of(t1, ";", f)) != -1){
                
                f = i - f;
                
                if(f ==0){
                    meta = vmCompilerMetaNew(zone);
                    vmCompilerMetaOperatorAddCompilerMeta(op, meta);
                }
                else{
                    meta = vmCompilerExecExpression((objv_tokenizer_t *) objv_array_objectAt(t1->childs, i-1),errors);
                    if(!meta){
                        vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeFor");
                        return NULL;
                    }
                    vmCompilerMetaOperatorAddCompilerMeta(op, meta);
                }
                
                f = i +1;
            }
            
            if(f < t1->childs->length){
                meta = vmCompilerExecExpression((objv_tokenizer_t *) objv_array_objectAt(t1->childs, f),errors);
                if(!meta){
                    vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeFor");
                    return NULL;
                }
                vmCompilerMetaOperatorAddCompilerMeta(op, meta);
            }
            
            if(op->metas->length != 3){
                vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeFor");
                return NULL;
            }
            
        }
        else if( i  >0 ){
            t3  = (objv_tokenizer_t *) objv_array_objectAt(t1->childs, i -1);
            op = vmCompilerMetaOperatorNew(zone,vmOperatorTypeForeach, t3->range.begin, (vm_uint32_t) (t3->range.end.p - t3->range.begin.p + 1));
            t3  = (objv_tokenizer_t *) objv_array_objectAt(t1->childs, i + 1);
            meta = vmCompilerExecExpression(t3,errors);
            
            if(!t3){
                vmCompilerErrorSet(errors,t1->range.begin, "InvokeFor");
                return NULL;
            }
            
            if(!meta){
                vmCompilerErrorSet(errors,t1->range.begin, "InvokeFor");
                return NULL;
            }
            
            vmCompilerMetaOperatorAddCompilerMeta(op, meta);
            
            
        }
        else{
            vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeFor");
            return NULL;
        }
        
        if(objv_object_isKindOfClass((objv_object_t *) t2, & objv_tokenizer_group_class) && * t2->range.begin.p == '{'){
            
            meta = vmCompilerExecInvokes(t2,errors);
            
            if(!meta){
                vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeIf");
                return NULL;
            }
        }
        else{
            meta = vmCompilerExecExpression(t2,errors);
            
            if(!meta){
                vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeIf");
                return NULL;
            }
        }
        
        vmCompilerMetaOperatorAddCompilerMeta(op, meta);
        
        return op;
    }
    else{
        
        vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeFor");
        return NULL;
    }
}

static vmCompilerMetaOperator * vmCompilerExecInvokeWhile(objv_tokenizer_t * tokenizer,objv_array_t * errors){
    vm_int32_t c = tokenizer->childs->length;
    vmCompilerMetaOperator *op;
    vmCompilerMeta * meta;
    objv_tokenizer_t * t1,*t2;
    
    objv_zone_t * zone = tokenizer->base.zone;
    
    if(c == 3){
        
        t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 1);
        t2 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 2);
        
        op = vmCompilerMetaOperatorNew(zone,vmOperatorTypeWhile, tokenizer->range.begin, 0);
        
        meta = vmCompilerExecExpression(t1,errors);
        
        if(!meta){
            vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeWhile");
            return NULL;
        }
        
        vmCompilerMetaOperatorAddCompilerMeta(op, meta);
        
        if(objv_object_isKindOfClass((objv_object_t *) t2, & objv_tokenizer_group_class) && * t2->range.begin.p == '{'){
            
            meta = vmCompilerExecInvokes(t2,errors);
            
            if(!meta){
                vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeWhile");
                return NULL;
            }
        }
        else{
            meta = vmCompilerExecExpression(t2,errors);
            
            if(!meta){
                vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeWhile");
                return NULL;
            }
        }
        
        vmCompilerMetaOperatorAddCompilerMeta(op, meta);
        
        return op;
    }
    else{
        vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeWhile");
        return NULL;
    }
}

static vmCompilerMetaOperator * vmCompilerExecInvokeTry(objv_tokenizer_t * tokenizer,objv_array_t * errors){
    vm_int32_t c = tokenizer->childs->length;
    vmCompilerMetaOperator *op;
    vmCompilerMeta * meta;
    objv_tokenizer_t *t2;
    objv_zone_t * zone = tokenizer->base.zone;
    
    if(c == 2){
        
        t2 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 1);
        
        op = vmCompilerMetaOperatorNew(zone,vmOperatorTypeTryCatchFinally, tokenizer->range.begin, 0);
        
        if(objv_object_isKindOfClass((objv_object_t *) t2, & objv_tokenizer_group_class) && * t2->range.begin.p == '{'){
            
            meta = vmCompilerExecInvokes(t2,errors);
            
            if(!meta){
                vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeTry");
                return NULL;
            }
        }
        else{
            meta = vmCompilerExecExpression(t2,errors);
            
            if(!meta){
                vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeTry");
                return NULL;
            }
        }
        
        vmCompilerMetaOperatorAddCompilerMeta(op, meta);
        
        
        return op;
    }
    else{
        vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeTry");
        return NULL;
    }
}

static vmCompilerMetaOperator * vmCompilerExecInvokeThrow(objv_tokenizer_t * tokenizer,objv_array_t * errors){
    vm_int32_t c = tokenizer->childs->length;
    vmCompilerMetaOperator *op;
    vmCompilerMeta * meta;
    objv_tokenizer_t *t1;
    objv_zone_t * zone = tokenizer->base.zone;
    if(c == 2){
        
        t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 1);
        
        op = vmCompilerMetaOperatorNew(zone,vmOperatorTypeThrow, tokenizer->range.begin, 0);
        
        meta = vmCompilerExecExpression(t1,errors);
        
        if(!meta){
            vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeThrow");
            return NULL;
        }
        
        vmCompilerMetaOperatorAddCompilerMeta(op, meta);
        
        return op;
    }
    else{
        vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeThrow");
        return NULL;
    }
}

static vmCompilerMeta * vmCompilerExecInvokeBreak(objv_tokenizer_t * tokenizer,objv_array_t * errors){
    vmCompilerMeta * meta;
    meta =  vmCompilerMetaNew(tokenizer->base.zone);
    meta->type = vmMetaTypeBreak;
    return meta;
}

static vmCompilerMeta * vmCompilerExecInvokeContinue(objv_tokenizer_t * tokenizer,objv_array_t * errors){
    
    vmCompilerMeta * meta;
    meta =  vmCompilerMetaNew(tokenizer->base.zone);
    meta->type = vmMetaTypeBreak;
    return meta;
}

static vmCompilerMeta * vmCompilerExecInvokeReturn(objv_tokenizer_t * tokenizer,objv_array_t * errors){
    vm_int32_t c = tokenizer->childs->length;
    vmCompilerMeta * meta;
    objv_tokenizer_t *t1;
    if(c == 2){
        
        t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 1);
        
        meta = vmCompilerExecExpression(t1,errors);
        
        if(!meta){
            vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeReturn");
            return NULL;
        }
        
        meta->type = meta->type | vmMetaTypeReturn;
        
        return meta;
    }
    else{
        meta = vmCompilerMetaNew(tokenizer->base.zone);
        meta->type = meta->type | vmMetaTypeReturn;
        return meta;
    }
}

static vm_boolean_t vmCompilerExecInvokeCatch(objv_tokenizer_t * tokenizer,vmCompilerMetaOperator * tryop,objv_array_t * errors){
    vm_int32_t c = tokenizer->childs->length;
    vmCompilerMeta * meta;
    objv_tokenizer_t * t1,*t2,*t3,*t4;
    objv_zone_t * zone = tokenizer->base.zone;
    
    if(c == 3){
        
        t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 1);
        t2 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 2);
        
        t3 = (objv_tokenizer_t *) objv_array_objectAt(t1->childs, 0);
        t4 = (objv_tokenizer_t *) objv_array_objectAt(t1->childs, 1);
        
        if(!t3 || !t4){
            vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeCatch");
            return vm_false;
        }
        
        meta = vmCompilerMetaNewWithString(zone,t3->range.begin, (vm_uint32_t) ( t3->range.end.p - t3->range.begin.p + 1));
        
        meta->type = meta->type | vmMetaTypeCatch;
        
        vmCompilerMetaOperatorAddCompilerMeta(tryop, meta);
        
        meta = vmCompilerMetaNewWithString(zone,t4->range.begin, (vm_uint32_t) (t4->range.end.p - t4->range.begin.p + 1));
        
        meta->type = meta->type;
        
        vmCompilerMetaOperatorAddCompilerMeta(tryop, meta);
        
        
        if(objv_object_isKindOfClass((objv_object_t *) t2, & objv_tokenizer_group_class) && * t2->range.begin.p == '{'){
            
            meta = vmCompilerExecInvokes(t2,errors);
            
            if(!meta){
                vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeCatch");
                return vm_false;
            }
        }
        else{
            meta = vmCompilerExecExpression(t2,errors);
            
            if(!meta){
                vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeCatch");
                return vm_false;
            }
        }
        
        vmCompilerMetaOperatorAddCompilerMeta(tryop, meta);
        
        return vm_true;
    }
    else{
        vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeCatch");
    }
    return vm_false;
}

static vm_boolean_t vmCompilerExecInvokeFinally(objv_tokenizer_t * tokenizer,vmCompilerMetaOperator * tryop,objv_array_t * errors){
    vm_int32_t c = tokenizer->childs->length;
    vmCompilerMeta * meta;
    objv_tokenizer_t * t2;
    if(c == 2){
        
        t2 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 1);
        
        if(objv_object_isKindOfClass((objv_object_t *) t2, & objv_tokenizer_group_class) && * t2->range.begin.p == '{'){
            
            meta = vmCompilerExecInvokes(t2,errors);
            
            if(!meta){
                vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeFinally");
                return vm_false;
            }
        }
        else{
            meta = vmCompilerExecExpression(t2,errors);
            
            if(!meta){
                vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeFinally");
                return vm_false;
            }
        }
        meta->type = meta->type | vmMetaTypeFinally;
        
        vmCompilerMetaOperatorAddCompilerMeta(tryop, meta);
        
        return vm_true;
    }
    else{
        vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeFinally");
    }
    return vm_false;
}

static vm_boolean_t  vmCompilerExecInvokeElse(objv_tokenizer_t * tokenizer,vmCompilerMetaOperator * ifop,objv_array_t * errors){
    vm_int32_t c = tokenizer->childs->length;
    vmCompilerMeta * meta;
    objv_tokenizer_t * t1,* t2;
    objv_zone_t * zone = tokenizer->base.zone;
    
    if(c >= 2){
        
        t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 1);
        
        if(objv_object_isKindOfClass((objv_object_t *) t1, & objv_tokenizer_name_class) && objv_tokenizer_equal_string(t1, "if")){
            t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 2);
            t2 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 3);
            
            meta = vmCompilerExecExpression(t1,errors);
            
            
            if(!meta){
                vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeElse");
                return vm_false;
            }
            
            vmCompilerMetaOperatorAddCompilerMeta(ifop, meta);
        }
        else{
            
            t2 = t1;
            
            meta = vmCompilerMetaNewWithBoolean(zone,vm_true);
            
            vmCompilerMetaOperatorAddCompilerMeta(ifop, meta);
        }
        
        if(objv_object_isKindOfClass((objv_object_t *) t2, & objv_tokenizer_group_class) && * t2->range.begin.p == '{'){
            
            meta = vmCompilerExecInvokes(t2,errors);
            
            if(!meta){
                vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeElse");
                return vm_false;
            }
        }
        else{
            meta = vmCompilerExecExpression(t2,errors);
            
            if(!meta){
                vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeElse");
                return vm_false;
            }
        }
        
        vmCompilerMetaOperatorAddCompilerMeta(ifop, meta);
        
        return vm_true;
    }
    else{
        vmCompilerErrorSet(errors,tokenizer->range.begin, "InvokeElse");
    }
    return vm_false;
}

static vmCompilerMeta * vmCompilerExecInvokes(objv_tokenizer_t * tokenizer,objv_array_t * errors){
    objv_zone_t * zone = tokenizer->base.zone;
    vmCompilerMetaOperator * op = vmCompilerMetaOperatorNew(zone,vmOperatorTypeInvoke, tokenizer->range.begin, 0);
    vmCompilerMeta * meta , * nilMeta = vmCompilerMetaNew(zone);
    vmCompilerMetaOperator * ifop = NULL,* tryop = NULL, * top;
    vm_int32_t i,c = tokenizer->childs->length;
    objv_tokenizer_t * token,* t;
    
    for(i=0;i<c;i++){
        
        token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, i);
        
        t = token;
        
        meta = NULL;
        top = NULL;
        
        if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_combi_class)){
            if(token->childs->length >0){
                t = (objv_tokenizer_t *) objv_array_objectAt(token->childs, 0);
            }
            else{
                meta = nilMeta;
            }
        }
        
        if( objv_object_isKindOfClass((objv_object_t *) t, & objv_tokenizer_name_class) ){
            
            if(objv_tokenizer_equal_string(t,"var")){
                if(!vmCompilerExecInvokeVar(token,op,errors)){
                    return NULL;
                }
                ifop = NULL;
                tryop = NULL;
            }
            else if(objv_tokenizer_equal_string(t,"if")){
                ifop =  vmCompilerExecInvokeIf(token,errors);
                if(!ifop){
                    return NULL;
                }
                meta = vmCompilerMetaNewWithOperator(zone,ifop);
                tryop = NULL;
            }
            else if(objv_tokenizer_equal_string(t,"else")){
                tryop = NULL;
                
                if(ifop){
                    if(!vmCompilerExecInvokeElse(token,ifop,errors)){
                        return NULL;
                    }
                }
                else{
                    vmCompilerErrorSet(errors,t->range.begin, "Exec Invokes else");
                    return NULL;
                }
            }
            else if(objv_tokenizer_equal_string(t,"for")){
                tryop = NULL;
                ifop = NULL;
                top =  vmCompilerExecInvokeFor(token,errors);
                if(!top){
                    return NULL;
                }
                meta = vmCompilerMetaNewWithOperator(zone,top);
            }
            else if(objv_tokenizer_equal_string(t,"while")){
                tryop = NULL;
                ifop = NULL;
                top =  vmCompilerExecInvokeWhile(token,errors);
                if(!top){
                    return NULL;
                }
                meta = vmCompilerMetaNewWithOperator(zone,top);
            }
            else if(objv_tokenizer_equal_string(t,"try")){
                ifop = NULL;
                tryop =  vmCompilerExecInvokeTry(token,errors);
                if(!tryop){
                    return NULL;
                }
                meta = vmCompilerMetaNewWithOperator(zone,tryop);
            }
            else if(objv_tokenizer_equal_string(t,"catch")){
                ifop = NULL;
                if(tryop){
                    if(!vmCompilerExecInvokeCatch(token,tryop,errors)){
                        return NULL;
                    }
                }
                else{
                    vmCompilerErrorSet(errors,t->range.begin, "Exec Invokes catch");
                    return NULL;
                }
            }
            else if(objv_tokenizer_equal_string(t,"finally")){
                ifop = NULL;
                if(tryop){
                    if(!vmCompilerExecInvokeFinally(token,tryop,errors)){
                        return NULL;
                    }
                }
                else{
                    vmCompilerErrorSet(errors,t->range.begin, "Exec Invokes finally");
                    return NULL;
                }
            }
            else if(objv_tokenizer_equal_string(t,"throw")){
                tryop = NULL;
                ifop = NULL;
                top =  vmCompilerExecInvokeThrow(token,errors);
                if(!top){
                    return NULL;
                }
                meta = vmCompilerMetaNewWithOperator(zone,top);
            }
            else if(objv_tokenizer_equal_string(t,"break")){
                tryop = NULL;
                ifop = NULL;
                meta =  vmCompilerExecInvokeBreak(token,errors);
                if(!meta){
                    return NULL;
                }
            }
            else if(objv_tokenizer_equal_string(t,"continue")){
                tryop = NULL;
                ifop = NULL;
                meta =  vmCompilerExecInvokeContinue(token,errors);
                if(!meta){
                    return NULL;
                }
            }
            else if(objv_tokenizer_equal_string(t,"return")){
                tryop = NULL;
                ifop = NULL;
                meta =  vmCompilerExecInvokeReturn(token,errors);
                if(!meta){
                    return NULL;
                }
            }
            else{
                tryop = NULL;
                ifop = NULL;
                meta = vmCompilerExecExpression(token,errors);
                if(!meta){
                    return NULL;
                }
            }
            
        }
        else{
            meta = vmCompilerExecExpression(token,errors);
            if(!meta){
                return NULL;
            }
        }
        
        
        if(meta && meta->type != vmMetaTypeVoid){
            vmCompilerMetaOperatorAddCompilerMeta(op, meta);
        }
        
    }
    
    
    return vmCompilerMetaNewWithOperator(zone,op);
}

static vm_boolean_t vmCompilerExecFunction(vmCompilerClassMeta * classMeta,objv_tokenizer_t * tokenizer,objv_array_t * errors){
    vm_int32_t index = 1,i,c;
    objv_tokenizer_t * token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index),*t;
    vmCompilerMetaOperator * op;
    vmCompilerMeta  * meta;
    objv_zone_t * zone = tokenizer->base.zone;
    
    op = vmCompilerMetaOperatorNew(zone,vmOperatorTypeFunction, token->range.begin, (vm_uint32_t) ( token->range.end.p - token->range.begin.p + 1));
    
    vmCompilerClassAddFunction(classMeta, op);
    
    index ++;
    
    token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index);
    
    c = token->childs->length;
    
    for(i=0;i<c;i++){
        t = (objv_tokenizer_t *) objv_array_objectAt(token->childs, i);
        
        if(objv_object_isKindOfClass((objv_object_t *) t, & objv_tokenizer_name_class)){
            meta = vmCompilerMetaNewWithArg(zone,t->range.begin, (vm_uint32_t) (t->range.end.p - t->range.begin.p + 1));
            vmCompilerMetaOperatorAddCompilerMeta(op, meta);
        }
        
    }
    
    index ++;
    
    token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index);
    
    meta = vmCompilerExecInvokes(token,errors);
    
    if(meta == NULL){
        vmCompilerErrorSet(errors,token->range.begin, "ExecFunction");
        return vm_false;
    }
    
    vmCompilerMetaOperatorAddCompilerMeta(op, meta);
    
    return vm_true;
}

vmCompilerClassMeta * vmCompilerClassSource(objv_string_t * source,objv_array_t * errors){
    
    objv_zone_t * zone = source->base.zone;
    objv_tokenizer_t * sourceTokenizer = objv_tokenizer_new(zone,source);
    objv_tokenizer_t * token,* t;
    objv_tokenizer_t * tokenizer;
    vmCompilerClassMeta * classMeta;
    vm_int32_t i,c;
    
    tokenizer = vm_compiler_tokenizer(sourceTokenizer, errors);
    
    if(tokenizer != sourceTokenizer){
        
        vm_compiler_tokenizer_comment_clear(tokenizer);
        
        objv_tokenizer_log(tokenizer, INT32_MAX);
        
        if(vm_compiler_class(tokenizer, errors)){
            
            vm_compiler_tokenizer_optimization(tokenizer);
            
            objv_tokenizer_log(tokenizer, INT32_MAX);
            
            classMeta = vmCompilerClassMetaNew(zone);
            
            c = tokenizer->childs->length;
            
            for(i=0;i<c;i++){
                
                token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, i);
                
                if(token->childs->length >0){
                    
                    t = (objv_tokenizer_t *) objv_array_objectAt(token->childs, 0);
                    
                    if(objv_object_isKindOfClass((objv_object_t *)t, & objv_tokenizer_name_class)){
                        
                        if(objv_tokenizer_equal_string(t, "extends")){
                            if(! vmCompilerExecExtends(classMeta,token,errors)){
                                return NULL;
                            }
                        }
                        else if(objv_tokenizer_equal_string(t, "var")){
                            if(! vmCompilerExecProperty(classMeta,token,errors)){
                                return NULL;
                            }
                        }
                        else if(objv_tokenizer_equal_string(t, "function")){
                            if(! vmCompilerExecFunction(classMeta,token,errors)){
                                return NULL;
                            }
                        }
                        else{
                            vmCompilerErrorSet(errors,token->range.begin, "Compiler Exc Class");
                            return NULL;
                        }
                    }
                    else{
                        vmCompilerErrorSet(errors,token->range.begin, "Compiler Exc Class");
                        
                        return NULL;
                    }
                    
                }
                else{
                    
                    vmCompilerErrorSet(errors,token->range.begin, "Compiler Exc Class");
                    
                    return NULL;
                }
                
            }
            
            return classMeta;
            
        }
        
    }
    else{
        objv_tokenizer_log(tokenizer, INT32_MAX);
    }
    
    return NULL;
    
}
