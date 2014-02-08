//
//  objv_json.h
//  objv
//
//  Created by zhang hailong on 14-2-3.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_json_h
#define objv_objv_json_h


#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_mbuf.h"
#include "objv.h"
#include "objv_string.h"
    
    void objv_json_encode_mbuf(objv_zone_t * zone, objv_object_t * object,objv_mbuf_t * mbuf,objv_boolean_t foramtted);
    
    objv_string_t * objv_json_encode(objv_zone_t * zone,objv_object_t * object,objv_boolean_t foramtted);
    
    objv_object_t * objv_json_decode(objv_zone_t * zone,const char * text);
    
    
#ifdef __cplusplus
}
#endif



#endif
