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
#define kDudeMax 260
#define kMissileBase 300
#define kMissileMax 318
#define kThingBase 400
#define kThingMax 433

#define kMaxPowerUps 49

#define kStatRespawn 8
#define kStatMarker 10
#define kStatFree 1024

#define kLensSize 80
#define kViewEffectMax 19

#define kNoTile -1


// defined by NoOne:
// -------------------------------

#define kMaxPAL 5

// marker sprite types
#define kMarkerSPStart 1
#define kMarkerMPStart 2
#define kMarkerOff 3
#define kMarkerOn 4
#define kMarkerAxis 5
#define kMarkerLowLink 6
#define kMarkerUpLink 7
#define kMarkerWarpDest 8
#define kMarkerUpWater 9
#define kMarkerLowWater 10
#define kMarkerUpStack 11
#define kMarkerLowStack 12
#define kMarkerUpGoo 13
#define kMarkerLowGoo 14
#define kMarkerPath 15

// sprite cstat
#define kSprBlock 0x0001
#define kSprTrans 0x0002
#define kSprFlipX 0x0004
#define kSprFlipY 0x0008
#define kSprFace 0x0000
#define kSprWall 0x0010
#define kSprFloor 0x0020
#define kSprSpin 0x0030
#define kSprRMask 0x0030
#define kSprOneSided 0x0040
#define kSprOriginAlign 0x0080
#define kSprHitscan 0x0100
#define kSprTransR 0x0200
#define kSprPushable 0x1000
#define kSprMoveMask 0x6000
#define kSprMoveNone 0x0000
#define kSprMoveForward 0x2000
#define kSprMoveFloor 0x2000
#define kSprMoveReverse 0x4000
#define kSprMoveCeiling 0x4000
#define kSprInvisible 0x8000

// sprite attributes
#define kHitagMovePhys 0x0001 // affected by movement physics
#define kHitagGravityPhys 0x0002 // affected by gravity
#define kHitagFalling 0x0004 // currently in z-motion
#define kHitagAutoAim 0x0008
#define kHitagRespawn 0x0010
#define kHitagFree 0x0020
#define kHitagSmoke 0x0100

// sector types 
#define kSecBase 600
#define kSecZMotion kSectorBase
#define kSecZSprite 602
#define kSecWarp 603
#define kSecTeleport 604
#define kSecPath 612
#define kSecRotateStep 613
#define kSecSlideMarked 614
#define kSecRotateMarked 615
#define kSecSlide 616
#define kSecRotate 617
#define kSecDamage 618
#define kSecCounter 619
#define kSecMax 620

// switch types
#define kSwitchBase 20
#define kSwitchToggle 20
#define kSwitchOneWay 21
#define kSwitchCombo 22
#define kSwitchPadlock 23
#define kSwitchMax 24

// projectile types
#define kProjectileEctoSkull 307

#define kGDXChannelEndLevelCustom 6 // custom level end
// -------------------------------

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
bool isDemoRecords(void);
bool isOriginalDemo();