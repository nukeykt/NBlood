//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 sirlemonhead, Nuke.YKT

This file is part of PCExhumed.

PCExhumed is free software; you can redistribute it and/or
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

#ifndef __exhumed_h__
#define __exhumed_h__

#include "compat.h"
#include "cache1d.h"
#include "vfs.h"
#include "common_game.h"
#include "grpscan.h"

#define kTimerTicks		120

#ifdef __WATCOMC__
void handleevents();
#endif

#pragma pack(push, 1)
struct demo_header
{
    uint8_t nMap;
    int16_t nWeapons;
    int16_t nCurrentWeapon;
    int16_t clip;
    int16_t items;

    int16_t nHealth;
    int16_t nFrame;
    int16_t nAction;
    int16_t nSprite;
    int16_t bIsMummified;
    int16_t someNetVal;
    int16_t invincibility;
    int16_t nAir;
    int16_t nSeq;
    int16_t nMaskAmount;
    uint16_t keys;
    int16_t nMagic;
    uint8_t item[8];
    int16_t nAmmo[7]; // TODO - kMaxWeapons?
    int16_t pad[2];
    int16_t nCurrentWeapon2;
    int16_t nWeaponFrame;
    int16_t bIsFiring;
    int16_t nNewWeapon;
    int16_t nWeaponState;
    int16_t nLastWeapon;
    int16_t nRun;

    int16_t nLives;
};

struct demo_input
{
    int32_t moveframes;

    int32_t xVel;
    int32_t yVel;
    int16_t nAngle;
    uint16_t buttons;
    int16_t nTarget;
    uint8_t horizon;
    int8_t nItem;
    int32_t h;
    uint8_t i;
    uint8_t pad[11];
};
#pragma pack(pop)

void ExitGame();
void ShutDown(void);
void DebugOut(const char *fmt, ...);
void bail2dos(const char *fmt, ...);
int ExhumedMain(int argc, char *argv[]);
void FinishLevel();
void SetHiRes();
void DoGameOverScene();
int Query(short n, short l, ...);
void EraseScreen(int nClearColour);
int FindGString(const char *str);
void WaitTicks(int nTicks);
int myprintext(int x, int y, const char *str, int shade);
int MyGetStringWidth(const char *str);
void mychangespritesect(int nSprite, int nSector);
void mydeletesprite(int nSprite);
void mysetbrightness(char nBrightness);
void DoPassword(int nPassword);
int CopyCharToBitmap(char nChar, int nTile, int xPos, int yPos);
void UpdateScreenSize();
void HandleAsync();

extern int32_t g_commandSetup;
extern int32_t g_noSetup;
extern char sHollyStr[];
extern int localclock;
extern int moveframes;
extern short bSerialPlay;
extern int nNetPlayerCount;
extern int htimer;
extern int nNetTime;
extern short nTotalPlayers;
extern short nFontFirstChar;
extern short nBackgroundPic;
extern short nShadowPic;
extern short nCreaturesLeft;
extern int lLocalButtons;
extern short nEnergyTowers;
extern short nEnergyChan;
extern short bInDemo;
extern short nFreeze;
extern short nCurBodyNum;
extern short nBodyTotal;
extern short bSnakeCam;
extern short nBestLevel;
extern short levelnum;
extern short nMapMode;
extern short nButtonColor;
extern short lastfps;
extern int flash;
extern short bNoCreatures;
extern short nLocalSpr;
extern short levelnew;
extern short textpages;
extern short nSnakeCam;
extern short bHiRes;
extern short bCoordinates;
extern int32_t bFullScreen;
extern int32_t screensize;
extern int32_t nGamma;
extern short bHolly;
extern int totalmoves;
extern int lCountDown;
extern short bSlipMode;
extern short nItemTextIndex;
extern const char* gString[];
extern const char* gPSDemoString[];
extern const char* gEXDemoString[];
extern short bNoSound;
extern int bVanilla;

extern ClockTicks tclocks, tclocks2;
extern short nRedTicks;
extern short nAlarmTicks;
extern short nClockVal;

extern int mouseaiming, aimmode, mouseflip;
extern int runkey_mode, auto_run;

static inline double calcFrameDelay(unsigned int const maxFPS) { return maxFPS ? timerGetPerformanceFrequency() / (double)maxFPS : 0.0; }

extern int r_showfps;

#endif
