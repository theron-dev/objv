//
//  CLMonitorService.h
//  objv
//
//  Created by zhang hailong on 14-3-10.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_CLMonitorService_h
#define objv_CLMonitorService_h


#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_cloud.h"
  
    typedef struct _CLMonitorService {
        CLService base;
    } CLMonitorService;

    OBJV_CLASS_DEC(CLMonitorService)
    
    OBJV_KEY_DEC(CLMonitorService)
 
#ifdef __cplusplus
}
#endif


#endif
