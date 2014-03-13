//
//  objv_tokenizer.c
//  objv
//
//  Created by zhang hailong on 14-2-6.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv.h"
#include "objv_tokenizer.h"
#include "objv_autorelease.h"
#include "objv_log.h"

OBJV_KEY_IMP(Tokenizer)

static void objv_tokenizer_method_dealloc(objv_class_t * clazz,objv_object_t * object){
    
    objv_tokenizer_t * tokenizer = (objv_tokenizer_t *) object, * p;
    int i;
    
    for(i=0;i< tokenizer->childs->length;i++){
        p = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, i);
        p->parent = NULL;
    }
    
    objv_object_release((objv_object_t *)tokenizer->childs);
    objv_object_release(tokenizer->userInfo);
    objv_object_release((objv_object_t *)tokenizer->ofString);
    
    if(clazz->superClass){
        
        objv_object_dealloc(clazz->superClass, object);
        
    }
}

static objv_object_t * objv_tokenizer_method_copy(objv_class_t * clazz,objv_object_t * object){
    
    objv_tokenizer_t * tokenizer = (objv_tokenizer_t *) object;

    return objv_object_new(object->zone, object->isa,tokenizer->ofString,& tokenizer->range.begin,& tokenizer->range.end,NULL);
}

static objv_object_t * objv_tokenizer_method_init(objv_class_t * clazz,objv_object_t * object,va_list ap){
    
    if(clazz->superClass){
        object = objv_object_initv(clazz->superClass, object,ap);
    }
    
    objv_string_t * ofString = va_arg(ap, objv_string_t *);
    objv_tokenizer_location_t * begin = ofString ? va_arg(ap, objv_tokenizer_location_t *) : NULL;
    objv_tokenizer_location_t * end = begin ? va_arg(ap, objv_tokenizer_location_t *) : NULL;
    
    objv_tokenizer_t * tokenizer = (objv_tokenizer_t *) object;
    char * p;
    
    if(ofString == NULL){
        objv_object_release(object);
        return NULL;
    }
    
    tokenizer->ofString = (objv_string_t *) objv_object_retain((objv_object_t *)ofString);
    tokenizer->childs = objv_array_alloc(tokenizer->base.zone, 4);
    
    if(begin && end){
        tokenizer->range.begin = * begin;
        tokenizer->range.end = * end;
    }
    else{
        p = (char *) tokenizer->ofString->UTF8String;
        tokenizer->range.begin.p = p;
        tokenizer->range.begin.line = 0;
        tokenizer->range.begin.index = 0;
        
        while(p ){
            
            tokenizer->range.end.p = p;
            if(*p == '\n'){
                tokenizer->range.end.line ++;
                tokenizer->range.end.index = 0;
            }
            else{
                tokenizer->range.end.index ++;
            }
            
            if(*p == '\0'){
                break;
            }
            
            p++;
        }
    }
    
    return object;
}

static objv_method_t objv_tokenizer_methods[] = {
    {OBJV_KEY(dealloc),"v()",(objv_method_impl_t)objv_tokenizer_method_dealloc}
    ,{OBJV_KEY(copy),"@()",(objv_method_impl_t)objv_tokenizer_method_copy}
    ,{OBJV_KEY(init),"@(*)",(objv_method_impl_t)objv_tokenizer_method_init}
};

objv_class_t objv_tokenizer_class = {OBJV_KEY(Tokenizer),& objv_Object_class
    ,objv_tokenizer_methods,sizeof(objv_tokenizer_methods) / sizeof(objv_method_t)
    ,NULL,0
    ,sizeof(objv_tokenizer_t)
    ,NULL,0};


objv_tokenizer_t * objv_tokenizer_new( objv_zone_t * zone,objv_string_t * source){
    return (objv_tokenizer_t *) objv_object_autorelease((objv_object_t *) objv_tokenizer_alloc(zone,source));
}

objv_tokenizer_t * objv_tokenizer_alloc(objv_zone_t * zone,objv_string_t * source){
    return (objv_tokenizer_t *) objv_object_alloc(zone, &objv_tokenizer_class,source,NULL);
}

void objv_tokenizer_child_add(objv_tokenizer_t * tokenizer,objv_tokenizer_t * token){
    
    if(tokenizer && token && token->parent != tokenizer){
        int i;
        objv_tokenizer_t * p;
        
        for(i=0;i<tokenizer->childs->length;i++){
            p = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, i);
            if(p->range.begin.p > token->range.begin.p){
                break;
            }
        }
        
        objv_array_insertAt(tokenizer->childs, (objv_object_t *) token, i);
        
        if(token->parent){
            objv_array_remove(token->parent->childs, (objv_object_t *) token);
        }
        
        token->parent = tokenizer;
    }
}

void objv_tokenizer_child_remove(objv_tokenizer_t * tokenizer,objv_tokenizer_t * token){
    if(tokenizer && token && token->parent == tokenizer){
        token->parent = NULL;
        objv_array_remove(tokenizer->childs, (objv_object_t *) token);
    }
}

void objv_tokenizer_setUserInfo(objv_tokenizer_t * tokenizer,objv_object_t * userInfo){
    if(tokenizer->userInfo != userInfo){
        objv_object_retain(userInfo);
        objv_object_release(tokenizer->userInfo);
        tokenizer->userInfo = userInfo;
    }
}

objv_boolean_t objv_tokenizer_equal_string(objv_tokenizer_t * tokenizer,const char * string){
    char * p = tokenizer->range.begin.p;
    char * c = (char *) string;
    
    while(p <= tokenizer->range.end.p && c && *c != '\0'){
        
        if(*p != *c){
            break;
        }
        
        p ++;
        c ++;
    }
    return c && *c == '\0' && p > tokenizer->range.end.p;
}

objv_boolean_t objv_tokenizer_scanf(objv_tokenizer_t * tokenizer,objv_tokenizer_scanf_t scanf,objv_tokenizer_scanf_context_t * ctx,objv_tokenizer_location_t * location){
    objv_tokenizer_t * token = NULL;
    objv_boolean_t rs = objv_true;
    int i,index = 0;
    
    memset(location, 0, sizeof(objv_tokenizer_location_t));
    
    location->p = tokenizer->range.begin.p;
    location->line = tokenizer->range.begin.line;
    location->index = tokenizer->range.begin.index;
    
    while(location->p <= tokenizer->range.end.p){
        
        for(i=index;i<tokenizer->childs->length;i++){
            token = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, i);
            if(location->p == token->range.begin.p){
                index = i + 1;
                break;
            }
            else if(location->p < token->range.begin.p){
                token = NULL;
                break;
            }
            else{
                token = NULL;
            }
        }
        
        rs = (* scanf)(tokenizer,location,token,ctx);
        
        if(!rs){
            break;
        }
        
        if(token){
            location->p = token->range.end.p;
            location->line = token->range.end.line;
            location->index = token->range.end.index;
            token = NULL;
            
            if(*location->p == '\n'){
                location->index = 0;
                location->line ++;
            }
        }
        
        if(*location->p == '\n'){
            location->index = 0;
            location->line ++;
        }
        
        location->p ++;
        
        location->index ++;
        
        
        
    }
    
    return rs;
}

OBJV_KEY_IMP(TokenizerComment)


objv_class_t objv_tokenizer_comment_class = {OBJV_KEY(TokenizerComment),& objv_tokenizer_class
    ,NULL,0
    ,NULL,0
    ,sizeof(objv_tokenizer_comment_t)
    ,NULL,0};


objv_boolean_t objv_tokenizer_comment_scanf(objv_tokenizer_t * tokenizer,objv_tokenizer_location_t * locaiton,objv_tokenizer_t * token,objv_tokenizer_scanf_context_t * ctx){
    
    objv_tokenizer_comment_t * comment;
    
    if(token == NULL){
        switch (ctx->s) {
            case 0:
            {
                if( * locaiton->p == '/'){
                    ctx->s = 1;
                    ctx->begin = * locaiton;
                }
                else if( * locaiton->p == '"'){
                    ctx->s = 5;
                }
                else if( * locaiton->p == '\''){
                    ctx->s = 6;
                }
            }
                break;
            case 1:
            {
                if( * locaiton->p == '/'){
                    ctx->s = 2;
                }
                else if(* locaiton->p == '*'){
                    ctx->s = 3;
                }
                else{
                    ctx->s = 0;
                }
            }
                break;
            case 2: //
            {
                if( * locaiton->p == '\n'){
                    comment = (objv_tokenizer_comment_t *) objv_object_alloc(tokenizer->base.zone, &objv_tokenizer_comment_class,tokenizer->ofString, & ctx->begin, locaiton);
                    objv_tokenizer_child_add(tokenizer, (objv_tokenizer_t *)comment);
                    objv_object_release((objv_object_t *) comment);
                    ctx->s = 0;
                }
            }
                break;
            case 3: /* */
            {
                if( * locaiton->p == '*'){
                    ctx->s = 4;
                }
            }
                break;
            case 4:
            {
                if( * locaiton->p == '*'){
                    
                }
                else if( * locaiton->p == '/'){
                    comment = (objv_tokenizer_comment_t *) objv_object_alloc(tokenizer->base.zone, &objv_tokenizer_comment_class,tokenizer->ofString,& ctx->begin, locaiton);
                    objv_tokenizer_child_add(tokenizer, (objv_tokenizer_t *)comment);
                    objv_object_release((objv_object_t *) comment);
                    
                    ctx->s = 0;
                }
                else{
                    ctx->s = 3;
                }
            }
                break;
            case 5:
                if(* locaiton->p == '"'){
                    ctx->s = 0;
                }
                else if(* locaiton->p == '\\'){
                    ctx->s = 7;
                }
                break;
            case 6:
                if(* locaiton->p == '\''){
                    ctx->s = 0;
                }
                else if(* locaiton->p == '\\'){
                    ctx->s = 7;
                }
                break;
            case 7:
                ctx->s = 5;
                break;
            case 8:
                ctx->s = 6;
                break;
            default:
                break;
        }
    }
    else{
        ctx->s = 0;
    }
    
    return objv_true;
}


OBJV_KEY_IMP(TokenizerString)

objv_class_t objv_tokenizer_string_class = {OBJV_KEY(TokenizerString),& objv_tokenizer_class
    ,NULL,0
    ,NULL,0
    ,sizeof(objv_tokenizer_string_t)
    ,NULL,0};

objv_boolean_t objv_tokenizer_string_scanf(objv_tokenizer_t * tokenizer,objv_tokenizer_location_t * locaiton,objv_tokenizer_t * token,objv_tokenizer_scanf_context_t * ctx){
    
    objv_tokenizer_string_t * string;
    
    if(token == NULL){
        switch (ctx->s) {
            case 0:
            {
                if( * locaiton->p == '\''){
                    ctx->s = 0x10;
                    ctx->begin = * locaiton;
                }
                else if( * locaiton->p == '"'){
                    ctx->s = 0x20;
                    ctx->begin = * locaiton;
                }
            }
                break;
            case 0x10:
            {
                if( * locaiton->p == '\\'){
                    ctx->s = 0x11;
                }
                else if(* locaiton->p == '\''){
                    ctx->s = 0;
                    string = (objv_tokenizer_string_t *) objv_object_alloc(tokenizer->base.zone, & objv_tokenizer_string_class,tokenizer->ofString,& ctx->begin, locaiton);
                    objv_tokenizer_child_add(tokenizer, (objv_tokenizer_t *)string);
                    objv_object_release((objv_object_t *) string);
                }
            }
                break;
            case 0x20:
            {
                if( * locaiton->p == '\\'){
                    ctx->s = 0x21;
                }
                else if(* locaiton->p == '"'){
                    ctx->s = 0;
                    string = (objv_tokenizer_string_t *) objv_object_alloc(tokenizer->base.zone, & objv_tokenizer_string_class,tokenizer->ofString,& ctx->begin, locaiton);
                    objv_tokenizer_child_add(tokenizer, (objv_tokenizer_t *)string);
                    objv_object_release((objv_object_t *) string);
                }
            }
                break;
            case 0x11:
            case 0x21:
            {
                ctx->s = ctx->s & 0x0f0;
            }
                break;
            default:
                break;
        }
    }
    else{
        ctx->s = 0;
    }
    
    return objv_true;
}


#define IS_NAME(c)          (( (c) >= 'A' && (c) <= 'Z' ) || ( (c) >= 'a' && (c) <= 'z' ) || ( (c) >= '0' && (c) <= '9' ) ||  (c) == '_')
#define IS_NAME_FIRST(c)    (( (c) >= 'A' && (c) <= 'Z' ) || ( (c) >= 'a' && (c) <= 'z' ) ||  (c) == '_')
#define IS_NUMBER(c)        ( ( (c) >= '0' && (c) <= '9' ) || (c) == 'e'|| (c) == 'E' || (c) == '.')
#define IS_NUMBER_FIRST(c)  ( ( (c) >= '0' && (c) <= '9' ) )
#define IS_HEX(c)           ( ( (c) >= '0' && (c) <= '9' ) || ( (c) >= 'A' && (c) <='F' ) || ( (c) >= 'a' && (c) <='f' ) )

OBJV_KEY_IMP(TokenizerValue)

objv_class_t objv_tokenizer_value_class = {OBJV_KEY(TokenizerValue),& objv_tokenizer_class
    ,NULL,0
    ,NULL,0
    ,sizeof(objv_tokenizer_value_t)
    ,NULL,0};


objv_boolean_t objv_tokenizer_value_scanf(objv_tokenizer_t * tokenizer,objv_tokenizer_location_t * locaiton,objv_tokenizer_t * token,objv_tokenizer_scanf_context_t * ctx){
    
    objv_tokenizer_value_t * value;
    
    if(token == NULL){
        switch (ctx->s) {
            case 0:
            {
                if( locaiton->p[0] == '0' && locaiton->p[1] =='x'){
                    ctx->s = 0x11;
                    ctx->begin = * locaiton;
                    ctx->prev = * locaiton;
                }
                else if(locaiton->p[0] == 't' && locaiton->p[1] == 'r' && locaiton->p[2] == 'u' && locaiton->p[3] == 'e'){
                    ctx->s = 0x32;
                    ctx->begin = * locaiton;
                    ctx->prev = * locaiton;
                }
                else if(locaiton->p[0] == 'f' && locaiton->p[1] == 'a' && locaiton->p[2] == 'l' && locaiton->p[3] == 's' && locaiton->p[4] == 'e'){
                    ctx->s = 0x33;
                    ctx->begin = * locaiton;
                    ctx->prev = * locaiton;
                }
                else if(locaiton->p[0] == 'n' && locaiton->p[1] == 'u' && locaiton->p[2] == 'l' && locaiton->p[3] == 'l'){
                    ctx->s = 0x32;
                    ctx->begin = * locaiton;
                    ctx->prev = * locaiton;
                }
                else if(IS_NUMBER_FIRST(* locaiton->p)){
                    ctx->s = 0x20;
                    ctx->begin = * locaiton;
                    ctx->prev = * locaiton;
                }
                else if(IS_NAME(* locaiton->p)){
                    ctx->s = 0x40;
                }
                
            }
                break;
            case 0x11:
            {
                ctx->s = 0x10;
                ctx->prev = * locaiton;
            }
                break;
            case 0x10:
            {
                if(IS_HEX(*locaiton->p)){
                    ctx->prev = * locaiton;
                }
                else{
                    ctx->s = 0;
                    value = (objv_tokenizer_value_t *) objv_object_alloc(tokenizer->base.zone, & objv_tokenizer_value_class,tokenizer->ofString,& ctx->begin, & ctx->prev);
                    objv_tokenizer_child_add(tokenizer, (objv_tokenizer_t *)value);
                    objv_object_release((objv_object_t *) value);
                }
            }
                break;
            case 0x20:
            {
                if(IS_NUMBER(*locaiton->p)){
                    ctx->prev = * locaiton;
                }
                else{
                    ctx->s = 0;
                    value = (objv_tokenizer_value_t *) objv_object_alloc(tokenizer->base.zone, & objv_tokenizer_value_class,tokenizer->ofString,& ctx->begin, & ctx->prev);
                    objv_tokenizer_child_add(tokenizer, (objv_tokenizer_t *)value);
                    objv_object_release((objv_object_t *) value);
                }
            }
                break;
            case 0x33:
            case 0x32:
            case 0x31:
                ctx->s --;
                ctx->prev = * locaiton;
                break;
            case 0x30:
            {
                ctx->s = 0;
                ctx->prev = * locaiton;
                value = (objv_tokenizer_value_t *) objv_object_alloc(tokenizer->base.zone, & objv_tokenizer_value_class,tokenizer->ofString,& ctx->begin, & ctx->prev);
                objv_tokenizer_child_add(tokenizer, (objv_tokenizer_t *)value);
                objv_object_release((objv_object_t *) value);
            }
                break;
            case 0x40:
                if(IS_NAME(* locaiton->p)){
                    
                }
                else{
                    ctx->s = 0;
                }
                break;
            default:
                break;
        }
    }
    else{
        ctx->s = 0;
    }
    
    return objv_true;
}

OBJV_KEY_IMP(TokenizerName)

objv_class_t objv_tokenizer_name_class = {OBJV_KEY(TokenizerName),& objv_tokenizer_class
    ,NULL,0
    ,NULL,0
    ,sizeof(objv_tokenizer_name_t)
    ,NULL,0};


objv_boolean_t objv_tokenizer_name_scanf(objv_tokenizer_t * tokenizer,objv_tokenizer_location_t * locaiton,objv_tokenizer_t * token,objv_tokenizer_scanf_context_t * ctx){
    
    objv_tokenizer_name_t * name;
    
    if(token == NULL){
        switch (ctx->s) {
            case 0:
            {
                if( IS_NAME_FIRST(* locaiton->p)){
                    ctx->s = 1;
                    ctx->begin = * locaiton;
                    ctx->prev = * locaiton;
                }
                
            }
                break;
            case 1:
            {
                if( IS_NAME( * locaiton->p )){
                    ctx->prev = * locaiton;
                }
                else{
                    ctx->s = 0;
                    name = (objv_tokenizer_name_t *) objv_object_alloc(tokenizer->base.zone, & objv_tokenizer_name_class,tokenizer->ofString,& ctx->begin, & ctx->prev);
                    objv_tokenizer_child_add(tokenizer, (objv_tokenizer_t *)name);
                    objv_object_release((objv_object_t *) name);
                    
                }
            }
                break;
            default:
                break;
        }
    }
    else{
        if(ctx->s == 1){
            name = (objv_tokenizer_name_t *) objv_object_alloc(tokenizer->base.zone, & objv_tokenizer_name_class,tokenizer->ofString,& ctx->begin, & ctx->prev);
            objv_tokenizer_child_add(tokenizer, (objv_tokenizer_t *)name);
            objv_object_release((objv_object_t *) name);
        }
        ctx->s = 0;
    }
    
    return objv_true;
}

OBJV_KEY_IMP(TokenizerOperator)

objv_class_t objv_tokenizer_operator_class = {OBJV_KEY(TokenizerOperator),& objv_tokenizer_class
    ,NULL,0
    ,NULL,0
    ,sizeof(objv_tokenizer_operator_t)
    ,NULL,0};

objv_boolean_t objv_tokenizer_operator_scanf(objv_tokenizer_t * tokenizer,objv_tokenizer_location_t * locaiton,objv_tokenizer_t * token,objv_tokenizer_scanf_context_t * ctx){
    
    objv_tokenizer_operator_t * op;
    if(token == NULL){
        switch (ctx->s) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
                if(* locaiton->p == ctx->op[ctx->s]){
                    if(ctx->s == 0){
                        ctx->begin = * locaiton;
                    }
                    ctx->s ++;
                    if(ctx->op[ctx->s] ==0){
                        ctx->s = 0;
                        op = (objv_tokenizer_operator_t *) objv_object_alloc(tokenizer->base.zone, & objv_tokenizer_operator_class,tokenizer->ofString,& ctx->begin,  locaiton);
                        objv_tokenizer_child_add(tokenizer, (objv_tokenizer_t *)op);
                        objv_object_release((objv_object_t *) op);
                    }
                }
                else{
                    ctx->s = 0;
                }
                break;
            default:
                break;
        }
    }
    else{
        ctx->s = 0;
    }
    
    return objv_true;
}


OBJV_KEY_IMP(TokenizerGroup)

objv_class_t objv_tokenizer_group_class = {OBJV_KEY(TokenizerGroup),& objv_tokenizer_class
    ,NULL,0
    ,NULL,0
    ,sizeof(objv_tokenizer_group_t)
    ,NULL,0};

objv_boolean_t objv_tokenizer_group_scanf(objv_tokenizer_t * tokenizer,objv_tokenizer_location_t * locaiton,objv_tokenizer_t * token,objv_tokenizer_scanf_context_t * ctx){
    
    objv_tokenizer_group_t * group;
    int c = sizeof(ctx->beginChar) / sizeof(char);
    int i = 0;
    
    if(ctx->focusTokenizer == NULL){
        return objv_false;
    }
    
    if(token == NULL){
        
        for(i=0;i<c;i++){
            if(ctx->beginChar[i] ==0 || ctx->beginChar[i] == * locaiton->p){
                break;
            }
        }
        
        if( i<c && ctx->beginChar[i] != 0){
            
            group = (objv_tokenizer_group_t *) objv_object_alloc(tokenizer->base.zone, & objv_tokenizer_group_class,tokenizer->ofString, locaiton, locaiton);
            
            objv_tokenizer_child_add(ctx->focusTokenizer, (objv_tokenizer_t *)group);
            
            ctx->focusTokenizer = (objv_tokenizer_t *) group;
            
            objv_object_release((objv_object_t *) group);
        }
        else {
            
            if(objv_object_isKindOfClass((objv_object_t *)ctx->focusTokenizer, &objv_tokenizer_group_class)){
                
                for(i=0;i<c;i++){
                    if(ctx->endChar[i] ==0 || ctx->endChar[i] == * locaiton->p){
                        break;
                    }
                }
                
                if(i<c && ctx->endChar[i] != 0){
                    
                    group = (objv_tokenizer_group_t *) ctx->focusTokenizer;
                    
                    if(* group->base.range.begin.p == ctx->beginChar[i]){
                        group->base.range.end = * locaiton;
                        ctx->focusTokenizer = group->base.parent;
                    }
                    else{
                        return objv_false;
                    }
                }
                
            }
        }
    }
    else{
        objv_tokenizer_child_add(ctx->focusTokenizer, (objv_tokenizer_t *)objv_object_copy(token->base.isa, (objv_object_t *)token));
    }
    
    return objv_true;
}

OBJV_KEY_IMP(TokenizerCombi)

objv_class_t objv_tokenizer_combi_class = {OBJV_KEY(TokenizerCombi),& objv_tokenizer_class
    ,NULL,0
    ,NULL,0
    ,sizeof(objv_tokenizer_combi_t)
    ,NULL,0};


objv_tokenizer_combi_t * objv_tokenizer_comib(objv_tokenizer_t * tokenizer,int index,int length){
    objv_tokenizer_t * t1 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index);
    objv_tokenizer_t * t2 = (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index + length - 1);
    objv_tokenizer_t * token = (objv_tokenizer_t *) objv_object_new(tokenizer->base.zone, &objv_tokenizer_combi_class, tokenizer->ofString,&t1->range.begin, &t2->range.end);
    int i;
    
    for(i=0;i<length;i++){
        objv_tokenizer_child_add(token, (objv_tokenizer_t *) objv_array_objectAt(tokenizer->childs, index + i));
        i --;
        length --;
    }
    
    objv_tokenizer_child_add(tokenizer, token);
    
    return (objv_tokenizer_combi_t *)token;
}

static void _objv_tokenizer_log_level(int level){
    while(level>0){
        objv_log("\t");
        level --;
    }
}

static void _objv_tokenizer_log(objv_tokenizer_t * tokenizer,int level,int maxLevel){
    int i;
    char *p;
    _objv_tokenizer_log_level(level);
    
    objv_log("%s %d:%d~%d:%d ",tokenizer->base.isa->name,tokenizer->range.begin.line,tokenizer->range.begin.index,tokenizer->range.end.line,tokenizer->range.end.index);
    
    if(tokenizer->base.isa != & objv_tokenizer_class){
        if(objv_object_isKindOfClass((objv_object_t *) tokenizer , & objv_tokenizer_group_class) && * tokenizer->range.begin.p == '{'){
            objv_log("{...}");
        }
        else if(objv_object_isKindOfClass((objv_object_t *) tokenizer , & objv_tokenizer_combi_class)){
            
        }
        else{
            p = tokenizer->range.begin.p;
            while(p <= tokenizer->range.end.p){
                objv_log("%c",*p);
                p++;
            }
        }
    }
    
    objv_log("\n");
    
    if(level <maxLevel){

        for(i=0;i<tokenizer->childs->length;i++){
            _objv_tokenizer_log((objv_tokenizer_t * ) objv_array_objectAt(tokenizer->childs, i),level +1,maxLevel);
        }
    }
}

void objv_tokenizer_log(objv_tokenizer_t * tokenizer,int level){
    _objv_tokenizer_log(tokenizer,0,level);
}

