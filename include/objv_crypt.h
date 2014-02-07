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
   
    unsigned long objv_crc32(unsigned long crc32,const char * bytes, unsigned int length);
    
    
#ifdef __cplusplus
}
#endif


#endif
