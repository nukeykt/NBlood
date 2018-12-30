#pragma once
#include "build.h"
#include "common_game.h"
#include "blood.h"

enum SurfaceType {
    kSurf0 = 0,
    kSurf1,
    kSurf2,
    kSurf3,
    kSurf4,
    kSurf5,
    kSurf6,
    kSurf7,
    kSurf8,
    kSurf9,
    kSurf10,
    kSurf11,
    kSurf12,
    kSurf13,
    kSurf14,
    kSurfMax
};

extern char surfType[kMaxTiles];
extern signed char tileShade[kMaxTiles];
extern short voxelIndex[kMaxTiles];

void qloadvoxel(int32_t nVoxel);
void CalcPicsiz(int a1, int a2, int a3);
int tileInit(char a1, const char *a2);
char * tileLoadTile(int nTile);
char * tileAllocTile(int nTile, int x, int y, int ox, int oy);
void tilePreloadTile(int nTile);
void tilePreloadTile2(int nTile);
char tileGetSurfType(int hit);
