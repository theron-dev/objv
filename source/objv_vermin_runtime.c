//
//  objv_vermin_runtime.c
//  objv
//
//  Created by zhang hailong on 14-2-5.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#include "objv_os.h"
#include "objv.h"
#include "objv_vermin.h"
#include "objv_value.h"
#include "objv_autorelease.h"
#include "objv_string.h"
#include "objv_exception.h"

#define vmMetaOperatorGet(binary,operatorOffset) (vmMetaOperator *) ( (char *) (binary) + (operatorOffset) )

#define vmMetaByOperator(op,index)   (vmMeta *) ( (char *)(op) + sizeof(vmMetaOperator) + sizeof(vmMeta) * (index))

#define vmUniqueKeyToString(binary,offset)  ((char *) (binary) + offset)

static vmVariant vmMetaToVariant(vmContext * ctx,objv_class_t * clazz,objv_object_t * object,void * binary,vmMeta * meta){
    vmVariant rs = {vmVariantTypeVoid,0};
	assert(ctx && meta);
    if(meta->type & vmMetaTypeOperator){
        rs = vmObjectOperatorExecute(ctx,clazz,object,binary,vmMetaOperatorGet(binary,meta->operatorOffset));
        rs.type = rs.type | (meta->type & (vmMetaTypeThrowable | vmMetaTypeCatch | vmMetaTypeFinally));
    }
    else{
        if(meta->type & vmMetaTypeInt32){
            rs.type = vmMetaTypeInt32 | (meta->type & (vmMetaTypeThrowable | vmMetaTypeCatch | vmMetaTypeFinally));
            rs.int32Value = meta->int32Value;
        }
        else if(meta->type & vmMetaTypeInt64){
            rs.type = vmMetaTypeInt64 | (meta->type & (vmMetaTypeThrowable | vmMetaTypeCatch | vmMetaTypeFinally));
            rs.int64Value = meta->int64Value;
        }
        else if(meta->type & vmMetaTypeBoolean){
            rs.type = vmMetaTypeBoolean | (meta->type & (vmMetaTypeThrowable | vmMetaTypeCatch | vmMetaTypeFinally));
            rs.booleanValue = meta->booleanValue;
        }
        else if(meta->type & vmMetaTypeDouble){
            rs.type = vmMetaTypeDouble | (meta->type & (vmMetaTypeThrowable | vmMetaTypeCatch | vmMetaTypeFinally));
            rs.doubleValue = meta->doubleValue;
        }
        else if(meta->type & vmMetaTypeString){
            if(meta->stringKey){
                rs.type = vmMetaTypeString | (meta->type & (vmMetaTypeThrowable | vmMetaTypeCatch | vmMetaTypeFinally));
                rs.stringValue = vmUniqueKeyToString(binary,meta->stringKey);
            }
        }
        else if(meta->type & vmMetaTypeObject){
            if(meta->objectKey){
                rs = vmContextVariant(ctx, vmContextKey(ctx, vmUniqueKeyToString(binary,meta->objectKey)));
                rs.type = rs.type | (meta->type & (vmMetaTypeThrowable | vmMetaTypeCatch | vmMetaTypeFinally));
            }
        }
    }
    return rs;
}


static vmVariant vmObjectOperatorInvoke(vmContext * ctx,objv_class_t * clazz,objv_object_t * object, void * binary,vmMetaOperator * op){
    vm_uint32_t i;
    vmMeta * meta = NULL;
    vmVariant rs = {vmVariantTypeVoid,0};
	assert(ctx && binary && op && op->type == vmOperatorTypeInvoke);
    
    for(i=0;i<op->metaCount;i++){
        meta = vmMetaByOperator(op,i);
        if(meta->type & vmMetaTypeReturn){
            rs = vmMetaToVariant(ctx,clazz,object,binary,meta);
            rs.type = (vmVariantType)( rs.type | vmMetaTypeReturn);
            return rs;
        }
        if(meta->type & vmMetaTypeThrowable){
            rs = vmMetaToVariant(ctx,clazz,object,binary,meta);
            rs.type = (vmVariantType)(rs.type | vmMetaTypeThrowable);
            return rs;
        }
        else if(meta->type & vmMetaTypeBreak){
            rs.type = (vmVariantType)vmMetaTypeBreak;
            rs.int64Value = 0;
            return rs;
        }
        else if(meta->type & vmMetaTypeContinue){
            rs.type = (vmVariantType)vmMetaTypeContinue;
            rs.int64Value = 0;
            return rs;
        }
        else if(meta->type & vmMetaTypeOperator){
            rs = vmObjectOperatorExecute(ctx, clazz, object, binary, vmMetaOperatorGet(binary, meta->operatorOffset));
            if(rs.type & (vmMetaTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
                return rs;
            }
        }
    }
    return rs;
}

static objv_key_t * vmVariantToUniqueKey(vmContext * ctx,vmVariant var){
    if(var.type & vmVariantTypeObject){
        if(var.objectValue){
            {
                objv_string_t * s = objv_object_stringValue(var.objectValue, NULL);
                if(s){
                    return vmContextKey(ctx, s->UTF8String);
                }
            }
        }
    }
    if(var.type & vmVariantTypeInt32){
        {
            char key[64];
            snprintf(key, sizeof(key),"%d",var.int32Value);
            return vmContextKey(ctx, key);
        }
    }
    if(var.type & vmVariantTypeInt64){
        {
            char key[64];
            snprintf(key, sizeof(key),"%lld",var.int64Value);
            return vmContextKey(ctx, key);
        }
    }
    if(var.type & vmVariantTypeDouble){
        {
            char key[64];
            snprintf(key, sizeof(key),"%f",var.doubleValue);
            return vmContextKey(ctx, key);
        }
    }
    if(var.type & vmVariantTypeString){
        if(var.stringValue){
            return vmContextKey(ctx, var.stringValue);
        }
    }
    return vmContextKey(ctx, "");
}

static void vmVariantTypeFor(vmVariant var1,vmVariant var2,vmVariantType * lowType,vmVariantType * highType){
    vmVariantType type1 = var1.type & ( vmVariantTypeInt32 | vmVariantTypeInt64 | vmVariantTypeBoolean | vmVariantTypeDouble | vmVariantTypeString | vmVariantTypeObject);
    vmVariantType type2 = var2.type & ( vmVariantTypeInt32 | vmVariantTypeInt64 | vmVariantTypeBoolean | vmVariantTypeDouble | vmVariantTypeString | vmVariantTypeObject);
    if(((vm_uint32_t)type1 - (vm_uint32_t)type2) >0){
        * lowType = type2;
        * highType = type1;
    }
    else{
        * lowType = type1;
        * highType = type2;
    }
}

vmVariant vmObjectOperatorFunction(vmContext * ctx,objv_class_t * clazz,objv_object_t * object, void * binary,vmMetaOperator * op,objv_array_t * arguments){
   
    vmVariant rs = {vmVariantTypeVoid,0};
    vmVariant obj = {vmVariantTypeVoid,0};
    vmMeta * meta = NULL;
    vm_int32_t i;
    
	assert(ctx && binary && op && op->type == vmOperatorTypeFunction);
    
    vmContextScopePush(ctx);
  
    if(clazz && object){
    
        obj.type = vmVariantTypeObject;
        obj.objectValue = object;
        obj.ofClass = clazz;
        
        vmContextSetVariant(ctx, ctx->keys.thisKey, obj);
        
        obj.ofClass = clazz->superClass;
            
        vmContextSetVariant(ctx, ctx->keys.superKey, obj);
        
    }
    
    obj.type = vmVariantTypeObject;
    obj.objectValue = (objv_object_t *) arguments;
    obj.ofClass = NULL;
    
    vmContextSetVariant(ctx, ctx->keys.argumentsKey, obj);
    
    for(i=0;i<op->metaCount;i++){
        meta = vmMetaByOperator(op, i);
        assert(meta->type & (vmMetaTypeArg | vmMetaTypeOperator));
        if(meta->type & vmMetaTypeArg){
            vmContextSetVariant(ctx, vmContextKey(ctx, vmUniqueKeyToString(binary, meta->objectKey)) , vmObjectToVariant(objv_array_objectAt(arguments, i)));
        }
        else if(meta->type & vmMetaTypeOperator){
            rs = vmObjectOperatorExecute(ctx, clazz, object, binary, vmMetaOperatorGet(binary,meta->operatorOffset));
            break;
        }
    }
    
    rs.type = rs.type & ~ vmMetaTypeReturn;
    
    vmContextScopePop(ctx);
    
    return rs;
}

vmVariant vmObjectOperatorExecute(vmContext * ctx,objv_class_t * clazz,objv_object_t * object,void * binary,vmMetaOperator * op){
    vmVariant rs = {vmVariantTypeVoid,0};
    
    switch (op->type) {
        case vmOperatorTypeFunction:
            assert(op->uniqueKey && op->metaCount >0);
        {
            vm_uint32_t i;
			vmMeta * meta = NULL;
            objv_array_t * arguments = objv_array_new(ctx->base.zone, op->metaCount);
			vmVariant obj = {vmVariantTypeVoid,0};
			objv_class_t * objClass = NULL;
            
			meta = vmMetaByOperator(op,0);
            
			obj = vmMetaToVariant(ctx ,clazz,object,binary,meta);
            
			if(obj.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
				return obj;
			}
            
			obj = vmVariantToObject(ctx->base.zone, obj);
            
			for(i=1;i<op->metaCount;i++){
                meta = vmMetaByOperator(op,i);
                objv_array_add(arguments, vmVariantToObject(ctx->base.zone, vmMetaToVariant(ctx ,clazz,object,binary,meta)).objectValue);
			}
            
            if(obj.ofClass){
                objClass = obj.ofClass;
            }
            else if(obj.objectValue){
                objClass = obj.objectValue->isa;
            }
            
            if(objClass && obj.objectValue){
                rs = vmObjectInvoke(objClass,obj.objectValue, vmContextKey(ctx, vmUniqueKeyToString(binary, op->uniqueKey)), arguments);
            }
            
        }
            break;
        case vmOperatorTypeNew:
            assert(op->uniqueKey);
        {
            vm_int32_t i;
			vmMeta * meta = NULL;
			objv_array_t * arguments = NULL;
            objv_class_t * objClass = NULL;
            objv_key_t * uniqueKey = vmContextKey(ctx, vmUniqueKeyToString(binary, op->uniqueKey));
            
			objClass = vmContextGetClass(ctx, uniqueKey);
            
			if(objClass == NULL){
                rs.type = vmVariantTypeObject | vmVariantTypeThrowable;
                rs.objectValue = (objv_object_t *) objv_exception_new(ctx->base.zone,0,"[new] not found class %s", uniqueKey->name);
                break;
			}
            
            if(op->metaCount >0){
                arguments = objv_array_new(ctx->base.zone,  op->metaCount);
                
                for(i=0;i<op->metaCount;i++){
                    meta = vmMetaByOperator(op,i);
                    objv_array_add(arguments, vmVariantToObject(ctx->base.zone, vmMetaToVariant(ctx ,clazz,object,binary,meta)).objectValue);
                }
            }
            
			rs = vmObjectNew(ctx, objClass, arguments);
            
        }
            break;
        case vmOperatorTypeInvoke:
            rs = vmObjectOperatorInvoke(ctx,clazz,object,binary,op);
            break;
        case vmOperatorTypeProperty:
            assert(op->metaCount ==1 || op->metaCount ==2 || op->metaCount ==3);
		{
			vmMeta * meta = vmMetaByOperator(op,0);
			vmVariant obj = {vmVariantTypeVoid,0};
			objv_class_t * objClass = NULL;
			objv_key_t * uniqueKey = NULL;
            
            obj = vmMetaToVariant(ctx,clazz,object,binary,meta);

            if(obj.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
                return obj;
            }
            
			obj = vmVariantToObject(ctx->base.zone, obj);
            
            if(obj.ofClass){
                objClass = obj.ofClass;
            }
            else if(obj.objectValue){
                objClass = obj.objectValue->isa;
            }
            
            if(objClass && obj.objectValue){
			
                if(op->uniqueKey){
                    uniqueKey = vmContextKey(ctx,  vmUniqueKeyToString(binary, op->uniqueKey));
                }
                else{
                    uniqueKey = vmVariantToUniqueKey(ctx, vmMetaToVariant(ctx,clazz,object,binary,vmMetaByOperator(op,1)));
                }
                
                if((op->uniqueKey && op->metaCount ==1) || (!op->uniqueKey && op->metaCount ==2)){
                    rs = vmObjectGetProperty(objClass, obj.objectValue, uniqueKey );
                }
                else{
                    rs = vmObjectSetProperty( objClass, obj.objectValue,uniqueKey, vmMetaToVariant(ctx,clazz,object,binary,vmMetaByOperator(op,op->uniqueKey ? 1:2)));
                }
                    
            }
		}
            break;
        case vmOperatorTypeDec:
        case vmOperatorTypeBeforeInc:
        case vmOperatorTypeBeforeDec:
        case vmOperatorTypeInc:
            assert(op->metaCount == 0 || op->metaCount == 1);
		{
			vmVariant obj = {vmVariantTypeVoid,0},v;
			objv_key_t * name;
			if(op->metaCount ==0){
				
                name = vmContextKey(ctx, vmUniqueKeyToString(binary, op->uniqueKey));
				obj = vmContextVariant(ctx, name);
                
				if(obj.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
					return obj;
				}
                
				if(op->type == vmOperatorTypeInc || op->type == vmOperatorTypeDec){
					rs = obj;
				}
                
				obj.int64Value = vmVariantToInt64( obj);
                
				if(op->type == vmOperatorTypeInc || op->type == vmOperatorTypeBeforeInc){
					obj.int64Value ++;
				}
				else{
					obj.int64Value --;
				}
                
				obj.type = vmVariantTypeInt64;
                
				vmContextSetVariant(ctx, name, obj);
                
				if(op->type == vmOperatorTypeBeforeInc || op->type == vmOperatorTypeBeforeDec){
					rs = obj;
				}
                
				return rs;
			}
			else {
				vmMeta * meta = vmMetaByOperator(op,0);
				obj = vmMetaToVariant(ctx,clazz,object,binary,meta);
                
				if(obj.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
					return obj;
				}
                
				if(op->uniqueKey && (obj.type & vmVariantTypeObject) && obj.objectValue){
                    
					name = vmContextKey(ctx, vmUniqueKeyToString(binary, op->uniqueKey));
                    
					v.int64Value = vmVariantToInt64( vmObjectGetProperty(obj.objectValue->isa, obj.objectValue, name));
                    
					v.type = vmVariantTypeInt64;
                    
					if(op->type == vmOperatorTypeInc || op->type == vmOperatorTypeDec){
						rs = v;
					}
                    
					if(op->type == vmOperatorTypeInc || op->type == vmOperatorTypeBeforeInc){
						v.int64Value ++;
					}
					else{
						v.int64Value --;
					}
                    
					if(op->type == vmOperatorTypeBeforeInc || op->type == vmOperatorTypeBeforeDec){
						rs = v;
					}
                    
					obj = vmObjectSetProperty( obj.objectValue->isa, obj.objectValue, name, v);
                    
					if(obj.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
						return obj;
					}
                    
					return rs;
				}
				else{
                    
					v.int64Value = vmVariantToInt64( obj);
					v.type = vmVariantTypeInt64;
                    
					if(op->type == vmOperatorTypeInc || op->type == vmOperatorTypeDec){
						rs = v;
					}
                    
					if(op->type == vmOperatorTypeInc || op->type == vmOperatorTypeBeforeInc){
						v.int64Value ++;
					}
					else{
						v.int64Value --;
					}
                    
					if(op->type == vmOperatorTypeBeforeInc || op->type == vmOperatorTypeBeforeDec){
						rs = obj;
					}
                    
					return rs;
				}
			}
		}
            break;
        case vmOperatorTypeAdd:
        case vmOperatorTypeSub:
        case vmOperatorTypeMultiplication:
        case vmOperatorTypeDivision:
        case vmOperatorTypeModulus:
        case vmOperatorTypeBitwiseAnd:
        case vmOperatorTypeBitwiseOr:
        case vmOperatorTypeBitwiseExclusiveOr:
        case vmOperatorTypeBitwiseShiftLeft:
        case vmOperatorTypeBitwiseShiftRight:
        case vmOperatorTypeGreater:
        case vmOperatorTypeGreaterEqual:
        case vmOperatorTypeLess:
        case vmOperatorTypeLessEqual:
        case vmOperatorTypeEqual:
        case vmOperatorTypeNotEqual:
        case vmOperatorTypeAbsEqual:
        case vmOperatorTypeAbsNotEqual:
            assert(op->metaCount == 2);
        {
            vmMeta * meta1 = vmMetaByOperator(op,0);
			vmMeta * meta2 = vmMetaByOperator(op,1);
			vmVariant obj1,obj2;
			vmVariantType lowType = vmVariantTypeVoid,highType = vmVariantTypeVoid;
			objv_string_t * str1 = NULL;
			objv_string_t * str2 = NULL;
            
			obj1 = vmMetaToVariant(ctx,clazz,object,binary,meta1);
            
			if(obj1.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
				return obj1;
			}
            
			obj2 = vmMetaToVariant(ctx,clazz,object,binary,meta2);
            
			if(obj2.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
				return obj2;
			}
            
			vmVariantTypeFor(obj1,obj2,&lowType,&highType);
            
			rs.type = vmVariantTypeVoid;
            
			switch(op->type){
				case vmOperatorTypeAdd:
					if(highType ==vmVariantTypeString || highType == vmVariantTypeObject){
                        {
                            objv_mbuf_t mbuf;
                            
                            objv_mbuf_init(& mbuf, 64);
                            
                            vmVariantToStringBuffer( obj1, & mbuf);
                            
                            vmVariantToStringBuffer( obj2, & mbuf);
                            
                            rs.type = vmVariantTypeObject;
                            rs.objectValue = (objv_object_t *)objv_string_new(ctx->base.zone, objv_mbuf_str(& mbuf));
                            
                            objv_mbuf_destroy(& mbuf);
                            
                        }
                    }
					else if(highType == vmVariantTypeDouble){
						rs.type = vmVariantTypeDouble;
						rs.doubleValue = vmVariantToDouble(obj1) + vmVariantToDouble(obj2);
					}
					else{
						rs.type = vmVariantTypeInt64;
						rs.int64Value = vmVariantToInt64(obj1) + vmVariantToInt64(obj2);
					}
					break;
				case vmOperatorTypeSub:
                    
					if(highType == vmVariantTypeDouble || highType == vmVariantTypeString|| highType == vmVariantTypeObject){
						rs.type = vmVariantTypeDouble;
						rs.doubleValue = vmVariantToDouble(obj1) - vmVariantToDouble(obj2);
					}
					else{
						rs.type = vmVariantTypeInt64;
						rs.int64Value = vmVariantToInt64(obj1) - vmVariantToInt64(obj2);
					}
                    break;
				case vmOperatorTypeMultiplication:
					if(highType == vmVariantTypeDouble || highType == vmVariantTypeString || highType == vmVariantTypeObject){
						rs.type = vmVariantTypeDouble;
						rs.doubleValue = vmVariantToDouble(obj1) * vmVariantToDouble(obj2);
					}
					else{
						rs.type = vmVariantTypeInt64;
						rs.int64Value = vmVariantToInt64(obj1) * vmVariantToInt64(obj2);
					}
					break;
				case vmOperatorTypeDivision:
					if(highType == vmVariantTypeDouble || highType == vmVariantTypeString || highType == vmVariantTypeObject){
						rs.type = vmVariantTypeDouble;
						rs.doubleValue =  vmVariantToDouble(obj2);
                        
						if(rs.doubleValue == 0.0){
                            rs.type = vmVariantTypeBoolean;
                            rs.booleanValue = objv_false;
						}
						else{
							rs.doubleValue =vmVariantToDouble(obj1) / rs.doubleValue;
						}
					}
					else{
						rs.type = vmVariantTypeInt64;
						rs.int64Value = vmVariantToInt64(obj2);
                        
						if(rs.int64Value ==0){
                            rs.type = vmVariantTypeBoolean;
                            rs.booleanValue = objv_false;
						}
						else{
							rs.int64Value = vmVariantToInt64(obj1) / rs.int64Value;
						}
					}
					break;
				case vmOperatorTypeModulus:
                    
					rs.type = vmVariantTypeInt64;
					rs.int64Value = vmVariantToInt64(obj2);
                    
					if(rs.int64Value ==0){
                        rs.type = vmVariantTypeBoolean;
                        rs.booleanValue = objv_false;
					}
					else{
						rs.int64Value =vmVariantToInt64(obj1) % rs.int64Value;
					}
                    
					break;
				case vmOperatorTypeBitwiseAnd:
					rs.type = vmVariantTypeInt64;
					rs.int64Value = vmVariantToInt64(obj1) & vmVariantToInt64(obj2);
					break;
				case vmOperatorTypeBitwiseOr:
					rs.type = vmVariantTypeInt64;
					rs.int64Value = vmVariantToInt64(obj1) | vmVariantToInt64(obj2);
					break;
				case vmOperatorTypeBitwiseExclusiveOr:
					rs.type = vmVariantTypeInt64;
					rs.int64Value = vmVariantToInt64(obj1) ^ vmVariantToInt64(obj2);
					break;
				case vmOperatorTypeBitwiseShiftLeft:
					rs.type = vmVariantTypeInt64;
					rs.int64Value = vmVariantToInt64(obj1) << vmVariantToInt64(obj2);
					break;
				case vmOperatorTypeBitwiseShiftRight:
					rs.type = vmVariantTypeInt64;
					rs.int64Value = vmVariantToInt64(obj1) >> vmVariantToInt64(obj2);
					break;
				case vmOperatorTypeGreater:
					rs.type = vmVariantTypeBoolean;
                    
					if(highType == vmVariantTypeString || lowType == vmVariantTypeString){
                        str1 = vmVariantToString(ctx->base.zone, obj1);
                        str2 = vmVariantToString(ctx->base.zone, obj2);
                        if(str1 && str2){
                            rs.booleanValue = strcmp(str1->UTF8String, str2->UTF8String) > 0;
                        }
                        else if(str1){
                            rs.booleanValue = objv_true;
                        }
                        else if(str2){
                            rs.booleanValue = objv_false;
                        }
                        else{
                            rs.booleanValue = objv_false;
                        }
					}
					else{
						rs.booleanValue = vmVariantToDouble(obj1) > vmVariantToDouble(obj2);
					}
                    
					break;
				case vmOperatorTypeGreaterEqual:
                    
					rs.type = vmVariantTypeBoolean;
                    
					if(highType == vmVariantTypeString || lowType == vmVariantTypeString){
                        str1 = vmVariantToString(ctx->base.zone, obj1);
                        str2 = vmVariantToString(ctx->base.zone, obj2);
                        if(str1 && str2){
                            rs.booleanValue = strcmp(str1->UTF8String, str2->UTF8String) >= 0;
                        }
                        else if(str1){
                            rs.booleanValue = objv_true;
                        }
                        else if(str2){
                            rs.booleanValue = objv_false;
                        }
                        else{
                            rs.booleanValue = objv_true;
                        }
					}
					else{
						rs.booleanValue = vmVariantToDouble(obj1) >= vmVariantToDouble(obj2);
					}

                    break;
				case vmOperatorTypeLess:
					rs.type = vmVariantTypeBoolean;
                    
					if(highType == vmVariantTypeString || lowType == vmVariantTypeString){
                        str1 = vmVariantToString(ctx->base.zone, obj1);
                        str2 = vmVariantToString(ctx->base.zone, obj2);
                        if(str1 && str2){
                            rs.booleanValue = strcmp(str1->UTF8String, str2->UTF8String) < 0;
                        }
                        else if(str1){
                            rs.booleanValue = objv_false;
                        }
                        else if(str2){
                            rs.booleanValue = objv_true;
                        }
                        else{
                            rs.booleanValue = objv_false;
                        }
					}
					else{
						rs.booleanValue = vmVariantToDouble(obj1) < vmVariantToDouble(obj2);
					}
					break;
				case vmOperatorTypeLessEqual:
					rs.type = vmVariantTypeBoolean;
                    
					if(highType == vmVariantTypeString || lowType == vmVariantTypeString){
                        str1 = vmVariantToString(ctx->base.zone, obj1);
                        str2 = vmVariantToString(ctx->base.zone, obj2);
                        if(str1 && str2){
                            rs.booleanValue = strcmp(str1->UTF8String, str2->UTF8String) <= 0;
                        }
                        else if(str1){
                            rs.booleanValue = objv_false;
                        }
                        else if(str2){
                            rs.booleanValue = objv_true;
                        }
                        else{
                            rs.booleanValue = objv_false;
                        }
					}
					else{
						rs.booleanValue = vmVariantToDouble(obj1) <= vmVariantToDouble(obj2);
					}
					break;
				case vmOperatorTypeAbsEqual:
					if(obj1.type != obj2.type){
						rs.type = vmVariantTypeBoolean;
						rs.booleanValue = objv_false;
						break;
					}
				case vmOperatorTypeEqual:
					rs.type = vmVariantTypeBoolean;
                    if(lowType == vmVariantTypeObject && highType == vmVariantTypeObject){
                        if(obj1.objectValue && obj2.objectValue){
                            rs.booleanValue = objv_object_equal(obj1.objectValue->isa, obj1.objectValue, obj2.objectValue);
                        }
                        else{
                            rs.booleanValue = obj1.objectValue == obj2.objectValue;
                        }
                    }
                    else if(highType == vmVariantTypeString || highType == vmVariantTypeObject){
                        str1 = objv_object_stringValue(obj1.objectValue, NULL);
                        str2 = objv_object_stringValue(obj2.objectValue, NULL);
                        if(str1 && str2){
                            rs.booleanValue = strcmp(str1->UTF8String, str2->UTF8String) ==0;
                        }
                        else {
                            rs.booleanValue = str1 == str2;
                        }
					}
					else{
						rs.booleanValue = vmVariantToDouble(obj1) == vmVariantToDouble(obj2);
					}
					break;
				case vmOperatorTypeAbsNotEqual:
					if(obj1.type != obj2.type){
						rs.type = vmVariantTypeBoolean;
						rs.booleanValue = objv_true;
						break;
					}
				case vmOperatorTypeNotEqual:
					rs.type = vmVariantTypeBoolean;
                    if(lowType == vmVariantTypeObject && highType == vmVariantTypeObject){
                        if(obj1.objectValue && obj2.objectValue){
                            rs.booleanValue = ! objv_object_equal(obj1.objectValue->isa, obj1.objectValue, obj2.objectValue);
                        }
                        else{
                            rs.booleanValue = obj1.objectValue != obj2.objectValue;
                        }
                    }
					else if(highType == vmVariantTypeString || highType == vmVariantTypeObject){
						str1 = objv_object_stringValue(obj1.objectValue, NULL);
                        str2 = objv_object_stringValue(obj2.objectValue, NULL);
                        if(str1 && str2){
                            rs.booleanValue = strcmp(str1->UTF8String, str2->UTF8String) !=0;
                        }
                        else {
                            rs.booleanValue = str1 != str2;
                        }
					}
					else{
						rs.booleanValue = vmVariantToDouble(obj1) != vmVariantToDouble(obj2);
					}
					break;
				default:
					assert(0);
			}
        }
            break;
        case vmOperatorTypeAnd:
        case vmOperatorTypeOr:
            assert(op->metaCount == 2);
        {
            vmMeta * meta1 = vmMetaByOperator(op,0);
			vmMeta * meta2 = vmMetaByOperator(op,1);
			vmVariant obj1,obj2;
            
			rs.type = vmVariantTypeBoolean;
            
			obj1 = vmMetaToVariant(ctx,clazz,object,binary,meta1);
            
			if(obj1.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
				return obj1;
			}
            
			rs.booleanValue = vmVariantToBoolean(obj1);
            
			switch(op->type){
				case vmOperatorTypeAnd:
					if(!rs.booleanValue){
						return rs;
					}
					break;
				case vmOperatorTypeOr:
					if(rs.booleanValue){
						return rs;
					}
					break;
				default:
					assert(0);
			}
            
            if((op->type == vmOperatorTypeAnd && rs.booleanValue) && (op->type == vmOperatorTypeOr && ! rs.booleanValue)){
                
                obj2 = vmMetaToVariant(ctx,clazz,object,binary,meta2);
                
                if(obj2.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
                    return obj2;
                }
                
                rs.booleanValue = vmVariantToBoolean( obj2);
                
            }

        }
            break;
        case vmOperatorTypeNot:
        case vmOperatorTypeBitwiseComplement:
        case vmOperatorTypeAntiNumber:
            assert(op->metaCount == 0 && op->uniqueKey || op->metaCount == 1);
        {
            vmMeta * meta;
			vmVariant obj = {vmVariantTypeVoid,0};
            
            if(op->metaCount == 0){
                obj = vmContextVariant(ctx, vmContextKey(ctx, vmUniqueKeyToString(binary, op->uniqueKey)));
            }
            else{
                meta = vmMetaByOperator(op,0);
                obj = vmMetaToVariant(ctx,clazz,object,binary,meta);
            }
            
			if(obj.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
				return obj;
			}
            
			switch (op->type) {
				case vmOperatorTypeNot:
					rs.type = vmVariantTypeBoolean;
					rs.booleanValue = ! vmVariantToBoolean( obj);
					break;
				case vmOperatorTypeBitwiseComplement:
					rs.type = vmVariantTypeInt64;
					rs.int64Value = ~ vmVariantToInt64(obj);
					break;
				case vmOperatorTypeAntiNumber:
					if(obj.type == vmVariantTypeDouble){
						rs.type = vmVariantTypeDouble;
						rs.doubleValue = - vmVariantToDouble(obj);
					}
					else{
						rs.type = vmVariantTypeInt64;
						rs.int64Value = - vmVariantToInt64(obj);
					}
					break;
				default:
					assert(0);
					break;
			}
    
        }
            break;
        case vmOperatorTypeAssign:
            assert( (op->uniqueKey && (op->metaCount ==1 || op->metaCount ==2)) || (!op->uniqueKey && op->metaCount ==3));
        {
            vmMeta * meta = vmMetaByOperator(op,0);
			vmVariant obj = {vmVariantTypeVoid,0};
			vmVariant value = {vmVariantTypeVoid,0};
			objv_class_t * objClass = NULL;
			objv_key_t * uniqueKey = NULL;
            
			if(op->uniqueKey){
				if(op->metaCount ==1){
					
                    obj = vmMetaToVariant(ctx,clazz,object,binary,meta);
                    
					if(obj.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
						return obj;
					}
                    
					vmContextSetVariant(ctx, vmContextKey(ctx, vmUniqueKeyToString(binary, op->uniqueKey)), obj);
                    
					return obj;
				}
				else if(op->metaCount == 2){
					vmMeta * meta1 = vmMetaByOperator(op, 0);
					vmMeta * meta2 = vmMetaByOperator(op, 1);
					vmVariant obj1 = {vmVariantTypeVoid,0};
					vmVariant obj2 = {vmVariantTypeVoid,0};
                    
                    obj1 = vmMetaToVariant(ctx, clazz, object, binary, meta1);
                    
                    if(obj1.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
                        return obj1;
                    }
                    
                    obj2 = vmMetaToVariant(ctx, clazz, object, binary, meta2);
                    
                    if(obj2.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
                        return obj2;
                    }

                    if(obj1.type & vmVariantTypeObject){
                
                        rs = vmObjectSetProperty(obj1.objectValue->isa, obj1.objectValue, vmContextKey(ctx, vmUniqueKeyToString(binary, op->uniqueKey)), obj2);
                        
                        if(rs.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
                            return rs;
                        }
                        
                    }
                    
					return obj2;
				}
			}
			else{
                
				obj = vmMetaToVariant(ctx,clazz,object,binary,meta);
                
                if(obj.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
                    return obj;
                }
                
				if(obj.ofClass){
					objClass = obj.ofClass;
				}
				else if(obj.objectValue){
					objClass = obj.objectValue->isa;
				}
                
                uniqueKey = vmVariantToUniqueKey(ctx, vmMetaToVariant(ctx,clazz,object,binary,vmMetaByOperator(op,1)));
                
                value = vmMetaToVariant(ctx,clazz,object,binary,vmMetaByOperator(op,2));
                
                if(value.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
                    return value;
                }
                
                if(objClass && obj.objectValue){
                    rs = vmObjectSetProperty(objClass, obj.objectValue,uniqueKey, value);
                    
                    if(rs.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
                        return rs;
                    }
                }
                
                return value;
			}
        }
            break;
        case vmOperatorTypeIfElse:
            assert(op->metaCount ==3);
        {
            vmMeta * meta1 = vmMetaByOperator(op, 0);
			vmMeta * meta2 = vmMetaByOperator(op, 1);
			vmMeta * meta3 = vmMetaByOperator(op, 2);
			vmVariant obj1 = {vmVariantTypeVoid,0};
            
            obj1 = vmMetaToVariant(ctx, clazz, object, binary, meta1);
            
            if(obj1.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
                return obj1;
            }
            
			if(vmVariantToBoolean( obj1)){
                return vmMetaToVariant(ctx, clazz, object, binary, meta2);
			}
			else{
				return vmMetaToVariant(ctx, clazz, object, binary, meta3);
			}

        }
            break;
        case vmOperatorTypeIfElseIfElse:
            assert(op->metaCount >=2);
        {
            vmMeta * meta = vmMetaByOperator(op, 0);
			vmVariant obj = {vmVariantTypeVoid,0};
			vm_int32_t i=0;
            
            obj = vmMetaToVariant(ctx,clazz,object,binary,meta);
            
			if(obj.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
				return obj;
			}
            
			if(vmVariantToBoolean(obj)){
                
				meta = vmMetaByOperator(op,++i);
                
                obj = vmMetaToVariant(ctx,clazz,object,binary,meta);
                
                if(obj.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
                    return obj;
                }
                
			}
			else{
				i++;
				while( ++i <op->metaCount){
					
                    assert(i+1 < op->metaCount);
                    
					meta = vmMetaByOperator(op, i);
                    
					obj = vmMetaToVariant(ctx,clazz,object,binary,meta);
					
                    if(obj.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
						return obj;
					}
                    
					if(vmVariantToBoolean(obj)){
						meta = vmMetaByOperator(op,++i);
						if(meta->type & vmMetaTypeReturn){
							obj = vmMetaToVariant(ctx,clazz,object,binary,meta);
							obj.type = (vmVariantType)(obj.type | vmMetaTypeReturn);
							return obj;
						}
						else if(meta->type & vmMetaTypeOperator){
                            
							obj = vmObjectOperatorExecute(ctx, clazz, object, binary, vmMetaOperatorGet(binary, meta->operatorOffset));
                            
							if(obj.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
								return obj;
							}
						}
                        
						break;
					}
					else{
						i++;
					}
				}
			}
        }
            break;
        case vmOperatorTypeFor:
            assert(op->metaCount ==4);
        {
            vmMeta * meta1 = vmMetaByOperator(op, 0);
			vmMeta * meta2 = vmMetaByOperator(op, 1);
			vmMeta * meta3 = vmMetaByOperator(op, 2);
			vmMeta * meta4 = vmMetaByOperator(op, 3);
			vmVariant obj = {vmVariantTypeVoid,0};
            
			obj = vmMetaToVariant(ctx,clazz,object,binary,meta1);
            
			if(obj.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
				return obj;
			}
            
			obj = vmMetaToVariant(ctx, clazz, object, binary, meta2);
            
			if(obj.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
				return obj;
			}
            
			while(vmVariantToBoolean(obj)){
                
				if(meta4->type & vmMetaTypeReturn){
					obj = vmMetaToVariant(ctx,clazz,object,binary,meta4);
					obj.type =(vmVariantType)( obj.type | vmMetaTypeReturn);
					return obj;
				}
				else if(meta4->type & vmMetaTypeBreak){
					break;
				}
				else if(meta4->type & vmMetaTypeContinue){
					obj = vmMetaToVariant(ctx,clazz,object,binary,meta3);
                    
					if(obj.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
						return obj;
					}
                    
					obj = vmMetaToVariant(ctx, clazz, object, binary, meta2);
                    
					if(obj.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
						return obj;
					}
                    
					continue;
				}
				else if(meta4->type & vmMetaTypeOperator){
                    
					obj = vmObjectOperatorExecute(ctx, clazz, object, binary, vmMetaOperatorGet(binary, meta4->operatorOffset));
                    
					if(obj.type & (vmVariantTypeThrowable | vmMetaTypeReturn)){
						return obj;
					}
                    
					if(obj.type & vmMetaTypeBreak){
						break;
					}
					else if(obj.type & vmMetaTypeContinue){
                        
						obj = vmMetaToVariant(ctx,clazz,object,binary,meta3);
                        
						if(obj.type & (vmVariantTypeThrowable )){
							return obj;
						}
                        
						obj = vmMetaToVariant(ctx, clazz, object, binary, meta2);
                        
						if(obj.type & (vmVariantTypeThrowable )){
							return obj;
						}
                        
						continue;
					}
				}
                
				obj = vmMetaToVariant(ctx,clazz,object,binary,meta3);
                
				if(obj.type & (vmMetaTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
					return obj;
				}
                
				obj = vmMetaToVariant(ctx, clazz, object, binary, meta2);
                
				if(obj.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
					return obj;
				}
			}
        }
            break;
        case vmOperatorTypeWhile:
            assert(op->metaCount ==2);
        {
            vmMeta * meta = vmMetaByOperator(op, 0);
			vmMeta * meta2 = vmMetaByOperator(op,1);
			vmVariant obj = {vmVariantTypeVoid,0};
            
			obj = vmMetaToVariant(ctx,clazz,object,binary,meta);
            
			if(obj.type & (vmMetaTypeThrowable)){
				return obj;
			}
            
			while(vmVariantToBoolean(obj)){
                
				if(meta2->type & vmMetaTypeReturn){
					obj = vmMetaToVariant(ctx,clazz,object,binary,meta2);
					obj.type = (vmVariantType)(obj.type | vmMetaTypeReturn);
					return obj;
				}
				else if(meta2->type & vmMetaTypeBreak){
					break;
				}
				else if(meta2->type & vmMetaTypeContinue){
					obj = vmMetaToVariant(ctx,clazz,object,binary,meta);
                    
					if(obj.type & (vmMetaTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
						return obj;
					}
                    
					continue;
				}
				else if(meta2->type & vmMetaTypeOperator){
                    
					obj = vmObjectOperatorExecute(ctx, clazz, object, binary, vmMetaOperatorGet(binary, meta2->operatorOffset));
                    
					if(obj.type & (vmMetaTypeThrowable | vmMetaTypeReturn)){
						return obj;
					}
                    
					if(obj.type & vmMetaTypeBreak){
						break;
					}
					else if(obj.type & vmMetaTypeContinue){
                        
						obj = vmMetaToVariant(ctx,clazz,object,binary,meta);
                        
						if(obj.type & (vmMetaTypeThrowable)){
							return obj;
						}
                        
						continue;
					}
				}
                
				obj = vmMetaToVariant(ctx,clazz,object,binary,meta);
                
				if(obj.type & (vmMetaTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
					return obj;
				}
                
			}
        }
            break;
        case vmOperatorTypeForeach:
            assert(op->metaCount == 2 && op->uniqueKey);
        {
            vmMeta * meta = vmMetaByOperator(op, 0);
			vmMeta * meta2 = vmMetaByOperator(op,1);
			vmVariant obj = {vmVariantTypeVoid,0};
			vmVariant v = {vmVariantTypeVoid,0};
            objv_key_t * uniqueKey;
            objv_iterator_t * iterator;
            objv_object_t * item;
            objv_class_t * objectClass = NULL;
            
            uniqueKey = vmContextKey(ctx, vmUniqueKeyToString(binary, op->uniqueKey));
            
			obj = vmMetaToVariant(ctx,clazz,object,binary,meta);
            
			if(obj.type & (vmMetaTypeThrowable | vmMetaTypeReturn )){
				return obj;
			}
            
			if(obj.type & vmVariantTypeObject){
				if(obj.objectValue){
					if(obj.ofClass){
						objectClass = obj.ofClass;
					}
					else{
						objectClass = obj.objectValue->isa;
					}
				}
                
                if(objectClass && obj.objectValue){
                    
                    iterator = objv_object_iterator(objectClass, obj.objectValue);
                    
                    while(iterator && (item = objv_iterator_next(iterator->base.isa,iterator))){
                        
                        v.type = vmVariantTypeObject;
                        v.objectValue = item;
                        
                        vmContextSetVariant(ctx, uniqueKey, v);
                        
                        if(meta2->type & vmMetaTypeReturn){
                            
                            obj = vmMetaToVariant(ctx,clazz,object,binary,meta2);
                            
                            if(obj.type & (vmMetaTypeThrowable | vmMetaTypeReturn)){
                                return obj;
                            }
                            
                            obj.type = (vmVariantType)( obj.type | vmMetaTypeReturn);
                
                            return obj;
                        }
                        else if(meta2->type & vmMetaTypeBreak){
                            break;
                        }
                        else if(meta2->type & vmMetaTypeContinue){
                            continue;
                        }
                        else if(meta2->type & vmMetaTypeOperator){
                            
                            obj = vmObjectOperatorExecute(ctx, clazz, object, binary, vmMetaOperatorGet(binary, meta2->operatorOffset));
                            
                            if(obj.type & (vmMetaTypeThrowable | vmMetaTypeReturn)){
                                return obj;
                            }
                            
                            if(obj.type & vmMetaTypeBreak){
                                break;
                            }
                            else if(obj.type & vmMetaTypeContinue){
                                continue;
                            }
                        }
                    }
                    
                }
			}
        
        }
            break;
        case vmOperatorTypeVar:
        case vmOperatorTypeVarWeak:
            assert(op->metaCount ==0 || op->metaCount ==1);
        {
            vmVariant obj = {vmVariantTypeVoid,0};
			if(op->metaCount ==1){
				vmMeta * meta = vmMetaByOperator(op, 0);
                
				obj = vmMetaToVariant(ctx, clazz, object, binary, meta);
                
				if(obj.type & (vmMetaTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
					return obj;
				}
			}
            
			vmContextSetVariant(ctx, vmContextKey(ctx, vmUniqueKeyToString(binary, op->uniqueKey)), obj);
        }
            break;
        case vmOperatorTypeIs:
            assert(op->metaCount ==2);
        {
            vmVariant obj1 = {vmVariantTypeVoid,0};
			vmVariant obj2 = {vmVariantTypeVoid,0};
			objv_key_t * uniqueKey;
			vmMeta * meta1 = vmMetaByOperator(op, 0);
			vmMeta * meta2 = vmMetaByOperator(op, 1);
			objv_class_t * objClass;
            
			obj1 = vmMetaToVariant(ctx,clazz,object,binary,meta1);
            
			if(obj1.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
				return obj1;
			}
            
			obj2 = vmMetaToVariant(ctx,clazz,object,binary,meta2);
            
			if(obj2.type & (vmVariantTypeThrowable | vmMetaTypeReturn | vmMetaTypeBreak | vmMetaTypeContinue)){
				return obj2;
			}
            
			obj1 = vmVariantToObject(ctx->base.zone, obj1);
            
			uniqueKey = vmVariantToUniqueKey(ctx, obj2);
            
			objClass = vmContextGetClass(ctx, uniqueKey);
            
			if(objClass == NULL){
                rs.type = vmVariantTypeObject | vmVariantTypeThrowable;
				rs.objectValue = (objv_object_t *) objv_exception_new(ctx->base.zone, 0, "[is] not found class %s",uniqueKey->name);
			}
			else{
				rs.type = vmVariantTypeBoolean;
				rs.booleanValue = objv_object_isKindOfClass(obj1.objectValue, objClass);
			}
        }
            break;
        case vmOperatorTypeTryCatchFinally:
            assert(op->metaCount >=1);
        {
            vmMeta * meta = vmMetaByOperator(op, 0);
			vmMeta * meta1, *meta2, * finallyMeta = NULL;
			vmVariant obj = {vmVariantTypeVoid,0};
			objv_key_t * uniqueKey;
			objv_class_t * objClass;
			vm_int32_t i=0;
            
            obj = vmMetaToVariant(ctx, clazz, object, binary, meta);
            
			if(obj.type & (vmMetaTypeReturn)){
				return obj;
			}
            
            
			for(i=1;i<op->metaCount;i++){
				meta = vmMetaByOperator(op, i);
				if(meta->type & vmMetaTypeFinally){
					finallyMeta = meta;
					break;
				}
			}
            
			if(obj.type & vmMetaTypeThrowable){
				obj = vmVariantToObject(ctx->base.zone, obj);
				obj.type = (vmVariantType)(obj.type | vmMetaTypeThrowable);
				for(i=1;i<op->metaCount;i++){
					meta = vmMetaByOperator(op, i);
					if(meta->type & vmMetaTypeCatch){
						assert(i + 2 <op->metaCount);
						meta1 = vmMetaByOperator(op, i+1);
						meta2 = vmMetaByOperator(op, i+2);
						assert(meta->type & vmVariantTypeString && meta1->type & vmVariantTypeString);
						uniqueKey = vmVariantToUniqueKey(ctx, vmMetaToVariant(ctx,clazz,object,binary,meta));
						objClass = vmContextGetClass(ctx, uniqueKey);
                        
						if(objClass && objv_object_isKindOfClass(obj.objectValue, objClass)){
                            
							uniqueKey = vmVariantToUniqueKey(ctx, vmMetaToVariant(ctx,clazz,object,binary,meta1));
                            
							obj.type = (vmVariantType)(obj.type & (~ vmMetaTypeThrowable));
                            
							vmContextSetVariant(ctx, uniqueKey, obj);
                            
							obj = vmMetaToVariant(ctx,clazz,object,binary,meta2);
                            
							break;
						}
						else{
							i +=2;
						}
					}
				}
			}
            
			if(finallyMeta){
				vmMetaToVariant(ctx,clazz,object,binary,finallyMeta);
			}
            
            if(obj.type & (vmMetaTypeReturn)){
				return obj;
			}
            
			return obj;
        }
            break;
        case vmOperatorTypeThrow:
            assert(op->metaCount ==1);
        {
            vmMeta * meta = vmMetaByOperator(op, 0);
			vmVariant obj = {vmVariantTypeVoid,0};
			obj = vmMetaToVariant(ctx,clazz,object,binary,meta);
            
			obj.type =(vmVariantType)( obj.type | vmMetaTypeThrowable);
            
			return obj;
        }
            break;
        default:
            assert(0);
            break;
    }
    
    return rs;
}

