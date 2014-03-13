//
//  objv_db_mysql.c
//  objv
//
//  Created by zhang hailong on 14-3-11.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include <mysql.h>

#include "objv_os.h"
#include "objv.h"
#include "objv_db_mysql.h"
#include "objv_autorelease.h"
#include "objv_value.h"
#include "objv_data.h"
#include "objv_log.h"

OBJV_KEY_IMP(DBMysql)

OBJV_KEY_IMP(DBMysqlCursor)
OBJV_CLASS_DEC(DBMysqlCursor)

static objv_db_cursor_t * objv_db_mysql_cursor_new(objv_zone_t * zone,objv_db_mysql_t * db,MYSQL_RES * res);

static void objv_db_mysql_sql(objv_zone_t * zone,objv_db_mysql_t *mysql,const char * sql,objv_object_t * data,objv_mbuf_t * mbuf);

static void objv_db_mysql_sql_propertyType(objv_zone_t * zone,objv_db_mysql_t *mysql,objv_property_t * property,objv_mbuf_t * mbuf);

static void objv_db_mysql_dealloc(objv_class_t * clazz, objv_object_t * object){
    
    objv_db_mysql_t * db = (objv_db_mysql_t *) object;

    objv_mutex_lock( & db->mutex);
    
    if(db->mysql){
        mysql_close(db->mysql);
        db->mysql = NULL;
    }

    objv_mutex_unlock( & db->mutex);
    
    objv_mutex_destroy(& db->mutex);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
}

static objv_object_t * objv_db_mysql_initv(objv_class_t * clazz,objv_object_t * object,va_list ap){

    if(clazz->superClass){
        object = objv_object_initv(clazz->superClass,object,ap);
    }
    
    if(object){
        
        objv_db_mysql_t * mysql = (objv_db_mysql_t *) object;
        
        objv_mutex_init( & mysql->mutex);
        
    }
    
    return object;
}

static OBJVDBStatus objv_db_mysql_method_exec (objv_class_t * clazz,objv_db_t * db,const char * sql,objv_object_t * data){
    
    objv_db_mysql_t * mysql = (objv_db_mysql_t *) db;
    
    objv_mbuf_t mbuf;
    
    objv_mbuf_init(& mbuf,128);
    
    objv_db_mysql_sql(mysql->base.base.zone,mysql,sql,data,& mbuf);
    
    objv_mutex_lock( & mysql->mutex);
    
    if(! mysql_query(mysql->mysql,objv_mbuf_str(& mbuf))){
        
        objv_db_set_exception_format(db, mysql_errno(mysql->mysql), "%s",mysql_error(mysql->mysql));
        
        objv_mutex_unlock( & mysql->mutex);
        
        objv_mbuf_destroy(& mbuf);
        
        return OBJVDBStatusException;
    }
    
    MYSQL_RES * res = mysql_store_result(mysql->mysql);
       
    if(res == NULL){
        
        objv_db_set_exception_format(db, mysql_errno(mysql->mysql), "%s",mysql_error(mysql->mysql));
        
        objv_mutex_unlock( & mysql->mutex);
        
        objv_mbuf_destroy(& mbuf);
        
        return OBJVDBStatusException;
    }
    
    mysql_free_result(res);
    
    objv_mutex_unlock( & mysql->mutex);
    
    objv_mbuf_destroy(& mbuf);
    
    return OBJVDBStatusOK;
}

static objv_db_cursor_t *  objv_db_mysql_method_query (objv_class_t * clazz,objv_db_t * db,const char * sql,objv_object_t * data){
    
    objv_db_mysql_t * mysql = (objv_db_mysql_t *) db;
    
    objv_mbuf_t mbuf;
    
    objv_mbuf_init(& mbuf,128);
    
    objv_db_mysql_sql(mysql->base.base.zone,mysql,sql,data,& mbuf);
    
    objv_mutex_lock( & mysql->mutex);
    
    if(! mysql_query(mysql->mysql,objv_mbuf_str(& mbuf))){
        
        objv_db_set_exception_format(db, mysql_errno(mysql->mysql), "%s",mysql_error(mysql->mysql));
        
        objv_mutex_unlock( & mysql->mutex);
        
        objv_mbuf_destroy(& mbuf);
        
        return NULL;
    }
    
    MYSQL_RES * res = mysql_store_result(mysql->mysql);
    
    if(res == NULL){
        
        objv_db_set_exception_format(db, mysql_errno(mysql->mysql), "%s",mysql_error(mysql->mysql));
        
        objv_mutex_unlock( & mysql->mutex);
        
        objv_mbuf_destroy(& mbuf);
        
        return NULL;
    }
    
    objv_mutex_unlock( & mysql->mutex);
    
    objv_mbuf_destroy(& mbuf);
    

    return objv_db_mysql_cursor_new(mysql->base.base.zone,mysql,res);
}

static OBJVDBStatus objv_db_mysql_method_beginTransaction (objv_class_t * clazz,objv_db_t * db){
    return objv_db_exec(db->base.isa, db, "BEGIN", NULL);
}

static OBJVDBStatus objv_db_mysql_method_commit (objv_class_t * clazz,objv_db_t * db){
    return objv_db_exec(db->base.isa, db, "COMMIT", NULL);
}

static OBJVDBStatus objv_db_mysql_method_rollback (objv_class_t * clazz,objv_db_t * db){
    return objv_db_exec(db->base.isa, db, "ROLLBACK", NULL);
}

static OBJVDBStatus objv_db_mysql_method_object_regClass (objv_class_t * clazz,objv_db_mysql_t * db,objv_class_t * dbObjectClass){
    
    objv_mbuf_t mbuf;
    objv_db_cursor_t * cursor;
    objv_zone_t * zone = db->base.base.zone;
    objv_dictionary_t * fields = NULL;
    const char * v;
    objv_object_t * key;
    objv_property_t * rowid;
    objv_property_t * prop;
    unsigned int propCount;
    OBJVDBStatus status = OBJVDBStatusOK;
    objv_class_t * tableClass = dbObjectClass;
    objv_boolean_t isExists = objv_false;
    objv_mbuf_init(& mbuf, 128);
    
    objv_mbuf_format(& mbuf, "SELECT COUNT(*) FROM `INFORMATION_SCHEMA`.`TABLES` WHERE `TABLE_SCHEMA`='%s' and `TABLE_NAME`='%s';",db->db->UTF8String,dbObjectClass->name);
    
    cursor = objv_db_query(clazz, (objv_db_t *) db, objv_mbuf_str(& mbuf), NULL);
    
    if (cursor && objv_db_cursor_next(cursor->base.isa, cursor)) {
        
        isExists = objv_db_cursor_intValueAtIndex(cursor->base.isa, cursor, 0, 0) > 0;
        
    }
    
    if(!isExists){
        
        fields = objv_dictionary_alloc(zone, 4);
        
        objv_mbuf_clear(& mbuf);
        objv_mbuf_format(& mbuf, "SELECT COLUMN_NAME FROM `INFORMATION_SCHEMA`.`COLUMNS` WHERE `TABLE_SCHEMA`='%s' AND `TABLE_TABLE`='%s';",db->db->UTF8String,dbObjectClass->name);
        
        cursor = objv_db_query(clazz, (objv_db_t *) db, objv_mbuf_str(& mbuf), NULL);
        
        while (cursor && objv_db_cursor_next(cursor->base.isa, cursor)) {
            
            v = objv_db_cursor_cStringValueAtIndex(cursor->base.isa, cursor, 0, NULL);
            
            if(v){
                
                key = (objv_object_t *) objv_string_new(zone, v);
                
                objv_dictionary_setValue(fields, key, key);
                
            }
            
        }
        
        rowid = objv_class_getPropertyOfClass(tableClass, ("rowid"),NULL);
        
        while(tableClass){
            
            prop = tableClass->propertys;
            propCount = tableClass->propertyCount;
            
            while(prop && propCount >0){
                
                if(rowid != prop && prop->serialization){
                    
                    key = (objv_object_t *) objv_string_new_nocopy(zone, prop->name);
                    
                    if(objv_dictionary_value(fields, key) == NULL){
                        
                        objv_mbuf_clear(& mbuf);
                        
                        objv_mbuf_format(& mbuf, "ALTER TABLE `%s` ADD COLUMN `%s` ",dbObjectClass->name,prop->name);
                        
                        objv_db_mysql_sql_propertyType(zone,db,prop,& mbuf);
                        
                        status = objv_db_exec(clazz, (objv_db_t *) db, objv_mbuf_str(& mbuf), NULL);

                    }
                    
                }
                
                prop ++;
                propCount --;
            }
            
            tableClass = tableClass->superClass;
        }

            
        objv_object_release((objv_object_t *) fields);
    }
    
    objv_mbuf_destroy(& mbuf);
    
    return status;
}

OBJV_CLASS_METHOD_IMP_BEGIN(DBMysql)

OBJV_CLASS_METHOD_IMP(init, "@(*)", objv_db_mysql_initv)
       
OBJV_CLASS_METHOD_IMP(dealloc, "v()", objv_db_mysql_dealloc)

OBJV_CLASS_METHOD_IMP(exec, "i(*,@)", objv_db_mysql_method_exec)

OBJV_CLASS_METHOD_IMP(query, "@(*,@)", objv_db_mysql_method_query)

OBJV_CLASS_METHOD_IMP(beginTransaction, "i()", objv_db_mysql_method_beginTransaction)

OBJV_CLASS_METHOD_IMP(commit, "i()", objv_db_mysql_method_commit)

OBJV_CLASS_METHOD_IMP(rollback, "i()", objv_db_mysql_method_rollback)

OBJV_CLASS_METHOD_IMP(objectRegClass, "i(*)", objv_db_mysql_method_object_regClass)

OBJV_CLASS_METHOD_IMP_END(DBMysql)

OBJV_CLASS_IMP_M(DBMysql, OBJV_CLASS(DB), objv_db_mysql_t)

objv_db_mysql_t * objv_db_mysql_alloc(objv_zone_t * zone,objv_url_t * url, objv_string_t * user,objv_string_t * password){
    
    if(url && user && password){
        
        MYSQL * mysql = mysql_init(NULL);
        
        if(mysql){
            
            if(mysql_real_connect(mysql,url->domain->UTF8String,user->UTF8String,password->UTF8String,url->path->UTF8String + 1,url->port ? 1433: atoi(url->port->UTF8String),NULL,0)){
                objv_db_mysql_t * m = (objv_db_mysql_t *) objv_object_alloc(zone,OBJV_CLASS(DBMysql),NULL);
                objv_string_t * charset = objv_object_stringValueForKey((objv_object_t *)url->queryValues,(objv_object_t *) objv_string_new_nocopy(zone,"charset"),NULL);
                if(charset){
                    mysql_set_character_set(mysql,charset->UTF8String);
                }
                m->mysql = mysql;
                m->db = objv_string_alloc(zone, url->path->UTF8String + 1);
                return m;
            }
            else{
                mysql_close(mysql);
                objv_log("\nmysql_real_connect error.\n%s\n",url->absoluteString->UTF8String);
            }
        }

    }

    return NULL;
}

objv_db_mysql_t * objv_db_mysql_new(objv_zone_t * zone,objv_url_t * url, objv_string_t * user,objv_string_t * password){
    return (objv_db_mysql_t *) objv_object_autorelease(( objv_object_t *) objv_db_mysql_alloc(zone,url,user,password));
}

typedef struct _objv_db_mysql_cursor_t {
    objv_db_cursor_t * base;
    objv_db_mysql_t * db;
    MYSQL_RES * res;
    MYSQL_ROW row;
    objv_hash_map_t * keys;
} objv_db_mysql_cursor_t;

static void objv_db_mysql_cursor_dealloc(objv_class_t * clazz,objv_object_t * object){
    
    objv_db_mysql_cursor_t * cursor = (objv_db_mysql_cursor_t *) object;
    
    if(cursor->res){
        mysql_free_result(cursor->res);
    }
    
    objv_hash_map_dealloc(cursor->keys);
    
    objv_object_release((objv_object_t *) cursor->db);
    
    if(clazz->superClass){
        objv_object_dealloc(clazz->superClass, object);
    }
}

static objv_boolean_t objv_db_mysql_cursor_method_next(objv_class_t * clazz,objv_db_mysql_cursor_t * cursor){
    cursor->row = mysql_fetch_row(cursor->res);
    return cursor->row ? objv_true : objv_false;
}

static int objv_db_mysql_cursor_method_indexOfKey(objv_class_t * clazz,objv_db_mysql_cursor_t * cursor,const char * key){
    
    MYSQL_FIELD * fd = (MYSQL_FIELD *) objv_hash_map_get(cursor->keys,(void *) key);
    
    if(fd){
        return (int) (fd - cursor->res->fields);
    }
    
    return -1;
}

static const char * objv_db_mysql_cursor_method_keyAtIndex(objv_class_t * clazz,objv_db_mysql_cursor_t * cursor,int index){
    if(index >= 0 && index < mysql_num_fields(cursor->res)){
        return cursor->res->fields[index].name;
    }
    return NULL;
}

static const char * objv_db_mysql_cursor_method_cStringValueAtIndex(objv_class_t * clazz,objv_db_mysql_cursor_t * cursor,int index,const char * defaultValue){
    if(index >= 0 && index < mysql_num_fields(cursor->res) && cursor->row){
        return cursor->row[index];
    }
    return defaultValue;
}

static int objv_db_mysql_cursor_method_intValueAtIndex(objv_class_t * clazz,objv_db_mysql_cursor_t * cursor,int index,int defaultValue){
    if(index >= 0 && index < mysql_num_fields(cursor->res) && cursor->row){
        return atoi(cursor->row[index]);
    }
    return defaultValue;
}

static long objv_db_mysql_cursor_method_longValueAtIndex(objv_class_t * clazz,objv_db_mysql_cursor_t * cursor,int index,long defaultValue){
    if(index >= 0 && index < mysql_num_fields(cursor->res) && cursor->row){
        return atol(cursor->row[index]);
    }
    return defaultValue;
}

static long long objv_db_mysql_cursor_method_longlongValueAtIndex(objv_class_t * clazz,objv_db_mysql_cursor_t * cursor,int index,long long defaultValue){
    if(index >= 0 && index < mysql_num_fields(cursor->res) && cursor->row){
        return atoll(cursor->row[index]);
    }
    return defaultValue;
}

static double objv_db_mysql_cursor_method_doubleValueAtIndex(objv_class_t * clazz,objv_db_mysql_cursor_t * cursor,int index,double defaultValue){
    if(index >= 0 && index < mysql_num_fields(cursor->res) && cursor->row){
        return atof(cursor->row[index]);
    }
    return defaultValue;
}

static unsigned int objv_db_mysql_cursor_method_count(objv_class_t * clazz,objv_db_mysql_cursor_t * cursor){
    return (unsigned int) mysql_num_rows(cursor->res);
}


OBJV_CLASS_METHOD_IMP_BEGIN(DBMysqlCursor)

OBJV_CLASS_METHOD_IMP(dealloc, "v()", objv_db_mysql_cursor_dealloc)

OBJV_CLASS_METHOD_IMP(next, "b()", objv_db_mysql_cursor_method_next)

OBJV_CLASS_METHOD_IMP(indexOfKey, "i(*)", objv_db_mysql_cursor_method_indexOfKey)

OBJV_CLASS_METHOD_IMP(keyAtIndex, "*(i)", objv_db_mysql_cursor_method_keyAtIndex)

OBJV_CLASS_METHOD_IMP(cStringValueAtIndex, "*(i)", objv_db_mysql_cursor_method_cStringValueAtIndex)

OBJV_CLASS_METHOD_IMP(intValueAtIndex, "i(i)", objv_db_mysql_cursor_method_intValueAtIndex)

OBJV_CLASS_METHOD_IMP(longValueAtIndex, "l(i)", objv_db_mysql_cursor_method_longValueAtIndex)

OBJV_CLASS_METHOD_IMP(longlongValueAtIndex, "q(i)", objv_db_mysql_cursor_method_longlongValueAtIndex)

OBJV_CLASS_METHOD_IMP(doubleValueAtIndex, "d(i)", objv_db_mysql_cursor_method_doubleValueAtIndex)

OBJV_CLASS_METHOD_IMP(count, "I()", objv_db_mysql_cursor_method_count)

OBJV_CLASS_METHOD_IMP_END(DBMysqlCursor)

OBJV_CLASS_IMP_M(DBMysqlCursor, OBJV_CLASS(DBCursor), objv_db_mysql_cursor_t)

static objv_db_cursor_t * objv_db_mysql_cursor_new(objv_zone_t * zone,objv_db_mysql_t * db,MYSQL_RES * res){
    objv_db_mysql_cursor_t * cursor = (objv_db_mysql_cursor_t *) objv_object_new(zone,OBJV_CLASS(DBMysqlCursor),NULL);
    MYSQL_FIELD * fd;
    cursor->db = (objv_db_mysql_t *) objv_object_retain((objv_object_t *) db);
    cursor->res = res;
    cursor->keys = objv_hash_map_alloc(4,objv_hash_map_hash_code_string,objv_map_compare_string);

    while((fd = mysql_fetch_field(res))){
        objv_hash_map_put(cursor->keys,fd->name,fd);
    }
    
    return (objv_db_cursor_t *) cursor;
}

static void objv_db_mysql_sql(objv_zone_t * zone,objv_db_mysql_t *mysql,const char * sql,objv_object_t * data,objv_mbuf_t * mbuf){
    int s = 0;
    char * p = (char *) sql;
    objv_mbuf_t key;
    
    objv_mbuf_init(& key,16);
    
    while( p ){
        
        if(s == 0){
            if(* p == 0){
                break;
            }
            else if(* p == ':'){
                s = 1;
                objv_mbuf_clear(& key);
            }
            else{
                objv_mbuf_append(mbuf,p,1);
            }
        }
        else if(s == 1){
            if( (*p >= 'a' && *p <='z') || (*p >= 'A' && *p <='Z') || (*p >= '0' && *p <='9') || *p == '_' ){
                objv_mbuf_append(& key,p,1);
            }
            else{
                if(key.length){
                    {
                        objv_object_t * v = objv_object_objectValueForKey(data,(objv_object_t *) objv_string_new(zone,objv_mbuf_str(& key)),NULL);
                        
                        if(v == NULL){
                            objv_mbuf_format(mbuf,"NULL");
                        }
                        else if(objv_object_isKindOfClass(v, OBJV_CLASS(Value))){
                            {
                                objv_value_t * value = (objv_value_t *) v;
                                if(value->type == OBJV_TYPE(int)){
                                    objv_mbuf_format(mbuf,"%d",value->intValue);
                                }
                                else if(value->type == OBJV_TYPE(uint)){
                                    objv_mbuf_format(mbuf,"%u",value->uintValue);
                                }
                                else if(value->type == OBJV_TYPE(long)){
                                    objv_mbuf_format(mbuf,"%ld",value->longValue);
                                }
                                else if(value->type == OBJV_TYPE(ulong)){
                                    objv_mbuf_format(mbuf,"%lu",value->ulongValue);
                                }
                                else if(value->type == OBJV_TYPE(longLong)){
                                    objv_mbuf_format(mbuf,"%lld",value->longLongValue);
                                }
                                else if(value->type == OBJV_TYPE(ulongLong)){
                                    objv_mbuf_format(mbuf,"%llu",value->ulongLongValue);
                                }
                                else if(value->type == OBJV_TYPE(float)){
                                    objv_mbuf_format(mbuf,"%f",value->floatValue);
                                }
                                else if(value->type == OBJV_TYPE(double)){
                                    objv_mbuf_format(mbuf,"%lf",value->doubleValue);
                                }
                                else if(value->type == OBJV_TYPE(boolean)){
                                    objv_mbuf_format(mbuf,"%d",value->booleanValue ? 1 : 0);
                                }
                                else{
                                    objv_mbuf_format(mbuf,"NULL");
                                }
                            }
                        }
                        else if(objv_object_isKindOfClass(v, OBJV_CLASS(Data))){
                            {
                                objv_data_t * data = (objv_data_t *) v;
                                
                                objv_mbuf_append(mbuf,"'",1);
                                objv_mbuf_extend(mbuf,mbuf->length + data->length + 32);
                                mbuf->length += mysql_real_escape_string(mysql->mysql,mbuf->data + mbuf->length,data->bytes,data->length);
                                objv_mbuf_append(mbuf,"'",1);
                            }
                        }
                        else if(objv_object_isKindOfClass(v, OBJV_CLASS(String))){
                            {
                                objv_string_t * string = (objv_string_t *) v;
                                
                                objv_mbuf_append(mbuf,"'",1);
                                objv_mbuf_extend(mbuf,mbuf->length + string->length + 32);
                                mbuf->length += mysql_real_escape_string(mysql->mysql,mbuf->data + mbuf->length,string->UTF8String,string->length);
                                objv_mbuf_append(mbuf,"'",1);
                            
                            }
                        }
                        else {
                            {
                                objv_string_t * string = (objv_string_t *) objv_object_stringValue(v, NULL);
                                if(string){
                                    objv_mbuf_append(mbuf,"'",1);
                                    objv_mbuf_extend(mbuf,mbuf->length + string->length + 32);
                                    mbuf->length += mysql_real_escape_string(mysql->mysql,mbuf->data + mbuf->length,string->UTF8String,string->length);
                                    objv_mbuf_append(mbuf,"'",1);
                                }
                                else{
                                    objv_mbuf_format(mbuf,"NULL");
                                }
                            }
                        }
                    }
                    
                }
                
                s = 0;
                
                if(*p != 0){
                    objv_mbuf_append(mbuf,p,1);
                }
            }
        }
        
        if(* p == 0){
            break;
        }
        p ++;
    }
    
    objv_mbuf_destroy(& key);
}

static void objv_db_mysql_sql_propertyType(objv_zone_t * zone,objv_db_mysql_t *mysql,objv_property_t * property,objv_mbuf_t * mbuf){
    if(property->type == OBJV_TYPE(int) || property->type == OBJV_TYPE(uint) || property->type == OBJV_TYPE(long) || property->type == OBJV_TYPE(ulong)){
        objv_mbuf_format(mbuf, "INT(11)");
    }
    else if(property->type == OBJV_TYPE(longLong) || property->type == OBJV_TYPE(ulongLong)){
        objv_mbuf_format(mbuf, "BIGINT");
    }
    else if(property->type == OBJV_TYPE(double) || property->type == OBJV_TYPE(float)){
        objv_mbuf_format(mbuf, "DOUBLE");
    }
    else if(property->type == OBJV_TYPE(boolean)){
        objv_mbuf_format(mbuf, "INT(1)");
    }
    else if(property->type == OBJV_TYPE(boolean)){
        objv_mbuf_format(mbuf, "INT(1)");
    }
    else if(property->type == OBJV_TYPE(object)) {
        objv_mbuf_format(mbuf, "TEXT");
    }
    else{
        objv_mbuf_format(mbuf, "VARCHAR(45)");
    }
}
