//
//  main.c
//  objvcloud
//
//  Created by zhang hailong on 14-2-28.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#include "objv_os.h"
#include "objv.h"
#include "objv_cloud.h"
#include "objv_json.h"
#include "objv_dispatch.h"
#include "objv_autorelease.h"
#include "objv_clcontext.h"
#include "CLAccept.h"
#include "objv_zombie.h"
#include "CLSRVProcess.h"
#include "CLLoader.h"

#define DEFAULT_PROCESS_COUNT   1

static void OBJVSRVServerLogMSG(OBJVSRVServer * srv,const char * format,va_list va){
    
    char sbuf[PATH_MAX];
    int fno;
    int len;
    
    snprintf(sbuf, sizeof(sbuf),"/var/log/%s.log",getprogname());
    
    fno = open(sbuf, O_WRONLY | O_APPEND);
    
    if(fno == -1){
        fno = open(sbuf, O_WRONLY | O_CREAT);
        if(fno != -1){
            fchmod(fno, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            close(fno);
        }
        fno = open(sbuf, O_WRONLY | O_APPEND);
    }
    
    if(fno != -1){
        
        len = vsnprintf(sbuf, sizeof(sbuf), format, va);
        
        write(fno, sbuf, len);
        
        close(fno);
    }
}

int main(int argc, char ** argv)
{
    
    OBJVSRVProcess processs[DEFAULT_PROCESS_COUNT] = {0};
    
    OBJVSRVServer srv = {
        {{argc,argv},{processs,DEFAULT_PROCESS_COUNT},{0},0},{0},OBJVSRVServerLogMSG
    };
    
#ifdef DEBUG
    srv.logCallback = NULL;
#endif
    
    {
        int i;
        for(i=0;i<srv.config.process.length;i++){
            srv.config.process.data[i].clazz = & CLSRVProcessClass;
        }
    }
    
    CLLoader();
    
    return OBJVSRVServerRun(& srv);
}

