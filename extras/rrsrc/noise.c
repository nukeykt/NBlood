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
int enemyindex[128];
short maxenemies[3];

short madenoise(short snum)
{
    struct player_struct *p;
    p = &ps[snum];
    p->at28e = 1;
    p->at286 = p->posx;
    p->at28a = p->posy;
    return 1;
}

short wakeup(short i, short snum)
{
    struct player_struct *p;
    long radius;
    p = &ps[snum];
    if (!p->at28e)
        return 0;
#ifdef RRRA
    if (sprite[i].pal == 30 || sprite[i].pal == 32 || sprite[i].pal == 33 || sprite[i].pal == 8)
#else
    if (sprite[i].pal == 30 || sprite[i].pal == 32 || sprite[i].pal == 33)
#endif
        return 0;

    radius = p->at290;

    if (p->at286 - radius < sprite[i].x && p->at286 + radius > sprite[i].x
        && p->at28a - radius < sprite[i].y && p->at28a + radius > sprite[i].y)
        return 1;
    return 0;
}
