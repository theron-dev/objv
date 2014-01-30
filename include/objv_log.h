//
//  objv_log.h
//  objv
//
//  Created by zhang hailong on 14-1-30.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_log_h
#define objv_objv_log_h

#ifdef __cplusplus
extern "C" {
#endif
    
    
    void objv_log(const char * format,...);
    
    void objv_vlog(const char * format,va_list va);
    
#ifdef __cplusplus
}
#endif


#endif
