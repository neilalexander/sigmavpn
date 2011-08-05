//
//  intf_dummy.c
//  Sigma
//
//  Created by Neil Alexander on 05/08/2011.
//  Copyright 2011. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include "../types.h"

static long intf_write(sigma_intf *instance, char* input, long len)
{
	printf("Dummy output: %s\n", input);
	return len;
}

static long intf_read(sigma_intf *instance, char* output, long len)
{
	return len;
}

static int intf_init()
{
	return 0;
}

extern sigma_intf* intf_descriptor()
{
	sigma_intf* intf_dummy = malloc(sizeof(sigma_intf));
	
	intf_dummy->init = intf_init;
	intf_dummy->read = intf_read;
	intf_dummy->write = intf_write;
//	intf_dummy->set = intf_set;
	
	return intf_dummy;
}
