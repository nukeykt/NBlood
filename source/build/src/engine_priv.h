// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.
//
// This file has been modified from Ken Silverman's original release
// by Jonathon Fowler (jf@jonof.id.au)
// by the EDuke32 team (development@voidpoint.com)

#pragma once

#ifndef ENGINE_PRIV_H
#define ENGINE_PRIV_H

#include "build.h"

#define MAXPERMS 512
#define MAXARTFILES_BASE 200
#define MAXARTFILES_TOTAL 220
#define MAXCLIPDIST 1024

// Uncomment to clear the screen before each top-level draw (classic only).
// FIXME: doesn't work with mirrors.
//#define ENGINE_CLEAR_SCREEN

#ifdef YAX_ENABLE
# define YAX_MAXDRAWS 8
#endif

#if !defined(NOASM) && defined __cplusplus
extern "C" {
#endif
    extern intptr_t asm1, asm2, asm3, asm4;
    extern int32_t globalx1, globaly2;
#if !defined(NOASM) && defined __cplusplus
}
#endif

#if !defined __cplusplus
# error This header can only be used as C++.
#endif

extern "C" {

extern uint16_t ATTRIBUTE((used)) sqrtable[4096], ATTRIBUTE((used)) shlookup[4096+256], ATTRIBUTE((used)) sqrtable_old[2048];

#if defined(_MSC_VER) && !defined(NOASM)

    //
    // Microsoft C Inline Assembly Routines
    //

    static inline int32_t nsqrtasm(int32_t a)
    {
        _asm
        {
            push ebx
            mov eax, a
            test eax, 0xff000000
            mov ebx, eax
            jnz short over24
            shr ebx, 12
            mov cx, word ptr shlookup[ebx*2]
            jmp short under24
            over24 :
            shr ebx, 24
                mov cx, word ptr shlookup[ebx*2+8192]
                under24 :
                shr eax, cl
                mov cl, ch
                mov ax, word ptr sqrtable[eax*2]
                shr eax, cl
                pop ebx
        }
    }

    static inline int32_t getclipmask(int32_t a, int32_t b, int32_t c, int32_t d)
    {
        _asm
        {
            push ebx
            mov eax, a
            mov ebx, b
            mov ecx, c
            mov edx, d
            sar eax, 31
            add ebx, ebx
            adc eax, eax
            add ecx, ecx
            adc eax, eax
            add edx, edx
            adc eax, eax
            mov ebx, eax
            shl ebx, 4
            or al, 0xf0
            xor eax, ebx
            pop ebx
        }
    }

#elif defined(__GNUC__) && defined(__i386__) && !defined(NOASM)	// _MSC_VER

    //
    // GCC "Inline" Assembly Routines
    //

#define nsqrtasm(a) \
    ({ int32_t __r, __a=(a); \
       __asm__ __volatile__ ( \
        "testl $0xff000000, %%eax\n\t" \
        "movl %%eax, %%ebx\n\t" \
        "jnz 0f\n\t" \
        "shrl $12, %%ebx\n\t" \
        "movw " ASMSYM("shlookup") "(,%%ebx,2), %%cx\n\t" \
        "jmp 1f\n\t" \
        "0:\n\t" \
        "shrl $24, %%ebx\n\t" \
        "movw (" ASMSYM("shlookup") "+8192)(,%%ebx,2), %%cx\n\t" \
        "1:\n\t" \
        "shrl %%cl, %%eax\n\t" \
        "movb %%ch, %%cl\n\t" \
        "movw " ASMSYM("sqrtable") "(,%%eax,2), %%ax\n\t" \
        "shrl %%cl, %%eax" \
        : "=a" (__r) : "a" (__a) : "ebx", "ecx", "cc"); \
     __r; })

#define getclipmask(a,b,c,d) \
    ({ int32_t __a=(a), __b=(b), __c=(c), __d=(d); \
       __asm__ __volatile__ ("sarl $31, %%eax; addl %%ebx, %%ebx; adcl %%eax, %%eax; " \
                "addl %%ecx, %%ecx; adcl %%eax, %%eax; addl %%edx, %%edx; " \
                "adcl %%eax, %%eax; movl %%eax, %%ebx; shl $4, %%ebx; " \
                "orb $0xf0, %%al; xorl %%ebx, %%eax" \
        : "=a" (__a), "=b" (__b), "=c" (__c), "=d" (__d) \
        : "a" (__a), "b" (__b), "c" (__c), "d" (__d) : "cc"); \
     __a; })

#else   // __GNUC__ && __i386__

    static FORCE_INLINE int32_t nsqrtasm(uint32_t a)
    {
        // JBF 20030901: This was a damn lot simpler to reverse engineer than
        // msqrtasm was. Really, it was just like simplifying an algebra equation.
        uint16_t const c = shlookup[(a & 0xff000000) ? ((a >> 24) + 4096):(a>>12)];
        a >>= c&0xff;
        return ((a&0xffff0000)|(sqrtable[a])) >> ((c&0xff00) >> 8);
    }

    static FORCE_INLINE int32_t getclipmask(int32_t a, int32_t b, int32_t c, int32_t d)
    {
        // Ken did this
        d = ((a<0)<<3) + ((b<0)<<2) + ((c<0)<<1) + (d<0);
        return (((d<<4)^0xf0)|d);
    }

#endif


static FORCE_INLINE int32_t ksqrt_inline(uint32_t n)
{
    if (enginecompatibilitymode == ENGINE_19950829)
        return ksqrtasm_old(n);
    return nsqrtasm(n);
}

extern int16_t thesector[MAXWALLSB], thewall[MAXWALLSB];
extern int16_t bunchfirst[MAXWALLSB], bunchlast[MAXWALLSB];
extern int16_t maskwall[MAXWALLSB], maskwallcnt;
extern tspriteptr_t tspriteptr[MAXSPRITESONSCREEN + 1];
extern uint8_t* mirrorBuffer;
extern int32_t xdimen, xdimenrecip, halfxdimen, xdimenscale, xdimscale, ydimen;
extern float fxdimen;
extern intptr_t frameoffset;
extern int32_t globalposx, globalposy, globalposz, globalhoriz;
extern fix16_t qglobalhoriz, qglobalang;
extern float fglobalposx, fglobalposy, fglobalposz;
extern int16_t globalang, globalcursectnum;
extern int32_t globalpal, cosglobalang, singlobalang;
extern int32_t cosviewingrangeglobalang, sinviewingrangeglobalang;
extern int32_t globalhisibility, globalpisibility, globalcisibility;
#ifdef USE_OPENGL
extern int32_t globvis2, globalvisibility2, globalhisibility2, globalpisibility2, globalcisibility2;
#endif
extern int32_t globvis, globalvisibility;
extern int32_t xyaspect;
extern int32_t globalshade;
extern int32_t globalclipdist;
extern int16_t globalpicnum;

extern int32_t globalorientation;

extern int16_t editstatus;

extern int16_t searchit;
extern int32_t searchx, searchy;
extern int16_t searchsector, searchwall, searchstat;
extern int16_t searchbottomwall, searchisbottom;

extern char inpreparemirror;

extern char picsiz[MAXTILES];
extern int16_t sectorborder[256];
extern int32_t qsetmode;
extern int32_t hitallsprites;

extern int32_t xb1[MAXWALLSB];
extern int32_t rx1[MAXWALLSB], ry1[MAXWALLSB];
extern int16_t bunchp2[MAXWALLSB];
extern int16_t numscans, numbunches;
extern int32_t rxi[8], ryi[8];

extern int16_t wallsect[MAXWALLS];

#ifdef USE_OPENGL

// For GL_EXP2 fog:
#define FOGSCALE 0.0000768f

void calc_and_apply_fog(int32_t shade, int32_t vis, int32_t pal);
void calc_and_apply_fog_factor(int32_t shade, int32_t vis, int32_t pal, float factor);
#endif


// int32_t wallmost(int16_t *mostbuf, int32_t w, int32_t sectnum, char dastat);
int32_t wallfront(int32_t l1, int32_t l2);

void set_globalang(fix16_t const ang);

int32_t animateoffs(int tilenum, int fakevar);

static FORCE_INLINE int32_t bad_tspr(tspriteptr_t tspr)
{
    // NOTE: tspr->owner >= MAXSPRITES (could be model) has to be handled by
    // caller.
    return (tspr->owner < 0 || (unsigned)tspr->picnum >= MAXTILES);
}

//
// getpalookup (internal)
//
static FORCE_INLINE int32_t getpalookup(int32_t davis, int32_t dashade)
{
    if (getpalookup_replace)
        return getpalookup_replace(davis, dashade);
    return min(max(dashade + (davis >> 8), 0), numshades - 1);
}

static FORCE_INLINE int32_t getpalookupsh(int32_t davis) { return getpalookup(davis, globalshade) << 8; }

void dorotspr_handle_bit2(int32_t *sx, int32_t *sy, int32_t *z, int32_t dastat,
                          int32_t cx1_plus_cx2, int32_t cy1_plus_cy2,
                          int32_t *ret_yxaspect, int32_t *ret_xyaspect);

////// yax'y stuff //////
#ifdef USE_OPENGL
extern void polymost_scansector(int32_t sectnum);
#endif
int32_t renderAddTsprite(int16_t z, int16_t sectnum);
#ifdef YAX_ENABLE
extern int32_t g_nodraw, scansector_retfast, scansector_collectsprites;
extern int32_t yax_globallev, yax_globalbunch;
extern int32_t yax_globalcf, yax_nomaskpass, yax_nomaskdidit;
extern uint8_t haveymost[bitmap_size(YAX_MAXBUNCHES)];
extern uint8_t yax_gotsector[bitmap_size(MAXSECTORS)];
extern int32_t yax_polymostclearzbuffer;

static FORCE_INLINE int32_t yax_isislandwall(int32_t line, int32_t cf) { return (yax_vnextsec(line, cf) >= 0); }
#endif

#ifdef YAX_DEBUG
extern char m32_debugstr[64][128];
extern int32_t m32_numdebuglines;
# define yaxdebug(fmt, ...)  do { if (m32_numdebuglines<64) Bsnprintf(m32_debugstr[m32_numdebuglines++], 128, fmt, ##__VA_ARGS__); } while (0)
# define yaxprintf(fmt, ...) do { LOG_F(INFO, fmt, ##__VA_ARGS__); } while (0)
#else
# define yaxdebug(fmt, ...)
# define yaxprintf(fmt, ...)
#endif



#if defined(_MSC_VER) && !defined(NOASM)

static inline void setgotpic(int32_t a)
{
    _asm {
        push ebx
        mov eax, a
        mov ebx, eax
        cmp byte ptr walock[eax], 200
        jae skipit
        mov byte ptr walock[eax], 199
skipit:
        shr eax, 3
        and ebx, 7
        mov dl, byte ptr gotpic[eax]
        mov bl, byte ptr pow2char[ebx]
        or dl, bl
        mov byte ptr gotpic[eax], dl
        pop ebx
    }
}

#elif defined(__GNUC__) && defined(__i386__) && !defined(NOASM)	// _MSC_VER

#define setgotpic(a) \
({ int32_t __a=(a); \
    __asm__ __volatile__ ( \
                   "movl %%eax, %%ebx\n\t" \
                   "cmpb $200, " ASMSYM("walock") "(%%eax)\n\t" \
                   "jae 0f\n\t" \
                   "movb $199, " ASMSYM("walock") "(%%eax)\n\t" \
                   "0:\n\t" \
                   "shrl $3, %%eax\n\t" \
                   "andl $7, %%ebx\n\t" \
                   "movb " ASMSYM("gotpic") "(%%eax), %%dl\n\t" \
                   "movb " ASMSYM("pow2char_") "(%%ebx), %%bl\n\t" \
                   "orb %%bl, %%dl\n\t" \
                   "movb %%dl, " ASMSYM("gotpic") "(%%eax)" \
                   : "=a" (__a) : "a" (__a) \
                   : "ebx", "edx", "memory", "cc"); \
                       __a; })

#else	// __GNUC__ && __i386__

static FORCE_INLINE void setgotpic(int32_t tilenume)
{
    if (walock[tilenume] < CACHE1D_LOCKED) walock[tilenume] = CACHE1D_UNLOCKED;
    bitmap_set(gotpic, tilenume);
}

#endif

// Get properties of parallaxed sky to draw.
// Returns: pointer to tile offset array. Sets-by-pointer the other three.
static FORCE_INLINE const int8_t *getpsky(int32_t picnum, int32_t *dapyscale, int32_t *dapskybits, int32_t *dapyoffs, int32_t *daptileyscale)
{
    psky_t const * const psky = &multipsky[getpskyidx(picnum)];

    if (dapskybits)
        *dapskybits = (pskybits_override == -1 ? psky->lognumtiles : pskybits_override);
    if (dapyscale)
        *dapyscale = (parallaxyscale_override == 0 ? psky->horizfrac : parallaxyscale_override);
    if (dapyoffs)
        *dapyoffs = psky->yoffs + parallaxyoffs_override;
    if (daptileyscale)
        *daptileyscale = psky->yscale;

    return psky->tileofs;
}

static FORCE_INLINE void set_globalpos(int32_t const x, int32_t const y, int32_t const z)
{
    globalposx = x, fglobalposx = (float)x;
    globalposy = y, fglobalposy = (float)y;
    globalposz = z, fglobalposz = (float)z;
}

}  // extern "C"

template <typename T> static FORCE_INLINE void tileUpdatePicnum(T * const tileptr, int const obj)
{
    auto &tile = *tileptr;

    if (picanm[tile].sf & PICANM_ANIMTYPE_MASK)
        tile += animateoffs(tile, obj);

    if (((obj & 16384) == 16384) && (globalorientation & CSTAT_WALL_ROTATE_90) && rottile[tile].newtile != -1)
        tile = rottile[tile].newtile;
}

struct wallsprite_dims
{
    int32_t dax, day;  // {sin,-cos}(angle) * xrepeat
    int32_t tilesizx;  // tilesiz[].x;
    int32_t xoff;      // (tile + sprite) xoffset
};

static inline wallsprite_dims get_wallspr_dims(uspriteptr_t spr)
{
    const int32_t tilenum = spr->picnum, ang = spr->ang;
    const int32_t xrepeat = spr->xrepeat;
    int32_t xoff = picanm[tilenum].xofs + spr->xoffset;

    if (spr->cstat&4)
        xoff = -xoff;

    int32_t const dax = sintable[ang&2047]*xrepeat;
    int32_t const day = sintable[(ang+1536)&2047]*xrepeat;

    return {dax, day, tilesiz[tilenum].x, xoff};
}

static inline vec2_t get_wallspr_center(void const *const ptr)
{
    auto const *spr = (uspriteptr_t)ptr;
    Bassert((spr->cstat & CSTAT_SPRITE_ALIGNMENT) == CSTAT_SPRITE_ALIGNMENT_WALL);

    auto const    dims = get_wallspr_dims(spr);
    int32_t const k    = dims.xoff;

    return { spr->x - mulscale16(dims.dax, k), spr->y - mulscale16(dims.day, k) };
}

// x1, y1: in/out
// rest x/y: out
static inline void get_wallspr_points(void const * const ptr, int32_t *x1, int32_t *x2, int32_t *y1, int32_t *y2)
{
    //These lines get the 2 points of the rotated sprite
    //Given: (x1, y1) starts out as the center point
    auto const dims = get_wallspr_dims((uspriteptr_t)ptr);

    int32_t const dax = dims.dax;
    int32_t const day = dims.day;
    int32_t const l = dims.tilesizx;
    int32_t const k = (l>>1) + dims.xoff;

    *x1 -= mulscale16(dax,k);
    *x2 = *x1 + mulscale16(dax,l);

    *y1 -= mulscale16(day,k);
    *y2 = *y1 + mulscale16(day,l);
}

struct floorsprite_dims
{
    int32_t cosang, sinang;
    vec2_t span, repeat, adjofs;
};

static inline floorsprite_dims get_floorspr_dims(uspriteptr_t spr, bool sloped)
{
    const int32_t tilenum = spr->picnum;
    const int32_t cosang = sintable[(spr->ang+512)&2047];
    const int32_t sinang = sintable[spr->ang&2047];

    vec2_t const span   = { tilesiz[tilenum].x, tilesiz[tilenum].y };
    vec2_t const repeat = { spr->xrepeat, spr->yrepeat };

    vec2_t adjofs = { picanm[tilenum].xofs, picanm[tilenum].yofs };

    if (!sloped)
    {
        // For sloped sprites, '[xy]offset' encode the slope.
        adjofs.x += spr->xoffset;
        adjofs.y += spr->yoffset;
    }

    if (spr->cstat & 4)
        adjofs.x = -adjofs.x;

    if (spr->cstat & 8)
        adjofs.y = -adjofs.y;

    return {cosang, sinang, span, repeat, adjofs};
}

static inline vec2_t get_floorspr_center(void const *const ptr, bool sloped)
{
    auto const *spr = (uspriteptr_t)ptr;
    Bassert((spr->cstat & CSTAT_SPRITE_ALIGNMENT) == (sloped ? CSTAT_SPRITE_ALIGNMENT_SLOPE : CSTAT_SPRITE_ALIGNMENT_FLOOR));

    auto const    dims   = get_floorspr_dims(spr, sloped);
    int32_t const cosang = dims.cosang, sinang = dims.sinang;
    vec2_t const  center = dims.adjofs * dims.repeat;

    int32_t const dx = dmulscale16(sinang, center.x, cosang, center.y);
    int32_t const dy = dmulscale16(sinang, center.y, -cosang, center.x);

    return { spr->x + dx, spr->y + dy };
}

// x1, y1: in/out
// rest x/y: out
static inline void get_floorspr_points(void const * const ptr, int32_t px, int32_t py,
                                int32_t *x1, int32_t *x2, int32_t *x3, int32_t *x4,
                                int32_t *y1, int32_t *y2, int32_t *y3, int32_t *y4,
                                int32_t const heinum)
{
    auto const    dims   = get_floorspr_dims((uspriteptr_t)ptr, heinum != 0);
    int32_t const cosang = dims.cosang, sinang = dims.sinang;
    vec2_t const  span = dims.span, repeat = dims.repeat, adjofs = dims.adjofs;

    int32_t const ratio = nsqrtasm(heinum*heinum+16777216);

    vec2_t const center = { ((span.x >> 1) + adjofs.x) * repeat.x, ((span.y >> 1) + adjofs.y) * repeat.y };
    vec2_t const rspan  = { span.x * repeat.x, span.y * repeat.y };
    vec2_t const ofs    = { -divscale12(mulscale16(cosang, rspan.y), ratio), -divscale12(mulscale16(sinang, rspan.y), ratio) };
    vec2_t const cossinslope = { divscale12(cosang, ratio), divscale12(sinang, ratio) };

    *x1 += dmulscale16(sinang, center.x, cossinslope.x, center.y) - px;
    *y1 += dmulscale16(cossinslope.y, center.y, -cosang, center.x) - py;

    *x2 = *x1 - mulscale16(sinang, rspan.x);
    *y2 = *y1 + mulscale16(cosang, rspan.x);

    *x3 = *x2 + ofs.x, *x4 = *x1 + ofs.x;
    *y3 = *y2 + ofs.y, *y4 = *y1 + ofs.y;
}

void getclosestpointonline(vec2_t const p, vec2_t w, vec2_t w2, vec2_t* const closest);

#endif	/* ENGINE_PRIV_H */
