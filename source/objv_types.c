//
//  objv_types.c
//  objv
//
//  Created by zhang hailong on 14-1-29.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#include "objv_os.h"
#include "objv.h"

objv_type_t objv_type_int = {"i",sizeof(int)};       //i
objv_type_t objv_type_uint = {"I",sizeof(unsigned int)};      //I
objv_type_t objv_type_long = {"l",sizeof(long)};      //l
objv_type_t objv_type_ulong = {"L",sizeof(unsigned long)};     //L
objv_type_t objv_type_longLong = {"q",sizeof(long long)};  //q
objv_type_t objv_type_ulongLong = {"Q",sizeof(unsigned long long)}; //Q
objv_type_t objv_type_float = {"f",sizeof(float)};     //f
objv_type_t objv_type_double = {"d",sizeof(double)};    //d
objv_type_t objv_type_boolean = {"b",sizeof(objv_boolean_t)};      //b
objv_type_t objv_type_void = {"v",0};      //v
objv_type_t objv_type_ptr = {"*",sizeof(void *)};       //*
objv_type_t objv_type_object = {"@",sizeof(objv_object_t *)};          //@



