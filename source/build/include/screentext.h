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
    TEXT_LINEWRAP        = 0x00001000,
    TEXT_UPPERCASE       = 0x00002000,
    TEXT_INVERTCASE      = 0x00004000,
    TEXT_IGNOREESCAPE    = 0x00008000,
    TEXT_LITERALESCAPE   = 0x00010000,

    TEXT_CONSTWIDTHNUMS  = 0x00040000,
};

struct ScreenTextSize_t
{
    const char * str;
    vec2_t pos;
    vec2_t empty, between;
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
            const char * str;
            vec2_t pos;
            vec2_t empty, between;
            vec2_t b1, b2;
            int32_t zoom, o, f;
            int16_t font, blockangle;
        };
    };
    int32_t alpha;
    int16_t charangle;
    int8_t shade;
    uint8_t pal;
};

vec2_t screentextGetSize(ScreenTextSize_t const &);
vec2_t screentextRender(ScreenText_t const &);
vec2_t screentextRenderShadow(ScreenText_t const &, vec2_t, int32_t);

typedef int32_t (*getstringtile_t)(int32_t, char *, int32_t);
void screentextSetStringTile(getstringtile_t func);

#ifdef __cplusplus
}
#endif

#endif
