//
//  objv_array.c
//  objv
//
//  Created by zhang hailong on 14-1-30.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv.h"
#include "objv_array.h"
#include "objv_autorelease.h"
#include "objv_value.h"

OBJV_KEY_DEC(ArrayIterator)
OBJV_KEY_IMP(ArrayIterator)

typedef struct _objv_array_iterator_t {
    objv_iterator_t base;
    objv_array_t * array;
    unsigned int index;
} objv_array_iterator_t;


static objv_object_t * objv_array_iterator_methods_next(objv_class_t * clazz, objv_object_t * obj){
    objv_array_iterator_t * iterator = (objv_array_iterator_t *) obj;
    return objv_array_objectAt(iterator->array, iterator->index ++);
}

static objv_object_t * objv_array_iterator_method_init (objv_class_t * clazz, objv_object_t * object,va_list ap){
    

    if(clazz->superClass){
        object = objv_object_initv(clazz->superClass, object, ap);
    }
    
    if(object){
        
        objv_array_iterator_t * iterator = (objv_array_iterator_t *) object;
    
        iterator->array = (objv_array_t *) objv_object_retain((objv_object_t *) va_arg(ap, objv_object_t *));
        
    }
    
    return object;
}

static void objv_array_iterator_method_dealloc (objv_class_t * clazz, objv_object_t * object){
    
    objv_array_iterator_t * iterator = (objv_array_iterator_t *) object;
    
    objv_object_release( ( objv_object_t *) iterator->array );
    
    if(clazz->superClass){
        
        objv_object_dealloc(clazz->superClass, object);
        
    }
}


static objv_method_t objv_array_iterator_methods[] = {
    {OBJV_KEY(next),"@()",(objv_method_impl_t)objv_array_iterator_methods_next}
    ,{OBJV_KEY(init),"@(*)",(objv_method_impl_t)objv_array_iterator_method_init}
    ,{OBJV_KEY(dealloc),"v()",(objv_method_impl_t)objv_array_iterator_method_dealloc}
};


static objv_class_t objv_array_iterator_class = {OBJV_KEY(ArrayIterator),& objv_iterator_class
    ,objv_array_iterator_methods,sizeof(objv_array_iterator_methods) / sizeof(objv_method_t)
    ,NULL,0
    ,sizeof(objv_array_iterator_t)
    ,NULL,0};


OBJV_KEY_IMP(Array)
OBJV_KEY_IMP(length)

static void objv_array_methods_dealloc(objv_class_t * clazz, objv_object_t * obj){
    
    objv_array_t * array = (objv_array_t *) obj;
    
    objv_object_t ** p = array->objects;
    unsigned int c = array->length;
    
    while (c > 0 && p) {
        
        objv_object_release( * p);
        
        c --;
        p ++;
    }
    
    if(array->objects){
        objv_zone_free(obj->zone, array->objects);
    }
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass,obj);
    }
}

static objv_object_t * objv_array_methods_init(objv_class_t * clazz, objv_object_t * object,va_list ap){
    
    if(clazz->superClass){
        object = objv_object_initv(clazz->superClass, object,ap);
    }
    
    if(object){
        
        objv_array_t * array = (objv_array_t *) object;
        unsigned int capacity = va_arg(ap, unsigned int);
        
        if(capacity == 0){
            capacity = 20;
        }
        
        array->size = capacity;
        array->objects = objv_zone_malloc(object->zone, capacity * sizeof(objv_object_t *));
        
    }
    
    return object;
}

static unsigned int objv_array_methods_length(objv_class_t * clazz, objv_object_t * obj){
    
    objv_array_t * array = (objv_array_t *) obj;
    
    return array->length;
}

static objv_iterator_t * objv_array_method_iterator (objv_class_t * clazz,objv_object_t * obj){
    objv_array_t * array = (objv_array_t *) obj;
    return (objv_iterator_t *) objv_object_autorelease((objv_object_t *) objv_object_alloc(obj->zone, &objv_array_iterator_class,array) );
}

static objv_object_t * objv_array_methods_objectForKey(objv_class_t * clazz,objv_object_t * object,objv_object_t * key){
    int index = objv_object_intValue(key, -1);
    objv_array_t * array = (objv_array_t *) object;
    return objv_array_objectAt(array, index);
}

static void objv_array_methods_setObjectForKey(objv_class_t * clazz,objv_object_t * object,objv_object_t * key,objv_object_t * value){
    int index = objv_object_intValue(key, -1);
    objv_array_t * array = (objv_array_t *) object;
    objv_array_replaceAt(array, value, index);
}

OBJV_CLASS_METHOD_IMP_BEGIN(Array)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", objv_array_methods_dealloc)

OBJV_CLASS_METHOD_IMP(init,"@(*)",objv_array_methods_init)

OBJV_CLASS_METHOD_IMP(length,"I()",objv_array_methods_length)

OBJV_CLASS_METHOD_IMP(iterator,"I()",objv_array_method_iterator)

OBJV_CLASS_METHOD_IMP(objectForKey,"@(@)",objv_array_methods_objectForKey)

OBJV_CLASS_METHOD_IMP(setObjectForKey,"v(@,@)",objv_array_methods_setObjectForKey)

OBJV_CLASS_METHOD_IMP_END(Array)

OBJV_CLASS_PROPERTY_IMP_BEGIN(Array)

OBJV_CLASS_PROPERTY_IMP(length, uint, OBJV_CLASS_METHOD(Array, 2), NULL, objv_false)

OBJV_CLASS_PROPERTY_IMP_END(Array)

OBJV_CLASS_IMP_P_M(Array, OBJV_CLASS(Object), objv_array_t)


objv_array_t * objv_array_alloc(objv_zone_t * zone,unsigned int capacity){
    return (objv_array_t *) objv_object_alloc(zone, OBJV_CLASS(Array),capacity);
}

objv_array_t * objv_array_new(objv_zone_t * zone,unsigned int capacity){
    return (objv_array_t *) objv_object_autorelease((objv_object_t *) objv_array_alloc(zone,capacity));
}

objv_array_t * objv_array_alloc_copy(objv_zone_t * zone,objv_array_t * array){
    
    objv_array_t * arr = objv_array_alloc(zone,array->size);
    
    arr->length = array->length;
    
    memcpy(arr->objects, array->objects, arr->size * sizeof(objv_object_t *));
    
    return arr;
}

void objv_array_add(objv_array_t * array,objv_object_t * object){
    
    if(array && object){
        
        if(array->length + 1 > array->size){
            array->size += 20;
            array->objects = objv_zone_realloc(array->base.zone,array->objects, array->size * sizeof(objv_object_t *));
        }
        
        array->objects[array->length ++] = objv_object_retain(object);
        
    }
    
}

void objv_array_remove(objv_array_t * array,objv_object_t * object){
    
    if(array && object){
        
        int i = 0;
        
        while (i < array->length) {
            
            if( objv_object_equal(object->isa, object , array->objects[i])){
                break;
            }
            
            i ++;
        }
        
        if(i < array->length){
            
            while (i + 1 < array->length) {
                array->objects[i] = array->objects[i +1];
                i ++;
            }
            
            array->length --;
            
            objv_object_release(object);
        }
        
    }
    
}

void objv_array_removeAt(objv_array_t * array,int index){
    
    if(array && index >=0 && index < array->length){
        
        objv_object_t * v = array->objects[index];
        
        while(index + 1 <array->length){
            array->objects[index] = array->objects[index + 1];
            index ++;
        }
        
        array->length --;
        
        objv_object_release(v);
    }
    
}

void objv_array_insertAt(objv_array_t * array,objv_object_t * object,int index){
    
    if(array && object && index >=0 && index <= array->length){
        
        if(array->length + 1 > array->size){
            array->size += 20;
            array->objects = objv_zone_realloc(array->base.zone,array->objects, array->size * sizeof(objv_object_t *));
        }
        
        int i = array->length ;
        
        while (i > index) {
            array->objects[i] = array->objects[i -1];
            i --;
        }
        
        array->objects[index] = objv_object_retain(object);
        
        array->length ++;
        
    }
    
}

void objv_array_replaceAt(objv_array_t * array,objv_object_t * object,int index){
    
    if(array && object && index >=0 && index < array->length){
        
        objv_object_retain(object);
        objv_object_release(array->objects[index]);
        
        array->objects[index] = object;
    
    }
}

void objv_array_clear(objv_array_t * array){
    
    if(array){
        
        objv_object_t ** p = array->objects;
        unsigned int c = array->length;
        
        while (c > 0 && p) {
            
            objv_object_release( * p);
            
            c --;
            p ++;
        }
        
        array->length = 0;
    }
}

objv_object_t * objv_array_objectAt(objv_array_t * array,int index){
    
    if(array && index >=0 && index < array->length){
        return array->objects[index];
    }
    
    return NULL;
}

objv_object_t * objv_array_last(objv_array_t * array){
    return objv_array_objectAt(array, array->length - 1);
}

void objv_array_removeLast(objv_array_t * array){
    objv_array_removeAt(array, array->length - 1);
}

objv_object_t * objv_array_first(objv_array_t * array){
    return objv_array_objectAt(array, 0);
}

void objv_array_removeFirst(objv_array_t * array){
    objv_array_removeAt(array, 0);
}

