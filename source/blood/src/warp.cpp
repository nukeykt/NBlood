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
#include "build.h"
#include "compat.h"
#include "common_game.h"
#include "blood.h"
#include "db.h"
#include "gameutil.h"
#include "levels.h"
#include "loadsave.h"
#include "view.h"
#include "warp.h"

ZONE gStartZone[8];

void warpInit(void)
{
    for (int i = 0; i < kMaxSectors; i++)
    {
        gUpperLink[i] = -1;
        gLowerLink[i] = -1;
    }
    for (int nSprite = 0; nSprite < kMaxSprites; nSprite++)
    {
        if (sprite[nSprite].statnum < kMaxStatus)
        {
            SPRITE *pSprite = &qsprite[nSprite];
            int nXSprite = pSprite->extra;
            if (nXSprite > 0)
            {
                XSPRITE *pXSprite = &xsprite[nXSprite];
                switch (pSprite->type)
                {
                case 1:
                    if (gGameOptions.nGameType < 2 && pXSprite->at10_0 >= 0 && pXSprite->at10_0 < 8)
                    {
                        ZONE *pZone = &gStartZone[pXSprite->at10_0];
                        pZone->x = pSprite->x;
                        pZone->y = pSprite->y;
                        pZone->z = pSprite->z;
                        pZone->sectnum = pSprite->sectnum;
                        pZone->ang = pSprite->ang;
                    }
                    DeleteSprite(nSprite);
                    break;
                case 2:
                    if (gGameOptions.nGameType >= 2 && pXSprite->at10_0 >= 0 && pXSprite->at10_0 < 8)
                    {
                        ZONE *pZone = &gStartZone[pXSprite->at10_0];
                        pZone->x = pSprite->x;
                        pZone->y = pSprite->y;
                        pZone->z = pSprite->z;
                        pZone->sectnum = pSprite->sectnum;
                        pZone->ang = pSprite->ang;
                    }
                    DeleteSprite(nSprite);
                    break;
                case 7:
                    gUpperLink[pSprite->sectnum] = nSprite;
                    pSprite->cstat |= 32768;
                    pSprite->cstat &= ~257;
                    break;
                case 6:
                    gLowerLink[pSprite->sectnum] = nSprite;
                    pSprite->cstat |= 32768;
                    pSprite->cstat &= ~257;
                    break;
                case 9:
                case 11:
                case 13:
                    gUpperLink[pSprite->sectnum] = nSprite;
                    pSprite->cstat |= 32768;
                    pSprite->cstat &= ~257;
                    pSprite->z = getflorzofslope(pSprite->sectnum, pSprite->x, pSprite->y);
                    break;
                case 10:
                case 12:
                case 14:
                    gLowerLink[pSprite->sectnum] = nSprite;
                    pSprite->cstat |= 32768;
                    pSprite->cstat &= ~257;
                    pSprite->z = getceilzofslope(pSprite->sectnum, pSprite->x, pSprite->y);
                    break;
                }
            }
        }
    }
    for (int i = 0; i < kMaxSectors; i++)
    {
        int nSprite = gUpperLink[i];
        if (nSprite >= 0)
        {
            SPRITE *pSprite = &qsprite[nSprite];
            int nXSprite = pSprite->extra;
            dassert(nXSprite > 0 && nXSprite < kMaxXSprites);
            XSPRITE *pXSprite = &xsprite[nXSprite];
            int nLink = pXSprite->at10_0;
            for (int j = 0; j < kMaxSectors; j++)
            {
                int nSprite2 = gLowerLink[j];
                if (nSprite2 >= 0)
                {
                    SPRITE *pSprite2 = &qsprite[nSprite2];
                    int nXSprite = pSprite2->extra;
                    dassert(nXSprite > 0 && nXSprite < kMaxXSprites);
                    XSPRITE *pXSprite2 = &xsprite[nXSprite];
                    if (pXSprite2->at10_0 == nLink)
                    {
                        pSprite->owner = gLowerLink[j];
                        pSprite2->owner = gUpperLink[i];
                    }
                }
            }
        }
    }
}

int CheckLink(SPRITE *pSprite)
{
    int nSector = pSprite->sectnum;
    int nUpper = gUpperLink[nSector];
    int nLower = gLowerLink[nSector];
    if (nUpper >= 0)
    {
        SPRITE *pUpper = &qsprite[nUpper];
        int z;
        if (pUpper->type == 7)
            z = pUpper->z;
        else
            z = getflorzofslope(pSprite->sectnum, pSprite->x, pSprite->y);
        if (z <= pSprite->z)
        {
            nLower = pUpper->owner;
            dassert(nLower >= 0 && nLower < kMaxSprites);
            SPRITE *pLower = &qsprite[nLower];
            dassert(pLower->sectnum >= 0 && pLower->sectnum < kMaxSectors);
            ChangeSpriteSect(pSprite->index, pLower->sectnum);
            pSprite->x += pLower->x-pUpper->x;
            pSprite->y += pLower->y-pUpper->y;
            int z2;
            if (pLower->type == 6)
                z2 = pLower->z;
            else
                z2 = getceilzofslope(pSprite->sectnum, pSprite->x, pSprite->y);
            pSprite->z += z2-z;
            ClearBitString(gInterpolateSprite, pSprite->index);
            return pUpper->type;
        }
    }
    if (nLower >= 0)
    {
        SPRITE *pLower = &qsprite[nLower];
        int z;
        if (pLower->type == 6)
            z = pLower->z;
        else
            z = getceilzofslope(pSprite->sectnum, pSprite->x, pSprite->y);
        if (z >= pSprite->z)
        {
            nUpper = pLower->owner;
            dassert(nUpper >= 0 && nUpper < kMaxSprites);
            SPRITE *pUpper = &qsprite[nUpper];
            dassert(pUpper->sectnum >= 0 && pUpper->sectnum < kMaxSectors);
            ChangeSpriteSect(pSprite->index, pUpper->sectnum);
            pSprite->x += pUpper->x-pLower->x;
            pSprite->y += pUpper->y-pLower->y;
            int z2;
            if (pUpper->type == 7)
                z2 = pUpper->z;
            else
                z2 = getflorzofslope(pSprite->sectnum, pSprite->x, pSprite->y);
            pSprite->z += z2-z;
            ClearBitString(gInterpolateSprite, pSprite->index);
            return pLower->type;
        }
    }
    return 0;
}

int CheckLink(long *x, long *y, long *z, int *nSector)
{
    int nUpper = gUpperLink[*nSector];
    int nLower = gLowerLink[*nSector];
    if (nUpper >= 0)
    {
        SPRITE *pUpper = &qsprite[nUpper];
        int z1;
        if (pUpper->type == 7)
            z1 = pUpper->z;
        else
            z1 = getflorzofslope(*nSector, *x, *y);
        if (z1 <= *z)
        {
            nLower = pUpper->owner;
            dassert(nLower >= 0 && nLower < kMaxSprites);
            SPRITE *pLower = &qsprite[nLower];
            dassert(pLower->sectnum >= 0 && pLower->sectnum < kMaxSectors);
            *nSector = pLower->sectnum;
            *x += pLower->x-pUpper->x;
            *y += pLower->y-pUpper->y;
            int z2;
            if (pUpper->type == 6)
                z2 = pLower->z;
            else
                z2 = getceilzofslope(*nSector, *x, *y);
            *z += z2-z1;
            return pUpper->type;
        }
    }
    if (nLower >= 0)
    {
        SPRITE *pLower = &qsprite[nLower];
        int z1;
        if (pLower->type == 6)
            z1 = pLower->z;
        else
            z1 = getceilzofslope(*nSector, *x, *y);
        if (z1 >= *z)
        {
            nUpper = pLower->owner;
            dassert(nUpper >= 0 && nUpper < kMaxSprites);
            SPRITE *pUpper = &qsprite[nUpper];
            dassert(pUpper->sectnum >= 0 && pUpper->sectnum < kMaxSectors);
            *nSector = pUpper->sectnum;
            *x += pUpper->x-pLower->x;
            *y += pUpper->y-pLower->y;
            int z2;
            if (pLower->type == 7)
                z2 = pUpper->z;
            else
                z2 = getflorzofslope(*nSector, *x, *y);
            *z += z2-z1;
            return pLower->type;
        }
    }
    return 0;
}

class WarpLoadSave : public LoadSave
{
public:
    virtual void Load();
    virtual void Save();
};

void WarpLoadSave::Load()
{
    Read(gStartZone, sizeof(gStartZone));
    Read(gUpperLink, sizeof(gUpperLink));
    Read(gLowerLink, sizeof(gLowerLink));
}

void WarpLoadSave::Save()
{
    Write(gStartZone, sizeof(gStartZone));
    Write(gUpperLink, sizeof(gUpperLink));
    Write(gLowerLink, sizeof(gLowerLink));
}

static WarpLoadSave *myLoadSave;

void WarpLoadSaveConstruct(void)
{
    myLoadSave = new WarpLoadSave();
}
