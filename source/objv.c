//
//  objv.c
//  objv
//
//  Created by zhang hailong on 14-1-29.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv.h"
#include "objv_private.h"
#include "objv_hash_map.h"
#include "objv_log.h"
#include "objv_iterator.h"
#include "objv_autorelease.h"
#include "objv_value.h"

OBJV_KEY_IMP(init)
OBJV_KEY_IMP(dealloc)
OBJV_KEY_IMP(retainCount)
OBJV_KEY_IMP(equal)
OBJV_KEY_IMP(hashCode)
OBJV_KEY_IMP(copy)
OBJV_KEY_IMP(Object)

static objv_hash_map_t * objv_keys = NULL;
static objv_mutex_t objv_keys_mutex;

objv_key_t * objv_key(const char * key){
    
    if(objv_keys == NULL){
        
        objv_mutex_init(& objv_keys_mutex);
        
        objv_mutex_lock(& objv_keys_mutex);
        
        if(objv_keys == NULL){
            objv_keys = objv_hash_map_alloc(32, objv_hash_map_hash_code_string, objv_map_compare_string);
        }
        
        objv_mutex_unlock(& objv_keys_mutex);
    }
    
    objv_mutex_lock(& objv_keys_mutex);
    
    objv_key_t * k = (objv_key_t *) objv_hash_map_get(objv_keys, (void *) key);
    
    if(k == NULL){
        
        k = (objv_key_t *) objv_zone_malloc(NULL, sizeof(objv_key_t) + strlen(key) + 1);
        
        k->name = (const char *) (k + 1);
        k->type = objv_key_type_dynamic;
        
        strcpy((char *) k->name, key);
        
        objv_hash_map_put(objv_keys, (void *) k->name, k);
    }
    
    objv_mutex_unlock(& objv_keys_mutex);
    
    return k;
}

objv_boolean_t objv_key_equal(objv_key_t * key1,objv_key_t * key2){
    
    if(key1 == key2){
        return objv_true;
    }
    
    if(key1->name == key2->name){
        return objv_true;
    }
    
    if(key1->type != key2->type){
        return strcmp(key1->name, key2->name) == 0 ? objv_true : objv_false;
    }
    
    return objv_false;
}

static void objv_object_methods_dealloc(objv_class_t * clazz, objv_object_t * obj){
    
    //objv_log("\n%s(0x%x) dealloc\n", obj->isa->name->name,(unsigned long) obj);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass,obj);
    }
}

static long objv_object_method_hashCode (objv_class_t * clazz, objv_object_t * object){
    return (long) object;
}

static objv_boolean_t objv_object_method_equal(objv_class_t * clazz, objv_object_t * object,objv_object_t * value){
    return object == value;
}


static int objv_object_methods_retainCount(objv_class_t * clazz,objv_object_t * obj){
    return obj->retainCount;
}

static objv_iterator_t * objv_object_methods_keyIterator(objv_class_t * clazz,objv_object_t * obj){
    return (objv_iterator_t * ) objv_object_autorelease( (objv_object_t *) objv_object_iterator_alloc(obj));
}

static objv_object_t * objv_object_methods_objectForKey(objv_class_t * clazz,objv_object_t * obj,objv_object_t * key){
    
    
    objv_class_t * c = obj->isa;
    objv_property_t * prop;
    unsigned int propCount;
    
    objv_string_t * skey = objv_object_stringValue(key, NULL);
    
    if(skey){

        while(c){
            
            prop = c->propertys;
            propCount = c->propertyCount;
            
            while(prop && propCount >0){
                
                if(prop->name->name == skey->UTF8String || strcmp(prop->name->name, skey->UTF8String)){
                    
                    return objv_property_objectValue(c, obj, prop, NULL);
                    
                }
                
                prop ++;
                propCount --;
            }
            
            c = c->superClass;
        }
    }
    
    return NULL;
}

static void objv_object_methods_setObjectForKey(objv_class_t * clazz,objv_object_t * obj
                                                ,objv_object_t * key,objv_object_t * value){
    objv_class_t * c = obj->isa;
    objv_property_t * prop;
    unsigned int propCount;
    
    objv_string_t * skey = objv_object_stringValue(key, NULL);
    
    if(skey){
        
        while(c){
            
            prop = c->propertys;
            propCount = c->propertyCount;
            
            while(prop && propCount >0){
                
                if(prop->name->name == skey->UTF8String || strcmp(prop->name->name, skey->UTF8String)){
                    
                    objv_property_setObjectValue(c, obj, prop, value);

                    return;
                }
                
                prop ++;
                propCount --;
            }
            
            c = c->superClass;
        }
    }
    
}

static objv_method_t objv_object_methods[] = {
    {OBJV_KEY(dealloc),"v()",(objv_method_impl_t)objv_object_methods_dealloc}
    ,{OBJV_KEY(hashCode),"l()",(objv_method_impl_t)objv_object_method_hashCode}
    ,{OBJV_KEY(equal),"b()",(objv_method_impl_t)objv_object_method_equal}
    ,{OBJV_KEY(retainCount),"i()",(objv_method_impl_t)objv_object_methods_retainCount}
    ,{OBJV_KEY(keyIterator),"@()",(objv_method_impl_t)objv_object_methods_keyIterator}
    ,{OBJV_KEY(objectForKey),"@(@)",(objv_method_impl_t)objv_object_methods_objectForKey}
    ,{OBJV_KEY(setObjectForKey),"v(@,@)",(objv_method_impl_t)objv_object_methods_setObjectForKey}
};


static void objv_object_class_initialize(objv_class_t * clazz){
    
}

objv_class_t objv_Object_class = {OBJV_KEY(Object),NULL
    ,objv_object_methods,sizeof(objv_object_methods) / sizeof(objv_method_t)
    ,NULL,0
    ,sizeof(objv_object_t)
    ,objv_object_class_initialize,0};

static objv_hash_map_t * _objv_classs = NULL;
static objv_mutex_t _objv_classs_mutex;


objv_class_t * objv_class(objv_key_t * className){
    
    if(className && _objv_classs){
        
        objv_mutex_lock(& _objv_classs_mutex);
        
        objv_class_t * clazz = (objv_class_t *) objv_hash_map_get(_objv_classs, className);
        
        objv_mutex_unlock(& _objv_classs_mutex);
     
        return clazz;
    }
    
    return NULL;
}

static void objv_class_reg(objv_class_t * clazz){
    
    if(clazz){
        
        if(_objv_classs == NULL){
            
            objv_mutex_init(& _objv_classs_mutex);
            
            _objv_classs = objv_hash_map_alloc(32, objv_hash_map_hash_code_key, objv_map_compare_key);
        }
        
        objv_mutex_lock(& _objv_classs_mutex);
        
        if(! objv_hash_map_get(_objv_classs, clazz->name)){
            objv_hash_map_put(_objv_classs, clazz->name, clazz);
        }
        
        objv_mutex_unlock(& _objv_classs_mutex);
        
    }
}

void objv_class_initialize(objv_class_t * clazz){
   
    if(clazz && ! clazz->initialized){
        
        if(clazz->superClass){
            objv_class_initialize(clazz->superClass);
        }
        
        if(clazz->name->type == objv_key_type_static){
            objv_class_reg(clazz);
        }
    
        if(clazz->initialize){
            (* clazz->initialize)(clazz);
        }
        
        clazz->initialized = objv_true;
    }
}

objv_method_t * objv_class_getMethod(objv_class_t * clazz,objv_key_t * name){
    
    if(clazz && name){
        
        objv_method_t * p = clazz->methods;
        int c = clazz->methodCount;
        
        while (c >0 ) {
            
            if(name == p->name
               || (p->name->type == name->type && p->name->name == name->name)
               || (p->name->type != name->type && p->name->name == name->name)){
                
                return p;
            }
            
            c --;
            p ++;
        }
    }
    
    return NULL;
}

objv_property_t * objv_class_getProperty(objv_class_t * clazz,objv_key_t * name){
    
    if(clazz && name){
        
        objv_property_t * p = clazz->propertys;
        int c = clazz->propertyCount;
        
        while (c >0 ) {
            
            if(objv_key_equal(name ,p->name ) ){
                
                return p;
            }
            
            c --;
            p ++;
        }
    }
    
    return NULL;
}

objv_boolean_t objv_class_isKindOfClass(objv_class_t * clazz,objv_class_t * ofClass){
    
    objv_class_t * c = clazz;
    
    while (c) {
        if(c == ofClass){
            return objv_true;
        }
        
        c = c->superClass;
    }
    
    return objv_false;
}

objv_object_t * objv_object_init(objv_class_t * clazz,objv_object_t * object,...){
    va_list ap;
    objv_object_t * o;
    
    va_start(ap, object);
    
    o = objv_object_initv(clazz,object,ap);
    
    va_end(ap);
    
    return o;
}

objv_object_t * objv_object_initv(objv_class_t * clazz,objv_object_t * object,va_list ap){
    
    if(clazz && object){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(init))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_object_method_init_t) method->impl)(c,object,ap);
        }
    }
    
    return object;
}

objv_object_t * objv_object_allocv(objv_zone_t * zone,objv_class_t * clazz,va_list ap){
    return objv_object_alloc_exertv(zone,clazz,0,ap);
}

objv_object_t * objv_object_alloc_exertv(objv_zone_t * zone,objv_class_t * clazz,size_t exert,va_list ap){
    
    
    if(zone == NULL){
        zone = objv_zone_default();
    }
    
    objv_class_initialize(clazz);
    
    if(clazz && clazz->size > 0){
        
        objv_object_t * obj = (objv_object_t *) objv_zone_malloc(zone, clazz->size + exert);
        
        objv_zone_memzero(zone, obj, clazz->size);
        
        obj->isa = clazz;
        obj->zone = zone;
        obj->retainCount = 1;
        objv_mutex_init(& obj->mutex);
        
        return objv_object_initv(clazz, obj, ap);
    }
    
    return NULL;

    
}

objv_object_t * objv_object_alloc_exert(objv_zone_t * zone,objv_class_t * clazz,size_t exert,...){
    va_list ap;
    objv_object_t * object = NULL;
    
    va_start(ap, exert);
    
    object = objv_object_alloc_exertv(zone,clazz,exert,ap);
    
    va_end(ap);
    
    return object;
}

objv_object_t * objv_object_alloc(objv_zone_t * zone,objv_class_t * clazz,...){
    
    va_list ap;
    objv_object_t * object = NULL;
    
    va_start(ap, clazz);
    
    object = objv_object_alloc_exertv(zone,clazz,0,ap);
    
    va_end(ap);
    
    return object;
}

#undef objv_object_retain

objv_object_t * objv_object_retain(objv_object_t * object){
    
    if(object){
        
        objv_zone_retain(object->zone, object);
        
        objv_mutex_lock(& object->mutex);
        
        assert(object->retainCount > 0);
        
        object->retainCount ++;
        
        objv_mutex_unlock(& object->mutex);
        

    }
    
    return object;
}

#undef objv_object_release

void objv_object_release(objv_object_t * object){
    
    if(object){
        
        objv_zone_release(object->zone, object);
        
        objv_mutex_lock(& object->mutex);
        
        assert(object->retainCount > 0);
        
        object->retainCount -- ;
        
        int dodealloc = object->retainCount == 0;
        
        objv_mutex_unlock(& object->mutex);

        if( dodealloc ){
            
            objv_object_dealloc(object->isa,object);
            
            {
                objv_object_weak_t * weak = object->weak, * tweak;
                
                while (weak) {
                    * weak->object = NULL;
                    tweak = weak;
                    weak = weak->next;
                    objv_zone_free(object->zone, tweak);
                }
            }
            
            objv_mutex_destroy(& object->mutex);
            
            objv_zone_free(object->zone, object);
            
        }
    }
    
}

objv_object_t * objv_object_weak(objv_object_t * object, objv_object_t ** toObject){
    if(object && toObject){
        
        objv_object_weak_t * weak = (objv_object_weak_t *) objv_zone_malloc(object->zone, sizeof(objv_object_weak_t));
        
        weak->object = toObject;
        weak->next = NULL;
        
        objv_mutex_lock(& object->mutex);
     
        if(object->weak){
            weak->next = object->weak;
            object->weak = weak;
        }
        else{
            object->weak = weak;
        }
        
        objv_mutex_unlock(& object->mutex);
    }
    return object;
}

void objv_object_unweak(objv_object_t * object, objv_object_t ** toObject){
    if(object && toObject){
        
        objv_object_weak_t * weak ,* pweak;
        
        objv_mutex_lock(& object->mutex);
        
        pweak = NULL;
        weak = object->weak;
        
        while (weak) {
            
            if(toObject == weak->object){
                
                if(pweak){
                    pweak->next = weak->next;
                    objv_zone_free(object->zone, weak);
                    weak = pweak->next;
                }
                else{
                    object->weak = weak->next;
                    objv_zone_free(object->zone, weak);
                    weak = object->weak;
                }
            }
            else{
                pweak = weak;
                weak = weak->next;
            }
        }
        
        objv_mutex_unlock(& object->mutex);
    }
}

objv_boolean_t objv_object_isKindOfClass(objv_object_t * object,objv_class_t * ofClass){
    if(object && ofClass){
        return objv_class_isKindOfClass(object->isa, ofClass);
    }
    return objv_false;
}


void objv_object_dealloc(objv_class_t * clazz, objv_object_t * object){
    
    if(clazz && object){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(dealloc))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            (* (objv_object_method_dealloc_t) method->impl)(c,object);
        }
    }
    
}

long objv_object_hashCode(objv_class_t * clazz,objv_object_t * object){
    
    if(clazz && object){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(hashCode))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_object_method_hashCode_t) method->impl)(c,object);
        }
        
    }
    
    return 0;
}


objv_boolean_t objv_object_equal(objv_class_t * clazz,objv_object_t * object
                                 ,objv_object_t * value){
    
    if(clazz && object){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(equal))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_object_method_equal_t) method->impl)(c,object,value);
        }
        
    }
    
    return object == value;
}

objv_object_t * objv_object_copy(objv_class_t * clazz,objv_object_t * object){
    
    if(clazz && object){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(copy))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_object_method_copy_t) method->impl)(c,object);
        }
        
    }
    
    return NULL;
}


