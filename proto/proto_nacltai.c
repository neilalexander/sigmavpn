//
//  proto_nacltai.c
//  Sigma nacltai protocol code
//
//  Created by Neil Alexander on 05/08/2011.
//  Copyright 2011. All rights reserved.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "../types.h"
#include "../include/crypto_box_curve25519xsalsa20poly1305.h"
#include "../include/crypto_scalarmult_curve25519.h"

#define uint64 unsigned long long
#define noncelength 16
#define nonceoffset (crypto_box_curve25519xsalsa20poly1305_NONCEBYTES - noncelength)
static const int overhead = crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES + noncelength;

struct tai
{
	uint64 x;
};

struct taia
{
	struct tai sec;
	unsigned long nano;
	unsigned long atto;
};

typedef struct sigma_proto_nacl
{
	sigma_proto baseproto;
	
	char encbuffer[MAX_BUFFER_SIZE + crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES];
	char decbuffer[MAX_BUFFER_SIZE + crypto_box_curve25519xsalsa20poly1305_ZEROBYTES];
	unsigned char privatekey[crypto_box_curve25519xsalsa20poly1305_SECRETKEYBYTES];
	unsigned char publickey[crypto_box_curve25519xsalsa20poly1305_PUBLICKEYBYTES];
	unsigned char precomp[crypto_box_curve25519xsalsa20poly1305_BEFORENMBYTES];
	unsigned char cenonce[crypto_box_curve25519xsalsa20poly1305_NONCEBYTES];
	unsigned char cdnonce[crypto_box_curve25519xsalsa20poly1305_NONCEBYTES];
	unsigned char taipublickey[crypto_box_curve25519xsalsa20poly1305_PUBLICKEYBYTES];
	struct taia cdtaip, cdtaie;
}
sigma_proto_nacl;

void tai_pack(char *s, struct tai *t)
{
	uint64 x;
	x = t->x;
	s[7] = x & 255; x >>= 8;
	s[6] = x & 255; x >>= 8;
	s[5] = x & 255; x >>= 8;
	s[4] = x & 255; x >>= 8;
	s[3] = x & 255; x >>= 8;
	s[2] = x & 255; x >>= 8;
	s[1] = x & 255; x >>= 8;
	s[0] = x;
}

void tai_unpack(char *s, struct tai *t)
{
	uint64 x;
	x = (unsigned char) s[0];
	x <<= 8; x += (unsigned char) s[1];
	x <<= 8; x += (unsigned char) s[2];
	x <<= 8; x += (unsigned char) s[3];
	x <<= 8; x += (unsigned char) s[4];
	x <<= 8; x += (unsigned char) s[5];
	x <<= 8; x += (unsigned char) s[6];
	x <<= 8; x += (unsigned char) s[7];
	t->x = x;
}

void taia_pack(char *s, struct taia *t)
{
	unsigned long x;
	tai_pack(s, &t->sec);
	s += 8;
	x = t->atto;
	s[7] = x & 255; x >>= 8;
	s[6] = x & 255; x >>= 8;
	s[5] = x & 255; x >>= 8;
	s[4] = x;
	x = t->nano;
	s[3] = x & 255; x >>= 8;
	s[2] = x & 255; x >>= 8;
	s[1] = x & 255; x >>= 8;
	s[0] = x;
} 

void taia_unpack(char *s, struct taia *t)
{
	unsigned long x;
	tai_unpack(s, &t->sec);
	s += 8;
	x = (unsigned char) s[4];
	x <<= 8; x += (unsigned char) s[5];
	x <<= 8; x += (unsigned char) s[6];
	x <<= 8; x += (unsigned char) s[7];
	t->atto = x;
	x = (unsigned char) s[0];
	x <<= 8; x += (unsigned char) s[1];
	x <<= 8; x += (unsigned char) s[2];
	x <<= 8; x += (unsigned char) s[3];
	t->nano = x;
}

void taia_now(struct taia *t)
{
	struct timeval now;
	gettimeofday(&now, (struct timezone *) 0);
	t->sec.x = 4611686018427387914ULL + (uint64) now.tv_sec;
	t->nano = 1000 * now.tv_usec + 500;
	t->atto++;
}

static int proto_set(sigma_proto* instance, char* param, char* value)
{	
	if (strcmp(param, "publickey") == 0)
	{
		if (strlen(value) != crypto_box_curve25519xsalsa20poly1305_PUBLICKEYBYTES * 2)
		{
			fprintf(stderr, "Public key is incorrect length\n");
			return -1;
		}
		
		hex2bin(((sigma_proto_nacl*) instance)->publickey, value, crypto_box_curve25519xsalsa20poly1305_PUBLICKEYBYTES);
	}
	
	if (strcmp(param, "privatekey") == 0)
	{
		if (strlen(value) != crypto_box_curve25519xsalsa20poly1305_SECRETKEYBYTES * 2)
		{
			fprintf(stderr, "Private key is incorrect length\n");
			return -1;
		}
		
		hex2bin(((sigma_proto_nacl*) instance)->privatekey, value, crypto_box_curve25519xsalsa20poly1305_SECRETKEYBYTES);
	}
	
	return 0;
}

static int proto_encode(sigma_proto *instance, unsigned char* input, unsigned char* output, unsigned int len)
{
	sigma_proto_nacl* inst = ((sigma_proto_nacl*) instance);
	
	//unsigned char n[crypto_box_curve25519xsalsa20poly1305_NONCEBYTES];
	unsigned char tempbuffer[len], tempbufferinput[len + crypto_box_curve25519xsalsa20poly1305_ZEROBYTES];
	
	//memset(n, 0, crypto_box_curve25519xsalsa20poly1305_NONCEBYTES);
	memset(tempbufferinput, 0, crypto_box_curve25519xsalsa20poly1305_ZEROBYTES);
	memcpy(tempbufferinput + crypto_box_curve25519xsalsa20poly1305_ZEROBYTES, input, len);
	
	taia_now(&inst->cdtaie);
	taia_pack(inst->cenonce + nonceoffset, &(inst->cdtaie));
	
	int result = crypto_box_curve25519xsalsa20poly1305_afternm(
		tempbuffer,
		tempbufferinput,
		len + crypto_box_curve25519xsalsa20poly1305_ZEROBYTES,
		inst->cenonce,
		inst->precomp
	);
	 
	if (result)
	{
		fprintf(stderr, "Encryption failed (length %i, given result %i)\n", len, result);
	}
	
	if ((len + crypto_box_curve25519xsalsa20poly1305_ZEROBYTES - crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES) > MAX_BUFFER_SIZE)
	{
		fprintf(stderr, "Encryption failed (packet length %i is above MAX_BUFFER_SIZE %i)\n", (len + crypto_box_curve25519xsalsa20poly1305_ZEROBYTES - crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES), MAX_BUFFER_SIZE);
		return -1;
	}
	
	memcpy((void*)(tempbuffer + crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES - noncelength), inst->cenonce + nonceoffset, noncelength);
	
	memcpy(output, tempbuffer + crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES, len + crypto_box_curve25519xsalsa20poly1305_ZEROBYTES - crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES);
	
	len += overhead;
	
	return len + crypto_box_curve25519xsalsa20poly1305_ZEROBYTES - crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES;
}

static int proto_decode(sigma_proto *instance, unsigned char* input, unsigned char* output, unsigned int len)
{
	sigma_proto_nacl* inst = (sigma_proto_nacl*) instance;
	
	if (len < overhead)
	{
		fprintf(stderr, "Short packet received: %d\n", len);
		return 0;
	}
	
	struct taia cdtaic;
	
	unsigned char tempbuffer[len + crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES], tempbufferout[len];
	
	len -= overhead;
	
	taia_unpack((char*)(input + crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES - noncelength), &cdtaic);

	memset(tempbuffer, 0, crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES);
	memcpy(tempbuffer + crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES, input, len + crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES);
	memcpy(inst->cdnonce + nonceoffset, tempbuffer + crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES - noncelength, noncelength);
	
	int result = crypto_box_curve25519xsalsa20poly1305_open_afternm(
		tempbufferout,
		tempbuffer,
		len + crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES,
		inst->cdnonce,
		inst->precomp
	);
	
	if (result)
	{
		fprintf(stderr, "Decryption failed (length %i, given result %i)\n", len, result);
		return -1;
	}
	
	if ((len - crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES) > MAX_BUFFER_SIZE)
	{
		fprintf(stderr, "Decryption failed (packet length %i is above MAX_BUFFER_SIZE %i)\n", (len - crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES), MAX_BUFFER_SIZE);
		return -1;
	}
	
	memcpy(output, tempbufferout + crypto_box_curve25519xsalsa20poly1305_ZEROBYTES, len + crypto_box_curve25519xsalsa20poly1305_ZEROBYTES);
	
	len += overhead;
	
	return len - crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES;
}

static int proto_init(sigma_proto *instance)
{
	sigma_proto_nacl* inst = ((sigma_proto_nacl*) instance);
	
	crypto_box_curve25519xsalsa20poly1305_beforenm(
		inst->precomp,
		inst->publickey,
		inst->privatekey
	);
	
	memset(inst->cenonce, 0, crypto_box_curve25519xsalsa20poly1305_NONCEBYTES);
	memset(inst->cdnonce, 0, crypto_box_curve25519xsalsa20poly1305_NONCEBYTES);
	
	crypto_scalarmult_curve25519_base(inst->taipublickey, inst->privatekey);
	
	inst->cenonce[nonceoffset - 1] = memcmp(inst->taipublickey, inst->publickey, crypto_box_curve25519xsalsa20poly1305_PUBLICKEYBYTES) > 0 ? 1 : 0;
	inst->cdnonce[nonceoffset - 1] = inst->cenonce[nonceoffset - 1] ? 0 : 1;
	
	return 0;
}

extern sigma_proto* proto_descriptor()
{
	sigma_proto_nacl* proto_nacltai = malloc(sizeof(sigma_proto_nacl));
	
	proto_nacltai->baseproto.encrypted = 1;
	proto_nacltai->baseproto.stateful = 0;
	proto_nacltai->baseproto.init = proto_init;
	proto_nacltai->baseproto.encode = proto_encode;
	proto_nacltai->baseproto.decode = proto_decode;
	proto_nacltai->baseproto.set = proto_set;
	proto_nacltai->baseproto.state = 0;
	
	return (sigma_proto*) proto_nacltai;
}
