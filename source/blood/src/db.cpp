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
#ifdef POLYMER
#include "polymer.h"
#endif
#include "compat.h"
#include "common_game.h"
#include "crc32.h"

#include "actor.h"
#include "blood.h"
#include "db.h"
#include "iob.h"

unsigned short gStatCount[kMaxStatus + 1];

XSPRITE xsprite[kMaxXSprites];
XSECTOR xsector[kMaxXSectors];
XWALL xwall[kMaxXWalls];

int xvel[kMaxSprites], yvel[kMaxSprites], zvel[kMaxSprites];

int gVisibility;

const char *gItemText[] = {
    "Skull Key",
    "Eye Key",
    "Fire Key",
    "Dagger Key",
    "Spider Key",
    "Moon Key",
    "Key 7",
    "Doctor's Bag",
    "Medicine Pouch",
    "Life Essence",
    "Life Seed",
    "Red Potion",
    "Feather Fall",
    "Limited Invisibility",
    "INVULNERABILITY",
    "Boots of Jumping",
    "Raven Flight",
    "Guns Akimbo",
    "Diving Suit",
    "Gas mask",
    "Clone",
    "Crystal Ball",
    "Decoy",
    "Doppleganger",
    "Reflective shots",
    "Beast Vision",
    "ShadowCloak",
    "Rage shroom",
    "Delirium Shroom",
    "Grow shroom",
    "Shrink shroom",
    "Death mask",
    "Wine Goblet",
    "Wine Bottle",
    "Skull Grail",
    "Silver Grail",
    "Tome",
    "Black Chest",
    "Wooden Chest",
    "Asbestos Armor",
    "Basic Armor",
    "Body Armor",
    "Fire Armor",
    "Spirit Armor",
    "Super Armor",
    "Blue Team Base",
    "Red Team Base",
    "Blue Flag",
    "Red Flag",
};

const char *gAmmoText[] = {
    "Spray can",
    "Bundle of TNT*",
    "Bundle of TNT",
    "Case of TNT",
    "Proximity Detonator",
    "Remote Detonator",
    "Trapped Soul",
    "4 shotgun shells",
    "Box of shotgun shells",
    "A few bullets",
    "Voodoo Doll",
    "OBSOLETE",
    "Full drum of bullets",
    "Tesla Charge",
    "OBSOLETE",
    "OBSOLETE",
    "Flares",
    "OBSOLETE",
    "OBSOLETE",
    "Gasoline Can",
    NULL,
};

const char *gWeaponText[] = {
    "RANDOM",
    "Sawed-off",
    "Tommy Gun",
    "Flare Pistol",
    "Voodoo Doll",
    "Tesla Cannon",
    "Napalm Launcher",
    "Pitchfork",
    "Spray Can",
    "Dynamite",
    "Life Leech",
};

void dbCrypt(char *pPtr, int nLength, int nKey)
{
    for (int i = 0; i < nLength; i++)
    {
        pPtr[i] = pPtr[i] ^ nKey;
        nKey++;
    }
}

void InsertSpriteSect(int nSprite, int nSector)
{
    dassert(nSprite >= 0 && nSprite < kMaxSprites);
    dassert(nSector >= 0 && nSector < kMaxSectors);
    int nOther = headspritesect[nSector];
    if (nOther >= 0)
    {
        prevspritesect[nSprite] = prevspritesect[nOther];
        nextspritesect[nSprite] = -1;
        nextspritesect[prevspritesect[nOther]] = nSprite;
        prevspritesect[nOther] = nSprite;
    }
    else
    {
        prevspritesect[nSprite] = nSprite;
        nextspritesect[nSprite] = -1;
        headspritesect[nSector] = nSprite;
    }
    sprite[nSprite].sectnum = nSector;
}

void RemoveSpriteSect(int nSprite)
{
    dassert(nSprite >= 0 && nSprite < kMaxSprites);
    int nSector = sprite[nSprite].sectnum;
    dassert(nSector >= 0 && nSector < kMaxSectors);
    int nOther = nextspritesect[nSprite];
    if (nOther < 0)
    {
        nOther = headspritesect[nSector];
    }
    prevspritesect[nOther] = prevspritesect[nSprite];
    if (headspritesect[nSector] != nSprite)
    {
        nextspritesect[prevspritesect[nSprite]] = nextspritesect[nSprite];
    }
    else
    {
        headspritesect[nSector] = nextspritesect[nSprite];
    }
    sprite[nSprite].sectnum = -1;
}

void InsertSpriteStat(int nSprite, int nStat)
{
    dassert(nSprite >= 0 && nSprite < kMaxSprites);
    dassert(nStat >= 0 && nStat <= kMaxStatus);
    int nOther = headspritestat[nStat];
    if (nOther >= 0)
    {
        prevspritestat[nSprite] = prevspritestat[nOther];
        nextspritestat[nSprite] = -1;
        nextspritestat[prevspritestat[nOther]] = nSprite;
        prevspritestat[nOther] = nSprite;
    }
    else
    {
        prevspritestat[nSprite] = nSprite;
        nextspritestat[nSprite] = -1;
        headspritestat[nStat] = nSprite;
    }
    sprite[nSprite].statnum = nStat;
    gStatCount[nStat]++;
}

void RemoveSpriteStat(int nSprite)
{
    dassert(nSprite >= 0 && nSprite < kMaxSprites);
    int nStat = sprite[nSprite].statnum;
    dassert(nStat >= 0 && nStat <= kMaxStatus);
    int nOther = nextspritestat[nSprite];
    if (nOther < 0)
    {
        nOther = headspritestat[nStat];
    }
    prevspritestat[nOther] = prevspritestat[nSprite];
    if (headspritestat[nStat] != nSprite)
    {
        nextspritestat[prevspritestat[nSprite]] = nextspritestat[nSprite];
    }
    else
    {
        headspritestat[nStat] = nextspritestat[nSprite];
    }
    sprite[nSprite].statnum = -1;
    gStatCount[nStat]--;
}

void qinitspritelists(void) // Replace
{
    for (short i = 0; i <= kMaxSectors; i++)
    {
        headspritesect[i] = -1;
    }
    for (short i = 0; i <= kMaxStatus; i++)
    {
        headspritestat[i] = -1;
    }
    for (short i = 0; i < kMaxSprites; i++)
    {
        sprite[i].sectnum = -1;
        sprite[i].index = -1;
        InsertSpriteStat(i, kMaxStatus);
    }
    memset(gStatCount, 0, sizeof(gStatCount));
}

int InsertSprite(int nSector, int nStat)
{
    int nSprite = headspritestat[kMaxStatus];
    dassert(nSprite < kMaxSprites);
    if (nSprite < 0)
    {
        return nSprite;
    }
    RemoveSpriteStat(nSprite);
    spritetype *pSprite = &sprite[nSprite];
    memset(&sprite[nSprite], 0, sizeof(spritetype));
    InsertSpriteStat(nSprite, nStat);
    InsertSpriteSect(nSprite, nSector);
    pSprite->cstat = 128;
    pSprite->clipdist = 32;
    pSprite->xrepeat = pSprite->yrepeat = 64;
    pSprite->owner = -1;
    pSprite->extra = -1;
    pSprite->index = nSprite;
    xvel[nSprite] = yvel[nSprite] = zvel[nSprite] = 0;

    return nSprite;
}

int qinsertsprite(short nSector, short nStat) // Replace
{
    return InsertSprite(nSector, nStat);
}

int DeleteSprite(int nSprite)
{
    if (sprite[nSprite].extra > 0)
    {
        dbDeleteXSprite(sprite[nSprite].extra);
    }
    dassert(sprite[nSprite].statnum >= 0 && sprite[nSprite].statnum < kMaxStatus);
    RemoveSpriteStat(nSprite);
    dassert(sprite[nSprite].sectnum >= 0 && sprite[nSprite].sectnum < kMaxSectors);
    RemoveSpriteSect(nSprite);
    InsertSpriteStat(nSprite, kMaxStatus);

    return nSprite;
}

int qdeletesprite(short nSprite) // Replace
{
    return DeleteSprite(nSprite);
}

int ChangeSpriteSect(int nSprite, int nSector)
{
    dassert(nSprite >= 0 && nSprite < kMaxSprites);
    dassert(nSector >= 0 && nSector < kMaxSectors);
    dassert(sprite[nSprite].sectnum >= 0 && sprite[nSprite].sectnum < kMaxSectors);
    RemoveSpriteSect(nSprite);
    InsertSpriteSect(nSprite, nSector);
    return 0;
}

int qchangespritesect(short nSprite, short nSector)
{
    return ChangeSpriteSect(nSprite, nSector);
}

int ChangeSpriteStat(int nSprite, int nStatus)
{
    dassert(nSprite >= 0 && nSprite < kMaxSprites);
    dassert(nStatus >= 0 && nStatus < kMaxStatus);
    dassert(sprite[nSprite].statnum >= 0 && sprite[nSprite].statnum < kMaxStatus);
    dassert(sprite[nSprite].sectnum >= 0 && sprite[nSprite].sectnum < kMaxSectors);
    RemoveSpriteStat(nSprite);
    InsertSpriteStat(nSprite, nStatus);
    return 0;
}

int qchangespritestat(short nSprite, short nStatus)
{
    return ChangeSpriteStat(nSprite, nStatus);
}

unsigned short nextXSprite[kMaxXSprites];
unsigned short nextXWall[kMaxXWalls];
unsigned short nextXSector[kMaxXSectors];

void InitFreeList(unsigned short *pList, int nCount)
{
    for (int i = 1; i < nCount; i++)
    {
        pList[i] = i-1;
    }
    pList[0] = nCount - 1;
}

void InsertFree(unsigned short *pList, int nIndex)
{
    pList[nIndex] = pList[0];
    pList[0] = nIndex;
}

unsigned short dbInsertXSprite(int nSprite)
{
    int nXSprite = nextXSprite[0];
    nextXSprite[0] = nextXSprite[nXSprite];
    if (nXSprite == 0)
    {
        ThrowError("Out of free XSprites");
    }
    memset(&xsprite[nXSprite], 0, sizeof(XSPRITE));
    if (!bVanilla)
        memset(&gSpriteHit[nXSprite], 0, sizeof(SPRITEHIT));
    xsprite[nXSprite].reference = nSprite;
    sprite[nSprite].extra = nXSprite;
    return nXSprite;
}

void dbDeleteXSprite(int nXSprite)
{
    dassert(xsprite[nXSprite].reference >= 0);
    dassert(sprite[xsprite[nXSprite].reference].extra == nXSprite);
    InsertFree(nextXSprite, nXSprite);
    sprite[xsprite[nXSprite].reference].extra = -1;
    xsprite[nXSprite].reference = -1;
}

unsigned short dbInsertXWall(int nWall)
{
    int nXWall = nextXWall[0];
    nextXWall[0] = nextXWall[nXWall];
    if (nXWall == 0)
    {
        ThrowError("Out of free XWalls");
    }
    memset(&xwall[nXWall], 0, sizeof(XWALL));
    xwall[nXWall].reference = nWall;
    wall[nWall].extra = nXWall;
    return nXWall;
}

void dbDeleteXWall(int nXWall)
{
    dassert(xwall[nXWall].reference >= 0);
    InsertFree(nextXWall, nXWall);
    wall[xwall[nXWall].reference].extra = -1;
    xwall[nXWall].reference = -1;
}

unsigned short dbInsertXSector(int nSector)
{
    int nXSector = nextXSector[0];
    nextXSector[0] = nextXSector[nXSector];
    if (nXSector == 0)
    {
        ThrowError("Out of free XSectors");
    }
    memset(&xsector[nXSector], 0, sizeof(XSECTOR));
    xsector[nXSector].reference = nSector;
    sector[nSector].extra = nXSector;
    return nXSector;
}

void dbDeleteXSector(int nXSector)
{
    dassert(xsector[nXSector].reference >= 0);
    InsertFree(nextXSector, nXSector);
    sector[xsector[nXSector].reference].extra = -1;
    xsector[nXSector].reference = -1;
}

void dbXSpriteClean(void)
{
    for (int i = 0; i < kMaxSprites; i++)
    {
        int nXSprite = sprite[i].extra;
        if (nXSprite == 0)
        {
            sprite[i].extra = -1;
        }
        if (sprite[i].statnum < kMaxStatus && nXSprite > 0)
        {
            dassert(nXSprite < kMaxXSprites);
            if (xsprite[nXSprite].reference != i)
            {
                int nXSprite2 = dbInsertXSprite(i);
                memcpy(&xsprite[nXSprite2], &xsprite[nXSprite], sizeof(XSPRITE));
                xsprite[nXSprite2].reference = i;
            }
        }
    }
    for (int i = 1; i < kMaxXSprites; i++)
    {
        int nSprite = xsprite[i].reference;
        if (nSprite >= 0)
        {
            dassert(nSprite < kMaxSprites);
            if (sprite[nSprite].statnum >= kMaxStatus || sprite[nSprite].extra != i)
            {
                InsertFree(nextXSprite, i);
                xsprite[i].reference = -1;
            }
        }
    }
}

void dbXWallClean(void)
{
    for (int i = 0; i < numwalls; i++)
    {
        int nXWall = wall[i].extra;
        if (nXWall == 0)
        {
            wall[i].extra = -1;
        }
        if (nXWall > 0)
        {
            dassert(nXWall < kMaxXWalls);
            if (xwall[nXWall].reference == -1)
            {
                wall[i].extra = -1;
            }
            else
            {
                xwall[nXWall].reference = i;
            }
        }
    }
    for (int i = 0; i < numwalls; i++)
    {
        int nXWall = wall[i].extra;
        if (nXWall > 0)
        {
            dassert(nXWall < kMaxXWalls);
            if (xwall[nXWall].reference != i)
            {
                int nXWall2 = dbInsertXWall(i);
                memcpy(&xwall[nXWall2], &xwall[nXWall], sizeof(XWALL));
                xwall[nXWall2].reference = i;
            }
        }
    }
    for (int i = 1; i < kMaxXWalls; i++)
    {
        int nWall = xwall[i].reference;
        if (nWall >= 0)
        {
            dassert(nWall < kMaxWalls);
            if (nWall >= numwalls || wall[nWall].extra != i)
            {
                InsertFree(nextXWall, i);
                xwall[i].reference = -1;
            }
        }
    }
}

void dbXSectorClean(void)
{
    for (int i = 0; i < numsectors; i++)
    {
        int nXSector = sector[i].extra;
        if (nXSector == 0)
        {
            sector[i].extra = -1;
        }
        if (nXSector > 0)
        {
            dassert(nXSector < kMaxXSectors);
            if (xsector[nXSector].reference == -1)
            {
                sector[i].extra = -1;
            }
            else
            {
                xsector[nXSector].reference = i;
            }
        }
    }
    for (int i = 0; i < numsectors; i++)
    {
        int nXSector = sector[i].extra;
        if (nXSector > 0)
        {
            dassert(nXSector < kMaxXSectors);
            if (xsector[nXSector].reference != i)
            {
                int nXSector2 = dbInsertXSector(i);
                memcpy(&xsector[nXSector2], &xsector[nXSector], sizeof(XSECTOR));
                xsector[nXSector2].reference = i;
            }
        }
    }
    for (int i = 1; i < kMaxXSectors; i++)
    {
        int nSector = xsector[i].reference;
        if (nSector >= 0)
        {
            dassert(nSector < kMaxSectors);
            if (nSector >= numsectors || sector[nSector].extra != i)
            {
                InsertFree(nextXSector, i);
                xsector[i].reference = -1;
            }
        }
    }
}

void dbInit(void)
{
    InitFreeList(nextXSprite, kMaxXSprites);
    for (int i = 1; i < kMaxXSprites; i++)
    {
        xsprite[i].reference = -1;
    }
    InitFreeList(nextXWall, kMaxXWalls);
    for (int i = 1; i < kMaxXWalls; i++)
    {
        xwall[i].reference = -1;
    }
    InitFreeList(nextXSector, kMaxXSectors);
    for (int i = 1; i < kMaxXSectors; i++)
    {
        xsector[i].reference = -1;
    }
    initspritelists();
    for (int i = 0; i < kMaxSprites; i++)
    {
        sprite[i].cstat = 128;
    }
}

void PropagateMarkerReferences(void)
{
    int nSprite = headspritestat[10];
    while (nSprite != -1)
    {
        int nNextSprite = nextspritestat[nSprite];
        switch (sprite[nSprite].type)
        {
        case 8:
        {
            int nOwner = sprite[nSprite].owner;
            if (nOwner >= 0 && nOwner < numsectors)
            {
                int nXSector = sector[nOwner].extra;
                if (nXSector > 0 && nXSector < kMaxXSectors)
                {
                    xsector[nXSector].at2c_0 = nSprite;
                    nSprite = nNextSprite;
                    continue;
                }
            }
            break;
        }
        case 3:
        {
            int nOwner = sprite[nSprite].owner;
            if (nOwner >= 0 && nOwner < numsectors)
            {
                int nXSector = sector[nOwner].extra;
                if (nXSector > 0 && nXSector < kMaxXSectors)
                {
                    xsector[nXSector].at2c_0 = nSprite;
                    nSprite = nNextSprite;
                    continue;
                }
            }
            break;
        }
        case 4:
        {
            int nOwner = sprite[nSprite].owner;
            if (nOwner >= 0 && nOwner < numsectors)
            {
                int nXSector = sector[nOwner].extra;
                if (nXSector > 0 && nXSector < kMaxXSectors)
                {
                    xsector[nXSector].at2e_0 = nSprite;
                    nSprite = nNextSprite;
                    continue;
                }
            }
            break;
        }
        case 5:
        {
            int nOwner = sprite[nSprite].owner;
            if (nOwner >= 0 && nOwner < numsectors)
            {
                int nXSector = sector[nOwner].extra;
                if (nXSector > 0 && nXSector < kMaxXSectors)
                {
                    xsector[nXSector].at2e_0 = nSprite;
                    nSprite = nNextSprite;
                    continue;
                }
            }
            break;
        }
        }
        DeleteSprite(nSprite);
        nSprite = nNextSprite;
    }
}

bool byte_1A76C6, byte_1A76C7, byte_1A76C8;

MAPHEADER2 byte_19AE44;

unsigned int dbReadMapCRC(const char *pPath)
{
    byte_1A76C7 = 0;
    byte_1A76C8 = 0;
    DICTNODE *pNode = gSysRes.Lookup(pPath, "MAP");
    if (!pNode)
    {
        ThrowError("Error opening map file %s", pPath);
    }
    char *pData = (char*)gSysRes.Lock(pNode);
    int nSize = pNode->size;
    MAPSIGNATURE header;
    IOBuffer(nSize, pData).Read(&header, 6);
    if (memcmp(header.signature, "BLM\x1a", 4))
    {
        ThrowError("Map file corrupted");
    }
    if ((header.version & 0xff00) == 0x600)
    {
    }
    else if ((header.version & 0xff00) == 0x700)
    {
        byte_1A76C8 = 1;
    }
    else
    {
        ThrowError("Map file is wrong version");
    }
    unsigned int nCRC = *(unsigned int*)(pData+nSize-4);
    gSysRes.Unlock(pNode);
    return nCRC;
}

int gMapRev, gSongId, gSkyCount;
//char byte_19AE44[128];

void dbLoadMap(const char *pPath, int *pX, int *pY, int *pZ, short *pAngle, short *pSector, unsigned int *pCRC)
{
    int16_t tpskyoff[256];
    memset(show2dsector, 0, sizeof(show2dsector));
    memset(show2dwall, 0, sizeof(show2dwall));
    memset(show2dsprite, 0, sizeof(show2dsprite));
#ifdef USE_OPENGL
    Polymost_prepare_loadboard();
#endif
    DICTNODE *pNode = gSysRes.Lookup(pPath, "MAP");
    if (!pNode)
    {
        ThrowError("Error opening map file %s", pPath);
    }
    char *pData = (char*)gSysRes.Lock(pNode);
    int nSize = pNode->size;
    MAPSIGNATURE header;
    IOBuffer IOBuffer1 = IOBuffer(nSize, pData);
    IOBuffer1.Read(&header, 6);
    if (memcmp(header.signature, "BLM\x1a", 4))
    {
        ThrowError("Map file corrupted");
    }
    byte_1A76C8 = 0;
    if ((header.version & 0xff00) == 0x600)
    {
    }
    else if ((header.version & 0xff00) == 0x700)
    {
        byte_1A76C8 = 1;
    }
    else
    {
        ThrowError("Map file is wrong version");
    }
    MAPHEADER mapHeader;
    IOBuffer1.Read(&mapHeader,37/* sizeof(mapHeader)*/);
    if (mapHeader.at16 != 0 && mapHeader.at16 != 0x7474614d && mapHeader.at16 != 0x4d617474)
    {
        dbCrypt((char*)&mapHeader, sizeof(mapHeader), 0x7474614d);
        byte_1A76C7 = 1;
    }

    psky_t *pSky = tileSetupSky(0);
    pSky->horizfrac = 65536;

    *pX = mapHeader.at0;
    *pY = mapHeader.at4;
    *pZ = mapHeader.at8;
    *pAngle = mapHeader.atc;
    *pSector = mapHeader.ate;
    pSky->lognumtiles = mapHeader.at10;
    gVisibility = g_visibility = mapHeader.at12;
    gSongId = mapHeader.at16;
    if (byte_1A76C8)
    {
        if (mapHeader.at16 == 0x7474614d || mapHeader.at16 == 0x4d617474)
        {
            byte_1A76C6 = 1;
        }
        else if (!mapHeader.at16)
        {
            byte_1A76C6 = 0;
        }
        else
        {
            ThrowError("Corrupted Map file");
        }
    }
    else if (mapHeader.at16)
    {
        ThrowError("Corrupted Map file");
    }
    parallaxtype = mapHeader.at1a;
    gMapRev = mapHeader.at1b;
    numsectors = mapHeader.at1f;
    numwalls = mapHeader.at21;
    dbInit();
    if (byte_1A76C8)
    {
        IOBuffer1.Read(&byte_19AE44, 128);
        dbCrypt((char*)&byte_19AE44, 128, numwalls);
    }
    else
    {
        memset(&byte_19AE44, 0, 128);
    }
    gSkyCount = 1<<pSky->lognumtiles;
    IOBuffer1.Read(tpskyoff, gSkyCount*sizeof(tpskyoff[0]));
    if (byte_1A76C8)
    {
        dbCrypt((char*)tpskyoff, gSkyCount*sizeof(tpskyoff[0]), gSkyCount*2);
    }
    for (int i = 0; i < ClipHigh(gSkyCount, MAXPSKYTILES); i++)
    {
        pSky->tileofs[i] = tpskyoff[i];
    }
    for (int i = 0; i < numsectors; i++)
    {
        sectortype *pSector = &sector[i];
        IOBuffer1.Read(pSector, sizeof(sectortype));
        if (byte_1A76C8)
        {
            dbCrypt((char*)pSector, sizeof(sectortype), gMapRev*sizeof(sectortype));
        }
        qsector_filler[i] = pSector->fogpal;
        pSector->fogpal = 0;
        if (sector[i].extra > 0)
        {
            const int nXSectorSize = 60;
            char pBuffer[nXSectorSize];
            int nXSector = dbInsertXSector(i);
            XSECTOR *pXSector = &xsector[nXSector];
            memset(pXSector, 0, sizeof(XSECTOR));
            int nCount;
            if (!byte_1A76C8)
            {
                nCount = nXSectorSize;
            }
            else
            {
                nCount = byte_19AE44.at48;
            }
            dassert(nCount <= nXSectorSize);
            IOBuffer1.Read(pBuffer, nCount);
            BitReader bitReader(pBuffer, nCount);
            pXSector->reference = bitReader.readSigned(14);
            pXSector->at1_6 = bitReader.readUnsigned(1);
            pXSector->at1_7 = bitReader.readUnsigned(17);
            pXSector->at4_0 = bitReader.readUnsigned(16);
            pXSector->at6_0 = bitReader.readUnsigned(10);
            pXSector->at7_2 = bitReader.readUnsigned(3);
            pXSector->at7_5 = bitReader.readUnsigned(3);
            pXSector->at8_0 = bitReader.readUnsigned(10);
            pXSector->at9_2 = bitReader.readUnsigned(8);
            pXSector->ata_2 = bitReader.readUnsigned(1);
            pXSector->ata_3 = bitReader.readUnsigned(1);
            pXSector->ata_4 = bitReader.readUnsigned(12);
            pXSector->atc_0 = bitReader.readUnsigned(12);
            pXSector->atd_4 = bitReader.readUnsigned(1);
            pXSector->atd_5 = bitReader.readUnsigned(1);
            pXSector->atd_6 = bitReader.readSigned(8);
            pXSector->ate_6 = bitReader.readUnsigned(8);
            pXSector->atf_6 = bitReader.readUnsigned(1);
            pXSector->atf_7 = bitReader.readUnsigned(1);
            pXSector->at10_0 = bitReader.readUnsigned(8);
            pXSector->at11_0 = bitReader.readUnsigned(4);
            pXSector->at11_4 = bitReader.readUnsigned(1);
            pXSector->at11_5 = bitReader.readUnsigned(1);
            pXSector->at11_6 = bitReader.readUnsigned(1);
            pXSector->at11_7 = bitReader.readUnsigned(1);
            pXSector->at12_0 = bitReader.readSigned(8);
            pXSector->at13_0 = bitReader.readUnsigned(1);
            pXSector->at13_1 = bitReader.readUnsigned(1);
            pXSector->at13_2 = bitReader.readUnsigned(1);
            pXSector->at13_3 = bitReader.readUnsigned(1);
            pXSector->at13_4 = bitReader.readUnsigned(1);
            pXSector->at13_5 = bitReader.readUnsigned(3);
            pXSector->at14_0 = bitReader.readUnsigned(8);
            pXSector->at15_0 = bitReader.readUnsigned(11);
            pXSector->at16_3 = bitReader.readUnsigned(1);
            pXSector->at16_4 = bitReader.readUnsigned(1);
            pXSector->at16_5 = bitReader.readUnsigned(1);
            pXSector->at16_6 = bitReader.readUnsigned(1);
            pXSector->at16_7 = bitReader.readUnsigned(3);
            pXSector->at17_2 = bitReader.readUnsigned(1);
            pXSector->at17_3 = bitReader.readUnsigned(1);
            pXSector->at17_4 = bitReader.readUnsigned(1);
            pXSector->at17_5 = bitReader.readUnsigned(1);
            pXSector->at17_6 = bitReader.readUnsigned(1);
            pXSector->at17_7 = bitReader.readUnsigned(1);
            pXSector->at18_0 = bitReader.readUnsigned(1);
            pXSector->at18_1 = bitReader.readUnsigned(1);
            pXSector->at18_2 = bitReader.readUnsigned(12);
            pXSector->at19_6 = bitReader.readUnsigned(12);
            pXSector->at1b_2 = bitReader.readUnsigned(1);
            pXSector->at1b_3 = bitReader.readUnsigned(1);
            pXSector->at1b_4 = bitReader.readUnsigned(4);
            pXSector->at1c_0 = bitReader.readSigned(32);
            pXSector->at20_0 = bitReader.readSigned(32);
            pXSector->at24_0 = bitReader.readSigned(32);
            pXSector->at28_0 = bitReader.readSigned(32);
            pXSector->at2c_0 = bitReader.readUnsigned(16);
            pXSector->at2e_0 = bitReader.readUnsigned(16);
            pXSector->at30_0 = bitReader.readUnsigned(1);
            pXSector->at30_1 = bitReader.readUnsigned(8);
            pXSector->at31_1 = bitReader.readUnsigned(8);
            pXSector->at32_1 = bitReader.readUnsigned(8);
            pXSector->at33_1 = bitReader.readUnsigned(3);
            pXSector->at33_4 = bitReader.readUnsigned(4);
            pXSector->at34_0 = bitReader.readUnsigned(8);
            pXSector->at35_0 = bitReader.readUnsigned(1);
            pXSector->at35_1 = bitReader.readUnsigned(10);
            pXSector->at36_3 = bitReader.readUnsigned(11);
            pXSector->at37_6 = bitReader.readUnsigned(1);
            pXSector->at37_7 = bitReader.readUnsigned(1);
            pXSector->at38_0 = bitReader.readUnsigned(11);
            pXSector->at39_3 = bitReader.readUnsigned(5);
            pXSector->at3a_0 = bitReader.readSigned(12);
            pXSector->at3b_4 = bitReader.readUnsigned(1);
            pXSector->at3b_5 = bitReader.readUnsigned(1);
            pXSector->at3b_6 = bitReader.readUnsigned(1);
            pXSector->at3b_7 = bitReader.readUnsigned(1);
            xsector[sector[i].extra].reference = i;
            xsector[sector[i].extra].at1_7 = xsector[sector[i].extra].at1_6<<16;
        }
    }
    for (int i = 0; i < numwalls; i++)
    {
        walltype *pWall = &wall[i];
        IOBuffer1.Read(pWall, sizeof(walltype));
        if (byte_1A76C8)
        {
            dbCrypt((char*)pWall, sizeof(walltype), (gMapRev*sizeof(sectortype)) | 0x7474614d);
        }
        if (wall[i].extra > 0)
        {
            const int nXWallSize = 24;
            char pBuffer[nXWallSize];
            int nXWall = dbInsertXWall(i);
            XWALL *pXWall = &xwall[nXWall];
            memset(pXWall, 0, sizeof(XWALL));
            int nCount;
            if (!byte_1A76C8)
            {
                nCount = nXWallSize;
            }
            else
            {
                nCount = byte_19AE44.at44;
            }
            dassert(nCount <= nXWallSize);
            IOBuffer1.Read(pBuffer, nCount);
            BitReader bitReader(pBuffer, nCount);
            pXWall->reference = bitReader.readSigned(14);
            pXWall->at1_6 = bitReader.readUnsigned(1);
            pXWall->at1_7 = bitReader.readUnsigned(17);
            pXWall->at4_0 = bitReader.readSigned(16);
            pXWall->at6_0 = bitReader.readUnsigned(10);
            pXWall->at7_2 = bitReader.readUnsigned(6);
            pXWall->at8_0 = bitReader.readUnsigned(10);
            pXWall->at9_2 = bitReader.readUnsigned(8);
            pXWall->ata_2 = bitReader.readUnsigned(1);
            pXWall->ata_3 = bitReader.readUnsigned(1);
            pXWall->ata_4 = bitReader.readUnsigned(12);
            pXWall->atc_0 = bitReader.readUnsigned(12);
            pXWall->atd_4 = bitReader.readUnsigned(1);
            pXWall->atd_5 = bitReader.readUnsigned(1);
            pXWall->atd_6 = bitReader.readUnsigned(1);
            pXWall->atd_7 = bitReader.readSigned(8);
            pXWall->ate_7 = bitReader.readSigned(8);
            pXWall->atf_7 = bitReader.readUnsigned(1);
            pXWall->at10_0 = bitReader.readUnsigned(1);
            pXWall->at10_1 = bitReader.readUnsigned(1);
            pXWall->at10_2 = bitReader.readUnsigned(3);
            pXWall->at10_5 = bitReader.readUnsigned(1);
            pXWall->at10_6 = bitReader.readUnsigned(1);
            pXWall->at10_7 = bitReader.readUnsigned(1);
            pXWall->at11_0 = bitReader.readUnsigned(2);
            pXWall->at11_2 = bitReader.readUnsigned(8);
            pXWall->at12_2 = bitReader.readUnsigned(8);
            pXWall->at13_2 = bitReader.readUnsigned(1);
            pXWall->at13_3 = bitReader.readUnsigned(1);
            pXWall->at13_4 = bitReader.readUnsigned(4);
            pXWall->at14_0 = bitReader.readUnsigned(32);
            xwall[wall[i].extra].reference = i;
            xwall[wall[i].extra].at1_7 = xwall[wall[i].extra].at1_6 << 16;
        }
    }
    initspritelists();
    for (int i = 0; i < mapHeader.at23; i++)
    {
        RemoveSpriteStat(i);
        spritetype *pSprite = &sprite[i];
        IOBuffer1.Read(pSprite, sizeof(spritetype));
        if (byte_1A76C8)
        {
            dbCrypt((char*)pSprite, sizeof(spritetype), (gMapRev*sizeof(spritetype)) | 0x7474614d);
        }
        InsertSpriteSect(i, sprite[i].sectnum);
        InsertSpriteStat(i, sprite[i].statnum);
        sprite[i].index = i;
        qsprite_filler[i] = pSprite->blend;
        pSprite->blend = 0;
        if (sprite[i].extra > 0)
        {
            const int nXSpriteSize = 56;
            char pBuffer[nXSpriteSize];
            int nXSprite = dbInsertXSprite(i);
            XSPRITE *pXSprite = &xsprite[nXSprite];
            memset(pXSprite, 0, sizeof(XSPRITE));
            int nCount;
            if (!byte_1A76C8)
            {
                nCount = nXSpriteSize;
            }
            else
            {
                nCount = byte_19AE44.at40;
            }
            dassert(nCount <= nXSpriteSize);
            IOBuffer1.Read(pBuffer, nCount);
            BitReader bitReader(pBuffer, nCount);
            pXSprite->reference = bitReader.readSigned(14);
            pXSprite->at1_6 = bitReader.readUnsigned(1);
            pXSprite->at1_7 = bitReader.readUnsigned(17);
            pXSprite->at4_0 = bitReader.readUnsigned(10);
            pXSprite->at5_2 = bitReader.readUnsigned(10);
            pXSprite->at6_4 = bitReader.readUnsigned(8);
            pXSprite->at7_4 = bitReader.readUnsigned(1);
            pXSprite->at7_5 = bitReader.readUnsigned(1);
            pXSprite->at7_6 = bitReader.readUnsigned(2);
            pXSprite->at8_0 = bitReader.readUnsigned(12);
            pXSprite->at9_4 = bitReader.readUnsigned(12);
            pXSprite->atb_0 = bitReader.readUnsigned(1);
            pXSprite->atb_1 = bitReader.readUnsigned(1);
            pXSprite->atb_2 = bitReader.readUnsigned(2);
            pXSprite->atb_4 = bitReader.readUnsigned(2);
            pXSprite->atb_6 = bitReader.readUnsigned(1);
            pXSprite->atb_7 = bitReader.readUnsigned(1);
            pXSprite->atc_0 = bitReader.readUnsigned(8);
            pXSprite->atd_0 = bitReader.readUnsigned(1);
            pXSprite->atd_1 = bitReader.readUnsigned(1);
            pXSprite->atd_2 = bitReader.readUnsigned(1);
            pXSprite->atd_3 = bitReader.readUnsigned(3);
            pXSprite->atd_6 = bitReader.readUnsigned(1);
            pXSprite->atd_7 = bitReader.readUnsigned(1);
            pXSprite->ate_0 = bitReader.readUnsigned(1);
            pXSprite->ate_1 = bitReader.readUnsigned(1);
            pXSprite->ate_2 = bitReader.readUnsigned(1);
            pXSprite->ate_3 = bitReader.readUnsigned(1);
            pXSprite->ate_4 = bitReader.readUnsigned(1);
            pXSprite->ate_5 = bitReader.readUnsigned(2);
            pXSprite->ate_7 = bitReader.readUnsigned(5);
            pXSprite->atf_4 = bitReader.readUnsigned(1);
            pXSprite->atf_5 = bitReader.readUnsigned(1);
            pXSprite->atf_6 = bitReader.readUnsigned(1);
            pXSprite->atf_7 = bitReader.readUnsigned(1);
            pXSprite->at10_0 = bitReader.readSigned(16);
            pXSprite->at12_0 = bitReader.readSigned(16);
            pXSprite->at14_0 = bitReader.readSigned(16);
            pXSprite->at16_0 = bitReader.readUnsigned(11);
            pXSprite->at17_3 = bitReader.readSigned(2);
            pXSprite->at17_5 = bitReader.readUnsigned(1);
            pXSprite->at17_6 = bitReader.readUnsigned(2);
            pXSprite->at18_0 = bitReader.readUnsigned(2);
            pXSprite->at18_2 = bitReader.readUnsigned(16);
            pXSprite->at1a_2 = bitReader.readUnsigned(6);
            pXSprite->at1b_0 = bitReader.readUnsigned(8);
            pXSprite->health = bitReader.readUnsigned(12);
            pXSprite->at1d_4 = bitReader.readUnsigned(1);
            pXSprite->at1d_5 = bitReader.readUnsigned(1);
            pXSprite->at1d_6 = bitReader.readUnsigned(1);
            pXSprite->at1d_7 = bitReader.readUnsigned(1);
            pXSprite->target = bitReader.readSigned(16);
            pXSprite->at20_0 = bitReader.readSigned(32);
            pXSprite->at24_0 = bitReader.readSigned(32);
            pXSprite->at28_0 = bitReader.readSigned(32);
            pXSprite->at2c_0 = bitReader.readUnsigned(16);
            pXSprite->at2e_0 = bitReader.readSigned(16);
            pXSprite->at30_0 = bitReader.readUnsigned(16);
            pXSprite->at32_0 = bitReader.readUnsigned(16);
            pXSprite->at34 = NULL;
            bitReader.skipBits(32);
            xsprite[sprite[i].extra].reference = i;
            xsprite[sprite[i].extra].at1_7 = xsprite[sprite[i].extra].at1_6 << 16;
            if (!byte_1A76C8)
            {
                xsprite[sprite[i].extra].atb_7 |= xsprite[sprite[i].extra].atf_5;
            }
        }
        if ((sprite[i].cstat & 0x30) == 0x30)
        {
            sprite[i].cstat &= ~0x30;
        }
    }
    unsigned int nCRC;
    IOBuffer1.Read(&nCRC, 4);
    if (Bcrc32(pData, nSize-4, 0) != nCRC)
    {
        ThrowError("Map File does not match CRC");
    }
    *pCRC = nCRC;
    gSysRes.Unlock(pNode);
    PropagateMarkerReferences();
    if (byte_1A76C8)
    {
        if (gSongId == 0x7474614d || gSongId == 0x4d617474)
        {
            byte_1A76C6 = 1;
        }
        else if (!gSongId)
        {
            byte_1A76C6 = 0;
        }
        else
        {
            ThrowError("Corrupted Map file");
        }
    }
    else if (gSongId != 0)
    {
        ThrowError("Corrupted Shareware Map file");
    }

#ifdef POLYMER
    if (videoGetRenderMode() == REND_POLYMER)
        polymer_loadboard();
#endif

    if ((header.version & 0xff00) == 0x600)
    {
        switch (header.version&0xff)
        {
        case 0:
            for (int i = 0; i < numsectors; i++)
            {
                sectortype *pSector = &sector[i];
                if (pSector->extra > 0)
                {
                    XSECTOR *pXSector = &xsector[pSector->extra];
                    pXSector->at18_2 = pXSector->ata_4;
                    if (pXSector->ata_4 > 0)
                    {
                        if (!pXSector->atd_4)
                        {
                            pXSector->atf_6 = 1;
                        }
                        else
                        {
                            pXSector->at19_6 = pXSector->ata_4;
                            pXSector->atc_0 = 0;
                            pXSector->atf_7 = 1;
                        }
                    }
                }
            }
            fallthrough__;
        case 1:
            for (int i = 0; i < numsectors; i++)
            {
                sectortype *pSector = &sector[i];
                if (pSector->extra > 0)
                {
                    XSECTOR *pXSector = &xsector[pSector->extra];
                    pXSector->ate_6 >>= 1;
                }
            }
            fallthrough__;
        case 2:
            for (int i = 0; i < kMaxSprites; i++)
            {
            }
            break;
            
        }
    }
}
