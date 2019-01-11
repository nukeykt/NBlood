#pragma once
#include "common.h"
#include "pragmas.h"
#include "misc.h"

#ifndef APPNAME
#define APPNAME "Blood"
#endif

#ifndef APPBASENAME
#define APPBASENAME "blood"
#endif

#define BYTEVERSION 100

void _SetErrorLoc(const char *pzFile, int nLine);
void _ThrowError(const char *pzFormat, ...);
void __dassert(const char *pzExpr, const char *pzFile, int nLine);

#define ThrowError(...) \
	{ \
		_SetErrorLoc(__FILE__,__LINE__); \
		_ThrowError(__VA_ARGS__); \
	}

#define dassert(x) if (!(x)) __dassert(#x,__FILE__,__LINE__)

#define kMaxSectors 1024
#define kMaxWalls 8192
#define kMaxSprites 4096

#define kMaxTiles 6144
#define kMaxStatus 1024
#define kMaxPlayers 8
#define kMaxXDim 1600
#define kMaxYDim 1200
#define kMaxPaLookups 256
#define kMaxPSkyTiles 256
#define kMaxViewSprites 1024

#define kMaxVoxels 512

// NUKE-TODO:
#define OSDTEXT_DEFAULT   "^00"
#define OSDTEXT_DARKRED   "^00"
#define OSDTEXT_GREEN     "^00"
#define OSDTEXT_RED       "^00"
#define OSDTEXT_YELLOW    "^00"

#define OSDTEXT_BRIGHT    "^S0"

#define OSD_ERROR OSDTEXT_DARKRED OSDTEXT_BRIGHT

#define WASDCONTROLS

extern char UserPath[BMAX_PATH];

int kopen4loadfrommod(const char *fileName, char searchfirst);

#pragma pack(push,1)

struct SECTOR
{
    short wallptr, wallnum;
    long ceilingz, floorz;
    unsigned short ceilingstat, floorstat;
    short ceilingpicnum, ceilingheinum;
    signed char ceilingshade;
    char ceilingpal, ceilingxpanning, ceilingypanning;
    short floorpicnum, floorheinum;
    signed char floorshade;
    char floorpal, floorxpanning, floorypanning;
    char visibility, filler;
    unsigned short lotag;
    short hitag, extra;
};

struct WALL
{
    long x, y;
    short point2, nextwall, nextsector;
    unsigned short cstat;
    short picnum, overpicnum;
    signed char shade;
    char pal, xrepeat, yrepeat, xpanning, ypanning;
    short lotag, hitag, extra;
};

struct SPRITE
{
    long x, y, z;
    short cstat, picnum;
    signed char shade;
    char pal, clipdist, filler;
    unsigned char xrepeat, yrepeat;
    signed char xoffset, yoffset;
    short sectnum, statnum;
    short ang, owner, index, yvel, zvel;
    short type, hitag, extra;
};

struct PICANM {
    unsigned int animframes : 5;
    unsigned int at0_5 : 1;
    unsigned int animtype : 2;
    signed int xoffset : 8;
    signed int yoffset : 8;
    unsigned int animspeed : 4;
    unsigned int at3_4 : 3; // type
    unsigned int at3_7 : 1; // filler
};

struct LOCATION {
    int x, y, z;
    int ang;
};

struct POINT2D {
    int x, y;
};

struct POINT3D {
    int x, y, z;
};

struct VECTOR2D {
    int dx, dy;
};

struct Aim {
    int dx, dy, dz;
};

#pragma pack(pop)

extern SECTOR *qsector;
extern SPRITE *qsprite, *qtsprite;
extern char qsprite_filler[], qsector_filler[];
extern WALL *qwall;
extern PICANM *qpicanm;


inline int IncBy(int a, int b)
{
    a += b;
    int q = a % b;
    a -= q;
    if (q < 0)
        a -= b;
    return a;
}

inline int DecBy(int a, int b)
{
    a--;
    int q = a % b;
    a -= q;
    if (q < 0)
        a -= b;
    return a;
}

inline int ClipLow(int a, int b)
{
    if (a < b)
        return b;
    return a;
}

inline int ClipHigh(int a, int b)
{
    if (a >= b)
        return b;
    return a;
}

inline int ClipRange(int a, int b, int c)
{
    if (a < b)
        return b;
    if (a > c)
        return c;
    return a;
}

inline int interpolate(int a, int b, int c)
{
    return a+mulscale16(b-a,c);
}

inline int interpolateang(int a, int b, int c)
{
    return a+mulscale16(((b-a+1024)&2047)-1024, c);
}

inline char Chance(int a1)
{
    return wrand() < (a1>>1);
}

inline unsigned int Random(int a1)
{
    return mulscale(wrand(), a1, 15);
}

inline int Random2(int a1)
{
    return mulscale(wrand(), a1, 14)-a1;
}

inline int Random3(int a1)
{
    return mulscale(wrand()+wrand(), a1, 15) - a1;
}

inline unsigned int QRandom(int a1)
{
    return mulscale(qrand(), a1, 15);
}

inline int QRandom2(int a1)
{
    return mulscale(qrand(), a1, 14)-a1;
}

inline void SetBitString(char *pArray, int nIndex)
{
    pArray[nIndex>>3] |= 1<<(nIndex&7);
}

inline void ClearBitString(char *pArray, int nIndex)
{
    pArray[nIndex >> 3] &= ~(1 << (nIndex & 7));
}

inline char TestBitString(char *pArray, int nIndex)
{
    return pArray[nIndex>>3] & (1<<(nIndex&7));
}

inline int scale(int a1, int a2, int a3, int a4, int a5)
{
    return a4 + (a5-a4) * (a1-a2) / (a3-a2);
}

inline int mulscale16r(int a, int b)
{
    int64_t acc = 1<<(16-1);
    acc += ((int64_t)a) * b;
    return (int)(acc>>16);
}

inline int mulscale30r(int a, int b)
{
    int64_t acc = 1<<(30-1);
    acc += ((int64_t)a) * b;
    return (int)(acc>>30);
}

inline int dmulscale30r(int a, int b, int c, int d)
{
    int64_t acc = 1<<(30-1);
    acc += ((int64_t)a) * b;
    acc += ((int64_t)c) * d;
    return (int)(acc>>30);
}

inline int approxDist(int dx, int dy)
{
    dx = klabs(dx);
    dy = klabs(dy);
    if (dx > dy)
        dy = (3*dy)>>3;
    else
        dx = (3*dx)>>3;
    return dx+dy;
}

class Rect {
public:
    int x1, y1, x2, y2;
    Rect(int _x1, int _y1, int _x2, int _y2)
    {
        x1 = _x1; y1 = _y1; x2 = _x2; y2 = _y2;
    }
    bool const isValid(void) const
    {
        return x1 < x2 && y1 < y2;
    }
    char const isEmpty(void) const
    {
        return !(x1 < x2 && y1 < y2);
    }
    char const operator!(void) const
    {
        return isEmpty();
    }

    Rect & operator&=(Rect &pOther)
    {
        x1 = ClipLow(x1, pOther.x1);
        y1 = ClipLow(y1, pOther.y1);
        x2 = ClipHigh(x2, pOther.x2);
        y2 = ClipHigh(y2, pOther.y2);
        return *this;
    }
};

