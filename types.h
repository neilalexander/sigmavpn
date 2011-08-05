//
//  types.h
//  Sigma
//
//  Created by Neil Alexander on 05/08/2011.
//  Copyright 2011. All rights reserved.
//

typedef struct sigma_intf
{
	int (*init) ();
	int (*set) ();
	long (*read) ();
	long (*write) ();
	
	int filedesc;
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

typedef struct sigma_sessionlist
{
	sigma_session session;
	struct sigma_sessionlist* next;
}
sigma_sessionlist;

extern sigma_proto* proto_descriptor();
static int proto_init();
static int proto_set(sigma_proto *instance, char* param, char* value);
static int proto_decode(sigma_proto *instance, unsigned char* input, unsigned char* output, int len);
static int proto_encode(sigma_proto *instance, unsigned char* input, unsigned char* output, int len);

extern sigma_intf* intf_descriptor();
static int intf_init();
static int intf_set(sigma_intf *instance, char* param, void* value);
static long intf_write(sigma_intf *instance, char* input, long len);
static long intf_read(sigma_intf *instance, char* output, long len);
