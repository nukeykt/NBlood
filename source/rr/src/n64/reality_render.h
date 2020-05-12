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

