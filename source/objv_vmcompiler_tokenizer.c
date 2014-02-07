//
//  objv_vermin_compiler_tokenizer.c
//  objv
//
//  Created by zhang hailong on 14-2-7.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//


#define READONLY

#include "objv_os.h"
#include "objv.h"
#include "objv_vmcompiler_tokenizer.h"
#include "objv_log.h"

/*
 
 ++
 --
 ===
 !==
 <<=
 >>=
 <<
 >>
 >=
 <=
 &&
 ||
 ==
 !=
 +=
 -=
 *=
 /=
 %=
 &=
 ^=
 |=
 
 +
 -
 *
 /
 %
 &
 |
 ^
 ~
 !
 >
 <
 =
 ? :
 
 
 */

objv_tokenizer_t * vm_compiler_tokenizer(objv_tokenizer_t * tokenizer,objv_array_t * errors){
    
    objv_tokenizer_t * focusTokenizer = NULL;
    
    objv_tokenizer_scanf_context_t ctx;
    objv_tokenizer_location_t location;
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_comment_scanf, &ctx, &location)){
        objv_log("Comment scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"Comment scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_string_scanf, &ctx, &location)){
        objv_log("String scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"String scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_value_scanf, &ctx, &location)){
        objv_log("Value scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"Value scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_name_scanf, &ctx, &location)){
        objv_log("Name scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"Name scanf Error");
        return tokenizer;
    }
    
    {
        vm_int32_t i;
        objv_tokenizer_t * token,* t;
        for(i=0;i<tokenizer->childs->length;i++){
            token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, i);
            if(objv_object_isKindOfClass((objv_object_t *) token, & objv_tokenizer_name_class)){
                if(objv_tokenizer_equal_string(token, "is")
                   || objv_tokenizer_equal_string(token, "in")
                   || objv_tokenizer_equal_string(token, "new")){
                    t = (objv_tokenizer_t *) objv_object_alloc(token->base.zone, & objv_tokenizer_operator_class,token->ofString,& token->range.begin,& token->range.end);
                    objv_tokenizer_child_remove(tokenizer, token);
                    objv_tokenizer_child_add(tokenizer, t);
                    objv_object_release((objv_object_t *) t);
                }
            }
        }
    }
    
    objv_log("is in new scanf OK\n");
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    
    strcpy(ctx.op, "++");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("++ scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"++ scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, "--");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("-- scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"-- scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, "===");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("=== scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"=== scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    
    strcpy(ctx.op, "!==");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("!== scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"!== scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    
    strcpy(ctx.op, ">>=");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log(">>= scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,">>= scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    
    strcpy(ctx.op, "<<=");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("<<= scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"<<= scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, "<<");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("<< scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"<< scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, ">>");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log(">> scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,">> scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, ">=");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log(">= scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,">= scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, "<=");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("<= scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"<= scanf Error");
        return tokenizer;
    }
    
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, "&&");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("&& scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"&& scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    
    strcpy(ctx.op, "||");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("|| scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"|| scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, "==");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("== scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"== scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, "!=");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("!= scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"!= scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, "+=");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("+= scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"+= scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, "-=");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("-= scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"-= scanf Error");
        return tokenizer;
    }
    
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, "*=");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("*= scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"*= scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, "/=");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("/= scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"/= scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, "%=");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("%%= scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"%= scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, "&=");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("&= scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"&= scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, "^=");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("^= scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"^= scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, "|=");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("|= scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"|= scanf Error");
        return tokenizer;
    }
    
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    
    strcpy(ctx.op, "+");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("+ scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"+ scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    
    strcpy(ctx.op, "-");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("- scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"- scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    
    strcpy(ctx.op, "*");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("* scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"* scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    
    strcpy(ctx.op, "/");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("/ scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"/ scanf Error");
        return tokenizer;
    }
    
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, "%");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("%% scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"% scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, "&");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("& scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"& scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, "|");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("| scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"| scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, "^");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("^ scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"^ scanf Error");
        return tokenizer;
    }
    
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, "~");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("~ scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"~ scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, "!");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("! scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"! scanf Error");
        return tokenizer;
    }
    
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, ">");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("> scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"> scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, "<");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("< scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"< scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, "=");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("= scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"= scanf Error");
        return tokenizer;
    }
    
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, "?");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("? scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"? scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, ":");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log(": scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,": scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, ",");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log(", scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,", scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, ".");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log(". scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,". scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.op, ";");
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_operator_scanf, &ctx, &location)){
        objv_log("; scanf OK\n");
    }
    else{
        vmCompilerErrorSet(errors,location,"; scanf Error");
        return tokenizer;
    }
    
    memset(&ctx, 0, sizeof(objv_tokenizer_scanf_context_t));
    memset(&location, 0, sizeof(objv_tokenizer_location_t));
    
    strcpy(ctx.beginChar, "([{");
    strcpy(ctx.endChar, ")]}");
    
    focusTokenizer = (objv_tokenizer_t *)objv_object_copy(tokenizer->base.isa,(objv_object_t *)tokenizer);
    
    
    ctx.focusTokenizer = focusTokenizer;
    
    if(objv_tokenizer_scanf(tokenizer, objv_tokenizer_group_scanf, &ctx, &location)){
        objv_log("() [] {} scanf OK\n");
    }
    else{
        objv_tokenizer_log(focusTokenizer, INT32_MAX);
        vmCompilerErrorSet(errors,location,"() [] {} scanf Error");
        return tokenizer;
    }
    
    if(ctx.focusTokenizer != focusTokenizer){
        
        vmCompilerErrorSet(errors,ctx.focusTokenizer->range.begin,"() [] {} scanf Error");
        
        return NULL;
    }
    
    return focusTokenizer;
}


void vm_compiler_tokenizer_comment_clear(objv_tokenizer_t * tokenizer){
    vm_int32_t i;
    objv_tokenizer_t * token;
    for(i=0;i<tokenizer->childs->length;i++){
        token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, i);
        if(objv_object_isKindOfClass((objv_object_t *) token,& objv_tokenizer_comment_class)){
            objv_array_removeAt(tokenizer->childs, i);
            i --;
        }
        else{
            vm_compiler_tokenizer_comment_clear(token);
        }
    }
}

void vm_compiler_tokenizer_optimization(objv_tokenizer_t * tokenizer){
    objv_tokenizer_t * token;
    vm_int32_t i;
    while(objv_object_isKindOfClass((objv_object_t *) tokenizer, & objv_tokenizer_combi_class) ){
        if(tokenizer->childs->length ==1){
            token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, 0);
            if(token->range.begin.p == tokenizer->range.begin.p && token->range.end.p == tokenizer->range.end.p){
                objv_object_retain((objv_object_t *) token);
                objv_tokenizer_child_remove(tokenizer, token);
                while(token->childs->length >0){
                    objv_tokenizer_child_add(tokenizer,(objv_tokenizer_t *)objv_array_objectAt(token->childs, 0));
                }
                objv_object_release((objv_object_t *)token);
                continue;
            }
        }
        break;
    }
    for(i=0;i<tokenizer->childs->length;i++){
        token = (objv_tokenizer_t *)objv_array_objectAt(tokenizer->childs, i);
        if(objv_object_isKindOfClass((objv_object_t * ) token, & objv_tokenizer_operator_class)
           && * token->range.begin.p == ';'){
            
            if(token->parent && objv_object_isKindOfClass((objv_object_t * ) token->parent, & objv_tokenizer_group_class) && * token->parent->range.begin.p == '('){
                
            }
            else{
                objv_tokenizer_child_remove(tokenizer, token);
                i --;
            }
        }
        else{
            vm_compiler_tokenizer_optimization(token);
        }
    }
}

vm_int32_t vm_compiler_tokenizer_operator_index_of(objv_tokenizer_t * tokenizer,const char * op,vm_int32_t index){
    vm_int32_t i = index;
    objv_tokenizer_t * token ;
    for(i=index;i<tokenizer->childs->length;i++){
        token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, i);
        if(objv_object_isKindOfClass((objv_object_t *) token, &objv_tokenizer_operator_class)
           && objv_tokenizer_equal_string(token, op)){
            break;
        }
    }
    return i < tokenizer->childs->length ? i : -1;
}

vm_int32_t vm_compiler_tokenizer_group_index_of(objv_tokenizer_t * tokenizer,char beginChar,vm_int32_t index){
    vm_int32_t i = index;
    objv_tokenizer_t * token ;
    for(i=index;i<tokenizer->childs->length;i++){
        token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, i);
        if(objv_object_isKindOfClass((objv_object_t *) token,& objv_tokenizer_group_class)
           && * token->range.begin.p == beginChar){
            break;
        }
    }
    return i < tokenizer->childs->length ? i : -1;
}
