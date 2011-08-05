//
//  types.c
//  Sigma
//
//  Created by Neil Alexander on 05/08/2011.
//  Copyright 2011. All rights reserved.
//

#include <stdio.h>

void hex2bin(unsigned char* dest, unsigned char* src, int count)
{
	int i;
	
	for (i = 0; i < count; i++)
	{
		if (*src >= '0' && *src <= '9') *dest = *src - '0';
		else if (*src >= 'a' && * src <='f') *dest = *src - 'a' + 10;
		else if (*src >= 'A' && * src <='F') *dest = *src - 'A' + 10;
		
		src++; *dest = *dest << 4;
		
		if (*src >= '0' && *src <= '9') *dest += *src - '0';
		else if (*src >= 'a' && *src <= 'f') *dest += *src - 'a' + 10;
		else if (*src >= 'A' && *src <= 'F') *dest += *src - 'A' + 10;
		
		src++; dest++;
	}
}