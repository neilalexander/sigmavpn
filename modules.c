//
//  modules.c
//  Sigma
//
//  Created by Neil Alexander on 05/08/2011.
//  Copyright 2011. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "types.h"
#include "modules.h"

sigma_proto* loadproto(char* protoname)
{
	char* path = malloc(sizeof(char) * 32);
	sprintf(path, "build/proto_%s.o", protoname);
	
	void* proto_lib = dlopen(path, RTLD_NOW);
	
	if (!proto_lib)
	{
		printf("Unable to load protocol %s: %s\n", protoname, dlerror());
		exit(-1);
	}
	
	void* init = dlsym(proto_lib, "proto_descriptor");
	
	if (!init)
	{
		printf("Unable to load protocol %s: %s\n", protoname, dlerror());
		exit(-1);
	}
	
	sigma_proto* proto = ((sigma_proto* (*)()) init)();
	
	return proto;
}

sigma_intf* loadinterface(char* intfname)
{
	char* path = malloc(sizeof(char) * 32);
	sprintf(path, "build/intf_%s.o", intfname);
	
	void* intf_lib = dlopen(path, RTLD_NOW);
	
	if (!intf_lib)
	{
		printf("Unable to load interface %s: %s\n", intfname, dlerror());
		exit(-1);
	}
	
	void* init = dlsym(intf_lib, "intf_descriptor");
	
	if (!init)
	{
		printf("Unable to load interface %s: %s\n", intfname, dlerror());
		exit(-1);
	}
	
	sigma_intf* intf = ((sigma_intf* (*)()) init)();
	
	return intf;
}