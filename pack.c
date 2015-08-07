//
//  pack.c
//  Auxiliary functions to parse, pack and unpack binary types
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

#include "pack.h"

size_t hex2bin(uint8_t* dest, const char* src, size_t count)
{
    size_t i;

    for (i = 0; i < count; i++)
    {
        if (*src >= '0' && *src <= '9') *dest = *src - '0';
        else if (*src >= 'a' && * src <='f') *dest = *src - 'a' + 10;
        else if (*src >= 'A' && * src <='F') *dest = *src - 'A' + 10;
        else break;

        src++; *dest = *dest << 4;

        if (*src >= '0' && *src <= '9') *dest += *src - '0';
        else if (*src >= 'a' && *src <= 'f') *dest += *src - 'a' + 10;
        else if (*src >= 'A' && *src <= 'F') *dest += *src - 'A' + 10;
        else break;

        src++; dest++;
    }

    return i;
}

void u32_pack(uint8_t * buf, uint32_t val)
{
#   if defined(__GNUC__) && (defined(__i486__) || defined(__i586__) || defined(__i686__) || defined(__x86_64__))
        asm(
            "bswap %0\n"
            "mov %0, (%1)\n"
            :
            :"r"(val), "r"(buf)
        );
#   else
        buf[0] = val >> 24;
        buf[1] = val >> 16;
        buf[2] = val >>  8;
        buf[3] = val >>  0;
#   endif
}

uint32_t u32_unpack(const uint8_t * buf)
{
    uint32_t val;

#   if defined(__GNUC__) && (defined(__i486__) || defined(__i586__) || defined(__i686__) || defined(__x86_64__))
        asm(
            "mov (%1), %0\n"
            "bswap %0\n"
            :"=r"(val)
            :"r"(buf)
        );
#   else
        val =
            (uint32_t) buf[0] << 24 |
            (uint32_t) buf[1] << 16 |
            (uint32_t) buf[2] <<  8 |
            (uint32_t) buf[3] <<  0;
#   endif

    return val;
}

void u64_pack(uint8_t * buf, uint64_t val)
{
#   if defined(__GNUC__) && (defined(__i486__) || defined(__i586__) || defined(__i686__))
        asm(
            "bswap %%edx\n"
            "mov %%edx, 0(%1)\n"
            "bswap %%eax\n"
            "mov %%eax, 4(%1)\n"
            :
            :"A"(val), "r"(buf)
        );
#   elif defined(__GNUC__) && defined(__x86_64__)
        asm(
            "bswap %0\n"
            "mov %0, (%1)\n"
            :
            :"r"(val), "r"(buf)
        );
#   else
        buf[0] = val >> 56;
        buf[1] = val >> 48;
        buf[2] = val >> 40;
        buf[3] = val >> 32;
        buf[4] = val >> 24;
        buf[5] = val >> 16;
        buf[6] = val >>  8;
        buf[7] = val >>  0;
#   endif
}

uint64_t u64_unpack(const uint8_t * buf)
{
    uint64_t val;

#   if defined(__GNUC__) && (defined(__i486__) || defined(__i586__) || defined(__i686__))
        asm(
            "mov 0(%1), %%edx\n"
            "bswap %%edx\n"
            "mov 4(%1), %%eax\n"
            "bswap %%eax\n"
            :"=A"(val)
            :"r"(buf)
        );
#   elif defined(__GNUC__) && defined(__x86_64__)
        asm(
            "mov (%1), %0\n"
            "bswap %0\n"
            :"=r"(val)
            :"r"(buf)
        );
#   else
        val =
            (uint64_t) buf[0] << 56 |
            (uint64_t) buf[1] << 48 |
            (uint64_t) buf[2] << 40 |
            (uint64_t) buf[3] << 32 |
            (uint64_t) buf[4] << 24 |
            (uint64_t) buf[5] << 16 |
            (uint64_t) buf[6] <<  8 |
            (uint64_t) buf[7] <<  0;
#   endif

    return val;
}
