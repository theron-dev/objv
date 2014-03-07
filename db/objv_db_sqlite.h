//
//  objv_db_sqlite.h
//  objv
//
//  Created by zhang hailong on 14-3-7.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_db_sqlite_h
#define objv_objv_db_sqlite_h


#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_db.h"
#include "objv_array.h"
    
    typedef struct _objv_db_sqlite_t {
        objv_db_t base;
        sqlite3 * READONLY sqlite;
    } objv_db_sqlite_t;
    
    OBJV_KEY_DEC(DBSqlite)
    OBJV_CLASS_DEC(DBSqlite)
   
    objv_db_sqlite_t * objv_db_sqlite_alloc(objv_zone_t * zone,const char * dbPath);
    
    objv_db_sqlite_t * objv_db_sqlite_new(objv_zone_t * zone,const char * dbPath);
    
#ifdef __cplusplus
}
#endif



#endif
