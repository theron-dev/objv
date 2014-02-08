//
//  objv_inifile.c
//  objv
//
//  Created by zhang hailong on 14-2-8.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv.h"
#include "objv_inifile.h"

objv_boolean_t objv_inifile_init(objv_inifile_t * inifile,const char * filePath){
    
    FILE * f = fopen(filePath,"r");
    size_t len;
    
    objv_zone_memzero(NULL,inifile,sizeof(objv_inifile_t));
    
    if(f){
    
        objv_mbuf_init(& inifile->data,1024);
        
        objv_mbuf_init(& inifile->section, 64);
        
        objv_mbuf_init(& inifile->key, 64);
        
        objv_mbuf_init(& inifile->value, 64);
        
        while((len = fread( (char *) inifile->data.data + inifile->data.length, 1
                           , inifile->data.size - inifile->data.length, f)) > 0){
            
            inifile->data.length += len;
            
            objv_mbuf_extend(& inifile->data, inifile->data.length + 1024);
            
        }
        
        fclose(f);
        
        inifile->p = (char *) objv_mbuf_str( & inifile->data );
        
        return objv_true;
    }
    
    return objv_false;
}

void objv_inifile_destroy(objv_inifile_t * inifile){
    
    if(inifile){
        objv_mbuf_destroy( & inifile->data );
        objv_mbuf_destroy( & inifile->section );
        objv_mbuf_destroy( & inifile->key );
        objv_mbuf_destroy( & inifile->value );
    }
}

objv_boolean_t objv_inifile_next(objv_inifile_t * inifile){
    if(inifile){
        
        char *p = inifile->p;
		
        objv_mbuf_clear( & inifile->key);
		objv_mbuf_clear( & inifile->value);
        
		while(p && *p !='\0'){
			if(inifile->s ==0){
				if(*p == '['){
					inifile->s =1;
					objv_mbuf_clear( & inifile->section);
				}
			}
			else if(inifile->s==1){
				//section
				if(*p == ']'){
					inifile->s = 2;
				}
				else{
					objv_mbuf_append( & inifile->section, p, 1);
				}
			}
			else if(inifile->s ==2){
				// key
                if(*p == ' ' || *p == '\t' || *p == '\n' || *p =='\r'){
                    
                }
				else if(*p == '['){
					inifile->s =1;
					objv_mbuf_clear( & inifile->section );
				}
				else if(*p == '='){
					inifile->s =3;
				}
				else {
					objv_mbuf_append(& inifile->key, p, 1);
				}
			}
			else if(inifile->s ==3){
				// value
				if(*p == '\r'){
				}
				else if(*p == '\n'){
					inifile->s = 2 ;
					break;
				}
				else{
					objv_mbuf_append( & inifile->value,p,1);
				}
			}
			p++;
		}
		inifile->p = p;
		return p != NULL && *p !='\0';
    }
    return objv_false;
}

const char * objv_inifile_section(objv_inifile_t * inifile){
    if(inifile){
        return objv_mbuf_str( & inifile->section );
    }
    return NULL;
}

const char * objv_inifile_key(objv_inifile_t * inifile){
    if(inifile){
        return objv_mbuf_str( & inifile->key );
    }
    return NULL;
}

const char * objv_inifile_value(objv_inifile_t * inifile){
    if(inifile){
        return objv_mbuf_str( & inifile->value );
    }
    return NULL;
}

