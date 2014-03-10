//
//  objv_mail.c
//  objv
//
//  Created by zhang hailong on 14-3-10.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv.h"
#include "objv_mail.h"
#include "objv_channel_tcp.h"
#include "objv_crypt.h"

OBJV_KEY_IMP(Mail)

static void objv_mail_method_dealloc(objv_class_t * clazz,objv_object_t * object){

    objv_mail_t * mail = (objv_mail_t *) object;
    
    objv_object_release((objv_object_t *) mail->smtp);
    objv_object_release((objv_object_t *) mail->user);
    objv_object_release((objv_object_t *) mail->password);
    objv_object_release((objv_object_t *) mail->exception);
    
    if(clazz->superClass){
        
        objv_object_dealloc(clazz->superClass, object);
        
    }
}

OBJV_CLASS_METHOD_IMP_BEGIN(Mail)

OBJV_CLASS_METHOD_IMP(dealloc,"v()",objv_mail_method_dealloc)

OBJV_CLASS_METHOD_IMP_END(Mail)

OBJV_CLASS_IMP_M(Mail,OBJV_CLASS(Object),objv_mail_t)


void objv_mail_setSmtp(objv_mail_t * mail,objv_url_t * url,objv_string_t * user,objv_string_t * password){
    if(mail){
        if(mail->smtp != url){
            objv_object_retain((objv_object_t *) url);
            objv_object_release((objv_object_t *) mail->smtp);
            mail->smtp = url;
        }
        if(mail->user != user){
            objv_object_retain((objv_object_t *) user);
            objv_object_release((objv_object_t *) mail->user);
            mail->user = user;
        }
        if(mail->password != password){
            objv_object_retain((objv_object_t *) password);
            objv_object_release((objv_object_t *) mail->password);
            mail->password = password;
        }
    }
}

void objv_mail_setException(objv_mail_t * mail, objv_exception_t * exception){
    if(mail && mail->exception != exception){
        objv_object_retain((objv_object_t *) exception);
        objv_object_release((objv_object_t *) mail->exception);
        mail->exception = exception;
    }
}



OBJVChannelStatus objv_mail_send(objv_mail_t * mail, objv_string_t * to, objv_string_t * title,objv_string_t * body,objv_timeinval_t timeout){
    
    if(mail && mail->smtp && mail->user && to && title && body){
        
        objv_zone_t * zone = mail->base.zone;
        OBJVChannelStatus status = OBJVChannelStatusError;
        objv_channel_t * channel = NULL;
        objv_mbuf_t mbuf;
        objv_mbuf_t text;
        int code;
        int s = 0;
        ssize_t len;
        char * p;
        int repeatCount;
        
        if(strcmp(mail->smtp->protocol->UTF8String, "tcp") == 0){
            
            channel = (objv_channel_t *) objv_channel_tcp_allocWithHost(zone, mail->smtp->domain, mail->smtp->port ? atoi(mail->smtp->port->UTF8String) : 25);
            
            objv_mbuf_init(& mbuf, 512);
            objv_mbuf_init(& text, 128);
            
            while(1) {
                
                if(s == 0){
                
                    while((status = objv_channel_connect(channel->base.isa, channel, timeout)) == OBJVChannelStatusNone);
                    
                    if(status != OBJVChannelStatusOK){
                        break;
                    }
                    
                    s = 0x11;
                }
                else if(s == 0x10) {
                    //HELO localhost
                    
                    if(code == 220){
                        
                        status = objv_channel_canWrite(channel->base.isa, channel, timeout);
                        
                        if(status != OBJVChannelStatusOK){
                            break;
                        }
                        
                        objv_mbuf_clear(& mbuf);
                        objv_mbuf_format(& mbuf, "HELO localhost\r\n", mail->smtp->domain);
                        
                        len = objv_channel_write(channel->base.isa, channel, mbuf.data, mbuf.length);
                        
                        if(len < 0){
                            status = OBJVChannelStatusError;
                            break;
                        }
                        else if(len == 0){
                            status = OBJVChannelStatusNone;
                            break;
                        }

                        
                        s = 0x21;
                        
                    }
                    else{
                        
                        status = OBJVChannelStatusError;
                        
                        objv_mail_setException(mail, objv_exception_new(zone, code, "%s",objv_mbuf_str(& text)));
                        
                        break;
                    }
                }
                else if(s == 0x20){
                    // HELO localhost results
                    if(code == 250){
                        
                        // AUTH LOGIN
                        objv_mbuf_clear(& mbuf);
                        
                        objv_mbuf_format(& mbuf, "AUTH LOGIN ");
                        
                        objv_base64_encode(mail->user->UTF8String, mail->user->length, & mbuf,objv_false);
                        
                        objv_mbuf_format(& mbuf, "\r\n");
                        
                        status = objv_channel_canWrite(channel->base.isa, channel, timeout);
                        
                        if(status != OBJVChannelStatusOK){
                            break;
                        }
                        
                        len = objv_channel_write(channel->base.isa, channel, mbuf.data, mbuf.length);
                        
                        if(len < 0){
                            status = OBJVChannelStatusError;
                            break;
                        }
                        else if(len == 0){
                            status = OBJVChannelStatusNone;
                            break;
                        }
                        
                        s = 0x31;
                        
                    }
                    else{
                        
                        status = OBJVChannelStatusError;
                        
                        objv_mail_setException(mail, objv_exception_new(zone, code, "%s",objv_mbuf_str(& text)));
                        
                        break;
                    }
                }
                else if(s == 0x30) {
                    //AUTH LOGIN results
                    
                    if(code == 334){
                        
                        // password
                        objv_mbuf_clear(& mbuf);
                        
                        objv_base64_encode(mail->password->UTF8String, mail->password->length, & mbuf,objv_false);
                        
                        objv_mbuf_format(& mbuf, "\r\n");
                        
                        status = objv_channel_canWrite(channel->base.isa, channel, timeout);
                        
                        if(status != OBJVChannelStatusOK){
                            break;
                        }
                        
                        len = objv_channel_write(channel->base.isa, channel, mbuf.data, mbuf.length);
                        
                        if(len < 0){
                            status = OBJVChannelStatusError;
                            break;
                        }
                        else if(len == 0){
                            status = OBJVChannelStatusNone;
                            break;
                        }
                        
                        s = 0x41;
                    }
                    else{
                        
                        status = OBJVChannelStatusError;
                        
                        objv_mail_setException(mail, objv_exception_new(zone, code, "%s",objv_mbuf_str(& text)));
                        
                        break;
                        
                    }
                    
                }
                else if(s == 0x40) {
                    //password results
                    
                    if(code == 235){
                        s = 0x50;
                    }
                    else{
                        
                        status = OBJVChannelStatusError;
                        
                        objv_mail_setException(mail, objv_exception_new(zone, code, "%s",objv_mbuf_str(& text)));
                        
                        break;
                        
                    }

                }
                else if(s == 0x50){
                    // mail
                    objv_mbuf_clear(& mbuf);
                    
                    objv_mbuf_format(& mbuf, "MAIL FROM:<%s>\r\n",mail->user->UTF8String);

                    status = objv_channel_canWrite(channel->base.isa, channel, timeout);
                    
                    if(status != OBJVChannelStatusOK){
                        break;
                    }
                    
                    len = objv_channel_write(channel->base.isa, channel, mbuf.data, mbuf.length);
                    
                    if(len < 0){
                        status = OBJVChannelStatusError;
                        break;
                    }
                    else if(len == 0){
                        status = OBJVChannelStatusNone;
                        break;
                    }
                    
                    s = 0x61;
                }
                else if(s == 0x60){
                    // MAIL FROM resutls
                    if(code == 250){
                        
                        objv_mbuf_clear(& mbuf);
                        
                        objv_mbuf_format(& mbuf, "RCPT TO:%s\r\n",to->UTF8String);
                        
                        status = objv_channel_canWrite(channel->base.isa, channel, timeout);
                        
                        if(status != OBJVChannelStatusOK){
                            break;
                        }
                        
                        len = objv_channel_write(channel->base.isa, channel, mbuf.data, mbuf.length);
                        
                        if(len < 0){
                            status = OBJVChannelStatusError;
                            break;
                        }
                        else if(len == 0){
                            status = OBJVChannelStatusNone;
                            break;
                        }
                        
                        s = 0x71;
                        
                    }
                    else{
                        status = OBJVChannelStatusError;
                        
                        objv_mail_setException(mail, objv_exception_new(zone, code, "%s",objv_mbuf_str(& text)));
                        
                        break;
                    }
                }
                else if(s == 0x70) {
                    // RCPT TO results
                    if(code == 250){
                        
                        objv_mbuf_clear(& mbuf);
                        
                        objv_mbuf_format(& mbuf, "DATA\r\n");
                        
                        status = objv_channel_canWrite(channel->base.isa, channel, timeout);
                        
                        if(status != OBJVChannelStatusOK){
                            break;
                        }
                        
                        len = objv_channel_write(channel->base.isa, channel, mbuf.data, mbuf.length);
                        
                        if(len < 0){
                            status = OBJVChannelStatusError;
                            break;
                        }
                        else if(len == 0){
                            status = OBJVChannelStatusNone;
                            break;
                        }
                        
                        s = 0x81;

                    }
                    else{
                        
                        status = OBJVChannelStatusError;
                        
                        objv_mail_setException(mail, objv_exception_new(zone, code, "%s",objv_mbuf_str(& text)));
                        
                        break;
                    }
                }
                else if(s == 0x80) {
                    // DATA results
                    if(code == 354){
                       
                        objv_mbuf_clear(& mbuf);
                        
                        objv_mbuf_format(& mbuf, "MIME_Version: 1.0\r\n");
                        objv_mbuf_format(& mbuf, "To: %s\r\n",to->UTF8String);
                        objv_mbuf_format(& mbuf, "From: %s\r\n",mail->user->UTF8String);
                        objv_mbuf_format(& mbuf, "Subject: =?utf8?B?");
                        
                        objv_base64_encode(title->UTF8String, title->length, & mbuf,objv_false);
                        
                        objv_mbuf_format(& mbuf, "?=\r\n");
                        
                        objv_mbuf_format(& mbuf, "Content-Type: text/html; charset=utf8\r\n");
                        objv_mbuf_format(& mbuf, "Content-Transfer-Encoding: base64\r\n\r\n");
                        
                        objv_base64_encode(body->UTF8String, body->length, & mbuf,objv_true);
                        
                        objv_mbuf_format(& mbuf, "\r\n\r\n.\r\n");
                        
                        status = objv_channel_canWrite(channel->base.isa, channel, timeout);
                        
                        if(status != OBJVChannelStatusOK){
                            break;
                        }
                        
                        len = objv_channel_write(channel->base.isa, channel, mbuf.data, mbuf.length);
                        
                        if(len < 0){
                            status = OBJVChannelStatusError;
                            break;
                        }
                        else if(len == 0){
                            status = OBJVChannelStatusNone;
                            break;
                        }
                        
                        s = 0x91;
                        
                    }
                    else{
                        
                        status = OBJVChannelStatusError;
                        
                        objv_mail_setException(mail, objv_exception_new(zone, code, "%s",objv_mbuf_str(& text)));
                        
                        break;
                    }
                }
                else if(s == 0x90){
                    // CONTENT results
                    if(code == 250){
                    
                        objv_mbuf_clear(& mbuf);
                        
                        objv_mbuf_format(& mbuf, "QUIT\r\n");
                        
                        status = objv_channel_canWrite(channel->base.isa, channel, timeout);
                        
                        if(status != OBJVChannelStatusOK){
                            break;
                        }
                        
                        len = objv_channel_write(channel->base.isa, channel, mbuf.data, mbuf.length);
                        
                        if(len < 0){
                            status = OBJVChannelStatusError;
                            break;
                        }
                        else if(len == 0){
                            status = OBJVChannelStatusNone;
                            break;
                        }
                        
                        s = 0xA1;

                    }
                    else{
                        
                        status = OBJVChannelStatusError;
                        
                        objv_mail_setException(mail, objv_exception_new(zone, code, "%s",objv_mbuf_str(& text)));
                        
                        break;
                        
                    }
                }
                else if(s == 0xA0){
                    // QUIT results
                    if(code == 221){
                        
                        status = OBJVChannelStatusOK;
                        
                        break;
                        
                    }
                    else{
                        
                        status = OBJVChannelStatusError;
                        
                        objv_mail_setException(mail, objv_exception_new(zone, code, "%s",objv_mbuf_str(& text)));
                        
                        break;
                        
                    }
                }
                else if((s & 0x0f) == 1){
                    
                   
                    status = objv_channel_canRead(channel->base.isa, channel, timeout);
                    
                    if(status != OBJVChannelStatusOK){
                        break;
                    }
                    
                    objv_mbuf_clear(& mbuf);
                    objv_mbuf_extend(& mbuf, 512);
                    
                    repeatCount = 3;
                    
                    while( (len = objv_channel_read(channel->base.isa, channel, mbuf.data, mbuf.size)) == 0 && repeatCount > 0) {
                        
                        status = objv_channel_canRead(channel->base.isa, channel, timeout);
                        
                        if(status != OBJVChannelStatusOK){
                            break;
                        }
                        
                        repeatCount --;
                    };
                    
                    if(len < 0){
                        status = OBJVChannelStatusError;
                        break;
                    }
                    else if(len == 0){
                        status = OBJVChannelStatusNone;
                        break;
                    }
                    
                    mbuf.length = len;
                    
                    p = (char *) objv_string_indexOf((const char *) objv_mbuf_str(& mbuf), " ");

                    if(p){
                        
                        *p = 0;
                        
                        code = atoi(objv_mbuf_str(& mbuf));
                        
                        objv_mbuf_clear(& text);
                        objv_mbuf_format(& text, "%s", p +1);
                        
                    }
                    else{
                        status = OBJVChannelStatusError;
                        break;
                    }
                    
                    s = s & 0x0f0;
                }
                
                
            }
            
            objv_mbuf_destroy(& mbuf);
            objv_mbuf_destroy(& text);
            
            objv_object_release((objv_object_t *) channel);
        }
        
        else{
            objv_mail_setException(mail, objv_exception_new(zone,0,"not support %s protocol",mail->smtp->protocol));
        }
        
        return status;
    }
    
    return OBJVChannelStatusError;
}


