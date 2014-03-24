//
//  objv_server.h
//  objv
//
//  Created by zhang hailong on 14-2-24.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_server_h
#define objv_objv_server_h


#ifdef __cplusplus
extern "C" {
#endif
    
#include "objv_os.h"
    
    
    struct _OBJVSRVProcess;
    struct _OBJVSRVServer;
    
    typedef struct _OBJVSRVServer {
        struct {
            struct {
                int argc;
                char ** args;
            } arg;
            struct {
                struct _OBJVSRVProcess * data;
                unsigned int length;
            } process;
            struct {
                int port;
            } socket;
            int isBackgroundRun;
        } config;
        struct {
            int mainpid;
            int port;
            int listenSocket;
            objv_mutex_t listenMutex;
            objv_mutex_t stdoutMutex;
        } run;
    } OBJVSRVServer;
    
    int OBJVSRVServerRun(OBJVSRVServer * server);
    
    int OBJVSRVServerAccept(OBJVSRVServer * server,double timeout,struct sockaddr * addr,socklen_t * socklen);
    
    struct _OBJVSRVProcess;
    
    typedef int (* OBJVSRVProcessCreate) (OBJVSRVServer * server,struct _OBJVSRVProcess * process);
    
    typedef void (* OBJVSRVProcessExit) (OBJVSRVServer * server,struct _OBJVSRVProcess * process,int signo);
    
    typedef void (* OBJVSRVProcessOpen) (OBJVSRVServer * server,struct _OBJVSRVProcess * process);
    
    typedef double (* OBJVSRVProcessTick) (OBJVSRVServer * server,struct _OBJVSRVProcess * process);
    
    typedef void (* OBJVSRVProcessClose) (OBJVSRVServer * server,struct _OBJVSRVProcess * process);
    
    typedef void (* OBJVSRVProcessKill) (OBJVSRVServer * server,struct _OBJVSRVProcess * process,int signo);
    
    typedef struct _OBJVSRVProcessClass{
        OBJVSRVProcessCreate create;
        OBJVSRVProcessExit exit;
        OBJVSRVProcessOpen open;
        OBJVSRVProcessTick tick;
        OBJVSRVProcessClose close;
        OBJVSRVProcessKill kill;
        void * userInfo;
    } OBJVSRVProcessClass;
    
    typedef struct _OBJVSRVProcess {
        const OBJVSRVProcessClass * clazz;
        int pid;
        int exit;
        void * userInfo;
    } OBJVSRVProcess;
    
#ifdef __cplusplus
}
#endif


#endif
