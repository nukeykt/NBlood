#ifndef HIGHTILE_PRIV_H
#define HIGHTILE_PRIV_H

#include "build.h"
#include "palette.h"
#include "vec.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct { uint8_t r, g, b, a; } coltype;
typedef struct { float r, g, b, a; } coltypef;
EXTERN int32_t globalnoeffect;

struct hicskybox_t {
    char *face[6];
};

typedef struct hicreplc_t {
    struct hicreplc_t *next;
    char *filename;
    struct hicskybox_t *skybox;
    vec2f_t scale;
    float alphacut, specpower, specfactor;
    char palnum;
    uint8_t flags, tfn;
} hicreplctyp;

typedef uint16_t polytintflags_t;

typedef struct {
    polytintflags_t f;
    uint8_t r, g, b;
    uint8_t sr, sg, sb;
} polytint_t;

extern polytint_t hictinting[MAXPALOOKUPS];
extern hicreplctyp *hicreplc[MAXTILES];
extern int32_t hicinitcounter;

typedef struct texcachehead_t
{
    char magic[4];	// 'PMST', was 'Polymost'
    int xdim, ydim;	// of image, unpadded
    int flags;		// 1 = !2^x, 2 = has alpha, 4 = lzw compressed
    int quality;    // r_downsize at the time the cache was written
} texcacheheader;

typedef struct texcachepic_t
{
    int size;
    int format;
    int xdim, ydim;	// of mipmap (possibly padded)
    int border, depth;
} texcachepicture;

hicreplctyp * hicfindsubst(int picnum, int palnum, int nozero = 0);
hicreplctyp * hicfindskybox(int picnum, int palnum, int nozero = 0);
void hictinting_applypixcolor(coltype* tcol, uint8_t pal, bool no_rb_swap);

void hicinit(void);
void hicsetpalettetint(int32_t palnum, char r, char g, char b, char sr, char sg, char sb, polytintflags_t effect);
// flags bitset: 1 = don't compress
int32_t hicsetsubsttex(int32_t picnum, int32_t palnum, const char *filen, float alphacut,
                       float xscale, float yscale, float specpower, float specfactor, char flags);
int32_t hicsetskybox(int32_t picnum, int32_t palnum, char *faces[6], int32_t flags);
int32_t hicclearsubst(int32_t picnum, int32_t palnum);

static inline int have_basepal_tint(void)
{
    polytint_t const & tint = hictinting[MAXPALOOKUPS-1];
    return (tint.r != 255 ||
            tint.g != 255 ||
            tint.b != 255);
}

static inline void hictinting_apply(float *color, int32_t palnum)
{
    polytint_t const & tint = hictinting[palnum];
    color[0] *= (float)tint.r * (1.f/255.f);
    color[1] *= (float)tint.g * (1.f/255.f);
    color[2] *= (float)tint.b * (1.f/255.f);
}

static inline void hictinting_apply_ub(uint8_t *color, int32_t palnum)
{
    polytint_t const & tint = hictinting[palnum];
    color[0] = (uint8_t)(color[0] * (float)tint.r * (1.f/255.f));
    color[1] = (uint8_t)(color[1] * (float)tint.g * (1.f/255.f));
    color[2] = (uint8_t)(color[2] * (float)tint.b * (1.f/255.f));
}

static inline void globaltinting_apply(float *color)
{
    color[0] *= (float)globalr * (1.f/255.f);
    color[1] *= (float)globalg * (1.f/255.f);
    color[2] *= (float)globalb * (1.f/255.f);
}

static inline void globaltinting_apply_ub(uint8_t *color)
{
    color[0] = (uint8_t)(color[0] * (float)globalr * (1.f/255.f));
    color[1] = (uint8_t)(color[1] * (float)globalg * (1.f/255.f));
    color[2] = (uint8_t)(color[2] * (float)globalb * (1.f/255.f));
}

// texcacheheader cachead.flags bits
enum
{
    CACHEAD_NONPOW2 = 1,
    CACHEAD_HASALPHA = 2,
    CACHEAD_COMPRESSED = 4,
    CACHEAD_NODOWNSIZE = 8,
    CACHEAD_HASFULLBRIGHT = 16,
    CACHEAD_NPOTWALL = 32,
};

// hicreplctyp hicr->flags bits
enum
{
    HICR_NOTEXCOMPRESS = 1,
    HICR_FORCEFILTER = 2,

    HICR_NODOWNSIZE = 16,
    HICR_ARTIMMUNITY = 32,
    HICR_INDEXED = 64,
    HICR_NOCHT = 128,
};

// hictinting[].f / gloadtile_hi() and mdloadskin() <effect> arg bits
enum
{
    HICTINT_GRAYSCALE = 1,
    HICTINT_INVERT = 2,
    HICTINT_COLORIZE = 4,
    HICTINT_USEONART = 8,
    HICTINT_APPLYOVERPALSWAP = 16,
    HICTINT_APPLYOVERALTPAL = 32,

    HICTINT_BLEND_MULTIPLY = 0<<6,
    HICTINT_BLEND_SCREEN = 1<<6,
    HICTINT_BLEND_OVERLAY = 2<<6,
    HICTINT_BLEND_HARDLIGHT = 3<<6,

    HICTINT_BLENDMASK = 64|128,

    HICTINT_ALWAYSUSEART = 256,
    HICTINT_NOFOGSHADE = 512,

    HICTINT_PRECOMPUTED = HICTINT_COLORIZE | HICTINT_BLENDMASK,
    HICTINT_IN_MEMORY = HICTINT_PRECOMPUTED | HICTINT_GRAYSCALE | HICTINT_INVERT,

    HICTINT_MEMORY_COMBINATIONS = 1<<5,
};

#define GRAYSCALE_COEFF_RED 0.3
#define GRAYSCALE_COEFF_GREEN 0.59
#define GRAYSCALE_COEFF_BLUE 0.11

static inline int32_t hicfxmask(size_t pal)
{
    return globalnoeffect ? 0 : (hictinting[pal].f & HICTINT_IN_MEMORY);
}
static inline int32_t hicfxid(size_t pal)
{
    return globalnoeffect ? 0 : ((hictinting[pal].f & (HICTINT_GRAYSCALE | HICTINT_INVERT | HICTINT_COLORIZE)) | ((hictinting[pal].f & HICTINT_BLENDMASK) << 3));
}

#ifdef __cplusplus
}
#endif

#endif
