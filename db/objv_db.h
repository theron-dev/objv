//
//  objv_db.h
//  objv
//
//  Created by zhang hailong on 14-3-7.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_db_h
#define objv_objv_db_h


#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_exception.h"
    
    typedef enum _OBJVDBStatus {
        OBJVDBStatusOK,OBJVDBStatusException
    } OBJVDBStatus;
    
    typedef struct _objv_db_object_t{
        objv_object_t base;
        objv_class_t * tableClass;
        unsigned long long rowid;
    } objv_db_object_t;
    
    OBJV_KEY_DEC(DBObject)
    OBJV_CLASS_DEC(DBObject)

    
    typedef struct _objv_db_t {
        objv_object_t base;
        objv_exception_t * READONLY exception;
    } objv_db_t;
    
    OBJV_KEY_DEC(DB)
    OBJV_CLASS_DEC(DB)
    
    typedef struct _objv_db_cursor_t {
        objv_object_t base;
    } objv_db_cursor_t;
    
    OBJV_KEY_DEC(DBCursor)
    OBJV_CLASS_DEC(DBCursor)
    
    void objv_db_set_exception(objv_db_t * db,objv_exception_t * exception);
    
    void objv_db_set_exception_format(objv_db_t * db,int code,const char * format,...);
    void objv_db_set_exception_formatv(objv_db_t * db,int code,const char * format,va_list va);
    
    
    OBJV_KEY_DEC(exec)
    OBJV_KEY_DEC(query)
    OBJV_KEY_DEC(beginTransaction)
    OBJV_KEY_DEC(commit)
    OBJV_KEY_DEC(rollback)
    OBJV_KEY_DEC(objectRegClass)
    OBJV_KEY_DEC(objectInsert)
    OBJV_KEY_DEC(objectDelete)
    OBJV_KEY_DEC(objectUpdate)
    
    typedef OBJVDBStatus ( * objv_db_method_exec_t) (objv_class_t * clazz,objv_db_t * db,const char * sql,objv_object_t * data);
    
    typedef objv_db_cursor_t * ( * objv_db_method_query_t) (objv_class_t * clazz,objv_db_t * db,const char * sql,objv_object_t * data);
    
    typedef OBJVDBStatus ( * objv_db_method_beginTransaction_t) (objv_class_t * clazz,objv_db_t * db);
    
    typedef OBJVDBStatus ( * objv_db_method_commit_t) (objv_class_t * clazz,objv_db_t * db);
    
    typedef OBJVDBStatus ( * objv_db_method_rollback_t) (objv_class_t * clazz,objv_db_t * db);
    
    typedef OBJVDBStatus ( * objv_db_method_object_regClass_t) (objv_class_t * clazz,objv_db_t * db,objv_class_t * dbObjectClass);
    
    typedef OBJVDBStatus ( * objv_db_method_object_insert_t) (objv_class_t * clazz,objv_db_t * db,objv_db_object_t * dbObject);
    
    typedef OBJVDBStatus ( * objv_db_method_object_delete_t) (objv_class_t * clazz,objv_db_t * db,objv_db_object_t * dbObject);
    
    typedef OBJVDBStatus ( * objv_db_method_object_update_t) (objv_class_t * clazz,objv_db_t * db,objv_db_object_t * dbObject);
    
    OBJVDBStatus objv_db_exec(objv_class_t * clazz,objv_db_t * db,const char * sql,objv_object_t * data);
    
    objv_db_cursor_t * objv_db_query(objv_class_t * clazz,objv_db_t * db,const char * sql,objv_object_t * data);
    
    OBJVDBStatus objv_db_beginTransaction(objv_class_t * clazz,objv_db_t * db);
    
    OBJVDBStatus objv_db_commit(objv_class_t * clazz,objv_db_t * db);
    
    OBJVDBStatus objv_db_rollback(objv_class_t * clazz,objv_db_t * db);
    
    OBJVDBStatus objv_db_object_regClass(objv_class_t * clazz,objv_db_t * db,objv_class_t * dbObjectClass);
    
    OBJVDBStatus objv_db_object_insert(objv_class_t * clazz,objv_db_t * db,objv_db_object_t * dbObject);
    
    OBJVDBStatus objv_db_object_delete(objv_class_t * clazz,objv_db_t * db,objv_db_object_t * dbObject);
    
    OBJVDBStatus objv_db_object_update(objv_class_t * clazz,objv_db_t * db,objv_db_object_t * dbObject);
    
    OBJV_KEY_DEC(next)
    OBJV_KEY_DEC(indexOfKey)
    OBJV_KEY_DEC(keyAtIndex)
    OBJV_KEY_DEC(cStringValueAtIndex)
    OBJV_KEY_DEC(intValueAtIndex)
    OBJV_KEY_DEC(longValueAtIndex)
    OBJV_KEY_DEC(longlongValueAtIndex)
    OBJV_KEY_DEC(doubleValueAtIndex)
    OBJV_KEY_DEC(count)
    
    typedef objv_boolean_t ( * objv_db_cursor_method_next_t) (objv_class_t * clazz,objv_db_cursor_t * cursor);
    
    typedef int (* objv_db_cursor_method_indexOfKey_t) (objv_class_t * clazz,objv_db_cursor_t * cursor,const char * key);
    
    typedef const char * (* objv_db_cursor_method_keyAtIndex_t) (objv_class_t * clazz,objv_db_cursor_t * cursor,int index);
    
    typedef const char * (* objv_db_cursor_method_cStringValueAtIndex_t) (objv_class_t * clazz,objv_db_cursor_t * cursor,int index,const char * defaultValue);
    
    typedef int (* objv_db_cursor_method_intValueAtIndex_t) (objv_class_t * clazz,objv_db_cursor_t * cursor,int index,int defaultValue);
    
    typedef long (* objv_db_cursor_method_longValueAtIndex_t) (objv_class_t * clazz,objv_db_cursor_t * cursor,int index,long defaultValue);
    
    typedef long long (* objv_db_cursor_method_longlongValueAtIndex_t) (objv_class_t * clazz,objv_db_cursor_t * cursor,int index,long long defaultValue);
    
    typedef double (* objv_db_cursor_method_doubleValueAtIndex_t) (objv_class_t * clazz,objv_db_cursor_t * cursor,int index,double defaultValue);
    
    typedef unsigned int (* objv_db_cursor_method_count_t) (objv_class_t * clazz,objv_db_cursor_t * cursor);
    

    objv_boolean_t objv_db_cursor_next(objv_class_t * clazz,objv_db_cursor_t * cursor);
    
    int objv_db_cursor_indexOfKey(objv_class_t * clazz,objv_db_cursor_t * cursor,const char * key);
    
    const char * objv_db_cursor_keyAtIndex (objv_class_t * clazz,objv_db_cursor_t * cursor,int index);
    
    const char * objv_db_cursor_cStringValueAtIndex (objv_class_t * clazz,objv_db_cursor_t * cursor,int index,const char * defaultValue);
    
    int objv_db_cursor_intValueAtIndex (objv_class_t * clazz,objv_db_cursor_t * cursor,int index,int defaultValue);
    
    long objv_db_cursor_longValueAtIndex (objv_class_t * clazz,objv_db_cursor_t * cursor,int index,long defaultValue);
    
    long long objv_db_cursor_longlongValueAtIndex (objv_class_t * clazz,objv_db_cursor_t * cursor,int index,long long defaultValue);
    
    double objv_db_cursor_doubleValueAtIndex (objv_class_t * clazz,objv_db_cursor_t * cursor,int index,double defaultValue);
    
    unsigned int objv_db_cursor_count (objv_class_t * clazz,objv_db_cursor_t * cursor);

    
    
#ifdef __cplusplus
}
#endif


#endif
