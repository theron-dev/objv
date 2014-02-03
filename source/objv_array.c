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

OBJV_KEY_IMP(Array)

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


static objv_method_t objv_array_methods[] = {
    {OBJV_KEY(dealloc),"v()",(objv_method_impl_t)objv_array_methods_dealloc}
};

objv_class_t objv_array_class = {OBJV_KEY(Array),& objv_object_class
    ,objv_array_methods,sizeof(objv_array_methods) / sizeof(objv_method_t)
    ,NULL,0
    ,sizeof(objv_array_t)
    ,NULL,0,0};


objv_array_t * objv_array_alloc(objv_zone_t * zone,unsigned int capacity){
    
    if(capacity == 0){
        capacity = 20;
    }
    
    objv_array_t * array = (objv_array_t *) objv_object_alloc(zone, &objv_array_class);
    
    array->size = capacity;
    array->objects = objv_zone_malloc(zone, capacity * sizeof(objv_object_t *));
    
    return array;
}

objv_array_t * objv_array_new(objv_zone_t * zone,unsigned int capacity){
    return (objv_array_t *) objv_object_autorelease((objv_object_t *) objv_array_alloc(zone,capacity));
}

objv_array_t * objv_array_alloc_copy(objv_zone_t * zone,objv_array_t * array){
    
    objv_array_t * arr = (objv_array_t *) objv_object_alloc(zone, &objv_array_class);
    
    arr->size = array->size;
    arr->length = array->length;
    arr->objects = objv_zone_malloc(zone, arr->size * sizeof(objv_object_t *));
    
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

void objv_array_sort(objv_array_t * array,objv_array_sort_compare_t compare,void * context){
    
}

void objv_array_insertBySort(objv_array_t * array,objv_array_sort_compare_t compare,void * context){
    
}

int objv_array_sort_compare_hashCode(objv_object_t * value1,objv_object_t * value2,void * context){
    long hashCode1 = objv_object_hashCode(value1->isa, value1);
    long hashCode2 = objv_object_hashCode(value2->isa, value2);
    long rs = hashCode1 - hashCode2;
    if(rs > 0){
        return 1;
    }
    else if(rs < 0){
        return -1;
    }
    return 0;
}

