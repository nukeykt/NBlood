//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 Nuke.YKT

This file is part of NBlood.

NBlood is free software; you can redistribute it and/or
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

#include "common_game.h"
#include "controls.h"
#include "messages.h"
#include "player.h"

enum VIEW_EFFECT {
    VIEW_EFFECT_0 = 0,
    VIEW_EFFECT_1,
    VIEW_EFFECT_2,
    VIEW_EFFECT_3,
    VIEW_EFFECT_4,
    VIEW_EFFECT_5,
    VIEW_EFFECT_6,
    VIEW_EFFECT_7,
    VIEW_EFFECT_8,
    VIEW_EFFECT_9,
    VIEW_EFFECT_10,
    VIEW_EFFECT_11,
    VIEW_EFFECT_12,
    VIEW_EFFECT_13,
    VIEW_EFFECT_14,
    VIEW_EFFECT_15,
    VIEW_EFFECT_16,
    VIEW_EFFECT_17,
    VIEW_EFFECT_18,
};

enum VIEWPOS {
    VIEWPOS_0 = 0,
    VIEWPOS_1
};

enum INTERPOLATE_TYPE {
    INTERPOLATE_TYPE_INT = 0,
    INTERPOLATE_TYPE_SHORT,
};

struct FONT {
    int tile, xSize, ySize, space;
};

extern int gZoom;
extern FONT gFont[5];
extern int gViewMode;
extern VIEWPOS gViewPos;
extern int gViewIndex;
extern long gScreenTilt;
extern int deliriumTilt, deliriumTurn, deliriumPitch;
extern int gScreenTiltO, deliriumTurnO, deliriumPitchO;
extern int gShowFrameRate;
extern char gInterpolateSprite[512];
extern char gInterpolateWall[1024];
extern char gInterpolateSector[128];
extern LOCATION gPrevSpriteLoc[kMaxSprites];
extern long gViewSize;
extern CGameMessageMgr gGameMessageMgr;
extern int gViewXCenter, gViewYCenter;
extern int gViewX0, gViewY0, gViewX1, gViewY1;
extern int gViewX0S, gViewY0S, gViewX1S, gViewY1S;

void viewGetFontInfo(int id, const char *unk1, int *pXSize, int *pYSize);
void viewUpdatePages(void);
void viewToggle(int viewMode);
void viewInitializePrediction(void);
void viewUpdatePrediction(GINPUT *pInput);
void sub_158B4(PLAYER *pPlayer);
void fakeProcessInput(PLAYER *pPlayer, GINPUT *pInput);
void fakePlayerProcess(PLAYER *pPlayer, GINPUT *pInput);
void fakeMoveDude(SPRITE *pSprite);
void fakeActAirDrag(SPRITE *pSprite, int num);
void fakeActProcessSprites(void);
void viewCorrectPrediction(void);
void viewBackupView(int nPlayer);
void viewClearInterpolations(void);
void viewAddInterpolation(void *data, INTERPOLATE_TYPE type);
void CalcInterpolations(void);
void RestoreInterpolations(void);
void viewDrawText(int nFont, const char *pString, int x, int y, int nShade, int nPalette, int position, char shadow);
void viewTileSprite(int nTile, int nShade, int nPalette, int x1, int y1, int x2, int y2);
void InitStatusBar(void);
void DrawStatSprite(int nTile, int x, int y, int nShade = 0, int nPalette = 0, unsigned int nStat = 0);
void DrawStatMaskedSprite(int nTile, int x, int y, int nShade = 0, int nPalette = 0, unsigned int nStat = 0);
void DrawStatNumber(const char *pFormat, int nNumber, int nTile, int x, int y, int nShade, int nPalette);
void TileHGauge(int nTile, int x, int y, int nMult, int nDiv);
void viewDrawPack(PLAYER *pPlayer, int x, int y);
void DrawPackItemInStatusBar(PLAYER *pPlayer, int x, int y, int x2, int y2);
void UpdateStatusBar(int arg);
void viewInit(void);
void viewResizeView(int size);
void UpdateFrame(void);
void viewDrawInterface(int arg);
SPRITE *viewInsertTSprite(int nSector, int nStatnum, SPRITE *pSprite);
SPRITE *viewAddEffect(int nTSprite, VIEW_EFFECT nViewEffect);
void viewProcessSprites(int cX, int cY, int cZ);
void CalcOtherPosition(SPRITE *pSprite, long *pX, long *pY, long *pZ, int *vsectnum, int nAng, int zm);
void CalcPosition(SPRITE *pSprite, long *pX, long *pY, long *pZ, int *vsectnum, int nAng, int zm);
void viewDrawSprite(long x, long y, long z, int a, int pn, signed char shade, char pal, unsigned short stat, long xd1, long yd1, long xd2, long yd2);
void viewBurnTime(int gScale);
void viewSetMessage(const char *pMessage);
void viewDisplayMessage(void);
void viewSetErrorMessage(const char *pMessage);
void DoLensEffect(void);
void UpdateDacs(int nPalette);
void viewDrawScreen(void);
void viewLoadingScreen(int nTile, const char *pText, const char *pText2, const char *pText3);
void viewUpdateDelirium(void);
void viewUpdateShake(void);


inline void viewInterpolateSector(int nSector, SECTOR *pSector)
{
    if (!TestBitString(gInterpolateSector, nSector))
    {
        viewAddInterpolation(&pSector->floorz, INTERPOLATE_TYPE_INT);
        viewAddInterpolation(&pSector->ceilingz, INTERPOLATE_TYPE_INT);
        viewAddInterpolation(&pSector->floorheinum, INTERPOLATE_TYPE_SHORT);
        SetBitString(gInterpolateSector, nSector);
    }
}

inline void viewInterpolateWall(int nWall, WALL *pWall)
{
    if (!TestBitString(gInterpolateWall, nWall))
    {
        viewAddInterpolation(&pWall->x, INTERPOLATE_TYPE_INT);
        viewAddInterpolation(&pWall->y, INTERPOLATE_TYPE_INT);
        SetBitString(gInterpolateWall, nWall);
    }
}

inline void viewBackupSpriteLoc(int nSprite, SPRITE *pSprite)
{
    if (!TestBitString(gInterpolateSprite, nSprite))
    {
        LOCATION *pPrevLoc = &gPrevSpriteLoc[nSprite];
        pPrevLoc->x = pSprite->x;
        pPrevLoc->y = pSprite->y;
        pPrevLoc->z = pSprite->z;
        pPrevLoc->ang = pSprite->ang;
        SetBitString(gInterpolateSprite, nSprite);
    }
}
