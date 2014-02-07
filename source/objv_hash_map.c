//
//  objv_hash_map.c
//  objv
//
//  Created by zhang hailong on 14-1-30.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//


#define READONLY


#include "objv_os.h"
#include "objv.h"
#include "objv_map.h"
#include "objv_hash_map.h"
#include "objv_crypt.h"

#define MAP_HASH_TABLE_SIZE 64

long objv_hash_map_hash_code_string(void * key ){
	return (long) objv_crc32(0, key, ( unsigned int) strlen(key));
}

long objv_hash_map_hash_code_ptr(void * key){
    return (long) key;
}

long objv_hash_map_hash_code_key(void * key){
    objv_key_t * k = (objv_key_t *) key;
    return k ? ((long) k->name << 1) + k->type : 0;
}

typedef struct _objv_hash_map_impl_t{
    objv_hash_map_t base;
    objv_map_t * maps[MAP_HASH_TABLE_SIZE];
    objv_hash_map_hash_code_t hashCode;
    objv_map_compare_t compare;
    int capacity;
} objv_hash_map_impl_t;

objv_hash_map_t * objv_hash_map_alloc(int capacity,objv_hash_map_hash_code_t hash_code ,objv_map_compare_t compare){
    
    objv_hash_map_impl_t * impl = (objv_hash_map_impl_t *) objv_zone_malloc(NULL,sizeof(objv_hash_map_impl_t));
    
    objv_zone_memzero(NULL,impl,sizeof(objv_hash_map_impl_t));

    impl->capacity = capacity;
    impl->hashCode = hash_code;
    impl->compare = compare;
    
    return (objv_hash_map_t *) impl;

}

void objv_hash_map_dealloc(objv_hash_map_t * map){
    
    if(map){
        
        objv_hash_map_impl_t * impl = (objv_hash_map_impl_t *) map;
        
        objv_map_t ** p = impl->maps;
        int c = MAP_HASH_TABLE_SIZE;
        
        while(c >0){
            
            if(*p ){
                
                objv_map_dealloc(* p);
                
            }
            
            c --;
            p ++;
        }
    
        objv_zone_free(NULL,map);
    }
}

void * objv_hash_map_put(objv_hash_map_t *  map,void * key,void * value){
    if(map && key && value){
        objv_hash_map_impl_t * impl = (objv_hash_map_impl_t *) map;
        long hashCode = (* impl->hashCode)(key);
        int index = abs(hashCode % MAP_HASH_TABLE_SIZE);
        void * v;
        objv_map_t * p = impl->maps[index];
        
        if(!p){
            p = objv_map_alloc(impl->capacity,impl->compare);
            impl->maps[index] = p;
        }
        
        v = objv_map_put(p,key,value);
        
        if(v == NULL){
            map->length ++;
        }
        
        return v;
    }
    return NULL;
}

void * objv_hash_map_get(objv_hash_map_t *  map,void * key){
    
    if(map && key){
        objv_hash_map_impl_t * impl = (objv_hash_map_impl_t *) map;
        long hashCode = (* impl->hashCode)(key);
        int index = abs(hashCode % MAP_HASH_TABLE_SIZE);
        
        objv_map_t * p = impl->maps[index];
        
        if(p){
            return objv_map_get(p,key);
        }
    }
    return NULL;
}

void * objv_hash_map_remove(objv_hash_map_t *  map,void * key){
    
    if(map && key){
        objv_hash_map_impl_t * impl = (objv_hash_map_impl_t *) map;
        long hashCode = (* impl->hashCode)(key);
        int index = abs(hashCode % MAP_HASH_TABLE_SIZE);
        void * v;
        objv_map_t * p = impl->maps[index];
        
        if(p){
            v = objv_map_remove(p,key);
            if(v){
                map->length --;
            }
            return v;
        }
    }
    return NULL;
    
}

void objv_hash_map_clear(objv_hash_map_t * map){
    
    if(map){
        objv_hash_map_impl_t * impl = (objv_hash_map_impl_t *) map;
        
        objv_map_t ** p = impl->maps;
        int c = MAP_HASH_TABLE_SIZE;
        
        while(c >0){
            
            if(*p ){
                
                objv_map_clear(* p);
                
            }
            
            c --;
            p ++;
        }
    }
}


void * objv_hash_map_keyAt(objv_hash_map_t * map,int index){
    
    if(map && index >=0 && index < map->length){
        
        objv_hash_map_impl_t * impl = (objv_hash_map_impl_t *) map;
        
        objv_map_t ** p = impl->maps;
        int c = MAP_HASH_TABLE_SIZE;
        int i = 0;
        
        while(c >0){
            
            if(*p ){
                
                if(index - i < (* p) ->length){
                    return  objv_map_keyAt(* p, index - i);
                }
                else{
                    i += (* p) ->length;
                }
            }
            
            c --;
            p ++;
        }
        
    }
    
    return NULL;
}

void * objv_hash_map_valueAt(objv_hash_map_t * map,int index){
    
    if(map && index >=0 && index < map->length){
        
        objv_hash_map_impl_t * impl = (objv_hash_map_impl_t *) map;
        
        objv_map_t ** p = impl->maps;
        int c = MAP_HASH_TABLE_SIZE;
        int i = 0;
        
        while(c >0){
            
            if(*p ){
                
                if(index - i < (* p) ->length){
                    return  objv_map_valueAt(* p, index - i);
                }
                else{
                    i += (* p) ->length;
                }
            }
            
            c --;
            p ++;
        }
        
    }
    
    return NULL;
}

