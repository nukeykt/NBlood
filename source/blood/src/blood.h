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

void QuitGame(void);
void PreloadCache(void);
void StartLevel(GAMEOPTIONS *gameOptions);
void ProcessFrame(void);