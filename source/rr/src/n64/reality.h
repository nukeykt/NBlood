// Copyright: 2020 Nuke.YKT, EDuke32 developers
// License: GPLv2

#pragma once

#ifdef USE_OPENGL

#include "vfs.h"
#include "reality_defs.h"
#include "reality_player.h"
#include "reality_render.h"
#include "reality_screens.h"
#include "reality_sound.h"
#include "reality_music.h"
#include "reality_util.h"

enum {
    GO_SCRIPTOFFSET = 0,
    GO_ACTOROFFSET,
    GO_BOARDOFFSET,
    GO_QUOTEOFFSET,
    GO_QUOTEDELTA,
    GO_PALETTEOFFSET,
    GO_MUSICCTLOFFSET,
    GO_MUSICTBLOFFSET,
    GO_MUSICSEQOFFSET,
    GO_SOUNDCTLOFFSET,
    GO_SOUNDTBLOFFSET,
    GO_SOUNDVOOFFSET,
    GO_SOUNDPEOFFSET,
    GO_SOUNDPSFFSET,
    GO_SOUNDMOFFSET,
    GO_SOUNDRATEOFFSET,
    GO_TILEINFOOFFSET,
    GO_TILEDATAOFFSET,
    GO_BOSSMDLOFFSET,
    GO_BOSSTRISOFFSET,
    GO_MAXOFFSETS
};

extern buildvfs_kfd rt_group;
// extern int rt_gamestate, rt_gamestatus;
extern uint16_t rt_palette[RT_PALNUM][256];
extern int rt_vtxnum;
extern rt_vertex_t *rt_sectvtx;
extern rt_walltype *rt_wall;
extern rt_sectortype *rt_sector;
extern int rt_boardnum;
extern int rt_levelnum;
extern const char* rt_level_names[];

buildvfs_kfd RT_InitGRP(const char *filename);
void RT_Init(void);
void RT_ROMSeek(int offset);
int RT_ROMRead(void *ptr, int count);
int RT_ROMGetOffset(int offset);
int RT_PrepareScript(void);
void RT_LoadBoard(int boardnum);
void RT_Execute(int spriteNum, int playerNum, int playerDist);
int RT_NextLevel(void);

#endif
