//
//  proto_nacl0.c
//  Sigma nacl0 protocol code
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
//  The nacl0 code is derived, with permission, from Quicktun written
//  by Ivo Smits. http://oss.ucis.nl/hg/quicktun/
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sodium.h>

#include "../types.h"
#include "../proto.h"

unsigned char n[crypto_box_NONCEBYTES];

typedef struct sigma_proto_nacl
{
    sigma_proto baseproto;

    unsigned char privatekey[crypto_box_SECRETKEYBYTES];
    unsigned char publickey[crypto_box_PUBLICKEYBYTES];
    unsigned char precomp[crypto_box_BEFORENMBYTES];
}
sigma_proto_nacl;

static int proto_set(sigma_proto* instance, char* param, char* value)
{
    if (strcmp(param, "publickey") == 0)
    {
        if (strlen(value) != crypto_box_PUBLICKEYBYTES * 2)
        {
            fprintf(stderr, "Public key is incorrect length\n");
            return -1;
        }

        hex2bin(((sigma_proto_nacl*) instance)->publickey, value, crypto_box_PUBLICKEYBYTES);
    }
        else
    if (strcmp(param, "privatekey") == 0)
    {
        if (strlen(value) != crypto_box_SECRETKEYBYTES * 2)
        {
            fprintf(stderr, "Private key is incorrect length\n");
            return -1;
        }

        hex2bin(((sigma_proto_nacl*) instance)->privatekey, value, crypto_box_SECRETKEYBYTES);
    }
        else
    {
        fprintf(stderr, "Unknown attribute '%s'\n", param);
        return -1;
    }

    return 0;
}

static int proto_encode(sigma_proto *instance, unsigned char* input, unsigned char* output, unsigned int len)
{
    if ((len + crypto_box_ZEROBYTES - crypto_box_BOXZEROBYTES) > MAX_BUFFER_SIZE)
    {
        fprintf(stderr, "Encryption failed (packet length %i is above MAX_BUFFER_SIZE %i)\n", (len + crypto_box_ZEROBYTES - crypto_box_BOXZEROBYTES), MAX_BUFFER_SIZE);
        return -1;
    }

    unsigned char *tempbuffer, *tempbufferinput;

    if ((tempbuffer = calloc(len + crypto_box_ZEROBYTES, 1)) == NULL) {
        perror("calloc");
        return -1;
    }
    if ((tempbufferinput = calloc(len + crypto_box_ZEROBYTES, 1)) == NULL) {
        perror("calloc");
        free(tempbuffer);
        return -1;
    }

    memcpy(tempbufferinput + crypto_box_ZEROBYTES, input, len);

    int result = crypto_box_afternm(
        tempbuffer,
        tempbufferinput,
        len + crypto_box_ZEROBYTES,
        n,
        ((sigma_proto_nacl*) instance)->precomp
    );

    if (result)
    {
        fprintf(stderr, "Encryption failed (length %i, given result %i)\n", len, result);
        free(tempbuffer);
        free(tempbufferinput);
        return -1;
    }

    memcpy(output, tempbuffer + crypto_box_BOXZEROBYTES, len + crypto_box_ZEROBYTES - crypto_box_BOXZEROBYTES);
    free(tempbuffer);
    free(tempbufferinput);

    return len + crypto_box_ZEROBYTES - crypto_box_BOXZEROBYTES;
}

static int proto_decode(sigma_proto *instance, unsigned char* input, unsigned char* output, unsigned int len)
{
    if ((len - crypto_box_BOXZEROBYTES) > MAX_BUFFER_SIZE)
    {
        fprintf(stderr, "Decryption failed (packet length %i is above MAX_BUFFER_SIZE %i)\n", (len - crypto_box_BOXZEROBYTES), MAX_BUFFER_SIZE);
        return 0;
    }

    if (len < crypto_box_BOXZEROBYTES)
    {
        fprintf(stderr, "Short packet received: %d\n", len);
        return 0;
    }

    unsigned char *tempbuffer, *tempbufferout;

    if ((tempbuffer = calloc(len + crypto_box_BOXZEROBYTES, 1)) == NULL) {
        perror("calloc");
        return 0;
    }
    if ((tempbufferout = calloc(len + crypto_box_BOXZEROBYTES, 1)) == NULL) {
        perror("calloc");
        free(tempbuffer);
        return 0;
    }

    memcpy(tempbuffer + crypto_box_BOXZEROBYTES, input, len);

    int result = crypto_box_open_afternm(
        tempbufferout,
        tempbuffer,
        len + crypto_box_BOXZEROBYTES,
        n,
        ((sigma_proto_nacl*) instance)->precomp
    );

    if (result)
    {
        fprintf(stderr, "Decryption failed (length %i, given result %i)\n", len, result);
        free(tempbuffer);
        free(tempbufferout);
        return 0;
    }

    memcpy(output, tempbufferout + crypto_box_ZEROBYTES, len - crypto_box_ZEROBYTES + crypto_box_BOXZEROBYTES);
    free(tempbuffer);
    free(tempbufferout);

    return len - crypto_box_ZEROBYTES + crypto_box_BOXZEROBYTES;
}

static int proto_init(sigma_proto *instance)
{
    crypto_box_beforenm(
        ((sigma_proto_nacl*) instance)->precomp,
        ((sigma_proto_nacl*) instance)->publickey,
        ((sigma_proto_nacl*) instance)->privatekey
    );

    memset(n, 0, crypto_box_NONCEBYTES);

    return 0;
}

static int proto_reload(sigma_proto *instance)
{
    return proto_init(instance);
}

extern sigma_proto* proto_descriptor()
{
    sigma_proto_nacl* proto_nacl0 = calloc(1, sizeof(sigma_proto_nacl));

    proto_nacl0->baseproto.encrypted = 1;
    proto_nacl0->baseproto.stateful = 0;
    proto_nacl0->baseproto.init = proto_init;
    proto_nacl0->baseproto.encode = proto_encode;
    proto_nacl0->baseproto.decode = proto_decode;
    proto_nacl0->baseproto.set = proto_set;
    proto_nacl0->baseproto.reload = proto_reload;
    proto_nacl0->baseproto.state = 0;

    return (sigma_proto*) proto_nacl0;
}
