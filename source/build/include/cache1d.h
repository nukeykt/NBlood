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
} cactype;

enum {
    CACHE1D_ENTRY_FREE      = 1,
    CACHE1D_ENTRY_PERMANENT = 255,
};

void	cacheInitBuffer(intptr_t dacachestart, int32_t dacachesize);
void	cacheAllocateBlock(intptr_t *newhandle, int32_t newbytes, char *newlockptr);
void	cacheAgeEntries(void);

#endif // cache1d_h_

