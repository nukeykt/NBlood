// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.
//
// This file has been modified from Ken Silverman's original release
// by Jonathon Fowler (jf@jonof.id.au)
// by the EDuke32 team (development@voidpoint.com)

#ifndef cache1d_h_
#define cache1d_h_

#include "compat.h"

typedef struct
{
    char *    lock;
    intptr_t *hand;
    int32_t   leng;
} cacheitem_t;

enum cachelock_t: char
{
    CACHE1D_FREE               = 1,
    CACHE1D_UNLOCKED           = 199,
    CACHE1D_LOCKED             = 200,
    CACHE1D_LOCKED_PERMANENTLY = 255,
};

extern int g_cacheNumEntries;
extern cacheitem_t * g_cache;

#if !defined DEBUG_ALLOCACHE_AS_MALLOC
void cacheInitBuffer(intptr_t dacachestart, int32_t dacachesize);
void cacheAllocateBlock(intptr_t *newhandle, int32_t newbytes, char *newlockptr);
void cacheAgeEntries(void);
#endif

#endif // cache1d_h_

