// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.
//
// This file has been modified from Ken Silverman's original release
// by Jonathon Fowler (jf@jonof.id.au)
// by the EDuke32 team (development@voidpoint.com)

#ifndef cache1d_h_
#define cache1d_h_

#include <inttypes.h>

#define MINCACHEINDEXSIZE 1024
#define MINCACHEBLOCKSIZE 16

typedef struct
{
    char *    lock;
    intptr_t *hand;
    int32_t   leng;
    int32_t   ovh;
} cacheindex_t;

enum cachelock_t : char
{
    CACHE1D_FREE      = 1,
    CACHE1D_UNLOCKED  = 199,
    CACHE1D_LOCKED    = 200,
    CACHE1D_PERMANENT = 255,
};

class cache1d
{
public:
    void    initBuffer(intptr_t dacachestart, uint32_t dacachesize, uint32_t minsize = 0);
    void    allocateBlock(intptr_t* newhandle, int32_t newbytes, char* newlockptr);

    void    ageBlocks(void);
    void    report(void);
    void    reset(void);

    int numBlocks(void) { return m_numBlocks; }
    cacheindex_t const * getIndex(void) { return m_index; }

private:
    void inc_and_check_cacnum(void);

    cacheindex_t *m_index{};

    intptr_t m_baseAddress{};
    int32_t  m_totalSize{};

    int m_maxBlocks{};
    int m_numBlocks{};
};

extern cache1d g_cache;

#endif // cache1d_h_
