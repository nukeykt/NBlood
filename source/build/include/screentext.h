/*
 * screentext.h
 *
 * Copyright © 2020, Evan Ramos. All rights reserved.
 */

#ifndef SCREENTEXT_H_
#define SCREENTEXT_H_
#pragma once

#include "compat.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline CONSTEXPR_CXX14 size_t utf8len(char const * s)
{
    size_t len = 0;
    char c = '\0';
    while ((c = *s) != '\0')
    {
        len += ((c & 0xC0) != 0x80);
        ++s;
    }
    return len;
}

static inline CONSTEXPR_CXX14 size_t utf8charbytes(char c)
{
    if ((c & 0xF8) == 0xF0)
        return 4;
    else if ((c & 0xF0) == 0xE0)
        return 3;
    else if ((c & 0xE0) == 0xC0)
        return 2;

    return 1;
}

typedef uint16_t ScreenTextGlyph_t;

enum ScreenTextSentinels : ScreenTextGlyph_t
{
    SCREENTEXT_CONTROLCODE = 1u<<15u,

    SCREENTEXT_PALCHANGE   = SCREENTEXT_CONTROLCODE | 1u<<14u,
    SCREENTEXT_CONSTWIDTH  = SCREENTEXT_CONTROLCODE | 1u<<13u,

    SCREENTEXT_NEWLINE     = SCREENTEXT_CONTROLCODE | 1u<<12u,
    SCREENTEXT_SPACE       = SCREENTEXT_CONTROLCODE | 1u<<11u,
    SCREENTEXT_TAB         = SCREENTEXT_CONTROLCODE | 1u<<10u,
};

static inline int screentextGlyphIsControlCode(ScreenTextGlyph_t g)
{
  return (g & SCREENTEXT_CONTROLCODE);
}
static inline ScreenTextGlyph_t screentextGlyphGetTile(ScreenTextGlyph_t g)
{
  return (g & ~SCREENTEXT_CONTROLCODE);
}

static inline int screentextGlyphIsPalChange(ScreenTextGlyph_t g)
{
  return (g & SCREENTEXT_PALCHANGE) == SCREENTEXT_PALCHANGE;
}
static inline uint8_t screentextGlyphGetPalChange(ScreenTextGlyph_t g)
{
  return (g & 0xFFu);
}
static inline int screentextGlyphIsConstWidth(ScreenTextGlyph_t g)
{
  return (g & SCREENTEXT_CONSTWIDTH) == SCREENTEXT_CONSTWIDTH;
}

static inline int screentextGlyphIsNewline(ScreenTextGlyph_t g)
{
  return (g & SCREENTEXT_NEWLINE) == SCREENTEXT_NEWLINE;
}
static inline int screentextGlyphIsSpace(ScreenTextGlyph_t g)
{
  return (g & SCREENTEXT_SPACE) == SCREENTEXT_SPACE;
}
static inline int screentextGlyphIsTab(ScreenTextGlyph_t g)
{
  return (g & SCREENTEXT_TAB) == SCREENTEXT_TAB;
}

enum ScreenTextFlags_t
{
    TEXT_XRIGHT          = 0x00000001,
    TEXT_XCENTER         = 0x00000002,
    TEXT_YBOTTOM         = 0x00000004,
    TEXT_YCENTER         = 0x00000008,

    TEXT_XOFFSETZERO     = 0x00000100,
    TEXT_XJUSTIFY        = 0x00000200,
    TEXT_YOFFSETZERO     = 0x00000400,
    TEXT_YJUSTIFY        = 0x00000800,

    TEXT_VARHEIGHT       = 0x00800000,
};

struct ScreenTextSize_t
{
    ScreenTextGlyph_t const * text;
    uint32_t len;
    vec2_t pos;
    vec2_t empty, between;
    int32_t constwidth;
    vec2_t b1, b2;
    int32_t zoom, o, f;
    int16_t font, blockangle;
};

struct ScreenText_t
{
    union
    {
        ScreenTextSize_t size;
        struct
        {
            ScreenTextGlyph_t const * text;
            uint32_t len;
            vec2_t pos;
            vec2_t empty, between;
            int32_t constwidth;
            vec2_t b1, b2;
            int32_t zoom, o, f;
            int16_t font, blockangle;
        };
    };
    int32_t standardhalfheight;
    int32_t alpha;
    int16_t charangle;
    int8_t shade;
    uint8_t pal;
};

vec2_t screentextGetSize(ScreenTextSize_t const &);
vec2_t screentextRender(ScreenText_t const &);
vec2_t screentextRenderShadow(ScreenText_t const &, vec2_t, int32_t);

struct TileFontPtr_t
{
    void * opaque;
};

static FORCE_INLINE uint32_t tilefontGetChr32FromASCII(char c)
{
    uint32_t chr32 = 0;
    memcpy(&chr32, &c, sizeof(char));
    return chr32;
}

TileFontPtr_t tilefontGetPtr(uint16_t tilenum);
TileFontPtr_t tilefontFind(uint16_t tilenum);
void tilefontDefineMapping(TileFontPtr_t tilefontPtr, uint32_t chr, uint16_t tilenum);
void tilefontMaybeDefineMapping(TileFontPtr_t tilefontPtr, uint32_t chr, uint16_t tilenum);
uint16_t tilefontLookup(TileFontPtr_t tilefontPtr, uint32_t chr);

struct LocalePtr_t
{
    void * opaque;
};

LocalePtr_t localeGetPtr(const char * localeName);
void localeDefineMapping(LocalePtr_t localePtr, const char * key, const char * val);
void localeMaybeDefineMapping(LocalePtr_t localePtr, const char * key, const char * val);
void localeSetCurrent(const char * localeName);
const char * localeLookup(const char * str);

#ifdef __cplusplus
}
#endif

#endif
