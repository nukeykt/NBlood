// Copyright: 2020 Nuke.YKT, EDuke32 developers
// License: GPLv2

#pragma once

#ifdef USE_OPENGL

#include "vfs.h"
#include "reality_defs.h"
#include "reality_player.h"
#include "reality_render.h"
#include "reality_util.h"

extern buildvfs_kfd rt_group;
extern int rt_gamestate, rt_gamestatus;
extern uint16_t rt_palette[RT_PALNUM][256];
extern rt_vertex_t *rt_sectvtx;
extern rt_walltype *rt_wall;
extern rt_sectortype *rt_sector;

buildvfs_kfd RT_InitGRP(const char *filename);
void RT_Init(void);
int RT_PrepareScript(void);
void RT_LoadBoard(int boardnum);
void RT_Execute(int spriteNum, int playerNum, int playerDist);

#endif
