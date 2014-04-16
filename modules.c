//
//  modules.c
//  Sigma module code
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
#include <dlfcn.h>

#include "types.h"
#include "modules.h"

sigma_proto* loadproto(char* protoname)
{
    char path[140];
    snprintf(path, sizeof(path), "%s/proto_%s.o", conf->modulepath, protoname);

    void* proto_lib = dlopen(path, RTLD_NOW);

    if (!proto_lib)
    {
        printf("Unable to load protocol %s: %s\n", protoname, dlerror());
        exit(-1);
    }

    void* init = dlsym(proto_lib, "proto_descriptor");

    if (!init)
    {
        printf("Unable to load protocol %s: %s\n", protoname, dlerror());
        exit(-1);
    }

    sigma_proto* proto = ((sigma_proto* (*)()) init)();

    return proto;
}

sigma_intf* loadinterface(char* intfname)
{
    char path[140];
    snprintf(path, sizeof(path), "%s/intf_%s.o", conf->modulepath, intfname);

    void* intf_lib = dlopen(path, RTLD_NOW);

    if (!intf_lib)
    {
        printf("Unable to load interface %s: %s\n", intfname, dlerror());
        exit(-1);
    }

    void* init = dlsym(intf_lib, "intf_descriptor");

    if (!init)
    {
        printf("Unable to load interface %s: %s\n", intfname, dlerror());
        exit(-1);
    }

    sigma_intf* intf = ((sigma_intf* (*)()) init)();

    return intf;
}

