//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2003 - 3D Realms Entertainment
Copyright (C) 2017-2019 Nuke.YKT

This file is part of Duke Nukem 3D version 1.5 - Atomic Edition

Duke Nukem 3D is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1996 - Todd Replogle
Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------
#include "duke3d.h"

short torchcnt;
short jaildoorcnt;
short minecartcnt;

extern char _wp1,_wp2,_wp3,_wp4,_wp5,_wp6;

short torchsector[64];
short torchsectorshade[64];
short torchtype[64];

short lightnincnt;

short jaildoorsound[32];
long jaildoordrag[32];
long jaildoorspeed[32];
short jaildoorsecthtag[32];
long jaildoordist[32];
short jaildoordir[32];
short jaildooropen[32];
short jaildoorsect[32];

short minecartdir[16];
long minecartspeed[16];
short minecartchildsect[16];
short minecartsound[16];
long minecartdist[16];
long minecartdrag[16];
short minecartopen[16];
short minecartsect[16];

short lightninsector[64];
short lightninsectorshade[64];

void dotorch(void)
{
    int ds;
    short j, i;
    short startwall, endwall;
    char shade;
    ds = TRAND&8;
    for (i = 0; i < torchcnt; i++)
    {
        shade = torchsectorshade[i] - ds;
        switch (torchtype[i])
        {
            case 0:
                sector[torchsector[i]].floorshade = shade;
                sector[torchsector[i]].ceilingshade = shade;
                break;
            case 1:
                sector[torchsector[i]].ceilingshade = shade;
                break;
            case 2:
                sector[torchsector[i]].floorshade = shade;
                break;
            case 4:
                sector[torchsector[i]].ceilingshade = shade;
                break;
            case 5:
                sector[torchsector[i]].floorshade = shade;
                break;
        }
        startwall = sector[torchsector[i]].wallptr;
        endwall = startwall + sector[torchsector[i]].wallnum;
        for (j = startwall; j < endwall; j++)
        {
            if (wall[j].lotag != 1)
            {
                switch (torchtype[i])
                {
                    case 0:
                        wall[j].shade = shade;
                        break;
                    case 1:
                        wall[j].shade = shade;
                        break;
                    case 2:
                        wall[j].shade = shade;
                        break;
                    case 3:
                        wall[j].shade = shade;
                        break;
                }
            }
        }
    }
}

void dojaildoor(void)
{
    short i, j;
    short startwall, endwall;
    long x, y;
    long speed;
    for (i = 0; i < jaildoorcnt; i++)
    {
        if (numplayers > 2)
            speed = jaildoorspeed[i];
        else
            speed = jaildoorspeed[i];
        if (speed < 2)
            speed = 2;
        if (jaildooropen[i] == 1)
        {
            jaildoordrag[i] -= speed;
            if (jaildoordrag[i] <= 0)
            {
                jaildoordrag[i] = 0;
                jaildooropen[i] = 2;
                switch (jaildoordir[i])
                {
                    case 10:
                        jaildoordir[i] = 30;
                        break;
                    case 20:
                        jaildoordir[i] = 40;
                        break;
                    case 30:
                        jaildoordir[i] = 10;
                        break;
                    case 40:
                        jaildoordir[i] = 20;
                        break;
                }
            }
            else
            {
                startwall = sector[jaildoorsect[i]].wallptr;
                endwall = startwall + sector[jaildoorsect[i]].wallnum;
                for (j = startwall; j < endwall; j++)
                {
                    switch (jaildoordir[i])
                    {
                        case 10:
                            x = wall[j].x;
                            y = wall[j].y + speed;
                            break;
                        case 20:
                            x = wall[j].x - speed;
                            y = wall[j].y;
                            break;
                        case 30:
                            x = wall[j].x;
                            y = wall[j].y - speed;
                            break;
                        case 40:
                            x = wall[j].x + speed;
                            y = wall[j].y;
                            break;
                    }
                    dragpoint(j,x,y);
                }
            }
        }
        if (jaildooropen[i] == 3)
        {
            jaildoordrag[i] -= speed;
            if (jaildoordrag[i] <= 0)
            {
                jaildoordrag[i] = 0;
                jaildooropen[i] = 0;
                switch (jaildoordir[i])
                {
                    case 10:
                        jaildoordir[i] = 30;
                        break;
                    case 20:
                        jaildoordir[i] = 40;
                        break;
                    case 30:
                        jaildoordir[i] = 10;
                        break;
                    case 40:
                        jaildoordir[i] = 20;
                        break;
                }
            }
            else
            {
                startwall = sector[jaildoorsect[i]].wallptr;
                endwall = startwall + sector[jaildoorsect[i]].wallnum;
                for (j = startwall; j < endwall; j++)
                {
                    switch (jaildoordir[i])
                    {
                        case 10:
                            x = wall[j].x;
                            y = wall[j].y + speed;
                            break;
                        case 20:
                            x = wall[j].x - speed;
                            y = wall[j].y;
                            break;
                        case 30:
                            x = wall[j].x;
                            y = wall[j].y - speed;
                            break;
                        case 40:
                            x = wall[j].x + speed;
                            y = wall[j].y;
                            break;
                    }
                    dragpoint(j,x,y);
                }
            }
        }
    }
}

void moveminecart(void)
{
    short i;
    short j;
    short csect;
    short startwall;
    short endwall;
    long speed;
    long y;
    long x;
    short nextj;
    long cx;
    long cy;
    long unk;
    long max_x;
    long min_y;
    long max_y;
    long min_x;
    for (i = 0; i < minecartcnt; i++)
    {
        speed = minecartspeed[i];
        if (speed < 2)
            speed = 2;

        if (minecartopen[i] == 1)
        {
            minecartdrag[i] -= speed;
            if (minecartdrag[i] <= 0)
            {
                minecartdrag[i] = minecartdist[i];
                minecartopen[i] = 2;
                switch (minecartdir[i])
                {
                    case 10:
                        minecartdir[i] = 30;
                        break;
                    case 20:
                        minecartdir[i] = 40;
                        break;
                    case 30:
                        minecartdir[i] = 10;
                        break;
                    case 40:
                        minecartdir[i] = 20;
                        break;
                }
            }
            else
            {
                startwall = sector[minecartsect[i]].wallptr;
                endwall = startwall + sector[minecartsect[i]].wallnum;
                for (j = startwall; j < endwall; j++)
                {
                    switch (minecartdir[i])
                    {
                        case 10:
                            x = wall[j].x;
                            y = wall[j].y + speed;
                            break;
                        case 20:
                            x = wall[j].x - speed;
                            y = wall[j].y;
                            break;
                        case 30:
                            x = wall[j].x;
                            y = wall[j].y - speed;
                            break;
                        case 40:
                            x = wall[j].x + speed;
                            y = wall[j].y;
                            break;
                    }
                    dragpoint(j,x,y);
                }
            }
        }
        if (minecartopen[i] == 2)
        {
            minecartdrag[i] -= speed;
            if (minecartdrag[i] <= 0)
            {
                minecartdrag[i] = minecartdist[i];
                minecartopen[i] = 1;
                switch (minecartdir[i])
                {
                    case 10:
                        minecartdir[i] = 30;
                        break;
                    case 20:
                        minecartdir[i] = 40;
                        break;
                    case 30:
                        minecartdir[i] = 10;
                        break;
                    case 40:
                        minecartdir[i] = 20;
                        break;
                }
            }
            else
            {
                startwall = sector[minecartsect[i]].wallptr;
                endwall = startwall + sector[minecartsect[i]].wallnum;
                for (j = startwall; j < endwall; j++)
                {
                    switch (minecartdir[i])
                    {
                        case 10:
                            x = wall[j].x;
                            y = wall[j].y + speed;
                            break;
                        case 20:
                            x = wall[j].x - speed;
                            y = wall[j].y;
                            break;
                        case 30:
                            x = wall[j].x;
                            y = wall[j].y - speed;
                            break;
                        case 40:
                            x = wall[j].x + speed;
                            y = wall[j].y;
                            break;
                    }
                    dragpoint(j,x,y);
                }
            }
        }
        csect = minecartchildsect[i];
        startwall = sector[csect].wallptr;
        endwall = startwall + sector[csect].wallnum;
        max_x = max_y = -0x20000;
        min_x = min_y = 0x20000;
        for (j = startwall; j < endwall; j++)
        {
            x = wall[j].x;
            y = wall[j].y;
            if (x > max_x)
                max_x = x;
            if (y > max_y)
                max_y = y;
            if (x < min_x)
                min_x = x;
            if (y < min_y)
                min_y = y;
        }
        cx = (max_x + min_x) >> 1;
        cy = (max_y + min_y) >> 1;
        j = headspritesect[csect];
        while (j != -1)
        {
            nextj = nextspritesect[j];
            if (badguy(&sprite[j]))
                setsprite(j,cx,cy,sprite[j].z);
            j = nextj;
        }
    }
}
