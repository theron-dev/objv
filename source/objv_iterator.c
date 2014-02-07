//
//  objv_iterator.c
//  objv
//
//  Created by zhang hailong on 14-2-6.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#include "objv_os.h"
#include "objv.h"
#include "objv_iterator.h"

OBJV_KEY_IMP(Iterator)
OBJV_KEY_IMP(next)
OBJV_KEY_IMP(iterator)

static objv_object_t * objv_iterator_method_next(objv_class_t * clazz, objv_object_t * obj){
  
    return NULL;
}

static objv_method_t objv_iterator_methods[] = {
    {OBJV_KEY(next),"@()",(objv_method_impl_t)objv_iterator_method_next}
};

objv_class_t objv_iterator_class = {OBJV_KEY(Iterator),& objv_object_class
    ,objv_iterator_methods,sizeof(objv_iterator_methods) / sizeof(objv_method_t)
    ,NULL,0
    ,sizeof(objv_iterator_t)
    ,NULL,0,0};


objv_iterator_t * objv_object_iterator(objv_class_t * clazz,objv_object_t * object){
    
    if(clazz && object){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(iterator))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_object_method_iterator_t) method->impl)(c,object);
        }
    }
    
    return NULL;
}

objv_object_t * objv_iterator_next(objv_class_t * clazz,objv_iterator_t * iterator){
    
    if(clazz && iterator){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(next))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_iterator_next_t) method->impl)(c,(objv_object_t *)iterator);
        }
    }
    
    return NULL;
}
