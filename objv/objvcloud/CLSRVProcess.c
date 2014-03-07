//
//  CLSRVProcess.c
//  objv
//
//  Created by zhang hailong on 14-3-3.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#include "objv_os.h"
#include "objv.h"
#include "objv_server.h"
#include "CLAccept.h"
#include "objv_autorelease.h"
#include "objv_json.h"
#include "CLSRVProcess.h"
#include "objv_value.h"
#include "objv_clcontext.h"
#include "objv_log.h"
#include "objv_zombie.h"

#include <execinfo.h>

static int CLSRVProcessCreate (OBJVSRVServer * server,OBJVSRVProcess * process){
    
    if(getenv(CL_ENV_CONFIG_KEY) == NULL){
        
        for (int i=1; i <server->config.arg.argc; i ++) {
            
            if(strcmp( server->config.arg.args[i] , "-config") == 0 && i + 1 < server->config.arg.argc){
                
                setenv(CL_ENV_CONFIG_KEY,server->config.arg.args[i + 1] ,0);
                
                break;
            }
            
        }
        
    }
    
    return 1;
}

static void CLSRVProcessExit (OBJVSRVServer * server,OBJVSRVProcess * process,int signo){
    OBJVSRVServerLog("\nCLSRVProcessExit pid: %d errno:%d signo:%d\n",process->pid, errno,signo);
    void * array[30];
    int size = backtrace( array, 30);
    char ** symbols = backtrace_symbols( array, size);
    for(int i=0;i<size;i++){
        OBJVSRVServerLog("%s\n",symbols[i]);
    }
    free(symbols);
}

static void CLSRVProcessKill (OBJVSRVServer * server,OBJVSRVProcess * process,int signo){
    OBJVSRVServerLog("\nCLSRVProcessKill pid: %d errno:%d signo:%d\n",process->pid, errno,signo);
    void * array[30];
    int size = backtrace( array, 30);
    char ** symbols = backtrace_symbols( array, size);
    for(int i=0;i<size;i++){
        OBJVSRVServerLog("%s\n",symbols[i]);
    }
    free(symbols);
}

static void CLSRVProcessOpen (OBJVSRVServer * server,OBJVSRVProcess * process){
    
    objv_zombie_t zombie;
    
    objv_zombie_init(& zombie, 102400);
    
    objv_zone_default_set(& zombie.zone);
    
    objv_zone_t * zone = objv_zone_default();
    
    CLAccept * ac = NULL;
    
    objv_autorelease_pool_push();
    
    objv_dispatch_set_main(objv_dispatch_get_current());
    
    objv_object_t * cfg = NULL;
    
    char * config = getenv(CL_ENV_CONFIG_KEY);
    
    if(config) {
        
        FILE * f = fopen(config, "r");
        objv_mbuf_t mbuf;
        char buf[1024];
        ssize_t len;
        
        if(f) {
            
            OBJVSRVServerLog("\nOpen Config File %s\n",config);
            
            objv_mbuf_init(& mbuf, 1024);
            
            while((len = fread(buf, 1, sizeof(buf), f)) >0){
                
                objv_mbuf_append(& mbuf, buf, len);
                
            }
            
            cfg = objv_json_decode(zone, objv_mbuf_str( & mbuf));
            
            objv_mbuf_clear(& mbuf);
            
            objv_json_encode_mbuf(zone, cfg, & mbuf, objv_true);
            
            OBJVSRVServerLog("\n%s\n",objv_mbuf_str(& mbuf));
            
            objv_mbuf_destroy(& mbuf);
            
            fclose(f);
        }
    }
    
    
    CLServiceContainer * container = (CLServiceContainer *) objv_object_new(zone, OBJV_CLASS(CLServiceContainer),NULL);
    
    CLServiceContainerSetConfig(container, cfg);
    
    CLServiceContext * ctx = (CLServiceContext *) objv_object_new(zone, OBJV_CLASS(CLServiceContext),NULL);
    
    CLServiceContextSetContainer(ctx, container);
    
    CLContextSetConfig(ctx->base.base.isa, (CLContext *)ctx,cfg);
    
    unsigned int maxThreadCount = 512;

    char * s = getenv(CL_ENV_MAX_THREAD_COUNT_KEY);
    
    if(s) {
        maxThreadCount = atoi(s);
    }
    
    if(maxThreadCount < 1){
        maxThreadCount = 1;
    }
   
    objv_dispatch_queue_t * connectQueue = objv_dispatch_queue_alloc(zone, "connectQueue", maxThreadCount);
    
    CLAcceptHandler handler = {server->run.listenSocket,server->run.listenMutex};
    
    ac = CLAcceptAllocWithHandler(zone, & handler);
    
    ac->ctx = (CLContext *) objv_object_retain((objv_object_t *) ctx);
    
    CLAcceptSetConnectQueue(ac, connectQueue);

    objv_dispatch_t * dispatch = objv_dispatch_get_current();
    
    objv_dispatch_addTask(dispatch, (objv_dispatch_task_t *) ac);

    while (1) {
        
        objv_autorelease_pool_push();
        
        objv_dispatch_run(dispatch,0.05);
        
        objv_autorelease_pool_pop();
        
    }
    
    objv_dispatch_cancelAllTasks(objv_dispatch_get_current());
    
    objv_dispatch_queue_cancelAllTasks(connectQueue);
    
    objv_object_release((objv_object_t *) connectQueue);
    
    objv_object_release((objv_object_t *) ac);
    
    objv_autorelease_pool_pop();
    
    //objv_zombie_destroy(& zombie);
    
}

static double CLSRVProcessTick (OBJVSRVServer * server,OBJVSRVProcess * process){
    return 0;
}

static void CLSRVProcessClose (OBJVSRVServer * server,OBJVSRVProcess * process){
    
}

OBJVSRVProcessClass CLSRVProcessClass = {
    CLSRVProcessCreate,
    CLSRVProcessExit,
    CLSRVProcessOpen,
    CLSRVProcessTick,
    CLSRVProcessClose,
    CLSRVProcessKill,
    0
};

