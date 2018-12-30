#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compat.h"
#include "build.h"
#include "common.h"
#include "common_game.h"

#include "blood.h"
#include "resource.h"

void qloadvoxel(int32_t nVoxel)
{
    static int nLastVoxel = 0;
    dassert(nVoxel >= 0 && nVoxel < kMaxVoxels);
    DICTNODE *hVox = gSysRes.Lookup(nVoxel, "KVX");
    if (!hVox)
        ThrowError("Missing voxel #%d", nVoxel);
    if (!hVox->lockCount)
        voxoff[nLastVoxel][0] = 0;
    nLastVoxel = nVoxel;
    char *pVox = (char*)gSysRes.Lock(hVox);
    for (int i = 0; i < MAXVOXMIPS; i++)
    {
        int nSize = *((int*)pVox);
        pVox += 4;
        voxoff[nVoxel][i] = (int)pVox;
        pVox += nSize;
    }
}

void CalcPicsiz(int a1, int a2, int a3)
{
    int nP = 0;
    for (int i = 2; i <= a2; i<<= 1)
        nP++;
    for (int i = 2; i <= a3; i<<= 1)
        nP+=1<<4;
    picsiz[a1] = nP;
}

CACHENODE tileNode[kMaxTiles];

bool artLoaded = false;
int nTileFiles = 0;

int tileStart[256];
int tileEnd[256];
int hTileFile[256];

char surfType[kMaxTiles];
signed char tileShade[kMaxTiles];
short voxelIndex[kMaxTiles];

const char *pzBaseFileName = "TILES000.ART"; //"TILES%03i.ART";

int tileInit(char a1, const char *a2)
{
    if (artLoaded)
        return 1;
    artLoadFiles(a2 ? a2 : pzBaseFileName, 96 << 20);
    for (int i = 0; i < kMaxTiles; i++)
        voxelIndex[i] = 0;

    FILE *hFile = fopen("SURFACE.DAT", "rb");
    if (hFile != NULL)
    {
        fread(surfType, 1, sizeof(surfType), hFile);
        fclose(hFile);
    }
    hFile = fopen("VOXEL.DAT", "rb");
    if (hFile != NULL)
    {
        fread(voxelIndex, 1, sizeof(voxelIndex), hFile);
        fclose(hFile);
    }
    hFile = fopen("SHADE.DAT", "rb");
    if (hFile != NULL)
    {
        fread(tileShade, 1, sizeof(tileShade), hFile);
        fclose(hFile);
    }
    artLoaded = 1;
    return 1;
}

char * tileLoadTile(int nTile)
{
    tileLoad(nTile);
    return (char*)waloff[nTile];
}

char * tileAllocTile(int nTile, int x, int y, int ox, int oy)
{
    dassert(nTile >= 0 && nTile < kMaxTiles);
    char *p = (char*)tileCreate(nTile, x, y);
    dassert(p != NULL);
    qpicanm[nTile].xoffset = ClipRange(ox, -127, 127);
    qpicanm[nTile].yoffset = ClipRange(oy, -127, 127);
    return (char*)waloff[nTile];
}

void tilePreloadTile(int nTile)
{
    int n = 0;
    switch (qpicanm[nTile].at3_4)
    {
    case 0:
        n = 1;
        break;
    case 1:
        n = 5;
        break;
    case 2:
        n = 8;
        break;
    case 3:
        n = 2;
        break;
    case 6:
    case 7:
        if (voxelIndex[nTile] < 0 || voxelIndex[nTile] >= kMaxVoxels)
        {
            voxelIndex[nTile] = -1;
            qpicanm[nTile].at3_4 = 0;
        }
        else
            qloadvoxel(voxelIndex[nTile]);
        break;
    }
    while(n--)
    {
        if (qpicanm[nTile].animtype)
        {
            for (int frame = qpicanm[nTile].animframes; frame >= 0; frame--)
            {
                if (qpicanm[nTile].animtype == 3)
                    tileLoadTile(nTile-frame);
                else
                    tileLoadTile(nTile+frame);
            }
        }
        else
            tileLoadTile(nTile);
        nTile += 1+qpicanm[nTile].animframes;
    }
}

void tilePreloadTile2(int nTile)
{
    int n = 0;
    switch (qpicanm[nTile].at3_4)
    {
    case 0:
        n = 1;
        break;
    case 1:
        n = 5;
        break;
    case 2:
        n = 8;
        break;
    case 3:
        n = 2;
        break;
    }
    while(n--)
    {
        if (qpicanm[nTile].animtype)
        {
            for (int frame = qpicanm[nTile].animframes; frame >= 0; frame--)
            {
                if (qpicanm[nTile].animtype == 3)
                    SetBitString(gotpic, nTile-frame);
                else
                    SetBitString(gotpic, nTile+frame);
            }
        }
        else
            SetBitString(gotpic, nTile);
        nTile += 1+qpicanm[nTile].animframes;
    }
}

char tileGetSurfType(int hit)
{
    int n = hit &0x1fff;
    switch (hit&0xe000)
    {
    case 0x4000:
        return surfType[sector[n].floorpicnum];
    case 0x6000:
        return surfType[sector[n].ceilingpicnum];
    case 0x8000:
        return surfType[wall[n].picnum];
    case 0xc000:
        return surfType[sprite[n].picnum];
    }
    return 0;
}
