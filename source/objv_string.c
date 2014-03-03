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
#include "objv_array.h"

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
    
    if(object != value && objv_object_isKindOfClass(value, OBJV_CLASS(String))){
        
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
        char * s = va_arg(ap, char *);
        objv_boolean_t copyed = va_arg(ap, objv_boolean_t);
        
        if(copyed){
            value->length = strlen(s);
            value->UTF8String = objv_zone_malloc(object->zone, value->length + 1);
            value->copyed = objv_true;
            strcpy(value->UTF8String, s);
        }
        else{
            value->length = strlen(s);
            value->UTF8String = s;
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

OBJV_CLASS_METHOD_IMP_BEGIN(String)

OBJV_CLASS_METHOD_IMP(dealloc,"v()",objv_string_methods_dealloc)

OBJV_CLASS_METHOD_IMP(init,"@(*)",objv_string_methods_init)

OBJV_CLASS_METHOD_IMP(hashCode,"l()",objv_string_method_hashCode)
OBJV_CLASS_METHOD_IMP(equal,"l()",objv_string_method_equal)
OBJV_CLASS_METHOD_IMP(stringValue,"@()",objv_string_methods_stringValue)
OBJV_CLASS_METHOD_IMP(intValue,"i()",objv_string_methods_intValue)
OBJV_CLASS_METHOD_IMP(uintValue,"I()",objv_string_methods_uintValue)
OBJV_CLASS_METHOD_IMP(longValue,"l()",objv_string_methods_longValue)
OBJV_CLASS_METHOD_IMP(ulongValue,"L()",objv_string_methods_ulongValue)
OBJV_CLASS_METHOD_IMP(longLongValue,"q()",objv_string_methods_longLongValue)
OBJV_CLASS_METHOD_IMP(ulongLongValue,"Q()",objv_string_methods_ulongLongValue)
OBJV_CLASS_METHOD_IMP(floatValue,"f()",objv_string_methods_floatValue)
OBJV_CLASS_METHOD_IMP(doubleValue,"d()",objv_string_methods_doubleValue)
OBJV_CLASS_METHOD_IMP(booleanValue,"b()",objv_string_methods_booleanValue)

OBJV_CLASS_METHOD_IMP_END(String)

OBJV_CLASS_PROPERTY_IMP_BEGIN(String)

OBJV_CLASS_PROPERTY_IMP(stringValue, object, objv_string_methods_stringValue, NULL, objv_false)
OBJV_CLASS_PROPERTY_IMP(intValue, object, objv_string_methods_intValue, NULL, objv_false)
OBJV_CLASS_PROPERTY_IMP(uintValue, object, objv_string_methods_uintValue, NULL, objv_false)
OBJV_CLASS_PROPERTY_IMP(longValue, object, objv_string_methods_longValue, NULL, objv_false)
OBJV_CLASS_PROPERTY_IMP(ulongValue, object, objv_string_methods_ulongValue, NULL, objv_false)
OBJV_CLASS_PROPERTY_IMP(longLongValue, object, objv_string_methods_longLongValue, NULL, objv_false)
OBJV_CLASS_PROPERTY_IMP(ulongLongValue, object, objv_string_methods_ulongLongValue, NULL, objv_false)
OBJV_CLASS_PROPERTY_IMP(floatValue, object, objv_string_methods_floatValue, NULL, objv_false)
OBJV_CLASS_PROPERTY_IMP(doubleValue, object, objv_string_methods_doubleValue, NULL, objv_false)
OBJV_CLASS_PROPERTY_IMP(booleanValue, object, objv_string_methods_booleanValue, NULL, objv_false)

OBJV_CLASS_PROPERTY_IMP_END(String)

OBJV_CLASS_IMP_P_M(String, OBJV_CLASS(Object), objv_string_t);


objv_string_t * objv_string_alloc(objv_zone_t * zone,const char * UTF8String){
    return (objv_string_t *) objv_object_alloc(zone,OBJV_CLASS(String),UTF8String,objv_true,NULL);
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

objv_string_t * objv_string_new_nocopy(objv_zone_t * zone,const char * UTF8String){
    return (objv_string_t *) objv_object_autorelease( (objv_object_t *) objv_string_alloc_nocopy(zone, UTF8String));
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
    return (objv_string_t *) objv_object_alloc(zone,OBJV_CLASS(String),UTF8String,objv_false,NULL);
}

struct _objv_array_t * objv_string_split_UTF8String(objv_zone_t * zone,const char * UTF8String,const char * splitString){
    
    if(UTF8String && splitString){
        
        objv_array_t * array = objv_array_new(zone, 4);
        objv_string_t * s;
        objv_mbuf_t mbuf;
        char * p = (char *) UTF8String;
        size_t l = strlen(splitString);
        
        objv_mbuf_init(& mbuf, 64);
        
        while (p && *p != 0) {
            
            if(l == 0){
                
                objv_mbuf_append(& mbuf, p, 1);
                
                s = objv_string_alloc(zone, objv_mbuf_str(& mbuf));
                objv_array_add(array, (objv_object_t *) s );
                objv_object_release((objv_object_t *) s);
                objv_mbuf_clear(& mbuf);
                
                p ++;
            }
            else if(strncmp(p, splitString, l) == 0){
                s = objv_string_alloc(zone, objv_mbuf_str(& mbuf));
                objv_array_add(array, (objv_object_t *) s );
                objv_object_release((objv_object_t *) s);
                objv_mbuf_clear(& mbuf);
                p += l;
            }
            else{
                objv_mbuf_append(& mbuf, p, 1);
                p ++;
            }
        }
        
        objv_mbuf_destroy(& mbuf);
        
        return array;
    }
    
    return NULL;
}

struct _objv_array_t * objv_string_split(objv_string_t * string,const char * splitString){
    if(string && splitString){
        return objv_string_split_UTF8String(string->base.zone,string->UTF8String,splitString);
    }
    return NULL;
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

const char * objv_string_lastIndexOf(const char * string,const char * substring){
    if(string && substring){
        char * p = (char *) string + strlen(string) - 1;
        char * s = (char *) substring + strlen(substring) - 1;
        char * es = s;
        
        while(p >= string && s >= substring ){
            
            if(*p == *s){
                s --;
            }
            else{
                p = p + (es - s);
                s = es ;
            }
            
            p --;
        }
        return s < substring ? p + 1 : NULL;
    }
    return NULL;
}

const char * objv_string_hasPrefix(const char * string,const char * substring){
    if(string && substring){
        char * p = (char *) string;
        char * s = (char *) substring;
        
        while(*p !=0 && * s != 0){
            
            if(*p == *s){
                s ++;
            }
            else{
                break;
            }
            
            p ++;
        }
        return * s == 0 ? p : NULL;
    }
    return NULL;
}

const char * objv_string_hasPrefixTo(const char * string,const char * substring,const char * toSubstring){
    if(string && substring){
        char * p = (char *) string;
        char * s = (char *) substring;
        
        while(*p !=0 && s != toSubstring){
            
            if(*p == *s){
                s ++;
            }
            else{
                break;
            }
            
            p ++;
        }
        return s == toSubstring ? string : NULL;
    }
    return NULL;
}

const char * objv_string_hasSuffix(const char * string,const char * substring){
    if(string && substring){
        char * p = (char *) string + strlen(string) - 1;
        char * s = (char *) substring + strlen(substring) - 1;
        
        while(p >= string && s >= substring ){
            
            if(*p == *s){
                s --;
            }
            else{
                break;
            }
            
            p --;
        }
        return s < substring ? p + 1 : NULL;
    }
    return NULL;
}
