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
    
    
    OBJV_KEY_DEC(Iterator)
    OBJV_KEY_DEC(next)
    OBJV_KEY_DEC(iterator)
    
    typedef struct _objv_iterator_t {
        objv_object_t base;
    } objv_iterator_t;
    
    
    extern objv_class_t objv_iterator_class;
    
    typedef objv_object_t * (* objv_iterator_next_t ) (objv_class_t * clazz,objv_object_t * object);
    
    objv_object_t * objv_iterator_next(objv_iterator_t * iterator);
    
    typedef objv_iterator_t * (* objv_object_method_iterator_t) (objv_class_t * clazz,objv_object_t * object);
    
    objv_iterator_t * objv_object_iterator(objv_class_t * clazz,objv_object_t * object);
    
#ifdef __cplusplus
}
#endif


#endif
