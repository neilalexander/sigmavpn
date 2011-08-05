//
//  intf_tuntap.h
//  Sigma
//
//  Created by Neil Alexander on 05/08/2011.
//  Copyright 2011. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "../types.h"

typedef struct sigma_intf_tuntap
{
	sigma_intf baseintf;
	
	int filedesc;
	char nodename[16];
	int tunmode;
}
sigma_intf_tuntap;

static int intf_write(sigma_intf *instance, char* input, int len)
{
	return len;
}

static int intf_read(sigma_intf *instance, char* output, int len)
{
	return len;
}

static int intf_init(sigma_intf* instance)
{
	sigma_intf_tuntap* tuntap = (sigma_intf_tuntap*) instance;
	
	if (!tuntap->nodename) strcpy(tuntap->nodename, "/dev/tun0");
	
	if ((tuntap->filedesc = open(tuntap->nodename, O_RDWR)) < 0)
	{
		fprintf(stderr, "Unable to open tuntap device '%s'\n", tuntap->nodename);
		return -1;
	}
	
	printf("Opened tun device\n");
	
	return 0;
}

static int intf_set(sigma_intf* instance, char* param, void* value)
{	
	sigma_intf_tuntap* tuntap = (sigma_intf_tuntap*) instance;
	
	if (strcmp(param, "nodename") == 0)
		memcpy(tuntap->nodename, (char*) value, 16);
	
	if (strcmp(param, "tunmode") == 0)
		tuntap->tunmode = *(int*) value;
	
	return 0;
}

extern sigma_intf* intf_descriptor()
{
	sigma_intf_tuntap* intf_tuntap = malloc(sizeof(sigma_intf_tuntap));
	
	intf_tuntap->baseintf.init = intf_init;
	intf_tuntap->baseintf.read = intf_read;
	intf_tuntap->baseintf.write = intf_write;
	intf_tuntap->baseintf.set = intf_set;
	
	return (sigma_intf*) intf_tuntap;
}
