//
//  types.h
//  Sigma type headers
//
//  Copyright (c) 2011, Neil Alexander T.
//  All rights reserved.
// 
//  Redistribution and use in source and binary forms, with
//  or without modification, are permitted provided that the following
//  conditions are met:
// 
//  - Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//  - Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//

#define MAX_BUFFER_SIZE 1546

typedef struct sigma_conf
{
	char modulepath[128];
	char configfile[128];
}
sigma_conf;

sigma_conf* conf;

typedef struct sigma_intf
{
	int (*init) ();
	int (*set) ();
	long (*read) ();
	long (*write) ();
	int (*reload) ();
	
	int filedesc;
}
sigma_intf;

typedef struct sigma_proto
{
	int	encrypted;
	int	stateful;
	int state;
	
	int (*init) ();
	int (*set) ();
	int	(*encode) ();
	int	(*decode) ();
	int	(*reload) ();
}
sigma_proto;

typedef struct sigma_session
{
	char sessionname[32];
	sigma_proto* proto;
	sigma_intf* local;
	sigma_intf* remote;
}
sigma_session;

typedef struct sigma_sessionlist
{
	sigma_session session;
	struct sigma_sessionlist* next;
}
sigma_sessionlist;

void* sessionwrapper(void* param);
int runsession(sigma_session* session);

extern sigma_proto* proto_descriptor();
static int proto_init();
static int proto_set(sigma_proto *instance, char* param, char* value);
static int proto_reload(sigma_proto *instance);
static int proto_decode(sigma_proto *instance, unsigned char* input, unsigned char* output, unsigned int len);
static int proto_encode(sigma_proto *instance, unsigned char* input, unsigned char* output, unsigned int len);

extern sigma_intf* intf_descriptor();
static int intf_init();
static int intf_set(sigma_intf *instance, char* param, void* value);
static int intf_reload(sigma_intf *instance);
static long intf_write(sigma_intf *instance, char* input, long len);
static long intf_read(sigma_intf *instance, char* output, long len);
