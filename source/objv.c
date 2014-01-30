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

OBJV_KEY_IMP(dealloc)
OBJV_KEY_IMP(retainCount)
OBJV_KEY_IMP(equal)
OBJV_KEY_IMP(hashCode)
OBJV_KEY_IMP(Object)


objv_boolean_t objv_key_equal(objv_key_t * key1,objv_key_t * key2){
    
    if(key1 == key2){
        return objv_true;
    }
    
    if(key1->name == key2->name){
        return objv_true;
    }
    
    if(key1->type == objv_key_type_dynamic || key2->type == objv_key_type_dynamic){
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


static int objv_object_methods_retainCount(objv_object_t * obj){
    return obj->retainCount;
}

static objv_method_t objv_object_methods[] = {
    {OBJV_KEY(dealloc),"v()",(objv_method_impl_t)objv_object_methods_dealloc}
    ,{OBJV_KEY(hashCode),"l()",(objv_method_impl_t)objv_object_method_hashCode}
    ,{OBJV_KEY(equal),"b()",(objv_method_impl_t)objv_object_method_equal}
    ,{OBJV_KEY(retainCount),"i()",(objv_method_impl_t)objv_object_methods_retainCount}
};


static void objv_object_class_initialize(objv_class_t * clazz){
    
}

objv_class_t objv_object_class = {OBJV_KEY(Object),NULL
    ,objv_object_methods,sizeof(objv_object_methods) / sizeof(objv_method_t)
    ,NULL,0
    ,sizeof(objv_object_t)
    ,objv_object_class_initialize,0,0};


void objv_class_initialize(objv_class_t * clazz){
   
    if(clazz && ! clazz->initialized){
        
        if(clazz->superClass){
            objv_class_initialize(clazz->superClass);
            clazz->offset = clazz->superClass->size;
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
    
    while (c && c != ofClass) {
        if(c == ofClass){
            return objv_true;
        }
        
        c = c->superClass;
    }
    
    return objv_false;
}



objv_object_t * objv_object_alloc(objv_zone_t * zone,objv_class_t * clazz){

    if(zone == NULL){
        zone = objv_zone_default();
    }
    
    objv_class_initialize(clazz);
    
    if(clazz && clazz->size > 0){
        
        objv_object_t * obj = (objv_object_t *) objv_zone_malloc(zone, clazz->size);
        
        objv_zone_memzero(zone, obj, clazz->size);
        
        obj->isa = clazz;
        obj->zone = zone;
        obj->retainCount = 1;
        
        return obj;

    }
    
    return NULL;
}

objv_object_t * objv_object_retain(objv_object_t * object){
    
    if(object){
        object->retainCount ++;
    }
    
    return object;
}


void objv_object_release(objv_object_t * object){
    
    if(object){
        
        assert(object->retainCount > 0);
        
        if( -- object->retainCount ==0){
            
            objv_object_dealloc(object->isa,object);
            
            objv_zone_free(object->zone, object);
        }
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


objv_boolean_t objv_object_equal(objv_class_t * clazz,objv_object_t * object,objv_object_t * value){
    
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


