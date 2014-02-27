//
//  objv_clchannel_http.h
//  objv
//
//  Created by zhang hailong on 14-2-27.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_clchannel_http_h
#define objv_objv_clchannel_http_h


#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_clchannel.h"
#include "objv_http.h"
    
    typedef enum _CLHttpChannelContentType {
        CLHttpChannelContentTypeChunked,
    } CLHttpChannelContentType;
    
    typedef struct _CLHttpChannel{
        CLChannel base;
        OBJVHttpRequest httpRequest;
        
        struct {
            objv_mbuf_t mbuf;
            size_t off;
            int state;
        } read;
        
        struct {
            objv_mbuf_t mbuf;
            size_t off;
            int state;
        } write;
        
        CLHttpChannelContentType contentType;
    } CLHttpChannel;
    
    OBJV_KEY_DEC(CLHttpChannel)
    
    OBJV_CLASS_DEC(CLHttpChannel)
    
#ifdef __cplusplus
}
#endif



#endif
