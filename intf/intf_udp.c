//
//  intf_udp.c
//  Sigma UDP interface code
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#include "../types.h"

typedef union
{
	struct sockaddr_in ipv4;
	struct sockaddr_in6 ipv6;
}
sigma_address;

int changes;

typedef struct sigma_intf_udp
{
	sigma_intf baseintf;
	
	sigma_address remoteaddr;
	sigma_address localaddr;
	
	long buffersize;
	unsigned int ipv6;
}
sigma_intf_udp;

static long intf_write(sigma_intf *instance, char* input, long len)
{
	sigma_intf_udp* udp = (sigma_intf_udp*) instance;

	if (udp->baseintf.filedesc < 0)
	{
		fprintf(stderr, "UDP socket is not accessible\n");
		return -1;
	}
	
	int ret;
	
	if (udp->ipv6)
		ret = sendto(udp->baseintf.filedesc, input, len, 0, (struct sockaddr*) &udp->remoteaddr, sizeof(struct sockaddr_in6));
	else
		ret = sendto(udp->baseintf.filedesc, input, len, 0, (struct sockaddr*) &udp->remoteaddr, sizeof(struct sockaddr_in));

	return ret;
}

static long intf_read(sigma_intf *instance, char* output, long len)
{
	sigma_intf_udp* udp = (sigma_intf_udp*) instance;
	
	if (!udp->baseintf.filedesc < 0)
	{
		fprintf(stderr, "UDP socket is not accessible\n");
		return -1;
	}
	
	len = recv(udp->baseintf.filedesc, output, udp->buffersize, 0);
	
	return len;
}

static int intf_init(sigma_intf* instance)
{
	sigma_intf_udp* udp = (sigma_intf_udp*) instance;
	char errorstring[64];
	
	changes = 0;
	
	if (udp->ipv6)
		udp->baseintf.filedesc = socket(AF_INET6, SOCK_DGRAM, 0);
	else
		udp->baseintf.filedesc = socket(AF_INET, SOCK_DGRAM, 0);
	
	int optval = 1;
	setsockopt(udp->baseintf.filedesc, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	
	if (udp->baseintf.filedesc < 0)
	{
		strerror_r(errno, errorstring, 64);
		fprintf(stderr, "Unable to create UDP socket: %s\n", errorstring);
		
		close(udp->baseintf.filedesc);
		udp->baseintf.filedesc = -1;
		
		return -1;
	}
	
	int bindresult;

	if (udp->ipv6)
		bindresult = bind(udp->baseintf.filedesc, (struct sockaddr*) &udp->localaddr, sizeof(struct sockaddr_in6));
	else
		bindresult = bind(udp->baseintf.filedesc, (struct sockaddr*) &udp->localaddr, sizeof(struct sockaddr_in));	

	if (bindresult < 0)
	{
		strerror_r(errno, errorstring, 64);
		fprintf(stderr, "Unable to bind UDP socket: %s\n", errorstring);
		
		close(udp->baseintf.filedesc);
		udp->baseintf.filedesc = -1;
		
		return -1;
	}
	
	return 0;
}

static int intf_set(sigma_intf* instance, char* param, void* value)
{	
	sigma_intf_udp* udp = (sigma_intf_udp*) instance;
	
	if (strcmp(param, "localaddr") == 0)
	{
		struct addrinfo hints, *results;
		int status;
		
		memset(&hints, 0, sizeof(hints));
		
		if (udp->ipv6)
			hints.ai_family = AF_INET6;
		else
			hints.ai_family = AF_INET;

		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;
		
		if (getaddrinfo((char*) value, NULL, &hints, &results) != 0)
		{
			printf("Unable to look up local address\n");
			return -1;
		}
		
		if (udp->ipv6)
		{
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *) results->ai_addr;
			
			if (memcmp(&udp->localaddr.ipv6.sin6_addr, &ipv6->sin6_addr, sizeof(struct sockaddr_in6)) != 0)
			{
				udp->localaddr.ipv6.sin6_family = AF_INET6;
				udp->localaddr.ipv6.sin6_addr = ipv6->sin6_addr;
				changes ++;
			}
		}
			else
		{
			struct sockaddr_in *ipv4 = (struct sockaddr_in*) results->ai_addr;
		
			if (memcmp(&udp->localaddr.ipv4.sin_addr, &ipv4->sin_addr, sizeof(struct sockaddr_in)) != 0)
			{
				udp->localaddr.ipv4.sin_family = AF_INET;
				udp->localaddr.ipv4.sin_addr.s_addr = ipv4->sin_addr.s_addr;
				changes ++;
			}
		}
		
		freeaddrinfo(results);
	}
		else
	if (strcmp(param, "localport") == 0)
	{
		unsigned int port = (unsigned int) htons(atoi(value));
		
		if (udp->ipv6)
		{
			if (udp->localaddr.ipv6.sin6_port != port)
			{
				udp->localaddr.ipv6.sin6_port = port;
				changes ++;
			}
		}
			else
		{
			if (udp->localaddr.ipv4.sin_port != port)
			{
				udp->localaddr.ipv4.sin_port = port;
				changes ++;
			}
		}
	}
		else
	if (strcmp(param, "remoteaddr") == 0)
	{
		struct addrinfo hints, *results;
		int status;
		
		memset(&hints, 0, sizeof(hints));
		
		if (udp->ipv6)
			hints.ai_family = AF_INET6;
		else
			hints.ai_family = AF_INET;
	
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;
		
		if (getaddrinfo((char*) value, NULL, &hints, &results) != 0)
		{
			printf("Unable to look up remote address\n");
			return -1;
		}
		
		if (udp->ipv6)
		{
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *) results->ai_addr;
		
			if (memcmp(&udp->localaddr.ipv6.sin6_addr, &ipv6->sin6_addr, sizeof(struct sockaddr_in6)) != 0)
			{
				
                                udp->remoteaddr.ipv6.sin6_family = AF_INET6;
				udp->remoteaddr.ipv6.sin6_addr = ipv6->sin6_addr;
				changes ++;
			}
		}
			else
		{
			struct sockaddr_in *ipv4 = (struct sockaddr_in *) results->ai_addr;

			if (memcmp(&udp->localaddr.ipv4.sin_addr, &ipv4->sin_addr, sizeof(struct sockaddr_in)) != 0)
			{
				udp->remoteaddr.ipv4.sin_family = AF_INET;
				udp->remoteaddr.ipv4.sin_addr.s_addr = ipv4->sin_addr.s_addr;
				changes ++;
			}
		}
		
		freeaddrinfo(results);
	}
		else
	if (strcmp(param, "remoteport") == 0)
	{
		unsigned int port = (unsigned int) htons(atoi(value));
		
		if (udp->ipv6)
		{
			if (udp->remoteaddr.ipv6.sin6_port != port)
			{
				udp->remoteaddr.ipv6.sin6_port = port;
				changes ++;
			}
		}
			else
		{
			if (udp->remoteaddr.ipv4.sin_port != port)
			{
				udp->remoteaddr.ipv4.sin_port = port;
				changes ++;
			}
		}
	}
		else
	if (strcmp(param, "ipv6") == 0)
	{
		udp->ipv6 = atoi(value);
	}
	
	return 0;
}

static int intf_reload(sigma_intf* instance)
{
	if (changes == 0)
		return 0;
	
	sigma_intf_udp* udp = (sigma_intf_udp*) instance;
	
	printf("%i changes\n", changes);

	printf("Closing down socket...\n");
	
	if (close(udp->baseintf.filedesc) == -1)
	{
		printf("Socket close failed\n");
		return -1;
	}

	printf("Restarting protocol...\n");
	intf_init(instance);
	
	return 0;
}

extern sigma_intf* intf_descriptor()
{
	sigma_intf_udp* intf_udp = malloc(sizeof(sigma_intf_udp));
	
	intf_udp->baseintf.init = intf_init;
	intf_udp->baseintf.read = intf_read;
	intf_udp->baseintf.write = intf_write;
	intf_udp->baseintf.set = intf_set;
	intf_udp->baseintf.reload = intf_reload;
	intf_udp->buffersize = (long) MAX_BUFFER_SIZE;
	
	return (sigma_intf*) intf_udp;
}
