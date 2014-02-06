//
//  objv_map.c
//  objv
//
//  Created by zhang hailong on 14-1-30.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY


#include "objv_os.h"
#include "objv.h"
#include "objv_map.h"


#define KEY_NOT_FOUND	0x7fffffff
#define KEY_ITEM_EXTEND_MIN	20

int objv_map_compare_string(void * key1 ,void * key2){
    return key1 == key2 ? 0 : strcmp((char *) key1,(char *) key2);
}

int objv_map_compare_any(void * key1 ,void * key2){

    long r = (long) key1 - (long) key2;
    if(r > 0){
        return 1;
    }
    else if(r < 0){
        return -1;
    }
    return 0;
}

int objv_map_compare_key(void * key1 ,void * key2){
    objv_key_t * k1 = (objv_key_t *) key1;
    objv_key_t * k2 = (objv_key_t *) key2;
    
    if(k1 == k2){
        return 0;
    }
    
    if(k1->name == k2->name){
        return 0;
    }
    
    long r = 0;
    
    if(k1->type == k2->type){
        r = (long) k1->name - (long) k2->name;
    }
    else{
        r = strcmp(k1->name, k2->name);
    }
    
    if(r > 0){
        return 1;
    }
    else if(r < 0){
        return -1;
    }
    
    return 0;
}

typedef struct _objv_map_item_t{
    void * key;
    void * value;
} objv_map_item_t;

typedef struct _objv_map_impl_t{
    objv_map_t base;
	objv_map_item_t * items;
	int size;
    objv_map_compare_t compare;
} objv_map_impl_t;


objv_map_t * objv_map_alloc(int capacity,objv_map_compare_t compare){
    
    objv_map_impl_t * impl = objv_zone_malloc(NULL,sizeof(objv_map_impl_t));
    
    objv_zone_memzero(NULL,impl,sizeof(objv_map_impl_t));
    
    if(capacity <= 0  ){
        capacity = KEY_ITEM_EXTEND_MIN;
    }
    
    impl->items = objv_zone_malloc(NULL,sizeof(objv_map_item_t) * capacity);
    impl->size = capacity;
    impl->compare = compare;
   
	return (objv_map_t *) impl;
}


void objv_map_dealloc(objv_map_t * map){
	objv_map_impl_t * m = (objv_map_impl_t *)map;
	if(m){
        if(m->items){
            objv_zone_free(NULL,m->items);
        }
        objv_zone_free(NULL,m);
	}
}


void * objv_map_keyAt(objv_map_t * map,int i){
	objv_map_impl_t * m = (objv_map_impl_t *)map;
	if(m && i >=0 && i< m->base.length){
		return m->items[i].key;
	}
	return NULL;
}

void * objv_map_valueAt(objv_map_t * map,int i){
	objv_map_impl_t * m = (objv_map_impl_t *)map;
	if(map && i >=0 && i< m->base.length){
		return m->items[i].value;
	}
	return NULL;
}

void objv_map_setValue(objv_map_t * map,int i,void * value){
	objv_map_impl_t * m = (objv_map_impl_t *)map;
	if(map && i >=0 && i< m->base.length){
        m->items[i].value = value;
	}
}


static int _objv_map_find(objv_map_impl_t * map,void * key,int * pIndex,int * nIndex){
	int i = -1 ;
	int rs = KEY_NOT_FOUND;
	*pIndex = 0;
	*nIndex = map->base.length - 1;
	
	while(*pIndex < *nIndex){
		i = (*pIndex + *nIndex) / 2;
		rs =  (* map->compare )(key,map->items[i].key);
		if(rs == 0){
			*nIndex = *pIndex = i;
			break;
		}
		else if(rs <0){
			*nIndex = i -1;
		}
		else{
			*pIndex = i +1;
		}
	}
	
	if(*pIndex > *nIndex){
		if(rs != KEY_NOT_FOUND){
			if(rs <0){
				*nIndex = (*nIndex) +1;
			}
			else{
				*pIndex = (*pIndex) -1;
			}
		}
	}
	else if(*pIndex == *nIndex){
		rs = (* map->compare )(key,map->items[*pIndex].key);
	}
	
	return rs;
}

void * objv_map_get(objv_map_t * map,void * key){
	objv_map_impl_t * m = (objv_map_impl_t *)map;
	if(m){
		int pIndex,nIndex;
		int rs = _objv_map_find(m, key, &pIndex, &nIndex);
		if(rs ==0 && pIndex == nIndex){
			return m->items[pIndex].value;
		}
	}
	return NULL;
}

static inline void _objv_map_extend(objv_map_impl_t * map,int length){
    
    if(map->size < length){
        
        map->size = length + KEY_ITEM_EXTEND_MIN;
        
        map->items = objv_zone_realloc(NULL,map->items,map->size * sizeof(objv_map_item_t));
        
    }
    
}

void * objv_map_put(objv_map_t * map,void * key, void * value){
	
    objv_map_impl_t * m = (objv_map_impl_t *)map;
	if(m && key && value){
		int pIndex,nIndex,i;
		int rs = _objv_map_find(m, key, &pIndex, &nIndex);
		objv_map_item_t * item;
        void * v = NULL;
        
		if(rs == KEY_NOT_FOUND){
			assert(m->base.length ==0);
			_objv_map_extend(m, m->base.length + 1);
			item = m->items;
            item->key = key;
            item->value = value;
            m->base.length ++;
			return NULL;
		}
		else if(rs == 0){
			item = m->items + pIndex;
            v = item->value;
            item->value = value;
			return v;
		}
		else if(rs <0){
			_objv_map_extend(m, m->base.length + 1);
			for(i=m->base.length ;i>nIndex;i--){
                m->items[i] = m->items[i -1];
			}
			item = m->items + nIndex;
            item->key = key;
            item->value = value;
			m->base.length ++;
			return NULL;
		}
		else{
			_objv_map_extend(m, m->base.length + 1);
			nIndex = pIndex +1;
			for(i=m->base.length ;i>nIndex;i--){
                m->items[i] = m->items[i -1];
			}
			item = m->items + nIndex;
            item->key = key;
            item->value = value;
			m->base.length ++;
			return NULL;
		}
	}
	return NULL;
}

void * objv_map_remove(objv_map_t * map,void* key){
	objv_map_impl_t * m = (objv_map_impl_t *)map;
	if(m && key){
		int pIndex,nIndex,i;
		int rs = _objv_map_find(m, key, &pIndex, &nIndex);
        void * v;
		if(rs == 0){
            v = m->items[pIndex].value;
            
			for(i = pIndex;i<m->base.length-1;i++){
                m->items[i] = m->items[i + 1];
			}
			m->base.length --;
			return v;
		}
	}
	return NULL;
}

void objv_map_clear(objv_map_t * map){
	objv_map_impl_t * m = (objv_map_impl_t *) map;
	if(m ){
		m->base.length = 0;
	}
}

