//
//  objv_object.c
//  objv
//
//  Created by zhang hailong on 14-1-30.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv.h"
#include "objv_value.h"
#include "objv_autorelease.h"

OBJV_KEY_IMP(stringValue)
OBJV_KEY_IMP(intValue)
OBJV_KEY_IMP(uintValue)
OBJV_KEY_IMP(longValue)
OBJV_KEY_IMP(ulongValue)
OBJV_KEY_IMP(longLongValue)
OBJV_KEY_IMP(ulongLongValue)
OBJV_KEY_IMP(floatValue)
OBJV_KEY_IMP(doubleValue)
OBJV_KEY_IMP(booleanValue)

OBJV_KEY_IMP(Value)

static long objv_value_method_hashCode (objv_class_t * clazz, objv_object_t * object){
    
    objv_value_t * value = (objv_value_t *) object;
    
    if(value->type == & objv_type_int){
        
        return value->intValue;
    }
    
    if(value->type == & objv_type_uint){
        
        return value->uintValue;
    }
    
    if(value->type == & objv_type_long){
        
        return value->longValue;
    }
    
    if(value->type == & objv_type_ulong){
        
        return value->ulongValue;
    }
    
    if(value->type == & objv_type_longLong){
        
        return value->longLongValue;
    }
    
    if(value->type == & objv_type_ulongLong){
        
        return value->ulongLongValue;
    }
    
    if(value->type == & objv_type_float){
        
        return value->floatValue;
    }
    
    if(value->type == & objv_type_double){
        
        return value->doubleValue;
    }
    
    if(value->malloc){
        
        unsigned char * p = (unsigned char *) value->ptrValue;
        size_t size = value->type->size;
        long hashCode = 0;
        
        while(size >0){
            
            hashCode += * p;
            
            size --;
            p ++;
        }
        
        return hashCode;
    }
    
    return (long)value->ptrValue;
}

static objv_boolean_t objv_value_method_equal(objv_class_t * clazz, objv_object_t * object,objv_object_t * value){
    
    if(object != value && objv_object_isKindOfClass(value, & objv_value_class)){
        
        objv_value_t * v1 = (objv_value_t *) object;
        objv_value_t * v2 = (objv_value_t *) value;
        
        if(v2 == NULL){
            return objv_false;
        }
        
        if(v1->type == &objv_type_float || v1->type == &objv_type_double
           || v2->type == &objv_type_float || v2->type == &objv_type_double){
            
            return objv_value_doubleValue(v1, 0) == objv_value_doubleValue(v2, 0);
            
        }
        else if(!v1->malloc && !v2->malloc && v1->type != &objv_type_ptr && v2->type != &objv_type_ptr){
            return objv_value_longLongValue(v1, 0) == objv_value_longLongValue(v2, 0);
        }
        else if(v1->type == &objv_type_ptr && v2->type == &objv_type_ptr){
            return v1->ptrValue == v2->ptrValue;
        }
    }
    
    return object == value;
}


static void objv_value_methods_dealloc(objv_class_t * clazz, objv_object_t * obj){
    
    objv_value_t * value = (objv_value_t *) obj;
    
    if(value->malloc){
        objv_zone_free(obj->zone,value->ptrValue);
    }
    
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass,obj);
    }
}

static objv_string_t * objv_value_methods_stringValue(objv_class_t * clazz,objv_object_t * obj){
    
    char text[128] = "";
    
    objv_value_t * value = (objv_value_t *) obj;
    
    if(value->type == & objv_type_int){
        sprintf(text,"%d",value->intValue);
    }
    
    if(value->type == & objv_type_uint){
        sprintf(text,"%u",value->uintValue);
    }
    
    if(value->type == & objv_type_long){
        
        sprintf(text,"%ld",value->longValue);
    }
    
    if(value->type == & objv_type_ulong){
        
        sprintf(text,"%lu",value->ulongValue);
    }
    
    if(value->type == & objv_type_longLong){
        
        sprintf(text,"%lld",value->longLongValue);
    }
    
    if(value->type == & objv_type_ulongLong){
        
        sprintf(text,"%llu",value->ulongLongValue);
    }
    
    if(value->type == & objv_type_float){
        
        sprintf(text,"%f",value->floatValue);
    }
    
    if(value->type == & objv_type_double){
        
        sprintf(text,"%lf",value->doubleValue);
    }
    

    return objv_string_new(obj->zone, text);
}

static int objv_value_methods_intValue(objv_class_t * clazz,objv_object_t * obj){
    return objv_value_intValue((objv_value_t *) obj, 0);
}

static unsigned int objv_value_methods_uintValue(objv_class_t * clazz,objv_object_t * obj){
    return objv_value_uintValue((objv_value_t *) obj, 0);
}

static long objv_value_methods_longValue(objv_class_t * clazz,objv_object_t * obj){
    return objv_value_longValue((objv_value_t *) obj, 0);
}

static unsigned long objv_value_methods_ulongValue(objv_class_t * clazz,objv_object_t * obj){
    return objv_value_ulongValue((objv_value_t *) obj, 0);
}

static long long objv_value_methods_longLongValue(objv_class_t * clazz,objv_object_t * obj){
    return objv_value_longValue((objv_value_t *) obj, 0);
}

static unsigned long long objv_value_methods_ulongLongValue(objv_class_t * clazz,objv_object_t * obj){
    return objv_value_ulongLongValue((objv_value_t *) obj, 0);
}

static float objv_value_methods_floatValue(objv_class_t * clazz,objv_object_t * obj){
    return objv_value_floatValue((objv_value_t *) obj, 0);
}

static double objv_value_methods_doubleValue(objv_class_t * clazz,objv_object_t * obj){
    return objv_value_doubleValue((objv_value_t *) obj, 0);
}

static objv_boolean_t objv_value_methods_booleanValue(objv_class_t * clazz,objv_object_t * obj){
    return objv_value_booleanValue((objv_value_t *) obj, 0);
}

static objv_method_t objv_value_methods[] = {
    {OBJV_KEY(dealloc),"v()",(objv_method_impl_t)objv_value_methods_dealloc}
    ,{OBJV_KEY(hashCode),"l()",(objv_method_impl_t)objv_value_method_hashCode}
    ,{OBJV_KEY(equal),"l()",(objv_method_impl_t)objv_value_method_equal}
    ,{OBJV_KEY(stringValue),"@()",(objv_method_impl_t)objv_value_methods_stringValue}
    ,{OBJV_KEY(intValue),"i()",(objv_method_impl_t)objv_value_methods_intValue}
    ,{OBJV_KEY(uintValue),"I()",(objv_method_impl_t)objv_value_methods_uintValue}
    ,{OBJV_KEY(longValue),"l()",(objv_method_impl_t)objv_value_methods_longValue}
    ,{OBJV_KEY(ulongValue),"L()",(objv_method_impl_t)objv_value_methods_ulongValue}
    ,{OBJV_KEY(longLongValue),"q()",(objv_method_impl_t)objv_value_methods_longLongValue}
    ,{OBJV_KEY(ulongLongValue),"Q()",(objv_method_impl_t)objv_value_methods_ulongLongValue}
    ,{OBJV_KEY(floatValue),"f()",(objv_method_impl_t)objv_value_methods_floatValue}
    ,{OBJV_KEY(doubleValue),"d()",(objv_method_impl_t)objv_value_methods_doubleValue}
    ,{OBJV_KEY(booleanValue),"b()",(objv_method_impl_t)objv_value_methods_booleanValue}
};

static objv_property_t objv_value_propertys[] = {
    {OBJV_KEY(stringValue),&objv_type_object,&objv_value_methods[3],NULL}
    ,{OBJV_KEY(intValue),&objv_type_int,&objv_value_methods[4],NULL}
    ,{OBJV_KEY(uintValue),&objv_type_uint,&objv_value_methods[5],NULL}
    ,{OBJV_KEY(longValue),&objv_type_long,&objv_value_methods[6],NULL}
    ,{OBJV_KEY(ulongValue),&objv_type_ulong,&objv_value_methods[7],NULL}
    ,{OBJV_KEY(longLongValue),&objv_type_longLong,&objv_value_methods[8],NULL}
    ,{OBJV_KEY(ulongLongValue),&objv_type_ulongLong,&objv_value_methods[9],NULL}
    ,{OBJV_KEY(floatValue),&objv_type_float,&objv_value_methods[10],NULL}
    ,{OBJV_KEY(doubleValue),&objv_type_double,&objv_value_methods[11],NULL}
    ,{OBJV_KEY(booleanValue),&objv_type_boolean,&objv_value_methods[12],NULL}
};

objv_class_t objv_value_class = {OBJV_KEY(Value),& objv_object_class
    ,objv_value_methods,sizeof(objv_value_methods) / sizeof(objv_method_t)
    ,objv_value_propertys,sizeof(objv_value_propertys) / sizeof(objv_property_t)
    ,sizeof(objv_value_t)
    ,NULL,0,0};


objv_value_t * objv_value_alloc_nullValue(objv_zone_t * zone){
 
    objv_value_t * v = (objv_value_t *) objv_object_alloc(zone,&objv_value_class);
    
    v->type = & objv_type_void;
   
    return v;
}

objv_value_t * objv_value_alloc_intValue(objv_zone_t * zone,int value){
    
    objv_value_t * v = (objv_value_t *) objv_object_alloc(zone,&objv_value_class);
    
    v->type = & objv_type_int;
    v->intValue = value;
    
    return v;
    
}

objv_value_t * objv_value_alloc_uintValue(objv_zone_t * zone,unsigned int value){
    
    objv_value_t * v = (objv_value_t *) objv_object_alloc(zone,&objv_value_class);
    
    v->type = & objv_type_uint;
    v->uintValue = value;
    
    return v;
    
}

objv_value_t * objv_value_alloc_longValue(objv_zone_t * zone,long value){
    
    objv_value_t * v = (objv_value_t *) objv_object_alloc(zone,&objv_value_class);
    
    v->type = & objv_type_long;
    v->longValue = value;
    
    return v;
}

objv_value_t * objv_value_alloc_ulongValue(objv_zone_t * zone,unsigned long value){
    
    objv_value_t * v = (objv_value_t *) objv_object_alloc(zone,&objv_value_class);
    
    v->type = & objv_type_ulong;
    v->ulongValue = value;
    
    return v;
}

objv_value_t * objv_value_alloc_longLongValue(objv_zone_t * zone,long long value){
    
    objv_value_t * v = (objv_value_t *) objv_object_alloc(zone,&objv_value_class);
    
    v->type = & objv_type_longLong;
    v->longLongValue = value;
    
    return v;
    
}

objv_value_t * objv_value_alloc_ulongLongValue(objv_zone_t * zone,unsigned long long value){
    
    objv_value_t * v = (objv_value_t *) objv_object_alloc(zone,&objv_value_class);
    
    v->type = & objv_type_ulongLong;
    v->ulongLongValue = value;
    
    return v;
    
}

objv_value_t * objv_value_alloc_floatValue(objv_zone_t * zone,float value){
    
    objv_value_t * v = (objv_value_t *) objv_object_alloc(zone,&objv_value_class);
    
    v->type = & objv_type_float;
    v->floatValue = value;
    
    return v;
    
}

objv_value_t * objv_value_alloc_doubleValue(objv_zone_t * zone,double value){
    
    objv_value_t * v = (objv_value_t *) objv_object_alloc(zone,&objv_value_class);
    
    v->type = & objv_type_double;
    v->doubleValue = value;
    
    return v;
    
}

objv_value_t * objv_value_alloc_booleanValue(objv_zone_t * zone,objv_boolean_t value){
    
    objv_value_t * v = (objv_value_t *) objv_object_alloc(zone,&objv_value_class);
    
    v->type = & objv_type_boolean;
    v->booleanValue = value;
    
    return v;
    
}

objv_value_t * objv_value_alloc_ptrValue(objv_zone_t * zone,void * value){
    
    objv_value_t * v = (objv_value_t *) objv_object_alloc(zone,&objv_value_class);
    
    v->type = & objv_type_ptr;
    v->ptrValue = value;
    
    return v;
    
}

objv_value_t * objv_value_alloc_structValue(objv_zone_t * zone,void * value,objv_type_t * type){
    
    objv_value_t * v = (objv_value_t *) objv_object_alloc(zone,&objv_value_class);
    
    v->type = type;
    v->ptrValue = objv_zone_malloc(zone,type->size);
    v->malloc = objv_true;
    
    memcpy(v->ptrValue,value,type->size);
    
    return v;
    
}


objv_value_t * objv_value_new_nullValue(objv_zone_t * zone){
    return (objv_value_t *) objv_object_autorelease((objv_object_t *) objv_value_alloc_nullValue(zone));
}

objv_value_t * objv_value_new_intValue(objv_zone_t * zone,int value){
    return (objv_value_t *) objv_object_autorelease((objv_object_t *) objv_value_alloc_intValue(zone,value));
}

objv_value_t * objv_value_new_uintValue(objv_zone_t * zone,unsigned int value){
    return (objv_value_t *) objv_object_autorelease((objv_object_t *) objv_value_alloc_uintValue(zone,value));
}

objv_value_t * objv_value_new_longValue(objv_zone_t * zone,long value){
    return (objv_value_t *) objv_object_autorelease((objv_object_t *) objv_value_alloc_longValue(zone,value));
}

objv_value_t * objv_value_new_ulongValue(objv_zone_t * zone,unsigned long value){
    return (objv_value_t *) objv_object_autorelease((objv_object_t *) objv_value_alloc_ulongValue(zone,value));
}

objv_value_t * objv_value_new_longLongValue(objv_zone_t * zone,long long value){
    return (objv_value_t *) objv_object_autorelease((objv_object_t *) objv_value_alloc_longLongValue(zone,value));
}

objv_value_t * objv_value_new_ulongLongValue(objv_zone_t * zone,unsigned long long value){
    return (objv_value_t *) objv_object_autorelease((objv_object_t *) objv_value_alloc_ulongLongValue(zone,value));
}

objv_value_t * objv_value_new_floatValue(objv_zone_t * zone,float value){
    return (objv_value_t *) objv_object_autorelease((objv_object_t *) objv_value_alloc_floatValue(zone,value));
}

objv_value_t * objv_value_new_doubleValue(objv_zone_t * zone,double value){
    return (objv_value_t *) objv_object_autorelease((objv_object_t *) objv_value_alloc_doubleValue(zone,value));
}

objv_value_t * objv_value_new_booleanValue(objv_zone_t * zone,objv_boolean_t value){
    return (objv_value_t *) objv_object_autorelease((objv_object_t *) objv_value_alloc_booleanValue(zone,value));
}

objv_value_t * objv_value_new_ptrValue(objv_zone_t * zone,void * value){
    return (objv_value_t *) objv_object_autorelease((objv_object_t *) objv_value_alloc_ptrValue(zone,value));
}

objv_value_t * objv_value_new_structValue(objv_zone_t * zone,void * value,objv_type_t * type){
    return (objv_value_t *) objv_object_autorelease((objv_object_t *) objv_value_alloc_structValue(zone,value,type));
}



int objv_value_intValue(objv_value_t * value,int defaultValue){
    
    if(value->type == & objv_type_int){
        
        return value->intValue;
    }
    
    if(value->type == & objv_type_uint){
        
        return value->uintValue;
    }
    
    if(value->type == & objv_type_long){
        
        return (int)value->longValue;
    }
    
    if(value->type == & objv_type_ulong){
        
        return (int)value->ulongValue;
    }
    
    if(value->type == & objv_type_longLong){
        
        return (int)value->longLongValue;
    }
    
    if(value->type == & objv_type_ulongLong){
        
        return (int)value->ulongLongValue;
    }
    
    if(value->type == & objv_type_float){
        
        return value->floatValue;
    }
    
    if(value->type == & objv_type_double){
        
        return value->doubleValue;
    }
    
    return defaultValue;
}

unsigned int objv_value_uintValue(objv_value_t * value,unsigned int defaultValue){
    
    if(value->type == & objv_type_int){
        
        return value->intValue;
    }
    
    if(value->type == & objv_type_uint){
        
        return value->uintValue;
    }
    
    if(value->type == & objv_type_long){
        
        return (unsigned int)value->longValue;
    }
    
    if(value->type == & objv_type_ulong){
        
        return (unsigned int)value->ulongValue;
    }
    
    if(value->type == & objv_type_longLong){
        
        return (unsigned int)value->longLongValue;
    }
    
    if(value->type == & objv_type_ulongLong){
        
        return (unsigned int)value->ulongLongValue;
    }
    
    if(value->type == & objv_type_float){
        
        return value->floatValue;
    }
    
    if(value->type == & objv_type_double){
        
        return value->doubleValue;
    }
    
    return defaultValue;
    
}

long objv_value_longValue(objv_value_t * value,long defaultValue){
    
    if(value->type == & objv_type_int){
        
        return value->intValue;
    }
    
    if(value->type == & objv_type_uint){
        
        return value->uintValue;
    }
    
    if(value->type == & objv_type_long){
        
        return value->longValue;
    }
    
    if(value->type == & objv_type_ulong){
        
        return value->ulongValue;
    }
    
    if(value->type == & objv_type_longLong){
        
        return value->longLongValue;
    }
    
    if(value->type == & objv_type_ulongLong){
        
        return value->ulongLongValue;
    }
    
    if(value->type == & objv_type_float){
        
        return value->floatValue;
    }
    
    if(value->type == & objv_type_double){
        
        return value->doubleValue;
    }
    
    return defaultValue;
    
}

unsigned long objv_value_ulongValue(objv_value_t * value,unsigned long defaultValue){
    
    if(value->type == & objv_type_int){
        
        return value->intValue;
    }
    
    if(value->type == & objv_type_uint){
        
        return value->uintValue;
    }
    
    if(value->type == & objv_type_long){
        
        return value->longValue;
    }
    
    if(value->type == & objv_type_ulong){
        
        return value->ulongValue;
    }
    
    if(value->type == & objv_type_longLong){
        
        return value->longLongValue;
    }
    
    if(value->type == & objv_type_ulongLong){
        
        return value->ulongLongValue;
    }
    
    if(value->type == & objv_type_float){
        
        return value->floatValue;
    }
    
    if(value->type == & objv_type_double){
        
        return value->doubleValue;
    }
    
    return defaultValue;
    
}

long long objv_value_longLongValue(objv_value_t * value,long long defaultValue){
    
    if(value->type == & objv_type_int){
        
        return value->intValue;
    }
    
    if(value->type == & objv_type_uint){
        
        return value->uintValue;
    }
    
    if(value->type == & objv_type_long){
        
        return value->longValue;
    }
    
    if(value->type == & objv_type_ulong){
        
        return value->ulongValue;
    }
    
    if(value->type == & objv_type_longLong){
        
        return value->longLongValue;
    }
    
    if(value->type == & objv_type_ulongLong){
        
        return value->ulongLongValue;
    }
    
    if(value->type == & objv_type_float){
        
        return value->floatValue;
    }
    
    if(value->type == & objv_type_double){
        
        return value->doubleValue;
    }
    
    return defaultValue;
    
}

unsigned long long objv_value_ulongLongValue(objv_value_t * value,unsigned long long defaultValue){
    
    if(value->type == & objv_type_int){
        
        return value->intValue;
    }
    
    if(value->type == & objv_type_uint){
        
        return value->uintValue;
    }
    
    if(value->type == & objv_type_long){
        
        return value->longValue;
    }
    
    if(value->type == & objv_type_ulong){
        
        return value->ulongValue;
    }
    
    if(value->type == & objv_type_longLong){
        
        return value->longLongValue;
    }
    
    if(value->type == & objv_type_ulongLong){
        
        return value->ulongLongValue;
    }
    
    if(value->type == & objv_type_float){
        
        return value->floatValue;
    }
    
    if(value->type == & objv_type_double){
        
        return value->doubleValue;
    }
    
    return defaultValue;
    
}

float objv_value_floatValue(objv_value_t * value,float defaultValue){
    
    if(value->type == & objv_type_int){
        
        return value->intValue;
    }
    
    if(value->type == & objv_type_uint){
        
        return value->uintValue;
    }
    
    if(value->type == & objv_type_long){
        
        return value->longValue;
    }
    
    if(value->type == & objv_type_ulong){
        
        return value->ulongValue;
    }
    
    if(value->type == & objv_type_longLong){
        
        return value->longLongValue;
    }
    
    if(value->type == & objv_type_ulongLong){
        
        return value->ulongLongValue;
    }
    
    if(value->type == & objv_type_float){
        
        return value->floatValue;
    }
    
    if(value->type == & objv_type_double){
        
        return value->doubleValue;
    }
    
    return defaultValue;
    
}

double objv_value_doubleValue(objv_value_t * value,double defaultValue){
    
    if(value->type == & objv_type_int){
        
        return value->intValue;
    }
    
    if(value->type == & objv_type_uint){
        
        return value->uintValue;
    }
    
    if(value->type == & objv_type_long){
        
        return value->longValue;
    }
    
    if(value->type == & objv_type_ulong){
        
        return value->ulongValue;
    }
    
    if(value->type == & objv_type_longLong){
        
        return value->longLongValue;
    }
    
    if(value->type == & objv_type_ulongLong){
        
        return value->ulongLongValue;
    }
    
    if(value->type == & objv_type_float){
        
        return value->floatValue;
    }
    
    if(value->type == & objv_type_double){
        
        return value->doubleValue;
    }
    
    return defaultValue;
    
}

objv_boolean_t objv_value_booleanValue(objv_value_t * value,objv_boolean_t defaultValue){
    
    if(value->type == & objv_type_int){
        
        return value->intValue;
    }
    
    if(value->type == & objv_type_uint){
        
        return value->uintValue;
    }
    
    if(value->type == & objv_type_long){
        
        return (objv_boolean_t)value->longValue;
    }
    
    if(value->type == & objv_type_ulong){
        
        return (objv_boolean_t)value->ulongValue;
    }
    
    if(value->type == & objv_type_longLong){
        
        return (objv_boolean_t)value->longLongValue;
    }
    
    if(value->type == & objv_type_ulongLong){
        
        return (objv_boolean_t)value->ulongLongValue;
    }
    
    if(value->type == & objv_type_float){
        
        return value->floatValue;
    }
    
    if(value->type == & objv_type_double){
        
        return value->doubleValue;
    }
    
    return defaultValue;
    
}

void * objv_value_ptrValue(objv_value_t * value,void * defaultValue){
    
    if(value->type == & objv_type_int){
        
        return defaultValue;
    }
    
    if(value->type == & objv_type_uint){
        
        return defaultValue;
    }
    
    if(value->type == & objv_type_long){
        
        return defaultValue;
    }
    
    if(value->type == & objv_type_ulong){
        
        return defaultValue;
    }
    
    if(value->type == & objv_type_longLong){
        
        return defaultValue;
    }
    
    if(value->type == & objv_type_ulongLong){
        
        return defaultValue;
    }
    
    if(value->type == & objv_type_float){
        
        return defaultValue;
    }
    
    if(value->type == & objv_type_double){
        
        return defaultValue;
    }
    
    return value->ptrValue;
    
}


int objv_object_intValue(objv_object_t * object,int defaultValue){
    
    if(object){
        
        if(objv_object_isKindOfClass(object, & objv_value_class)){
            return objv_value_intValue((objv_value_t *) object, defaultValue);
        }
        
        objv_class_t * c = object->isa;
        
        objv_property_t * prop = NULL;
        
        while(c && (prop = objv_class_getProperty(c, OBJV_KEY(intValue))) == NULL){
            c = c->superClass;
        }
        
        if(prop && prop->getter){
            
            return (* (objv_object_property_intValue_t)prop->getter)(c,object);
            
        }
        
    }
    
    return defaultValue;
}

unsigned int objv_object_uintValue(objv_object_t * object,unsigned int defaultValue){
    
    if(object){
        
        if(objv_object_isKindOfClass(object, & objv_value_class)){
            return objv_value_uintValue((objv_value_t *) object, defaultValue);
        }
        
        objv_class_t * c = object->isa;
        
        objv_property_t * prop = NULL;
        
        while(c && (prop = objv_class_getProperty(c, OBJV_KEY(uintValue))) == NULL){
            c = c->superClass;
        }
        
        if(prop && prop->getter){
            
            return (* (objv_object_property_uintValue_t)prop->getter)(c,object);
            
        }
        
    }
    
    return defaultValue;
}

long objv_object_longValue(objv_object_t * object,long defaultValue){
    
    if(object){
        
        if(objv_object_isKindOfClass(object, & objv_value_class)){
            return objv_value_longValue((objv_value_t *) object, defaultValue);
        }
        
        objv_class_t * c = object->isa;
        
        objv_property_t * prop = NULL;
        
        while(c && (prop = objv_class_getProperty(c, OBJV_KEY(longValue))) == NULL){
            c = c->superClass;
        }
        
        if(prop && prop->getter){
            
            return (* (objv_object_property_longValue_t)prop->getter)(c,object);
            
        }
        
    }
    
    return defaultValue;
    
}

unsigned long objv_object_ulongValue(objv_object_t * object,unsigned long defaultValue){
    
    if(object){
        
        if(objv_object_isKindOfClass(object, & objv_value_class)){
            return objv_value_ulongValue((objv_value_t *) object, defaultValue);
        }
        
        objv_class_t * c = object->isa;
        
        objv_property_t * prop = NULL;
        
        while(c && (prop = objv_class_getProperty(c, OBJV_KEY(ulongValue))) == NULL){
            c = c->superClass;
        }
        
        if(prop && prop->getter){
            
            return (* (objv_object_property_ulongValue_t)prop->getter)(c,object);
            
        }
        
    }
    
    return defaultValue;
    
}

long long objv_object_longLongValue(objv_object_t * object,long long defaultValue){
    
    if(object){
        
        if(objv_object_isKindOfClass(object, & objv_value_class)){
            return objv_value_longLongValue((objv_value_t *) object, defaultValue);
        }
        
        objv_class_t * c = object->isa;
        
        objv_property_t * prop = NULL;
        
        while(c && (prop = objv_class_getProperty(c, OBJV_KEY(longLongValue))) == NULL){
            c = c->superClass;
        }
        
        if(prop && prop->getter){
            
            return (* (objv_object_property_longLongValue_t)prop->getter)(c,object);
            
        }
        
    }
    
    return defaultValue;
    
}

unsigned long long objv_object_ulongLongValue(objv_object_t * object,unsigned long long defaultValue){
    
    if(object){
        
        if(objv_object_isKindOfClass(object, & objv_value_class)){
            return objv_value_ulongLongValue((objv_value_t *) object, defaultValue);
        }
        
        objv_class_t * c = object->isa;
        
        objv_property_t * prop = NULL;
        
        while(c && (prop = objv_class_getProperty(c, OBJV_KEY(ulongLongValue))) == NULL){
            c = c->superClass;
        }
        
        if(prop && prop->getter){
            
            return (* (objv_object_property_ulongLongValue_t)prop->getter)(c,object);
            
        }
        
    }
    
    return defaultValue;
    
}

float objv_object_floatValue(objv_object_t * object,float defaultValue){
    
    if(object){
        
        if(objv_object_isKindOfClass(object, & objv_value_class)){
            return objv_value_floatValue((objv_value_t *) object, defaultValue);
        }
        
        objv_class_t * c = object->isa;
        
        objv_property_t * prop = NULL;
        
        while(c && (prop = objv_class_getProperty(c, OBJV_KEY(floatValue))) == NULL){
            c = c->superClass;
        }
        
        if(prop && prop->getter){
            
            return (* (objv_object_property_floatValue_t)prop->getter)(c,object);
            
        }
        
    }
    
    return defaultValue;
}

double objv_object_doubleValue(objv_object_t * object,double defaultValue){
    
    if(object){
        
        if(objv_object_isKindOfClass(object, & objv_value_class)){
            return objv_value_doubleValue((objv_value_t *) object, defaultValue);
        }
        
        objv_class_t * c = object->isa;
        
        objv_property_t * prop = NULL;
        
        while(c && (prop = objv_class_getProperty(c, OBJV_KEY(uintValue))) == NULL){
            c = c->superClass;
        }
        
        if(prop && prop->getter){
            
            return (* (objv_object_property_uintValue_t)prop->getter)(c,object);
            
        }
        
    }
    
    return defaultValue;
    
}

objv_boolean_t objv_object_booleanValue(objv_object_t * object,objv_boolean_t defaultValue){
    if(object){
        
        if(objv_object_isKindOfClass(object, & objv_value_class)){
            return objv_value_booleanValue((objv_value_t *) object, defaultValue);
        }
        
        objv_class_t * c = object->isa;
        
        objv_property_t * prop = NULL;
        
        while(c && (prop = objv_class_getProperty(c, OBJV_KEY(booleanValue))) == NULL){
            c = c->superClass;
        }
        
        if(prop && prop->getter){
            
            return (* (objv_object_property_booleanValue_t)prop->getter)(c,object);
            
        }
        
    }
    
    return defaultValue;
}

objv_string_t * objv_object_stringValue(objv_object_t * object,objv_string_t * defaultValue){
    
    if(object){
        
        if(objv_object_isKindOfClass(object, &objv_string_class)){
            return (objv_string_t *) object;
        }
        
        objv_class_t * c = object->isa;
        
        objv_property_t * prop = NULL;
        
        while(c && (prop = objv_class_getProperty(c, OBJV_KEY(stringValue))) == NULL){
            c = c->superClass;
        }
        
        if(prop && prop->getter){
            
            return (* (objv_object_property_stringValue_t)prop->getter)(c,object);
            
        }
        
    }
    
    return defaultValue;
}

int objv_property_intValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property, int defaultValue){
    if(clazz && object && property && property->getter){
        if(property->type == & objv_type_int){
            return (* (objv_object_property_intValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_uint){
            return (* (objv_object_property_uintValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_long){
            return (int) (* (objv_object_property_longValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_ulong){
            return (int) (* (objv_object_property_ulongValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_longLong){
            return (int) (* (objv_object_property_longLongValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_ulongLong){
            return (int) (* (objv_object_property_ulongLongValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_float){
            return (int) (* (objv_object_property_floatValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_double){
            return (int) (* (objv_object_property_doubleValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_object){
            return objv_object_intValue( (* (objv_object_property_objectValue_t) property->getter)(clazz,object),defaultValue);
        }
    }
    return defaultValue;
}

unsigned int objv_property_uintValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,unsigned int defaultValue){
    if(clazz && object && property && property->getter){
        if(property->type == & objv_type_int){
            return (* (objv_object_property_intValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_uint){
            return (* (objv_object_property_uintValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_long){
            return (unsigned int) (* (objv_object_property_longValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_ulong){
            return (unsigned int) (* (objv_object_property_ulongValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_longLong){
            return (unsigned int) (* (objv_object_property_longLongValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_ulongLong){
            return (unsigned int) (* (objv_object_property_ulongLongValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_float){
            return (unsigned int) (* (objv_object_property_floatValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_double){
            return (unsigned int) (* (objv_object_property_doubleValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_object){
            return objv_object_uintValue( (* (objv_object_property_objectValue_t) property->getter)(clazz,object),defaultValue);
        }
    }
    return defaultValue;
}

long objv_property_longValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,long defaultValue){
    if(clazz && object && property && property->getter){
        if(property->type == & objv_type_int){
            return (* (objv_object_property_intValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_uint){
            return (* (objv_object_property_uintValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_long){
            return (long) (* (objv_object_property_longValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_ulong){
            return (long) (* (objv_object_property_ulongValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_longLong){
            return (long) (* (objv_object_property_longLongValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_ulongLong){
            return (long) (* (objv_object_property_ulongLongValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_float){
            return (long) (* (objv_object_property_floatValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_double){
            return (long) (* (objv_object_property_doubleValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_object){
            return objv_object_longValue( (* (objv_object_property_objectValue_t) property->getter)(clazz,object),defaultValue);
        }
    }
    return defaultValue;
}

unsigned long objv_property_ulongValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,unsigned long defaultValue){
    if(clazz && object && property && property->getter){
        if(property->type == & objv_type_int){
            return (* (objv_object_property_intValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_uint){
            return (* (objv_object_property_uintValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_long){
            return (unsigned long) (* (objv_object_property_longValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_ulong){
            return (unsigned long) (* (objv_object_property_ulongValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_longLong){
            return (unsigned long) (* (objv_object_property_longLongValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_ulongLong){
            return (unsigned long) (* (objv_object_property_ulongLongValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_float){
            return (unsigned long) (* (objv_object_property_floatValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_double){
            return (unsigned long) (* (objv_object_property_doubleValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_object){
            return objv_object_ulongValue( (* (objv_object_property_objectValue_t) property->getter)(clazz,object),defaultValue);
        }
    }
    return defaultValue;
}

long long objv_property_longLongValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,long long defaultValue){
    if(clazz && object && property && property->getter){
        if(property->type == & objv_type_int){
            return (* (objv_object_property_intValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_uint){
            return (* (objv_object_property_uintValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_long){
            return (long long) (* (objv_object_property_longValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_ulong){
            return (long long) (* (objv_object_property_ulongValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_longLong){
            return (long long) (* (objv_object_property_longLongValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_ulongLong){
            return (long long) (* (objv_object_property_ulongLongValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_float){
            return (long long) (* (objv_object_property_floatValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_double){
            return (long long) (* (objv_object_property_doubleValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_object){
            return objv_object_longLongValue( (* (objv_object_property_objectValue_t) property->getter)(clazz,object),defaultValue);
        }
    }
    return defaultValue;
}

unsigned long long objv_property_ulongLongValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,unsigned long long defaultValue){
    if(clazz && object && property && property->getter){
        if(property->type == & objv_type_int){
            return (* (objv_object_property_intValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_uint){
            return (* (objv_object_property_uintValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_long){
            return (unsigned long long) (* (objv_object_property_longValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_ulong){
            return (unsigned long long) (* (objv_object_property_ulongValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_longLong){
            return (unsigned long long) (* (objv_object_property_longLongValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_ulongLong){
            return (unsigned long long) (* (objv_object_property_ulongLongValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_float){
            return (unsigned long long) (* (objv_object_property_floatValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_double){
            return (unsigned long long) (* (objv_object_property_doubleValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_object){
            return objv_object_ulongLongValue( (* (objv_object_property_objectValue_t) property->getter)(clazz,object),defaultValue);
        }
    }
    return defaultValue;
}

float objv_property_floatValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,float defaultValue){
    if(clazz && object && property && property->getter){
        if(property->type == & objv_type_int){
            return (* (objv_object_property_intValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_uint){
            return (* (objv_object_property_uintValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_long){
            return (float) (* (objv_object_property_longValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_ulong){
            return (float) (* (objv_object_property_ulongValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_longLong){
            return (float) (* (objv_object_property_longLongValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_ulongLong){
            return (float) (* (objv_object_property_ulongLongValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_float){
            return (float) (* (objv_object_property_floatValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_double){
            return (float) (* (objv_object_property_doubleValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_object){
            return objv_object_floatValue( (* (objv_object_property_objectValue_t) property->getter)(clazz,object),defaultValue);
        }
    }
    return defaultValue;
}

double objv_property_doubleValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,double defaultValue){
    if(clazz && object && property && property->getter){
        if(property->type == & objv_type_int){
            return (* (objv_object_property_intValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_uint){
            return (* (objv_object_property_uintValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_long){
            return (double) (* (objv_object_property_longValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_ulong){
            return (double) (* (objv_object_property_ulongValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_longLong){
            return (double) (* (objv_object_property_longLongValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_ulongLong){
            return (double) (* (objv_object_property_ulongLongValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_float){
            return (double) (* (objv_object_property_floatValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_double){
            return (double) (* (objv_object_property_doubleValue_t) property->getter)(clazz,object);
        }
        else if(property->type == & objv_type_object){
            return objv_object_doubleValue( (* (objv_object_property_objectValue_t) property->getter)(clazz,object),defaultValue);
        }
    }
    return defaultValue;
}

objv_boolean_t objv_property_booleanValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,objv_boolean_t defaultValue){
    if(clazz && object && property && property->getter){
        if(property->type == & objv_type_int){
            return (* (objv_object_property_intValue_t) property->getter)(clazz,object) !=0 ;
        }
        else if(property->type == & objv_type_uint){
            return (* (objv_object_property_uintValue_t) property->getter)(clazz,object) != 0;
        }
        else if(property->type == & objv_type_long){
            return (* (objv_object_property_longValue_t) property->getter)(clazz,object) != 0;
        }
        else if(property->type == & objv_type_ulong){
            return (* (objv_object_property_ulongValue_t) property->getter)(clazz,object) != 0;
        }
        else if(property->type == & objv_type_longLong){
            return (* (objv_object_property_longLongValue_t) property->getter)(clazz,object) != 0;
        }
        else if(property->type == & objv_type_ulongLong){
            return (* (objv_object_property_ulongLongValue_t) property->getter)(clazz,object) != 0;
        }
        else if(property->type == & objv_type_float){
            return (* (objv_object_property_floatValue_t) property->getter)(clazz,object) != 0.0f;
        }
        else if(property->type == & objv_type_double){
            return (* (objv_object_property_doubleValue_t) property->getter)(clazz,object) != 0.0;
        }
        else if(property->type == & objv_type_object){
            return objv_object_booleanValue( (* (objv_object_property_objectValue_t) property->getter)(clazz,object),defaultValue);
        }
    }
    return defaultValue;
}

objv_string_t * objv_property_stringValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,objv_string_t * defaultValue){
    if(clazz && object && property && property->getter){
        if(property->type == & objv_type_int){
            return objv_string_new_format(object->zone,"%d", (* (objv_object_property_intValue_t) property->getter)(clazz,object));
        }
        else if(property->type == & objv_type_uint){
            return objv_string_new_format(object->zone,"%u", (* (objv_object_property_uintValue_t) property->getter)(clazz,object));
        }
        else if(property->type == & objv_type_long){
            return objv_string_new_format(object->zone,"%ld", (* (objv_object_property_longValue_t) property->getter)(clazz,object));
        }
        else if(property->type == & objv_type_ulong){
            return objv_string_new_format(object->zone,"%lu", (* (objv_object_property_ulongValue_t) property->getter)(clazz,object));
        }
        else if(property->type == & objv_type_longLong){
            return objv_string_new_format(object->zone,"%lld",(* (objv_object_property_longLongValue_t) property->getter)(clazz,object));
        }
        else if(property->type == & objv_type_ulongLong){
            return objv_string_new_format(object->zone,"%llu",(* (objv_object_property_ulongLongValue_t) property->getter)(clazz,object));
        }
        else if(property->type == & objv_type_float){
            return objv_string_new_format(object->zone,"%f",(* (objv_object_property_floatValue_t) property->getter)(clazz,object));
        }
        else if(property->type == & objv_type_double){
            return objv_string_new_format(object->zone,"%lf",(* (objv_object_property_doubleValue_t) property->getter)(clazz,object));
        }
        else if(property->type == & objv_type_object){
            return objv_object_stringValue( (* (objv_object_property_objectValue_t) property->getter)(clazz,object),defaultValue);
        }
    }
    return defaultValue;
}

objv_object_t * objv_property_objectValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,objv_object_t * defaultValue){
    if(clazz && object && property && property->getter){
        if(property->type == & objv_type_int){
            return (objv_object_t *) objv_value_new_intValue(object->zone, (* (objv_object_property_intValue_t) property->getter)(clazz,object));
        }
        else if(property->type == & objv_type_uint){
            return (objv_object_t *) objv_value_new_uintValue(object->zone, (* (objv_object_property_uintValue_t) property->getter)(clazz,object));
        }
        else if(property->type == & objv_type_long){
            return (objv_object_t *) objv_value_new_longValue(object->zone, (* (objv_object_property_longValue_t) property->getter)(clazz,object));
        }
        else if(property->type == & objv_type_ulong){
            return (objv_object_t *) objv_value_new_ulongValue(object->zone, (* (objv_object_property_ulongValue_t) property->getter)(clazz,object));
        }
        else if(property->type == & objv_type_longLong){
            return (objv_object_t *) objv_value_new_longLongValue(object->zone,(* (objv_object_property_longLongValue_t) property->getter)(clazz,object));
        }
        else if(property->type == & objv_type_ulongLong){
            return (objv_object_t *) objv_value_new_ulongLongValue(object->zone,(* (objv_object_property_ulongLongValue_t) property->getter)(clazz,object));
        }
        else if(property->type == & objv_type_float){
            return (objv_object_t *) objv_value_new_floatValue(object->zone,(* (objv_object_property_floatValue_t) property->getter)(clazz,object));
        }
        else if(property->type == & objv_type_double){
            return (objv_object_t *) objv_value_new_doubleValue(object->zone,(* (objv_object_property_doubleValue_t) property->getter)(clazz,object));
        }
        else if(property->type == & objv_type_object){
            return (* (objv_object_property_objectValue_t) property->getter)(clazz,object);
        }
    }
    return defaultValue;
}

void objv_property_setIntValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property, int value){
    if(clazz && object && property && property->setter){
        if(property->type == & objv_type_int){
            (* (objv_object_property_setIntValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_uint){
            (* (objv_object_property_setUintValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_long){
            (* (objv_object_property_setLongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_ulong){
            (* (objv_object_property_setUlongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_longLong){
            (* (objv_object_property_setLongLongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_ulongLong){
            (* (objv_object_property_setUlongLongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_float){
            (* (objv_object_property_setFloatValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_double){
            (* (objv_object_property_setDoubleValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_object){
            (* (objv_object_property_setObjectValue_t) property->setter)(clazz,object,(objv_object_t *)objv_value_new_intValue(object->zone, value));
        }
    }
}

void objv_property_setUintValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,unsigned int value){
    if(clazz && object && property && property->setter){
        if(property->type == & objv_type_int){
            (* (objv_object_property_setIntValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_uint){
            (* (objv_object_property_setUintValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_long){
            (* (objv_object_property_setLongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_ulong){
            (* (objv_object_property_setUlongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_longLong){
            (* (objv_object_property_setLongLongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_ulongLong){
            (* (objv_object_property_setUlongLongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_float){
            (* (objv_object_property_setFloatValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_double){
            (* (objv_object_property_setDoubleValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_object){
            (* (objv_object_property_setObjectValue_t) property->setter)(clazz,object,(objv_object_t *)objv_value_new_uintValue(object->zone, value));
        }
    }
}

void objv_property_setLongValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,long value){
    if(clazz && object && property && property->setter){
        if(property->type == & objv_type_int){
            (* (objv_object_property_setIntValue_t) property->setter)(clazz,object,(int)value);
        }
        else if(property->type == & objv_type_uint){
            (* (objv_object_property_setUintValue_t) property->setter)(clazz,object,(unsigned int)value);
        }
        else if(property->type == & objv_type_long){
            (* (objv_object_property_setLongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_ulong){
            (* (objv_object_property_setUlongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_longLong){
            (* (objv_object_property_setLongLongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_ulongLong){
            (* (objv_object_property_setUlongLongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_float){
            (* (objv_object_property_setFloatValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_double){
            (* (objv_object_property_setDoubleValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_object){
            (* (objv_object_property_setObjectValue_t) property->setter)(clazz,object,(objv_object_t *)objv_value_new_longValue(object->zone, value));
        }
    }
}

void objv_property_setUlongValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,unsigned long value){
    if(clazz && object && property && property->setter){
        if(property->type == & objv_type_int){
            (* (objv_object_property_setIntValue_t) property->setter)(clazz,object,(int)value);
        }
        else if(property->type == & objv_type_uint){
            (* (objv_object_property_setUintValue_t) property->setter)(clazz,object,(unsigned int)value);
        }
        else if(property->type == & objv_type_long){
            (* (objv_object_property_setLongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_ulong){
            (* (objv_object_property_setUlongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_longLong){
            (* (objv_object_property_setLongLongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_ulongLong){
            (* (objv_object_property_setUlongLongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_float){
            (* (objv_object_property_setFloatValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_double){
            (* (objv_object_property_setDoubleValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_object){
            (* (objv_object_property_setObjectValue_t) property->setter)(clazz,object,(objv_object_t *)objv_value_new_ulongValue(object->zone, value));
        }
    }
}

void objv_property_setLongLongValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,long long value){
    if(clazz && object && property && property->setter){
        if(property->type == & objv_type_int){
            (* (objv_object_property_setIntValue_t) property->setter)(clazz,object,(int)value);
        }
        else if(property->type == & objv_type_uint){
            (* (objv_object_property_setUintValue_t) property->setter)(clazz,object,(unsigned int)value);
        }
        else if(property->type == & objv_type_long){
            (* (objv_object_property_setLongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_ulong){
            (* (objv_object_property_setUlongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_longLong){
            (* (objv_object_property_setLongLongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_ulongLong){
            (* (objv_object_property_setUlongLongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_float){
            (* (objv_object_property_setFloatValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_double){
            (* (objv_object_property_setDoubleValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_object){
            (* (objv_object_property_setObjectValue_t) property->setter)(clazz,object,(objv_object_t *)objv_value_new_longLongValue(object->zone, value));
        }
    }
}

void objv_property_setUlongLongValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,unsigned long long value){
    if(clazz && object && property && property->setter){
        if(property->type == & objv_type_int){
            (* (objv_object_property_setIntValue_t) property->setter)(clazz,object,(int)value);
        }
        else if(property->type == & objv_type_uint){
            (* (objv_object_property_setUintValue_t) property->setter)(clazz,object,(unsigned int)value);
        }
        else if(property->type == & objv_type_long){
            (* (objv_object_property_setLongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_ulong){
            (* (objv_object_property_setUlongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_longLong){
            (* (objv_object_property_setLongLongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_ulongLong){
            (* (objv_object_property_setUlongLongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_float){
            (* (objv_object_property_setFloatValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_double){
            (* (objv_object_property_setDoubleValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_object){
            (* (objv_object_property_setObjectValue_t) property->setter)(clazz,object,(objv_object_t *)objv_value_new_ulongLongValue(object->zone, value));
        }
    }
}

void objv_property_setFloatValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,float value){
    if(clazz && object && property && property->setter){
        if(property->type == & objv_type_int){
            (* (objv_object_property_setIntValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_uint){
            (* (objv_object_property_setUintValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_long){
            (* (objv_object_property_setLongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_ulong){
            (* (objv_object_property_setUlongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_longLong){
            (* (objv_object_property_setLongLongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_ulongLong){
            (* (objv_object_property_setUlongLongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_float){
            (* (objv_object_property_setFloatValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_double){
            (* (objv_object_property_setDoubleValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_object){
            (* (objv_object_property_setObjectValue_t) property->setter)(clazz,object,(objv_object_t *)objv_value_new_floatValue(object->zone, value));
        }
    }
}

void objv_property_setDoubleValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,double value){
    if(clazz && object && property && property->setter){
        if(property->type == & objv_type_int){
            (* (objv_object_property_setIntValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_uint){
            (* (objv_object_property_setUintValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_long){
            (* (objv_object_property_setLongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_ulong){
            (* (objv_object_property_setUlongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_longLong){
            (* (objv_object_property_setLongLongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_ulongLong){
            (* (objv_object_property_setUlongLongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_float){
            (* (objv_object_property_setFloatValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_double){
            (* (objv_object_property_setDoubleValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_object){
            (* (objv_object_property_setObjectValue_t) property->setter)(clazz,object,(objv_object_t *)objv_value_new_doubleValue(object->zone, value));
        }
    }
}

void objv_property_setBooleanValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,objv_boolean_t value){
    if(clazz && object && property && property->setter){
        if(property->type == & objv_type_int){
            (* (objv_object_property_setIntValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_uint){
            (* (objv_object_property_setUintValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_long){
            (* (objv_object_property_setLongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_ulong){
            (* (objv_object_property_setUlongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_longLong){
            (* (objv_object_property_setLongLongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_ulongLong){
            (* (objv_object_property_setUlongLongValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_float){
            (* (objv_object_property_setFloatValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_double){
            (* (objv_object_property_setDoubleValue_t) property->setter)(clazz,object,value);
        }
        else if(property->type == & objv_type_object){
            (* (objv_object_property_setObjectValue_t) property->setter)(clazz,object,(objv_object_t *)objv_value_new_booleanValue(object->zone, value));
        }
    }
}

void objv_property_setObjectValue(objv_class_t * clazz,objv_object_t * object,objv_property_t * property,objv_object_t * value){
    if(clazz && object && property && property->setter){
        if(property->type == & objv_type_int){
            (* (objv_object_property_setIntValue_t) property->setter)(clazz,object,objv_object_intValue(value, 0));
        }
        else if(property->type == & objv_type_uint){
            (* (objv_object_property_setUintValue_t) property->setter)(clazz,object,objv_object_uintValue(value, 0));
        }
        else if(property->type == & objv_type_long){
            (* (objv_object_property_setLongValue_t) property->setter)(clazz,object,objv_object_longValue(value, 0));
        }
        else if(property->type == & objv_type_ulong){
            (* (objv_object_property_setUlongValue_t) property->setter)(clazz,object,objv_object_ulongValue(value, 0));
        }
        else if(property->type == & objv_type_longLong){
            (* (objv_object_property_setLongLongValue_t) property->setter)(clazz,object,objv_object_longLongValue(value, 0));
        }
        else if(property->type == & objv_type_ulongLong){
            (* (objv_object_property_setUlongLongValue_t) property->setter)(clazz,object,objv_object_ulongLongValue(value, 0));
        }
        else if(property->type == & objv_type_float){
            (* (objv_object_property_setFloatValue_t) property->setter)(clazz,object,objv_object_floatValue(value, 0));
        }
        else if(property->type == & objv_type_double){
            (* (objv_object_property_setDoubleValue_t) property->setter)(clazz,object,objv_object_doubleValue(value, 0));
        }
        else if(property->type == & objv_type_object){
            (* (objv_object_property_setObjectValue_t) property->setter)(clazz,object,value);
        }
    }
}
