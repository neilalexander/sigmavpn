//
//  proto_raw.c
//  Sigma raw protocol code
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../types.h"
#include "../proto.h"

static int proto_encode(sigma_proto *instance, unsigned char* input, unsigned char* output, unsigned int len)
{
    if (instance->state == 1)
    {
        memcpy(output, input, len);
        return len;
    }

    return -1;
}

static int proto_decode(sigma_proto *instance, unsigned char* input, unsigned char* output, unsigned int len)
{
    if (instance->state == 1)
    {
        memcpy(output, input, len);
        return len;
    }

    return -1;
}

static int proto_init(sigma_proto *instance)
{
    instance->state = 1;
    return 0;
}

static int proto_set(sigma_proto* instance, char* param, char* value)
{
    if (strcmp(param, "state") == 0)
        instance->state = atoi(value);

    return 0;
}

static int proto_reload(sigma_proto *instance)
{
    instance->state = 1;
    return 0;
}

extern sigma_proto* proto_descriptor()
{
    sigma_proto* proto_raw = calloc(1, sizeof(sigma_proto));

    proto_raw->encrypted = 0;
    proto_raw->stateful = 0;
    proto_raw->init = proto_init;
    proto_raw->encode = proto_encode;
    proto_raw->decode = proto_decode;
    proto_raw->reload = proto_reload;
    proto_raw->set = proto_set;
    proto_raw->state = 0;

    return proto_raw;
}
