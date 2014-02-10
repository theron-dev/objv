//
//  objv_iterator.h
//  objv
//
//  Created by zhang hailong on 14-2-6.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_iterator_h
#define objv_objv_iterator_h


#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_hash_map.h"
    
    OBJV_KEY_DEC(Iterator)
    OBJV_KEY_DEC(next)
    OBJV_KEY_DEC(iterator)
    OBJV_KEY_DEC(keyIterator)
    
    typedef struct _objv_iterator_t {
        objv_object_t base;
    } objv_iterator_t;
    
    OBJV_CLASS_DEC(Iterator)
    
    typedef objv_object_t * (* objv_iterator_next_t ) (objv_class_t * clazz,objv_object_t * object);
    
    objv_object_t * objv_iterator_next(objv_class_t * clazz,objv_iterator_t * iterator);
    
    typedef objv_iterator_t * (* objv_object_method_iterator_t) (objv_class_t * clazz,objv_object_t * object);
    
    objv_iterator_t * objv_object_iterator(objv_class_t * clazz,objv_object_t * object);
    
    objv_iterator_t * objv_object_keyIterator(objv_class_t * clazz,objv_object_t * object);
    
    typedef struct _objv_object_iterator_t {
        objv_iterator_t base;
        objv_hash_map_t * READONLY keys;
        int READONLY index;
    } objv_object_iterator_t;
    
    OBJV_KEY_DEC(ObjectIterator)
    
    OBJV_CLASS_DEC(ObjectIterator)
    
    objv_object_iterator_t * objv_object_iterator_alloc(objv_object_t * object);
    
#ifdef __cplusplus
}
#endif


#endif
