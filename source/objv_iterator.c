//
//  objv_iterator.c
//  objv
//
//  Created by zhang hailong on 14-2-6.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv.h"
#include "objv_iterator.h"
#include "objv_string.h"

OBJV_KEY_IMP(Iterator)
OBJV_KEY_IMP(next)
OBJV_KEY_IMP(iterator)
OBJV_KEY_IMP(keyIterator)


OBJV_CLASS_IMP(Iterator, OBJV_CLASS(Object), objv_iterator_t)

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

objv_iterator_t * objv_object_keyIterator(objv_class_t * clazz,objv_object_t * object){
    
    if(clazz && object){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(keyIterator))) == NULL){
            
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

OBJV_KEY_IMP(ObjectIterator)

static void ObjectIteratorMethodDealloc(objv_class_t * clazz,objv_object_t * object){
    
}

static objv_object_t * ObjectIteratorMethodInit(objv_class_t * clazz,objv_object_t * object,va_list ap){
    
    if(clazz->superClass){
        object = objv_object_initv(clazz->superClass, object, ap);
    }
    
    if(object){
        
        objv_object_iterator_t * iterator = (objv_object_iterator_t *) object;
        
        objv_class_t * c = object->isa;
        objv_property_t * prop;
        unsigned int propCount;
        
        iterator->keys = objv_hash_map_alloc(4, objv_hash_map_hash_code_key, objv_map_compare_key);
        
        while (c) {
            
            prop = c->propertys;
            propCount = c->propertyCount;
            
            while(propCount >0 && prop){
                
                if(objv_hash_map_get(iterator->keys, (void *) prop->name) == NULL){
                    objv_hash_map_put(iterator->keys, (void *) prop->name, prop);
                }
                
                propCount --;
                prop ++;
            }
            
            c = c->superClass;
        }
        
    }
    
    return object;
}

static objv_object_t * ObjectIteratorMethodNext(objv_class_t * clazz,objv_object_iterator_t * object){
    
    objv_key_t * key = objv_hash_map_keyAt(object->keys, object->index ++);
    
    if(key){
        return (objv_object_t *) objv_string_new_nocopy(object->base.base.zone, key);
    }
    
    return  NULL;
}

OBJV_CLASS_METHOD_IMP_BEGIN(ObjectIterator)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", ObjectIteratorMethodDealloc)

OBJV_CLASS_METHOD_IMP(init, "@(*)", ObjectIteratorMethodInit)

OBJV_CLASS_METHOD_IMP(next, "@()", ObjectIteratorMethodNext)

OBJV_CLASS_METHOD_IMP_END(ObjectIterator)

OBJV_CLASS_IMP_M(ObjectIterator, OBJV_CLASS(Iterator), objv_object_iterator_t)

objv_object_iterator_t * objv_object_iterator_alloc(objv_object_t * object){
    if(object){
        return (objv_object_iterator_t *) objv_object_alloc(object->zone, OBJV_CLASS(ObjectIterator),object,NULL);
    }
    return NULL;
}
