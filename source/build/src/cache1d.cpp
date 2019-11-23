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

#define CACHEINCREMENT 1024
#define MEANCACHEBLOCKSIZE 24576

static bool g_cacheInit;
static char zerochar;
static int32_t lockrecip[200];

cache1d g_cache;

#if !defined DEBUG_ALLOCACHE_AS_MALLOC
static int osdfunc_cacheinfo(osdcmdptr_t UNUSED(parm))
{
    UNREFERENCED_CONST_PARAMETER(parm);

    g_cache.report();

    return OSDCMD_OK;
}

void cache1d::initBuffer(intptr_t dacachestart, int32_t dacachesize)
{
    if (!g_cacheInit)
    {
        for (int i = 1; i < 200; i++)
            lockrecip[i] = tabledivide32_noinline(1 << 28, 200 - i);

        OSD_RegisterFunction("cacheinfo", "cacheinfo: displays cache statistics", osdfunc_cacheinfo);

        g_cacheInit = true;
    }

    // we allocate this block with aligned_alloc, but I'm leaving this here in
    // case we run on any platforms that just implement it as regular malloc

    m_baseAddress = ((uintptr_t)dacachestart + 15) & ~(uintptr_t)0xf;
    m_totalSize   = (dacachesize - (((uintptr_t)(dacachestart)) & 0xf)) & ~(uintptr_t)0xf;

    m_maxBlocks = max(m_totalSize / MEANCACHEBLOCKSIZE, 8192);
    m_index     = (cacheindex_t *)Xaligned_alloc(Bgetpagesize(), m_maxBlocks * sizeof(cacheindex_t));
    Bmemset(m_index, 0, m_maxBlocks * sizeof(cacheindex_t));

    m_index[0].leng = m_totalSize;
    m_index[0].lock = &zerochar;

    m_numBlocks = 1;

#ifdef DEBUGGINGAIDS
    buildprint("Cache object array initialized with ", m_maxBlocks, " entries.\n");
#endif
    initprintf("Initialized %.1fM cache\n", (float)(dacachesize/1024.f/1024.f));
}

// Dynamic cache resizing -- increase cache array size when full
void cache1d::inc_and_check_cacnum(void)
{
    if (++m_numBlocks >= m_maxBlocks)
    {
        auto new_index = (cacheindex_t *)Xaligned_alloc(Bgetpagesize(), (m_maxBlocks + CACHEINCREMENT) * sizeof(cacheindex_t));

        Bmemset(new_index + m_maxBlocks, 0, CACHEINCREMENT * sizeof(cacheindex_t));
        Bmemcpy(new_index, m_index, m_maxBlocks * sizeof(cacheindex_t));

        Xaligned_free(m_index);
        m_index = new_index;

#ifdef DEBUGGINGAIDS
        buildprint("Cache increased from \'", m_maxBlocks, "\' to \'", m_maxBlocks + CACHEINCREMENT, "\' entries.\n");
#endif
        m_maxBlocks += CACHEINCREMENT;
    }
}

int32_t cache1d::findBlock(int32_t newbytes, int32_t *besto, int32_t *bestz)
{
    int32_t bestval = 0x7fffffff;

    for (native_t z=m_numBlocks-1, o1=m_totalSize; z>=0; z--)
    {
        o1 -= m_index[z].leng;

        int32_t const o2 = o1 + newbytes;

        if (o2 > m_totalSize)
            continue;

        int32_t daval = 0;

        for (native_t i=o1, zz=z; i<o2; i+=m_index[zz++].leng)
        {
            if (*m_index[zz].lock == 0)
                continue;
            else if (*m_index[zz].lock >= CACHE1D_LOCKED)
            {
                daval = 0x7fffffff;
                break;
            }

            // Potential for eviction increases with
            //  - smaller item size
            //  - smaller lock byte value (but in [1 .. 199])
            daval += mulscale32(m_index[zz].leng + 65536, lockrecip[*m_index[zz].lock]);

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

void cache1d::allocateBlock(intptr_t* newhandle, int32_t newbytes, char* newlockptr)
{
    // Make all requests a multiple of the system page size
    int const pageSize = Bgetpagesize();
    newbytes = (newbytes + pageSize-1) & ~(pageSize-1);

#ifdef DEBUGGINGAIDS
    if (EDUKE32_PREDICT_FALSE(!newlockptr || *newlockptr == 0))
    {
        report();
        fatal_exit("ALLOCACHE CALLED WITH LOCK OF 0!");
    }
#endif

    if (EDUKE32_PREDICT_FALSE((unsigned)newbytes > (unsigned)m_totalSize))
    {
        buildprint("Cache size: ", m_totalSize, "\n");
        buildprint("*Newhandle: 0x", hex((intptr_t)newhandle),", Newbytes: ", newbytes, ", *Newlock: ", *newlockptr, "\n");
        report();
        fatal_exit("BUFFER TOO BIG TO FIT IN CACHE!");
    }

    int32_t bestz = 0;
    int32_t besto = 0;
    int cnt = m_numBlocks-1;

    // if we can't find a block, try to age the cache until we can
    // it's better than the alternative of aborting the entire program
    while (findBlock(newbytes, &besto, &bestz) == 0x7fffffff)
    {
        OSD_Printf("WARNING: request for %d byte block exhausted cache!\n"
                   "Attempting to make it fit...\n",
                   newbytes);
        ageBlocks();
        if (!cnt--)
        {
            report();
            fatal_exit("CACHE SPACE ALL LOCKED UP!");
        }
    }

    //printf("%d %d %d\n",besto,newbytes,*newlockptr);

    //Suck things out
    int32_t sucklen = -newbytes;
    int32_t suckz = bestz;

    for (;sucklen<0; sucklen+=m_index[suckz++].leng)
        if (*m_index[suckz].lock)
            *m_index[suckz].hand = 0;

    //Remove all blocks except 1
    suckz -= bestz+1;
    m_numBlocks -= suckz;

    Bmemmove(&m_index[bestz], &m_index[bestz + suckz], (m_numBlocks - bestz) * sizeof(cacheindex_t));

    m_index[bestz].hand = newhandle;
    *newhandle      = m_baseAddress + besto;
    m_index[bestz].leng = newbytes;
    m_index[bestz].lock = newlockptr;

    //Add new empty block if necessary
    if (sucklen <= 0)
        return;

    if (++bestz == m_numBlocks)
    {
        inc_and_check_cacnum();
        m_index[bestz].leng = sucklen;
        m_index[bestz].lock = &zerochar;
        return;
    }

    if (*m_index[bestz].lock == 0)
    {
        m_index[bestz].leng += sucklen;
        return;
    }

    inc_and_check_cacnum();

    for (native_t z=m_numBlocks-1; z>bestz; z--)
        m_index[z] = m_index[z-1];

    m_index[bestz].leng = sucklen;
    m_index[bestz].lock = &zerochar;
}

void cache1d::ageBlocks(void)
{
    static int agecount;

    if (agecount >= m_numBlocks)
        agecount = m_numBlocks-1;

    int cnt = min(m_maxBlocks >> 4, m_numBlocks-1);

    while(cnt--)
    {
        // If we have pointer to lock char and it's in [2 .. 199], decrease.
        if (m_index[agecount].lock)
        {
             if ((((*m_index[agecount].lock)-2)&255) < CACHE1D_UNLOCKED-1)
                (*m_index[agecount].lock)--;
             else if (*m_index[agecount].lock == CACHE1D_PERMANENT)
                 cnt++;
        }

        if (--agecount < 0)
            agecount = m_numBlocks-1;
    }
}

void cache1d::report(void)
{
    int32_t usedSize = 0;

    for (native_t i = 0; i < m_numBlocks-1; i++)
    {
        buildprint("#", i, "- ");

        if (m_index[i].hand)
            buildprint("PTR: 0x", hex(*m_index[i].hand), ", ");
        else
            buildprint("PTR: NULL, ");

        buildprint("SIZ: ", m_index[i].leng, ", ");

        if (m_index[i].lock)
        {
            buildprint("LCK: ", *m_index[i].lock, "\n");

            if (*m_index[i].lock)
                usedSize += m_index[i].leng;
        }
        else
            buildprint("LCK: NULL\n");
    }

    buildprint("Cache used: ", usedSize, "\n");
    buildprint("Remaining:  ", m_totalSize - usedSize, "\n");
    buildprint("Total size: ", m_totalSize, "\n");
    buildprint("Num blocks: ", m_numBlocks, "\n");
}
#else
void cache1d::initBuffer(intptr_t dacachestart, int32_t dacachesize)
{
    UNREFERENCED_PARAMETER(dacachestart);
    UNREFERENCED_PARAMETER(dacachesize);
}

void cache1d::allocateBlock(intptr_t *newhandle, int32_t newbytes, char *newlockptr)
{
    UNREFERENCED_PARAMETER(newlockptr);
    *newhandle = (intptr_t)Xmalloc(newbytes);
}

void cache1d::ageBlocks(void) {}
void cache1d::report(void) {}
#endif
