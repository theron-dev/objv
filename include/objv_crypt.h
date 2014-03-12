//
//  objv_crypt.h
//  objv
//
//  Created by zhang hailong on 14-2-7.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_crypt_h
#define objv_objv_crypt_h


#ifdef __cplusplus
extern "C" {
#endif
   
#include "objv_mbuf.h"
    
    unsigned long objv_crc32(unsigned long crc32,const char * bytes, unsigned int length);

    objv_boolean_t objv_base64_encode(void * data,size_t length,objv_mbuf_t * mbuf,objv_boolean_t chunked);
    
    objv_boolean_t objv_base64_decode(const char * text,objv_mbuf_t * mbuf);
    
    objv_boolean_t objv_gzip_encode(void * data,size_t length,objv_mbuf_t * mbuf);
    
    objv_boolean_t objv_gzip_decode(void * data,size_t length,objv_mbuf_t * mbuf);
    
#ifdef __cplusplus
}
#endif


#endif
