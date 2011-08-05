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
		loadinterface("dummy")
	};
	
	session.proto->init(session.proto);
	session.local->set(session.local, "nodename", "/dev/tun0");
	session.local->init(session.local);
	
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
			char tuntapbuf[1514];
			long readvalue = session.local->read(session.local, tuntapbuf, 1514);
	
			if (readvalue < 0)
			{
				fprintf(stderr, "Read error %ld: %s\n", readvalue, strerror(errno));
				return -1;
			}
			
			long writevalue = session.remote->write(session.remote, tuntapbuf, 1514);
			
			if (writevalue < 0)
			{
				fprintf(stderr, "Write error %ld: %s\n", writevalue, strerror(errno));
				return -1;
			}
		}
	}	

	return 0;
}