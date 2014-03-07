//
//  objv_db_sqlite.c
//  objv
//
//  Created by zhang hailong on 14-3-7.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include <sqlite3.h>

#include "objv_os.h"
#include "objv.h"
#include "objv_db_sqlite.h"
#include "objv_autorelease.h"
#include "objv_value.h"
#include "objv_data.h"

OBJV_KEY_IMP(DBSqliteCursor)
OBJV_CLASS_DEC(DBSqliteCursor)

static objv_db_cursor_t * objv_db_sqlite_cursor_new(objv_zone_t * zone,objv_db_sqlite_t * db,sqlite3_stmt * stmt);

static void objv_db_sqlite_bind_data(objv_zone_t * zone,sqlite3_stmt * stmt,objv_object_t * data, sqlite3_destructor_type type);

OBJV_KEY_IMP(DBSqlite)

static void objv_db_sqlite_dealloc(objv_class_t * clazz, objv_object_t * object){
    
    objv_db_sqlite_t * db = (objv_db_sqlite_t *) object;
    
    if(db->sqlite){
        sqlite3_close(db->sqlite);
    }
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
}

static OBJVDBStatus objv_db_sqlite_method_exec (objv_class_t * clazz,objv_db_t * db,const char * sql,objv_object_t * data){
    
    objv_db_sqlite_t * sqlite = (objv_db_sqlite_t *) db;
    
    sqlite3_stmt * stmt;
    
    if(sqlite3_prepare_v2(sqlite->sqlite, sql, -1, &stmt, NULL) != SQLITE_OK){
        
        objv_db_set_exception_format(db, sqlite3_errcode(sqlite->sqlite), "%s",sqlite3_errmsg(sqlite->sqlite));
        
        return OBJVDBStatusException;
    }
    
    objv_db_sqlite_bind_data(db->base.zone, stmt,data,SQLITE_STATIC);
    
    int rs = sqlite3_step(stmt);
    
    sqlite3_finalize(stmt);
    
    if( rs == SQLITE_OK || rs == SQLITE_ROW || rs == SQLITE_DONE){
        return OBJVDBStatusOK;
    }
    
    objv_db_set_exception_format(db, sqlite3_errcode(sqlite->sqlite), "%s",sqlite3_errmsg(sqlite->sqlite));
    
    return OBJVDBStatusException;
}

static objv_db_cursor_t *  objv_db_sqlite_method_query (objv_class_t * clazz,objv_db_t * db,const char * sql,objv_object_t * data){
    
    objv_db_sqlite_t * sqlite = (objv_db_sqlite_t *) db;
    
    sqlite3_stmt * stmt;
    
    if(sqlite3_prepare_v2(sqlite->sqlite, sql, -1, &stmt, NULL) != SQLITE_OK){
        
        objv_db_set_exception_format(db, sqlite3_errcode(sqlite->sqlite), "%s",sqlite3_errmsg(sqlite->sqlite));
        
        return NULL;
    }
    
    objv_db_sqlite_bind_data(db->base.zone, stmt,data,SQLITE_STATIC);
    
    
    return objv_db_sqlite_cursor_new(db->base.zone,sqlite,stmt);

}

static OBJVDBStatus objv_db_sqlite_method_beginTransaction (objv_class_t * clazz,objv_db_t * db){
    return objv_db_exec(db->base.isa, db, "begin exclusive transaction", NULL);
}

static OBJVDBStatus objv_db_sqlite_method_commit (objv_class_t * clazz,objv_db_t * db){
    return objv_db_exec(db->base.isa, db, "commit transaction", NULL);
}

static OBJVDBStatus objv_db_sqlite_method_rollback (objv_class_t * clazz,objv_db_t * db){
    return objv_db_exec(db->base.isa, db, "rollback transaction", NULL);
}


OBJV_CLASS_METHOD_IMP_BEGIN(DBSqlite)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", objv_db_sqlite_dealloc)

OBJV_CLASS_METHOD_IMP(exec, "i(*,@)", objv_db_sqlite_method_exec)

OBJV_CLASS_METHOD_IMP(query, "@(*,@)", objv_db_sqlite_method_query)

OBJV_CLASS_METHOD_IMP(beginTransaction, "i()", objv_db_sqlite_method_beginTransaction)

OBJV_CLASS_METHOD_IMP(commit, "i()", objv_db_sqlite_method_commit)

OBJV_CLASS_METHOD_IMP(rollback, "i()", objv_db_sqlite_method_rollback)

OBJV_CLASS_METHOD_IMP_END(DBSqlite)

OBJV_CLASS_IMP_M(DBSqlite, OBJV_CLASS(DB), objv_db_sqlite_t)

objv_db_sqlite_t * objv_db_sqlite_alloc(objv_zone_t * zone,const char * dbPath){
    sqlite3 * sqlite = NULL;
    if(SQLITE_OK == sqlite3_open(dbPath, &sqlite)){
        objv_db_sqlite_t * db = (objv_db_sqlite_t *) objv_object_alloc(zone, OBJV_CLASS(DBSqlite),NULL);
        db->sqlite = sqlite;
    }
    return NULL;
}

objv_db_sqlite_t * objv_db_sqlite_new(objv_zone_t * zone,const char * dbPath){
    return (objv_db_sqlite_t *) objv_object_autorelease((objv_object_t *) objv_db_sqlite_alloc(zone,dbPath));
}

typedef struct _objv_db_sqlite_cursor_t {
    objv_db_cursor_t * base;
    objv_db_sqlite_t * sqlite;
    sqlite3_stmt * stmt;
} objv_db_sqlite_cursor_t;

static void objv_db_sqlite_cursor_dealloc(objv_class_t * clazz,objv_object_t * object){
    
    objv_db_sqlite_cursor_t * cursor = (objv_db_sqlite_cursor_t *) object;
    
    if(cursor->stmt){
        sqlite3_finalize(cursor->stmt);
    }
    
    objv_object_release((objv_object_t *) cursor->sqlite);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
}

OBJV_CLASS_METHOD_IMP_BEGIN(DBSqliteCursor)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", objv_db_sqlite_cursor_dealloc)

OBJV_CLASS_METHOD_IMP_END(DBSqliteCursor)

OBJV_CLASS_IMP_M(DBSqliteCursor, OBJV_CLASS(DBCursor), objv_db_sqlite_cursor_t)

static objv_db_cursor_t * objv_db_sqlite_cursor_new(objv_zone_t * zone,objv_db_sqlite_t * db,sqlite3_stmt * stmt){
    objv_db_sqlite_cursor_t  * cursor = (objv_db_sqlite_cursor_t *) objv_object_new(zone, OBJV_CLASS(DBSqliteCursor),NULL);
    if(cursor){
        cursor->sqlite = (objv_db_sqlite_t *) objv_object_retain((objv_object_t *) db);
        cursor->stmt = stmt;
    }
    return (objv_db_cursor_t *) cursor;
}

static void objv_db_sqlite_bind_data(objv_zone_t * zone,sqlite3_stmt * stmt,objv_object_t * data, sqlite3_destructor_type type){
   
    int c = sqlite3_bind_parameter_count(stmt);
    
    for(int i=1;i<=c;i++){
        
        const char * name = sqlite3_bind_parameter_name(stmt, i);
        objv_object_t * key = NULL;
        objv_object_t * v;
        
        if(name){
            key = (objv_object_t *) objv_string_new(zone, name);
        }
        else{
            key = (objv_object_t *) objv_string_new_format(zone, "@%d",i -1);
        }
        
        v = objv_object_objectValueForKey(data, key, NULL);
        
        if(v == NULL){
            sqlite3_bind_null(stmt, i);
        }
        else if(objv_object_isKindOfClass(v, OBJV_CLASS(Value))){
            {
                objv_value_t * value = (objv_value_t *) v;
                if(value->type == OBJV_TYPE(int)){
                    sqlite3_bind_int(stmt, i, value->intValue);
                }
                else if(value->type == OBJV_TYPE(uint)){
                    sqlite3_bind_int(stmt, i, value->uintValue);
                }
                else if(value->type == OBJV_TYPE(long)){
                    sqlite3_bind_int(stmt, i, value->longValue);
                }
                else if(value->type == OBJV_TYPE(ulong)){
                    sqlite3_bind_int(stmt, i, value->ulongValue);
                }
                else if(value->type == OBJV_TYPE(longLong)){
                    sqlite3_bind_int64(stmt, i, value->longLongValue);
                }
                else if(value->type == OBJV_TYPE(ulongLong)){
                    sqlite3_bind_int64(stmt, i, value->ulongLongValue);
                }
                else if(value->type == OBJV_TYPE(float)){
                    sqlite3_bind_double(stmt, i, value->floatValue);
                }
                else if(value->type == OBJV_TYPE(double)){
                    sqlite3_bind_double(stmt, i, value->doubleValue);
                }
                else if(value->type == OBJV_TYPE(boolean)){
                    sqlite3_bind_int(stmt, i, value->booleanValue ? 1 : 0);
                }
                else{
                    sqlite3_bind_null(stmt, i);
                }
            }
        }
        else if(objv_object_isKindOfClass(v, OBJV_CLASS(Data))){
            {
                objv_data_t * data = (objv_data_t *) v;
                sqlite3_bind_blob(stmt, i, data->bytes,(int) data->length, type);
            }
        }
        else if(objv_object_isKindOfClass(v, OBJV_CLASS(String))){
            {
                objv_string_t * string = (objv_string_t *) v;
                sqlite3_bind_text(stmt, i, string->UTF8String,(int) string->length, type);
            }
        }
        else {
            {
                objv_string_t * string = (objv_string_t *) objv_object_stringValue(v, NULL);
                if(string){
                    sqlite3_bind_text(stmt, i, string->UTF8String,(int) string->length, type);
                }
                else{
                    sqlite3_bind_null(stmt, i);
                }
            }
        }
        
    }
}

