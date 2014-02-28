//
//  objv_http.c
//  objv
//
//  Created by zhang hailong on 14-2-27.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#include "objv_http.h"


#define DEF_HEADER_COUNT    16

OBJVHttpString OBJVHttpStringZero = {0,0};


void OBJVHTTPRequestReset(OBJVHttpRequest * request){
    objv_zone_memzero(NULL, request,sizeof(OBJVHttpRequest));
}

OBJVHttpRequestState OBJVHTTPRequestRead(OBJVHttpRequest * request,size_t offset,size_t length,char * ofString){
    
    size_t c = length;
    char * p = ofString + offset;
    size_t off = offset;
    OBJVHttpHeader * h;
    
    request->ofString = ofString;
    
    while(p && c >0){
        
        switch (request->state.state) {
            case OBJVHttpRequestStateNone:
            {
                request->method.location = off;
                request->method.length = 1;
                request->state.state = OBJVHttpRequestStateMethod;
            }
                break;
            case OBJVHttpRequestStateMethod:
            {
                if(*p == ' '){
                    request->state.state = OBJVHttpRequestStatePath;
                    request->path.location = off + 1;
                }
                else{
                    request->method.length ++;
                }
            }
                break;
            case OBJVHttpRequestStatePath:
            {
                if( *p == ' '){
                    request->state.state = OBJVHttpRequestStateVersion;
                }
                else{
                    request->path.length ++;
                }
            }
                break;
            case OBJVHttpRequestStateVersion:
            {
                if(*p == '\r'){
                    
                }
                else if( *p == '\n'){
                    request->state.state = OBJVHttpRequestStateHeaderKey;
                    request->state.key.length = 0;
                }
                else{
                    if(request->version.length ==0){
                        request->version.location = off;
                    }
                    request->version.length ++;
                }
            }
                break;
            case OBJVHttpRequestStateHeaderKey:
            {
                if(* p == '\r'){
                    
                }
                else if(*p == '\n'){
                    request->state.state = OBJVHttpRequestStateOK;
                }
                else if(*p == ':'){
                    request->state.state = OBJVHttpRequestStateHeaderValue;
                    request->state.value.length = 0;
                }
                else {
                    if(request->state.key.length ==0){
                        request->state.key.location = off;
                    }
                    request->state.key.length ++;
                }
            }
                break;
            case OBJVHttpRequestStateHeaderValue:
            {
                if(* p == '\r'){
                    
                }
                else if(*p == '\n'){
                    
                    if(request->headers.data == NULL){
                        request->headers.size = DEF_HEADER_COUNT;
                        request->headers.data = malloc(request->headers.size * sizeof(OBJVHttpHeader));
                    }
                    else if(request->headers.length + 1 > request->headers.size){
                        request->headers.size += DEF_HEADER_COUNT;
                        request->headers.data = realloc(request->headers.data,request->headers.size * sizeof(OBJVHttpHeader));
                    }
                    
                    h = request->headers.data + request->headers.length;
                    h->key = request->state.key;
                    h->value = request->state.value;
                    
                    request->headers.length ++;
                    
                    request->state.state = OBJVHttpRequestStateHeaderKey;
                    request->state.key.length = 0;
                    request->state.value.length = 0;
                }
                else {
                    if(request->state.value.length ==0){
                        if( *p == ' '){
                            
                        }
                        else {
                            request->state.value.location = off;
                            request->state.value.length = 1;
                        }
                    }
                    else {
                        request->state.value.length ++;
                    }
                }
            }
                break;
            default:
                return OBJVHttpRequestStateOK;
                break;
        }
        
        c -- ;
        p ++;
        off ++;
    }
    
    request->length = off;
    
    return request->state.state;
}

objv_boolean_t OBJVHttpStringEqual(OBJVHttpString string,const char * cString,char * ofString){
    
    char * p1 = ofString + string.location;
    char * p2 = (char *) cString;
    size_t len = string.length;
    
    while(p2 && p1 && len >0 && *p1 == * p2){
        
        len --;
        p1 ++;
        p2 ++;
    }
    
    return p2 && * p2 == 0 && len == 0;
}

#define TOLOWER(c)  ((c) >= 'A' && (c) <= 'Z' ? (c) + ('a' - 'A') : (c))

objv_boolean_t OBJVHttpStringEqualNoCase(OBJVHttpString string,const char * cString, char * ofString){
    char * p1 = ofString + string.location;
    char * p2 = (char *) cString;
    size_t len = string.length;
    
    while(p2 && p1 && len >0 && TOLOWER(*p1) == TOLOWER(*p2)){

        len --;
        p1 ++;
        p2 ++;
    }
    
    return p2 && * p2 == 0 && len == 0;
}

objv_boolean_t OBJVHttpStringHasPrefix(OBJVHttpString string,const char * cString,char * ofString){
    char * p1 = ofString + string.location;
    char * p2 = (char *) cString;
    size_t len = string.length;
    
    while(p2 && p1 && len >0 && *p1 == * p2){
        
        len --;
        p1 ++;
        p2 ++;
    }
    
    return p2 && * p2 == 0;
}


OBJVHttpHeader * OBJVHttpRequestGetHeader(OBJVHttpRequest * request,const char * key){
    unsigned int c = request->headers.length;
    OBJVHttpHeader * h = request->headers.data;
    
    while(c >0){
        
        if(OBJVHttpStringEqual( h->key, key,request->ofString)){
            return h;
        }
        
        c --;
        h ++;
    }
    
    return NULL;
}

OBJVHttpHeader * OBJVHttpRequestNextHeader(OBJVHttpRequest * request,OBJVHttpHeader * header){
    if(header - request->headers.data < request->headers.length -1){
        return header + 1;
    }
    return NULL;
}
