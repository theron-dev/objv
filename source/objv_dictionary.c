//
//  objv_dictionary.c
//  objv
//
//  Created by zhang hailong on 14-1-30.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//


#define READONLY

#include "objv_os.h"
#include "objv.h"
#include "objv_dictionary.h"
#include "objv_autorelease.h"
#include "objv_value.h"

OBJV_KEY_IMP(Dictionary)

typedef struct _objv_dictionary_item_t{
    objv_object_t * key;
    objv_object_t * value;
} objv_dictionary_item_t;


static void objv_dictionary_methods_dealloc(objv_class_t * clazz, objv_object_t * obj){
    
    objv_dictionary_t * dictionary = (objv_dictionary_t *) obj;
    
    objv_dictionary_item_t * item;
    
    if(dictionary->map){
        int c = dictionary->map->length;
        for(int i=0;i<c;i++){
            item = objv_hash_map_valueAt(dictionary->map,i);
            objv_object_release(item->key);
            objv_object_release(item->value);
            objv_zone_free(obj->zone,item);
        }
    }
    
    objv_hash_map_dealloc(dictionary->map);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass,obj);
    }
}


static long objv_dictionary_key_hash_code(void * key){
    objv_object_t * obj = (objv_object_t *) key;
    return objv_object_hashCode(obj->isa, obj);
}

static int objv_dictionary_key_compare(void * key1,void * key2){
    objv_object_t * obj1 = (objv_object_t *) key1;
    objv_object_t * obj2 = (objv_object_t *) key2;
    
    if(objv_object_equal(obj1->isa,obj1,obj2)){
        return 0;
    }
    
    long r = (long) key1 - (long) key2;
    
    if(r >0 ){
        return 1;
    }
    else if(r < 0){
        return -1;
    }
    
    return 0;
}

static objv_object_t * objv_dictionary_methods_init(objv_class_t * clazz, objv_object_t * object,va_list ap){
    
    if(clazz->superClass){
        object = objv_object_initv(clazz->superClass, object,ap);
    }
    
    if(object){
    
        objv_dictionary_t * dictionary = (objv_dictionary_t *) object;
        unsigned int capacity = va_arg(ap, unsigned int);
        
        if(capacity == 0){
            capacity = 20;
        }
        
        dictionary->map = objv_hash_map_alloc(capacity,objv_dictionary_key_hash_code,objv_dictionary_key_compare);
        
    }
    
    return object;
}

static unsigned int objv_dictionary_methods_length(objv_class_t * clazz, objv_object_t * object){
    
    objv_dictionary_t * dictionary = (objv_dictionary_t *) object;
    
    return dictionary->map->length;
}

static objv_object_t * objv_dictionary_methods_objectForKey(objv_class_t * clazz,objv_object_t * object,objv_object_t * key){
    return objv_dictionary_value((objv_dictionary_t *) object, key);
}

static void objv_dictionary_methods_setObjectForKey(objv_class_t * clazz,objv_object_t * object,objv_object_t * key,objv_object_t * value){
    if(value){
        objv_dictionary_setValue((objv_dictionary_t *) object, key, value);
    }
    else{
        objv_dictionary_remove((objv_dictionary_t *) object, key);
    }
}

OBJV_CLASS_METHOD_IMP_BEGIN(Dictionary)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", objv_dictionary_methods_dealloc)

OBJV_CLASS_METHOD_IMP(init,"@(*)",objv_dictionary_methods_init)

OBJV_CLASS_METHOD_IMP(length,"I()",objv_dictionary_methods_length)

OBJV_CLASS_METHOD_IMP(objectForKey,"@(@)",objv_dictionary_methods_objectForKey)

OBJV_CLASS_METHOD_IMP(setObjectForKey,"v(@,@)",objv_dictionary_methods_setObjectForKey)

OBJV_CLASS_METHOD_IMP_END(Dictionary)

OBJV_CLASS_PROPERTY_IMP_BEGIN(Dictionary)

OBJV_CLASS_PROPERTY_IMP(length, uint, objv_dictionary_methods_length, NULL, objv_false)

OBJV_CLASS_PROPERTY_IMP_END(Dictionary)

OBJV_CLASS_IMP_P_M(Dictionary, OBJV_CLASS(Object), objv_dictionary_t)


objv_dictionary_t * objv_dictionary_alloc(objv_zone_t * zone,unsigned int capacity){
    return (objv_dictionary_t *) objv_object_alloc(zone, OBJV_CLASS(Dictionary),capacity,NULL,NULL);
}

objv_dictionary_t * objv_dictionary_new(objv_zone_t * zone,unsigned int capacity){
    return (objv_dictionary_t *) objv_object_autorelease((objv_object_t *) objv_dictionary_alloc(zone,capacity));
}

void objv_dictionary_setValue(objv_dictionary_t * dictionary,objv_object_t * key, objv_object_t * value){
    
    if(dictionary && key && value){
        
        objv_dictionary_item_t * item = objv_zone_malloc(dictionary->base.zone,sizeof(objv_dictionary_item_t));
        
        objv_dictionary_item_t * r;
        
        item->key = objv_object_retain(key);
        item->value = objv_object_retain(value);
        
        r = objv_hash_map_put(dictionary->map,key,item);
        
        if(r){
            objv_object_release(r->key);
            objv_object_release(r->value);
            objv_zone_free(dictionary->base.zone,r);
        }
    
    }
    
}

objv_object_t * objv_dictionary_value(objv_dictionary_t * dictionary,objv_object_t * key){
    
    if(dictionary && key ){
        
        objv_dictionary_item_t * r;
     
        r = objv_hash_map_get(dictionary->map,key);
        
        if(r){
            return r->value;
        }
        
    }

    return NULL;
}

void objv_dictionary_remove(objv_dictionary_t * dictionary,objv_object_t * key){
    
    if(dictionary && key ){
        
        objv_dictionary_item_t * r;
        
        r = objv_hash_map_remove(dictionary->map,key);
        
        if(r){
            objv_object_release(r->key);
            objv_object_release(r->value);
            objv_zone_free(dictionary->base.zone,r);
        }
        
    }
    
}

unsigned int objv_dictionary_length(objv_dictionary_t * dictionary){
    if(dictionary ){
        return dictionary->map->length;
    }
    return 0;
}

objv_object_t * objv_dictionary_keyAt(objv_dictionary_t * dictionary,int index){
    
    if(dictionary ){
    
        objv_dictionary_item_t * r;
        
        r = objv_hash_map_valueAt(dictionary->map,index);
        
        if(r){
            return r->key;
        }
        
    }
    return NULL;
}

objv_object_t * objv_dictionary_valueAt(objv_dictionary_t * dictionary,int index){
    
    if(dictionary ){
        
        objv_dictionary_item_t * r;
        
        r = objv_hash_map_valueAt(dictionary->map,index);
        
        if(r){
            return r->value;
        }
        
    }
    return NULL;
}

void objv_dictionary_clear(objv_dictionary_t * dictionary){
    
    if(dictionary  && dictionary->map){
      
        objv_dictionary_item_t * item;
        int c = dictionary->map->length;
        
        for(int i=0;i<c;i++){
            item = objv_hash_map_valueAt(dictionary->map,i);
            objv_object_release(item->key);
            objv_object_release(item->value);
            objv_zone_free(dictionary->base.zone,item);
        }
        
        objv_hash_map_clear(dictionary->map);
        
    }
    
}
