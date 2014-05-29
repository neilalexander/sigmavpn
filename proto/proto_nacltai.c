//
//  proto_nacltai.c
//  Sigma nacltai protocol code
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
//  The nacltai code is derived, with permission, from Quicktun written
//  by Ivo Smits. http://oss.ucis.nl/hg/quicktun/
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sodium.h>

#include "../types.h"
#include "../proto.h"

#define uint64 unsigned long long
#define noncelength 16
#define nonceoffset (crypto_box_NONCEBYTES - noncelength)

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

struct taia *tailog;
struct taia lasttai;

typedef struct sigma_proto_nacl
{
    sigma_proto baseproto;

    unsigned char privatekey[crypto_box_SECRETKEYBYTES];
    unsigned char publickey[crypto_box_PUBLICKEYBYTES];
    unsigned char precomp[crypto_box_BEFORENMBYTES];
    unsigned char encnonce[crypto_box_NONCEBYTES];
    unsigned char decnonce[crypto_box_NONCEBYTES];

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
        if (strlen(value) != crypto_box_PUBLICKEYBYTES * 2)
        {
            fprintf(stderr, "Public key is incorrect length\n");
            return -1;
        }

        hex2bin((unsigned char *) ((sigma_proto_nacl*) instance)->publickey, value, crypto_box_PUBLICKEYBYTES);
    }
        else
    if (strcmp(param, "privatekey") == 0)
    {
        if (strlen(value) != crypto_box_SECRETKEYBYTES * 2)
        {
            fprintf(stderr, "Private key is incorrect length\n");
            return -1;
        }

        hex2bin((unsigned char *) ((sigma_proto_nacl*) instance)->privatekey, value, crypto_box_SECRETKEYBYTES);
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
    sigma_proto_nacl* inst = (sigma_proto_nacl*) instance;

    unsigned char tempbufferinput[len + crypto_box_ZEROBYTES];

    memset(tempbufferinput, 0, crypto_box_ZEROBYTES);
    memcpy(tempbufferinput + crypto_box_ZEROBYTES, input, len);

    len += crypto_box_ZEROBYTES;

    taia_now(&inst->cdtaie);

    if (memcmp(&inst->cdtaie, &lasttai, sizeof(struct taia)) == 0)
        inst->cdtaie.atto ++;

    memcpy(&lasttai, &inst->cdtaie, sizeof(struct taia));

    taia_pack((char *) inst->encnonce + nonceoffset, &(inst->cdtaie));

    int result = crypto_box_afternm(
        output,
        tempbufferinput,
        len,
        inst->encnonce,
        ((sigma_proto_nacl*) instance)->precomp
    );

    memcpy(output, inst->encnonce + nonceoffset, noncelength);

    if (result)
    {
        fprintf(stderr, "Encryption failed (length %i, given result %i)\n", len, result);
        return -1;
    }

    return len;
}

static int proto_decode(sigma_proto *instance, unsigned char* input, unsigned char* output, unsigned int len)
{
    if (len < crypto_box_ZEROBYTES)
    {
        fprintf(stderr, "Short packet received: %d\n", len);
        return 0;
    }

    sigma_proto_nacl* inst = (sigma_proto_nacl*) instance;

    struct taia cdtaic;
    unsigned char tempbufferout[len];

    taia_unpack((char*) input, &cdtaic);

    memcpy(inst->decnonce + nonceoffset, input, noncelength);
    memset(input, 0, crypto_box_BOXZEROBYTES);

    int result = crypto_box_open_afternm(
        tempbufferout,
        input,
        len,
        inst->decnonce,
        inst->precomp
    );

    struct taia *taicheck = &tailog[0];
    struct taia *taioldest = tailog;

    int i;
    for (i = 0; i < 5; i ++)
    {
        if (memcmp(input + crypto_box_BOXZEROBYTES, taicheck, crypto_box_NONCEBYTES) == 0)
        {
            fprintf(stderr, "Timestamp reuse detected, possible replay attack (packet length %i)\n", len);
            return 0;
        }

        if (memcmp(taicheck, taioldest, crypto_box_NONCEBYTES) < 0)
            taioldest = taicheck;

        taicheck ++;
    }

    if (memcmp(input + crypto_box_BOXZEROBYTES, taioldest, crypto_box_NONCEBYTES) < 0)
    {
        fprintf(stderr, "Timestamp older than our oldest known timestamp, possible replay attack (packet length %i)\n", len);
        return 0;
    }

    inst->cdtaip = cdtaic;

    if (result)
    {
        fprintf(stderr, "Decryption failed (length %i, given result %i)\n", len, result);
        return 0;
    }

    len -= crypto_box_ZEROBYTES;
    memcpy(output, tempbufferout + crypto_box_ZEROBYTES, len);

    return len;
}

static int proto_init(sigma_proto *instance)
{
    sigma_proto_nacl* inst = ((sigma_proto_nacl*) instance);
    unsigned char taipublickey[crypto_box_PUBLICKEYBYTES];

    crypto_box_beforenm(
        inst->precomp,
        inst->publickey,
        inst->privatekey
    );

    memset(inst->encnonce, 0, crypto_box_NONCEBYTES);
    memset(inst->decnonce, 0, crypto_box_NONCEBYTES);
    tailog = calloc(10, crypto_box_NONCEBYTES);

    crypto_scalarmult_curve25519_base(taipublickey, inst->privatekey);

    inst->encnonce[nonceoffset - 1] = memcmp(taipublickey, inst->publickey, crypto_box_PUBLICKEYBYTES) > 0 ? 1 : 0;
    inst->decnonce[nonceoffset - 1] = inst->encnonce[nonceoffset - 1] ? 0 : 1;

    return 0;
}

static int proto_reload(sigma_proto *instance)
{
    return proto_init(instance);
}

extern sigma_proto* proto_descriptor()
{
    sigma_proto_nacl* proto_nacltai = calloc(1, sizeof(sigma_proto_nacl));

    proto_nacltai->baseproto.encrypted = 1;
    proto_nacltai->baseproto.stateful = 0;
    proto_nacltai->baseproto.init = proto_init;
    proto_nacltai->baseproto.encode = proto_encode;
    proto_nacltai->baseproto.decode = proto_decode;
    proto_nacltai->baseproto.set = proto_set;
    proto_nacltai->baseproto.reload = proto_reload;
    proto_nacltai->baseproto.state = 0;

    return (sigma_proto*) proto_nacltai;
}
