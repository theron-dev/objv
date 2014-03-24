//
//  objv_server.c
//  objv
//
//  Created by zhang hailong on 14-2-24.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#include "objv_os.h"
#include "objv.h"
#include "objv_server.h"
#include "objv_log.h"

#include <execinfo.h>

static OBJVSRVServer * gServer = NULL;
static void OBJVSRVServerRunProcess(OBJVSRVProcess * process);
static void OBJVSRVServerSIGProcessQuit(int signo);
static void OBJVSRVServerSIGNAN(int signo);
static int processExit;
static OBJVSRVProcess * gProcess = NULL;
static void OBJVSRVServerSIGProcessKill(int signo);

int OBJVSRVServerRun(OBJVSRVServer * server){
    
    int i;
    
    gServer = server;
    
    for(i=1;i<server->config.arg.argc;i++){
        if(strcmp(server->config.arg.args[i], "-p") ==0 && i + 1 <server->config.arg.argc){
            i ++;
            if(server->config.socket.port == 0){
                server->config.socket.port = atoi(server->config.arg.args[i]);
            }
        }
        else if(strcmp(server->config.arg.args[i], "-b") ==0){
            server->config.isBackgroundRun = 1;
        }
    }
    
    if(server->config.isBackgroundRun){
        {
            pid_t pid;
            if(( pid = fork() ) < 0 )
            {
                fprintf(stderr, "fork error\n");
                exit(EXIT_FAILURE);
            }
            else if(pid == 0)
            {
                
            }
            else{
                exit(EXIT_SUCCESS);
            }
        }
    }
    
    server->run.mainpid = getpid();
    
    objv_mutex_init(&server->run.stdoutMutex);
    
    objv_log("OBJVSRVServerRun(%d) begin",server->run.mainpid);
    
    
    server->run.listenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    objv_mutex_init(&server->run.listenMutex);
    
    {
        int res;
        struct sockaddr_in addr ;
        socklen_t socklen = sizeof(struct sockaddr_in);
        int fl;
        int fn = 1;
        
        memset(&addr, 0, sizeof(struct sockaddr_in));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(server->config.socket.port);
        addr.sin_addr.s_addr = INADDR_ANY;
        
        res = bind(server->run.listenSocket, (struct sockaddr *) & addr, sizeof(struct sockaddr_in));
        
        if(res != 0){
            fprintf(stderr,"socket bind error\n");
            close(server->run.listenSocket);
            exit(EXIT_FAILURE);
        }
        
        getsockname(server->run.listenSocket, (struct sockaddr *) & addr, &socklen);
        
        server->run.port = ntohs(addr.sin_port);
        
        objv_log("bind %d\n",server->run.port);
        
        res = listen(server->run.listenSocket, SOMAXCONN);
        
        if(res != 0){
            fprintf(stderr,"socket listen error\n");
            close(server->run.listenSocket);
            exit(EXIT_FAILURE);
        }
        
        fl =  fcntl(server->run.listenSocket, F_GETFL) ;
        fcntl(server->run.listenSocket, F_SETFL, fl | O_NONBLOCK);
        
        setsockopt(server->run.listenSocket, SOL_SOCKET, SO_RCVLOWAT, (void *)&fn, sizeof(fn));
        setsockopt(server->run.listenSocket, SOL_SOCKET, SO_SNDLOWAT, (void *)&fn, sizeof(fn));
        
    }
    
    signal(SIGCHLD, OBJVSRVServerSIGProcessQuit);
    signal(SIGINT, OBJVSRVServerSIGProcessQuit);
    signal(SIGTERM, OBJVSRVServerSIGProcessQuit);
    signal(SIGKILL, OBJVSRVServerSIGProcessQuit);
    signal(SIGABRT, OBJVSRVServerSIGProcessQuit);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTTOU, OBJVSRVServerSIGNAN);
    signal(ETIMEDOUT, OBJVSRVServerSIGNAN);
    
    signal(SIGINT, OBJVSRVServerSIGProcessQuit);
    signal(SIGQUIT, OBJVSRVServerSIGProcessQuit);
    signal(SIGBUS, OBJVSRVServerSIGProcessQuit);
    signal(SIGSEGV, OBJVSRVServerSIGProcessQuit);
    signal(SIGSYS, OBJVSRVServerSIGProcessQuit);
    signal(SIGTERM, OBJVSRVServerSIGProcessQuit);
    signal(SIGURG, OBJVSRVServerSIGProcessQuit);
    signal(SIGXCPU, OBJVSRVServerSIGProcessQuit);
    signal(SIGXFSZ, OBJVSRVServerSIGProcessQuit);
    
    {
        unsigned int c = server->config.process.length;
        OBJVSRVProcess * p = server->config.process.data;
        
        while(c > 0 && p->clazz){
            
            OBJVSRVServerRunProcess(p);
            
            p++;
            c --;
        }
    }
    
    {
        char command[128] = "";
        fd_set rds;
        int res;
        struct timeval timeo = {30,0};
        
        while(strcmp(command, "exit")){
            
            memset(command, 0, sizeof(command));
            
            if(server->config.isBackgroundRun){
                usleep(300000);
            }
            
            FD_ZERO(&rds);
            
            FD_SET(STDIN_FILENO, &rds);
            
            res = select(STDIN_FILENO + 1, &rds, NULL, NULL, &timeo);
            
            if(res == 0){
                
            }
            else if(res == -1){
                if(errno == EINTR){
                    
                }
                else{
                    break;
                }
            }
            else{
                if(FD_ISSET(STDIN_FILENO, &rds)){
                    
                    scanf("%s",command);
                    
                }
            }
            
            
        }
    }
    
    signal(SIGCHLD, SIG_IGN);
    
    {
        unsigned int c = server->config.process.length;
        OBJVSRVProcess * p = server->config.process.data;
        int     stat;
        
        while(c > 0 && p->clazz){
            
            if(p->pid){
                kill(p->pid, SIGKILL);
                waitpid(p->pid, &stat, WNOHANG);
                (* p->clazz->exit)(server,p,0);
                p->pid = 0;
            }
            
            p++;
            c --;
        }
    }
    
    
    close(server->run.listenSocket);
    objv_mutex_destroy(&server->run.listenMutex);

    objv_log("main end");
    
    objv_mutex_destroy(&server->run.stdoutMutex);
    
    gServer = NULL;
    
    return 0;
}


static void OBJVSRVServerSIGNAN(int signo){
    
    objv_log("NAN sig:%d errno:%d",signo,errno);
    
    void * array[30];
    int size = backtrace( array, 30);
    char ** symbols = backtrace_symbols( array, size);
    for(int i=0;i<size;i++){
        objv_log("%s",symbols[i]);
    }
    free(symbols);
}

static void OBJVSRVServerSIGProcessQuit(int signo)
{
    int c = 0;
    int     stat;
    OBJVSRVProcess *p;
    if(signo == SIGCHLD){
        if(getpid() == gServer->run.mainpid){
            c = gServer->config.process.length;
            p = gServer->config.process.data;
            while(c > 0 && p->clazz){
                if(p->pid){
                    if(p->pid == waitpid(p->pid, &stat, WNOHANG)){
                        (* p->clazz->exit)(gServer,p,signo);
                        p->pid = 0;
                        OBJVSRVServerRunProcess(p);
                    }
                }
                else{
                    OBJVSRVServerRunProcess(p);
                }
                
                p++;
                c --;
            }
        }
    }
    else{
        if(getpid() == gServer->run.mainpid){
            c = gServer->config.process.length;
            p = gServer->config.process.data;
            while(c > 0 && p->clazz){
                
                if(p->pid){
                    kill(p->pid, SIGKILL);
                    waitpid(p->pid, &stat, WNOHANG);
                    (* p->clazz->exit)(gServer,p,signo);
                    p->pid = 0;
                }
                
                p++;
                c --;
            }
            exit(EXIT_FAILURE);
        }
        else {
            objv_log("process(%d) exit",getpid());
            processExit = 1;
        }
    }
}

static void OBJVSRVServerSIGChildProcessQuit(int signo)
{
    int     stat;
    waitpid(-1, &stat, WNOHANG);
}

static void OBJVSRVServerSIGProcessKill(int signo){
    if(gProcess){
        (* gProcess->clazz->kill)(gServer,gProcess,signo);
    }
}

static void OBJVSRVServerRunProcess(OBJVSRVProcess * process){
    
    pid_t pid;
    double t;
    
    if((* process->clazz->create)(gServer,process)){
        
        if(( pid = fork() ) < 0 )
        {
            fprintf(stderr, "fork error\n");
            exit(EXIT_FAILURE);
        }
        else if(pid == 0)
        {
            signal(SIGCHLD, OBJVSRVServerSIGChildProcessQuit);
            
            signal(SIGINT, OBJVSRVServerSIGProcessKill);
            signal(SIGQUIT, OBJVSRVServerSIGProcessKill);
            signal(SIGBUS, OBJVSRVServerSIGProcessKill);
            signal(SIGSEGV, OBJVSRVServerSIGProcessKill);
            signal(SIGSYS, OBJVSRVServerSIGProcessKill);
            signal(SIGTERM, OBJVSRVServerSIGProcessKill);
            signal(SIGURG, OBJVSRVServerSIGProcessKill);
            signal(SIGXCPU, OBJVSRVServerSIGProcessKill);
            signal(SIGXFSZ, OBJVSRVServerSIGProcessKill);
            signal(SIGILL, OBJVSRVServerSIGProcessKill);
            signal(SIGTRAP, OBJVSRVServerSIGProcessKill);
            signal(SIGABRT, OBJVSRVServerSIGProcessKill);
            signal(SIGKILL, OBJVSRVServerSIGProcessKill);
            
            signal(SIGFPE, OBJVSRVServerSIGNAN);
            signal(SIGPIPE, SIG_IGN);
            signal(SIGTTOU, OBJVSRVServerSIGNAN);
            signal(ETIMEDOUT, OBJVSRVServerSIGNAN);
            
            process->pid  = getpid();
            
            gProcess = process;
            
            (* process->clazz->open)(gServer,process);
            
            while(!processExit && !process->exit){
                
                t = (* process->clazz->tick)(gServer,process);
                
                if(t){
                    usleep(t * 1000000);
                }
            }
            
            (* process->clazz->close)(gServer,process);
            
            exit(EXIT_SUCCESS);
        }
        
        process->pid = pid;
        
    }
    
}

int OBJVSRVServerAccept(OBJVSRVServer * server,double timeout,struct sockaddr * addr,socklen_t * socklen){
    
    int client = 0;
    fd_set rds;
    int res;
    int fl;
    int fn = 1;
    
    struct timeval timeo = {(int)timeout, (timeout - (int) timeout) * 1000000};
    
    pthread_mutex_lock(&server->run.listenMutex);
    
    FD_ZERO(&rds);
    
    FD_SET(server->run.listenSocket, &rds);
    
    res = select(server->run.listenSocket + 1, &rds, NULL, NULL, &timeo);
    
    if(res == 0){
        
    }
    else if(res == -1){
        if(errno == EINTR){
            
        }
        else{
            client = -1;
        }
    }
    else{
        if(FD_ISSET(server->run.listenSocket, &rds)){
            
            client = accept(server->run.listenSocket, addr,socklen);
            
            if(client != -1){
                fl =  fcntl(client, F_GETFL) ;
                fcntl(client, F_SETFL, fl | O_NONBLOCK);
                setsockopt(server->run.listenSocket, SOL_SOCKET, SO_RCVLOWAT, (void *)&fn, sizeof(fn));
                setsockopt(server->run.listenSocket, SOL_SOCKET, SO_SNDLOWAT, (void *)&fn, sizeof(fn));
            }
        }
    }
    
    pthread_mutex_unlock(&server->run.listenMutex);
    
    return client;
}
