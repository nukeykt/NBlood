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

#include "lava.h"

short geosectorwarp[64];
short geosectorwarp2[64];
short geosector[64];
long geox[64];
long geoy[64];
long geoz[64];
long geox2[64];
long geoy2[64];
long geoz2[64];
short geocnt;

char gbrightness;
char brightness;

long thunderflash;
long thundertime;

long winderflash;
long windertime;

void thunder(void)
{
    struct player_struct *p;
    int r1, r2;
    short startwall, endwall, i, j;
    unsigned char shade;

    p = &ps[screenpeek];

    if (!thunderflash)
    {
        if ((gotpic[RRTILE2577>>3]&(1<<(RRTILE2577&7))) > 0)
        {
            gotpic[RRTILE2577>>3] &= ~(1<<(RRTILE2577&7));
            if (waloff[RRTILE2577] != -1)
            {
                visibility = 256;
                if (TRAND > 65000)
                {
                    thunderflash = 1;
                    thundertime = 256;
                    sound(351+(rand()%3));
                }
            }
        }
        else
        {
            visibility = p->visibility;
            brightness = ud.brightness>>2;
        }
    }
    else
    {
        thundertime -= 4;
        if (thundertime < 0)
        {
            thunderflash = 0;
            brightness = ud.brightness>>2;
            setbrightness(brightness,palette);
            visibility = p->visibility;
        }
    }
    if (!winderflash)
    {
        if ((gotpic[RRTILE2562>>3]&(1<<(RRTILE2562&7))) > 0)
        {
            gotpic[RRTILE2562>>3] &= ~(1<<(RRTILE2562&7));
            if (waloff[RRTILE2562] != -1)
            {
                if (TRAND > 65000)
                {
                    winderflash = 1;
                    windertime = 128;
                    sound(351+(rand()%3));
                }
            }
        }
    }
    else
    {
        windertime -= 4;
        if (windertime < 0)
        {
            winderflash = 0;
            for (i = 0; i < lightnincnt; i++)
            {
                startwall = sector[lightninsector[i]].wallptr;
                endwall = startwall + sector[lightninsector[i]].wallnum;
                sector[lightninsector[i]].floorshade = lightninsectorshade[i];
                sector[lightninsector[i]].ceilingshade = lightninsectorshade[i];
                for (j = startwall; j < endwall; j++)
                    wall[j].shade = lightninsectorshade[i];
            }
        }
    }
    if (thunderflash == 1)
    {
        r1 = TRAND&4;
        brightness += r1;
        switch (r1)
        {
        case 0:
            visibility = 2048;
            break;
        case 1:
            visibility = 1024;
            break;
        case 2:
            visibility = 512;
            break;
        case 3:
            visibility = 256;
            break;
        default:
            visibility = 4096;
            break;
        }
        if (brightness > 8)
            brightness = 0;
        setbrightness(brightness,palette);
    }
    if (winderflash == 1)
    {
        r2 = TRAND&8;
        shade = torchsectorshade[i]+r2;
        for (i = 0; i < lightnincnt; i++)
        {
            startwall = sector[lightninsector[i]].wallptr;
            endwall = startwall + sector[lightninsector[i]].wallnum;
            sector[lightninsector[i]].floorshade = lightninsectorshade[i] - shade;
            sector[lightninsector[i]].ceilingshade = lightninsectorshade[i] - shade;
            for (j = startwall; j < endwall; j++)
                wall[j].shade = lightninsectorshade[i] - shade;
        }
    }
}