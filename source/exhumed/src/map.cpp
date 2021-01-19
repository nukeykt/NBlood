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

#include "typedefs.h"
#include <string.h>
#include "player.h"
#include "init.h"
#include "engine.h"
#include "exhumed.h"
#include "view.h"
#include "names.h"
#include "util.h"

int bShowTowers = kFalse;
int bFollowMode = kTrue;
int ldMapZoom;
int lMapZoom;
int32_t mapx, mapy, mapang;

int32_t x = 0;
int32_t y = 0;
int16_t nAngle = 0;

int mapForward, mapStrafe;
fix16_t mapTurn;

int32_t nMapPic = -1;

void MarkSectorSeen(short nSector);


// TEMP - taken from Blood
template<typename T> void GetSpriteExtents(T const* const pSprite, int* top, int* bottom)
{
    *top = *bottom = pSprite->z;
    if ((pSprite->cstat & 0x30) != 0x20)
    {
        int height = tilesiz[pSprite->picnum].y;
        int center = height / 2 + picanm[pSprite->picnum].yofs;
        *top -= (pSprite->yrepeat << 2) * center;
        *bottom += (pSprite->yrepeat << 2) * (height - center);
    }
}


void InitMap()
{
    memset(show2dsector, 0, sizeof(show2dsector));
    memset(show2dwall,   0, sizeof(show2dwall));
    memset(show2dsprite, 0, sizeof(show2dsprite));

    mapForward = 0;
    mapStrafe = 0;
    mapTurn = 0;

    nMapPic = -1;
    ldMapZoom = 64;
    lMapZoom  = 1000;
    bFollowMode = kTrue;
}

void GrabMap()
{
    for (int i = 0; i < numsectors; i++) {
        MarkSectorSeen(i);
    }
}

void MarkSectorSeen(short nSector)
{
    int i = nSector;

    if (!(pow2char[i & 7] & show2dsector[i >> 3]))
    {
        show2dsector[i >> 3] |= pow2char[i & 7];
        walltype* wal = &wall[sector[i].wallptr];

        for (int j = sector[i].wallnum; j > 0; j--, wal++)
        {
            i = wal->nextsector;
            if (i < 0)
                continue;

            // if it's a dark sector and the torch is off, don't reveal the next sector
            if (sector[i].ceilingpal == 3 && !nPlayerTorch[nLocalPlayer])
                continue;

            uint16_t const nextwall = wal->nextwall;
            if (nextwall < MAXWALLS && wall[nextwall].cstat & 0x0071)
                continue;

            if (sector[i].ceilingz >= sector[i].floorz)
                continue;

            show2dsector[i >> 3] |= pow2char[i & 7];
        }
    }
}

static void G_DrawOverheadMap(int32_t cposx, int32_t cposy, int32_t czoom, int16_t cang)
{
    int32_t i, j, k, x1, y1, x2=0, y2=0, ox, oy;
    int32_t z1, z2, startwall, endwall;
    int32_t xvect, yvect, xvect2, yvect2;
    char col;
    uwallptr_t wal, wal2;

    int32_t tmpydim = (xdim*5)/8;

    renderSetAspect(65536, divscale16(tmpydim*320, xdim*200));

    xvect = Sin(-cang) * czoom;
    yvect = Sin(1536 - cang) * czoom;

    xvect2 = mulscale16(xvect, yxaspect);
    yvect2 = mulscale16(yvect, yxaspect);

    renderDisableFog();

    // draw player position arrow
    /*
    renderDrawLine(xdim << 11, (ydim << 11) - 20480, xdim << 11, (ydim << 11) + 20480, 24);
    renderDrawLine((xdim << 11) - 20480, ydim << 11, xdim << 11, (ydim << 11) - 20480, 24);
    renderDrawLine((xdim << 11) + 20480, ydim << 11, xdim << 11, (ydim << 11) - 20480, 24);
    */
    short nPlayerSprite = PlayerList[nLocalPlayer].nSprite;

    int nPlayerZ = sprite[nPlayerSprite].z;

    // Draw red lines
    for (i=numsectors-1; i>=0; i--)
    {
        if (!(show2dsector[i>>3]&pow2char[i&7])) continue;

        startwall = sector[i].wallptr;
        endwall = sector[i].wallptr + sector[i].wallnum;

        z1 = sector[i].ceilingz;
        z2 = sector[i].floorz;

        for (j=startwall, wal=(uwallptr_t)&wall[startwall]; j<endwall; j++, wal++)
        {
            k = wal->nextwall;
            if (k < 0) continue;

            if (sector[wal->nextsector].ceilingz == z1 && sector[wal->nextsector].floorz == z2)
                if (((wal->cstat|wall[wal->nextwall].cstat)&(16+32)) == 0) continue;

            if (nMapMode == 2)
                col = 111;
            else
                col = 111 - min(klabs(z2 - nPlayerZ) >> 13, 12);

            ox = wal->x-cposx;
            oy = wal->y-cposy;
            x1 = dmulscale16(ox, xvect, -oy, yvect)+(xdim<<11);
            y1 = dmulscale16(oy, xvect2, ox, yvect2)+(ydim<<11);

            wal2 = (uwallptr_t)&wall[wal->point2];
            ox = wal2->x-cposx;
            oy = wal2->y-cposy;
            x2 = dmulscale16(ox, xvect, -oy, yvect)+(xdim<<11);
            y2 = dmulscale16(oy, xvect2, ox, yvect2)+(ydim<<11);

            renderDrawLine(x1, y1, x2, y2, col);
        }
    }

    // Draw white lines
    for (i=numsectors-1; i>=0; i--)
    {
        if (!(show2dsector[i>>3]&pow2char[i&7])) continue;

        startwall = sector[i].wallptr;
        endwall = sector[i].wallptr + sector[i].wallnum;
        z2 = sector[i].floorz;

        if (nMapMode == 2)
        {
            col = 111;
        }
        else
        {
            col = klabs(z2 - nPlayerZ) >> 13;
            if (col > 15)
                continue;
            col = 111 - col;
        }

        k = -1;
        for (j=startwall, wal=(uwallptr_t)&wall[startwall]; j<endwall; j++, wal++)
        {
            if (wal->nextwall >= 0) continue;

            if (tilesiz[wal->picnum].x == 0) continue;
            if (tilesiz[wal->picnum].y == 0) continue;

            if (j == k)
            {
                x1 = x2;
                y1 = y2;
            }
            else
            {
                ox = wal->x-cposx;
                oy = wal->y-cposy;
                x1 = dmulscale16(ox, xvect, -oy, yvect)+(xdim<<11);
                y1 = dmulscale16(oy, xvect2, ox, yvect2)+(ydim<<11);
            }

            k = wal->point2;
            wal2 = (uwallptr_t)&wall[k];
            ox = wal2->x-cposx;
            oy = wal2->y-cposy;
            x2 = dmulscale16(ox, xvect, -oy, yvect)+(xdim<<11);
            y2 = dmulscale16(oy, xvect2, ox, yvect2)+(ydim<<11);

            renderDrawLine(x1, y1, x2, y2, col);
        }
    }

    if (bShowTowers)
    {
        for (int nSprite = headspritestat[406]; nSprite != -1; nSprite = nextspritestat[nSprite])
        {
            int ox = sprite[nSprite].x - cposx;
            int oy = sprite[nSprite].y - cposy;

            int x1 = mulscale16(ox, xvect) - mulscale16(oy, yvect);
            int y1 = mulscale16(oy, xvect2) + mulscale16(ox, yvect2);

            //int nTile = kTile72 + (((int)totalclock / 64) & 3);
            int nTile = kTile67 + (((int)totalclock / 64) & 2);

            int8_t xofsbak = picanm[nTile].xofs;
            int8_t yofsbak = picanm[nTile].yofs;
            picanm[nTile].xofs = 0; picanm[nTile].yofs = 0;

            rotatesprite_win((x1 << 4) + (xdim << 15), (y1 << 4) + (ydim << 15), 65535, 0, nTile, 0, 0, 0);

            picanm[nTile].xofs = xofsbak;
            picanm[nTile].yofs = yofsbak;

            #if 0
            char nCol = 170;
            int val = 2048;

            renderDrawLine(
                (x1 - val) + (xdim << 11),
                (y1 - val) + (ydim << 11),
                (x1 - val) + (xdim << 11),
                (y1 + val) + (ydim << 11),
                nCol);

            renderDrawLine(
                x1 + (xdim << 11),
                (y1 - val) + (ydim << 11),
                x1 + (xdim << 11),
                (y1 + val) + (ydim << 11),
                nCol);

            renderDrawLine(
                (x1 + val) + (xdim << 11),
                (y1 - val) + (ydim << 11),
                (x1 + val) + (xdim << 11),
                (y1 + val) + (ydim << 11),
                nCol);
            #endif
        }
    }

    renderEnableFog();

    videoSetCorrectedAspect();

    // Draw player sprite position representation
    if (bFollowMode && nMapPic >= 0)
    {
        spritetype* pSprite = &sprite[PlayerList[nLocalPlayer].nSprite];

        int px = 0;
        int py = 0;
        int pa = 0;
        int x1 = dmulscale16(px, xvect, -py, yvect);
        int y1 = dmulscale16(py, xvect2, px, yvect);

        {
            int ceilZ, ceilHit, floorZ, floorHit;

            vec3_t pos;
            getzrange(&pos, pSprite->sectnum, &ceilZ, &ceilHit, &floorZ, &floorHit, (pSprite->clipdist << 2) + 16, CLIPMASK0);

            int nTop, nBottom;
            GetSpriteExtents(pSprite, &nTop, &nBottom);
            int nScale = mulscale((pSprite->yrepeat + ((floorZ - nBottom) >> 8)) * czoom, yxaspect, 16);
            nScale = ClipRange(nScale, 22000, 65536 << 1);

            nScale = mulscale16(czoom* (pSprite->yrepeat), yxaspect);

            rotatesprite((xdim << 15) + (x1 << 4), (ydim << 15) + (y1 << 4), nScale, pa, nMapPic, pSprite->shade, pSprite->pal, (pSprite->cstat & 2) >> 1,
                windowxy1.x, windowxy1.y, windowxy2.x, windowxy2.y);
        }
    }

#if 0
    for (TRAVERSE_CONNECT(p))
    {
        if (ud.scrollmode && p == screenpeek) continue;

        auto const pPlayer = g_player[p].ps;
        auto const pSprite = (uspriteptr_t)&sprite[pPlayer->i];

        ox = pSprite->x - cposx;
        oy = pSprite->y - cposy;
        daang = (pSprite->ang - cang) & 2047;
        if (p == screenpeek)
        {
            ox = 0;
            oy = 0;
            daang = 0;
        }
        x1 = mulscale16(ox, xvect) - mulscale16(oy, yvect);
        y1 = mulscale16(oy, xvect2) + mulscale16(ox, yvect2);

        if (p == screenpeek || GTFLAGS(GAMETYPE_OTHERPLAYERSINMAP))
        {
            if (pSprite->xvel > 16 && pPlayer->on_ground)
                i = APLAYERTOP+(((int32_t) totalclock>>4)&3);
            else
                i = APLAYERTOP;

            i = VM_OnEventWithReturn(EVENT_DISPLAYOVERHEADMAPPLAYER, pPlayer->i, p, i);

            if (i < 0)
                continue;

            j = klabs(pPlayer->truefz - pPlayer->pos.z) >> 8;
            j = mulscale16(czoom * (pSprite->yrepeat + j), yxaspect);

            if (j < 22000) j = 22000;
            else if (j > (65536<<1)) j = (65536<<1);

            rotatesprite_win((x1<<4)+(xdim<<15), (y1<<4)+(ydim<<15), j, daang, i, pSprite->shade,
                P_GetOverheadPal(pPlayer), 0);
        }
    }
#endif
}

void UpdateMap()
{
    // ceilingpal 3 is a dark area for the torch.
    if (sector[initsect].ceilingpal != 3 || (nPlayerTorch[nLocalPlayer] != 0)) {
        MarkSectorSeen(initsect);
    }
}

void SetMapPosition(int32_t _x, int32_t _y, int16_t _nAngle)
{
    x = _x;
    y = _y;
    nAngle = _nAngle;
}

void DrawMap()
{
    if (bFollowMode)
    {
        x = initx;
        y = inity;
        nAngle = inita;
    }
    else
    {
        nAngle += fix16_to_int(mapTurn) >> 3;
        x += mulscale18(mapForward, Cos(nAngle));
        y += mulscale18(mapForward, Sin(nAngle));
        x -= mulscale18(mapStrafe, Cos(nAngle + 512));
        y -= mulscale18(mapStrafe, Sin(nAngle + 512));

        mapTurn = 0;
        mapForward = 0;
        mapStrafe = 0;
    }

    if (!nFreeze && nMapMode)
    {
        if (nMapMode == 2)
        {
            videoClearViewableArea(blackcol);
            RefreshBackground();
            renderDrawMapView(x, y, lMapZoom, nAngle);
        }
        G_DrawOverheadMap(x, y, lMapZoom, nAngle);

        if (bFollowMode) {
            // TODO printext(320 - 120, 0, "Map Follow Mode", kTileFont);
        }
        else {
            // TODO printext(0, 60, "Map Scroll Mode", kTileFont);
        }
    }
}
