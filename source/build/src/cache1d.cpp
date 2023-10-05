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
#include "build.h"
#include "cache1d.h"
#include "compat.h"
#include "klzw.h"
#include "lz4.h"
#include "osd.h"
#include "pragmas.h"
#include "vfs.h"

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

#define MAXCACHEOBJECTS 16384

void cache1d::reset(void)
{
    Bmemset(m_index, 0, m_maxBlocks * sizeof(cacheindex_t));

    m_index[0].leng = m_totalSize;
    m_index[0].lock = &zerochar;

    m_numBlocks = 1;
}

void cache1d::initBuffer(intptr_t dacachestart, uint32_t dacachesize, uint32_t)
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

    m_maxBlocks = MAXCACHEOBJECTS;
    m_index     = (cacheindex_t *)Xaligned_alloc(16, m_maxBlocks * sizeof(cacheindex_t));

    reset();

    DVLOG_F(LOG_DEBUG, "Cache object array initialized with %d entries.", m_maxBlocks);
    LOG_F(INFO, "Initialized %.1fM cache", (float)(dacachesize/1024.f/1024.f));
}

// Dynamic cache resizing -- increase cache array size when full
void cache1d::inc_and_check_cacnum(void)
{
    if (++m_numBlocks >= m_maxBlocks)
    {
        auto new_index = (cacheindex_t *)Xaligned_alloc(Bgetpagesize(), (m_maxBlocks + MINCACHEINDEXSIZE) * sizeof(cacheindex_t));

        Bmemset(new_index + m_maxBlocks, 0, MINCACHEINDEXSIZE * sizeof(cacheindex_t));
        Bmemcpy(new_index, m_index, m_maxBlocks * sizeof(cacheindex_t));

        Xaligned_free(m_index);
        m_index = new_index;
        m_maxBlocks += MINCACHEINDEXSIZE;
        DLOG_F(INFO, "Cache size increased by %d to new max of %d entries", MINCACHEINDEXSIZE, m_maxBlocks);


    }
}

void cache1d::allocateBlock(intptr_t *newhandle, int32_t newbytes, char *newlockptr)
{
    int i, z, zz, bestz=0, daval, bestval, besto=0, o1, o2, sucklen, suckz;

    newbytes = ((newbytes+15)& ~15);

    if (EDUKE32_PREDICT_FALSE((unsigned)newbytes > (unsigned)m_totalSize))
    {
        LOG_F(ERROR, "Cache size: %d", m_totalSize);
        LOG_F(ERROR, "*Newhandle: 0x%08" PRIxPTR ", Newbytes: %d, *Newlock: %d", (intptr_t)newhandle, newbytes, *newlockptr);
        report();
        fatal_exit("BUFFER TOO BIG TO FIT IN CACHE!");
    }

    if (*newlockptr == 0)
    {
        report();
        fatal_exit("ALLOCACHE CALLED WITH LOCK OF 0!");
    }

    //Find best place
    bestval = INT32_MAX; o1 = m_totalSize;
    for (z = m_numBlocks - 1; z >= 0; z--)
    {
        o1 -= m_index[z].leng;
        o2 = o1 + newbytes;
        if (o2 > m_totalSize)
            continue;

        daval = 0;
        for (i = o1, zz = z; i < o2; i += m_index[zz++].leng)
        {
            if (*m_index[zz].lock == 0)
                continue;
            else if (*m_index[zz].lock >= CACHE1D_LOCKED)
            {
                daval = INT32_MAX;
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
            besto   = o1;
            bestz   = z;
            if (bestval == 0)
                break;
        }
    }

    //printf("%ld %ld %ld\n",besto,newbytes,*newlockptr);

    if (bestval == INT32_MAX)
    {
        report();
        fatal_exit("CACHE SPACE ALL LOCKED UP!");
    }

    //Suck things out
    for(sucklen=-newbytes,suckz=bestz;sucklen<0;sucklen+=m_index[suckz++].leng)
        if (*m_index[suckz].lock) *m_index[suckz].hand = 0;

    //Remove all blocks except 1
    suckz -= (bestz+1);
    m_numBlocks -= suckz;
    // Bmemmove(&m_index[bestz], &m_index[bestz + suckz], (m_numBlocks - bestz) * sizeof(cacheindex_t));
    copybufbyte(&m_index[bestz+suckz],&m_index[bestz],(m_numBlocks-bestz)*sizeof(cacheindex_t));
    m_index[bestz].hand = newhandle; *newhandle = m_baseAddress+besto;
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

    for(z=m_numBlocks-1;z>bestz;z--)
        m_index[z] = m_index[z-1];

    m_index[bestz].leng = sucklen;
    m_index[bestz].lock = &zerochar;
}

//void cache1d::suckcache(intptr_t suckptr)
//{
//    int i;
//
//    //Can't exit early, because invalid pointer might be same even though lock = 0
//    for(i=0;i<m_numBlocks;i++)
//        if (*m_index[i].hand == suckptr)
//        {
//            if (*m_index[i].lock) *m_index[i].hand = 0;
//            m_index[i].lock = &zerochar;
//            m_index[i].hand = 0;
//
//            //Combine empty blocks
//            if ((i > 0) && (*m_index[i-1].lock == 0))
//            {
//                m_index[i-1].leng += m_index[i].leng;
//                m_numBlocks--; copybuf(&m_index[i+1],&m_index[i],(m_numBlocks-i)*sizeof(cactype));
//            }
//            else if ((i < m_numBlocks-1) && (*m_index[i+1].lock == 0))
//            {
//                m_index[i+1].leng += m_index[i].leng;
//                m_numBlocks--; copybuf(&m_index[i+1],&m_index[i],(m_numBlocks-i)*sizeof(cactype));
//            }
//        }
//}

void cache1d::ageBlocks(void)
{
    static int agecount;

    if (agecount >= m_numBlocks)
        agecount = m_numBlocks-1;

    if (agecount < 0)
        return;

    int cnt = min(m_maxBlocks >> 4, m_numBlocks-1);

    unsigned char ch;

    while(cnt--)
    {
        // If we have pointer to lock char and it's in [2 .. 199], decrease.
        ch = (*m_index[agecount].lock);
        if (((ch-2)&255) < CACHE1D_UNLOCKED-1)
            (*m_index[agecount].lock) = ch-1;

        if (--agecount < 0) agecount = m_numBlocks-1;
    }
}

void cache1d::report(void)
{
    int32_t usedSize = 0;
    inthashtable_t h_blocktotile = { nullptr, INTHASH_SIZE(m_maxBlocks) };

    inthash_init(&h_blocktotile);

    for (native_t j = 0; j < MAXTILES-1; j++)
    {
        if (waloff[j])
            inthash_add(&h_blocktotile, waloff[j], j, true);

        if (classicht[j].ptr)
            inthash_add(&h_blocktotile, classicht[j].ptr, j, true);

        if (tiletovox[j] != -1)
        {
            for (int i=0; i<MAXVOXMIPS; i++)
                if (voxoff[tiletovox[j]][i])
                    inthash_add(&h_blocktotile, (intptr_t)voxoff[tiletovox[j]][i], j, true);
        }
    }

    LOG_F(INFO, "Block listing:");

    int constexpr reportLineSize = 128;
    auto buf = (char*)Balloca(reportLineSize);

    for (int i = 0; i < m_numBlocks; i++)
    {
        buf[0] = '\0';
        int len = Bsnprintf(buf, reportLineSize, "%4d ", i);

        usedSize += m_index[i].leng;

        if (m_index[i].hand && *m_index[i].hand)
        {
            len += Bsnprintf(buf+len, reportLineSize-len, "@ %x: ", (int32_t)(*m_index[i].hand - m_baseAddress));
        }
        else
        {
            Bstrcat(buf, "FREE");
            LOG_F(INFO, "%s", buf);
            continue;
        }

        len += Bsnprintf(buf+len, reportLineSize-len, "SIZ:%5d ", m_index[i].leng);
        len += Bsnprintf(buf+len, reportLineSize-len, "LCK:%d ", *m_index[i].lock);

        int const tile = inthash_find(&h_blocktotile, *m_index[i].hand);

        if (tile != -1)
        {
            auto typestr = *m_index[i].hand == waloff[tile] ? "ART:%4d " :
                *m_index[i].hand == classicht[tile].ptr ? "HI:%4d " :
                "VOX:%4d "; // needs to be last or else we have to loop through voxoff[tile][]

            len += Bsnprintf(buf + len, reportLineSize - len, typestr, tile);
        }

        if (len < reportLineSize)
            buf[len-1] = '\0';

        LOG_F(INFO, "%s", buf);
    }

    LOG_F(INFO, "Cache size:  %dKB", m_totalSize >> 10);
    LOG_F(INFO, "Used:        %dKB", usedSize >> 10);
    LOG_F(INFO, "Remaining:   %dKB", (m_totalSize - usedSize) >> 10);
    LOG_F(INFO, "Block count: %d/%d",m_numBlocks, m_maxBlocks);

    inthash_free(&h_blocktotile);
}
#else
void cache1d::initBuffer(intptr_t dacachestart, uint32_t dacachesize, uint32_t minsize /*= 0*/)
{
    UNREFERENCED_PARAMETER(dacachestart);
    UNREFERENCED_PARAMETER(dacachesize);
    UNREFERENCED_PARAMETER(minsize);
}

void cache1d::allocateBlock(intptr_t *newhandle, int32_t newbytes, char *newlockptr)
{
    UNREFERENCED_PARAMETER(newlockptr);
    *newhandle = (intptr_t)Xmalloc(newbytes);
}

void cache1d::ageBlocks(void) {}
void cache1d::report(void) {}
void cache1d::reset(void) {}
#endif
