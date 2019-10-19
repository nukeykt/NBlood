// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.
//
// This file has been modified from Ken Silverman's original release
// by Jonathon Fowler (jf@jonof.id.au)
// by the EDuke32 team (development@voidpoint.com)

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

#if !defined DEBUG_ALLOCACHE_AS_MALLOC
#define CACHEINCREMENT 1024
#define MEANCACHEBLOCKSIZE 24576

static int g_cacheMaxEntries;
static int32_t g_cacheSize;
static char zerochar;
static intptr_t g_cacheBaseAddress;
static int32_t lockrecip[200];

int g_cacheNumEntries;
cacheitem_t * g_cache = nullptr;

static void reportandexit(const char *errormessage);


void cacheInitBuffer(intptr_t dacachestart, int32_t dacachesize)
{
    int32_t i;

    for (i=1; i<200; i++)
        lockrecip[i] = tabledivide32_noinline(1<<28, 200-i);

    // we allocate this block with aligned_alloc, but I'm leaving this here in
    // case we run on any platforms that just implement it as regular malloc

    g_cacheBaseAddress = ((uintptr_t)dacachestart+15)&~(uintptr_t)0xf;
    g_cacheSize = (dacachesize-(((uintptr_t)(dacachestart))&0xf))&~(uintptr_t)0xf;

    g_cacheMaxEntries = max(g_cacheSize / MEANCACHEBLOCKSIZE, 8192); 
    g_cache = (cacheitem_t *) Xaligned_alloc(Bgetpagesize(), g_cacheMaxEntries * sizeof(cacheitem_t));
    Bmemset(g_cache, 0, g_cacheMaxEntries * sizeof(cacheitem_t));

    g_cache[0].leng = g_cacheSize;
    g_cache[0].lock = &zerochar;
    g_cacheNumEntries = 1;

#ifdef DEBUGGINGAIDS
    initprintf("Cache object array initialized with \"%d\" entries. \n", g_cacheMaxEntries);
#endif
    initprintf("Initialized %.1fM cache\n", (float)(dacachesize/1024.f/1024.f));
}

// Dynamic cache resizing -- increase cache array size when full
static inline void inc_and_check_cacnum(void)
{
    if (++g_cacheNumEntries >= g_cacheMaxEntries)
    {
        auto new_cac = (cacheitem_t *)Xaligned_alloc(Bgetpagesize(), (g_cacheMaxEntries + CACHEINCREMENT) * sizeof(cacheitem_t));

        Bmemset(new_cac + g_cacheMaxEntries, 0, CACHEINCREMENT * sizeof(cacheitem_t));
        Bmemcpy(new_cac, g_cache, g_cacheMaxEntries * sizeof(cacheitem_t));

        Xaligned_free(g_cache);
        g_cache = new_cac;

#ifdef DEBUGGINGAIDS
        initprintf("Cache increased from \'%d\' to \'%d\' entries. \n", g_cacheMaxEntries, g_cacheMaxEntries + CACHEINCREMENT);
#endif
        g_cacheMaxEntries += CACHEINCREMENT;
    }
}

int32_t cacheFindBlock(int32_t newbytes, int32_t *besto, int32_t *bestz)
{
    int32_t bestval = 0x7fffffff;

    for (native_t z=g_cacheNumEntries-1, o1=g_cacheSize; z>=0; z--)
    {
        o1 -= g_cache[z].leng;

        int32_t const o2 = o1 + newbytes;

        if (o2 > g_cacheSize)
            continue;

        int32_t daval = 0;

        for (native_t i=o1, zz=z; i<o2; i+=g_cache[zz++].leng)
        {
            if (*g_cache[zz].lock == 0)
                continue;
            else if (*g_cache[zz].lock >= CACHE1D_LOCKED)
            {
                daval = 0x7fffffff;
                break;
            }

            // Potential for eviction increases with
            //  - smaller item size
            //  - smaller lock byte value (but in [1 .. 199])
            daval += mulscale32(g_cache[zz].leng + 65536, lockrecip[*g_cache[zz].lock]);

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

    if (EDUKE32_PREDICT_FALSE((unsigned)newbytes > (unsigned)g_cacheSize))
    {
        initprintf("Cachesize: %d\n",g_cacheSize);
        initprintf("*Newhandle: 0x%" PRIxPTR ", Newbytes: %d, *Newlock: %d\n",(intptr_t)newhandle,newbytes,*newlockptr);
        reportandexit("BUFFER TOO BIG TO FIT IN CACHE!");
    }

    int32_t bestz = 0;
    int32_t besto = 0;
    int cnt = g_cacheNumEntries-1;

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

    for (;sucklen<0; sucklen+=g_cache[suckz++].leng)
        if (*g_cache[suckz].lock)
            *g_cache[suckz].hand = 0;

    //Remove all blocks except 1
    suckz -= bestz+1;
    g_cacheNumEntries -= suckz;

    Bmemmove(&g_cache[bestz], &g_cache[bestz + suckz], (g_cacheNumEntries - bestz) * sizeof(cacheitem_t));

    g_cache[bestz].hand = newhandle;
    *newhandle      = g_cacheBaseAddress + besto;
    g_cache[bestz].leng = newbytes;
    g_cache[bestz].lock = newlockptr;

    //Add new empty block if necessary
    if (sucklen <= 0)
        return;

    if (++bestz == g_cacheNumEntries)
    {
        inc_and_check_cacnum();
        g_cache[bestz].leng = sucklen;
        g_cache[bestz].lock = &zerochar;
        return;
    }

    if (*g_cache[bestz].lock == 0)
    {
        g_cache[bestz].leng += sucklen;
        return;
    }

    inc_and_check_cacnum();

    for (native_t z=g_cacheNumEntries-1; z>bestz; z--)
        g_cache[z] = g_cache[z-1];

    g_cache[bestz].leng = sucklen;
    g_cache[bestz].lock = &zerochar;
}

void cacheAgeEntries(void)
{
    static int agecount;

    if (agecount >= g_cacheNumEntries)
        agecount = g_cacheNumEntries-1;

    int cnt = min(g_cacheMaxEntries >> 4, g_cacheNumEntries-1);

    while(cnt--)
    {
        // If we have pointer to lock char and it's in [2 .. 199], decrease.
        if (g_cache[agecount].lock)
        {
             if ((((*g_cache[agecount].lock)-2)&255) < CACHE1D_UNLOCKED-1)
                (*g_cache[agecount].lock)--;
             else if (*g_cache[agecount].lock == CACHE1D_LOCKED_PERMANENTLY)
                 cnt++;
        }

        if (--agecount < 0)
            agecount = g_cacheNumEntries-1;
    }
}

static void reportandexit(const char *errormessage)
{
    //setvmode(0x3);
    int32_t j = 0;
    for (native_t i = 0; i < g_cacheNumEntries; i++)
    {
        buildprint(i, "- ");

        if (g_cache[i].hand)
            initprintf("ptr: 0x%" PRIxPTR ", ", *g_cache[i].hand);
        else
            initprintf("ptr: NULL, ");

        initprintf("leng: %d, ", g_cache[i].leng);

        if (g_cache[i].lock)
            initprintf("lock: %d\n", *g_cache[i].lock);
        else
            initprintf("lock: NULL\n");

        j += g_cache[i].leng;
    }

    initprintf("Cachesize = %d\n", g_cacheSize);
    initprintf("Cacnum = %d\n", g_cacheNumEntries);
    initprintf("Cache length sum = %d\n", j);

    static char msg[128];
    Bsnprintf(msg, sizeof(msg), "ERROR: %s\n", errormessage);
    fatal_exit(msg);
}
#else
void cacheInitBuffer(intptr_t dacachestart, int32_t dacachesize)
{
    UNREFERENCED_PARAMETER(dacachestart);
    UNREFERENCED_PARAMETER(dacachesize);
}

void cacheAllocateBlock(intptr_t *newhandle, int32_t newbytes, char *newlockptr)
{
    UNREFERENCED_PARAMETER(newlockptr);
    *newhandle = (intptr_t)Xmalloc(newbytes);
}

void cacheAgeEntries(void) {}

static void reportandexit(const char *errormessage)
{
    static char msg[128];
    Bsnprintf(msg, sizeof(msg), "ERROR: %s\n", errormessage);
    fatal_exit(msg);
}
#endif
