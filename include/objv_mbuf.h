//
//  objv_mbuf.h
//  objv
//
//  Created by zhang hailong on 14-2-3.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_mbuf_h
#define objv_objv_mbuf_h


#ifdef __cplusplus
extern "C" {
#endif
    
    typedef struct _objv_mbuf_t {
        void * READONLY data;
        size_t READONLY size;
        size_t READONLY length;
    } objv_mbuf_t;
    
    void objv_mbuf_init(objv_mbuf_t * mbuf,size_t capacity);
    
    void objv_mbuf_destroy(objv_mbuf_t * mbuf);
    
    void objv_mbuf_append(objv_mbuf_t * mbuf,void * bytes,size_t length);
   
    void objv_mbuf_formatv(objv_mbuf_t * mbuf,const char * format,va_list va);
    
    void objv_mbuf_format(objv_mbuf_t * mbuf,const char * format,...);
    
    const char * objv_mbuf_str(objv_mbuf_t * mbuf);
    
    void objv_mbuf_clear(objv_mbuf_t * mbuf);
    
#ifdef __cplusplus
}
#endif


#endif
