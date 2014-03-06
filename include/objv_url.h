//
//  objv_url.h
//  objv
//
//  Created by zhang hailong on 14-3-4.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_url_h
#define objv_objv_url_h


#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_string.h"
#include "objv_dictionary.h"
    
    OBJV_KEY_DEC(URL)
    
    OBJV_CLASS_DEC(URL)
    
    typedef struct _objv_url_t {
        objv_object_t base;
        objv_string_t * READONLY protocol;
        objv_string_t * READONLY domain;
        objv_string_t * READONLY port;
        objv_string_t * READONLY path;
        objv_string_t * READONLY query;
        objv_string_t * READONLY token;
        objv_string_t * READONLY absoluteString;
        objv_dictionary_t * READONLY queryValues;
    } objv_url_t;
    
    objv_string_t * objv_url_encode(objv_zone_t * zone,const char * value);
    
    void objv_url_encode_mbuf(objv_zone_t * zone,const char * value,objv_mbuf_t * mbuf);
    
    objv_string_t * objv_url_decode(objv_zone_t * zone,const char * value);
    
    void objv_url_decode_mbuf(objv_zone_t * zone,const char * value,objv_mbuf_t * mbuf);
    
    
    objv_url_t * objv_url_alloc(objv_zone_t * zone,const char * url);
    
    objv_url_t * objv_url_allocWithFormatV(objv_zone_t * zone,const char * format,va_list va);
    
    objv_url_t * objv_url_allocWithFormat(objv_zone_t * zone,const char * format,...);
    
    objv_url_t * objv_url_allocWithBaseUrl(objv_zone_t * zone,const char * url,objv_url_t * baseUrl);
    
    objv_url_t * objv_url_allocWithQueryValues(objv_zone_t * zone,const char * url,objv_dictionary_t * queryValues);
    
    objv_url_t * objv_url_allocWithBaseUrlAndQueryValues(objv_zone_t * zone,const char * url,objv_url_t * baseUrl,objv_dictionary_t * queryValues);
    
    objv_url_t * objv_url_new(objv_zone_t * zone,const char * url);
    
    objv_url_t * objv_url_newWithBaseUrl(objv_zone_t * zone,const char * url,objv_url_t * baseUrl);
    
    objv_url_t * objv_url_newWithQueryValues(objv_zone_t * zone,const char * url,objv_dictionary_t * queryValues);
    
    objv_url_t * objv_url_newWithBaseUrlAndQueryValues(objv_zone_t * zone,const char * url,objv_url_t * baseUrl,objv_dictionary_t * queryValues);
    
    objv_url_t * objv_url_newWithFormatV(objv_zone_t * zone,const char * format,va_list va);
    
    objv_url_t * objv_url_newWithFormat(objv_zone_t * zone,const char * format,...);
    
    
#ifdef __cplusplus
}
#endif


#endif
