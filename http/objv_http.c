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


void OBJVHTTPHeadersInit(OBJVHttpHeaders * headers){
    objv_zone_memzero(NULL, headers,sizeof(OBJVHttpHeaders));
}

void OBJVHTTPHeadersDestroy(OBJVHttpHeaders * headers){
    if(headers->data){
        objv_zone_free(NULL, headers->data);
    }
    objv_zone_memzero(NULL, headers,sizeof(OBJVHttpHeaders));
}

void OBJVHTTPHeadersClear(OBJVHttpHeaders * headers){
    headers->length = 0;
}

void OBJVHTTPRequestReset(OBJVHttpRequest * request){
    request->method = OBJVHttpStringZero;
    request->path = OBJVHttpStringZero;
    request->version = OBJVHttpStringZero;
    request->state.state = OBJVHttpRequestStateNone;
    request->state.key = OBJVHttpStringZero;
    request->state.value = OBJVHttpStringZero;
    request->ofString = NULL;
    request->length = 0;
    OBJVHTTPHeadersClear(& request->headers);
}

void OBJVHTTPRequestInit(OBJVHttpRequest * request){
    objv_zone_memzero(NULL, request,sizeof(OBJVHttpRequest));
}

void OBJVHTTPRequestDestroy(OBJVHttpRequest * request){
    OBJVHTTPHeadersDestroy( & request->headers);
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
                        request->headers.data = objv_zone_malloc(NULL,request->headers.size * sizeof(OBJVHttpHeader));
                    }
                    else if(request->headers.length + 1 > request->headers.size){
                        request->headers.size += DEF_HEADER_COUNT;
                        request->headers.data = objv_zone_realloc(NULL,request->headers.data,request->headers.size * sizeof(OBJVHttpHeader));
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


OBJVHttpHeader * OBJVHttpHeadersGetHeader(OBJVHttpHeaders * headers,const char * key,char * ofString){
    unsigned int c = headers->length;
    OBJVHttpHeader * h = headers->data;
    
    while(c >0){
        
        if(OBJVHttpStringEqual( h->key, key,ofString)){
            return h;
        }
        
        c --;
        h ++;
    }
    
    return NULL;
}

OBJVHttpHeader * OBJVHttpHeadersNextHeader(OBJVHttpHeaders * headers,OBJVHttpHeader * header){
    if(header - headers->data < headers->length -1){
        return header + 1;
    }
    return NULL;
}

void OBJVHTTPResponseInit(OBJVHttpResponse * response){
    objv_zone_memzero(NULL, response,sizeof(OBJVHttpResponse));
}

void OBJVHTTPResponseDestroy(OBJVHttpResponse * response){
    OBJVHTTPHeadersDestroy( & response->headers);
    objv_zone_memzero(NULL, response,sizeof(OBJVHttpResponse));
}

void OBJVHTTPResponseReset(OBJVHttpResponse * response){
    response->status = OBJVHttpStringZero;
    response->statusCode = OBJVHttpStringZero;
    response->version = OBJVHttpStringZero;
    response->state.state = OBJVHttpRequestStateNone;
    response->state.key = OBJVHttpStringZero;
    response->state.value = OBJVHttpStringZero;
    response->ofString = NULL;
    response->length = 0;
    OBJVHTTPHeadersClear(& response->headers);
}

OBJVHttpRequestState OBJVHTTPResponseRead(OBJVHttpResponse * response,size_t offset,size_t length,char * ofString){
    size_t c = length;
    char * p = ofString + offset;
    size_t off = offset;
    OBJVHttpHeader * h;
    
    response->ofString = ofString;
    
    while(p && c >0){
        
        switch (response->state.state) {
            case OBJVHttpRequestStateNone:
            {
                if(*p == '/'){
                    response->version.location = off+1;
                    response->version.length = 0;
                    response->state.state = OBJVHttpRequestStateVersion;
                }
                
            }
                break;
            case OBJVHttpRequestStateVersion:
            {
                if(*p == ' '){
                    response->state.state = OBJVHttpRequestStateStatusCode;
                    response->statusCode.location = off + 1;
                }
                else{
                    response->version.length ++;
                }
            }
                break;
            case OBJVHttpRequestStateStatusCode:
            {
                if( *p == ' '){
                    response->state.state = OBJVHttpRequestStateStatus;
                    response->status.location = off + 1;
                }
                else{
                    response->statusCode.length ++;
                }
            }
                break;
            case OBJVHttpRequestStateStatus:
            {
                if(*p == '\r'){
                    
                }
                else if( *p == '\n'){
                    response->state.state = OBJVHttpRequestStateHeaderKey;
                    response->state.key.length = 0;
                }
                else{
                    if(response->status.length ==0){
                        response->status.location = off;
                    }
                    response->status.length ++;
                }
            }
                break;
            case OBJVHttpRequestStateHeaderKey:
            {
                if(* p == '\r'){
                    
                }
                else if(*p == '\n'){
                    response->state.state = OBJVHttpRequestStateOK;
                }
                else if(*p == ':'){
                    response->state.state = OBJVHttpRequestStateHeaderValue;
                    response->state.value.length = 0;
                }
                else {
                    if(response->state.key.length ==0){
                        response->state.key.location = off;
                    }
                    response->state.key.length ++;
                }
            }
                break;
            case OBJVHttpRequestStateHeaderValue:
            {
                if(* p == '\r'){
                    
                }
                else if(*p == '\n'){
                    
                    if(response->headers.data == NULL){
                        response->headers.size = DEF_HEADER_COUNT;
                        response->headers.data = objv_zone_malloc(NULL,response->headers.size * sizeof(OBJVHttpHeader));
                    }
                    else if(response->headers.length + 1 > response->headers.size){
                        response->headers.size += DEF_HEADER_COUNT;
                        response->headers.data = objv_zone_realloc(NULL,response->headers.data,response->headers.size * sizeof(OBJVHttpHeader));
                    }
                    
                    h = response->headers.data + response->headers.length;
                    h->key = response->state.key;
                    h->value = response->state.value;
                    
                    response->headers.length ++;
                    
                    response->state.state = OBJVHttpRequestStateHeaderKey;
                    response->state.key.length = 0;
                    response->state.value.length = 0;
                }
                else {
                    if(response->state.value.length ==0){
                        if( *p == ' '){
                            
                        }
                        else {
                            response->state.value.location = off;
                            response->state.value.length = 1;
                        }
                    }
                    else {
                        response->state.value.length ++;
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
    
    response->length = off;
    
    return response->state.state;
}

