//
//  objv_inifile.h
//  objv
//
//  Created by zhang hailong on 14-2-8.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_inifile_h
#define objv_objv_inifile_h


#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_mbuf.h"

    typedef struct _objv_inifile_t {
        objv_mbuf_t READONLY key;
        objv_mbuf_t READONLY value;
        objv_mbuf_t READONLY section;
        objv_mbuf_t READONLY data;
        char * READONLY p;
        int READONLY s;
    } objv_inifile_t;
    
    objv_boolean_t objv_inifile_init(objv_inifile_t * inifile,const char * filePath);
    
    void objv_inifile_destroy(objv_inifile_t * inifile);
    
    objv_boolean_t objv_inifile_next(objv_inifile_t * inifile);
    
    const char * objv_inifile_section(objv_inifile_t * inifile);
    
    const char * objv_inifile_key(objv_inifile_t * inifile);
    
    const char * objv_inifile_value(objv_inifile_t * inifile);
    
#ifdef __cplusplus
}
#endif



#endif
