//
//  intf_udp.c
//  Sigma UDP interface code
//
//  Created by Neil Alexander on 05/08/2011.
//  Copyright 2011. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "../types.h"

typedef struct sigma_intf_udp
{
	sigma_intf baseintf;
	
	#ifdef IPV6
		struct sockaddr_in6 remoteaddr;
		struct sockaddr_in6 localaddr;
	#else
		struct sockaddr_in remoteaddr;
		struct sockaddr_in localaddr;
	#endif
	
	long buffersize;
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
	
	#ifdef IPV6
		ret = sendto(udp->baseintf.filedesc, input, len, 0, (struct sockaddr*) &udp->remoteaddr, sizeof(struct sockaddr_in6));
	#else
		ret = sendto(udp->baseintf.filedesc, input, len, 0, (struct sockaddr*) &udp->remoteaddr, sizeof(struct sockaddr_in));
	#endif

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
	
	#ifdef IPV6
		udp->baseintf.filedesc = socket(PF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	#else
		udp->baseintf.filedesc = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	#endif
	
	if (udp->baseintf.filedesc < 0)
	{
		fprintf(stderr, "Unable to create UDP socket\n");
		return -1;
	}
	
	#ifdef IPV6
		if (bind(udp->baseintf.filedesc, (struct sockaddr*) &udp->localaddr, sizeof(struct sockaddr_in6)))
	#else
		if (bind(udp->baseintf.filedesc, (struct sockaddr*) &udp->localaddr, sizeof(struct sockaddr_in)))	
	#endif
	{
		fprintf(stderr, "Unable to bind UDP socket\n");
		return -1;
	}
	
	return 0;
}

static int intf_set(sigma_intf* instance, char* param, void* value)
{	
	sigma_intf_udp* udp = (sigma_intf_udp*) instance;
	
	if (strcmp(param, "localaddr") == 0)
	{
		#ifdef IPV6
		#	inet_pton(AF_INET6, *(char*) value, &(udp->localaddr.sin_addr));
		#elsif
		#	inet_pton(AF_INET, *(char*) value, &(udp->localaddr.sin_addr));
		#endif
		
		struct hostent *host;
		host = gethostbyname((char*) value);
		
		if (!host)
		{
			printf("Unable to look up address\n");
			return -1;
		}
		else
			if (!host->h_addr_list[0])
			{
				printf("No addresses available\n");
				return -1;
			}
		
		udp->localaddr.sin_family = host->h_addrtype;
		udp->localaddr.sin_addr.s_addr = *((unsigned long*)host->h_addr_list[0]);
	}
	
	if (strcmp(param, "localport") == 0)
	{
		udp->localaddr.sin_port = atoi(value);
	}
	
	if (strcmp(param, "remoteaddr") == 0)
	{
		#ifdef IPV6
		#	inet_pton(AF_INET6, *(char*) value, &(udp->remoteaddr.sin_addr));
		#elsif
		#	inet_pton(AF_INET, *(char*) value, &(udp->remoteaddr.sin_addr));
		#endif
		
		struct hostent *host;
		host = gethostbyname((char*) value);
		
		if (!host)
		{
			printf("Unable to look up address\n");
			return -1;
		}
			else
		if (!host->h_addr_list[0])
		{
			printf("No addresses available\n");
			return -1;
		}
		
		udp->remoteaddr.sin_family = host->h_addrtype;
		udp->remoteaddr.sin_addr.s_addr = *((unsigned long*)host->h_addr_list[0]);
	}
	
	if (strcmp(param, "remoteport") == 0)
	{
		udp->remoteaddr.sin_port = atoi(value);
	}
	
	return 0;
}

extern sigma_intf* intf_descriptor()
{
	sigma_intf_udp* intf_udp = malloc(sizeof(sigma_intf_udp));
	
	intf_udp->baseintf.init = intf_init;
	intf_udp->baseintf.read = intf_read;
	intf_udp->baseintf.write = intf_write;
	intf_udp->baseintf.set = intf_set;
	intf_udp->buffersize = (long) MAX_BUFFER_SIZE;

	#ifdef IPV6
		intf_udp->localaddr.sin_family = AF_INET6;
		intf_udp->remoteaddr.sin_family = AF_INET6;
	#elsif
		intf_udp->localaddr.sin_family = AF_INET;
		intf_udp->remoteaddr.sin_family = AF_INET;
	#endif
	
	return (sigma_intf*) intf_udp;
}