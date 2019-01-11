#include "build.h"
#include "polymer.h"
#include "compat.h"
#include "common_game.h"
#include "crc32.h"

#include "blood.h"
#include "db.h"
#include "iob.h"

unsigned short gStatCount[kMaxStatus + 1];

XSPRITE xsprite[kMaxXSprites];
XSECTOR xsector[kMaxXSectors];
XWALL xwall[kMaxXWalls];

int xvel[kMaxSprites], yvel[kMaxSprites], zvel[kMaxSprites];

long gVisibility;

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
    qsprite[nSprite].sectnum = nSector;
}

void RemoveSpriteSect(int nSprite)
{
    dassert(nSprite >= 0 && nSprite < kMaxSprites);
    int nSector = qsprite[nSprite].sectnum;
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
    qsprite[nSprite].sectnum = -1;
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
    qsprite[nSprite].statnum = nStat;
    gStatCount[nStat]++;
}

void RemoveSpriteStat(int nSprite)
{
    dassert(nSprite >= 0 && nSprite < kMaxSprites);
    int nStat = qsprite[nSprite].statnum;
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
    qsprite[nSprite].statnum = -1;
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
        qsprite[i].sectnum = -1;
        qsprite[i].index = -1;
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
    SPRITE *pSprite = &qsprite[nSprite];
    memset(&qsprite[nSprite], 0, sizeof(SPRITE));
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
    if (qsprite[nSprite].extra > 0)
    {
        dbDeleteXSprite(qsprite[nSprite].extra);
    }
    dassert(qsprite[nSprite].statnum >= 0 && qsprite[nSprite].statnum < kMaxStatus);
    RemoveSpriteStat(nSprite);
    dassert(qsprite[nSprite].sectnum >= 0 && qsprite[nSprite].sectnum < kMaxSectors);
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
    dassert(qsprite[nSprite].sectnum >= 0 && qsprite[nSprite].sectnum < kMaxSectors);
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
    dassert(qsprite[nSprite].statnum >= 0 && qsprite[nSprite].statnum < kMaxStatus);
    dassert(qsprite[nSprite].sectnum >= 0 && qsprite[nSprite].sectnum < kMaxSectors);
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
    xsprite[nXSprite].reference = nSprite;
    qsprite[nSprite].extra = nXSprite;
    return nXSprite;
}

void dbDeleteXSprite(int nXSprite)
{
    dassert(xsprite[nXSprite].reference >= 0);
    dassert(qsprite[xsprite[nXSprite].reference].extra == nXSprite);
    InsertFree(nextXSprite, nXSprite);
    qsprite[xsprite[nXSprite].reference].extra = -1;
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
    qwall[nWall].extra = nXWall;
    return nXWall;
}

void dbDeleteXWall(int nXWall)
{
    dassert(xwall[nXWall].reference >= 0);
    InsertFree(nextXWall, nXWall);
    qwall[xwall[nXWall].reference].extra = -1;
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
    qsector[nSector].extra = nXSector;
    return nXSector;
}

void dbDeleteXSector(int nXSector)
{
    dassert(xsector[nXSector].reference >= 0);
    InsertFree(nextXSector, nXSector);
    qsector[xsector[nXSector].reference].extra = -1;
    xsector[nXSector].reference = -1;
}

void dbXSpriteClean(void)
{
    for (int i = 0; i < kMaxSprites; i++)
    {
        int nXSprite = qsprite[i].extra;
        if (nXSprite == 0)
        {
            qsprite[i].extra = -1;
        }
        if (qsprite[i].statnum < kMaxStatus && nXSprite > 0)
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
            if (qsprite[nSprite].statnum >= kMaxStatus || qsprite[nSprite].extra != i)
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
        int nXWall = qwall[i].extra;
        if (nXWall == 0)
        {
            qwall[i].extra = -1;
        }
        if (nXWall > 0)
        {
            dassert(nXWall < kMaxXWalls);
            if (xwall[nXWall].reference == -1)
            {
                qwall[i].extra = -1;
            }
            else
            {
                xwall[nXWall].reference = i;
            }
        }
    }
    for (int i = 0; i < numwalls; i++)
    {
        int nXWall = qwall[i].extra;
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
            if (nWall >= numwalls || qwall[nWall].extra != i)
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
        int nXSector = qsector[i].extra;
        if (nXSector == 0)
        {
            qsector[i].extra = -1;
        }
        if (nXSector > 0)
        {
            dassert(nXSector < kMaxXSectors);
            if (xsector[nXSector].reference == -1)
            {
                qsector[i].extra = -1;
            }
            else
            {
                xsector[nXSector].reference = i;
            }
        }
    }
    for (int i = 0; i < numsectors; i++)
    {
        int nXSector = qsector[i].extra;
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
            if (nSector >= numsectors || qsector[nSector].extra != i)
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
        qsprite[i].cstat = 128;
    }
}

void PropagateMarkerReferences(void)
{
    int nSprite = headspritestat[19];
    while (nSprite != -1)
    {
        int nNextSprite = nextspritestat[nSprite];
        switch (qsprite[nSprite].type)
        {
        case 8:
        {
            int nOwner = qsprite[nSprite].owner;
            if (nOwner >= 0 && nOwner < numsectors)
            {
                int nXSector = qsector[nOwner].extra;
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
            int nOwner = qsprite[nSprite].owner;
            if (nOwner >= 0 && nOwner < numsectors)
            {
                int nXSector = qsector[nOwner].extra;
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
            int nOwner = qsprite[nSprite].owner;
            if (nOwner >= 0 && nOwner < numsectors)
            {
                int nXSector = qsector[nOwner].extra;
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
            int nOwner = qsprite[nSprite].owner;
            if (nOwner >= 0 && nOwner < numsectors)
            {
                int nXSector = qsector[nOwner].extra;
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

unsigned long dbReadMapCRC(const char *pPath)
{
    //char path1[BMAX_PATH];
    char path2[BMAX_PATH];
    char node[BMAX_PATH], dir[BMAX_PATH], fname[BMAX_PATH], ext[BMAX_PATH];
    byte_1A76C7 = 0;
    byte_1A76C8 = 0;
    _splitpath(pPath, node, dir, fname, ext);
    _makepath(path2, NULL, NULL, fname, NULL);
    DICTNODE *pNode = gSysRes.Lookup(path2, "MAP");
    if (!pNode)
    {
        ThrowError("Error opening map file %s", path2);
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
    unsigned long nCRC = *(unsigned long*)(pData+nSize-4);
    gSysRes.Unlock(pNode);
    return nCRC;
}

int gMapRev, gSongId, gSkyCount;
//char byte_19AE44[128];

void dbLoadMap(const char *pPath, long *pX, long *pY, long *pZ, short *pAngle, short *pSector, unsigned long *pCRC)
{
    char path2[BMAX_PATH];
    char node[BMAX_PATH], dir[BMAX_PATH], fname[BMAX_PATH], ext[BMAX_PATH];
    int16_t tpskyoff[256];
    memset(show2dsector, 0, sizeof(show2dsector));
    memset(show2dwall, 0, sizeof(show2dwall));
    memset(show2dsprite, 0, sizeof(show2dsprite));
#ifdef USE_OPENGL
    Polymost_prepare_loadboard();
#endif
    _splitpath(pPath, node, dir, fname, ext);
    _makepath(path2, NULL, NULL, fname, NULL);
    DICTNODE *pNode = gSysRes.Lookup(path2, "MAP");
    if (!pNode)
    {
        ThrowError("Error opening map file %s", path2);
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
    if (mapHeader.at16 != 0 && mapHeader.at16 != 'ttaM' && mapHeader.at16 != 'Matt')
    {
        dbCrypt((char*)&mapHeader, sizeof(mapHeader), 'ttaM');
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
        if (mapHeader.at16 == 'ttaM' || mapHeader.at16 == 'Matt')
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
        SECTOR *pSector = &qsector[i];
        IOBuffer1.Read(pSector, sizeof(SECTOR));
        if (byte_1A76C8)
        {
            dbCrypt((char*)pSector, sizeof(SECTOR), gMapRev*sizeof(SECTOR));
        }
        qsector_filler[i] = pSector->filler;
        pSector->filler = 0;
        if (qsector[i].extra > 0)
        {
            int nXSector = dbInsertXSector(i);
            XSECTOR *pXSector = &xsector[nXSector];
            memset(pXSector, 0, sizeof(XSECTOR));
            int nCount;
            if (!byte_1A76C8)
            {
                nCount = sizeof(XSECTOR);
            }
            else
            {
                nCount = byte_19AE44.at48;
            }
            IOBuffer1.Read(pXSector, nCount);
            xsector[qsector[i].extra].reference = i;
            xsector[qsector[i].extra].at1_7 = xsector[qsector[i].extra].at1_6<<16;
        }
    }
    for (int i = 0; i < numwalls; i++)
    {
        WALL *pWall = &qwall[i];
        IOBuffer1.Read(pWall, sizeof(WALL));
        if (byte_1A76C8)
        {
            dbCrypt((char*)pWall, sizeof(WALL), (gMapRev*sizeof(SECTOR)) | 'ttaM');
        }
        if (qwall[i].extra > 0)
        {
            int nXWall = dbInsertXWall(i);
            XWALL *pXWall = &xwall[nXWall];
            memset(pXWall, 0, sizeof(XWALL));
            int nCount;
            if (!byte_1A76C8)
            {
                nCount = sizeof(XWALL);
            }
            else
            {
                nCount = byte_19AE44.at44;
            }
            IOBuffer1.Read(pXWall, nCount);
            xwall[qwall[i].extra].reference = i;
            xwall[qwall[i].extra].at1_7 = xwall[qwall[i].extra].at1_6 << 16;
        }
    }
    initspritelists();
    for (int i = 0; i < mapHeader.at23; i++)
    {
        RemoveSpriteStat(i);
        SPRITE *pSprite = &qsprite[i];
        IOBuffer1.Read(pSprite, sizeof(SPRITE));
        if (byte_1A76C8)
        {
            dbCrypt((char*)pSprite, sizeof(SPRITE), (gMapRev*sizeof(SPRITE)) | 'ttaM');
        }
        InsertSpriteSect(i, qsprite[i].sectnum);
        InsertSpriteStat(i, qsprite[i].statnum);
        qsprite[i].index = i;
        qsprite_filler[i] = pSprite->filler;
        pSprite->filler = 0;
        if (qsprite[i].extra > 0)
        {
            int nXSprite = dbInsertXSprite(i);
            XSPRITE *pXSprite = &xsprite[nXSprite];
            memset(pXSprite, 0, sizeof(XSPRITE));
            int nCount;
            if (!byte_1A76C8)
            {
                nCount = sizeof(XSPRITE);
            }
            else
            {
                nCount = byte_19AE44.at40;
            }
            IOBuffer1.Read(pXSprite, nCount);
            xsprite[qsprite[i].extra].reference = i;
            xsprite[qsprite[i].extra].at1_7 = xsprite[qsprite[i].extra].at1_6 << 16;
            if (!byte_1A76C8)
            {
                xsprite[qsprite[i].extra].atb_7 |= xsprite[qsprite[i].extra].atf_5;
            }
        }
        if ((qsprite[i].cstat & 0x30) == 0x30)
        {
            qsprite[i].cstat &= ~0x30;
        }
    }
    unsigned long nCRC;
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
        if (gSongId == 'ttaM' || gSongId == 'Matt')
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

    if (videoGetRenderMode() == REND_POLYMER)
        polymer_loadboard();

    if ((header.version & 0xff00) == 0x603)
    {
        switch (header.version&0xff)
        {
        case 0:
            for (int i = 0; i < numsectors; i++)
            {
                SECTOR *pSector = &qsector[i];
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
        case 1:
            for (int i = 0; i < numsectors; i++)
            {
                SECTOR *pSector = &qsector[i];
                if (pSector->extra > 0)
                {
                    XSECTOR *pXSector = &xsector[pSector->extra];
                    pXSector->ate_6 >>= 1;
                }
            }
        case 2:
            for (int i = 0; i < kMaxSprites; i++)
            {
            }
            break;
            
        }
    }
}
