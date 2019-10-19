// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.
//
// This file has been modified from Ken Silverman's original release
// by Jonathon Fowler (jf@jonof.id.au)
// by the EDuke32 team (development@voidpoint.com)


#ifdef _WIN32
// for FILENAME_CASE_CHECK
# define NEED_SHELLAPI_H
# include "windows_inc.h"
#endif

#include "baselayer.h"
#include "cache1d.h"
#include "compat.h"
#include "klzw.h"
#include "lz4.h"
#include "osd.h"
#include "pragmas.h"
#include "vfs.h"

//   This module keeps track of a standard linear cacheing system.
//   To use this module, here's all you need to do:
//
//   Step 1: Allocate a nice BIG buffer, like from 1MB-4MB and
//           Call initcache(int32_t cachestart, int32_t cachesize) where
//
//              cachestart = (intptr_t)(pointer to start of BIG buffer)
//              cachesize = length of BIG buffer
//
//   Step 2: Call allocache(intptr_t *bufptr, int32_t bufsiz, char *lockptr)
//              whenever you need to allocate a buffer, where:
//
//              *bufptr = pointer to multi-byte pointer to buffer
//                 Confused?  Using this method, cache2d can remove
//                 previously allocated things from the cache safely by
//                 setting the multi-byte pointer to 0.
//              bufsiz = number of bytes to allocate
//              *lockptr = pointer to locking char which tells whether
//                 the region can be removed or not.  If *lockptr = 0 then
//                 the region is not locked else its locked.
//
//   Step 3: If you need to remove everything from the cache, or every
//           unlocked item from the cache, you can call uninitcache();
//              Call uninitcache(0) to remove all unlocked items, or
//              Call uninitcache(1) to remove everything.
//           After calling uninitcache, it is still ok to call allocache
//           without first calling initcache.

#define CACHEINCREMENT 1024
#define MEANCACHEBLOCKSIZE 24576
int32_t cachemaxobjects;

#if !defined DEBUG_ALLOCACHE_AS_MALLOC
static int32_t cachesize;
static char zerochar;
static intptr_t cachestart;
static int32_t lockrecip[200];

int32_t cacnum;
cactype * cac = nullptr;
#endif


static void reportandexit(const char *errormessage);


void cacheInitBuffer(intptr_t dacachestart, int32_t dacachesize)
{
#ifndef DEBUG_ALLOCACHE_AS_MALLOC
    int32_t i;

    for (i=1; i<200; i++)
        lockrecip[i] = tabledivide32_noinline(1<<28, 200-i);

    // we allocate this block with aligned_alloc, but I'm leaving this here in
    // case we run on any platforms that just implement it as regular malloc

    cachestart = ((uintptr_t)dacachestart+15)&~(uintptr_t)0xf;
    cachesize = (dacachesize-(((uintptr_t)(dacachestart))&0xf))&~(uintptr_t)0xf;

    cachemaxobjects = max(cachesize / MEANCACHEBLOCKSIZE, 8192); 
    cac = (cactype *) Xaligned_alloc(Bgetpagesize(), cachemaxobjects * sizeof(cactype));
    Bmemset(cac, 0, cachemaxobjects * sizeof(cactype));

    cac[0].leng = cachesize;
    cac[0].lock = &zerochar;
    cacnum = 1;

    #ifdef DEBUGGINGAIDS
    initprintf("Cache object array initialized with \"%d\" entries. \n", cachemaxobjects);
    #endif
    initprintf("Initialized %.1fM cache\n", (float)(dacachesize/1024.f/1024.f));
#else
    UNREFERENCED_PARAMETER(dacachestart);
    UNREFERENCED_PARAMETER(dacachesize);
#endif
}

#ifdef DEBUG_ALLOCACHE_AS_MALLOC
void cacheAllocateBlock(intptr_t *newhandle, int32_t newbytes, char *newlockptr)
{
    UNREFERENCED_PARAMETER(newlockptr);

    *newhandle = (intptr_t)Xmalloc(newbytes);
}
#else
// Dynamic cache resizing -- increase cache array size when full
static inline void inc_and_check_cacnum(void)
{
    if (++cacnum > cachemaxobjects)
    {
        auto new_cac = (cactype *)Xaligned_alloc(Bgetpagesize(), (cachemaxobjects + CACHEINCREMENT) * sizeof(cactype));

        Bmemset(new_cac + cachemaxobjects, 0, CACHEINCREMENT * sizeof(cactype));
        Bmemcpy(new_cac, cac, cachemaxobjects * sizeof(cactype));

        Xaligned_free(cac);
        cac = new_cac;

#ifdef DEBUGGINGAIDS
        initprintf("Cache increased from \'%d\' to \'%d\' entries. \n", cachemaxobjects, cachemaxobjects + CACHEINCREMENT);
#endif

        cachemaxobjects += CACHEINCREMENT;
    }
}

int32_t cacheFindBlock(int32_t newbytes, int32_t *besto, int32_t *bestz)
{
    int32_t bestval = 0x7fffffff;

    for (native_t z=cacnum-1, o1=cachesize; z>=0; z--)
    {
        o1 -= cac[z].leng;

        int32_t const o2 = o1 + newbytes;

        if (o2 > cachesize)
            continue;

        int32_t daval = 0;

        for (native_t i=o1, zz=z; i<o2; i+=cac[zz++].leng)
        {
            if (*cac[zz].lock == 0)
                continue;
            else if (*cac[zz].lock >= 200)
            {
                daval = 0x7fffffff;
                break;
            }

            // Potential for eviction increases with
            //  - smaller item size
            //  - smaller lock byte value (but in [1 .. 199])
            daval += mulscale32(cac[zz].leng + 65536, lockrecip[*cac[zz].lock]);

            if (daval >= bestval)
                break;
        }

        if (daval < bestval)
        {
            bestval = daval;
            *besto  = o1;
            *bestz  = z;

            if (bestval == 0)
                break;
        }
    }

    return bestval;
}

void cacheAllocateBlock(intptr_t* newhandle, int32_t newbytes, char* newlockptr)
{
    // Make all requests a multiple of the system page size
    int const pageSize = Bgetpagesize();
    newbytes = (newbytes + pageSize-1) & ~(pageSize-1);

#ifdef DEBUGGINGAIDS
    if (EDUKE32_PREDICT_FALSE(!newlockptr || *newlockptr == 0))
        reportandexit("ALLOCACHE CALLED WITH LOCK OF 0!");
#endif

    if (EDUKE32_PREDICT_FALSE((unsigned)newbytes > (unsigned)cachesize))
    {
        initprintf("Cachesize: %d\n",cachesize);
        initprintf("*Newhandle: 0x%" PRIxPTR ", Newbytes: %d, *Newlock: %d\n",(intptr_t)newhandle,newbytes,*newlockptr);
        reportandexit("BUFFER TOO BIG TO FIT IN CACHE!");
    }

    int32_t bestz = 0;
    int32_t besto = 0;
    int cnt = cacnum-1;

    // if we can't find a block, try to age the cache until we can
    // it's better than the alternative of aborting the entire program
    while (cacheFindBlock(newbytes, &besto, &bestz) == 0x7fffffff)
    {
        OSD_Printf("WARNING: request for %d byte block exhausted cache!\n"
                   "Attempting to make it fit...\n",
                   newbytes);
        cacheAgeEntries();
        if (!cnt--) reportandexit("CACHE SPACE ALL LOCKED UP!");
    }

    //printf("%d %d %d\n",besto,newbytes,*newlockptr);

    //Suck things out
    int32_t sucklen = -newbytes;
    int32_t suckz = bestz;

    for (;sucklen<0; sucklen+=cac[suckz++].leng)
        if (*cac[suckz].lock)
            *cac[suckz].hand = 0;

    //Remove all blocks except 1
    suckz -= bestz+1;
    cacnum -= suckz;

    Bmemmove(&cac[bestz], &cac[bestz + suckz], (cacnum - bestz) * sizeof(cactype));

    cac[bestz].hand = newhandle;
    *newhandle      = cachestart + besto;
    cac[bestz].leng = newbytes;
    cac[bestz].lock = newlockptr;

    //Add new empty block if necessary
    if (sucklen <= 0)
        return;

    if (++bestz == cacnum)
    {
        inc_and_check_cacnum();
        cac[bestz].leng = sucklen;
        cac[bestz].lock = &zerochar;
        return;
    }

    if (*cac[bestz].lock == 0)
    {
        cac[bestz].leng += sucklen;
        return;
    }

    inc_and_check_cacnum();

    for (native_t z=cacnum-1; z>bestz; z--)
        cac[z] = cac[z-1];

    cac[bestz].leng = sucklen;
    cac[bestz].lock = &zerochar;
}
#endif

void cacheAgeEntries(void)
{
#ifndef DEBUG_ALLOCACHE_AS_MALLOC
    static int agecount;

    if (agecount >= cacnum)
        agecount = cacnum-1;

    int cnt = min(cachemaxobjects >> 4, cacnum-1);

    while(cnt--)
    {
        // If we have pointer to lock char and it's in [2 .. 199], decrease.
        if (cac[agecount].lock)
        {
             if ((((*cac[agecount].lock)-2)&255) < 198)
                (*cac[agecount].lock)--;
             else if (*cac[agecount].lock == 255)
                 cnt++;
        }

        if (--agecount < 0)
            agecount = cacnum-1;
    }
#endif
}

static void reportandexit(const char *errormessage)
{
#ifndef DEBUG_ALLOCACHE_AS_MALLOC
    //setvmode(0x3);
    int32_t j = 0;
    for (native_t i = 0; i < cacnum; i++)
    {
        buildprint(i, "- ");

        if (cac[i].hand)
            initprintf("ptr: 0x%" PRIxPTR ", ", *cac[i].hand);
        else
            initprintf("ptr: NULL, ");

        initprintf("leng: %d, ", cac[i].leng);

        if (cac[i].lock)
            initprintf("lock: %d\n", *cac[i].lock);
        else
            initprintf("lock: NULL\n");

        j += cac[i].leng;
    }

    initprintf("Cachesize = %d\n", cachesize);
    initprintf("Cacnum = %d\n", cacnum);
    initprintf("Cache length sum = %d\n", j);
#endif

    static char msg[128];
    Bsnprintf(msg, sizeof(msg), "ERROR: %s\n", errormessage);
    fatal_exit(msg);
}

