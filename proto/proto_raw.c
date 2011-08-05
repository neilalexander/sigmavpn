//
//  proto_raw.c
//  Sigma raw protocol code
//
//  Created by Neil Alexander on 05/08/2011.
//  Copyright 2011. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../types.h"

static int proto_encode(sigma_proto *instance, unsigned char* input, unsigned char* output, int len)
{
	memcpy(output, input, len);
	return len;
}

static int proto_decode(sigma_proto *instance, unsigned char* input, unsigned char* output, int len)
{
	memcpy(output, input, len);
	return len;
}

static int proto_init(sigma_proto *instance)
{
	return 0;
}

extern sigma_proto* proto_descriptor()
{
	sigma_proto* proto_raw = malloc(sizeof(sigma_proto));
	
	proto_raw->encrypted = 0;
	proto_raw->stateful = 0;
	proto_raw->init = proto_init;
	proto_raw->encode = proto_encode;
	proto_raw->decode = proto_decode;
	proto_raw->state = 0;

	return proto_raw;
}
