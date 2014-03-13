//
//  CLLoader.c
//  objv
//
//  Created by zhang hailong on 14-3-10.
//  Copyright (c) 2014年 hailong.org. All rights reserved.
//

#include "objv_os.h"
#include "objv.h"
#include "CLMonitorService.h"

void CLLoader(void){
    
    
    objv_class_initialize(OBJV_CLASS(CLMonitorService));
    
}

