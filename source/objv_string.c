//
//  objv_string.h
//  objv
//
//  Created by zhang hailong on 14-1-30.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY


#include "objv_os.h"
#include "objv.h"
#include "objv_string.h"
#include "objv_autorelease.h"
#include "objv_value.h"
#include "objv_mbuf.h"

OBJV_KEY_IMP(String)

static long objv_string_method_hashCode (objv_class_t * clazz, objv_object_t * object){
    
    objv_string_t * value = (objv_string_t *) object;
    
    unsigned char * p = (unsigned char *) value->UTF8String;
    size_t length = value->length;
    long hashCode = 0;
    
    while(length >0){
        
        hashCode += * p;
        
        length --;
        p ++;
    }
    
    return hashCode == 0 ? (long)value->UTF8String : hashCode;
}

static objv_boolean_t objv_string_method_equal(objv_class_t * clazz, objv_object_t * object,objv_object_t * value){
    
    if(object != value && objv_object_isKindOfClass(value, & objv_string_class)){
        
        objv_string_t * v1 = (objv_string_t *) object;
        objv_string_t * v2 = (objv_string_t *) value;
        
        return strcmp(v1->UTF8String, v2->UTF8String) ==0;
    
    }
    
    return object == value;
}

static void objv_string_methods_dealloc(objv_class_t * clazz, objv_object_t * obj){
    
    objv_string_t * value = (objv_string_t *) obj;
    
    if(value->copyed && value->UTF8String){
        objv_zone_free(obj->zone,value->UTF8String);
    }
    
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass,obj);
    }
}

static objv_object_t * objv_string_methods_init(objv_class_t * clazz, objv_object_t * object,va_list ap){
    
    if(clazz->superClass){
        object = objv_object_initv(clazz->superClass, object, ap);
    }
    
    if(object){
        
        objv_string_t * value = (objv_string_t *) object;
        const char * s = va_arg(ap, const char *);
        objv_boolean_t copyed = va_arg(ap, objv_boolean_t);
        
        if(copyed){
            value->length = strlen(s);
            value->UTF8String = objv_zone_malloc(object->zone, value->length + 1);
            value->copyed = objv_true;
            strcpy(value->UTF8String, s);
        }
        else{
            value->length = strlen(s);
            value->UTF8String = objv_zone_malloc(object->zone, value->length + 1);
            value->copyed = objv_false;
        }
    }
    
    return object;
}

static objv_string_t * objv_string_methods_stringValue(objv_class_t * clazz,objv_object_t * obj){
    return (objv_string_t *) obj;
}

static int objv_string_methods_intValue(objv_class_t * clazz,objv_object_t * obj){
    objv_string_t * string = (objv_string_t *) obj;
    return atoi(string->UTF8String);
}

static unsigned int objv_string_methods_uintValue(objv_class_t * clazz,objv_object_t * obj){
    objv_string_t * string = (objv_string_t *) obj;
    return atoi(string->UTF8String);
}

static long objv_string_methods_longValue(objv_class_t * clazz,objv_object_t * obj){
    objv_string_t * string = (objv_string_t *) obj;
    return atol(string->UTF8String);
}

static unsigned long objv_string_methods_ulongValue(objv_class_t * clazz,objv_object_t * obj){
    objv_string_t * string = (objv_string_t *) obj;
    return atol(string->UTF8String);
}

static long long objv_string_methods_longLongValue(objv_class_t * clazz,objv_object_t * obj){
    objv_string_t * string = (objv_string_t *) obj;
    return atoll(string->UTF8String);
}

static unsigned long long objv_string_methods_ulongLongValue(objv_class_t * clazz,objv_object_t * obj){
    objv_string_t * string = (objv_string_t *) obj;
    return atoll(string->UTF8String);
}

static float objv_string_methods_floatValue(objv_class_t * clazz,objv_object_t * obj){
    objv_string_t * string = (objv_string_t *) obj;
    return atof(string->UTF8String);
}

static double objv_string_methods_doubleValue(objv_class_t * clazz,objv_object_t * obj){
    objv_string_t * string = (objv_string_t *) obj;
    return atof(string->UTF8String);
}

static objv_boolean_t objv_string_methods_booleanValue(objv_class_t * clazz,objv_object_t * obj){
    objv_string_t * string = (objv_string_t *) obj;
    
    if(strcmp(string->UTF8String,"true") ==0){
        return objv_true;
    }
    
    if(strcmp(string->UTF8String,"false") ==0){
        return objv_true;
    }
    
    return atoi(string->UTF8String) ? objv_true : objv_false;
}


static objv_method_t objv_string_methods[] = {
    {OBJV_KEY(dealloc),"v()",(objv_method_impl_t)objv_string_methods_dealloc}
    ,{OBJV_KEY(init),"@(*)",(objv_method_impl_t) objv_string_methods_init}
    ,{OBJV_KEY(hashCode),"l()",(objv_method_impl_t)objv_string_method_hashCode}
    ,{OBJV_KEY(equal),"l()",(objv_method_impl_t)objv_string_method_equal}
    ,{OBJV_KEY(stringValue),"@()",(objv_method_impl_t)objv_string_methods_stringValue}
    ,{OBJV_KEY(intValue),"i()",(objv_method_impl_t)objv_string_methods_intValue}
    ,{OBJV_KEY(uintValue),"I()",(objv_method_impl_t)objv_string_methods_uintValue}
    ,{OBJV_KEY(longValue),"l()",(objv_method_impl_t)objv_string_methods_longValue}
    ,{OBJV_KEY(ulongValue),"L()",(objv_method_impl_t)objv_string_methods_ulongValue}
    ,{OBJV_KEY(longLongValue),"q()",(objv_method_impl_t)objv_string_methods_longLongValue}
    ,{OBJV_KEY(ulongLongValue),"Q()",(objv_method_impl_t)objv_string_methods_ulongLongValue}
    ,{OBJV_KEY(floatValue),"f()",(objv_method_impl_t)objv_string_methods_floatValue}
    ,{OBJV_KEY(doubleValue),"d()",(objv_method_impl_t)objv_string_methods_doubleValue}
    ,{OBJV_KEY(booleanValue),"b()",(objv_method_impl_t)objv_string_methods_booleanValue}
};

static objv_property_t objv_string_propertys[] = {
    {OBJV_KEY(stringValue),&objv_type_object,&objv_string_methods[3],NULL}
    ,{OBJV_KEY(intValue),&objv_type_int,&objv_string_methods[4],NULL}
    ,{OBJV_KEY(uintValue),&objv_type_uint,&objv_string_methods[5],NULL}
    ,{OBJV_KEY(longValue),&objv_type_long,&objv_string_methods[6],NULL}
    ,{OBJV_KEY(ulongValue),&objv_type_ulong,&objv_string_methods[7],NULL}
    ,{OBJV_KEY(longLongValue),&objv_type_longLong,&objv_string_methods[8],NULL}
    ,{OBJV_KEY(ulongLongValue),&objv_type_ulongLong,&objv_string_methods[9],NULL}
    ,{OBJV_KEY(floatValue),&objv_type_float,&objv_string_methods[10],NULL}
    ,{OBJV_KEY(doubleValue),&objv_type_double,&objv_string_methods[11],NULL}
    ,{OBJV_KEY(booleanValue),&objv_type_boolean,&objv_string_methods[12],NULL}
};

objv_class_t objv_string_class = {OBJV_KEY(String),& objv_Object_class
    ,objv_string_methods,sizeof(objv_string_methods) / sizeof(objv_method_t)
    ,objv_string_propertys,sizeof(objv_string_propertys) / sizeof(objv_property_t)
    ,sizeof(objv_string_t)
    ,NULL,0};


objv_string_t * objv_string_alloc(objv_zone_t * zone,const char * UTF8String){
    return (objv_string_t *) objv_object_alloc(zone,&objv_string_class,UTF8String,objv_true);
}

objv_string_t * objv_string_alloc_format(objv_zone_t * zone,const char * format,...){
    va_list ap;
    objv_string_t * s;
    
    va_start(ap, format);
    
    s = objv_string_alloc_formatv(zone, format, ap);
    
    va_end(ap);

    return s;
}

objv_string_t * objv_string_alloc_formatv(objv_zone_t * zone,const char * format,va_list ap){
    objv_mbuf_t mbuf;
    objv_string_t * s;
    
    objv_mbuf_init(& mbuf, 128);
    
    objv_mbuf_formatv(& mbuf, format, ap);
    
    s = objv_string_alloc(zone, objv_mbuf_str(& mbuf));
    
    objv_mbuf_destroy(& mbuf);
    
    return s;
}

objv_string_t * objv_string_new(objv_zone_t * zone,const char * UTF8String){
    return (objv_string_t *) objv_object_autorelease( (objv_object_t *) objv_string_alloc(zone, UTF8String));
}

objv_string_t * objv_string_new_format(objv_zone_t * zone,const char * format,...){
    
    va_list ap;
    objv_string_t * s;
    
    va_start(ap, format);
    
    s = objv_string_alloc_formatv(zone, format, ap);
    
    va_end(ap);
    
    return (objv_string_t *) objv_object_autorelease((objv_object_t *) s);
}

objv_string_t * objv_string_unicode_alloc(objv_zone_t * zone,unsigned short * unicode,size_t length){
    
    objv_string_t * s;
    
    objv_mbuf_t mbuf;
  
    objv_mbuf_init(& mbuf, length * 2);
   
    objv_unicode_to_utf8(unicode,length, & mbuf);
    
    s = objv_string_alloc(zone,objv_mbuf_str(& mbuf));
    
    objv_mbuf_destroy(& mbuf);
    
    return s;
    
}

objv_string_t * objv_string_unicode_new(objv_zone_t * zone,unsigned short * unicode,size_t length){
    return (objv_string_t *) objv_object_autorelease( (objv_object_t *) objv_string_unicode_alloc(zone, unicode,length));
}

objv_string_t * objv_string_alloc_nocopy(objv_zone_t * zone,const char * UTF8String){
    return (objv_string_t *) objv_object_alloc(zone,&objv_string_class,UTF8String,objv_false);
}

size_t objv_unicode_to_utf8(unsigned short * unicode, size_t length, objv_mbuf_t * mbuf){
   
    char c;
    size_t l = 0;
    
    while(length >0){
        
        if(* unicode <0x80){
            c = * unicode;
            objv_mbuf_append(mbuf, & c, 1);
            
            l += 1;
        }
        else if( * unicode < 0x800){
            
            c = 0xc0 | ( * unicode >> 6);
            objv_mbuf_append(mbuf, & c, 1);
            
            c = 0x80 | ( * unicode & 0x3f);
            objv_mbuf_append(mbuf, & c, 1);
            
            l += 2;
        }
        else{
            
            c = 0xe0 | ( * unicode >> 12);
            objv_mbuf_append(mbuf, & c, 1);
            
            c = 0x80 | ( (* unicode >> 6) & 0x3f );
            objv_mbuf_append(mbuf, & c, 1);
            
            c = 0x80 | ( * unicode & 0x3f);
            objv_mbuf_append(mbuf, & c, 1);
            
            l += 3;
        }
        
        unicode ++;
        length --;
    }
    
    return l;
}

const char * objv_string_indexOf(const char * string,const char * substring){
    if(string && substring){
        char * p = (char *) string;
        char * s = (char *) substring;
        
        while(*p !=0 && * s != 0){
            
            if(*p == *s){
                s ++;
            }
            else{
                p = p - (s - substring);
                s = (char *) substring;
            }
            
            p ++;
        }
        return * s == 0 ? p - (s - substring) : NULL;
    }
    return NULL;
}
