//
//  tai.c
//  tai/taia functions
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

#include "tai.h"
#include "pack.h"
#include <time.h>

void taia_pack(uint8_t *s, const struct taia *t)
{
    u64_pack(s, t->sec);
    u32_pack(s + 8, t->nano);
    u32_pack(s + 12, t->atto);
}

void taia_unpack(const uint8_t *s, struct taia *t)
{
    t->sec = u64_unpack(s);
    t->nano = u32_unpack(s + 8);
    t->atto = u32_unpack(s + 12);
}

void taia_now(struct taia *t)
{
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);

    uint64_t abssec = 4611686018427387914ULL + (uint64_t) now.tv_sec;
    if (t->sec == abssec && t->nano == (uint32_t) now.tv_nsec)
    {
        t->atto++;
    }
    else
    {
        t->sec = abssec;
        t->nano = now.tv_nsec;
        t->atto = 0;
    }
}
