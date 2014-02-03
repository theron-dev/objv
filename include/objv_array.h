//
//  objv_array.h
//  objv
//
//  Created by zhang hailong on 14-1-30.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_array_h
#define objv_objv_array_h



#ifdef __cplusplus
extern "C" {
#endif
    
    
    OBJV_KEY_DEC(Array)
    
    typedef struct _objv_array_t {
        objv_object_t base;
        objv_object_t ** READONLY objects;
        unsigned int READONLY size;
        unsigned int READONLY length;
    } objv_array_t;
    
    
    extern objv_class_t objv_array_class;
    
    objv_array_t * objv_array_alloc(objv_zone_t * zone,unsigned int capacity);
    
    objv_array_t * objv_array_new(objv_zone_t * zone,unsigned int capacity);
    
    objv_array_t * objv_array_alloc_copy(objv_zone_t * zone,objv_array_t * array);
    
    void objv_array_add(objv_array_t * array,objv_object_t * object);
    
    void objv_array_remove(objv_array_t * array,objv_object_t * object);
    
    void objv_array_removeAt(objv_array_t * array,int index);
    
    void objv_array_insertAt(objv_array_t * array,objv_object_t * object,int index);
    
    void objv_array_clear(objv_array_t * array);
    
    objv_object_t * objv_array_objectAt(objv_array_t * array,int index);
    
    objv_object_t * objv_array_last(objv_array_t * array);
    
    void objv_array_removeLast(objv_array_t * array);
    
    objv_object_t * objv_array_first(objv_array_t * array);
    
    void objv_array_removeFirst(objv_array_t * array);
    
    typedef int (* objv_array_sort_compare_t)(objv_object_t * value1,objv_object_t * value2,void * context);
    
    void objv_array_sort(objv_array_t * array,objv_array_sort_compare_t compare,void * context);
    
    void objv_array_insertBySort(objv_array_t * array,objv_array_sort_compare_t compare,void * context);
    
    int objv_array_sort_compare_hashCode(objv_object_t * value1,objv_object_t * value2,void * context);
    
    
#ifdef __cplusplus
}
#endif


#endif
