#pragma once

#include "build.h"
#include "reality.h"

extern float rt_viewhorizang;
extern tileinfo_t rt_tileinfo[RT_TILENUM];
extern int32_t rt_tilemap[MAXTILES];

void RT_LoadTiles(void);
