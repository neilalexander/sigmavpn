//
//  intf_tuntap.h
//  Sigma TUN/TAP interface code
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
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "../types.h"
#include "../intf.h"

#ifdef linux
#include <net/if.h>
#include <linux/if_tun.h>
#include <linux/if_ether.h>
#endif

typedef struct sigma_intf_tuntap
{
    sigma_intf baseintf;

    int filedesc;
    char nodename[16];
    int tunmode;
    int protocolinfo;

    long buffersize;
}
sigma_intf_tuntap;

static long intf_write(sigma_intf *instance, char* input, long len)
{
    sigma_intf_tuntap* tuntap = (sigma_intf_tuntap*) instance;

    if (!tuntap->filedesc < 0)
        return -1;

    len = write(tuntap->baseintf.filedesc, input, tuntap->buffersize);

    return len;
}

static long intf_read(sigma_intf *instance, char* output, long len)
{
    sigma_intf_tuntap* tuntap = (sigma_intf_tuntap*) instance;

    if (!tuntap->filedesc < 0)
        return -1;

    len = read(tuntap->baseintf.filedesc, output, tuntap->buffersize);

    return len;
}

static int intf_init(sigma_intf* instance)
{
    sigma_intf_tuntap* tuntap = (sigma_intf_tuntap*) instance;

    if (!tuntap->nodename)
        strcpy(tuntap->nodename, "/dev/tun0");

    #ifdef __linux__
        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));

        if ((tuntap->baseintf.filedesc = open("/dev/net/tun", O_RDWR)) < 0)
        {
            fprintf(stderr, "Unable to find /dev/net/tun\n");
            return -1;
        }

        strcpy(ifr.ifr_name, tuntap->nodename);
        ifr.ifr_flags = tuntap->tunmode ? IFF_TUN : IFF_TAP;
        ifr.ifr_flags |= tuntap->protocolinfo ? 0 : IFF_NO_PI;

        if (ioctl(tuntap->baseintf.filedesc, TUNSETIFF, (void *) &ifr) < 0)
        {
            fprintf(stderr, "Unable to configure tuntap device\n");
            return -1;
        }
    #else
        if ((tuntap->baseintf.filedesc = open(tuntap->nodename, O_RDWR)) < 0)
        {
            fprintf(stderr, "Unable to open tuntap device '%s'\n", tuntap->nodename);
            return -1;
        }
    #endif

    return 0;
}

static int intf_set(sigma_intf* instance, char* param, void* value)
{
    sigma_intf_tuntap* tuntap = (sigma_intf_tuntap*) instance;

    if (strcmp(param, "interface") == 0)
        memcpy(tuntap->nodename, (char*) value, 16);

    if (strcmp(param, "tunmode") == 0)
        tuntap->tunmode = *(int*) value;

    if (strcmp(param, "protocolinfo") == 0)
        tuntap->protocolinfo = *(int*) value;

    return 0;
}

static int intf_reload(sigma_intf* instance)
{
        sigma_intf_tuntap* tuntap = (sigma_intf_tuntap*) instance;

        if (close(tuntap->baseintf.filedesc) == -1)
        {
                printf("Interface close failed\n");
                return -1;
        }

        tuntap->baseintf.filedesc = -1;

        intf_init(instance);

        return 0;
}

extern sigma_intf* intf_descriptor()
{
    sigma_intf_tuntap* intf_tuntap = calloc(1, sizeof(sigma_intf_tuntap));

    intf_tuntap->baseintf.init = intf_init;
    intf_tuntap->baseintf.read = intf_read;
    intf_tuntap->baseintf.write = intf_write;
    intf_tuntap->baseintf.set = intf_set;
    intf_tuntap->baseintf.reload = intf_reload;
    intf_tuntap->baseintf.updateremote = 0;
    intf_tuntap->buffersize = (long) MAX_BUFFER_SIZE;

    return (sigma_intf*) intf_tuntap;
}
