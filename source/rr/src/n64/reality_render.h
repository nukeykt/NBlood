// Copyright 2020 Nuke.YKT, EDuke32 developers
// Polymost code: Copyright Ken Silverman, Copyright (c) 2018, Alex Dawson
#pragma once

#include "build.h"
#include "reality.h"

extern float rt_viewhorizang;
extern tileinfo_t rt_tileinfo[RT_TILENUM];
extern int32_t rt_tilemap[MAXTILES];
extern vec3f_t rt_sky_color[2];

#define MOVESECTNUM 40
#define MOVESECTVTXNUM 1024

extern int ms_list[MOVESECTNUM], ms_listvtxptr[MOVESECTNUM];
extern int ms_list_cnt, ms_vtx_cnt;
extern int ms_dx[MOVESECTVTXNUM], ms_dy[MOVESECTVTXNUM];
extern int ms_vx[MOVESECTVTXNUM], ms_vy[MOVESECTVTXNUM];

#define MAXEXPLOSIONS 16

struct explosioninstance_t {
    uint8_t status;
    uint8_t type;
    uint8_t c_enable, r_enable;
    int16_t x, y, z;
    int16_t c_size, r_size, r_time, c_time, c_step, r_step;
};

struct smokeinstance_t {
    uint8_t status;
    uint8_t type;
    int16_t x, y, z;
    int16_t orientation;
    float scale, phase, phase_step;
};


extern explosioninstance_t explosions[MAXEXPLOSIONS];
extern smokeinstance_t smoke[MAXEXPLOSIONS];

#define BOSS2_VTXNUM 418
#define BOSS2_TRIS 469
#define BOSS2_FRAMES 15

#pragma pack(push, 1)
struct boss2vtx_t {
    int16_t x, y, z, u, v;
    int16_t color[3];
};

struct boss2tris_t {
    int16_t vtx[3];
    int16_t tile;
};

#pragma pack(pop)

extern int boss2seq, boss2seqframe;
extern int boss2mdlstate, boss2mdlstate2;
extern int boss2timer_step;
extern int boss2_frame, boss2_frame2;
extern int boss2timer;
extern float boss2_interp;

void RT_LoadTiles(void);
void RT_DrawRooms(int x, int y, int z, fix16_t ang, fix16_t horiz, int16_t sectnum, int smoothRatio);
void RT_GLInit(void);

void RT_DisablePolymost(int useShader);
void RT_EnablePolymost();

enum {
    RTRS_SCALED = 4096,
};

void RT_MS_Reset(void);
void RT_MS_Add(int sectnum, int x, int y);
void RT_MS_Update(int sectnum, int ang, int x, int y);
void RT_MS_SetInterpolation(int sectnum);
void RT_AdjustCeilingPanning(int sectnum, int x, int y);
void RT_AdjustFloorPanning(int sectnum, int x, int y);
void RT_RotateSpriteSetColor(int a1, int a2, int a3, int a4);
void RT_RotateSpriteSetShadePal(int ss, int shade, int pal);
void RT_RotateSpriteSetShadePalAlpha(int shade, int pal, int alpha);
void RT_RotateSprite(float x, float y, float sx, float sy, int tilenum, int orientation, bool screenCorrection = true);
void RT_RotateSpriteText(float x, float y, float sx, float sy, int tilenum, int orientation, bool buildcoords = false);
void RT_DrawTileFlash(int x, int y, int picnum, float sx, float sy, int orientation, int color);
void RT_AddSmoke(int16_t x, int16_t y, int16_t z, uint8_t type);
void RT_AddExplosion(int16_t x, int16_t y, int16_t z, uint8_t type);
void RT_AnimateModels(void);
void RT_LoadBOSS2MDL(void);
void RT_RenderScissor(float x1, float y1, float x2, float y2, bool absolute = false);
void RT_RenderUnsetScissor(void);
