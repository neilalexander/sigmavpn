//
//  types.h
//  Sigma
//
//  Created by Neil Alexander on 05/08/2011.
//  Copyright 2011. All rights reserved.
//

#ifndef Sigma_session_h
#define Sigma_session_h

typedef struct sigma_intf
{
	int (*init) ();
	int (*set) ();
	int (*read) ();
	int (*write) ();
	
	int data;
} sigma_intf;

typedef struct sigma_proto
{
	int	encrypted;
	int	stateful;
	int state;
	
	int (*init) ();
	int (*set) ();
	int	(*encode) ();
	int	(*decode) ();
}
sigma_proto;

typedef struct sigma_session
{
	sigma_proto* proto;
	sigma_intf* local;
	sigma_intf* remote;
	
	int maxpacketsize;
}
sigma_session;

extern sigma_proto* proto_descriptor();
static int proto_init();
static int proto_set(sigma_proto *instance, char* param, char* value);
static int proto_decode(sigma_proto *instance, unsigned char* input, unsigned char* output, int len);
static int proto_encode(sigma_proto *instance, unsigned char* input, unsigned char* output, int len);

extern sigma_intf* intf_descriptor();
static int intf_init();
static int intf_set(sigma_intf *instance, char* param, void* value);
static int intf_write(sigma_intf *instance, char* input, int len);
static int intf_read(sigma_intf *instance, char* output, int len);

#endif
