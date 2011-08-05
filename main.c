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

#include "types.h"
#include "modules.h"

int main(int argc, const char** argv)
{
	sigma_session session =
	{
		loadproto("nacl0"),
		loadinterface("tuntap"),
		loadinterface("udp")
	};
	
	session.proto->set(session.proto, "publickey", getenv("PUBLIC_KEY"));
	session.proto->set(session.proto, "privatekey", getenv("PRIVATE_KEY"));
	session.proto->init(session.proto);
	
	session.local->set(session.local, "nodename", getenv("INTERFACE"));
	session.local->init(session.local);
	
	int localport = atoi(getenv("LOCAL_PORT"));
	int remoteport = atoi(getenv("REMOTE_PORT"));
	int tunmode = atoi(getenv("TUN_MODE"));
	int protocolinfo = atoi(getenv("USE_PI"));
	session.remote->set(session.remote, "localaddr", getenv("LOCAL_ADDRESS"));
	session.remote->set(session.remote, "remoteaddr", getenv("REMOTE_ADDRESS"));
	session.remote->set(session.remote, "localport", &localport);
	session.remote->set(session.remote, "remoteport", &remoteport);
	session.remote->set(session.remote, "tunmode", &tunmode);
	session.remote->set(session.remote, "protocolinfo", &protocolinfo);
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
			char tuntapbuf[MAX_BUFFER_SIZE], tuntapbufenc[MAX_BUFFER_SIZE];
			long readvalue = session.local->read(session.local, tuntapbuf, MAX_BUFFER_SIZE);
	
			if (readvalue < 0)
			{
				fprintf(stderr, "TUN/TAP Read error %ld: %s\n", readvalue, strerror(errno));
				return -1;
			}
			
			readvalue = session.proto->encode(session.proto, tuntapbuf, tuntapbufenc, readvalue);
			
			long writevalue = session.remote->write(session.remote, tuntapbufenc, readvalue);
			
			if (writevalue < 0)
			{
				fprintf(stderr, "TUN/TAP Write error %ld: %s\n", writevalue, strerror(errno));
				return -1;
			}
		}
		
		if (FD_ISSET(session.remote->filedesc, &sockets) != 0)
		{
			char udpbuf[MAX_BUFFER_SIZE], udpbufenc[MAX_BUFFER_SIZE];
			long readvalue = session.remote->read(session.remote, udpbufenc, MAX_BUFFER_SIZE);
			
			if (readvalue < 0)
			{
				fprintf(stderr, "UDP Read error %ld: %s\n", readvalue, strerror(errno));
				return -1;
			}
			
			readvalue = session.proto->decode(session.proto, udpbufenc, udpbuf, readvalue);
			
			long writevalue = session.local->write(session.local, udpbuf, readvalue);
			
			if (writevalue < 0)
			{
				fprintf(stderr, "UDP Write error %ld: %s\n", writevalue, strerror(errno));
				return -1;
			}
		}
	}

	return 0;
}