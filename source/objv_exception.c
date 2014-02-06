//
//  objv_exception.c
//  objv
//
//  Created by zhang hailong on 14-2-6.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv_exception.h"


OBJV_KEY_IMP(Exception)

static void objv_exception_methods_dealloc(objv_class_t * clazz, objv_object_t * obj){
    
    objv_exception_t * ex = (objv_exception_t *) obj;
    
    objv_object_release((objv_object_t *) ex->message);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass,obj);
    }
}

static objv_object_t * objv_exception_methods_alloc(objv_class_t * clazz, objv_object_t * obj,int code,objv_string_t * message){
    
    objv_exception_t * ex = (objv_exception_t *) obj;
    
    ex->code = code;
    ex->message = (objv_string_t *) objv_object_retain((objv_object_t *) message);
 
    return obj;
}


static objv_method_t objv_exception_methods[] = {
    {OBJV_KEY(dealloc),"v()",(objv_method_impl_t) objv_exception_methods_dealloc},
    {OBJV_KEY(alloc),"@(i,@)",(objv_method_impl_t) objv_exception_methods_alloc}
};

objv_class_t objv_exception_class = {OBJV_KEY(Exception),& objv_object_class
    ,objv_exception_methods,sizeof(objv_exception_methods) / sizeof(objv_method_t)
    ,NULL,0
    ,sizeof(objv_exception_t)
    ,NULL,0,0};


objv_exception_t * objv_exception_alloc(objv_zone_t * zone,int code,const char * format,...){
    
    va_list ap;
    objv_exception_t * ex;
    
    va_start(ap, format);
    
    ex = objv_exception_allocv(zone, code, format, ap);
    
    va_end(ap);
    
    return ex;
}

objv_exception_t * objv_exception_allocv(objv_zone_t * zone,int code,const char * format,va_list ap){
    objv_exception_t * ex;
    objv_string_t * s = objv_string_alloc_formatv(zone, format, ap);
    ex = (objv_exception_t *) objv_object_alloc(zone, & objv_exception_class,code,s);
    objv_object_release((objv_object_t *) s);
    return ex;
}

objv_exception_t * objv_exception_new(objv_zone_t * zone,int code,const char * format,...){
    va_list ap;
    objv_exception_t * ex;
    
    va_start(ap, format);
    
    ex = objv_exception_newv(zone, code, format, ap);
    
    va_end(ap);
    
    return ex;
}

objv_exception_t * objv_exception_newv(objv_zone_t * zone,int code,const char * format,va_list ap){
    return (objv_exception_t *) objv_object_retain((objv_object_t *) objv_exception_allocv(zone,code,format,ap));
}
