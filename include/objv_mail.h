//
//  objv_mail.h
//  objv
//
//  Created by zhang hailong on 14-3-10.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_mail_h
#define objv_objv_mail_h


#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_channel.h"
#include "objv_url.h"
#include "objv_exception.h"
    
    OBJV_KEY_DEC(Mail)
    
    OBJV_CLASS_DEC(Mail)
    
    typedef struct _objv_mail_t {
        objv_object_t base;
        objv_url_t * READONLY smtp;
        objv_string_t * READONLY user;
        objv_string_t * READONLY password;
        objv_exception_t * READONLY exception;
    } objv_mail_t;

    void objv_mail_setSmtp(objv_mail_t * mail,objv_url_t * url,objv_string_t * user,objv_string_t * password);
    
    void objv_mail_setException(objv_mail_t * mail, objv_exception_t * exception);
                                
    OBJVChannelStatus objv_mail_send(objv_mail_t * mail, objv_string_t * to, objv_string_t * title,objv_string_t * body,objv_timeinval_t timeout);
    
#ifdef __cplusplus
}
#endif


#endif
