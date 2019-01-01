#include "compat.h"
#include "build.h"
#include "pragmas.h"
#include "common_game.h"

#include "blood.h"
#include "db.h"
#include "gameutil.h"
#include "globals.h"
#include "trig.h"

char flicker1[] = {
    0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0,
    1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1
};

char flicker2[] = {
    1, 2, 4, 2, 3, 4, 3, 2, 0, 0, 1, 2, 4, 3, 2, 0,
    2, 1, 0, 1, 0, 2, 3, 4, 3, 2, 1, 1, 2, 0, 0, 1,
    1, 2, 3, 4, 4, 3, 2, 1, 2, 3, 4, 4, 2, 1, 0, 1,
    0, 0, 0, 0, 1, 2, 3, 4, 3, 2, 1, 2, 3, 4, 3, 2
};

char flicker3[] = {
    4, 4, 4, 4, 3, 4, 4, 4, 4, 4, 4, 2, 4, 3, 4, 4,
    4, 4, 2, 1, 3, 3, 3, 4, 3, 4, 4, 4, 4, 4, 2, 4,
    4, 4, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 1, 0, 1,
    0, 1, 0, 1, 0, 2, 3, 4, 4, 4, 4, 4, 4, 4, 3, 4
};

char flicker4[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    4, 0, 0, 3, 0, 1, 0, 1, 0, 4, 4, 4, 4, 4, 2, 0,
    0, 0, 0, 4, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 2, 1, 2, 1, 2, 1, 2, 1, 4, 3, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0, 0, 0 ,0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0, 0, 0 ,0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0, 0, 0 ,0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0, 0, 0 ,0, 0
};

char strobe[] = {
    64, 64, 64, 48, 36, 27, 20, 15, 11, 9, 6, 5, 4, 3, 2, 2,
    1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

int GetWaveValue(int a, int b, int c)
{
    b &= 2047;
    switch (a)
    {
    case 0:
        return c;
    case 1:
        return (b>>10)*c;
    case 2:
        return (klabs(128-(b>>3))*c)>>7;
    case 3:
        return ((b>>3)*c)>>8;
    case 4:
        return ((255-(b>>3))*c)>>8;
    case 5:
        return (c+mulscale30(c,Sin(b)))>>1;
    case 6:
        return flicker1[b>>5]*c;
    case 7:
        return (flicker2[b>>5]*c)>>2;
    case 8:
        return (flicker3[b>>5]*c)>>2;
    case 9:
        return (flicker4[b>>4]*c)>>2;
    case 10:
        return (strobe[b>>5]*c)>>6;
    case 11:
        if (b*4 > 2048)
            return 0;
        return (c-mulscale30(c, Cos(b*4)))>>1;
    }
    return 0;
}

int shadeCount;
short shadeList[512];

void DoSectorLighting(void)
{
    for (int i = 0; i < shadeCount; i++)
    {
        int nXSector = shadeList[i];
        XSECTOR *pXSector = &xsector[nXSector];
        int nSector = pXSector->reference;
        dassert(sector[nSector].extra == nXSector);
        if (pXSector->at12_0)
        {
            int v4 = pXSector->at12_0;
            if (pXSector->at11_5)
            {
                sector[nSector].floorshade -= v4;
                if (pXSector->at18_0)
                {
                    int nTemp = pXSector->at33_4;
                    pXSector->at33_4 = sector[nSector].floorpal;
                    sector[nSector].floorpal = nTemp;
                }
            }
            if (pXSector->at11_6)
            {
                sector[nSector].ceilingshade -= v4;
                if (pXSector->at18_0)
                {
                    int nTemp = pXSector->at1b_4;
                    pXSector->at1b_4 = sector[nSector].ceilingpal;
                    sector[nSector].ceilingpal = nTemp;
                }
            }
            if (pXSector->at11_7)
            {
                int nStartWall = sector[nSector].wallptr;
                int nEndWall = nStartWall + sector[nSector].wallnum;
                for (int j = nStartWall; j < nEndWall; j++)
                {
                    wall[j].shade -= v4;
                    if (pXSector->at18_0)
                    {
                        wall[j].pal = sector[nSector].floorpal;
                    }
                }
            }
            pXSector->at12_0 = 0;
        }
        if (pXSector->at11_4 || pXSector->at1_7)
        {
            int t1 = pXSector->at11_0;
            int t2 = pXSector->atd_6;
            if (!pXSector->at11_4 && pXSector->at1_7)
            {
                t2 = mulscale16(t2, pXSector->at1_7);
            }
            int v4 = GetWaveValue(t1, pXSector->at10_0*8+pXSector->ate_6*gGameClock, t2);
            if (pXSector->at11_5)
            {
                sector[nSector].floorshade = ClipRange(sector[nSector].floorshade+v4, -128, 127);
                if (pXSector->at18_0 && v4 != 0)
                {
                    int nTemp = pXSector->at33_4;
                    pXSector->at33_4 = sector[nSector].floorpal;
                    sector[nSector].floorpal = nTemp;
                }
            }
            if (pXSector->at11_6)
            {
                sector[nSector].ceilingshade = ClipRange(sector[nSector].ceilingshade+v4, -128, 127);
                if (pXSector->at18_0 && v4 != 0)
                {
                    int nTemp = pXSector->at1b_4;
                    pXSector->at1b_4 = sector[nSector].ceilingpal;
                    sector[nSector].ceilingpal = nTemp;
                }
            }
            if (pXSector->at11_7)
            {
                int nStartWall = sector[nSector].wallptr;
                int nEndWall = nStartWall + sector[nSector].wallnum;
                for (int j = nStartWall; j < nEndWall; j++)
                {
                    wall[j].shade = ClipRange(wall[j].shade+v4, -128, 127);
                    if (pXSector->at18_0 && v4 != 0)
                    {
                        wall[j].pal = sector[nSector].floorpal;
                    }
                }
            }
            pXSector->at12_0 = v4;
        }
    }
}

void UndoSectorLighting(void)
{
    for (int i = 0; i < numsectors; i++)
    {
        int nXSprite = sector[i].extra;
        if (nXSprite > 0)
        {
            XSECTOR *pXSector = &xsector[i];
            if (pXSector->at12_0)
            {
                int v4 = pXSector->at12_0;
                if (pXSector->at11_5)
                {
                    sector[i].floorshade -= v4;
                    if (pXSector->at18_0)
                    {
                        int nTemp = pXSector->at33_4;
                        pXSector->at33_4 = sector[i].floorpal;
                        sector[i].floorpal = nTemp;
                    }
                }
                if (pXSector->at11_6)
                {
                    sector[i].ceilingshade -= v4;
                    if (pXSector->at18_0)
                    {
                        int nTemp = pXSector->at1b_4;
                        pXSector->at1b_4 = sector[i].ceilingpal;
                        sector[i].ceilingpal = nTemp;
                    }
                }
                if (pXSector->at11_7)
                {
                    int nStartWall = sector[i].wallptr;
                    int nEndWall = nStartWall + sector[i].wallnum;
                    for (int j = nStartWall; j < nEndWall; j++)
                    {
                        wall[j].shade -= v4;
                        if (pXSector->at18_0)
                        {
                            wall[j].pal = sector[i].floorpal;
                        }
                    }
                }
                pXSector->at12_0 = 0;
            }
        }
    }
}

int panCount;
short panList[kMaxXSectors];
short wallPanList[kMaxXWalls];
int wallPanCount;

void DoSectorPanning(void)
{
    for (int i = 0; i < panCount; i++)
    {
        int nXSector = panList[i];
        XSECTOR *pXSector = &xsector[nXSector];
        int nSector = pXSector->reference;
        dassert(nSector >= 0 && nSector < kMaxSectors);
        SECTOR *pSector = &qsector[nSector];
        dassert(pSector->extra == nXSector);
        if (pXSector->at13_0 || pXSector->at1_7)
        {
            int angle = pXSector->at15_0+1024;
            int speed = pXSector->at14_0<<10;
            if (!pXSector->at13_0 && (pXSector->at1_7&0xffff))
                speed = mulscale16(speed, pXSector->at1_7);

            if (pXSector->at13_1) // Floor
            {
                int nTile = pSector->floorpicnum;
                int px = (pSector->floorxpanning<<8)+pXSector->at32_1;
                int py = (pSector->floorypanning<<8)+pXSector->at34_0;
                if (pSector->floorstat&64)
                    angle -= 512;
                int xBits = (picsiz[nTile]&15)-((pSector->floorstat&8)!=0);
                px += mulscale30(speed<<2, Cos(angle))>>xBits;
                int yBits = (picsiz[nTile]/16)-((pSector->floorstat&8)!=0);
                py -= mulscale30(speed<<2, Sin(angle))>>yBits;
                pSector->floorxpanning = px>>8;
                pSector->floorypanning = py>>8;
                pXSector->at32_1 = px&255;
                pXSector->at34_0 = py&255;
            }
            if (pXSector->at13_2) // Ceiling
            {
                int nTile = pSector->ceilingpicnum;
                int px = (pSector->ceilingxpanning<<8)+pXSector->at30_1;
                int py = (pSector->ceilingypanning<<8)+pXSector->at31_1;
                if (pSector->ceilingstat&64)
                    angle -= 512;
                int xBits = (picsiz[nTile]&15)-((pSector->ceilingstat&8)!=0);
                px += mulscale30(speed<<2, Cos(angle))>>xBits;
                int yBits = (picsiz[nTile]/16)-((pSector->ceilingstat&8)!=0);
                py -= mulscale30(speed<<2, Sin(angle))>>yBits;
                pSector->ceilingxpanning = px>>8;
                pSector->ceilingypanning = py>>8;
                pXSector->at30_1 = px&255;
                pXSector->at31_1 = py&255;
            }
        }
    }
    for (int i = 0; i < wallPanCount; i++)
    {
        int nXWall = wallPanList[i];
        XWALL *pXWall = &xwall[nXWall];
        int nWall = pXWall->reference;
        dassert(wall[nWall].extra == nXWall);
        if (pXWall->atd_6 || pXWall->at1_7)
        {
            int psx = pXWall->atd_7<<10;
            int psy = pXWall->ate_7<<10;
            if (!pXWall->atd_6 && (pXWall->at1_7 & 0xffff))
            {
                psx = mulscale16(psx, pXWall->at1_7);
                psy = mulscale16(psy, pXWall->at1_7);
            }
            int nTile = wall[nWall].picnum;
            int px = (wall[nWall].xpanning<<8)+pXWall->at11_2;
            int py = (wall[nWall].ypanning<<8)+pXWall->at12_2;
            px += (psx<<2)>>((uint8_t)picsiz[nTile]&15);
            py += (psy<<2)>>((uint8_t)picsiz[nTile]/16);
            wall[nWall].xpanning = px>>8;
            wall[nWall].ypanning = py>>8;
            pXWall->at11_2 = px&255;
            pXWall->at12_2 = py&255;
        }
    }
}

void InitSectorFX(void)
{
    shadeCount = 0;
    panCount = 0;
    wallPanCount = 0;
    for (int i = 0; i < numsectors; i++)
    {
        int nXSector = sector[i].extra;
        if (nXSector > 0)
        {
            XSECTOR *pXSector = &xsector[nXSector];
            if (pXSector->atd_6)
                shadeList[shadeCount++] = nXSector;
            if (pXSector->at14_0)
                panList[panCount++] = nXSector;
        }
    }
    for (int i = 0; i < numwalls; i++)
    {
        int nXWall = wall[i].extra;
        if (nXWall > 0)
        {
            XWALL *pXWall = &xwall[nXWall];
            if (pXWall->atd_7 || pXWall->ate_7)
                wallPanList[wallPanCount++] = nXWall;
        }
    }
}

class CSectorListMgr
{
public:
    CSectorListMgr();
    int CreateList(short);
    void AddSector(int, short);
    int GetSectorCount(int);
    short *GetSectorList(int);
private:
    int nLists;
    int nListSize[32];
    int nListStart[32];
    short nSectors[kMaxSectors];
};

CSectorListMgr::CSectorListMgr()
{
    nLists = 0;
}

int CSectorListMgr::CreateList(short nSector)
{
    int nStart = 0;
    if (nLists)
        nStart = nListStart[nLists-1]+nListStart[nLists-1];
    int nList = nLists;
    nListStart[nList] = nStart;
    nListSize[nList] = 1;
    nLists++;
    short *pList = GetSectorList(nList);
    pList[0] = nSector;
    return nList;
}

void CSectorListMgr::AddSector(int nList, short nSector)
{
    for (int i = nLists; i > nList; i--)
    {
        short *pList = GetSectorList(i);
        int nCount = GetSectorCount(i);
        memmove(pList+1,pList,nCount*sizeof(short));
        nListStart[i]++;
    }
    short *pList = GetSectorList(nList);
    int nCount = GetSectorCount(nList);
    pList[nCount] = nSector;
    nListSize[nList]++;
}

int CSectorListMgr::GetSectorCount(int nList)
{
    return nListSize[nList];
}

short * CSectorListMgr::GetSectorList(int nList)
{
    return nSectors+nListStart[nList];
}
