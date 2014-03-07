//
//  objv_db.c
//  objv
//
//  Created by zhang hailong on 14-3-7.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv.h"

#include "objv_db.h"

#include "objv_value.h"

#include "objv_string.h"

#include "objv_iterator.h"

#include "objv_array.h"

#include "objv_autorelease.h"

OBJV_KEY_IMP(DB)
OBJV_KEY_IMP(DBCursor)
OBJV_KEY_IMP(exec)
OBJV_KEY_IMP(query)
OBJV_KEY_IMP(next)
OBJV_KEY_IMP(indexOfKey)
OBJV_KEY_IMP(keyAtIndex)
OBJV_KEY_IMP(cStringValueAtIndex)
OBJV_KEY_IMP(intValueAtIndex)
OBJV_KEY_IMP(longValueAtIndex)
OBJV_KEY_IMP(longlongValueAtIndex)
OBJV_KEY_IMP(doubleValueAtIndex)
OBJV_KEY_IMP(count)
OBJV_KEY_IMP(beginTransaction)
OBJV_KEY_IMP(commit)
OBJV_KEY_IMP(rollback)

static void objv_db_method_dealloc(objv_class_t * clazz,objv_object_t * object){

    objv_db_t * db = (objv_db_t *) object;
    
    objv_object_release( (objv_object_t *) db->exception);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
}

static OBJVDBStatus objv_db_method_exec(objv_class_t * clazz,objv_db_t * db,const char * sql,objv_object_t * data){
    
    objv_db_set_exception_format(db, 0, "not implement db exec");
    
    return OBJVDBStatusException;
}

static objv_db_cursor_t * objv_db_method_query(objv_class_t * clazz,objv_db_t * db,const char * sql,objv_object_t * data){
    
    objv_db_set_exception_format(db, 0, "not implement db query");
    
    return NULL;
}

OBJV_CLASS_METHOD_IMP_BEGIN(DB)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", objv_db_method_dealloc)

OBJV_CLASS_METHOD_IMP(exec, "i(*,@)", objv_db_method_exec)

OBJV_CLASS_METHOD_IMP(query, "@(*,@)", objv_db_method_query)

OBJV_CLASS_METHOD_IMP_END(DB)

OBJV_CLASS_IMP_M(DB, OBJV_CLASS(Object), objv_db_t)


void objv_db_set_exception(objv_db_t * db,objv_exception_t * exception){
    
    if(db && db->exception != exception){
        
        objv_object_retain((objv_object_t *) exception);
        objv_object_release((objv_object_t *) db->exception);
        
        db->exception = exception;

    }
    
}

void objv_db_set_exception_format(objv_db_t * db,int code,const char * format,...){
    if(db){
        va_list va;
        
        va_start(va, format);
        
        objv_db_set_exception_formatv(db, code,format,va);
        
        va_end(va);
    }
}

void objv_db_set_exception_formatv(objv_db_t * db,int code,const char * format,va_list va){
    if(db){
        objv_db_set_exception(db, objv_exception_newv(db->base.zone, code, format, va));
    }
}


OBJVDBStatus objv_db_exec(objv_class_t * clazz,objv_db_t * db,const char * sql,objv_object_t * data){
    
    if(clazz && db && sql){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(exec))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_db_method_exec_t) method->impl)(c,db,sql,data);
        }
    }
    
    return OBJVDBStatusException;
}

objv_db_cursor_t * objv_db_query(objv_class_t * clazz,objv_db_t * db,const char * sql,objv_object_t * data){
    
    if(clazz && db && sql){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(query))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_db_method_query_t) method->impl)(c,db,sql,data);
        }
    }
    
    return NULL;
}

OBJVDBStatus objv_db_beginTransaction(objv_class_t * clazz,objv_db_t * db){
    
    if(clazz && db){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(beginTransaction))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_db_method_beginTransaction_t) method->impl)(c,db);
        }
    }
    
    return OBJVDBStatusException;
}

OBJVDBStatus objv_db_commit(objv_class_t * clazz,objv_db_t * db){
    
    if(clazz && db){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(commit))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_db_method_commit_t) method->impl)(c,db);
        }
    }
    
    return OBJVDBStatusException;
}

OBJVDBStatus objv_db_rollback(objv_class_t * clazz,objv_db_t * db){
    
    if(clazz && db){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(rollback))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_db_method_rollback_t) method->impl)(c,db);
        }
    }
    
    return OBJVDBStatusException;
}


static objv_boolean_t objv_db_cursor_method_next(objv_class_t * clazz,objv_db_t * db,const char * sql,objv_object_t * data){
    return objv_false;
}


static objv_object_t * objv_db_cursor_methods_objectForKey(objv_class_t * clazz,objv_object_t * object,objv_object_t * key){

    objv_string_t * skey = objv_object_stringValue(key, NULL);
    
    if(skey){
        
        int index = objv_db_cursor_indexOfKey(object->isa, (objv_db_cursor_t *) object, skey->UTF8String);
        
        if(index >=0){
            
            const char * v = objv_db_cursor_cStringValueAtIndex(object->isa, (objv_db_cursor_t *) object, index, NULL);
            
            if(v){
                
                return (objv_object_t *) objv_string_new(object->zone, v);
                
            }
            
        }
        
    }
    
    return NULL;
}

typedef struct _objv_db_cursor_keyIterator_t {
    objv_iterator_t base;
    objv_array_t * keys;
    unsigned int index;
} objv_db_cursor_keyIterator_t;

OBJV_CLASS_DEC(DBCursorKeyIterator)
OBJV_KEY_IMP(DBCursorKeyIterator);

static void objv_db_cursor_keyIterator_dealloc(objv_class_t * clazz,objv_object_t * object){

    objv_db_cursor_keyIterator_t * keyIterator = (objv_db_cursor_keyIterator_t *) object;
    
    objv_object_release((objv_object_t *) keyIterator->keys);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
}

static objv_object_t * objv_db_cursor_keyIterator_init(objv_class_t * clazz,objv_object_t * object,va_list ap){
    
    if(clazz->superClass){
        object = objv_object_initv(clazz, object, ap);
    }
    
    if(object){
        
        objv_db_cursor_keyIterator_t * keyIterator = (objv_db_cursor_keyIterator_t *) object;
        objv_db_cursor_t * cursor = va_arg(ap, objv_db_cursor_t *);
        if(cursor){
            
            unsigned int count = objv_db_cursor_count(cursor->base.isa, cursor);
            const char * key;
            
            if(count > 0){
                
                keyIterator->keys = objv_array_alloc(object->zone, count);
                
                for(int i=0;i<count;i++){
                    
                    key = objv_db_cursor_cStringValueAtIndex(cursor->base.isa, cursor, i, NULL);
                    
                    if(key){
                        
                        objv_array_add(keyIterator->keys, (objv_object_t *) objv_string_new(object->zone, key));
                        
                    }
                    
                }
            }
        }
    }
    
    return object;
}

static objv_object_t * objv_db_cursor_keyIterator_next(objv_class_t * clazz,objv_object_t * object) {

    objv_db_cursor_keyIterator_t * keyIterator = (objv_db_cursor_keyIterator_t *) object;
    
    if(keyIterator->keys && keyIterator->index < keyIterator->keys->length){
    
        return objv_array_objectAt(keyIterator->keys, keyIterator->index ++);
    }
    
    return NULL;
}


OBJV_CLASS_METHOD_IMP_BEGIN(DBCursorKeyIterator)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", objv_db_cursor_keyIterator_dealloc)

OBJV_CLASS_METHOD_IMP(init, "@(*)", objv_db_cursor_keyIterator_init)

OBJV_CLASS_METHOD_IMP(next, "@()", objv_db_cursor_keyIterator_next)

OBJV_CLASS_METHOD_IMP_END(DBCursorKeyIterator)

OBJV_CLASS_IMP_M(DBCursorKeyIterator, OBJV_CLASS(Iterator), objv_db_cursor_keyIterator_t)


static objv_object_t * objv_db_cursor_method_keyIterator(objv_class_t * clazz,objv_db_cursor_t * cursor){
    return objv_object_new(cursor->base.zone, OBJV_CLASS(DBCursorKeyIterator),cursor,NULL);
}


OBJV_CLASS_METHOD_IMP_BEGIN(DBCursor)

OBJV_CLASS_METHOD_IMP(next, "b()", objv_db_cursor_method_next)

OBJV_CLASS_METHOD_IMP(objectForKey, "@(@)", objv_db_cursor_methods_objectForKey)

OBJV_CLASS_METHOD_IMP(keyIterator, "@()", objv_db_cursor_method_keyIterator)

OBJV_CLASS_METHOD_IMP_END(DBCursor)

OBJV_CLASS_IMP_M(DBCursor, OBJV_CLASS(Object), objv_db_cursor_t)


objv_boolean_t objv_db_cursor_next(objv_class_t * clazz,objv_db_cursor_t * cursor){
    if(clazz && cursor){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(next))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_db_cursor_method_next_t) method->impl)(c,cursor);
        }
    }
    return objv_false;
}

int objv_db_cursor_indexOfKey(objv_class_t * clazz,objv_db_cursor_t * cursor,const char * key){
    
    if(clazz && cursor){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(indexOfKey))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_db_cursor_method_indexOfKey_t) method->impl)(c,cursor,key);
        }
    }
    
    return -1;
}

const char * objv_db_cursor_keyAtIndex (objv_class_t * clazz,objv_db_cursor_t * cursor,int index){
    
    if(clazz && cursor){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(keyAtIndex))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_db_cursor_method_keyAtIndex_t) method->impl)(c,cursor,index);
        }
    }
    
    return NULL;
}

const char * objv_db_cursor_cStringValueAtIndex (objv_class_t * clazz,objv_db_cursor_t * cursor,int index,const char * defaultValue){
    
    if(clazz && cursor){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(cStringValueAtIndex))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_db_cursor_method_cStringValueAtIndex_t) method->impl)(c,cursor,index,defaultValue);
        }
    }
    
    return defaultValue;
    
}

int objv_db_cursor_intValueAtIndex (objv_class_t * clazz,objv_db_cursor_t * cursor,int index,int defaultValue){
    
    if(clazz && cursor){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(intValueAtIndex))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_db_cursor_method_intValueAtIndex_t) method->impl)(c,cursor,index,defaultValue);
        }
    }
    
    return defaultValue;
}

long objv_db_cursor_longValueAtIndex (objv_class_t * clazz,objv_db_cursor_t * cursor,int index,long defaultValue){
    
    if(clazz && cursor){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(longValueAtIndex))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_db_cursor_method_longValueAtIndex_t) method->impl)(c,cursor,index,defaultValue);
        }
    }
    
    return defaultValue;
}

long long objv_db_cursor_longlongValueAtIndex (objv_class_t * clazz,objv_db_cursor_t * cursor,int index,long long defaultValue){
    if(clazz && cursor){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(longlongValueAtIndex))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_db_cursor_method_longlongValueAtIndex_t) method->impl)(c,cursor,index,defaultValue);
        }
    }
    
    return defaultValue;
}

double objv_db_cursor_doubleValueAtIndex (objv_class_t * clazz,objv_db_cursor_t * cursor,int index,double defaultValue){
    if(clazz && cursor){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(doubleValueAtIndex))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_db_cursor_method_doubleValueAtIndex_t) method->impl)(c,cursor,index,defaultValue);
        }
    }
    
    return defaultValue;
}

unsigned int objv_db_cursor_count (objv_class_t * clazz,objv_db_cursor_t * cursor){
    if(clazz && cursor){
        
        objv_class_t * c = clazz;
        
        objv_method_t * method = NULL;
        
        while(c && (method = objv_class_getMethod(c, OBJV_KEY(count))) == NULL){
            
            c = c->superClass;
        }
        
        if(method){
            return (* (objv_db_cursor_method_count_t) method->impl)(c,cursor);
        }
    }
    
    return 0;
}



