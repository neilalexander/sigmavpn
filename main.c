//
//  main.c
//  Sigma main code
//
//  Created by Neil Alexander on 05/08/2011.
//  Copyright 2011. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/select.h>
#include <errno.h>
#include <pthread.h>

#include "types.h"
#include "modules.h"
#include "dep/ini.h"

#define THREAD_MAX_COUNT 16
#define THREAD_STACK_SIZE 131072

sigma_sessionlist* sessions;
sigma_sessionlist* head;
sigma_sessionlist* pointer;

static int handler(void* user, const char* section, const char* name, const char* value)
{
	if (name == NULL && value == NULL)
	{
		sigma_sessionlist* newobject = malloc(sizeof(sigma_sessionlist));
		strlcpy(newobject->session.sessionname, section, 32);
		
		if (sessions == NULL)
		{
			sessions = newobject;
			head = sessions;
		}
			else
		{
			head->next = newobject;
			head = head->next;
		}
	}
		else
	{
		pointer = sessions;
		
		while (pointer)
		{			
			if (strcmp(pointer->session.sessionname, section) == 0)
				break;
			
			pointer = pointer->next;
		}
		
		if (pointer == NULL)
			return -1;
		
		if (pointer == NULL)
		{
			fprintf(stderr, "Session %s not found (this should never happen)\n", section);
			return -1;
		}
		
		if (strcmp(name, "proto") == 0)
		{
			pointer->session.proto = loadproto((char*) value);
		}
			else
		if (strncmp(name, "proto_", 6) == 0)
		{
			if (pointer->session.proto == NULL)
			{
				fprintf(stderr, "Parameter '%s' ignored; 'proto=' should appear before '%s=' in the config file\n", name, name);
				return -1;
			}
			
			pointer->session.proto->set(pointer->session.proto, name + 6, value);
		}
		
		if (strcmp(name, "peer") == 0)
		{
			pointer->session.remote = loadinterface((char*) value);
		}
			else
		if (strncmp(name, "peer_", 5) == 0)
		{
			if (pointer->session.remote == NULL)
			{
				fprintf(stderr, "Parameter '%s' ignored; 'peer=' should appear before '%s=' in the config file\n", name, name);
				return -1;
			}
				
			pointer->session.remote->set(pointer->session.remote, name + 5, value);
		}
		
		if (strcmp(name, "local") == 0)
		{
			pointer->session.local = loadinterface((char*) value);
		}
			else
		if (strncmp(name, "local_", 6) == 0)
		{
			if (pointer->session.local == NULL)
			{
				fprintf(stderr, "Parameter '%s' ignored; 'local=' should appear before '%s=' in the config file\n", name, name);
				return -1;
			}
				
			pointer->session.local->set(pointer->session.local, name + 6, value);
		}
	}
	
	return 0;
}

int main(int argc, const char** argv)
{
	sessions = NULL;
	pointer = NULL;
	
	if (ini_parse("sigma.conf", handler, (void*) NULL) < 0)
	{
        printf("Configuration file 'sigma.conf' is missing\n");
        return 1;
    }
	
	pthread_t threads[THREAD_MAX_COUNT];
	//pthread_attr_t attr;
	//pthread_attr_init(&attr);
	//pthread_attr_setstacksize(&attr, THREAD_STACK_SIZE);
	
	pointer = sessions;
	int threadnum = 0;
	
	while (pointer != NULL)
	{
		if (threadnum == THREAD_MAX_COUNT)
		{
			fprintf(stderr, "Maximum thread count reached; this binary only supports %i concurrent sessions\n", THREAD_MAX_COUNT);
			return -1;
		}
		
		if (pointer == NULL)
		{
			fprintf(stderr, "No session data found\n");
			break;
		}
		
		int rc = pthread_create(&threads[threadnum], 0, sessionwrapper, &pointer->session);
		
		if (rc)
		{
			fprintf(stderr, "Thread returned %d\n", rc);
			return -1;
		}

		threadnum ++;
		pointer = pointer->next;
	}
	
	pointer = sessions;
	threadnum = 0;
	
	while (pointer != NULL)
	{
		pthread_join(threads[threadnum], NULL);
		threadnum ++;
	}

	return 0;
}

void* sessionwrapper(void* param)
{
	sigma_session* sessionparam;
	sessionparam = (sigma_session*) param;
	int status = runsession(sessionparam);
	pthread_exit(&status);
}

int runsession(sigma_session* session)
{
	fd_set sockets;
	
	if (session->proto == NULL)
	{
		fprintf(stderr, "Protocol loading failed.\n");
		return -1;
	}
	
	if (session->local == NULL)
	{
		fprintf(stderr, "Local interface loading failed.\n");
		return -1;
	}
	
	if (session->remote == NULL)
	{
		fprintf(stderr, "Peer interface loading failed.\n");
		return -1;
	}
	
	session->proto->init(pointer->session.proto);
	session->local->init(pointer->session.local);
	session->remote->init(pointer->session.remote);
	
	while (1)
	{
		FD_ZERO(&sockets);
		FD_SET(session->local->filedesc, &sockets);
		FD_SET(session->remote->filedesc, &sockets);
		
		int len = select(sizeof(sockets) * 2, &sockets, NULL, NULL, 0);
		
		if (len < 0)
		{
			fprintf(stderr, "Poll error");
			return -1;
		}
		
		if (FD_ISSET(session->local->filedesc, &sockets) != 0)
		{
			char tuntapbuf[MAX_BUFFER_SIZE], tuntapbufenc[MAX_BUFFER_SIZE];
			long readvalue = session->local->read(session->local, tuntapbuf, MAX_BUFFER_SIZE);
			
			if (readvalue < 0)
			{
				fprintf(stderr, "TUN/TAP Read error %ld: %s\n", readvalue, strerror(errno));
				return -1;
			}
			
			readvalue = session->proto->encode(session->proto, tuntapbuf, tuntapbufenc, readvalue);
			
			long writevalue = session->remote->write(session->remote, tuntapbufenc, readvalue);
			
			if (writevalue < 0)
			{
				fprintf(stderr, "TUN/TAP Write error %ld: %s\n", writevalue, strerror(errno));
				return -1;
			}
		}
		
		if (FD_ISSET(session->remote->filedesc, &sockets) != 0)
		{
			char udpbuf[MAX_BUFFER_SIZE], udpbufenc[MAX_BUFFER_SIZE];
			long readvalue = session->remote->read(session->remote, udpbufenc, MAX_BUFFER_SIZE);
			
			if (readvalue < 0)
			{
				fprintf(stderr, "UDP Read error %ld: %s\n", readvalue, strerror(errno));
				return -1;
			}
			
			readvalue = session->proto->decode(session->proto, udpbufenc, udpbuf, readvalue);
			
			long writevalue = session->local->write(session->local, udpbuf, readvalue);
			
			if (writevalue < 0)
			{
				fprintf(stderr, "UDP Write error %ld: %s\n", writevalue, strerror(errno));
				return -1;
			}
		}
	}
	
	return 0;
}