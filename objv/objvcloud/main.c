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
#include "objv_log.h"

#define DEFAULT_PROCESS_COUNT   1


int main(int argc, char ** argv)
{
    
    OBJVSRVProcess processs[DEFAULT_PROCESS_COUNT] = {0};
    
    OBJVSRVServer srv = {
        {{argc,argv},{processs,DEFAULT_PROCESS_COUNT},{0},0},{0}
    };
    
#ifndef DEBUG
    {
        int fno = open("/var/log/objvcloud.log", O_WRONLY | O_CREAT);
        
        if(fno != -1){
            
            fchmod(fno, 0777);
            
            objv_log_stdout(fno);
            
        }
        
    }
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

