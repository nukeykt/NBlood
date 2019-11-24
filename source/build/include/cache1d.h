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
} cacheindex_t;

enum cachelock_t: char
{
    CACHE1D_FREE      = 1,
    CACHE1D_UNLOCKED  = 199,
    CACHE1D_LOCKED    = 200,
    CACHE1D_PERMANENT = 255,
};

class cache1d
{
public:
    void    initBuffer(intptr_t dacachestart, int32_t dacachesize);
    void    allocateBlock(intptr_t *newhandle, int32_t newbytes, char *newlockptr);
    void    ageBlocks();
    int32_t findBlock(int32_t newbytes, int32_t *besto, int32_t *bestz);
    void    report();

    inline int numBlocks() { return m_numBlocks; };
    inline cacheindex_t const * getIndex() { return m_index; }

private:
    void inc_and_check_cacnum();

    cacheindex_t *m_index{};

    intptr_t m_baseAddress{};
    int32_t  m_totalSize{};

    int m_maxBlocks{};
    int m_numBlocks{};
};

extern cache1d g_cache;

#endif // cache1d_h_

