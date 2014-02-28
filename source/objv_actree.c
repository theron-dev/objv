//
//  objv_actree.c
//  objv
//
//  Created by zhang hailong on 14-2-28.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv.h"
#include "objv_actree.h"
#include "objv_autorelease.h"

OBJV_KEY_IMP(ACTree)


static void objv_actree_methods_dealloc(objv_class_t * clazz, objv_object_t * obj){
    
    objv_actree_t * actree = (objv_actree_t *) obj;
    
    objv_object_release(actree->key);
    objv_object_release(actree->value);
    objv_object_release((objv_object_t *)actree->childs);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass,obj);
    }
}

static objv_object_t * objv_actree_methods_init(objv_class_t * clazz, objv_object_t * object,va_list ap){
    
    if(clazz->superClass){
        object = objv_object_initv(clazz->superClass, object, ap);
    }
    
    if(object){
        
        objv_actree_t * actree = (objv_actree_t *) object;
        objv_object_t * key = va_arg(ap, objv_object_t *);
        objv_object_t * value = va_arg(ap, objv_object_t *);
        
        actree->key = objv_object_retain(key);
        actree->value = objv_object_retain(value);
        
    }
    
    return object;
}


OBJV_CLASS_METHOD_IMP_BEGIN(ACTree)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", objv_actree_methods_dealloc)

OBJV_CLASS_METHOD_IMP(init, "@(*)", objv_actree_methods_init)

OBJV_CLASS_METHOD_IMP_END(ACTree)

OBJV_CLASS_IMP_M(ACTree, OBJV_CLASS(Object), objv_actree_t)


objv_actree_t * objv_actree_alloc(objv_zone_t * zone,objv_object_t * key, objv_object_t * value){
    return (objv_actree_t *) objv_object_alloc(zone, OBJV_CLASS(ACTree),key,value,NULL);
}

objv_actree_t * objv_actree_new(objv_zone_t * zone,objv_object_t * key, objv_object_t * value){
    return (objv_actree_t *) objv_object_autorelease(objv_object_alloc(zone, OBJV_CLASS(ACTree),key,value,NULL));
}

void objv_actree_setValueOfIndex(objv_actree_t * actree,objv_array_t * keys,objv_object_t * value, unsigned int index){
    if(actree && keys && keys->length > index){
        
        objv_object_t * key = objv_array_objectAt(keys, index);
        objv_actree_t * ac;
        objv_zone_t * zone = actree->base.zone;
        
        if(objv_object_equal(key->isa, key,actree->key)){
            
            if(index + 1 < keys->length){
                
                key = objv_array_objectAt(keys, index + 1);
                
                if(actree->childs == NULL){
                    actree->childs = objv_dictionary_alloc(actree->base.zone, 4);
                }
                
                ac = (objv_actree_t *) objv_dictionary_value(actree->childs, key);
                
                if(ac == NULL){
                    ac = objv_actree_alloc(zone, key, NULL);
                    objv_dictionary_setValue(actree->childs, key, (objv_object_t *) ac);
                    objv_actree_setValueOfIndex(ac,keys,value,index + 1);
                    objv_object_release((objv_object_t *) ac);
                    
                }
                else{
                    objv_actree_setValueOfIndex(ac,keys,value,index + 1);
                }
                
            }
            else{
                
                if(actree->value != value){
                    objv_object_retain(value);
                    objv_object_release(actree->value);
                    actree->value = value;
                }
                
            }
            
        }
    }
}

void objv_actree_setValue(objv_actree_t * actree,objv_array_t * keys,objv_object_t * value){
    objv_actree_setValueOfIndex(actree,keys,value,0);
}

objv_object_t * objv_actree_valueOfIndex(objv_actree_t * actree,objv_array_t * keys,unsigned int index){
    
    if(actree && keys && keys->length > index){
        
        objv_object_t * key = objv_array_objectAt(keys, index);
        objv_actree_t * ac;
        
        if(objv_object_equal(key->isa, key,actree->key)){
            if(index + 1 < keys->length){
                key = objv_array_objectAt(keys, index + 1);
                ac = (objv_actree_t *) objv_dictionary_value(actree->childs, key);
                return objv_actree_valueOfIndex(ac,keys,index + 1);
            }
            else{
                return actree->value;
            }
        }
        
    }
    return NULL;
}

objv_object_t * objv_actree_value(objv_actree_t * actree,objv_array_t * keys){
    return objv_actree_valueOfIndex(actree,keys,0);
}

void objv_actree_removeOfIndex(objv_actree_t * actree,objv_array_t * keys,unsigned int index){
    if(actree && keys && keys->length > index){
        objv_object_t * key = objv_array_objectAt(keys, index);
        objv_actree_t * ac;
        
        if(objv_object_equal(key->isa, key,actree->key)){
            if(index + 1 < keys->length){
                key = objv_array_objectAt(keys, index + 1);
                ac = (objv_actree_t *) objv_dictionary_value(actree->childs, key);
                if(ac){
                    objv_actree_removeOfIndex(ac,keys,index + 1);
                }
                if(ac->value == NULL && (ac->childs == NULL || ac->childs->map->length ==0)){
                    objv_dictionary_remove(actree->childs, key);
                }
            }
            else{
                objv_object_release(actree->value);
                actree->value = NULL;
            }
        }
    }
}

void objv_actree_remove(objv_actree_t * actree,objv_array_t * keys){
    objv_actree_removeOfIndex(actree,keys,0);
}
