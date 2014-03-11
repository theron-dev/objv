//
//  objv_os.h
//  objv
//
//  Created by zhang hailong on 14-1-29.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#ifndef objv_objv_os_h
#define objv_objv_os_h

#ifdef TARGET_OS_MAC

#pragma pack(4)

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <netinet/in.h>
#include <dns.h>
#include <dns_util.h>
#include <resolv.h>
#include <sys/stat.h>
#include <utime.h>
#include <zlib.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <assert.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <math.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <sys/ioctl.h>
#include <sys/sockio.h>
#include <net/ethernet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define closesocket(c) close(c)

#define BITS_LOW
//#define BITS_HIGH

#endif

#ifdef TARGET_OS_WIN32


#endif

#ifdef TARGET_OS_LINUX

#pragma pack(4)

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <utime.h>
#include <zlib.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <assert.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <math.h>
#include <net/if.h>

#define closesocket(a) close(a)

#define EXPORT

#define HAS_PTHREAD

#define BITS_LOW
//#define BITS_HIGH


#endif

#include "objv_types.h"

#ifdef __cplusplus
extern "C" {
#endif

  
#ifdef _POSIX_THREADS
    
#define DIR_SEPARATED  "/"
    
    typedef pthread_t objv_thread_t;
    
    static inline objv_thread_t objv_thread_self(){
        return pthread_self();
    }
    
    typedef pthread_mutex_t objv_mutex_t;
    
    static inline int objv_mutex_init(objv_mutex_t * m){
        return pthread_mutex_init(m, NULL);
    }
    
    static inline int objv_mutex_destroy(objv_mutex_t * m){
        return pthread_mutex_destroy(m);
    }
    
    static inline int objv_mutex_lock(objv_mutex_t *m) {
        return pthread_mutex_lock(m);
    }
    
    static inline int objv_mutex_try_lock(objv_mutex_t *m) {
        return pthread_mutex_trylock(m);
    }
    
    static inline int objv_mutex_unlock(objv_mutex_t *m) {
        return pthread_mutex_unlock(m);
    }

    typedef pthread_key_t objv_thread_key_t;
    
    static inline int objv_thread_key_create(objv_thread_key_t * key, void (* dealloc)(void *)){
        return pthread_key_create(key, dealloc);
    }
    
    static inline int objv_thread_key_dealloc(objv_thread_key_t key){
        return pthread_key_delete(key);
    }
    
    static inline void * objv_thread_key_value(objv_thread_key_t key){
        return pthread_getspecific(key);
    }
    
    static inline void objv_thread_key_setValue(objv_thread_key_t key, void * value){
        pthread_setspecific(key,value);
    }
    
    
    typedef struct _objv_waiter_t{
        pthread_mutex_t mutex;
        pthread_cond_t cond;
    } objv_waiter_t;
    
    static inline int objv_waiter_init(objv_waiter_t * waiter){
        pthread_mutex_init(&waiter->mutex, NULL);
        return pthread_cond_init(& waiter->cond, NULL);
    }
    
    static inline void objv_waiter_destroy(objv_waiter_t * waiter){
        pthread_cond_destroy(&waiter->cond);
        pthread_mutex_destroy(&waiter->mutex);
    }
    
    static inline void objv_waiter_lock(objv_waiter_t * waiter){
        pthread_mutex_lock(&waiter->mutex);
    }
    
    static inline void objv_waiter_unlock(objv_waiter_t * waiter){
        pthread_mutex_unlock(&waiter->mutex);
    }
    
    static inline void objv_waiter_wait(objv_waiter_t * waiter){
        pthread_cond_wait(&waiter->cond, &waiter->mutex);
    }
    
    static inline int objv_waiter_wait_timeout(objv_waiter_t * waiter,objv_timeinval_t timeout){
        struct timeval now;
        struct timespec tm ;
        
        gettimeofday(& now, NULL);
        
        timeout += now.tv_sec + (double) now.tv_usec / 1000000;
        
        tm.tv_sec = (long)timeout ;
        tm.tv_nsec = (long)((timeout - (long)timeout) * 1000000000);
        
        return pthread_cond_timedwait(&waiter->cond, &waiter->mutex, & tm);
    }
    
    static inline void objv_waiter_join(objv_waiter_t * waiter){
        pthread_cond_broadcast(&waiter->cond);
    }
    
    static inline void objv_thread_yield(void){
        usleep(1);
    }
    
    static inline objv_thread_t objv_thread_create(void * (* callback)(void *),void * userInfo){
        objv_thread_t t = 0;
        pthread_create(&t, NULL, callback, userInfo);
        return t;
    }
    
    static inline void objv_thread_exit(void * userInfo){
        pthread_exit(userInfo);
    }
    
    static inline void objv_thread_detach(objv_thread_t t){
        pthread_detach(t);
    }
    
    typedef int objv_os_file_t;
    
    static inline objv_os_file_t objv_os_file_open(const char * path,int flags,mode_t mode){
        int rc;
        do{ rc = open(path, flags,mode); } while( rc<0 && errno==EINTR );
        return rc;
    }
    
    static inline void objv_os_file_close(objv_os_file_t file){
        int rc;
        do{ rc = close(file); } while( rc<0 && errno==EINTR );
    }
    
    static inline int objv_os_file_lock(objv_os_file_t file,int op){
        int rc;
        do{ rc = flock(file,op); }while( rc<0 && errno==EINTR );
        return rc;
    }
    
    static inline ssize_t objv_os_file_read(objv_os_file_t file,void * bytes,size_t length){
        ssize_t rc;
        do{ rc = read(file, bytes, length); }while( rc<0 && errno==EINTR );
        return rc;
    }
    
    static inline ssize_t objv_os_file_write(objv_os_file_t file,void * bytes,size_t length){
        ssize_t rc;
        do{ rc = write(file, bytes, length); }while( rc<0 && errno==EINTR );
        return rc;
    }
    
    static inline off_t objv_os_file_seek(objv_os_file_t file,off_t off,int by){
        return lseek(file, off, by);
    }
    
    typedef int objv_os_socket_t;
    
    static inline ssize_t objv_os_socket_read(objv_os_socket_t file,void * bytes,size_t length){
        ssize_t rc;
        do{ rc = recv(file, bytes, length,0); }while( rc<0 && errno==EINTR );
        return rc;
    }
    
    static inline ssize_t objv_os_socket_write(objv_os_socket_t file,void * bytes,size_t length){
        ssize_t rc;
        do{ rc = send(file, bytes, length,0); }while( rc<0 && errno==EINTR );
        return rc;
    }
    
    static inline void objv_os_socket_close(objv_os_socket_t sock){
        
        fd_set ds,ws;
        struct timeval timeo = {0, 1};
        char buffer[1024];
        ssize_t len,res;
        int c = 6;
        timeo.tv_usec = 100;
        
        while(1){
            FD_ZERO(&ws);
            FD_SET(sock, &ws);
            res = select(sock +1, NULL,&ws, NULL, &timeo);
            if(res == 0 && c < 0){
                c --;
                continue;
            }
            shutdown(sock, SHUT_WR);
            break;
        }
        
        while(1){
            FD_ZERO(&ds);
            FD_SET(sock, &ds);
            res = select(sock +1, &ds,NULL, NULL, &timeo);
            if(res == 0){
                shutdown(sock, SHUT_RD);
                break;
            }
            if(res < 0){
                if(errno == EINTR){
                    shutdown(sock, SHUT_RD);
                }
                break;
            }
            else if(FD_ISSET(sock, &ds)){
                
                len = recv(sock, buffer, sizeof(buffer), 0);
                
                if(len ==0){
                    shutdown(sock, SHUT_RD);
                    break;
                }
                else if(len < 0){
                    if(errno == EINTR){
                        shutdown(sock, SHUT_RD);
                    }
                    break;
                }
            }
        }
        
        usleep(2);
        
        close(sock);

    }
    
    static inline struct in_addr objv_os_resolv(const char *domain){
        struct in_addr addr;
        struct hostent *host;
        
        addr.s_addr = inet_addr(domain);
        
        if(addr.s_addr != INADDR_BROADCAST){
            return addr;
        }
        
        host = gethostbyname(domain);
        if(host && host->h_addr_list && host->h_length >0){
#if defined( TARGET_OS_WIN32) || defined( TARGET_OS_ANDROID)
            addr.s_addr = *(unsigned long *)host->h_addr_list[0];
#else
            addr.s_addr = *(in_addr_t *)host->h_addr_list[0];
#endif
        }
        
        return addr;
    }
    
#elif defined(TARGET_OS_WIN32)
    
    #define DIR_SEPARATED  "\\"
    
    typedef pthread_mutex_t objv_mutex_t;
    
    typedef struct _objv_mutex_t{
        HANDLE handle;
    }  objv_mutex_t;
    
    static inline int objv_mutex_init(objv_mutex_t * m){
        m->handle = CreateMutex(NULL,0,NULL);
        return 0;
    }
    
    static inline int objv_mutex_destroy(objv_mutex_t * m){
        
        CloseHandle(mutex->handle);
        
        return 0;
    }
    
    static inline int objv_mutex_lock(objv_mutex_t *m) {
        WaitForSingleObject(mutex->handle,INFINITE);
    }
    
    static inline int objv_mutex_try_lock(objv_mutex_t *m) {
        return (WAIT_OBJECT_0 != WaitForSingleObject(m->handle, 0));
    }
    
    static inline int objv_mutex_unlock(objv_mutex_t *m) {
        ReleaseMutex(mutex->handle);
        return 0;
    }

    
    typedef struct _objv_waiter_t{
        HANDLE mutex;
        HANDLE event;
    } objv_waiter_t;
    
    static inline int objv_waiter_init(objv_waiter_t * waiter){
        waiter->mutex = CreateMutex(NULL,0,NULL);
        waiter->event = CreateEvent(NULL,0,0,NULL);
        return 0;
    }
    
    static inline void objv_waiter_destroy(objv_waiter_t * waiter){
        CloseHandle(waiter->mutex);
        CloseHandle(waiter->event);
    }
    
    static inline void objv_waiter_lock(objv_waiter_t * waiter){
        WaitForSingleObject(waiter->mutex,INFINITE);
    }
    
    static inline void objv_waiter_unlock(objv_waiter_t * waiter){
         ReleaseMutex(waiter->mutex);
    }
    
    static inline void objv_waiter_wait(objv_waiter_t * waiter){
        ReleaseMutex(waiter->mutex);
        ResetEvent(waiter->event);
        WaitForSingleObject(waiter->mutex,INFINITE);
    }
    
    static inline void objv_waiter_join(objv_waiter_t * waiter){
        ReleaseMutex(waiter->mutex);
        SetEvent(waiter->event);
        WaitForSingleObject(waiter->mutex,INFINITE);
    }
    
    
#endif
    
    static inline objv_timeinval_t objv_timestamp(){
        
        struct timeval tm;
        
        gettimeofday(& tm,NULL);
        
        return (double) tm.tv_sec + (double) tm.tv_usec / 1000000.0;
    }
    
    
#ifdef __cplusplus
}
#endif

#endif
