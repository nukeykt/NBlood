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

#include "levels.h"
#include "resource.h"

#define TILTBUFFER 4078

#define kExplodeMax 8
#define kDudeBase 200
#define kDudePlayer1 231
#define kDudePlayer8 238
#define kDudeMax 254
#define kMissileBase 300
#define kMissileMax 318
#define kThingBase 400
#define kThingMax 433

#define kMaxPowerUps 49

#define kStatRespawn 8
#define kStatMarker 10
#define kStatFree 1024

#define kMarkerWarpDest 8

#define kLensSize 80
#define kViewEffectMax 19

#define kNoTile -1

struct INIDESCRIPTION {
    const char *pzName;
    const char *pzFilename;
    const char **pzArts;
    int nArts;
};

struct INICHAIN {
    INICHAIN *pNext;
    char zName[BMAX_PATH];
    INIDESCRIPTION *pDescription;
};

extern INICHAIN *pINIChain;
extern INICHAIN const*pINISelected;

typedef struct {
    int32_t usejoystick;
    int32_t usemouse;
    int32_t fullscreen;
    int32_t xdim;
    int32_t ydim;
    int32_t bpp;
    int32_t forcesetup;
    int32_t noautoload;
} ud_setup_t;

enum INPUT_MODE {
    INPUT_MODE_0 = 0,
    INPUT_MODE_1,
    INPUT_MODE_2,
    INPUT_MODE_3,
};

extern Resource gSysRes, gGuiRes;
extern INPUT_MODE gInputMode;
extern ud_setup_t gSetup;
extern char SetupFilename[BMAX_PATH];
extern int32_t gNoSetup;
extern short BloodVersion;
extern int gNetPlayers;
extern bool gRestartGame;
#define GAMEUPDATEAVGTIMENUMSAMPLES 100
extern double g_gameUpdateTime, g_gameUpdateAndDrawTime;
extern double g_gameUpdateAvgTime;
extern int blood_globalflags;
extern bool bVanilla;

void QuitGame(void);
void PreloadCache(void);
void StartLevel(GAMEOPTIONS *gameOptions);
void ProcessFrame(void);
void ScanINIFiles(void);
bool LoadArtFile(const char *pzFile);
void LoadExtraArts(void);