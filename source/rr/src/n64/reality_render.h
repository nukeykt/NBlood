// Copyright 2020 Nuke.YKT, EDuke32 developers
// Polymost code: Copyright Ken Silverman, Copyright (c) 2018, Alex Dawson
#pragma once

#include "build.h"
#include "reality.h"

extern float rt_viewhorizang;
extern tileinfo_t rt_tileinfo[RT_TILENUM];
extern int32_t rt_tilemap[MAXTILES];
extern float rt_sky_color[2][3];

void RT_LoadTiles(void);
void RT_DrawRooms(int x, int y, int z, fix16_t ang, fix16_t horiz, int16_t sectnum, int smoothRatio);
void RT_GLInit(void);

void RT_DisablePolymost();
void RT_EnablePolymost();

void RT_MS_Reset(void);
void RT_MS_Add(int sectnum, int x, int y);
void RT_MS_Update(int sectnum, int ang, int x, int y);
void RT_MS_SetInterpolation(int sectnum);
void RT_AdjustCeilingPanning(int sectnum, int x, int y);
void RT_AdjustFloorPanning(int sectnum, int x, int y);
void RT_RotateSpriteSetColor(int a1, int a2, int a3, int a4);
void RT_RotateSpriteSetShadePal(int ss, int shade, int pal);
void RT_RotateSprite(float x, float y, float sx, float sy, int tilenum, int orientation);
void RT_RotateSpriteText(float x, float y, float sx, float sy, int tilenum, int orientation, bool buildcoords = false);
void RT_DrawTileFlash(int x, int y, int picnum, float sx, float sy, int orientation, int color);
void RT_AddSmoke(int16_t x, int16_t y, int16_t z, uint8_t type);
void RT_AddExplosion(int16_t x, int16_t y, int16_t z, uint8_t type);
void RT_AnimateModels(void);
void RT_LoadBOSS2MDL(void);
