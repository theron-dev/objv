//
//  objv_tokenizer.c
//  objv
//
//  Created by zhang hailong on 14-2-6.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#include "objv_os.h"
#include "objv.h"
#include "objv_tokenizer.h"


static void hobj_tokenizer_destroy_impl(htokenizer_t * obj,InvokeTickDeclare);

static hobj_t * hobj_tokenizer_copy_impl(htokenizer_t * obj,InvokeTickDeclare);

HOBJ_KEY_IMP(Tokenizer)

HOBJ_CLASS_METHOD_IMP_BEGIN(Tokenizer)

HOBJ_CLASS_METHOD_IMP(destroy, hobj_tokenizer_destroy_impl)

HOBJ_CLASS_METHOD_IMP(copy, hobj_tokenizer_copy_impl)

HOBJ_CLASS_METHOD_IMP_END(Tokenizer)

HOBJ_CLASS_IMP_I(Tokenizer,  htokenizer_t, &hobj_class)

static void hobj_tokenizer_destroy_impl(htokenizer_t * obj,InvokeTickDeclare){
    obj->parent = NULL;
    hobj_release((hobj_t *)obj->childs, InvokeTickArg);
    hobj_release(obj->userInfo, InvokeTickArg);
}

htokenizer_t * htokenizer_new( hcchar * source,InvokeTickDeclare){
    return (htokenizer_t *) hobj_autorelease((hobj_t *)htokenizer_alloc(source,InvokeTickArg), InvokeTickArg);
}

htokenizer_t * htokenizer_alloc(hcchar * source,InvokeTickDeclare){
    htokenizer_t * t = (htokenizer_t *) hobj_alloc(HOBJ_CLASS(Tokenizer), InvokeTickArg);
    hchar * p = (hchar *) source;
    
    t->ofString = source;
    t->range.begin.p = (hchar *) source;
    t->range.begin.line = 0;
    t->range.begin.index = 0;
    
    while(p ){
        
        t->range.end.p = p;
        if(*p == '\n'){
            t->range.end.line ++;
            t->range.end.index = 0;
        }
        else{
            t->range.end.index ++;
        }
        
        if(*p == '\0'){
            break;
        }
        
        p++;
    }
    
    return t;
}

void htokenizer_child_add(htokenizer_t * tokenizer,htokenizer_t * token,InvokeTickDeclare){
    hint32 i = 0,c;
    htokenizer_t * t;
    if(tokenizer->childs == NULL){
        tokenizer->childs = hobj_array_alloc(InvokeTickArg);
    }
    
    c = hobj_array_count(tokenizer->childs, InvokeTickArg);
    
    for(i=0;i<c;i++){
        t = (htokenizer_t *) hobj_array_objectAt(tokenizer->childs, i, InvokeTickArg);
        if(t->range.begin.p > token->range.begin.p){
            break;
        }
    }
    
    hobj_array_insert(tokenizer->childs, (hobj_t *) token, i,InvokeTickArg);
    
    if(token->parent){
        hobj_array_remove(token->parent->childs, (hobj_t *) token, InvokeTickArg);
    }
    
    token->parent = tokenizer;
}

void htokenizer_child_remove(htokenizer_t * tokenizer,htokenizer_t * token,InvokeTickDeclare){
    if(tokenizer && token && token->parent == tokenizer){
        hobj_array_remove(tokenizer->childs, (hobj_t *) token, InvokeTickArg);
        token->parent = NULL;
    }
}

void htokenizer_user_info_set(htokenizer_t * tokenizer,hobj_t * userInfo,InvokeTickDeclare){
    hobj_retain(userInfo, InvokeTickArg);
    hobj_release(tokenizer->userInfo, InvokeTickArg);
    tokenizer->userInfo = userInfo;
}

hbool htokenizer_equal_string(htokenizer_t * tokenizer,hcchar * string){
    hchar * p = tokenizer->range.begin.p;
    hchar * c = (hchar *) string;
    while(p <= tokenizer->range.end.p && c && *c != '\0'){
        
        if(*p != *c){
            break;
        }
        
        p ++;
        c ++;
    }
    return c && *c == '\0' && p > tokenizer->range.end.p;
}

hbool htokenizer_scanf(htokenizer_t * tokenizer,htokenizer_scanf_t scanf,htokenizer_scanf_context_t * ctx,htokenizer_location_t * location,InvokeTickDeclare){
    htokenizer_t * token = NULL;
    hbool rs = hbool_true;
    hint32 i,c,index = 0;
    memset(location, 0, sizeof(htokenizer_location_t));
    
    location->p = tokenizer->range.begin.p;
    location->line = tokenizer->range.begin.line;
    location->index = tokenizer->range.begin.index;
    
    while(location->p <= tokenizer->range.end.p){
        
        c = hobj_array_count(tokenizer->childs, InvokeTickArg);
        
        for(i=index;i<c;i++){
            token = (htokenizer_t *) hobj_array_objectAt(tokenizer->childs, i, InvokeTickArg);
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
        
        rs = (* scanf)(tokenizer,location,token,ctx,InvokeTickArg);
        
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

HOBJ_KEY_IMP(TokenizerComment)

HOBJ_CLASS_IMP(TokenizerComment, htokenizer_comment_t, &hobj_Tokenizer_class)

hbool htokenizer_comment_scanf(htokenizer_t * tokenizer,htokenizer_location_t * locaiton,htokenizer_t * token,htokenizer_scanf_context_t * ctx, InvokeTickDeclare){
    
    htokenizer_comment_t * comment;
    
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
                    comment = (htokenizer_comment_t *) hobj_alloc(HOBJ_CLASS(TokenizerComment), InvokeTickArg);
                    comment->base.ofString = tokenizer->ofString;
                    comment->base.range.begin = ctx->begin;
                    comment->base.range.end = * locaiton;
                    htokenizer_child_add(tokenizer, (htokenizer_t *)comment, InvokeTickArg);
                    hobj_release((hobj_t *) comment, InvokeTickArg);
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
                    comment = (htokenizer_comment_t *) hobj_alloc(HOBJ_CLASS(TokenizerComment), InvokeTickArg);
                    comment->base.ofString = tokenizer->ofString;
                    comment->base.range.begin = ctx->begin;
                    comment->base.range.end = * locaiton;
                    htokenizer_child_add(tokenizer, (htokenizer_t *)comment, InvokeTickArg);
                    hobj_release((hobj_t *) comment, InvokeTickArg);
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
    
    return hbool_true;
}


HOBJ_KEY_IMP(TokenizerString)

HOBJ_CLASS_IMP(TokenizerString,  htokenizer_string_t, &hobj_Tokenizer_class)

hbool htokenizer_string_scanf(htokenizer_t * tokenizer,htokenizer_location_t * locaiton,htokenizer_t * token,htokenizer_scanf_context_t * ctx, InvokeTickDeclare){
    
    htokenizer_string_t * string;
    
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
                    string = (htokenizer_string_t *) hobj_alloc(HOBJ_CLASS(TokenizerString), InvokeTickArg);
                    string->base.ofString = tokenizer->ofString;
                    string->base.range.begin = ctx->begin;
                    string->base.range.end = * locaiton;
                    htokenizer_child_add(tokenizer, (htokenizer_t *)string, InvokeTickArg);
                    hobj_release((hobj_t *) string, InvokeTickArg);
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
                    string = (htokenizer_string_t *) hobj_alloc(HOBJ_CLASS(TokenizerString), InvokeTickArg);
                    string->base.ofString = tokenizer->ofString;
                    string->base.range.begin = ctx->begin;
                    string->base.range.end = * locaiton;
                    htokenizer_child_add(tokenizer, (htokenizer_t *)string, InvokeTickArg);
                    hobj_release((hobj_t *) string, InvokeTickArg);
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
    
    return hbool_true;
}


#define IS_NAME(c)          (( (c) >= 'A' && (c) <= 'Z' ) || ( (c) >= 'a' && (c) <= 'z' ) || ( (c) >= '0' && (c) <= '9' ) ||  (c) == '_')
#define IS_NAME_FIRST(c)    (( (c) >= 'A' && (c) <= 'Z' ) || ( (c) >= 'a' && (c) <= 'z' ) ||  (c) == '_')
#define IS_NUMBER(c)        ( ( (c) >= '0' && (c) <= '9' ) || (c) == 'e'|| (c) == 'E' || (c) == '.')
#define IS_NUMBER_FIRST(c)  ( ( (c) >= '0' && (c) <= '9' ) )
#define IS_HEX(c)           ( ( (c) >= '0' && (c) <= '9' ) || ( (c) >= 'A' && (c) <='F' ) || ( (c) >= 'a' && (c) <='f' ) )

HOBJ_KEY_IMP(TokenizerValue)

HOBJ_CLASS_IMP(TokenizerValue,  htokenizer_value_t, &hobj_Tokenizer_class)

hbool htokenizer_value_scanf(htokenizer_t * tokenizer,htokenizer_location_t * locaiton,htokenizer_t * token,htokenizer_scanf_context_t * ctx, InvokeTickDeclare){
    
    htokenizer_value_t * value;
    
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
                    value = (htokenizer_value_t *) hobj_alloc(HOBJ_CLASS(TokenizerValue), InvokeTickArg);
                    value->base.ofString = tokenizer->ofString;
                    value->base.range.begin = ctx->begin;
                    value->base.range.end = ctx->prev;
                    htokenizer_child_add(tokenizer, (htokenizer_t *)value, InvokeTickArg);
                    hobj_release((hobj_t *) value, InvokeTickArg);
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
                    value = (htokenizer_value_t *) hobj_alloc(HOBJ_CLASS(TokenizerValue), InvokeTickArg);
                    value->base.ofString = tokenizer->ofString;
                    value->base.range.begin = ctx->begin;
                    value->base.range.end = ctx->prev;
                    htokenizer_child_add(tokenizer, (htokenizer_t *)value, InvokeTickArg);
                    hobj_release((hobj_t *) value, InvokeTickArg);
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
                value = (htokenizer_value_t *) hobj_alloc(HOBJ_CLASS(TokenizerValue), InvokeTickArg);
                value->base.ofString = tokenizer->ofString;
                value->base.range.begin = ctx->begin;
                value->base.range.end = ctx->prev;
                htokenizer_child_add(tokenizer, (htokenizer_t *)value, InvokeTickArg);
                hobj_release((hobj_t *) value, InvokeTickArg);
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
    
    return hbool_true;
}

HOBJ_KEY_IMP(TokenizerName)

HOBJ_CLASS_IMP(TokenizerName,  htokenizer_name_t, &hobj_Tokenizer_class)

hbool htokenizer_name_scanf(htokenizer_t * tokenizer,htokenizer_location_t * locaiton,htokenizer_t * token,htokenizer_scanf_context_t * ctx, InvokeTickDeclare){
    
    htokenizer_name_t * name;
    
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
                    name = (htokenizer_name_t *) hobj_alloc(HOBJ_CLASS(TokenizerName), InvokeTickArg);
                    name->base.ofString = tokenizer->ofString;
                    name->base.range.begin = ctx->begin;
                    name->base.range.end = ctx->prev;
                    htokenizer_child_add(tokenizer, (htokenizer_t *)name, InvokeTickArg);
                    hobj_release((hobj_t *) name, InvokeTickArg);
                    
                }
            }
                break;
            default:
                break;
        }
    }
    else{
        if(ctx->s == 1){
            name = (htokenizer_name_t *) hobj_alloc(HOBJ_CLASS(TokenizerName), InvokeTickArg);
            name->base.ofString = tokenizer->ofString;
            name->base.range.begin = ctx->begin;
            name->base.range.end = ctx->prev;
            htokenizer_child_add(tokenizer, (htokenizer_t *)name, InvokeTickArg);
            hobj_release((hobj_t *) name, InvokeTickArg);
        }
        ctx->s = 0;
    }
    
    return hbool_true;
}

HOBJ_KEY_IMP(TokenizerOperator)

HOBJ_CLASS_IMP(TokenizerOperator, htokenizer_operator_t, &hobj_Tokenizer_class)

hbool htokenizer_operator_scanf(htokenizer_t * tokenizer,htokenizer_location_t * locaiton,htokenizer_t * token,htokenizer_scanf_context_t * ctx, InvokeTickDeclare){
    
    htokenizer_operator_t * op;
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
                        op = (htokenizer_operator_t *) hobj_alloc(HOBJ_CLASS(TokenizerOperator), InvokeTickArg);
                        op->base.ofString = tokenizer->ofString;
                        op->base.range.begin = ctx->begin;
                        op->base.range.end = * locaiton;
                        htokenizer_child_add(tokenizer, (htokenizer_t *)op, InvokeTickArg);
                        hobj_release((hobj_t *) op, InvokeTickArg);
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
    
    return hbool_true;
}


HOBJ_KEY_IMP(TokenizerGroup)

HOBJ_CLASS_IMP(TokenizerGroup, htokenizer_group_t, &hobj_Tokenizer_class)

hbool htokenizer_group_scanf(htokenizer_t * tokenizer,htokenizer_location_t * locaiton,htokenizer_t * token,htokenizer_scanf_context_t * ctx, InvokeTickDeclare){
    
    htokenizer_group_t * group;
    hint32 c = sizeof(ctx->beginChar) / sizeof(hchar);
    hint32 i = 0;
    
    if(ctx->focusTokenizer == NULL){
        return hbool_false;
    }
    
    if(token == NULL){
        
        for(i=0;i<c;i++){
            if(ctx->beginChar[i] ==0 || ctx->beginChar[i] == * locaiton->p){
                break;
            }
        }
        
        if( i<c && ctx->beginChar[i] != 0){
            
            group = (htokenizer_group_t *) hobj_alloc(HOBJ_CLASS(TokenizerGroup), InvokeTickArg);
            group->base.ofString = tokenizer->ofString;
            group->base.range.begin = * locaiton;
            
            htokenizer_child_add(ctx->focusTokenizer, (htokenizer_t *)group, InvokeTickArg);
            
            ctx->focusTokenizer = (htokenizer_t *)group;
            
            hobj_release((hobj_t *) group, InvokeTickArg);
        }
        else {
            
            if(hobj_is_kind((hobj_t *)ctx->focusTokenizer, HOBJ_CLASS(TokenizerGroup), InvokeTickArg)){
                
                for(i=0;i<c;i++){
                    if(ctx->endChar[i] ==0 || ctx->endChar[i] == * locaiton->p){
                        break;
                    }
                }
                
                if(i<c && ctx->endChar[i] != 0){
                    group = (htokenizer_group_t *) ctx->focusTokenizer;
                    
                    if(* group->base.range.begin.p == ctx->beginChar[i]){
                        group->base.range.end = * locaiton;
                        ctx->focusTokenizer = group->base.parent;
                    }
                    else{
                        return hbool_false;
                    }
                }
                
            }
        }
    }
    else{
        htokenizer_child_add(ctx->focusTokenizer, (htokenizer_t *)hobj_copy((hobj_t *)token,InvokeTickArg), InvokeTickArg);
    }
    
    return hbool_true;
}

static hobj_t * hobj_tokenizer_copy_impl(htokenizer_t * obj,InvokeTickDeclare){
    if(obj){
        htokenizer_t * tokenizer = (htokenizer_t *) hobj_alloc(obj->base.clazz, InvokeTickArg);
        tokenizer->ofString = obj->ofString;
        tokenizer->range = obj->range;
        return hobj_autorelease((hobj_t *)tokenizer, InvokeTickArg);
    }
    return NULL;
}

HOBJ_KEY_IMP(TokenizerCombi)

HOBJ_CLASS_IMP(TokenizerCombi,  htokenizer_combi_t, &hobj_Tokenizer_class)


htokenizer_combi_t * htokenizer_comib(htokenizer_t * tokenizer,hint32 index,hint32 length,InvokeTickDeclare){
    htokenizer_t * t1 = (htokenizer_t *) hobj_array_objectAt(tokenizer->childs, index, InvokeTickArg);
    htokenizer_t * t2 = (htokenizer_t *) hobj_array_objectAt(tokenizer->childs, index + length - 1, InvokeTickArg);
    htokenizer_t * token = (htokenizer_t *) hobj_new(HOBJ_CLASS(TokenizerCombi), InvokeTickArg);
    hint32 i;
    token->ofString = tokenizer->ofString;
    token->range.begin = t1->range.begin;
    token->range.end = t2->range.end;
    
    for(i=0;i<length;i++){
        htokenizer_child_add(token, (htokenizer_t *) hobj_array_objectAt(tokenizer->childs, index + i, InvokeTickArg), InvokeTickArg);
        i --;
        length --;
    }
    
    htokenizer_child_add(tokenizer, token, InvokeTickArg);
    
    return (htokenizer_combi_t *)token;
}

static void _htokenizer_log_level(hint32 level){
    while(level>0){
        hlog("\t");
        level --;
    }
}

static void _htokenizer_log(htokenizer_t * tokenizer,hint32 level,hint32 maxLevel,InvokeTickDeclare){
    hint32 i,c;
    hchar *p;
    _htokenizer_log_level(level);
    
    hlog("%s %d:%d~%d:%d ",tokenizer->base.clazz->name->name,tokenizer->range.begin.line,tokenizer->range.begin.index,tokenizer->range.end.line,tokenizer->range.end.index);
    
    if(tokenizer->base.clazz != HOBJ_CLASS(Tokenizer)){
        if(hobj_is_kind((hobj_t *) tokenizer , HOBJ_CLASS(TokenizerGroup), InvokeTickArg) && * tokenizer->range.begin.p == '{'){
            hlog("{...}");
        }
        else if(hobj_is_kind((hobj_t *) tokenizer , HOBJ_CLASS(TokenizerCombi), InvokeTickArg)){
            
        }
        else{
            p = tokenizer->range.begin.p;
            while(p <= tokenizer->range.end.p){
                hlog("%c",*p);
                p++;
            }
        }
    }
    
    hlog("\n");
    
    if(level <maxLevel){
        c = hobj_array_count(tokenizer->childs, InvokeTickArg);
        
        for(i=0;i<c;i++){
            _htokenizer_log((htokenizer_t * ) hobj_array_objectAt(tokenizer->childs, i, InvokeTickArg),level +1,maxLevel,InvokeTickArg);
        }
    }
}

void htokenizer_log(htokenizer_t * tokenizer,hint32 level,InvokeTickDeclare){
    _htokenizer_log(tokenizer,0,level,InvokeTickArg);
}

