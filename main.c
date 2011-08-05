//
//  main.c
//  Sigma
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

#include "types.h"
#include "modules.h"

int main(int argc, const char** argv)
{
	sigma_session session =
	{
		loadproto("raw"),
		loadinterface("tuntap"),
		loadinterface("udp")
	};
	
	session.proto->init(session.proto);
	
	session.local->set(session.local, "nodename", getenv("INTERFACE"));
	session.local->init(session.local);
	
	//printf("Remote address: %s (%s)\n", getenv("REMOTE_ADDRESS"), getenv("REMOTE_PORT"));
	
	int localport = atoi(getenv("LOCAL_PORT"));
	int remoteport = atoi(getenv("REMOTE_PORT"));
	session.remote->set(session.remote, "localaddr", getenv("LOCAL_ADDRESS"));
	session.remote->set(session.remote, "remoteaddr", getenv("REMOTE_ADDRESS"));
	session.remote->set(session.remote, "localport", &localport);
	session.remote->set(session.remote, "remoteport", &remoteport);
	session.remote->init(session.remote);
	
	fd_set sockets;

	while (1)
	{
		FD_ZERO(&sockets);
		FD_SET(session.local->filedesc, &sockets);
		FD_SET(session.remote->filedesc, &sockets);
		
		int len = select(sizeof(sockets) * 2, &sockets, NULL, NULL, 0);
		
		if (len < 0)
		{
			fprintf(stderr, "Poll error");
			return -1;
		}
		
		if (FD_ISSET(session.local->filedesc, &sockets) != 0)
		{
			char tuntapbuf[1000];
			long readvalue = session.local->read(session.local, tuntapbuf, 1000);
	
			if (readvalue < 0)
			{
				fprintf(stderr, "Read error %ld: %s\n", readvalue, strerror(errno));
				return -1;
			}
			
			long writevalue = session.remote->write(session.remote, tuntapbuf, 1000);
			
			if (writevalue < 0)
			{
				fprintf(stderr, "Write error %ld: %s\n", writevalue, strerror(errno));
				return -1;
			}
		}
		
		if (FD_ISSET(session.remote->filedesc, &sockets) != 0)
		{
			char udpbuf[1000];
			long readvalue = session.remote->read(session.remote, udpbuf, 1000);
			
			if (readvalue < 0)
			{
				fprintf(stderr, "Read error %ld: %s\n", readvalue, strerror(errno));
				return -1;
			}
			
			long writevalue = session.local->write(session.local, udpbuf, 1000);
			
			if (writevalue < 0)
			{
				fprintf(stderr, "Write error %ld: %s\n", writevalue, strerror(errno));
				return -1;
			}
		}
	}	

	return 0;
}