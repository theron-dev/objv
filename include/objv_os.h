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

#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#ifdef _POSIX_THREADS
#include <pthread.h>
#endif
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>

#endif

#ifdef TARGET_OS_WIN32


#endif

#include "objv_types.h"

#ifdef __cplusplus
extern "C" {
#endif

  
#ifdef _POSIX_THREADS
    
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
        do{ rc = read(file, bytes, length); }while( rc<0 && errno==EINTR );
        return rc;
    }
    
    static inline ssize_t objv_os_socket_write(objv_os_socket_t file,void * bytes,size_t length){
        ssize_t rc;
        do{ rc = write(file, bytes, length); }while( rc<0 && errno==EINTR );
        return rc;
    }
    
#elif defined(TARGET_OS_WIN32)
    
    
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
