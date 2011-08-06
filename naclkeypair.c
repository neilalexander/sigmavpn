//
//  naclkeypair.c
//  Sigma NaCl keypair generator
//
//  Created by Neil Alexander on 06/08/2011.
//  Copyright 2011. All rights reserved.
//

#include <stdio.h>
#include "include/crypto_box_curve25519xsalsa20poly1305.h"

int main()
{
	unsigned char pk[crypto_box_curve25519xsalsa20poly1305_PUBLICKEYBYTES];
	unsigned char sk[crypto_box_curve25519xsalsa20poly1305_SECRETKEYBYTES];
	
	crypto_box_curve25519xsalsa20poly1305_keypair(pk, sk);
	
	int i;
	
	printf("PRIVATE KEY: ");
	
	for (i = 0; i < crypto_box_curve25519xsalsa20poly1305_SECRETKEYBYTES; i ++)
		printf("%02x", sk[i]);
	
	printf("\n PUBLIC KEY: ");
	
	for (i = 0; i < crypto_box_curve25519xsalsa20poly1305_PUBLICKEYBYTES; i ++)
		printf("%02x", pk[i]);
	
	printf("\n");
}