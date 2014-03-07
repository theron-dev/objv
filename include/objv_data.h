//
//  objv_data.h
//  objv
//
//  Created by zhang hailong on 14-3-7.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_data_h
#define objv_objv_data_h


#ifdef __cplusplus
extern "C" {
#endif
    

    typedef struct _objv_data_t {
        objv_object_t base;
        void * READONLY bytes;
        unsigned int READONLY length;
        objv_boolean_t READONLY copyed;
    } objv_data_t;
    
    OBJV_KEY_DEC(Data);
    OBJV_CLASS_DEC(Data);
    
    objv_data_t * objv_data_alloc(objv_zone_t * zone,void * bytes,unsigned int length);
    
    objv_data_t * objv_data_alloc_nocopy(objv_zone_t * zone,void * bytes,unsigned int length);
    
    objv_data_t * objv_data_new(objv_zone_t * zone,void * bytes,unsigned int length);
    
    objv_data_t * objv_data_new_nocopy(objv_zone_t * zone,void * bytes,unsigned int length);
    
    
#ifdef __cplusplus
}
#endif


#endif
