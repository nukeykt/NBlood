//-------------------------------------------------------------------------
/*
Copyright (C) 2016 EDuke32 developers and contributors

This file is part of EDuke32.

EDuke32 is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
//-------------------------------------------------------------------------

#pragma once

#include "screentext.h"
#include "menus.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAXUSERQUOTES 6

extern int32_t user_quote_time[MAXUSERQUOTES];
extern int32_t minitext_lowercase;
extern int32_t minitext_yofs;

void G_InitText(void);

enum {
    TEXT_INTERNALSPACE   = 0x00000010,
    TEXT_TILESPACE       = 0x00000020,
    TEXT_INTERNALLINE    = 0x00000040,
    TEXT_TILELINE        = 0x00000080,

    TEXT_UPPERCASE       = 0x00002000,
    TEXT_INVERTCASE      = 0x00004000,
    TEXT_IGNOREESCAPE    = 0x00008000,
    TEXT_LITERALESCAPE   = 0x00010000,

    TEXT_CONSTWIDTHNUMS  = 0x00040000,
    TEXT_DIGITALNUMBER   = 0x00080000,
    TEXT_BIGALPHANUM     = 0x00100000,
    TEXT_GRAYFONT        = 0x00200000,
};

extern int32_t minitext_(int32_t x, int32_t y, const char *t, int32_t s, int32_t p, int32_t sb);
extern void menutext_(int32_t x, int32_t y, int32_t s, char const *t, int32_t o, int32_t f);
extern void captionmenutext(int32_t x, int32_t y, char const *t);
extern vec2_t gametext_(int32_t x, int32_t y, const char *t, int32_t s, int32_t p, int32_t o, int32_t a, int32_t f);
extern void gametext_simple(int32_t x, int32_t y, const char *t);
#define mpgametext_x (5<<16)
extern vec2_t mpgametext(int32_t x, int32_t y, char const * t, int32_t s, int32_t o, int32_t a, int32_t f);
extern vec2_t mpgametextsize(char const * t, int32_t f);
extern int32_t textsc(int32_t sc);

#define minitextshade(x, y, t, s, p, sb) minitext_(x,y,t,s,p,sb)
#define minitext(x, y, t, p, sb) minitext_(x,y,t,0,p,sb)
#define menutext(x, y, t) menutext_((x), (y), 0, (t), 10|16, 0)
#define menutext_centeralign(x, y, t) menutext_((x), (y), 0, (t), 10|16, TEXT_XCENTER|TEXT_YCENTER)
#define menutext_center(y, t) menutext_(160<<16, (y)<<16, 0, (t), 10|16, TEXT_XCENTER)
#define gametext(x, y, t) gametext_simple((x)<<16, (y)<<16, (t))
#define gametext_widenumber(x, y, t) gametext_((x)<<16, (y)<<16, (t), 0, MF_Bluefont.pal, 1024, 0, TEXT_CONSTWIDTHNUMS)
#define gametext_number(x, y, t) gametext_((x)<<16, (y)<<16, (t), 0, MF_Bluefont.pal, 0, 0, TEXT_CONSTWIDTHNUMS)
#define gametext_pal(x, y, t, p) gametext_((x)<<16, (y)<<16, (t), 0, (p), 0, 0, 0)
#define gametext_center(y, t) gametext_(160<<16, (y)<<16, (t), 0, MF_Bluefont.pal, 0, 0, TEXT_XCENTER)
#define gametext_center_number(y, t) gametext_(160<<16, (y)<<16, (t), 0, MF_Bluefont.pal, 0, 0, TEXT_XCENTER|TEXT_CONSTWIDTHNUMS)
#define gametext_center_shade(y, t, s) gametext_(160<<16, (y)<<16, (t), (s), MF_Bluefont.pal, 0, 0, TEXT_XCENTER)
#define gametext_center_shade_pal(y, t, s, p) gametext_(160<<16, (y)<<16, (t), (s), (p), 0, 0, TEXT_XCENTER)

extern void G_PrintGameText(int32_t tile, int32_t x, int32_t y, const char *t,
                            int32_t s, int32_t p, int32_t o,
                            int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                            int32_t z, int32_t a);

extern int32_t G_GetStringTileASCII(TileFontPtr_t tilefontPtr, int32_t font, char c, int32_t f);
extern void G_SetScreenTextEmpty(vec2_t & empty, int32_t font, int32_t f);

uint32_t G_ScreenTextFromString(ScreenTextGlyph_t * text, char const * str, char const * const end, TileFontPtr_t tilefontPtr, int32_t font, int32_t flags);

static inline int32_t PopulateConstWidth(TileFontPtr_t tilefontPtr, int32_t font, int32_t flags)
{
    char numeral = '0'; // this is subject to change as an implementation detail
    uint16_t const tile = G_GetStringTileASCII(tilefontPtr, font, numeral, flags);
    Bassert(tile < MAXTILES);
    return (tilesiz[tile].x - 1) << 16;
}

static inline vec2_t G_ScreenTextSize(const int32_t font,
    int32_t x, int32_t y, const int32_t zoom, const int32_t blockangle,
    const char * str, const int32_t o,
    int32_t xspace, int32_t yline, int32_t xbetween, int32_t ybetween,
    const int32_t f, int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    if (str == nullptr || (unsigned)font >= MAXTILES)
    {
        debug_break();
        return {};
    }

    TileFontPtr_t tilefontPtr = tilefontFind(font);


    ScreenTextSize_t data{};

    str = localeLookup(str);

    data.constwidth = PopulateConstWidth(tilefontPtr, font, f);

    size_t const strbuflen = strlen(str);
    size_t const textbufcount = ((f & TEXT_CONSTWIDTHNUMS) ? strbuflen << 1 : strbuflen) + 1;
    auto text = (ScreenTextGlyph_t *)Balloca(sizeof(ScreenTextGlyph_t) * textbufcount);
    uint32_t const textlen = G_ScreenTextFromString(text, str, str + strbuflen, tilefontPtr, font, f);
    data.text = text;
    data.len = textlen;

    data.pos = {x, y};
    data.empty = {xspace, yline};
    data.between = {xbetween, ybetween};
    data.b1 = {x1, y1};
    data.b2 = {x2, y2};
    data.zoom = zoom;
    data.o = o;
    data.f = f;
    data.font = font;
    data.blockangle = blockangle;

    return screentextGetSize(data);
}

static inline vec2_t G_ScreenText(const int32_t font,
    int32_t x, int32_t y, const int32_t zoom, const int32_t blockangle, const int32_t charangle,
    const char * str, const int32_t shade, uint32_t pal, int32_t o, int32_t alpha,
    int32_t xspace, int32_t yline, int32_t xbetween, int32_t ybetween, const int32_t f,
    const int32_t x1, const int32_t y1, const int32_t x2, const int32_t y2)
{
    if (str == nullptr || (unsigned)font >= MAXTILES)
    {
        debug_break();
        return {};
    }

    TileFontPtr_t tilefontPtr = tilefontFind(font);

    ScreenText_t data{};

    str = localeLookup(str);

    data.constwidth = PopulateConstWidth(tilefontPtr, font, f);

    size_t const strbuflen = strlen(str);
    size_t const textbufcount = ((f & TEXT_CONSTWIDTHNUMS) ? strbuflen << 1 : strbuflen) + 1;
    auto text = (ScreenTextGlyph_t *)Balloca(sizeof(ScreenTextGlyph_t) * textbufcount);
    uint32_t const textlen = G_ScreenTextFromString(text, str, str + strbuflen, tilefontPtr, font, f);
    data.text = text;
    data.len = textlen;

    data.pos = {x, y};
    data.empty = {xspace, yline};
    data.between = {xbetween, ybetween};
    data.b1 = {x1, y1};
    data.b2 = {x2, y2};
    data.zoom = zoom;
    data.o = o;
    data.f = f;
    data.font = font;
    data.blockangle = blockangle;
    data.standardhalfheight = (tilesiz[font].y>>1)<<16;
    data.alpha = alpha;
    data.charangle = charangle;
    data.shade = shade;
    data.pal = pal;

    return screentextRender(data);
}

static inline vec2_t G_ScreenTextShadow(int32_t sx, int32_t sy, int32_t sp, const int32_t font,
    int32_t x, int32_t y, const int32_t zoom, const int32_t blockangle, const int32_t charangle,
    const char * str, const int32_t shade, uint32_t pal, int32_t o, const int32_t alpha,
    int32_t xspace, int32_t yline, int32_t xbetween, int32_t ybetween, const int32_t f,
    const int32_t x1, const int32_t y1, const int32_t x2, const int32_t y2)
{
    if (str == nullptr || (unsigned)font >= MAXTILES)
    {
        debug_break();
        return {};
    }

    TileFontPtr_t tilefontPtr = tilefontFind(font);

    ScreenText_t data{};

    Bassert(!(f & TEXT_CONSTWIDTHNUMS));

    str = localeLookup(str);

    size_t const strbuflen = strlen(str);
    size_t const textbufcount = strbuflen + 1;
    auto text = (ScreenTextGlyph_t *)Balloca(sizeof(ScreenTextGlyph_t) * textbufcount);
    uint32_t const textlen = G_ScreenTextFromString(text, str, str + strbuflen, tilefontPtr, font, f);
    data.text = text;
    data.len = textlen;

    data.pos = {x, y};
    data.empty = {xspace, yline};
    data.between = {xbetween, ybetween};
    data.b1 = {x1, y1};
    data.b2 = {x2, y2};
    data.zoom = zoom;
    data.o = o;
    data.f = f;
    data.font = font;
    data.blockangle = blockangle;
    data.standardhalfheight = (tilesiz[font].y>>1)<<16;
    data.alpha = alpha;
    data.charangle = charangle;
    data.shade = shade;
    data.pal = pal;

    return screentextRenderShadow(data, {sx, sy}, sp);
}

#ifdef __cplusplus
}
#endif
