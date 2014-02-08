//
//  objv_vm.c
//  objv
//
//  Created by zhang hailong on 14-2-7.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#define READONLY

#include "objv_os.h"
#include "objv.h"
#include "objv_vermin.h"
#include "objv_vmcompiler.h"
#include "objv_vmcompiler_binary.h"
#include "objv_vmcompiler_meta.h"
#include "objv_autorelease.h"
#include "objv_inifile.h"
#include "objv_log.h"

vm_boolean_t vmContextLoadClassSource(vmContext * ctx,const char * className,objv_string_t * source,objv_array_t * errors){
    objv_zone_t * zone = ctx->base.zone;
    vmCompilerClassMeta * classMeta = vmCompilerClassSource(source, errors);
    vmCompilerBinary * binary ;
    vmMetaBinary * b;
    vm_boolean_t rs = vm_false;
    
    if(classMeta){
        
        binary = vmCompilerBinaryAlloc(zone);
        
        if(vmCompilerBinaryAddClass(binary, classMeta, className)){
        
            b = vmCompilerBinaryBytes(binary);
        
            rs = vmContextLoadBinary(ctx, b, vm_true);
        
        }
        
        objv_object_release((objv_object_t *) binary);
        
    }
    return rs;
}

vmVariant vmRun(vmContext * ctx,objv_string_t * source,objv_array_t * errors){
    
    char className[128];
    vmVariant v = {vmVariantTypeVoid,0};
    objv_class_t * clazz;
    objv_object_t * object;
    
    sprintf(className,"unname_%ld_%d",time(NULL),rand());
    
    if(vmContextLoadClassSource(ctx,className,source,errors)){
       
        clazz = vmContextGetClass(ctx, vmContextKey(ctx, className));
        
        if(clazz){
            
            vmContextScopePush(ctx);
            
            v = vmObjectNew(ctx, clazz, NULL);
            
            if(v.type & vmVariantTypeObject && v.objectValue){
                object = v.objectValue;
                
                v = vmObjectInvoke(clazz, object, vmContextKey(ctx, "main"), NULL);
                
            }
            
            vmContextScopePop(ctx);
        }
        
    }
    
    return v;
    
}

vm_boolean_t vmCompilerBinaryLoadClassSource(vmCompilerBinary * binary,const char * className,objv_string_t * source,objv_array_t * errors){
    vmCompilerClassMeta * classMeta = vmCompilerClassSource(source, errors);
    return classMeta && vmCompilerBinaryAddClass(binary, classMeta, className);;
}

static vm_boolean_t vmCompilerBinaryLoadFileContent(objv_mbuf_t * mbuf,const char * filePath){
    
    FILE * f = fopen(filePath,"r");
    size_t len;
    
    if(f){
        
        while((len = fread( (char *) mbuf->data + mbuf->length, 1
                           , mbuf->size - mbuf->length, f)) > 0 ){
            
            mbuf->length += len;
            
            objv_mbuf_extend(mbuf, mbuf->length + 1024);
        }
        
        fclose(f);
        
        return vm_true;
    }
    
    return vm_false;
}

vm_boolean_t vmCompilerBinaryLoadProject(vmCompilerBinary * binary,const char * project){
    
    if(binary && project){
        
        const char * lp = objv_string_indexOf(project, DIR_SEPARATED);
        size_t dirlen = lp ? lp - project + 1: 0;
        objv_inifile_t inifile;
        objv_mbuf_t path;
        objv_mbuf_t source;
        objv_string_t * s;
        objv_array_t * errors;
        objv_zone_t * zone = binary->base.zone;
        vm_boolean_t rs = vm_true;
        
        if(objv_inifile_init(& inifile, project)){
        
            objv_mbuf_init(& path, 128);
            objv_mbuf_init(& source, 1024);
            
            while (objv_inifile_next(& inifile)) {
                
                if(strcmp(objv_inifile_section(& inifile), "SOURCES") ==0){
                    
                    objv_mbuf_clear(& path);
                    objv_mbuf_clear(& source);
                    
                    if(dirlen){
                        objv_mbuf_append(& path, (void *) project, dirlen);
                    }
                    
                    objv_mbuf_format(& path, "%s", objv_inifile_value(& inifile));
                    
                    if(vmCompilerBinaryLoadFileContent(& source, objv_mbuf_str(& path))){
                        
                        s = objv_string_alloc_nocopy(zone, objv_mbuf_str(& source));
                        errors = objv_array_alloc(zone, 4);
                        
                        if(! vmCompilerBinaryLoadClassSource(binary, objv_inifile_key(& inifile), s, errors)){
                            rs = vm_false;
                            objv_log("ERROR: compiler class %s: %s\n",objv_inifile_key(& inifile),objv_mbuf_str(& path));
                            vmCompilerErrorsLog(errors);
                        }
                        
                        objv_object_release((objv_object_t *) s);
                        objv_object_release((objv_object_t *) errors);
                        
                    }
                    else{
                        rs = vm_false;
                        objv_log("ERROR: Not Found File %s\n",objv_mbuf_str(& path));
                    }
                    
                }
                else if(strcmp(objv_inifile_section(& inifile ), "RESOURCES")){
                    
                    objv_mbuf_clear(& path);
                    objv_mbuf_clear(& source);
                    
                    if(dirlen){
                        objv_mbuf_append(& path, (void *) project, dirlen);
                    }
                    
                    objv_mbuf_format(& path, "%s", objv_inifile_value(& inifile));
                  
                    if(vmCompilerBinaryLoadFileContent(& source, objv_mbuf_str(& path))){
                        
                        if(! vmCompilerBinaryAddStringResource(binary, objv_inifile_key(& inifile), objv_mbuf_str(& source))){
                            rs = vm_false;
                            objv_log("ERROR: Resource %s: %s\n",objv_inifile_key(& inifile),objv_mbuf_str(& path));
                        }
                        
                    }
                    else{
                        rs = vm_false;
                        objv_log("ERROR: Not Found File %s\n",objv_mbuf_str(& path));
                    }

                }
                else if(strcmp(objv_inifile_section(& inifile ), "STRINGS")){
                    
                    if(! vmCompilerBinaryAddStringResource(binary, objv_inifile_key(& inifile), objv_inifile_value(& inifile))){
                        rs = vm_false;
                        objv_log("ERROR: String %s: %s\n",objv_inifile_key(& inifile),objv_inifile_value(& inifile));
                    }
                    
                }
                else if(strcmp(objv_inifile_section(& inifile ), "PROJECTS")){
                
                    objv_mbuf_clear(& path);
                    objv_mbuf_clear(& source);
                    
                    if(dirlen){
                        objv_mbuf_append(& path, (void *) project, dirlen);
                    }
                    
                    objv_mbuf_format(& path, "%s", objv_inifile_value(& inifile));
                  
                    rs = vmCompilerBinaryLoadProject(binary, objv_mbuf_str(& path));
                    
                }
                
                if(!rs){
                    break;
                }
            }
            
            objv_mbuf_destroy(& path);
            objv_mbuf_destroy(& source);
            
            objv_inifile_destroy(& inifile);
        }
        
        return rs;
        
    }
    
    return vm_false;
}

