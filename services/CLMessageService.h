//
//  CLMessageService.h
//  objv
//
//  Created by zhang hailong on 14-3-11.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_CLMessageService_h
#define objv_CLMessageService_h


#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_cloud.h"
    
    typedef struct _CLMessageService {
        CLService base;
    } CLMessageService;
    
    OBJV_CLASS_DEC(CLMessageService)
    
    OBJV_KEY_DEC(CLMessageService)
    
#ifdef __cplusplus
}
#endif


#endif
