#include <stdlib.h>
#include <string.h>

#include "compat.h"
#include "a.h"
#include "build.h"
#include "pragmas.h"
#include "mmulti.h"
#include "common_game.h"

#include "aihand.h"
#include "blood.h"
#include "choke.h"
#include "config.h"
#include "db.h"
#include "gamemenu.h"
#include "gameutil.h"
#include "globals.h"
#include "levels.h"
#include "loadsave.h"
#include "map2d.h"
#include "messages.h"
#include "menu.h"
#include "mirrors.h"
#include "network.h"
#include "player.h"
#include "replace.h"
#include "screen.h"
#include "sectorfx.h"
#include "tile.h"
#include "trig.h"
#include "view.h"
#include "warp.h"
#include "weapon.h"

struct VIEW {
	int at0;
	int at4;
	int at8; // bob height
	int atc; // bob width
	int at10;
	int at14;
	int at18; // bob sway y
	int at1c; // bob sway x
	int at20;
	int at24; // horiz
	int at28; // horizoff
	int at2c;
	int at30; // angle
	int at34; // weapon z
	int at38; // view z
	int at3c;
	int at40;
	int at44;
	int at48;
	int at4c;
	long at50; // x
	long at54; // y
	long at58; // z
	long at5c; //xvel
	long at60; //yvel
	long at64; //zvel
	short at68;
	unsigned int at6a;
	char at6e;
	char at6f;
	char at70;
	char at71;
	char at72;
	short at73;
	SPRITEHIT at75;
};

VIEW gPrevView[kMaxPlayers];
VIEWPOS gViewPos;
int gViewIndex;

struct INTERPOLATE {
    void *pointer;
    int value;
    int value2;
    INTERPOLATE_TYPE type;
};

int pcBackground;
int gViewMode = 3;
long gViewSize = 1;

VIEW predict, predictOld;

VIEW predictFifo[256];

int gInterpolate;
int nInterpolations;
char gInterpolateSprite[512];
char gInterpolateWall[1024];
char gInterpolateSector[128];

INTERPOLATE gInterpolation[4096];

int gViewXCenter, gViewYCenter;
int gViewX0, gViewY0, gViewX1, gViewY1;
int gViewX0S, gViewY0S, gViewX1S, gViewY1S;
int xscale, yscale, xstep, ystep;

long gScreenTilt;

CGameMessageMgr gGameMessageMgr;

void RotateYZ(int *pX, int *pY, int *pZ, int ang)
{
	int oY, oZ, angSin, angCos;
	oY = *pY;
	oZ = *pZ;
	angSin = Sin(ang);
	angCos = Cos(ang);
	*pY = dmulscale30r(oY,angCos,oZ,-angSin);
	*pZ = dmulscale30r(oY,angSin,oZ,angCos);
}

void RotateXZ(int *pX, int *pY, int *pZ, int ang)
{
	int oX, oZ, angSin, angCos;
	oX = *pX;
	oZ = *pZ;
    angSin = Sin(ang);
    angCos = Cos(ang);
	*pX = dmulscale30r(oX,angCos,oZ,-angSin);
	*pZ = dmulscale30r(oX,angSin,oZ,angCos);
}

void RotateXY(int *pX, int *pY, int *pZ, int ang)
{
	int oX, oY, angSin, angCos;
	oX = *pX;
	oY = *pY;
    angSin = Sin(ang);
    angCos = Cos(ang);
	*pX = dmulscale30r(oX,angCos,oY,-angSin);
	*pY = dmulscale30r(oX,angSin,oY,angCos);
}

FONT gFont[5];

void FontSet(int id, int tile, int space)
{
	if (id < 0 || id >= 5 || tile < 0 || tile >= kMaxTiles)
		return;

	FONT *pFont = &gFont[id];
	int xSize = 0;
	int ySize = 0;
	pFont->tile = tile;
	for (int i = 0; i < 96; i++)
	{
		if (tilesiz[tile+i].x > xSize)
			xSize = tilesiz[tile+i].x;
		if (tilesiz[tile+i].y > ySize)
			ySize = tilesiz[tile+i].y;
	}
	pFont->xSize = xSize;
	pFont->ySize = ySize;
	pFont->space = space;
}

void viewGetFontInfo(int id, const char *unk1, int *pXSize, int *pYSize)
{
	if (id < 0 || id >= 5)
		return;
	FONT *pFont = &gFont[id];
	if (!unk1)
	{
		if (pXSize)
			*pXSize = pFont->xSize;
		if (pYSize)
			*pYSize = pFont->ySize;
	}
	else
	{
		int width = -pFont->space;
		for (const char *pBuf = unk1; *pBuf != 0; pBuf++)
		{
			int tile = ((*pBuf-32)&127)+pFont->tile;
			if (tilesiz[tile].x != 0 && tilesiz[tile].y != 0)
				width += tilesiz[tile].x+pFont->space;
		}
		if (pXSize)
			*pXSize = width;
		if (pYSize)
			*pYSize = pFont->ySize;
	}
}

void viewUpdatePages(void)
{
	pcBackground = numpages;
}

void viewToggle(int viewMode)
{
	if (viewMode == 3)
		gViewMode = 4;
	else
	{
		gViewMode = 3;
		viewResizeView(gViewSize);
	}
}

void viewInitializePrediction(void)
{
	predict.at30 = gMe->pSprite->ang;
	predict.at20 = gMe->at77;
	predict.at24 = gMe->at7b;
	predict.at28 = gMe->at7f;
	predict.at2c = gMe->at83;
	predict.at6f = gMe->at31c;
	predict.at70 = gMe->at2e;
	predict.at72 = gMe->at87;
	predict.at71 = gMe->atc.buttonFlags.jump;
	predict.at50 = gMe->pSprite->x;
	predict.at54 = gMe->pSprite->y;
	predict.at58 = gMe->pSprite->z;
	predict.at68 = gMe->pSprite->sectnum;
	predict.at73 = gMe->pSprite->hitag;
	predict.at5c = xvel[gMe->pSprite->index];
	predict.at60 = yvel[gMe->pSprite->index];
	predict.at64 = zvel[gMe->pSprite->index];
	predict.at6a = gMe->pXSprite->at30_0;
	predict.at48 = gMe->at2f;
	predict.at4c = gMe->at316;
	predict.at6e = gMe->atc.keyFlags.lookCenter;
	memcpy(&predict.at75,&gSpriteHit[gMe->pSprite->extra],sizeof(SPRITEHIT));
	predict.at0 = gMe->at37;
	predict.at4 = gMe->at3b;
	predict.at8 = gMe->at3f;
	predict.atc = gMe->at43;
	predict.at10 = gMe->at47;
	predict.at14 = gMe->at4b;
	predict.at18 = gMe->at4f;
	predict.at1c = gMe->at53;
	predict.at34 = gMe->at6f-gMe->at67-(12<<8);
	predict.at38 = gMe->at67;
	predict.at3c = gMe->at6b;
	predict.at40 = gMe->at6f;
	predict.at44 = gMe->at73;
	memcpy(&predictOld, &predict, sizeof(VIEW));
}

void viewUpdatePrediction(GINPUT *pInput)
{
	int top, bottom;
	memcpy(&predictOld, &predict, sizeof(VIEW));
	short var_18 = gMe->pSprite->cstat;
    gMe->pSprite->cstat = 0;
    fakePlayerProcess(gMe, pInput);
    fakeActProcessSprites();
    gMe->pSprite->cstat = var_18;
    predictFifo[gPredictTail&255] = predict;
    gPredictTail++;
}

void sub_158B4(PLAYER *pPlayer)
{
    predict.at38 = predict.at58 - gPosture[pPlayer->at5f][predict.at48].at24;
    predict.at40 = predict.at58 - gPosture[pPlayer->at5f][predict.at48].at28;
}

void fakeProcessInput(PLAYER *pPlayer, GINPUT *pInput)
{
    POSTURE *pPosture = &gPosture[pPlayer->at5f][predict.at48];
    predict.at70 = pInput->syncFlags.run;
    predict.at71 = pInput->buttonFlags.jump;
    if (predict.at48 == 1)
    {
        int sinVal = Sin(predict.at30);
        int cosVal = Cos(predict.at30);
        if (pInput->forward)
        {
            int fwd = pInput->forward;
            if (fwd > 0)
                fwd *= pPosture->at0;
            else
                fwd *= pPosture->at8;
            predict.at5c += mulscale30(fwd, cosVal);
            predict.at60 += mulscale30(fwd, sinVal);
        }
        if (pInput->strafe)
        {
            int stf = pInput->strafe*pPosture->at4;
            predict.at5c += mulscale16(stf, sinVal);
            predict.at60 -= mulscale16(stf, cosVal);
        }
    }
    else if (predict.at6a < 0x100)
    {
        int drag = 0x10000;
        if (predict.at6a > 0)
            drag -= divscale16(predict.at6a, 0x100);
        int sinVal = Sin(predict.at30);
        int cosVal = Cos(predict.at30);
        if (pInput->forward)
        {
            int fwd = pInput->forward;
            if (fwd > 0)
                fwd *= pPosture->at0;
            else
                fwd *= pPosture->at8;
            if (predict.at6a)
                fwd = mulscale16(fwd, drag);
            predict.at5c += mulscale30(fwd, cosVal);
            predict.at60 += mulscale30(fwd, sinVal);
        }
        if (pInput->strafe)
        {
            int stf = pInput->strafe*pPosture->at4;
            if (predict.at6a)
                stf = mulscale16(stf, drag);
            predict.at5c += mulscale30(stf, sinVal);
            predict.at60 -= mulscale30(stf, cosVal);
        }
    }
    if (pInput->turn)
    {
        predict.at30 = (predict.at30+pInput->turn*4)&2047;
    }
    if (pInput->keyFlags.spin180 && predict.at4c == 0)
        predict.at4c = -1024;
    if (predict.at4c < 0)
    {
        short step;
        if (predict.at48 == 1)
            step = 64;
        else
            step = 128;

        predict.at4c += step;
        if (predict.at4c >= 0)
            predict.at4c = 0;

        predict.at30 += step;
    }

    if (!predict.at71)
        predict.at6f = 0;

    switch (predict.at48)
    {
    case 1:
        if (predict.at71)
            predict.at64 -= 23301;
        if (predict.at8&2)
            predict.at64 += 23301;
        break;
    case 2:
        if (predict.at8&2)
            predict.at48 = 0;
        break;
    default:
        if (!predict.at6f && predict.at71 && predict.at6a == 0)
        {
            if (packItemActive(pPlayer, 4))
                predict.at64 = -1529173;
            else
                predict.at64 = -764586;
            predict.at6f = 1;
        }
        if (predict.at8 & 2)
            predict.at48 = 2;
        break;
    }
    if (predict.at6e && !pInput->buttonFlags.lookUp && !pInput->buttonFlags.lookUp)
    {
        if (predict.at20 < 0)
            predict.at20 = ClipHigh(predict.at20+4, 0);
        if (predict.at20 > 0)
            predict.at20 = ClipLow(predict.at20-4, 0);
        if (predict.at20 == 0)
            predict.at6e = 0;
    }
    else
    {
        if (pInput->buttonFlags.lookUp)
            predict.at20 = ClipHigh(predict.at20+4, 60);
        if (pInput->buttonFlags.lookUp)
            predict.at20 = ClipLow(predict.at20-4, -60);
    }
    if (pInput->mlook < 0)
        predict.at20 = ClipRange((pInput->mlook+3)>>2, -60, 60);
    else if (pInput->mlook < 0)
        predict.at20 = ClipRange(pInput->mlook>>2, -60, 60);

    if (predict.at20 > 0)
        predict.at24 = mulscale30(120, Sin(predict.at20*8));
    else if (predict.at20 < 0)
        predict.at24 = mulscale30(180, Sin(predict.at20*8));
    else
        predict.at24 = 0;

    int nSector = predict.at68;
    if (predict.at6a < 16 && ((predict.at75.florhit&0xe000) == 0x4000 || (predict.at75.florhit&0xe000) == 0) && (sector[nSector].floorstat&2) != 0)
    {
        int floorZ = getflorzofslope(nSector, predict.at50, predict.at54);
        int newX = predict.at50 + mulscale16(64, Cos(predict.at30));
        int newY = predict.at54 + mulscale16(64, Sin(predict.at30));
        short newSector;
        updatesector(newX, newY, &newSector);
        if (newSector == nSector)
        {
            int newFloorZ = getflorzofslope(newSector, newX, newY);
            predict.at28 = interpolate(predict.at28, (floorZ-newFloorZ)>>3, 0x4000);
        }
    }
    else
    {
        predict.at28 = interpolate(predict.at28, 0, 0x4000);
        if (klabs(predict.at28) < 4)
            predict.at28 = 0;
    }
    predict.at2c = (-predict.at24)<<7;
}

void fakePlayerProcess(PLAYER *pPlayer, GINPUT *pInput)
{
    SPRITE *pSprite = pPlayer->pSprite;
    XSPRITE *pXSprite = pPlayer->pXSprite;
    POSTURE *pPosture = &gPosture[pPlayer->at5f][predict.at48];

    int top, bottom;
    GetSpriteExtents(pSprite, &top, &bottom);

    top += predict.at58-pSprite->z;
    bottom += predict.at58-pSprite->z;

    int floordist = (predict.at58-bottom)/4;
    int ceildist = (predict.at58-top)/4;

    int clipdist = pSprite->clipdist<<2;
    short nSector = predict.at68;
    if (!gNoClip)
    {
        pushmove_old((int32_t*)&predict.at50, (int32_t*)&predict.at54, (int32_t*)&predict.at58, &predict.at68, clipdist, ceildist, floordist, 0x10001);
        if (predict.at68 == -1)
            predict.at68 = nSector;
    }
    fakeProcessInput(pPlayer, pInput);

    int vel = approxDist(predict.at5c, predict.at60);

    predict.at3c += mulscale16(0x7000, predict.at64-predict.at3c);
    int tmp = predict.at5c-pPosture->at24-predict.at38;
    int mult;
    if (tmp > 0) mult = 40960; else mult = 6144;
    predict.at3c += mulscale16(tmp<<8, mult);
    predict.at38 += predict.at38>>8;

    predict.at44 += mulscale16(0x5000,predict.at64-predict.at44);
    tmp = predict.at58-pPosture->at28-predict.at40;
    if (tmp > 0) mult = 32768; else mult = 3072;
    predict.at44 += mulscale16(tmp<<8, mult);
    predict.at40 += predict.at44>>8;

    predict.at0 -= 4;
    if (predict.at0 < 0)
        predict.at0 = 0;

    vel >>= 16;
	if (predict.at48 == 1)
	{
		predict.at4 = (predict.at4+17)&2047;
		predict.at14 = (predict.at14+17)&2047;
		predict.at8 = mulscale30(10*pPosture->at14,Sin(predict.at4));
		predict.atc = mulscale30(predict.at0*pPosture->at18,Sin(predict.at4-256));
		predict.at18 = mulscale30(predict.at0*pPosture->at1c,Sin(predict.at14*2));
		predict.at1c = mulscale30(predict.at0*pPosture->at20,Sin(predict.at14-341));
	}
	else
	{
		if (pXSprite->at30_0 < 256)
		{
			predict.at4 = (predict.at4+(pPosture->atc[predict.at70]<<2))&2047;
			predict.at14 = (predict.at14+(pPosture->atc[predict.at70]<<2)/2)&2047;
			if (predict.at70)
			{
				if (predict.at0 < 60)
                    predict.at0 = ClipHigh(predict.at0 + vel, 60);
			}
			else
			{
				if (predict.at0 < 30)
                    predict.at0 = ClipHigh(predict.at0 + vel, 30);
			}
		}
		predict.at8 = mulscale30(predict.at0*pPosture->at14,Sin(predict.at4));
		predict.atc = mulscale30(predict.at0*pPosture->at18,Sin(predict.at4-256));
		predict.at18 = mulscale30(predict.at0*pPosture->at1c,Sin(predict.at14*2));
		predict.at1c = mulscale30(predict.at0*pPosture->at20,Sin(predict.at14-341));
	}
	if (pXSprite->health != 0)
	{
		predict.at72 = 0;
		if (predict.at48 == 1)
		{
			predict.at72 = 1;
			if (gLowerLink[predict.at68] > 0)
			{
				if (sprite[gLowerLink[predict.at68]].lotag == 14 || sprite[gLowerLink[predict.at68]].lotag == 10)
				{
					int z = getceilzofslope(predict.at68, predict.at50, predict.at54);
					if (z > predict.at38)
						predict.at72 = 0;
				}
			}
		}
	}
}

void fakeMoveDude(SPRITE *pSprite)
{
    PLAYER *pPlayer = NULL;
    int bottom, top;
    if (pSprite->type >= 231 && pSprite->type <= 238)
    {
        pPlayer = &gPlayer[pSprite->type - 231];
    }
    dassert(pSprite->type >= kDudeBase && pSprite->type < kDudeMax);
    GetSpriteExtents(pSprite, &top, &bottom);
	top += predict.at58 - pSprite->z;
	bottom += predict.at58 - pSprite->z;
    int var28 = (bottom-predict.at58)/4;
    int var5c = (predict.at58-top)/4;
    int clipdist = pSprite->clipdist*4;
    int nSector = predict.at68;
    dassert(nSector >= 0 && nSector < kMaxSectors);
    if (predict.at5c != 0 || predict.at60 != 0)
    {
        if (pPlayer && gNoClip)
        {
            int x = predict.at50 + (predict.at5c >> 12);
            int y = predict.at54 + (predict.at60 >> 12);
            predict.at50 = x;
            predict.at54 = y;
            if (!FindSector(x, y, &nSector))
                nSector = predict.at68;
        }
        else
        {
            int cstatbak = pSprite->cstat;
            pSprite->cstat &= ~0x101;
            predict.at75.hit = ClipMove(&predict.at50, &predict.at54, &predict.at58, &nSector, predict.at5c >> 12, predict.at60 >> 12, clipdist, var5c, var28, 0x13001);
            if (nSector == -1)
                nSector = predict.at68;
                    
            if (sector[nSector].lotag >= 612 && sector[nSector].lotag <= 617)
            {
                short nSector2 = nSector;
                pushmove_old((int32_t*)&predict.at50, (int32_t*)&predict.at54, (int32_t*)&predict.at58, &nSector2, clipdist, var5c, var28, 0x10001);
                if (nSector2 != -1)
                    nSector = nSector2;
            }

            dassert(nSector >= 0);

            predict.at68 = nSector;

            pSprite->cstat = cstatbak;
        }
        if ((predict.at75.hit & 0xe000) == 0x8000)
        {
            int nWall = predict.at75.hit&(kMaxWalls-1);
            WALL *pWall = &qwall[nWall];
            if (pWall->nextsector != -1)
            {
                SECTOR *pSector = &qsector[pWall->nextsector];
                if (top >= pSector->ceilingz && bottom <= pSector->ceilingz) { }
                actWallBounceVector(&predict.at5c, &predict.at60, nWall, 0);
            }
        }
    }
    if (predict.at68 != nSector)
    {
        dassert(nSector >= 0 && nSector < kMaxSectors);
        predict.at68 = nSector;
    }
    char var4 = 0, var8 = 0;
    int nXSector = sector[nSector].extra;
    if (nXSector > 0)
    {
        XSECTOR *pXSector = &xsector[nXSector];
        if (pXSector->at13_4)
            var8 = 1;
        if (pXSector->at13_5)
            var4 = 1;
    }
    int link1 = gUpperLink[nSector];
    int link2 = gLowerLink[nSector];
    if (link1 >= 0 && (qsprite[link1].type == 9 || qsprite[link1].type == 13))
        var4 = 1;
    if (link2 >= 0 && (qsprite[link2].type == 10 || qsprite[link1].type == 14))
        var4 = 1;
    if (pPlayer)
        clipdist += 16;

    if (predict.at64)
        predict.at58 += predict.at64 >> 8;

    SPRITE tSprite = *pSprite;
    tSprite.x = predict.at50;
    tSprite.y = predict.at54;
    tSprite.z = predict.at58;
    tSprite.sectnum = predict.at68;
    long var54, var50, var4c, var48;
    GetZRange(&tSprite, &var54, &var50, &var4c, &var48, clipdist, 0x10001);
    GetSpriteExtents(&tSprite, &top, &bottom);
    if (predict.at73 & 2)
    {
        int tmp = 58254;
        if (var4)
        {
            if (var8)
            {
                int z = getceilzofslope(nSector, predict.at50, predict.at54);
                if (z > top)
                {
                    tmp = 58254+(-80099*(bottom-z))/(bottom-top);
                }
            }
            else
            {
                int z = getflorzofslope(nSector, predict.at50, predict.at54);
                if (z < bottom)
                {
                    tmp = 58254+(-80099*(top-z))/(bottom-top);
                }
            }
        }
        else
        {
            if (!var8 && bottom >= var4c)
            {
                tmp = 0;
            }
        }
        if (tmp)
        {
            GetSpriteExtents(&tSprite, &top, &bottom);
            if (bottom >= var4c)
            {
                int var48_bak = var48;
                int var4c_bak = var4c;
                GetZRange(&tSprite, &var54, &var50, &var4c, &var48, pSprite->clipdist<<2, 0x13001);
                if (bottom <= var4c && predict.at58-var4c_bak < var28)
                {
                    var48 = var48_bak;
                    var4c = var4c_bak;
                }
            }
            if (bottom >= var4c)
            {
                predict.at75.florhit = var48;
                predict.at58 += var4c-bottom;
                long var44 = predict.at64-velFloor[predict.at68];
                if (var44 > 0)
                {
                    actFloorBounceVector(&predict.at5c, &predict.at60, &var44, predict.at68, 0);
                    predict.at64 = var44;
                    if (klabs(var44) >= 0x1000)
                    {
                        predict.at73 &= ~4;
                        predict.at64 = velFloor[predict.at68];
                    }
                    else
                        predict.at73 |= 4;
                }
                else if (predict.at64 == 0)
                {
                    predict.at73 &= ~4;
                }
            }
            else
            {
                predict.at75.florhit = 0;
                if (predict.at73 & 2)
                    predict.at73 |= 4;
            }
            if (top <= var54)
            {
                predict.at75.ceilhit = var50;
                predict.at58 += klabs(var54-top);
                if (predict.at64 <= 0)
                {
                    if (predict.at73 & 4)
                    {
                        predict.at64 = mulscale16(-predict.at64,0x2000);
                    }
                }
            }
            else
                predict.at75.ceilhit = 0;

            GetSpriteExtents(&tSprite, &top, &bottom);

            int zDiff = var4c-bottom;
            if (zDiff < 0)
                zDiff = 0;
            predict.at6a = zDiff >> 8;
            if (predict.at5c != 0 || predict.at60 != 0)
            {
                if ((var48 & 0xe000) == 0xc000 && (sprite[var48 & (kMaxSprites-1)].cstat&0x30) == 0)
                {
                    int nSprite = var48 & (kMaxSprites-1);
                    predict.at5c += mulscale(4,predict.at50-sprite[nSprite].x,2);
                    predict.at60 += mulscale(4,predict.at54-sprite[nSprite].y,2);
                }
                else
                {
                    int nSector = pSprite->sectnum;
                    if ((sector[nSector].extra <= 0 || !xsector[sector[nSector].extra].at13_4) && predict.at6a < 0x100)
                    {
                        int drag = gDudeDrag;
                        if (predict.at6a > 0)
                        {
                            drag -= scale(gDudeDrag,predict.at6a,0x100);
                        }
                        predict.at5c -= mulscale16r(predict.at5c, drag);
                        predict.at60 -= mulscale16r(predict.at60, drag);
                        if (approxDist(predict.at5c, predict.at60) < 0x1000)
                        {
                            predict.at5c = 0;
                            predict.at60 = 0;
                        }
                    }
                }
            }
        }
    }
}

void sub_17430(SPRITE *pSprite, int num)
{
    int nSector = predict.at68;
    int xvec = 0;
    int yvec = 0;
    dassert(nSector >= 0 && nSector < kMaxSectors);
    int nXSector = sector[nSector].extra;
    if (nXSector > 0)
    {
        dassert(nXSector < kMaxXSectors);
        XSECTOR *pXSector = &xsector[nXSector];
        if (pXSector->at35_1 && (pXSector->at37_6 || pXSector->at1_7))
        {
            int vel = pXSector->at35_1<<12;
            if (!pXSector->at37_6 && pXSector->at1_7)
            {
                vel = mulscale16(vel, pXSector->at1_7);
            }
            xvec = mulscale30(vel, Cos(pXSector->at36_3));
            yvec = mulscale30(vel, Sin(pXSector->at36_3));
        }
    }
    predict.at5c = interpolate(predict.at5c, xvec, num);
    predict.at60 = interpolate(predict.at60, yvec, num);
    predict.at64 = interpolate(predict.at64, 0, num);
}

void fakeActProcessSprites(void)
{
	SPRITE *pSprite = gMe->pSprite;
	if (pSprite->statnum == 6)
	{
		int nXSprite = pSprite->extra;
		dassert(nXSprite > 0 && nXSprite < kMaxXSprites);
		int nSector = predict.at68;
		int nXSector = sector[nSector].extra;
        XSECTOR *pXSector = NULL;
        if (nXSector > 0)
        {
            dassert(nXSector > 0 && nXSector < kMaxXSectors);
            dassert(xsector[nXSector].reference == nSector);
            XSECTOR *pXSector = &xsector[nXSector];
        }
		if (pXSector)
		{
            int top, bottom;
            GetSpriteExtents(pSprite, &top, &bottom);
			top += predict.at58 - pSprite->z;
			bottom += predict.at58 - pSprite->z;
			if (getflorzofslope(nSector, predict.at50, predict.at54) < bottom)
			{
				int ua = pXSector->at15_0;
				if (pXSector->at13_0 || pXSector->at1_6 || pXSector->at1_7)
				{
					int uv = pXSector->at14_0 << 9;
					if (!pXSector->at13_0 && pXSector->at1_7)
						uv = mulscale16(uv, pXSector->at1_7);
					
					if (sector[nSector].floorstat&0x40)
						ua = (GetWallAngle(sector[nSector].wallptr)+512)&2047;
					predict.at5c += mulscale30(uv,Cos(ua));
					predict.at60 += mulscale30(uv,Sin(ua));
				}
			}
		}
        if (pXSector && pXSector->at13_4)
            sub_17430(pSprite, 5376);
        else
            sub_17430(pSprite, 128);

        if ((predict.at73 & 4) != 0 || predict.at5c != 0 || predict.at60 != 0 || predict.at64 != 0 || velFloor[predict.at68] != 0 || velCeil[predict.at68] != 0)
        {
            fakeMoveDude(pSprite);
        }
	}
}

void viewCorrectPrediction(void)
{
    if (gGameOptions.nGameType == 0) return;
    SPRITE *pSprite = gMe->pSprite;
    VIEW *pView = &predictFifo[(gNetFifoTail-1)&255];
    if (pSprite->ang != pView->at30 || pView->at24 != gMe->at7b || pView->at50 != pSprite->x || pView->at54 != pSprite->y || pView->at58 != pSprite->z)
    {
        viewInitializePrediction();
        predictOld = gPrevView[myconnectindex];
        gPredictTail = gNetFifoTail;
        while (gPredictTail < gNetFifoHead[myconnectindex])
        {
            viewUpdatePrediction(&gFifoInput[gPredictTail&255][myconnectindex]);
        }
    }
}

void viewBackupView(int nPlayer)
{
    PLAYER *pPlayer = &gPlayer[nPlayer];
    VIEW *pView = &gPrevView[nPlayer];
    pView->at30 = pPlayer->pSprite->ang;
    pView->at50 = pPlayer->pSprite->x;
    pView->at54 = pPlayer->pSprite->y;
    pView->at38 = pPlayer->at67;
    pView->at34 = pPlayer->at6f-pPlayer->at67-0xc00;
    pView->at24 = pPlayer->at7b;
    pView->at28 = pPlayer->at7f;
    pView->at2c = pPlayer->at83;
    pView->at8 = pPlayer->at3f;
    pView->atc = pPlayer->at43;
    pView->at18 = pPlayer->at4f;
    pView->at1c = pPlayer->at53;
}

void viewClearInterpolations(void)
{
    nInterpolations = 0;
    memset(gInterpolateSprite, 0, sizeof(gInterpolateSprite));
    memset(gInterpolateWall, 0, sizeof(gInterpolateWall));
    memset(gInterpolateSector, 0, sizeof(gInterpolateSector));
}

void viewAddInterpolation(void *data, INTERPOLATE_TYPE type)
{
    if (nInterpolations == 4096)
        ThrowError("Too many interpolations");
    INTERPOLATE *pInterpolate = &gInterpolation[nInterpolations++];
    pInterpolate->pointer = data;
    pInterpolate->type = type;
    switch (type)
    {
    case INTERPOLATE_TYPE_INT:
        pInterpolate->value = *((int*)data);
        break;
    case INTERPOLATE_TYPE_SHORT:
        pInterpolate->value = *((short*)data);
        break;
    }
}

void CalcInterpolations(void)
{
    int i;
    INTERPOLATE *pInterpolate = gInterpolation;
    for (i = 0; i < nInterpolations; i++, pInterpolate++)
    {
        switch (pInterpolate->type)
        {
        case INTERPOLATE_TYPE_INT:
        {
            pInterpolate->value2 = *((int*)pInterpolate->pointer);
            int newValue = interpolate(pInterpolate->value, *((int*)pInterpolate->pointer), gInterpolate);
            *((int*)pInterpolate->pointer) = newValue;
            break;
        }
        case INTERPOLATE_TYPE_SHORT:
        {
            pInterpolate->value2 = *((short*)pInterpolate->pointer);
            int newValue = interpolate(pInterpolate->value, *((short*)pInterpolate->pointer), gInterpolate);
            *((short*)pInterpolate->pointer) = newValue;
            break;
        }
        }
    }
}

void RestoreInterpolations(void)
{
    int i;
    INTERPOLATE *pInterpolate = gInterpolation;
    for (i = 0; i < nInterpolations; i++, pInterpolate++)
    {
        switch (pInterpolate->type)
        {
        case INTERPOLATE_TYPE_INT:
            *((int*)pInterpolate->pointer) = pInterpolate->value2;
            break;
        case INTERPOLATE_TYPE_SHORT:
            *((short*)pInterpolate->pointer) = pInterpolate->value2;
            break;
        }
    }
}

void viewDrawText(int nFont, const char *pString, int x, int y, int nShade, int nPalette, int position, char shadow)
{
    if (nFont < 0 || nFont >= 5 || !pString) return;
    FONT *pFont = &gFont[nFont];

    if (position)
    {
        const char *s = pString;
        int width = -pFont->space;
        while (*s)
        {
            int nTile = ((*s-' ')&127)+pFont->tile;
            if (tilesiz[nTile].x && tilesiz[nTile].y)
                width += tilesiz[nTile].x+pFont->space;
            s++;
        }
        if (position == 1)
            width >>= 1;
        x -= width;
    }
    const char *s = pString;
    while (*s)
    {
        int nTile = ((*s-' ')&127) + pFont->tile;
        if (tilesiz[nTile].x && tilesiz[nTile].y)
        {
            if (shadow)
            {
                viewDrawSprite((x+1)<<16, (y+1)<<16, 65536, 0, nTile, 127, nPalette, 26, 0, 0, xdim-1, ydim-1);
            }
            viewDrawSprite(x<<16, y<<16, 65536, 0, nTile, nShade, nPalette, 26, 0, 0, xdim-1, ydim-1);
            x += tilesiz[nTile].x+pFont->space;
        }
        s++;
    }
}

void viewTileSprite(int nTile, int nShade, int nPalette, int x1, int y1, int x2, int y2)
{
    Rect rect1 = Rect(x1, y1, x2, y2);
    Rect rect2 = Rect(0, 0, xdim, ydim);
    rect1 &= rect2;

    if (!rect1)
        return;

    dassert(nTile >= 0 && nTile < kMaxTiles);
    int width = tilesiz[nTile].x;
    int height = tilesiz[nTile].y;
    int bx1 = DecBy(rect1.x1+1, width);
    int by1 = DecBy(rect1.y1+1, height);
    int bx2 = IncBy(rect1.x2, width);
    int by2 = IncBy(rect1.y2, height);
    for (int x = bx1; x < bx2; x += width)
        for (int y = by1; y < by2; y += height)
            rotatesprite(x<<16, y<<16, 65536, 0, nTile, nShade, nPalette, 64+16+8, x1, y1, x2-1, y2-1);
}

void InitStatusBar(void)
{
    tileLoadTile(2200);
}
void DrawStatSprite(int nTile, int x, int y, int nShade, int nPalette, unsigned int nStat)
{
    rotatesprite(x<<16, y<<16, 65536, 0, nTile, nShade, nPalette, nStat | 74, 0, 0, xdim-1, ydim-1);
}
void DrawStatMaskedSprite(int nTile, int x, int y, int nShade, int nPalette, unsigned int nStat)
{
    rotatesprite(x<<16, y<<16, 65536, 0, nTile, nShade, nPalette, nStat | 10, 0, 0, xdim-1, ydim-1);
}

void DrawStatNumber(const char *pFormat, int nNumber, int nTile, int x, int y, int nShade, int nPalette)
{
    char tempbuf[80];
    int width = tilesiz[nTile].x+1;
    sprintf(tempbuf, pFormat, nNumber);
    for (unsigned int i = 0; i < strlen(tempbuf); i++, x += width)
    {
        if (tempbuf[i] == ' ') continue;
        DrawStatMaskedSprite(nTile+tempbuf[i]-'0', x, y, nShade, nPalette);
    }
}

void TileHGauge(int nTile, int x, int y, int nMult, int nDiv)
{
    int bx = (tilesiz[nTile].x*nMult)/nDiv+x-160;
    int xdimcorrect = scale(ydim, 4, 3);
    int xscalecorrect = divscale16(xdimcorrect, 320);
    int sbx = (xdim>>1)+mulscale16(bx, xscalecorrect)-1;
    rotatesprite(x<<16, y<<16, 65536, 0, nTile, 0, 0, 90, 0, 0, sbx, ydim-1);
}

int gPackIcons[5] = {
    2569, 2564, 2566, 2568, 2560
};

typedef struct {
    short nTile;
    char xRepeat;
    char yRepeat;
} WEAPONICON;

WEAPONICON gWeaponIcon[] = {
    { -1, 0, 0 },
    { -1, 0, 0 },
    { 524, 32, 32 },
    { 559, 32, 32 },
    { 558, 32, 32 },
    { 526, 32, 32 },
    { 589, 32, 32 },
    { 618, 32, 32 },
    { 539, 32, 32 },
    { 800, 32, 32 },
    { 525, 32, 32 },
};

int dword_14C508;

void viewDrawPack(PLAYER *pPlayer, int x, int y)
{
    int packs[5];
    if (pPlayer->at31d)
    {
        int nPacks = 0;
        int width = 0;
        for (int i = 0; i < 5; i++)
        {
            if (pPlayer->packInfo[i].at1)
            {
                packs[nPacks++] = i;
                width += tilesiz[gPackIcons[i]].x + 1;
            }
        }
        width /= 2;
        x -= width;
        for (int i = 0; i < nPacks; i++)
        {
            int nPack = packs[i];
            DrawStatSprite(2568, x+1, y-8);
            DrawStatSprite(2568, x+1, y-6);
            DrawStatSprite(gPackIcons[nPack], x+1, y+1);
            if (nPack == pPlayer->at321)
                DrawStatMaskedSprite(2559, x+1, y+1);
            int nShade;
            if (pPlayer->packInfo[nPack].at0)
                nShade = 4;
            else
                nShade = 24;
            DrawStatNumber("%3d", pPlayer->packInfo[nPack].at1, 2250, x-4, y-13, nShade, 0);
            x += tilesiz[gPackIcons[nPack]].x + 1;
        }
    }
    if (pPlayer->at31d == dword_14C508)
    {
        viewUpdatePages();
    }
    dword_14C508 = pPlayer->at31d;
}

void DrawPackItemInStatusBar(PLAYER *pPlayer, int x, int y, int x2, int y2)
{
    if (pPlayer->at321 < 0) return;

    DrawStatSprite(gPackIcons[pPlayer->at321], x, y);
    DrawStatNumber("%3d", pPlayer->packInfo[pPlayer->at321].at1, 2250, x2, y2, 0, 0);
}

char gTempStr[128];

void UpdateStatusBar(int arg)
{
    PLAYER *pPlayer = gView;
    XSPRITE *pXSprite = pPlayer->pXSprite;

    int nPalette = 0;

    if (gGameOptions.nGameType == 3)
    {
        if (pPlayer->at2ea & 1)
            nPalette = 7;
        else
            nPalette = 10;
    }

    if (gViewSize < 0) return;

    if (gViewSize <= 1)
    {
        if (pPlayer->at1ba)
            TileHGauge(2260, 124, 175, pPlayer->at1ba, 65536);
        else
            viewDrawPack(pPlayer, 166, 200-tilesiz[2201].y/2);
    }
    if (gViewSize == 1)
    {
        DrawStatSprite(2201, 34, 187, 16, nPalette);
        if (pXSprite->health >= 16 || (gGameClock&16) || pXSprite->health == 0)
        {
            DrawStatNumber("%3d", pXSprite->health>>4, 2190, 8, 183, 0, 0);
        }
        if (pPlayer->atbd && pPlayer->atc7 != -1)
        {
            int num = pPlayer->at181[pPlayer->atc7];
            if (pPlayer->atc7 == 6)
                num /= 10;
            DrawStatNumber("%3d", num, 2240, 42, 183, 0, 0);
        }
        DrawStatSprite(2173, 284, 187, 16, nPalette);
        if (pPlayer->at33e[1])
        {
            TileHGauge(2207, 250, 175, pPlayer->at33e[1], 3200);
            DrawStatNumber("%3d", pPlayer->at33e[1]>>4, 2230, 255, 178, 0, 0);
        }
        if (pPlayer->at33e[0])
        {
            TileHGauge(2209, 250, 183, pPlayer->at33e[0], 3200);
            DrawStatNumber("%3d", pPlayer->at33e[0]>>4, 2230, 255, 186, 0, 0);
        }
        if (pPlayer->at33e[2])
        {
            TileHGauge(2208, 250, 191, pPlayer->at33e[2], 3200);
            DrawStatNumber("%3d", pPlayer->at33e[2]>>4, 2230, 255, 194, 0, 0);
        }
        DrawPackItemInStatusBar(pPlayer, 286, 186, 302, 183);
    }
    else if (gViewSize > 1)
    {
        viewDrawPack(pPlayer, 160, 200-tilesiz[2200].y);
        DrawStatMaskedSprite(2200, 160, 172, 16, nPalette);
        DrawPackItemInStatusBar(pPlayer, 265, 186, 260, 172);
        if (pXSprite->health >= 16 || (gGameClock&16) || pXSprite->health == 0)
        {
            DrawStatNumber("%3d", pXSprite->health>>4, 2190, 86, 183, 0, 0);
        }
        if (pPlayer->atbd && pPlayer->atc7 != -1)
        {
            int num = pPlayer->at181[pPlayer->atc7];
            if (pPlayer->atc7 == 6)
                num /= 10;
            DrawStatNumber("%3d", num, 2240, 216, 183, 0, 0);
        }
        for (int i = 9; i >= 1; i--)
        {
            int x = 135+((i-1)/3)*23;
            int y = 182+((i-1)%3)*6;
            int num = pPlayer->at181[i];
            if (i == 6)
                num /= 10;
            if (i == pPlayer->atc7)
            {
                DrawStatNumber("%3d", num, 2230, x, y, -128, 10);
            }
            else
            {
                DrawStatNumber("%3d", num, 2230, x, y, 32, 10);
            }
        }

        if (pPlayer->atc7 == 10)
        {
            DrawStatNumber("%2d", pPlayer->at181[10], 2230, 291, 194, -128, 10);
        }
        else
        {
            DrawStatNumber("%2d", pPlayer->at181[10], 2230, 291, 194, 32, 10);
        }

        if (pPlayer->atc7 == 11)
        {
            DrawStatNumber("%2d", pPlayer->at181[11], 2230, 309, 194, -128, 10);
        }
        else
        {
            DrawStatNumber("%2d", pPlayer->at181[11], 2230, 309, 194, 32, 10);
        }

        if (pPlayer->at33e[1])
        {
            TileHGauge(2207, 44, 174, pPlayer->at33e[1], 3200);
            DrawStatNumber("%3d", pPlayer->at33e[1]>>4, 2230, 50, 177, 0, 0);
        }
        if (pPlayer->at33e[0])
        {
            TileHGauge(2209, 44, 182, pPlayer->at33e[0], 3200);
            DrawStatNumber("%3d", pPlayer->at33e[0]>>4, 2230, 50, 185, 0, 0);
        }
        if (pPlayer->at33e[2])
        {
            TileHGauge(2208, 44, 190, pPlayer->at33e[2], 3200);
            DrawStatNumber("%3d", pPlayer->at33e[2]>>4, 2230, 50, 193, 0, 0);
        }
        sprintf(gTempStr, "v%s", GetVersionString());
        viewDrawText(3, gTempStr, 20, 191, 32, 0, 1, 0);

        for (int i = 0; i < 6; i++)
        {
            int nTile = 2220+i;
            int x = 73+(i&1)*173;
            int y = 171+(i>>1)*11;
            if (pPlayer->at88[i+1])
                DrawStatSprite(nTile, x, y);
            else
                DrawStatSprite(nTile, x, y, 40, 5);
        }
        DrawStatMaskedSprite(2202, 118, 185, pPlayer->at2e ? 16 : 40);
        DrawStatMaskedSprite(2202, 201, 185, pPlayer->at2e ? 16 : 40);
        if (pPlayer->at1ba)
        {
            TileHGauge(2260, 124, 175, pPlayer->at1ba, 65536);
        }
    }
    if (gGameOptions.nGameType < 1) return;

    if (gGameOptions.nGameType == 3)
    {
        int x = 1, y = 1;
        if (dword_21EFD0[0] == 0 || (gGameClock & 8))
        {
            viewDrawText(0, "BLUE", x, y, -128, 10, 0, 0);
            dword_21EFD0[0] = ClipLow(dword_21EFD0[0]-arg, 0);
            sprintf(gTempStr, "%-3d", dword_21EFB0[0]);
            viewDrawText(0, gTempStr, x, y+10, -128, 10, 0, 0);
        }
        x = 319;
        if (dword_21EFD0[1] == 0 || (gGameClock & 8))
        {
            viewDrawText(0, "RED", x, y, -128, 7, 2, 0);
            dword_21EFD0[1] = ClipLow(dword_21EFD0[1]-arg, 0);
            sprintf(gTempStr, "%3d", dword_21EFB0[1]);
            viewDrawText(0, gTempStr, x, y+10, -128, 7, 2, 0);
        }
        return;
    }
    for (int nRows = (gNetPlayers-1) / 4; nRows >= 0; nRows--)
    {
        for (int nCol = 0; nCol < 4; nCol++)
        {
            DrawStatSprite(2229, 40+nCol*80, 4+nRows*9, 16);
        }
    }
    for (int i = 0, p = connecthead; p >= 0; i++, p = connectpoint2[p])
    {
        int x = 80*(i&3);
        int y = 9*(i/4);
        int col = gPlayer[p].at2ea&3;
        char *name = gProfile[p].name;
        if (gProfile[p].skill == 2)
            sprintf(gTempStr, "%s", name);
        else
            sprintf(gTempStr, "%s [%d]", name, gProfile[p].skill);
        strupr(gTempStr);
        viewDrawText(4, gTempStr, x+4, y+1, -128, 11+col, 0, 0);
        sprintf(gTempStr, "%2d", gPlayer[p].at2c6);
        viewDrawText(4, gTempStr, x+76, y+1, -128, 11+col, 2, 0);
    }
}

int *lensTable;

int gZoom = 1024;

int dword_172CE0[16][3];

void viewInit(void)
{
    initprintf("Initializing status bar\n");
    InitStatusBar();
    FontSet(0, 4096, 0);
    FontSet(1, 4192, 1);
    FontSet(2, 4288, 1);
    FontSet(3, 4384, 1);
    FontSet(4, 4480, 0);

    DICTNODE *hLens = gSysRes.Lookup("LENS", "DAT");
    dassert(hLens != NULL);
    dassert(gSysRes.Size(hLens) == kLensSize * kLensSize * sizeof(int));

    lensTable = (int*)gSysRes.Lock(hLens);
    char *data = tileAllocTile(4077, kLensSize, kLensSize, 0, 0);
    memset(data, 255, kLensSize*kLensSize);
    gGameMessageMgr.SetState(gMessageState);
    gGameMessageMgr.SetCoordinates(1, 1);
    char nFont;
    if (gMessageFont == 0)
        nFont = 3;
    else
        nFont = 0;

    gGameMessageMgr.SetFont(nFont);
    gGameMessageMgr.SetMaxMessages(gMessageCount);
    gGameMessageMgr.SetMessageTime(gMessageTime);

    for (int i = 0; i < 16; i++)
    {
        dword_172CE0[i][0] = mulscale16(wrand(), 2048);
        dword_172CE0[i][1] = mulscale16(wrand(), 2048);
        dword_172CE0[i][2] = mulscale16(wrand(), 2048);
    }
    gViewMap.sub_25C38(0, 0, gZoom, 0, gFollowMap);
}

void viewResizeView(int size)
{
    gViewXCenter = xdim-xdim/2;
    gViewYCenter = ydim-ydim/2;
    xscale = divscale16(xdim, 320);
    yscale = divscale16(ydim, 200);
    xstep = divscale16(320, xdim);
    ystep = divscale16(200, ydim);
    gViewSize = ClipRange(size, 0, 6);
    if (gViewSize == 0 || gViewSize == 1)
    {
        gViewX0 = 0;
        gViewX1 = xdim-1;
        gViewY0 = 0;
        gViewY1 = ydim-1;
        if (gGameOptions.nGameType > 0 && gGameOptions.nGameType < 3)
        {
            gViewY0 = (tilesiz[2229].y*ydim*((gNetPlayers+3)/4))/200;
        }
        gViewX0S = divscale16(gViewX0, xscale);
        gViewY0S = divscale16(gViewY0, yscale);
        gViewX1S = divscale16(gViewX1, xscale);
        gViewY1S = divscale16(gViewY1, yscale);
        videoSetViewableArea(gViewX0, gViewY0, gViewX1, gViewY1);
        gGameMessageMgr.SetCoordinates(gViewX0S+1, gViewY0S+1);
        return;
    }
    gViewX0 = 0;
    gViewY0 = 0;
    gViewX1 = xdim-1;
    gViewY1 = ydim-1-(25*ydim)/200;
    if (gGameOptions.nGameType > 0 && gGameOptions.nGameType < 3)
    {
        gViewY0 = (tilesiz[2229].y*ydim*((gNetPlayers+3)/4))/200;
    }

    int height = gViewY1-gViewY0;
    gViewX0 += mulscale16(xdim*(gViewSize-2),4096);
    gViewX1 -= mulscale16(xdim*(gViewSize-2),4096);
    gViewY0 += mulscale16(height*(gViewSize-2),4096);
    gViewY1 -= mulscale16(height*(gViewSize-2),4096);
    gViewX0S = divscale16(gViewX0, xscale);
    gViewY0S = divscale16(gViewY0, yscale);
    gViewX1S = divscale16(gViewX1, xscale);
    gViewY1S = divscale16(gViewY1, yscale);
    videoSetViewableArea(gViewX0, gViewY0, gViewX1, gViewY1);
    gGameMessageMgr.SetCoordinates(gViewX0S + 1, gViewY0S + 1);
    viewUpdatePages();
}

void UpdateFrame(void)
{
    viewTileSprite(230, 0, 0, 0, 0, xdim, gViewY0-3);
    viewTileSprite(230, 0, 0, 0, gViewY1+4, xdim, ydim);
    viewTileSprite(230, 0, 0, 0, gViewY0-3, gViewX0-3, gViewY1+4);
    viewTileSprite(230, 0, 0, gViewX1+4, gViewY0-3, xdim, gViewY1+4);

    viewTileSprite(230, 20, 0, gViewX0-3, gViewY0-3, gViewX0, gViewY1+1);
    viewTileSprite(230, 20, 0, gViewX0, gViewY0-3, gViewX1+4, gViewY0);
    viewTileSprite(230, 10, 1, gViewX1+1, gViewY0, gViewX1+4, gViewY1+4);
    viewTileSprite(230, 10, 1, gViewX0-3, gViewY1+1, gViewX1+1, gViewY1+4);
}

void viewDrawInterface(int arg)
{
    if (gViewMode == 3 && gViewSize >= 2 && pcBackground != 0)
    {
        UpdateFrame();
        pcBackground--;
    }
    UpdateStatusBar(arg);
}

SPRITE *viewInsertTSprite(int nSector, int nStatnum, SPRITE *pSprite)
{
    int nTSprite = spritesortcnt;
    SPRITE *pTSprite = &qtsprite[nTSprite];
    memset(pTSprite, 0, sizeof(SPRITE));
    pTSprite->cstat = 128;
    pTSprite->xrepeat = 64;
    pTSprite->yrepeat = 64;
    pTSprite->owner = -1;
    pTSprite->extra = -1;
    pTSprite->type = -spritesortcnt;
    pTSprite->statnum = nStatnum;
    pTSprite->sectnum = nSector;
    spritesortcnt++;
    if (pSprite)
    {
        pTSprite->x = pSprite->x;
        pTSprite->y = pSprite->y;
        pTSprite->z = pSprite->z;
        pTSprite->owner = pSprite->owner;
        pTSprite->ang = pSprite->ang;
    }
    return &qtsprite[nTSprite];
}

int effectDetail[] = {
    4, 4, 4, 4, 0, 0, 0, 0, 0, 1, 4, 4, 0, 0, 0, 1, 0, 0, 0
};

SPRITE *viewAddEffect(int nTSprite, VIEW_EFFECT nViewEffect)
{
    dassert(nViewEffect >= 0 && nViewEffect < kViewEffectMax);
    SPRITE *pTSprite = &qtsprite[nTSprite];
    if (gDetail < effectDetail[nViewEffect] || nTSprite >= kMaxViewSprites) return NULL;
    switch (nViewEffect)
    {
    case VIEW_EFFECT_18:
        for (int i = 0; i < 16; i++)
        {
            SPRITE *pNSprite = viewInsertTSprite(pTSprite->sectnum, 32767, pTSprite);
            int ang = (gFrameClock*2048)/120;
            int nRand1 = dword_172CE0[i][0];
            int nRand2 = dword_172CE0[i][1];
            int nRand3 = dword_172CE0[i][2];
            ang += nRand3;
            int x = mulscale30(512, Cos(ang));
            int y = mulscale30(512, Sin(ang));
            int z = 0;
            RotateYZ(&x, &y, &z, nRand1);
            RotateXZ(&x, &y, &z, nRand2);
            pNSprite->x = pTSprite->x + x;
            pNSprite->y = pTSprite->y + y;
            pNSprite->z = pTSprite->z + (z<<4);
            pNSprite->picnum = 1720;
            pNSprite->shade = -128;
        }
        break;
    case VIEW_EFFECT_16:
    case VIEW_EFFECT_17:
    {
        int top, bottom;
        GetSpriteExtents(pTSprite, &top, &bottom);
        SPRITE *pNSprite = viewInsertTSprite(pTSprite->sectnum, 32767, pTSprite);
        pNSprite->shade = -128;
        pNSprite->pal = 0;
        pNSprite->z = top;
        if (nViewEffect == VIEW_EFFECT_16)
            pNSprite->xrepeat = pNSprite->yrepeat = 24;
        else
            pNSprite->xrepeat = pNSprite->yrepeat = 64;
        pNSprite->picnum = 3558;
        return pNSprite;
    }
    case VIEW_EFFECT_15:
    {
        SPRITE *pNSprite = viewInsertTSprite(pTSprite->sectnum, 32767, pTSprite);
        pNSprite->z = pTSprite->z;
        pNSprite->cstat |= 2;
        pNSprite->shade = -128;
        pNSprite->xrepeat = pTSprite->xrepeat;
        pNSprite->yrepeat = pTSprite->yrepeat;
        pNSprite->picnum = 2135;
        break;
    }
    case VIEW_EFFECT_14:
    {
        SPRITE *pNSprite = viewInsertTSprite(pTSprite->sectnum, 32767, pTSprite);
        pNSprite->shade = -128;
        pNSprite->pal = 0;
        pNSprite->xrepeat = pNSprite->yrepeat = 64;
        pNSprite->picnum = 2605;
        return pNSprite;
    }
    case VIEW_EFFECT_13:
    {
        SPRITE *pNSprite = viewInsertTSprite(pTSprite->sectnum, 32767, pTSprite);
        pNSprite->shade = 26;
        pNSprite->pal = 0;
        pNSprite->cstat |= 2;
        pNSprite->xrepeat = pNSprite->yrepeat = 64;
        pNSprite->picnum = 2089;
        break;
    }
    case VIEW_EFFECT_11:
    {
        SPRITE *pNSprite = viewInsertTSprite(pTSprite->sectnum, 32767, pTSprite);
        int top, bottom;
        GetSpriteExtents(pTSprite, &top, &bottom);
        pNSprite->shade = 26;
        pNSprite->pal = 0;
        pNSprite->cstat |= 2;
        pNSprite->xrepeat = pNSprite->yrepeat = 24;
        pNSprite->picnum = 626;
        pNSprite->z = top;
        break;
    }
    case VIEW_EFFECT_10:
    {
        int nAng = pTSprite->ang;
        if (pTSprite->cstat & 16)
        {
            nAng = (nAng+512)&2047;
        }
        else
        {
            nAng = (nAng+1024)&2047;
        }
        for (int i = 0; i < 5 && spritesortcnt < kMaxViewSprites; i++)
        {
            int nSector = pTSprite->sectnum;
            SPRITE *pNSprite = viewInsertTSprite(nSector, 32767, NULL);
            int nLen = 128+(i<<7);
            int x = mulscale30(nLen, Cos(nAng));
            pNSprite->x = pTSprite->x + x;
            int y = mulscale30(nLen, Sin(nAng));
            pNSprite->y = pTSprite->y + y;
            pNSprite->z = pTSprite->z;
            dassert(nSector >= 0 && nSector < kMaxSectors);
            FindSector(pNSprite->x, pNSprite->y, pNSprite->z, &nSector);
            pNSprite->sectnum = nSector;
            pNSprite->owner = pTSprite->owner;
            pNSprite->picnum = pTSprite->picnum;
            pNSprite->cstat |= 2;
            if (i < 2)
                pNSprite->cstat |= 514;
            pNSprite->shade = ClipLow(pTSprite->shade-16, -128);
            pNSprite->xrepeat = pTSprite->xrepeat;
            pNSprite->yrepeat = pTSprite->yrepeat;
            pNSprite->picnum = pTSprite->picnum;
        }
        break;
    }
    case VIEW_EFFECT_8:
    {
        SPRITE *pNSprite = viewInsertTSprite(pTSprite->sectnum, 32767, pTSprite);
        pNSprite->shade = -128;
        pNSprite->z = pTSprite->z;
        pNSprite->picnum = 908;
        pNSprite->statnum = 0;
        pNSprite->xrepeat = pNSprite->yrepeat = (tilesiz[pTSprite->picnum].x*pTSprite->xrepeat)/64;
        break;
    }
    case VIEW_EFFECT_6:
    {
        SPRITE *pNSprite = viewInsertTSprite(pTSprite->sectnum, 32767, pTSprite);
        int top, bottom;
        GetSpriteExtents(pTSprite, &top, &bottom);
        pNSprite->z = top;
        if (IsDudeSprite(pTSprite))
            pNSprite->picnum = 672;
        else
            pNSprite->picnum = 754;
        pNSprite->cstat |= 2;
        pNSprite->shade = 8;
        pNSprite->xrepeat = pTSprite->xrepeat;
        pNSprite->yrepeat = pTSprite->yrepeat;
        break;
    }
    case VIEW_EFFECT_7:
    {
        SPRITE *pNSprite = viewInsertTSprite(pTSprite->sectnum, 32767, pTSprite);
        int top, bottom;
        GetSpriteExtents(pTSprite, &top, &bottom);
        pNSprite->z = bottom;
        if (pTSprite->type >= kDudeBase && pTSprite->type < kDudeMax)
            pNSprite->picnum = 672;
        else
            pNSprite->picnum = 754;
        pNSprite->cstat |= 2;
        pNSprite->shade = 8;
        pNSprite->xrepeat = pTSprite->xrepeat;
        pNSprite->yrepeat = pTSprite->yrepeat;
        break;
    }
    case VIEW_EFFECT_4:
    {
        SPRITE *pNSprite = viewInsertTSprite(pTSprite->sectnum, 32767, pTSprite);
        int top, bottom;
        GetSpriteExtents(pTSprite, &top, &bottom);
        pNSprite->z = top;
        pNSprite->picnum = 2101;
        pNSprite->shade = -128;
        pNSprite->xrepeat = pNSprite->yrepeat = (tilesiz[pTSprite->picnum].x*pTSprite->xrepeat)/32;
        break;
    }
    case VIEW_EFFECT_5:
    {
        SPRITE *pNSprite = viewInsertTSprite(pTSprite->sectnum, 32767, pTSprite);
        int top, bottom;
        GetSpriteExtents(pTSprite, &top, &bottom);
        pNSprite->z = bottom;
        pNSprite->picnum = 2101;
        pNSprite->shade = -128;
        pNSprite->xrepeat = pNSprite->yrepeat = (tilesiz[pTSprite->picnum].x*pTSprite->xrepeat)/32;
        break;
    }
    case VIEW_EFFECT_0:
    {
        SPRITE *pNSprite = viewInsertTSprite(pTSprite->sectnum, 32767, pTSprite);
        pNSprite->z = getflorzofslope(pTSprite->sectnum, pNSprite->x, pNSprite->y);
        pNSprite->shade = 127;
        pNSprite->cstat |= 2;
        pNSprite->xrepeat = pTSprite->xrepeat;
        pNSprite->yrepeat = pTSprite->yrepeat>>2;
        pNSprite->picnum = pTSprite->picnum;
        pNSprite->pal = 5;
        int height = tilesiz[pNSprite->picnum].y;
        int center = height/2+qpicanm[pNSprite->picnum].yoffset;
        pNSprite->z -= (pNSprite->yrepeat<<2)*(height-center);
        if (videoGetRenderMode() >= REND_POLYMOST)
        {
            int nAngle = getangle(pNSprite->x-gView->pSprite->x, pNSprite->y-gView->pSprite->y);
            pNSprite->x += Cos(nAngle)>>25;
            pNSprite->y += Sin(nAngle)>>25;
        }
        break;
    }
    case VIEW_EFFECT_1:
    {
        SPRITE *pNSprite = viewInsertTSprite(pTSprite->sectnum, 32767, pTSprite);
        pNSprite->shade = -128;
        pNSprite->pal = 2;
        pNSprite->cstat |= 2;
        pNSprite->z = pTSprite->z;
        pNSprite->xrepeat = pTSprite->xrepeat;
        pNSprite->yrepeat = pTSprite->yrepeat;
        pNSprite->picnum = 2427;
        break;
    }
    case VIEW_EFFECT_2:
    {
        SPRITE *pNSprite = viewInsertTSprite(pTSprite->sectnum, 32767, pTSprite);
        SECTOR *pSector = &qsector[pTSprite->sectnum];
        pNSprite->x = pTSprite->x;
        pNSprite->y = pTSprite->y;
        pNSprite->z = pSector->ceilingz;
        pNSprite->picnum = 624;
        pNSprite->shade = ((pTSprite->z-pSector->ceilingz)>>8)-64;
        pNSprite->pal = 2;
        pNSprite->xrepeat = pNSprite->yrepeat = 64;
        pNSprite->cstat |= 106;
        pNSprite->ang = pTSprite->ang;
        pNSprite->owner = pTSprite->owner;
        break;
    }
    case VIEW_EFFECT_3:
    {
        SPRITE *pNSprite = viewInsertTSprite(pTSprite->sectnum, 32767, pTSprite);
        SECTOR *pSector = &qsector[pTSprite->sectnum];
        pNSprite->x = pTSprite->x;
        pNSprite->y = pTSprite->y;
        pNSprite->z = pSector->floorz;
        pNSprite->picnum = 624;
        char nShade = (pSector->floorz-pTSprite->z)>>8; 
        pNSprite->shade = nShade-32;
        pNSprite->pal = 2;
        pNSprite->xrepeat = pNSprite->yrepeat = nShade;
        pNSprite->cstat |= 98;
        pNSprite->ang = pTSprite->ang;
        pNSprite->owner = pTSprite->owner;
        break;
    }
    case VIEW_EFFECT_9:
    {
        SPRITE *pNSprite = viewInsertTSprite(pTSprite->sectnum, 32767, pTSprite);
        pNSprite->z = pTSprite->z;
        if (gDetail > 1)
            pNSprite->cstat |= 514;
        pNSprite->shade = ClipLow(pTSprite->shade-32, -128);
        pNSprite->xrepeat = pTSprite->xrepeat;
        pNSprite->yrepeat = 64;
        pNSprite->picnum = 775;
        break;
    }
    case VIEW_EFFECT_12:
    {
        dassert(pTSprite->type >= kDudePlayer1 && pTSprite->type <= kDudePlayer8);
        PLAYER *pPlayer = &gPlayer[pTSprite->extra-kDudePlayer1];
        if (gWeaponIcon[pPlayer->atbd].nTile < 0) break;
        SPRITE *pNSprite = viewInsertTSprite(pTSprite->sectnum, 32767, pTSprite);
        int top, bottom;
        GetSpriteExtents(pTSprite, &top, &bottom);
        pNSprite->x = pTSprite->x;
        pNSprite->y = pTSprite->y;
        pNSprite->z = pTSprite->z-(32<<8);
        pNSprite->picnum = gWeaponIcon[pPlayer->atbd].nTile;
        pNSprite->shade = pTSprite->shade;
        pNSprite->xrepeat = gWeaponIcon[pPlayer->atbd].xRepeat;
        pNSprite->yrepeat = gWeaponIcon[pPlayer->atbd].yRepeat;
        break;
    }
    }
    return NULL;
}

LOCATION gPrevSpriteLoc[kMaxSprites];

void viewProcessSprites(int cX, int cY, int cZ)
{
    dassert(spritesortcnt <= kMaxViewSprites);
    int nViewSprites = spritesortcnt;
    for (int nTSprite = nViewSprites-1; nTSprite >= 0; nTSprite--)
    {
        SPRITE *pTSprite = &qtsprite[nTSprite];
        //int nXSprite = pTSprite->extra;
        int nXSprite = qsprite[pTSprite->owner].extra;
        XSPRITE *pTXSprite = NULL;
        if (qsprite_filler[pTSprite->owner] > gDetail)
        {
            pTSprite->xrepeat = 0;
            continue;
        }
        if (nXSprite > 0)
        {
            pTXSprite = &xsprite[nXSprite];
        }
        int nTile = pTSprite->picnum;
        if (nTile < 0 || nTile >= kMaxTiles)
        {
            continue;
        }

        int nSprite = pTSprite->owner;
        if (gViewInterpolate && TestBitString(gInterpolateSprite, nSprite) && !(pTSprite->hitag&512))
        {
            LOCATION *pPrevLoc = &gPrevSpriteLoc[nSprite];
            pTSprite->x = interpolate(pPrevLoc->x, pTSprite->x, gInterpolate);
            pTSprite->y = interpolate(pPrevLoc->y, pTSprite->y, gInterpolate);
            pTSprite->z = interpolate(pPrevLoc->z, pTSprite->z, gInterpolate);
            pTSprite->ang = pPrevLoc->ang+mulscale16(((pTSprite->ang-pPrevLoc->ang+1024)&2047)-1024, gInterpolate);
        }
        int nAnim = 0;
        switch (qpicanm[nTile].at3_4)
        {
            case 0:
                if (nXSprite > 0)
                {
                    dassert(nXSprite < kMaxXSprites);
                    switch (pTSprite->type)
                    {
                    case 20:
                    case 21:
                        if (xsprite[nXSprite].at1_6)
                        {
                            nAnim = 1;
                        }
                        break;
                    case 22:
                        nAnim = xsprite[nXSprite].at10_0;
                        break;
                    }
                }
                break;
            case 1:
            {
                long dX = cX - pTSprite->x;
                long dY = cY - pTSprite->y;
                RotateVector(&dX, &dY, 128-pTSprite->ang);
                nAnim = GetOctant(dX, dY);
                if (nAnim <= 4)
                {
                    pTSprite->cstat &= ~4;
                }
                else
                {
                    nAnim = 8 - nAnim;
                    pTSprite->cstat |= 4;
                }
                break;
            }
            case 2:
            {
                long dX = cX - pTSprite->x;
                long dY = cY - pTSprite->y;
                RotateVector(&dX, &dY, 128-pTSprite->ang);
                nAnim = GetOctant(dX, dY);
                break;
            }
            case 3:
            {
                int top, bottom;
                GetSpriteExtents(pTSprite, &top, &bottom);
                if (getflorzofslope(pTSprite->sectnum, pTSprite->x, pTSprite->y) > bottom)
                    nAnim = 1;
                break;
            }
            case 6:
            case 7:
            {
                if (gDetail >= 4 && videoGetRenderMode() != REND_POLYMER)
                {
                    if ((pTSprite->hitag&16) == 0)
                    {
                        pTSprite->cstat |= 48;
                        pTSprite->yoffset = qpicanm[pTSprite->picnum].yoffset;
                        pTSprite->picnum = voxelIndex[pTSprite->picnum];
                        if (qpicanm[nTile].at3_4 == 7)
                        {
                            pTSprite->ang = (gGameClock<<3)&2047;
                        }
                    }
                }
                break;
            }
        }
        while (nAnim > 0)
        {
            pTSprite->picnum += qpicanm[pTSprite->picnum].animframes+1;
            nAnim--;
        }
        SECTOR *pSector = &qsector[pTSprite->sectnum];
        XSECTOR *pXSector;
        int nShade = pTSprite->shade;
        if (pSector->extra)
        {
            pXSector = &xsector[pSector->extra];
        }
        else
        {
            pXSector = NULL;
        }
        if ((pSector->ceilingstat&1) && (pSector->floorstat&32768) == 0)
        {
            nShade += tileShade[pSector->ceilingpicnum]+pSector->ceilingshade;
        }
        else
        {
            nShade += tileShade[pSector->floorpicnum]+pSector->floorshade;
        }
        nShade += tileShade[pTSprite->picnum];
        pTSprite->shade = ClipRange(nShade, -128, 127);
        if ((pTSprite->hitag&16) && sprite[pTSprite->owner].owner == 3)
        {
            dassert(pTXSprite != NULL);
            pTSprite->xrepeat = 48;
            pTSprite->yrepeat = 48;
            pTSprite->shade = -128;
            pTSprite->picnum = 2272 + 2*pTXSprite->atb_4;
            pTSprite->cstat &= ~514;
            if (((IsItemSprite(pTSprite) || IsAmmoSprite(pTSprite)) && gGameOptions.nItemSettings == 2)
                || (IsWeaponSprite(pTSprite) && gGameOptions.nWeaponSettings == 3))
            {
                pTSprite->xrepeat = pTSprite->yrepeat = 48;
            }
            else
            {
                pTSprite->xrepeat = pTSprite->yrepeat = 0;
            }
        }
        if (spritesortcnt >= kMaxViewSprites) continue;
        if (pTXSprite && pTXSprite->at2c_0 > 0)
        {
            pTSprite->shade = ClipRange(pTSprite->shade-16-QRandom(8), -128, 127);
        }
        if (pTSprite->hitag&256)
        {
            viewAddEffect(nTSprite, VIEW_EFFECT_6);
        }
        if (pTSprite->hitag&1024)
        {
            pTSprite->cstat |= 4;
        }
        if (pTSprite->hitag&2048)
        {
            pTSprite->cstat |= 8;
        }
        switch (pTSprite->statnum)
        {
        case 0:
        {
            switch (pTSprite->type)
            {
            case 32:
                if (pTXSprite)
                {
                    if (pTXSprite->at1_6 > 0)
                    {
                        pTSprite->shade = -128;
                        viewAddEffect(nTSprite, VIEW_EFFECT_11);
                    }
                    else
                    {
                        pTSprite->shade = -8;
                    }
                }
                else
                {
                    pTSprite->shade = -128;
                    viewAddEffect(nTSprite, VIEW_EFFECT_11);
                }
                break;
            case 30:
                if (pTXSprite)
                {
                    if (pTXSprite->at1_6 > 0)
                    {
                        pTSprite->picnum++;
                        viewAddEffect(nTSprite, VIEW_EFFECT_4);
                    }
                    else
                    {
                        viewAddEffect(nTSprite, VIEW_EFFECT_6);
                    }
                }
                else
                {
                    pTSprite->picnum++;
                    viewAddEffect(nTSprite, VIEW_EFFECT_4);
                }
                break;
            default:
                if (pXSector && pXSector->at18_0)
                {
                    pTSprite->pal = pSector->floorpal;
                }
                break;
            }
            break;
        }
        case 3:
        {
            switch (pTSprite->type)
            {
            case 145:
                if (pTXSprite && pTXSprite->at1_6 > 0 && gGameOptions.nGameType == 3)
                {
                    SPRITE *pNTSprite = viewAddEffect(nTSprite, VIEW_EFFECT_17);
                    if (pNTSprite)
                        pNTSprite->pal = 10;
                }
                break;
            case 146:
                if (pTXSprite && pTXSprite->at1_6 > 0 && gGameOptions.nGameType == 3)
                {
                    SPRITE *pNTSprite = viewAddEffect(nTSprite, VIEW_EFFECT_17);
                    if (pNTSprite)
                        pNTSprite->pal = 7;
                }
                break;
            case 147:
                pTSprite->pal = 10;
                pTSprite->cstat |= 1024;
                break;
            case 148:
                pTSprite->pal = 7;
                pTSprite->cstat |= 1024;
                break;
            default:
                if (pTSprite->type >= 100 && pTSprite->type <= 106)
                    pTSprite->shade = -128;
                if (pXSector && pXSector->at18_0)
                {
                    pTSprite->pal = pSector->floorpal;
                }
                break;
            }
            break;
        }
        case 5:
        {
            switch (pTSprite->type)
            {
            case 302:
                pTSprite->yrepeat = 128;
                pTSprite->cstat |= 32;
                break;
            case 306:
                viewAddEffect(nTSprite, VIEW_EFFECT_15);
                break;
            case 300:
                viewAddEffect(nTSprite, VIEW_EFFECT_10);
                break;
            case 301:
            case 303:
                if (pTSprite->statnum == 14)
                {
                    dassert(pTXSprite != NULL);
                    if (pTXSprite->target == gView->at5b)
                    {
                        pTSprite->xrepeat = 0;
                        break;
                    }
                }
                viewAddEffect(nTSprite, VIEW_EFFECT_1);
                if (pTSprite->type == 301)
                {
                    SECTOR *pSector = &qsector[pTSprite->sectnum];
                    int zDiff = (pTSprite->z-pSector->ceilingz)>>8;
                    if ((pSector->ceilingstat&1) == 0 && zDiff < 64)
                    {
                        viewAddEffect(nTSprite, VIEW_EFFECT_2);
                    }
                    zDiff = (pSector->floorz-pTSprite->z)>>8;
                    if ((pSector->floorstat&1) == 0 && zDiff < 64)
                    {
                        viewAddEffect(nTSprite, VIEW_EFFECT_3);
                    }
                }
                break;
            }
            break;
        }
        case 6:
        {
            if (pTSprite->type == 212 && pTXSprite->at34 == &hand13A3B4)
            {
                SPRITE *pTTarget = &qsprite[pTXSprite->target];
                dassert(pTXSprite != NULL && pTTarget != NULL);
                if (IsPlayerSprite(pTTarget))
                {
                    pTSprite->xrepeat = 0;
                    break;
                }
            }
            if (pXSector && pXSector->at18_0)
            {
                pTSprite->pal = pSector->floorpal;
            }
            if (powerupCheck(gView, 25) > 0)
            {
                pTSprite->shade = -128;
            }
            if (IsPlayerSprite(pTSprite))
            {
                PLAYER *pPlayer = &gPlayer[pTSprite->type-kDudePlayer1];
                if (powerupCheck(pPlayer, 13) && !powerupCheck(gView, 25))
                {
                    pTSprite->cstat |= 2;
                    pTSprite->pal = 5;
                }
                else if (powerupCheck(pPlayer, 14))
                {
                    pTSprite->shade = -128;
                    pTSprite->pal = 5;
                }
                else if (powerupCheck(pPlayer, 23))
                {
                    pTSprite->pal = 11+(gView->at2ea&3);
                }
                if (powerupCheck(pPlayer, 24))
                {
                    viewAddEffect(nTSprite, VIEW_EFFECT_13);
                }
                if (gShowWeapon && gGameOptions.nGameType > 0 && gView)
                {
                    viewAddEffect(nTSprite, VIEW_EFFECT_12);
                }
                if (pPlayer->at37b && (gView != pPlayer || gViewPos != VIEWPOS_0))
                {
                    SPRITE *pNTSprite = viewAddEffect(nTSprite, VIEW_EFFECT_14);
                    if (pNTSprite)
                    {
                        POSTURE *pPosture = &gPosture[pPlayer->at5f][pPlayer->at2f];
                        pNTSprite->x += mulscale28(pPosture->at30, Cos(pTSprite->ang));
                        pNTSprite->y += mulscale28(pPosture->at30, Sin(pTSprite->ang));
                        pNTSprite->z = pPlayer->pSprite->z-pPosture->at2c;
                    }
                }
                if (pPlayer->at90 > 0 && gGameOptions.nGameType == 3)
                {
                    if (pPlayer->at90&1)
                    {
                        SPRITE *pNTSprite = viewAddEffect(nTSprite, VIEW_EFFECT_16);
                        if (pNTSprite)
                        {
                            pNTSprite->pal = 10;
                            pNTSprite->cstat |= 4;
                        }
                    }
                    if (pPlayer->at90&2)
                    {
                        SPRITE *pNTSprite = viewAddEffect(nTSprite, VIEW_EFFECT_16);
                        if (pNTSprite)
                        {
                            pNTSprite->pal = 7;
                            pNTSprite->cstat |= 4;
                        }
                    }
                }
            }
            if (pTSprite->owner != gView->pSprite->index || gViewPos != VIEWPOS_0)
            {
                if (getflorzofslope(pTSprite->sectnum, pTSprite->x, pTSprite->y) >= cZ)
                {
                    viewAddEffect(nTSprite, VIEW_EFFECT_0);
                }
            }
            break;
        }
        case 11:
        {
            if (pTSprite->type == 454)
            {
                if (pTXSprite->at1_6)
                {
                    if (pTXSprite->at10_0)
                    {
                        pTSprite->picnum = 772;
                        if (pTXSprite->at12_0)
                        {
                            viewAddEffect(nTSprite, VIEW_EFFECT_9);
                        }
                    }
                }
                else
                {
                    if (pTXSprite->at10_0)
                    {
                        pTSprite->picnum = 773;
                    }
                    else
                    {
                        pTSprite->picnum = 656;
                    }
                }
            }
            break;
        }
        case 4:
        {
            if (pXSector && pXSector->at18_0)
            {
                pTSprite->pal = pSector->floorpal;
            }
            if (pTSprite->hitag&1)
            {
                if (getflorzofslope(pTSprite->sectnum, pTSprite->x, pTSprite->y) >= cZ)
                {
                    if (pTSprite->type < 400 || pTSprite->type >= 433 || !gSpriteHit[nXSprite].florhit)
                    {
                        viewAddEffect(nTSprite, VIEW_EFFECT_0);
                    }
                }
            }
            break;
        }
        }
    }

    for (int nTSprite = spritesortcnt-1; nTSprite >= nViewSprites; nTSprite--)
    {
        SPRITE *pTSprite = &qtsprite[nTSprite];
        int nAnim = 0;
        switch (qpicanm[pTSprite->picnum].at3_4)
        {
            case 1:
            {
                long dX = cX - pTSprite->x;
                long dY = cY - pTSprite->y;
                RotateVector(&dX, &dY, 128-pTSprite->ang);
                nAnim = GetOctant(dX, dY);
                if (nAnim <= 4)
                {
                    pTSprite->cstat &= ~4;
                }
                else
                {
                    nAnim = 8 - nAnim;
                    pTSprite->cstat |= 4;
                }
                break;
            }
            case 2:
            {
                long dX = cX - pTSprite->x;
                long dY = cY - pTSprite->y;
                RotateVector(&dX, &dY, 128-pTSprite->ang);
                nAnim = GetOctant(dX, dY);
                break;
            }
        }
        while (nAnim > 0)
        {
            pTSprite->picnum += qpicanm[pTSprite->picnum].animframes+1;
            nAnim--;
        }
    }
}

long othercameradist = 1280;
long cameradist = -1;
long othercameraclock;
long cameraclock;

void CalcOtherPosition(SPRITE *pSprite, long *pX, long *pY, long *pZ, int *vsectnum, int nAng, int zm)
{
    int vX = mulscale30(-Cos(nAng), 1280);
    int vY = mulscale30(-Sin(nAng), 1280);
    int vZ = mulscale(zm, 1280, 3)-(16<<8);
    int bakCstat = pSprite->cstat;
    pSprite->cstat &= ~256;
    dassert(*vsectnum >= 0 && *vsectnum < kMaxSectors);
    FindSector(*pX, *pY, *pZ, vsectnum);
    short nHSprite, nHWall, nHSector;
    int hX, hY, hZ;
    vec3_t pos = {*pX, *pY, *pZ};
    hitdata_t hitdata;
    hitscan(&pos, *vsectnum, vX, vY, vZ, &hitdata, CLIPMASK1);
    nHSector = hitdata.sect;
    nHWall = hitdata.wall;
    nHSprite = hitdata.sprite;
    hX = hitdata.pos.x;
    hY = hitdata.pos.y;
    hZ = hitdata.pos.z;
    int dX = hX-*pX;
    int dY = hY-*pY;
    if (klabs(vX)+klabs(vY) > klabs(dX)+klabs(dY))
    {
        *vsectnum = nHSector;
        dX -= ksgn(vX)<<6;
        dY -= ksgn(vY)<<6;
        int nDist;
        if (klabs(vX) > klabs(vY))
        {
            nDist = ClipHigh(divscale16(dX,vX), othercameradist);
        }
        else
        {
            nDist = ClipHigh(divscale16(dY,vY), othercameradist);
        }
        othercameradist = nDist;
    }
    *pX += mulscale16(vX, othercameradist);
    *pY += mulscale16(vY, othercameradist);
    *pZ += mulscale16(vZ, othercameradist);
    othercameradist = ClipHigh(othercameradist+((gGameClock-othercameraclock)<<10), 65536);
    othercameraclock = gGameClock;
    dassert(*vsectnum >= 0 && *vsectnum < kMaxSectors);
    FindSector(*pX, *pY, *pZ, vsectnum);
    pSprite->cstat = bakCstat;
}

void CalcPosition(SPRITE *pSprite, long *pX, long *pY, long *pZ, int *vsectnum, int nAng, int zm)
{
    int vX = mulscale30(-Cos(nAng), 1280);
    int vY = mulscale30(-Sin(nAng), 1280);
    int vZ = mulscale(zm, 1280, 3)-(16<<8);
    int bakCstat = pSprite->cstat;
    pSprite->cstat &= ~256;
    dassert(*vsectnum >= 0 && *vsectnum < kMaxSectors);
    FindSector(*pX, *pY, *pZ, vsectnum);
    short nHSprite, nHWall, nHSector;
    int hX, hY, hZ;
    hitscangoal.x = hitscangoal.y = 0x1fffffff;
    vec3_t pos = { *pX, *pY, *pZ };
    hitdata_t hitdata;
    hitscan(&pos, *vsectnum, vX, vY, vZ, &hitdata, CLIPMASK1);
    nHSector = hitdata.sect;
    nHWall = hitdata.wall;
    nHSprite = hitdata.sprite;
    hX = hitdata.pos.x;
    hY = hitdata.pos.y;
    hZ = hitdata.pos.z;
    int dX = hX-*pX;
    int dY = hY-*pY;
    if (klabs(vX)+klabs(vY) > klabs(dX)+klabs(dY))
    {
        *vsectnum = nHSector;
        dX -= ksgn(vX)<<6;
        dY -= ksgn(vY)<<6;
        int nDist;
        if (klabs(vX) > klabs(vY))
        {
            nDist = ClipHigh(divscale16(dX,vX), cameradist);
        }
        else
        {
            nDist = ClipHigh(divscale16(dY,vY), cameradist);
        }
        cameradist = nDist;
    }
    *pX += mulscale16(vX, cameradist);
    *pY += mulscale16(vY, cameradist);
    *pZ += mulscale16(vZ, cameradist);
    cameradist = ClipHigh(cameradist+((gGameClock-cameraclock)<<10), 65536);
    cameraclock = gGameClock;
    dassert(*vsectnum >= 0 && *vsectnum < kMaxSectors);
    FindSector(*pX, *pY, *pZ, vsectnum);
    pSprite->cstat = bakCstat;
}

void viewDrawSprite(long x, long y, long z, int a, int pn, signed char shade, char pal, unsigned short stat, long xd1, long yd1, long xd2, long yd2)
{
    if (stat & 256)
    {
        a = (a+1024)&2047;
        stat ^= 4;
    }
    rotatesprite(x, y, z, a, pn, shade, pal, stat, xd1, yd1, xd2, yd2);
}

struct {
    short nTile;
    unsigned char nStat;
    unsigned char nPal;
    int nScale;
    short nX, nY;
} burnTable[9] = {
     { 2101, 2, 0, 118784, 10, 220 },
     { 2101, 2, 0, 110592, 40, 220 },
     { 2101, 2, 0, 81920, 85, 220 },
     { 2101, 2, 0, 69632, 120, 220 },
     { 2101, 2, 0, 61440, 160, 220 },
     { 2101, 2, 0, 73728, 200, 220 },
     { 2101, 2, 0, 77824, 235, 220 },
     { 2101, 2, 0, 110592, 275, 220 },
     { 2101, 2, 0, 122880, 310, 220 }
};

void viewBurnTime(int gScale)
{
    if (!gScale) return;

    for (int i = 0; i < 9; i++)
    {
        int nTile = burnTable[i].nTile+qanimateoffs(burnTable[i].nTile,32768+i);
        int nScale = burnTable[i].nScale;
        if (gScale < 600)
        {
            nScale = scale(nScale, gScale, 600);
        }
        viewDrawSprite(burnTable[i].nX<<16, burnTable[i].nY<<16, nScale, 0, nTile,
            0, burnTable[i].nPal, burnTable[i].nStat, windowxy1.x, windowxy1.y, windowxy2.x, windowxy2.y);
    }
}

void viewSetMessage(const char *pMessage)
{
    gGameMessageMgr.Add(pMessage, 15);
}

void viewDisplayMessage(void)
{
    gGameMessageMgr.Display();
}

char errMsg[256];

void viewSetErrorMessage(const char *pMessage)
{
    if (!pMessage)
    {
        strcpy(errMsg, "");
    }
    else
    {
        strcpy(errMsg, pMessage);
    }
}

void DoLensEffect(void)
{
    char *d = (char*)waloff[4077];
    dassert(d != NULL);
    char *s = (char*)waloff[4079];
    dassert(s != NULL);
    for (int i = 0; i < kLensSize*kLensSize; i++, d++)
    {
        if (lensTable[i] >= 0)
        {
            *d = *(s+lensTable[i]);
        }
    }
    tileInvalidate(4077, -1, -1);
}

void UpdateDacs(int nPalette)
{
    static RGB newDAC[256];
    static int oldPalette;
    if (oldPalette != nPalette)
    {
        scrSetPalette(nPalette);
        oldPalette = nPalette;
    }

    if (videoGetRenderMode() >= REND_POLYMOST)
    {
        polytint_t *tint = &hictinting[MAXPALOOKUPS-1];
        int nRed = 0;
        int nGreen = 0;
        int nBlue = 0;
        tint->f = 0;
        switch (nPalette)
        {
        case 0:
        default:
            tint->r = 255;
            tint->g = 255;
            tint->b = 255;
            break;
        case 1:
            tint->r = 180;
            tint->g = 190;
            tint->b = 205;
            //nRed += 0;
            //nGreen += 3;
            //nBlue += 9;
            break;
        case 2:
            tint->r = 255;
            tint->g = 218;
            tint->b = 215;
            //nRed += 1;
            //nGreen += 0;
            //nBlue += 0;
            break;
        case 3:
            tint->r = 79;
            tint->g = 78;
            tint->b = 54;
            //nRed += 25;
            //nGreen += 26;
            //nBlue += 0;
            break;
        case 4:
            tint->r = 255;
            tint->g = 255;
            tint->b = 255;
            break;
        }
        nRed += gView->at377;
        nGreen += gView->at377;
        nBlue -= gView->at377;

        nRed += ClipHigh(gView->at366, 85)*2;
        nGreen -= ClipHigh(gView->at366, 85)*3;
        nBlue -= ClipHigh(gView->at366, 85)*3;

        nRed -= gView->at36a;
        nGreen -= gView->at36a;
        nBlue -= gView->at36a;

        nRed -= gView->at36e>>6;
        nGreen -= gView->at36e>>5;
        nBlue -= gView->at36e>>6;

        videoSetPalette(0, nPalette, 8+2);
        videoTintBlood(nRed, nGreen, nBlue);
    }
    else
    {
        for (int i = 0; i < 256; i++)
        {
            int nRed = baseDAC[i].red;
            int nGreen = baseDAC[i].green;
            int nBlue = baseDAC[i].blue;
            nRed += gView->at377;
            nGreen += gView->at377;
            nBlue -= gView->at377;

            nRed += ClipHigh(gView->at366, 85)*2;
            nGreen -= ClipHigh(gView->at366, 85)*3;
            nBlue -= ClipHigh(gView->at366, 85)*3;

            nRed -= gView->at36a;
            nGreen -= gView->at36a;
            nBlue -= gView->at36a;

            nRed -= gView->at36e>>6;
            nGreen -= gView->at36e>>5;
            nBlue -= gView->at36e>>6;

            newDAC[i].red = ClipRange(nRed, 0, 255);
            newDAC[i].green = ClipRange(nGreen, 0, 255);
            newDAC[i].blue = ClipRange(nBlue, 0, 255);
        }
        if (memcmp(newDAC, curDAC, 768) != 0)
        {
            memcpy(curDAC, newDAC, 768);
            gSetDacRange(0, 256, curDAC);
        }
    }
}

bool gPrediction = 0;

char otherMirrorGotpic[2];
char bakMirrorGotpic[2];
// long gVisibility;

int deliriumTilt, deliriumTurn, deliriumPitch;
int gScreenTiltO, deliriumTurnO, deliriumPitchO;

int gShowFrameRate = 1;

void viewUpdateDelirium(void)
{
    gScreenTiltO = gScreenTilt;
    deliriumTurnO = deliriumTurn;
    deliriumPitchO = deliriumPitch;
	int powerCount;
	if (powerCount = powerupCheck(gView,28))
	{
		int tilt1 = 170, tilt2 = 170, pitch = 20;
        int timer = gFrameClock*4;
		if (powerCount < 512)
		{
			int powerScale = (powerCount<<16) / 512;
			tilt1 = mulscale16(tilt1, powerScale);
			tilt2 = mulscale16(tilt2, powerScale);
			pitch = mulscale16(pitch, powerScale);
		}
		int sin2 = costable[(2*timer-512)&2047] / 2;
		int sin3 = costable[(3*timer-512)&2047] / 2;
		gScreenTilt = mulscale30(sin2+sin3,tilt1);
		int sin4 = costable[(4*timer-512)&2047] / 2;
		deliriumTurn = mulscale30(sin3+sin4,tilt2);
		int sin5 = costable[(5*timer-512)&2047] / 2;
		deliriumPitch = mulscale30(sin4+sin5,pitch);
		return;
	}
	gScreenTilt = ((gScreenTilt+1024)&2047)-1024;
	if (gScreenTilt > 0)
	{
		gScreenTilt -= 8;
		if (gScreenTilt < 0)
			gScreenTilt = 0;
	}
	else if (gScreenTilt < 0)
	{
		gScreenTilt += 8;
		if (gScreenTilt >= 0)
			gScreenTilt = 0;
	}
}

void viewDrawScreen(void)
{
    int nPalette = 0;
    static int lastUpdate;

    int delta = ClipLow(gGameClock - lastUpdate, 0);
    lastUpdate = gGameClock;
    if (!gPaused && (!CGameMenuMgr::m_bActive || gGameOptions.nGameType != 0))
    {
        gInterpolate = divscale16(gGameClock-gNetFifoClock+4, 4);
    }
    if (gInterpolate < 0 || gInterpolate > 65536)
    {
        gInterpolate = ClipRange(gInterpolate, 0, 65536);
    }
    if (gViewInterpolate)
    {
        CalcInterpolations();
    }

    if (gViewMode == 3 || gViewMode == 4 || gOverlayMap)
    {
        DoSectorLighting();
    }
    if (gViewMode == 3 || gOverlayMap)
    {
        int yxAspect = yxaspect;
        int viewingRange = viewingrange;
        if (r_usenewaspect)
        {
            newaspect_enable = 1;
            videoSetCorrectedAspect();
        }
        int cX = gView->pSprite->x;
        int cY = gView->pSprite->y;
        int cZ = gView->at67;
        int zDelta = gView->at6f-gView->at67-(12<<8);
        int cA = gView->pSprite->ang;
        int va0 = gView->at7b;
        int v90 = gView->at7f;
        int v74 = gView->at43;
        int v8c = gView->at3f;
        int v4c = gView->at53;
        int v48 = gView->at4f;
        int nSectnum = gView->pSprite->sectnum;
        if (gViewInterpolate)
        {
            if (numplayers > 1 && gView == gMe && gPrediction && gMe->pXSprite->health > 0)
            {
                nSectnum = predict.at68;
                cX = interpolate(predictOld.at50, predict.at50, gInterpolate);
                cY = interpolate(predictOld.at54, predict.at54, gInterpolate);
                cZ = interpolate(predictOld.at38, predict.at38, gInterpolate);
                zDelta = interpolate(predictOld.at34, predict.at34, gInterpolate);
                cA = mulscale16(((predict.at30-predictOld.at30+1024)&2047)-1024, gInterpolate) + predictOld.at30;
                va0 = interpolate(predictOld.at24, predict.at24, gInterpolate);
                v90 = interpolate(predictOld.at28, predict.at28, gInterpolate);
                v74 = interpolate(predictOld.atc, predict.atc, gInterpolate);
                v8c = interpolate(predictOld.at8, predict.at8, gInterpolate);
                v4c = interpolate(predictOld.at1c, predict.at1c, gInterpolate);
                v48 = interpolate(predictOld.at18, predict.at18, gInterpolate);
            }
            else
            {
                VIEW *pView = &gPrevView[gViewIndex];
                cX = interpolate(pView->at50, cX, gInterpolate);
                cY = interpolate(pView->at54, cY, gInterpolate);
                cZ = interpolate(pView->at38, cZ, gInterpolate);
                zDelta = interpolate(pView->at34, zDelta, gInterpolate);
                cA = mulscale16(((cA-pView->at30+1024)&2047)-1024, gInterpolate) + pView->at30;
                va0 = interpolate(pView->at24, va0, gInterpolate);
                v90 = interpolate(pView->at28, v90, gInterpolate);
                v74 = interpolate(pView->atc, v74, gInterpolate);
                v8c = interpolate(pView->at8, v8c, gInterpolate);
                v4c = interpolate(pView->at1c, v4c, gInterpolate);
                v48 = interpolate(pView->at18, v48, gInterpolate);
            }
        }

        if (gView->at35a)
        {
            int nValue = ClipHigh(gView->at35a*8, 2000);
            va0 += QRandom2(nValue>>8);
            cA += QRandom2(nValue>>8);
            cX += QRandom2(nValue>>4);
            cY += QRandom2(nValue>>4);
            cZ += QRandom2(nValue);
            v4c += QRandom2(nValue);
            v48 += QRandom2(nValue);
        }
        if (gView->at37f)
        {
            int nValue = ClipHigh(gView->at37f*8, 2000);
            va0 += QRandom2(nValue>>8);
            cA += QRandom2(nValue>>8);
            cX += QRandom2(nValue>>4);
            cY += QRandom2(nValue>>4);
            cZ += QRandom2(nValue);
            v4c += QRandom2(nValue);
            v48 += QRandom2(nValue);
        }
        va0 += mulscale30(0x40000000-Cos(gView->at35e<<2), 30);
        if (gViewPos == 0)
        {
            if (gViewHBobbing)
            {
                cX -= mulscale30(v74, Sin(cA))>>4;
                cY += mulscale30(v74, Cos(cA))>>4;
            }
            if (gViewVBobbing)
            {
                cZ += v8c;
            }
            if (gSlopeTilting)
            {
                va0 += v90;
            }
            cZ += va0*10;
            cameradist = -1;
            cameraclock = gGameClock;
        }
        else
        {
            CalcPosition(gView->pSprite, (long*)&cX, (long*)&cY, (long*)&cZ, &nSectnum, cA, va0);
        }
        CheckLink((long*)&cX, (long*)&cY, (long*)&cZ, &nSectnum);
        int v78 = interpolateang(gScreenTiltO, gScreenTilt, gInterpolate);
        char v14 = 0;
        char v10 = 0;
        char vc = powerupCheck(gView, 28) > 0;
        char v4 = powerupCheck(gView, 21) > 0;
        renderSetRollAngle(0);
        if (v78 || vc)
        {
            if (videoGetRenderMode() == REND_CLASSIC)
            {
                int vr = viewingrange;
                if (!waloff[4078])
                {
                    tileAllocTile(4078, 640, 640, 0, 0);
                }
                renderSetTarget(4078, 640, 640);
                int nAng = v78&511;
                if (nAng > 256)
                {
                    nAng = 512-nAng;
                }
                renderSetAspect(mulscale16(vr, dmulscale32(Cos(nAng), 256000, Sin(nAng), 160000)), yxaspect);
            }
            else
                renderSetRollAngle(v78);
        }
        else if (v4 && gNetPlayers > 1)
        {
            int tmp = (gGameClock/240)%(gNetPlayers-1);
            int i = connecthead;
            while (1)
            {
                if (i == gViewIndex)
                    i = connectpoint2[i];
                if (tmp == 0)
                    break;
                i = connectpoint2[i];
                tmp--;
            }
            PLAYER *pOther = &gPlayer[i];
            //othercameraclock = gGameClock;
            if (!waloff[4079])
            {
                tileAllocTile(4079, kLensSize, kLensSize, 0, 0);
            }
            renderSetTarget(4079, kLensSize, kLensSize);
            renderSetAspect(65536, 78643);
            long vd8 = pOther->pSprite->x;
            long vd4 = pOther->pSprite->y;
            long vd0 = pOther->at67;
            int vcc = pOther->pSprite->sectnum;
            long v50 = pOther->pSprite->ang;
            long v54 = 0;
            if (pOther->at35a)
            {
                int nValue = ClipHigh(pOther->at35a*8, 2000);
                v54 += QRandom2(nValue>>8);
                v50 += QRandom2(nValue>>8);
                vd8 += QRandom2(nValue>>4);
                vd4 += QRandom2(nValue>>4);
                vd0 += QRandom2(nValue);
            }
            if (pOther->at37f)
            {
                int nValue = ClipHigh(pOther->at37f*8, 2000);
                v54 += QRandom2(nValue >> 8);
                v50 += QRandom2(nValue >> 8);
                vd8 += QRandom2(nValue >> 4);
                vd4 += QRandom2(nValue >> 4);
                vd0 += QRandom2(nValue);
            }
            CalcOtherPosition(pOther->pSprite, &vd8, &vd4, &vd0, &vcc, v50, 0);
            CheckLink(&vd8, &vd4, &vd0, &vcc);
            if (IsUnderwaterSector(vcc))
            {
                v14 = 10;
            }
            memcpy(bakMirrorGotpic, gotpic+510, 2);
            memcpy(gotpic+510, otherMirrorGotpic, 2);
            g_visibility = ClipLow(gVisibility-32*pOther->at362, 0);
            int vc4, vc8;
            getzsofslope(vcc, vd8, vd4, &vc8, &vc4);
            if (vd0 >= vc4)
            {
                vd0 = vc4-(8<<4);
            }
            if (vd0 <= vc8)
            {
                vd0 = vc8+(8<<4);
            }
            v54 = ClipRange(v54, -200, 200);
RORHACKOTHER:
            int ror_status[16];
            for (int i = 0; i < 16; i++)
                ror_status[i] = TestBitString(gotpic, 4080 + i);
            DrawMirrors(vd8, vd4, vd0, v50, v54 + 90);
            drawrooms(vd8, vd4, vd0, v50, v54 + 90, vcc);
            bool do_ror_hack = false;
            for (int i = 0; i < 16; i++)
                if (!ror_status[i] && TestBitString(gotpic, 4080 + i))
                    do_ror_hack = true;
            if (do_ror_hack)
                goto RORHACKOTHER;
            memcpy(otherMirrorGotpic, gotpic+510, 2);
            memcpy(gotpic+510, bakMirrorGotpic, 2);
            viewProcessSprites(vd8, vd4, vd0);
            renderDrawMasks();
            renderRestoreTarget();
        }
        else
        {
            othercameraclock = gGameClock;
        }

        if (!vc)
        {
            deliriumTilt = 0;
            deliriumTurn = 0;
            deliriumPitch = 0;
        }
        int nSprite = headspritestat[2];
        int unk = 0;
        while (nSprite >= 0)
        {
            SPRITE *pSprite = &qsprite[nSprite];
            int nXSprite = pSprite->extra;
            dassert(nXSprite > 0 && nXSprite < kMaxXSprites);
            XSPRITE *pXSprite = &xsprite[nXSprite];
            if (TestBitString(gotsector, pSprite->sectnum))
            {
                unk += pXSprite->at14_0*32;
            }
            nSprite = nextspritestat[nSprite];
        }
        nSprite = headspritestat[5];
        while (nSprite >= 0)
        {
            SPRITE *pSprite = &qsprite[nSprite];
            switch (pSprite->type)
            {
            case 301:
            case 302:
            case 303:
            case 306:
                if (TestBitString(gotsector, pSprite->sectnum))
                {
                    unk += 256;
                }
                break;
            }
            nSprite = nextspritestat[nSprite];
        }
        g_visibility = ClipLow(gVisibility - 32 * gView->at362 - unk, 0);
        cA = (cA + interpolateang(deliriumTurnO, deliriumTurn, gInterpolate)) & 2047;
        int vfc, vf8;
        getzsofslope(nSectnum, cX, cY, &vfc, &vf8);
        if (cZ >= vf8)
        {
            cZ = vf8-(8<<8);
        }
        if (cZ <= vfc)
        {
            cZ = vfc+(8<<8);
        }
        va0 = ClipRange(va0, -200, 200);
RORHACK:
        int ror_status[16];
        for (int i = 0; i < 16; i++)
            ror_status[i] = TestBitString(gotpic, 4080+i);
        int deliriumPitchI = interpolate(deliriumPitchO, deliriumPitch, gInterpolate);
        DrawMirrors(cX, cY, cZ, cA, va0 + 90 + deliriumPitchI);
        int bakCstat = gView->pSprite->cstat;
        if (gViewPos == 0)
        {
            gView->pSprite->cstat |= 32768;
        }
        else
        {
            gView->pSprite->cstat |= 514;
        }
        drawrooms(cX, cY, cZ, cA, va0 + 90 + deliriumPitchI, nSectnum);
        bool do_ror_hack = false;
        for (int i = 0; i < 16; i++)
            if (!ror_status[i] && TestBitString(gotpic, 4080+i))
                do_ror_hack = true;
        if (do_ror_hack)
            goto RORHACK;

        viewProcessSprites(cX, cY, cZ);
        sub_5571C(1);
        renderDrawMasks();
        sub_5571C(0);
        sub_557C4(cX, cY, gInterpolate);
        renderDrawMasks();
        gView->pSprite->cstat = bakCstat;

        if (v78 || vc)
        {
            if (videoGetRenderMode() == REND_CLASSIC)
            {
                dassert(waloff[ TILTBUFFER ] != NULL);
                renderRestoreTarget();
                char vrc = 70;
                if (vc)
                {
                    vrc = 103;
                }
                int nAng = v78 & 511;
                if (nAng > 256)
                {
                    nAng = 512 - nAng;
                }
                int nScale = dmulscale32(Cos(nAng), 256000, Sin(nAng), 160000)>>1;
                rotatesprite(160<<16, 100<<16, nScale, v78+512, TILTBUFFER, 0, 0, vrc, gViewX0, gViewY0, gViewX1, gViewY1);
            }
        }

        if (r_usenewaspect)
        {
            newaspect_enable = 0;
            renderSetAspect(viewingRange, yxAspect);
        }
        int nClipDist = gView->pSprite->clipdist<<2;
        long ve8, vec, vf0, vf4;
        GetZRange(gView->pSprite, &vf4, &vf0, &vec, &ve8, nClipDist, 0);
        int tmpSect = nSectnum;
        if ((vf0 & 0xe000) == 0x4000)
        {
            tmpSect = vf0 & (kMaxWalls-1);
        }
#if 0
        int v8 = byte_1CE5C2 > 0 && (sector[tmpSect].ceilingstat&1);
        if (gWeather.at12d8 > 0 || v8)
        {
            gWeather.Draw(cX, cY, cZ, cA, va0 + 90 + deliriumPitch, gWeather.at12d8);
            if (v8)
            {
                gWeather.at12d8 = ClipRange(delta*8+gWeather.at12d8, 0, 4095);
            }
            else
            {
                gWeather.at12d8 = ClipRange(gWeather.at12d8-delta*64, 0, 4095);
            }
        }
#endif
        if (gViewPos == 0)
        {
            if (gAimReticle)
            {
                rotatesprite(160<<16, 90<<16, 65536, 0, 2319, 0, 0, 2, gViewX0, gViewY0, gViewX1, gViewY1);
            }
            cX = (v4c>>8)+160;
            cY = (v48>>8)+220+(zDelta>>7);
            int nShade = sector[nSectnum].floorshade;
            int nPalette = 0;
            if (sector[gView->pSprite->sectnum].extra > 0)
            {
                SECTOR *pSector = &qsector[gView->pSprite->sectnum];
                XSECTOR *pXSector = &xsector[pSector->extra];
                if (pXSector->at18_0)
                {
                    nPalette = pSector->floorpal;
                }
            }
            WeaponDraw(gView, nShade, cX, cY, nPalette);
        }
        if (gViewPos == 0 && gView->pXSprite->at2c_0 > 60)
        {
            viewBurnTime(gView->pXSprite->at2c_0);
        }
        if (packItemActive(gView, 1))
        {
            rotatesprite(0, 0, 65536, 0, 2344, 0, 0, 18, gViewX0, gViewY0, gViewX1, gViewY1);
            rotatesprite(320<<16, 0, 65536, 1024, 2344, 0, 0, 22, gViewX0, gViewY0, gViewX1, gViewY1);
            rotatesprite(0, 200<<16, 65536, 0, 2344, 0, 0, 22, gViewX0, gViewY0, gViewX1, gViewY1);
            rotatesprite(320<<16, 200<<16, 65536, 1024, 2344, 0, 0, 18, gViewX0, gViewY0, gViewX1, gViewY1);
            if (gDetail >= 4)
            {
                rotatesprite(15<<16, 3<<16, 65536, 0, 2346, 32, 0, 19, gViewX0, gViewY0, gViewX1, gViewY1);
                rotatesprite(212<<16, 77<<16, 65536, 0, 2347, 32, 0, 19, gViewX0, gViewY0, gViewX1, gViewY1);
            }
        }
        if (powerupCheck(gView, 39) > 0)
        {
            rotatesprite(0, 200<<16, 65536, 0, 2358, 0, 0, 22, gViewX0, gViewY0, gViewX1, gViewY1);
            rotatesprite(320<<16, 200<<16, 65536, 1024, 2358, 0, 0, 18, gViewX0, gViewY0, gViewX1, gViewY1);
        }
        if (v4 && gNetPlayers > 1)
        {
            DoLensEffect();
            renderSetAspect(65536, 54613);
            rotatesprite(280<<16, 35<<16, 53248, 512, 4077, v10, v14, 6, gViewX0, gViewY0, gViewX1, gViewY1);
            rotatesprite(280<<16, 35<<16, 53248, 0, 1683, v10, 0, 35, gViewX0, gViewY0, gViewX1, gViewY1);
            //renderSetAspect(65536, divscale16(xdim*200, ydim*320));
            videoSetCorrectedAspect();
        }
        if (powerupCheck(gView, 14) > 0)
        {
            nPalette = 4;
        }
        else if(powerupCheck(gView, 24) > 0)
        {
            nPalette = 1;
        }
        else
        {
            if (gView->at87)
            {
                if (gView->pXSprite->at17_6 == 1)
                {
                    nPalette = 1;
                }
                else if (gView->pXSprite->at17_6 == 2)
                {
                    nPalette = 3;
                }
                else
                {
                    nPalette = 2;
                }
            }
        }
    }
    if (gViewMode == 4)
    {
        gViewMap.sub_25DB0(gView->pSprite);
    }
    viewDrawInterface(delta);
    int zn = ((gView->at6f-gView->at67-(12<<8))>>7)+220;
    PLAYER *pPSprite = &gPlayer[gMe->pSprite->type-kDudePlayer1];
    if (pPSprite->at376 == 1)
    {
        static int lastClock;
        gChoke.sub_84110(160, zn);
        if ((gGameClock % 5) == 0 && gGameClock != lastClock)
        {
            gChoke.at1c(pPSprite);
        }
        lastClock = gGameClock;
    }
    if (byte_1A76C6)
    {
        DrawStatSprite(2048, xdim-15, 20);
    }
    viewDisplayMessage();
    CalcFrameRate();
    if (gShowFrameRate)
    {
        int fX, fY;
        if (gViewMode == 3)
        {
            fX = gViewX1;
        }
        else
        {
            fX = xdim;
        }
        if (gViewMode == 3)
        {
            fY = gViewY0;
        }
        else
        {
            fY = 0;
        }
        if (gViewMode == 4)
        {
            fY += mulscale16(20, yscale);
        }
        sprintf(gTempStr, "%3i", gFrameRate);
        printext256(fX-12, fY, 31, -1, gTempStr, 1);
        fY += 8;
        sprintf(gTempStr, "pos=%d,%d,%d", gView->pSprite->x, gView->pSprite->y, gView->pSprite->z);
        printext256(fX-strlen(gTempStr)*4, fY, 31, -1, gTempStr, 1);
    }
    if (gPaused)
    {
        viewDrawText(1, "PAUSED", 160, 10, 0, 0, 1, 0);
    }
    else if (gView != gMe)
    {
        sprintf(gTempStr, "] %s [", gProfile[gView->at57].name);
        viewDrawText(0, gTempStr, 160, 10, 0, 0, 1, 0);
    }
    if (errMsg[0])
    {
        viewDrawText(0, errMsg, 160, 20, 0, 0, 1, 0);
    }
    if (gViewInterpolate)
    {
        RestoreInterpolations();
    }
    UpdateDacs(nPalette);
}

void viewLoadingScreen(int nTile, const char *pText, const char *pText2, const char *pText3)
{
    int vc;
    gMenuTextMgr.GetFontInfo(1, NULL, NULL, &vc);
    if (nTile)
    {
        videoClearScreen(0);
        rotatesprite(160<<16, 100<<16, 65536, 0, nTile, 0, 0, 74, 0, 0, xdim-1, ydim-1);
    }
    if (pText)
    {
        rotatesprite(160<<16, 20<<16, 65536, 0, 2038, -128, 0, 78, 0, 0, xdim-1, ydim-1);
        viewDrawText(1, pText, 160, 20, -128, 0, 1, 1);
    }
    if (pText2)
    {
        viewDrawText(1, pText2, 160, 50, -128, 0, 1, 1);
    }
    if (pText3)
    {
        viewDrawText(1, pText3, 160, 70, -128, 0, 1, 1);
    }
    viewDrawText(3, "Please Wait", 160, 134, -128, 0, 1, 1);
}

class ViewLoadSave : public LoadSave {
public:
    void Load(void);
    void Save(void);
};

static ViewLoadSave myLoadSave;

static long messageTime;
static char message[256];

void ViewLoadSave::Load(void)
{
    Read(&messageTime, sizeof(messageTime));
    Read(message, sizeof(message));
    Read(otherMirrorGotpic, sizeof(otherMirrorGotpic));
    Read(bakMirrorGotpic, sizeof(bakMirrorGotpic));
    Read(&gScreenTilt, sizeof(gScreenTilt));
    Read(&deliriumTilt, sizeof(deliriumTilt));
    Read(&deliriumTurn, sizeof(deliriumTurn));
    Read(&deliriumPitch, sizeof(deliriumPitch));
}

void ViewLoadSave::Save(void)
{
    Write(&messageTime, sizeof(messageTime));
    Write(message, sizeof(message));
    Write(otherMirrorGotpic, sizeof(otherMirrorGotpic));
    Write(bakMirrorGotpic, sizeof(bakMirrorGotpic));
    Write(&gScreenTilt, sizeof(gScreenTilt));
    Write(&deliriumTilt, sizeof(deliriumTilt));
    Write(&deliriumTurn, sizeof(deliriumTurn));
    Write(&deliriumPitch, sizeof(deliriumPitch));
}
