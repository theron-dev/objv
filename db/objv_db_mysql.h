//
//  objv_db_mysql.h
//  objv
//
//  Created by zhang hailong on 14-3-11.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_db_mysql_h
#define objv_objv_db_mysql_h


#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_db.h"
#include "objv_array.h"
#include "objv_url.h"
    
    struct st_mysql;
    
    typedef struct _objv_db_mysql_t {
        objv_db_t base;
        objv_string_t * db;
        struct st_mysql * READONLY mysql;
        objv_mutex_t READONLY mutex;
    } objv_db_mysql_t;
    
    OBJV_KEY_DEC(DBMysql)
    OBJV_CLASS_DEC(DBMysql)
    
    objv_db_mysql_t * objv_db_mysql_alloc(objv_zone_t * zone,objv_url_t * url, objv_string_t * user,objv_string_t * password);
    
    objv_db_mysql_t * objv_db_mysql_new(objv_zone_t * zone,objv_url_t * url, objv_string_t * user,objv_string_t * password);
    
    
#ifdef __cplusplus
}
#endif


#endif
