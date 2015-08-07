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
#include <sys/time.h>
#include <pthread.h>

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
    static pthread_spinlock_t update_lock = PTHREAD_SPINLOCK_INITIALIZER;
    static struct timespec last = { 0, 0 };
    static uint32_t attos = 0;

    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);

    pthread_spin_lock(&update_lock);

    if (
		(now.tv_sec > last.tv_sec) ||
		(now.tv_sec == last.tv_sec && now.tv_nsec > last.tv_nsec)
	)
    {
        last = now;
        attos = 0;
    }

    t->sec = 4611686018427387914ULL + (uint64_t) now.tv_sec;
    t->nano = (uint32_t) now.tv_nsec;
    t->atto = attos++;

    pthread_spin_unlock(&update_lock);
}
