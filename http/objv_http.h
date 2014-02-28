//
//  objv_http.h
//  objv
//
//  Created by zhang hailong on 14-2-27.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_http_h
#define objv_objv_http_h


#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_os.h"
#include "objv.h"
    
    typedef struct _OBJVHttpString {
        size_t location;
        size_t length;
    } OBJVHttpString;

    
    objv_boolean_t OBJVHttpStringEqual(OBJVHttpString string,const char * cString, char * ofString);
    
    objv_boolean_t OBJVHttpStringEqualNoCase(OBJVHttpString string,const char * cString, char * ofString);
    
    objv_boolean_t OBJVHttpStringHasPrefix(OBJVHttpString string,const char * cString, char * ofString);
    
    extern OBJVHttpString OBJVHttpStringZero;
    
    typedef struct _OBJVHttpHeader {
        OBJVHttpString key;
        OBJVHttpString value;
    } OBJVHttpHeader;
    
    typedef enum _OBJVHttpRequestState {
        OBJVHttpRequestStateNone
        ,OBJVHttpRequestStateMethod
        ,OBJVHttpRequestStatePath
        ,OBJVHttpRequestStateVersion
        ,OBJVHttpRequestStateHeaderKey
        ,OBJVHttpRequestStateHeaderValue
        ,OBJVHttpRequestStateOK
        ,OBJVHttpRequestStateFail
    } OBJVHttpRequestState;
    
    typedef struct _OBJVHttpRequest {
        OBJVHttpString method;
        OBJVHttpString path;
        OBJVHttpString version;
        struct {
            OBJVHttpHeader * data;
            unsigned int size;
            unsigned int length;
        } headers;
        struct {
            OBJVHttpRequestState state;
            OBJVHttpString key;
            OBJVHttpString value;
        } state;
        char * ofString;
        size_t length;
    } OBJVHttpRequest;
    
    void OBJVHTTPRequestReset(OBJVHttpRequest * request);
    
    OBJVHttpRequestState OBJVHTTPRequestRead(OBJVHttpRequest * request,size_t,size_t,char * ofString);
    
    OBJVHttpHeader * OBJVHttpRequestGetHeader(OBJVHttpRequest * request,const char * key);
    
    OBJVHttpHeader * OBJVHttpRequestNextHeader(OBJVHttpRequest * request,OBJVHttpHeader * header);

#ifdef __cplusplus
}
#endif


#endif
