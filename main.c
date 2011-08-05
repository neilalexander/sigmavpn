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

#include "types.h"
#include "modules.h"

int main(int argc, const char** argv)
{
	unsigned char publickey[64], privatekey[64];
	
	//unsigned char* publickeyhex = getenv("PUBLIC_KEY");
	//hex2bin(publickey, publickeyhex, 64);
	
	//unsigned char* privatekeyhex = getenv("PRIVATE_KEY");
	//hex2bin(privatekey, privatekeyhex, 64);

	sigma_proto* naclproto = loadproto("raw");
	naclproto->init(naclproto);
	
	sigma_intf* tuntap = loadinterface("tuntap");
	tuntap->set(tuntap, "nodename", "/dev/tap1");
	int tunmode = 2;
	tuntap->set(tuntap, "tunmode", &tunmode);
	tuntap->init(tuntap);
	
	char* raw = "HELLO!";
	unsigned char enc[1514];
	unsigned char cle[1514];
	
	//naclproto->set(naclproto, "publickey", publickey);
	//naclproto->set(naclproto, "privatekey", privatekey);
	
	//naclproto->encode(naclproto, raw, enc, 64);
	//naclproto->decode(naclproto, enc, cle, 64);
	//printf("decoded: %s\n", cle);
	
	return 0;
}