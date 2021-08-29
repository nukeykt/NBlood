// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.
//
// This file has been modified from Ken Silverman's original release.

/* Routines for generation of pseudo-random integer sequences. */

#pragma once

#ifndef random_h
#define random_h

#include "compat.h"

#ifdef engine_c_
int32_t randomseed;
#else
extern int32_t randomseed;
#endif

#if !KRANDDEBUG
static FORCE_INLINE int32_t krand(void)
{
    randomseed = (randomseed * 1664525ul) + 221297ul;
    return ((uint32_t) randomseed)>>16;
}
#else
int32_t    krand(void);
#endif

static FORCE_INLINE int32_t seed_krand(int32_t* seed)
{
    *seed = (*seed * 1664525ul) + 221297ul;
    return ((uint32_t)*seed) >> 16;
}

#endif
