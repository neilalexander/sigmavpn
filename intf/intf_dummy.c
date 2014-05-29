//
//  intf_dummy.c
//  Sigma dummy interface code
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

#include "../types.h"
#include "../intf.h"

static long intf_write(sigma_intf *instance, char* input, long len)
{
    return len;
}

static long intf_read(sigma_intf *instance, char* output, long len)
{
    return len;
}

static int intf_set(sigma_intf *instance, char* param, char* value)
{
    return 0;
}

static int intf_init(sigma_intf *instance)
{
    return 0;
}

static int intf_reload(sigma_intf *instance)
{
    return 0;
}

extern sigma_intf* intf_descriptor()
{
    sigma_intf* intf_dummy = calloc(1, sizeof(sigma_intf));

    intf_dummy->init = intf_init;
    intf_dummy->read = intf_read;
    intf_dummy->write = intf_write;
    intf_dummy->set = intf_set;
    intf_dummy->reload = intf_reload;
    intf_dummy->state = 0;

    return intf_dummy;
}
