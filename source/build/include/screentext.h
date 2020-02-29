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

enum ScreenTextFlags_t {
    TEXT_XRIGHT          = 0x00000001,
    TEXT_XCENTER         = 0x00000002,
    TEXT_YBOTTOM         = 0x00000004,
    TEXT_YCENTER         = 0x00000008,
    TEXT_INTERNALSPACE   = 0x00000010,
    TEXT_TILESPACE       = 0x00000020,
    TEXT_INTERNALLINE    = 0x00000040,
    TEXT_TILELINE        = 0x00000080,
    TEXT_XOFFSETZERO     = 0x00000100,
    TEXT_XJUSTIFY        = 0x00000200,
    TEXT_YOFFSETZERO     = 0x00000400,
    TEXT_YJUSTIFY        = 0x00000800,
    TEXT_LINEWRAP        = 0x00001000,
    TEXT_UPPERCASE       = 0x00002000,
    TEXT_INVERTCASE      = 0x00004000,
    TEXT_IGNOREESCAPE    = 0x00008000,
    TEXT_LITERALESCAPE   = 0x00010000,
    TEXT_BACKWARDS       = 0x00020000,
    TEXT_CONSTWIDTHNUMS  = 0x00040000,
};

typedef int32_t (*getstringtile_t)(int32_t, char *, int32_t);
void screentextSetStringTile(getstringtile_t func);

extern int32_t G_GetStringLineLength(const char *text, const char *end, int32_t iter);
extern int32_t G_GetStringNumLines(const char *text, const char *end, int32_t iter);
extern char* G_GetSubString(const char *text, const char *end, int32_t iter, int32_t length);
extern vec2_t G_ScreenTextSize(int32_t font, int32_t x, int32_t y, int32_t z, int32_t blockangle, const char *str, int32_t o, int32_t xspace, int32_t yline, int32_t xbetween, int32_t ybetween, int32_t f, int32_t x1, int32_t y1, int32_t x2, int32_t y2);
extern void G_AddCoordsFromRotation(vec2_t *coords, const vec2_t *unitDirection, int32_t magnitude);
extern vec2_t G_ScreenText(int32_t font, int32_t x, int32_t y, int32_t z, int32_t blockangle, int32_t charangle, const char *str, int32_t shade, int32_t pal, int32_t o, int32_t alpha, int32_t xspace, int32_t yline, int32_t xbetween, int32_t ybetween, int32_t f, int32_t x1, int32_t y1, int32_t x2, int32_t y2);
extern vec2_t G_ScreenTextShadow(int32_t sx, int32_t sy, int32_t sp, int32_t font, int32_t x, int32_t y, int32_t z, int32_t blockangle, int32_t charangle, const char *str, int32_t shade, int32_t pal, int32_t o, int32_t alpha, int32_t xspace, int32_t yline, int32_t xbetween, int32_t ybetween, int32_t f, int32_t x1, int32_t y1, int32_t x2, int32_t y2);

#ifdef __cplusplus
}
#endif

#endif
