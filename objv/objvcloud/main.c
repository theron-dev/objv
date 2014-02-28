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

int main(int argc, const char * argv[])
{

    const char * cfgp = NULL;
    
    if(argc > 1){
        cfgp = argv[1];
    }
    
    objv_zombie_t zombie;
    
    objv_zombie_init(& zombie, 102400);
    
    objv_zone_t * zone = (objv_zone_t *) & zombie;
    CLAccept * ac = NULL;
    
    objv_autorelease_pool_push();
    
    objv_dispatch_set_main(objv_dispatch_get_current());

    objv_object_t * cfg = NULL;
    
    if(cfgp) {
        FILE * f = fopen(cfgp, "r");
        objv_mbuf_t mbuf;
        char buf[1024];
        ssize_t len;
        
        if(f) {
            
            objv_mbuf_init(& mbuf, 1024);
            
            while((len = fread(buf, 1, sizeof(buf), f)) >0){
                
                objv_mbuf_append(& mbuf, buf, len);
                
            }
            
            cfg = objv_json_decode(zone, objv_mbuf_str( & mbuf));
            
            objv_mbuf_destroy(& mbuf);
            
            fclose(f);
        }
    }
    

    CLServiceContainer * container = (CLServiceContainer *) objv_object_new(zone, OBJV_CLASS(CLServiceContainer),NULL);
    
    CLServiceContainerSetConfig(container, cfg);
    
    CLServiceContext * ctx = (CLServiceContext *) objv_object_new(zone, OBJV_CLASS(CLServiceContext),NULL);
    
    CLServiceContextSetContainer(ctx, container);
    
  
    ac = CLAcceptAlloc(zone, 0);
    
    if(ac == NULL){
        
        printf("\nListener Error \n");
        
        goto toexit;
        
    }
    
    printf("\nPORT:%d\n",ac->port);
    
    objv_dispatch_addTask(objv_dispatch_get_current(), (objv_dispatch_task_t *) ac);
    
    objv_timeinval_t t = objv_timestamp();
    
    while (1) {
        
        objv_autorelease_pool_push();
        
        objv_dispatch_run(objv_dispatch_get_current(),0.2);
        
        objv_autorelease_pool_pop();
        
        if(objv_timestamp() - t > 10){
            break;
        }
    }
    
    
toexit:
    
    objv_dispatch_cancelAllTasks(objv_dispatch_get_current());
    
    objv_object_release((objv_object_t *) ac);
    
    objv_autorelease_pool_pop();
    
    objv_zombie_destroy(& zombie);
    
    return 0;
}

