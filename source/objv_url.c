//
//  objv_url.c
//  objv
//
//  Created by zhang hailong on 14-3-4.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv.h"
#include "objv_url.h"
#include "objv_value.h"
#include "objv_autorelease.h"

OBJV_KEY_IMP(URL)

static void objv_url_method_dealloc(objv_class_t * clazz,objv_object_t * object){
    
    objv_url_t * url = (objv_url_t *) object;
    
    objv_object_release((objv_object_t *) url->protocol);
    objv_object_release((objv_object_t *) url->domain);
    objv_object_release((objv_object_t *) url->user);
    objv_object_release((objv_object_t *) url->password);
    objv_object_release((objv_object_t *) url->port);
    objv_object_release((objv_object_t *) url->path);
    objv_object_release((objv_object_t *) url->query);
    objv_object_release((objv_object_t *) url->token);
    objv_object_release((objv_object_t *) url->absoluteString);
    objv_object_release((objv_object_t *) url->queryValues);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
    
}

OBJV_CLASS_METHOD_IMP_BEGIN(URL)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", objv_url_method_dealloc)

OBJV_CLASS_METHOD_IMP_END(URL)

OBJV_CLASS_IMP_M(URL, OBJV_CLASS(Object), objv_url_t)


objv_url_t * objv_url_alloc(objv_zone_t * zone,const char * url){
    
    if(url){
        
        objv_url_t * u = (objv_url_t *) objv_object_alloc(zone, OBJV_CLASS(URL),NULL);
        
        char * p = (char *) url;
        char * i = (char *) objv_string_indexOf(p, "://");
        char * ii, *iii;
        
        if(i){
            u->protocol = objv_string_alloc_with_length(zone, p, i - p);
            p = i + 3;
        }
        
        i = (char *) objv_string_indexOf(p, "/");
        
        if(i){
            
            ii = (char *) objv_string_indexOfTo(p, "@",i);
            
            if(ii){
                
                iii = (char *) objv_string_indexOfTo(p, ":",ii);
                
                if(iii){
                    u->user = objv_string_alloc_with_length(zone, p, iii - p);
                    u->password = objv_string_alloc_with_length(zone, iii + 1, iii - ii - 1);
                }
                else{
                    u->user = objv_string_alloc_with_length(zone, p, ii - p);
                }
                
                p = ii + 1;
            }
            
            ii = (char *) objv_string_indexOfTo(p, ":",i);
            
            if(ii && i > ii){
                u->domain = objv_string_alloc_with_length(zone, p, ii - p);
                if(i - 1 > ii){
                    u->port = objv_string_alloc_with_length(zone, ii + 1, i - ii - 1);
                }
            }
            else{
                u->domain = objv_string_alloc_with_length(zone, p, i - p);
            }
            
            p = i;
            
            i = (char *) objv_string_indexOf(p, "?");
            
            if(i){
                
                u->path = objv_string_alloc_with_length(zone, p, i - p);
                
                p = i + 1;
                
                i = (char *) objv_string_indexOf(p, "#");
                
                if(i){
                    
                    u->query = objv_string_alloc_with_length(zone, p, i - p);
                    u->token = objv_string_alloc(zone, i + 1);
                    
                }
                else{
                    u->query = objv_string_alloc(zone, p);
                }
                
            }
            else{
                u->path = objv_string_alloc(zone, p);
            }
        }
        else {
            
            u->path = objv_string_alloc(zone, "/");
            
            i = (char *) objv_string_indexOf(p, "?");
            
            if(i){
                
                u->domain = objv_string_alloc_with_length(zone, p, i - p);
                
                p = i + 1;
                
                i = (char *) objv_string_indexOf(p, "#");
                
                if(i){
                    u->query = objv_string_alloc_with_length(zone, p, i - p);
                    u->token = objv_string_alloc(zone, i + 1);
                    
                }
                else{
                    u->query = objv_string_alloc(zone, p);
                }
                
            }
            else {
                
                i = (char *) objv_string_indexOf(p, "#");
                
                if(i){
                    u->domain = objv_string_alloc_with_length(zone, p, i -p);
                    u->token = objv_string_alloc(zone, i + 1);
                }
                else{
                    u->domain = objv_string_alloc(zone, p);
                }
                
            }
            
        }

        u->absoluteString = objv_string_alloc(zone, url);
        
        if(u->query){
            {
                objv_mbuf_t key;
                objv_mbuf_t value;
                int s =0;
                p = (char *) u->query->UTF8String;
                
                u->queryValues = objv_dictionary_alloc(zone, 4);
                
                objv_mbuf_init(& key, 32);
                objv_mbuf_init(& value, 64);
                
                while(p){
                    
                    if(s == 0){
                        
                        if( *p == 0){
                            break;
                        }
                        
                        if(*p == '&'){
                        
                        }
                        else if(*p == '='){
                            s = 1;
                        }
                        else {
                            objv_mbuf_append(& key, p, 1);
                        }
                    }
                    else {
                        
                        if( *p == '&' || *p == 0){
                            
                            s = 0;
                            
                            objv_dictionary_setValue(u->queryValues, (objv_object_t *) objv_string_new(zone, objv_mbuf_str(& key))
                                                     , (objv_object_t *) objv_url_decode(zone, objv_mbuf_str(& value)));
                            
                            objv_mbuf_clear(& value);
                            objv_mbuf_clear(& key);
                        }
                        else {
                            objv_mbuf_append(& value, p, 1);
                        }
                    }
                    
                    if(*p == 0){
                        break;
                    }
                }
                
                objv_mbuf_destroy(& key);
                objv_mbuf_destroy(& value);
            }
        }
        
        return u;
    }
    
    return NULL;
}

objv_url_t * objv_url_allocWithFormatV(objv_zone_t * zone,const char * format,va_list va){
    objv_mbuf_t mbuf;
    objv_url_t * u;
    
    objv_mbuf_init(& mbuf, 64);
    
    objv_mbuf_formatv(& mbuf, format, va);
    
    u = objv_url_alloc(zone, objv_mbuf_str( & mbuf));
    
    objv_mbuf_destroy( & mbuf);
    
    return u;
}

objv_url_t * objv_url_allocWithFormat(objv_zone_t * zone,const char * format,...){
    va_list ap;
    objv_url_t * u;

    va_start(ap, format);
    
    u = objv_url_allocWithFormatV(zone, format, ap);
    
    va_end(ap);
    
    return u;
}

objv_url_t * objv_url_allocWithBaseUrl(objv_zone_t * zone,const char * url,objv_url_t * baseUrl){
    
    if(url){
        
        if(baseUrl == NULL){
            return objv_url_alloc(zone, url);
        }
        
        char * p = (char *) url;
        char * i = (char *) objv_string_indexOf(p, "://");
        
        if(i){
            return objv_url_alloc(zone, url);
        }
        
        if(*p == '/'){
            
            {
                objv_mbuf_t mbuf;
                objv_url_t * u = NULL;
                objv_mbuf_init(& mbuf, 128);
                
                if(baseUrl->protocol){
                    objv_mbuf_format(& mbuf, "%s://",baseUrl->protocol->UTF8String);
                }
                
                if(baseUrl->domain){
                    objv_mbuf_append(& mbuf, baseUrl->domain->UTF8String,baseUrl->domain->length);
                }
                
                if(baseUrl->port){
                    objv_mbuf_format(& mbuf, ":%s",baseUrl->domain->UTF8String);
                }
                
                objv_mbuf_format(& mbuf, "%s",p);
                
                u = objv_url_alloc(zone, objv_mbuf_str(& mbuf));
                
                objv_mbuf_destroy(& mbuf);
                
                return u;
            }
        
        }
        
        if(*p == '?'){
            
            {
                objv_mbuf_t mbuf;
                objv_url_t * u = NULL;
                objv_mbuf_init(& mbuf, 128);
                
                if(baseUrl->protocol){
                    objv_mbuf_format(& mbuf, "%s://",baseUrl->protocol->UTF8String);
                }
                
                if(baseUrl->domain){
                    objv_mbuf_append(& mbuf, baseUrl->domain->UTF8String,baseUrl->domain->length);
                }
                
                if(baseUrl->port){
                    objv_mbuf_format(& mbuf, ":%s",baseUrl->domain->UTF8String);
                }
                
                if(baseUrl->path){
                    objv_mbuf_format(& mbuf, "%s",baseUrl->path->UTF8String);
                }
                
                objv_mbuf_format(& mbuf, "%s",p);
                
                u = objv_url_alloc(zone, objv_mbuf_str(& mbuf));
                
                objv_mbuf_destroy(& mbuf);
                
                return u;
            }
            
        }
        
        if(*p == '#'){
            
            {
                objv_mbuf_t mbuf;
                objv_url_t * u = NULL;
                objv_mbuf_init(& mbuf, 128);
                
                if(baseUrl->protocol){
                    objv_mbuf_format(& mbuf, "%s://",baseUrl->protocol->UTF8String);
                }
                
                if(baseUrl->domain){
                    objv_mbuf_append(& mbuf, baseUrl->domain->UTF8String,baseUrl->domain->length);
                }
                
                if(baseUrl->port){
                    objv_mbuf_format(& mbuf, ":%s",baseUrl->domain->UTF8String);
                }
                
                if(baseUrl->path){
                    objv_mbuf_format(& mbuf, "%s",baseUrl->path->UTF8String);
                }
                
                if(baseUrl->query){
                    objv_mbuf_format(& mbuf, "%s",baseUrl->query->UTF8String);
                }
                
                objv_mbuf_format(& mbuf, "%s",p);
                
                u = objv_url_alloc(zone, objv_mbuf_str(& mbuf));
                
                objv_mbuf_destroy(& mbuf);
                
                return u;
            }
            
        }
        
        {
            objv_mbuf_t mbuf;
            objv_url_t * u = NULL;
            objv_array_t * paths = NULL;
            objv_string_t * s;
            int i;
            
            objv_mbuf_init(& mbuf, 128);
            
            if(baseUrl->protocol){
                objv_mbuf_format(& mbuf, "%s://",baseUrl->protocol->UTF8String);
            }
            
            if(baseUrl->domain){
                objv_mbuf_append(& mbuf, baseUrl->domain->UTF8String,baseUrl->domain->length);
            }
            
            if(baseUrl->port){
                objv_mbuf_format(& mbuf, ":%s",baseUrl->domain->UTF8String);
            }

            if(baseUrl->path){
                paths = objv_string_split(baseUrl->path, "/");
                objv_array_removeLast(paths);
            }
            
            while(1){
                
                if(p[0] == '.' && p[1] == '.'){
                    objv_array_removeLast(paths);
                    if(p[2] == '/'){
                        p += 3;
                        continue;
                    }
                    else{
                        p += 2;
                        break;
                    }
                }
                else if(p[0] == '.'){
                    if(p[1] == '/'){
                        p += 2;
                        break;
                    }
                    else{
                        p += 1;
                        break;
                    }
                }
                else{
                    break;
                }
            
            }
            
            objv_mbuf_append(& mbuf, "/", 1);
            
            if(paths){
                
                for(i = 0;i<paths->length;i++){
                    s = (objv_string_t *) objv_array_objectAt(paths, i);
                    objv_mbuf_append(& mbuf, s->UTF8String, s->length);
                    objv_mbuf_append(& mbuf, "/", 1);
                }
                
            }
            
            objv_mbuf_format(& mbuf, "%s",p);
            
            u = objv_url_alloc(zone, objv_mbuf_str(& mbuf));
            
            objv_mbuf_destroy(& mbuf);
            
            return u;

        }
        
    }
    
    return NULL;
}

objv_url_t * objv_url_allocWithQueryValues(objv_zone_t * zone,const char * url,objv_dictionary_t * queryValues){
    return objv_url_allocWithBaseUrlAndQueryValues(zone, url,NULL,queryValues);
}

objv_url_t * objv_url_allocWithBaseUrlAndQueryValues(objv_zone_t * zone,const char * url,objv_url_t * baseUrl,objv_dictionary_t * queryValues){
    
    if(queryValues && url){
        
        objv_mbuf_t mbuf;
        
        objv_url_t * u = NULL;
        objv_object_t * key;
        objv_object_t * value;
        objv_string_t * s;
        
        char * p = (char *) url;
        char * token = NULL;
        char * i = (char *) objv_string_indexOf(p, "#");
        int n;
        
        objv_mbuf_init(& mbuf, 128);
        
        if(i){
            objv_mbuf_append(& mbuf, p, i - p);
            token = i;
        }
        else{
            objv_mbuf_append(& mbuf, p, strlen(p));
        }
        
        i = (char *) objv_string_indexOf(p, "?");
        
        if(i == NULL){
            objv_mbuf_append(& mbuf, "?", 1);
        }
        
        for(n =0; n<queryValues->map->length; n++){
            
            key = objv_dictionary_keyAt(queryValues, n);
            value = objv_dictionary_valueAt(queryValues, n);
            s = objv_object_stringValue(value, NULL);
            
            if(n == 0 && i == NULL){
                objv_mbuf_format(& mbuf, "%@=",key);
            }
            else{
                objv_mbuf_format(& mbuf, "&%@=",key);
            }
            
            if(s){
                objv_url_encode_mbuf(zone, s->UTF8String, & mbuf);
            }
        }
        
        if(token){
            objv_mbuf_append(& mbuf, token, strlen(token));
        }
        
        u = objv_url_allocWithBaseUrl(zone, objv_mbuf_str(& mbuf),baseUrl);
        
        objv_mbuf_destroy(& mbuf);
        
        return u;
        
    }
    
    return objv_url_allocWithBaseUrl(zone, url,baseUrl);
}

objv_url_t * objv_url_new(objv_zone_t * zone,const char * url){
    return (objv_url_t *) objv_object_autorelease((objv_object_t *) objv_url_alloc(zone, url));
}

objv_url_t * objv_url_newWithBaseUrl(objv_zone_t * zone,const char * url,objv_url_t * baseUrl){
    return (objv_url_t *) objv_object_autorelease((objv_object_t *) objv_url_allocWithBaseUrl(zone, url,baseUrl));
}

objv_url_t * objv_url_newWithQueryValues(objv_zone_t * zone,const char * url,objv_dictionary_t * queryValues){
    return (objv_url_t *) objv_object_autorelease((objv_object_t *) objv_url_allocWithQueryValues(zone, url,queryValues));
}

objv_url_t * objv_url_newWithBaseUrlAndQueryValues(objv_zone_t * zone,const char * url,objv_url_t * baseUrl,objv_dictionary_t * queryValues){
    return (objv_url_t *) objv_object_autorelease((objv_object_t *) objv_url_allocWithBaseUrlAndQueryValues(zone, url,baseUrl,queryValues));
}

objv_url_t * objv_url_newWithFormatV(objv_zone_t * zone,const char * format,va_list va){
    return (objv_url_t *) objv_object_autorelease((objv_object_t *) objv_url_allocWithFormatV(zone, format,va));
}

objv_url_t * objv_url_newWithFormat(objv_zone_t * zone,const char * format,...){
    va_list ap;
    objv_url_t * u;
    
    va_start(ap ,format);
    
    u = objv_url_newWithFormatV(zone,format,ap);
    
    va_end(ap);
    
    return u;
}

objv_string_t * objv_url_encode(objv_zone_t * zone,const char * value){
    if(value){
        objv_mbuf_t mbuf;
        objv_string_t * v = NULL;
        
        objv_mbuf_init(& mbuf, 64);
        
        objv_url_encode_mbuf(zone,value,& mbuf);
        
        v = objv_string_new(zone, objv_mbuf_str(& mbuf));
        
        objv_mbuf_destroy(& mbuf);
        
        return v;
    }
    return NULL;
}

void objv_url_encode_mbuf(objv_zone_t * zone,const char * value,objv_mbuf_t * mbuf){
    
    if(value){
        
        unsigned char * p = (unsigned char *) value;
        
        while(p && *p != 0){
            
            if( (*p >= '0' && *p <='9') || (*p >= 'A' && *p <='Z') || (*p >= 'a' && *p <='z') || *p == '.' || *p == '-' || *p == '_'){
                objv_mbuf_append( mbuf, p, 1);
            }
            else if(*p == ' '){
                objv_mbuf_append( mbuf, "+", 1);
            }
            else {
                objv_mbuf_format( mbuf, "%%%02x",* p);
            }
            p ++;
        }
    }
    
}

objv_string_t * objv_url_decode(objv_zone_t * zone,const char * value){
    
    if(value){
        objv_mbuf_t mbuf;
        objv_string_t * v = NULL;
        
        objv_mbuf_init(& mbuf, 64);
        
        objv_url_decode_mbuf(zone,value,& mbuf);
        
        v = objv_string_new(zone, objv_mbuf_str(& mbuf));
        
        objv_mbuf_destroy(& mbuf);
        
        return v;
    }
    return NULL;
}

void objv_url_decode_mbuf(objv_zone_t * zone,const char * value,objv_mbuf_t * mbuf){
    
    if(value){
        
        unsigned int uc;
        char sx[4];
        char * p = (char *) value;
        
        while(p && *p != 0){
            
            if( *p == '%'){
                if(p[1] != 0 && p[2] != 0){
                    sx[0] = p[1];
                    sx[1] = p[2];
                    sx[2] = 0;
                    uc = 0;
                    sscanf(sx, "%02x",& uc);
                    
                    objv_mbuf_append(mbuf, & uc, 1);
                }
                else{
                    break;
                }
            }
            else if(* p == '+'){
                objv_mbuf_append(mbuf, " ", 1);
            }
            else{
                objv_mbuf_append(mbuf, p, 1);
            }
            
            p ++;
        }
    }
}
