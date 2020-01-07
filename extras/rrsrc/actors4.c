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

extern char actor_tog;

void moveactors(void)
{
    long x, m, l, *t;
    short a, i, j, ns, nexti, nextj, sect, p, pi;
    spritetype *s;
    unsigned short k, pst;

    if (jaildoorcnt)
        dojaildoor();

    if (minecartcnt)
        moveminecart();

#ifdef RRRA
    i = headspritestat[117];
    while (i >= 0)
    {
        nexti = nextspritestat[i];
        if (sprite[i].hitag > 2)
            sprite[i].hitag = 0;
        if ((sprite[i].picnum == RRTILE8488 || sprite[i].picnum == RRTILE8490) && sprite[i].hitag != 2)
        {
            sprite[i].hitag = 2;
            sprite[i].extra = -100;
        }
        if (sprite[i].hitag == 0)
        {
            sprite[i].extra++;
            if (sprite[i].extra >= 30)
                sprite[i].hitag = 1;
        }
        else if (sprite[i].hitag == 1)
        {
            sprite[i].extra--;
            if (sprite[i].extra <= -30)
                sprite[i].hitag = 0;
        }
        else if (sprite[i].hitag == 2)
        {
            sprite[i].extra--;
            if (sprite[i].extra <= -104)
            {
                spawn(i, sprite[i].lotag);
                deletesprite(i);
            }
        }
        j = movesprite(i,0,0,sprite[i].extra*2,CLIPMASK0);
        i = nexti;
    }

    i = headspritestat[118];
    while (i >= 0)
    {
        nexti = nextspritestat[i];
        if (sprite[i].hitag > 1)
            sprite[i].hitag = 0;
        if (sprite[i].hitag == 0)
        {
            sprite[i].extra++;
            if (sprite[i].extra >= 20)
                sprite[i].hitag = 1;
        }
        else if (sprite[i].hitag == 1)
        {
            sprite[i].extra--;
            if (sprite[i].extra <= -20)
                sprite[i].hitag = 0;
        }
        j = movesprite(i,0,0,sprite[i].extra,CLIPMASK0);
        i = nexti;
    }

    if (ps[screenpeek].raat609 > 0)
    {
        ps[screenpeek].raat609--;
        if (ps[screenpeek].raat609 == 0)
        {
            ps[screenpeek].gm = MODE_EOL;
            ud.eog = 1;
            ud.level_number++;
            if (ud.level_number > 6)
                ud.level_number = 0;
            ud.m_level_number = ud.level_number;
        }
    }
    
    if (ps[screenpeek].raat607 > 0)
    {
        short ti;
        for (ti = 0; ti < MAXSPRITES; ti++)
        {
            switch (sprite[ti].picnum)
            {
            //case 4049:
            //case 4050:
            case BILLYCOCK:
            case BILLYRAY:
            case BILLYRAYSTAYPUT:
            case BRAYSNIPER:
            case DOGRUN:
            case LTH:
            case HULKJUMP:
            case HULK:
            case HULKSTAYPUT:
            case HEN:
            case DRONE:
            case PIG:
            case MINION:
            case MINIONSTAYPUT:
            case UFO1:
            case UFO2:
            case UFO3:
            case UFO4:
            case UFO5:
            case COOT:
            case COOTSTAYPUT:
            case VIXEN:
            case BIKERB:
            case BIKERBV2:
            case BIKER:
            case MAKEOUT:
            case CHEERB:
            case CHEER:
            case CHEERSTAYPUT:
            case COOTPLAY:
            case BILLYPLAY:
            case MINIONBOAT:
            case HULKBOAT:
            case CHEERBOAT:
            case RABBIT:
            case MAMA:
                if (ps[screenpeek].raat607 == 3)
                {
                    sprite[ti].xrepeat = sprite[ti].xrepeat<<1;
                    sprite[ti].yrepeat = sprite[ti].yrepeat<<1;
                    sprite[ti].clipdist = mulscale7(sprite[ti].xrepeat, tilesizx[sprite[ti].picnum]);
                }
                else if (ps[screenpeek].raat607 == 2)
                {
                    sprite[ti].xrepeat = sprite[ti].xrepeat>>1;
                    sprite[ti].yrepeat = sprite[ti].yrepeat>>1;
                    sprite[ti].clipdist = mulscale7(sprite[ti].xrepeat, tilesizx[sprite[ti].picnum]);
                }
                break;
            }
        }
        ps[screenpeek].raat607 = 0;
    }

    i = headspritestat[121];
    while (i >= 0)
    {
        nexti = nextspritestat[i];
        sprite[i].extra++;
        if (sprite[i].extra < 100)
        {
            if (sprite[i].extra == 90)
            {
                sprite[i].picnum--;
                if (sprite[i].picnum < PIG+7)
                    sprite[i].picnum = PIG+7;
                sprite[i].extra = 1;
            }
            movesprite(i,0,0,-300,CLIPMASK0);
            if (sector[sprite[i].sectnum].ceilingz + (4<<8) > sprite[i].z)
            {
                sprite[i].picnum = 0;
                sprite[i].extra = 100;
            }
        }
        else if (sprite[i].extra == 200)
        {
            setsprite(i,sprite[i].x,sprite[i].y,sector[sprite[i].sectnum].floorz-10);
            sprite[i].extra = 1;
            sprite[i].picnum = PIG+11;
            spawn(i,TRANSPORTERSTAR);
        }
        i = nexti;
    }
    
    i = headspritestat[119];
    while (i >= 0)
    {
        nexti = nextspritestat[i];
        if (sprite[i].hitag > 0)
        {
            if (sprite[i].extra == 0)
            {
                sprite[i].hitag--;
                sprite[i].extra = 150;
                spawn(i,RABBIT);
            }
            else
                sprite[i].extra--;
        }
        i = nexti;
    }
    i = headspritestat[116];
    while (i >= 0)
    {
        nexti = nextspritestat[i];
        if (sprite[i].extra)
        {
            if (sprite[i].extra == sprite[i].lotag)
                sound(183);
            sprite[i].extra--;
            j = movesprite(i,
                (sprite[i].hitag*sintable[(sprite[i].ang+512)&2047])>>14,
                (sprite[i].hitag*sintable[sprite[i].ang&2047])>>14,
                sprite[i].hitag<<1,CLIPMASK0);
            if (j > 0)
            {
                spritesound(PIPEBOMB_EXPLODE,i);
                deletesprite(i);
            }
            if (sprite[i].extra == 0)
            {
                sound(215);
                deletesprite(i);
                earthquaketime = 32;
                ps[myconnectindex].pals[0] = 32;
                ps[myconnectindex].pals[1] = 32;
                ps[myconnectindex].pals[2] = 32;
                ps[myconnectindex].pals_time = 48;
            }
        }
        i = nexti;
    }

    i = headspritestat[115];
    while (i >= 0)
    {
        nexti = nextspritestat[i];
        if (sprite[i].extra)
        {
            if (sprite[i].picnum != RRTILE8162)
                sprite[i].picnum = RRTILE8162;
            sprite[i].extra--;
            if (sprite[i].extra == 0)
            {
                int rvar;
                rvar = TRAND&127;
                if (rvar < 96)
                {
                    sprite[i].picnum = RRTILE8162+3;
                }
                else if (rvar < 112)
                {
                    if (ps[screenpeek].SlotWin & 1)
                    {
                        sprite[i].picnum = RRTILE8162+3;
                    }
                    else
                    {
                        sprite[i].picnum = RRTILE8162+2;
                        spawn(i,BATTERYAMMO);
                        ps[screenpeek].SlotWin |= 1;
                        spritesound(52,i);
                    }
                }
                else if (rvar < 120)
                {
                    if (ps[screenpeek].SlotWin & 2)
                    {
                        sprite[i].picnum = RRTILE8162+3;
                    }
                    else
                    {
                        sprite[i].picnum = RRTILE8162+6;
                        spawn(i,HEAVYHBOMB);
                        ps[screenpeek].SlotWin |= 2;
                        spritesound(52,i);
                    }
                }
                else if (rvar < 126)
                {
                    if (ps[screenpeek].SlotWin & 4)
                    {
                        sprite[i].picnum = RRTILE8162+3;
                    }
                    else
                    {
                        sprite[i].picnum = RRTILE8162+5;
                        spawn(i,SIXPAK);
                        ps[screenpeek].SlotWin |= 4;
                        spritesound(52,i);
                    }
                }
                else
                {
                    if (ps[screenpeek].SlotWin & 8)
                    {
                        sprite[i].picnum = RRTILE8162+3;
                    }
                    else
                    {
                        sprite[i].picnum = RRTILE8162+4;
                        spawn(i,ATOMICHEALTH);
                        ps[screenpeek].SlotWin |= 8;
                        spritesound(52,i);
                    }
                }
            }
        }
        i = nexti;
    }
    
    i = headspritestat[122];
    while (i >= 0)
    {
        nexti = nextspritestat[i];
        if (sprite[i].extra)
        {
            if (sprite[i].picnum != RRTILE8589)
                sprite[i].picnum = RRTILE8589;
            sprite[i].extra--;
            if (sprite[i].extra == 0)
            {
                int rvar;
                rvar = TRAND & 127;
                if (rvar < 96)
                {
                    sprite[i].picnum = RRTILE8589+4;
                }
                else if (rvar < 112)
                {
                    if (ps[screenpeek].SlotWin & 1)
                    {
                        sprite[i].picnum = RRTILE8589+4;
                    }
                    else
                    {
                        sprite[i].picnum = RRTILE8589+5;
                        spawn(i,BATTERYAMMO);
                        ps[screenpeek].SlotWin |= 1;
                        spritesound(342,i);
                    }
                }
                else if (rvar < 120)
                {
                    if (ps[screenpeek].SlotWin & 2)
                    {
                        sprite[i].picnum = RRTILE8589+4;
                    }
                    else
                    {
                        sprite[i].picnum = RRTILE8589+6;
                        spawn(i,HEAVYHBOMB);
                        ps[screenpeek].SlotWin |= 2;
                        spritesound(342,i);
                    }
                }
                else if (rvar < 126)
                {
                    if (ps[screenpeek].SlotWin & 4)
                    {
                        sprite[i].picnum = RRTILE8589+4;
                    }
                    else
                    {
                        sprite[i].picnum = RRTILE8589+2;
                        spawn(i,SIXPAK);
                        ps[screenpeek].SlotWin |= 4;
                        spritesound(342,i);
                    }
                }
                else
                {
                    if (ps[screenpeek].SlotWin & 8)
                    {
                        sprite[i].picnum = RRTILE8589+4;
                    }
                    else
                    {
                        sprite[i].picnum = RRTILE8589+3;
                        spawn(i,ATOMICHEALTH);
                        ps[screenpeek].SlotWin |= 8;
                        spritesound(342,i);
                    }
                }
            }
        }
        i = nexti;
    }

    i = headspritestat[123];
    while (i >= 0)
    {
        nexti = nextspritestat[i];
        if (sprite[i].lotag == 5)
            if (Sound[330].num == 0)
                spritesound(330,i);
        i = nexti;
    }
#endif

    i = headspritestat[107];
    while (i >= 0)
    {
        nexti = nextspritestat[i];

        if (sprite[i].hitag == 100)
        {
            sprite[i].z += (4<<8);
            if (sprite[i].z >= sector[sprite[i].sectnum].floorz + 15168)
                sprite[i].z = sector[sprite[i].sectnum].floorz + 15168;
        }

        if (sprite[i].picnum == LUMBERBLADE)
        {
            sprite[i].extra++;
            if (sprite[i].extra == 192)
            {
                sprite[i].hitag = 0;
                sprite[i].z = sector[sprite[i].sectnum].floorz - 15168;
                sprite[i].extra = 0;
                sprite[i].picnum = RRTILE3410;
                j = headspritestat[0];
                while (j >= 0)
                {
                    nextj = nextspritestat[j];
                    if (sprite[j].picnum == 128)
                        if (sprite[j].hitag == 999)
                            sprite[j].picnum = 127;
                    j = nextj;
                }
            }
        }
        i = nexti;
    }

    if (chickenplant)
    {
        i = headspritestat[106];
        while (i >= 0)
        {
            nexti = nextspritestat[i];
            switch (sprite[i].picnum)
            {
                case RRTILE285:
                    sprite[i].lotag--;
                    if (sprite[i].lotag < 0)
                    {
                        j = spawn(i,RRTILE3190);
                        sprite[j].ang = sprite[i].ang;
                        sprite[i].lotag = 128;
                    }
                    break;
                case RRTILE286:
                    sprite[i].lotag--;
                    if (sprite[i].lotag < 0)
                    {
                        j = spawn(i,RRTILE3192);
                        sprite[j].ang = sprite[i].ang;
                        sprite[i].lotag = 256;
                    }
                    break;
                case RRTILE287:
                    sprite[i].lotag--;
                    if (sprite[i].lotag < 0)
                    {
                        lotsofmoney(&sprite[i],(TRAND&3)+4);
                        sprite[i].lotag = 84;
                    }
                    break;
                case RRTILE288:
                    sprite[i].lotag--;
                    if (sprite[i].lotag < 0)
                    {
                        j = spawn(i,RRTILE3132);
                        sprite[i].lotag = 96;
#ifndef RRRA
                        spritesound(472,j);
#endif
                    }
                    break;
                case RRTILE289:
                    sprite[i].lotag--;
                    if (sprite[i].lotag < 0)
                    {
                        j = spawn(i,RRTILE3120);
                        sprite[j].ang = sprite[i].ang;
                        sprite[i].lotag = 448;
                    }
                    break;
                case RRTILE290:
                    sprite[i].lotag--;
                    if (sprite[i].lotag < 0)
                    {
                        j = spawn(i,RRTILE3122);
                        sprite[j].ang = sprite[i].ang;
                        sprite[i].lotag = 64;
                    }
                    break;
                case RRTILE291:
                    sprite[i].lotag--;
                    if (sprite[i].lotag < 0)
                    {
                        j = spawn(i,RRTILE3123);
                        sprite[j].ang = sprite[i].ang;
                        sprite[i].lotag = 512;
                    }
                    break;
                case RRTILE292:
                    sprite[i].lotag--;
                    if (sprite[i].lotag < 0)
                    {
                        j = spawn(i,RRTILE3124);
                        sprite[j].ang = sprite[i].ang;
                        sprite[i].lotag = 224;
                    }
                    break;
                case RRTILE293:
                    sprite[i].lotag--;
                    if (sprite[i].lotag < 0)
                    {
                        guts(&sprite[i],JIBS1,1,myconnectindex);
                        guts(&sprite[i],JIBS2,1,myconnectindex);
                        guts(&sprite[i],JIBS3,1,myconnectindex);
                        guts(&sprite[i],JIBS4,1,myconnectindex);
                        sprite[i].lotag = 256;
                    }
                    break;
            }
            i = nexti;
        }
    }

    i = headspritestat[105];
    while (i >= 0)
    {
        nexti = nextspritestat[i];
        if (sprite[i].picnum == RRTILE280)
            if (sprite[i].lotag == 100)
        {
            pst = pinsectorresetup(SECT);
            if (pst)
            {
                sprite[i].lotag = 0;
                if (sprite[i].extra == 1)
                {
                    pst = checkpins(SECT);
                    if (!pst)
                    {
                        sprite[i].extra = 2;
                    }
                }
                if (sprite[i].extra == 2)
                {
                    sprite[i].extra = 0;
                    resetpins(SECT);
                }
            }
        }
        i = nexti;
    }
    
    i = headspritestat[108];
    while (i >= 0)
    {
        nexti = nextspritestat[i];

        s = &sprite[i];
        if (s->picnum == RRTILE296)
        {
            p = findplayer(s,&x);
            if (x < 2047)
            {
                j = headspritestat[108];
                while (j >= 0)
                {
                    nextj = nextspritestat[j];
                    if (sprite[j].picnum == RRTILE297)
                    {
                        ps[p].ang = sprite[j].ang;
                        ps[p].bobposx = ps[p].oposx = ps[p].posx = sprite[j].x;
                        ps[p].bobposy = ps[p].oposy = ps[p].posy = sprite[j].y;
                        ps[p].oposz = ps[p].posz = sprite[j].z - (36<<8);
                        pi = ps[p].i;
                        changespritesect(pi,sprite[j].sectnum);
                        ps[p].cursectnum = sprite[pi].sectnum;
                        spritesound(70,j);
                        deletesprite(j);
                    }
                    j = nextj;
                }
            }
        }
        i = nexti;
    }

    i = headspritestat[1];
    while(i >= 0)
    {
        nexti = nextspritestat[i];

        s = &sprite[i];

        sect = s->sectnum;

        if( s->xrepeat == 0 || sect < 0 || sect >= MAXSECTORS)
            KILLIT(i);

        t = &hittype[i].temp_data[0];

        hittype[i].bposx = s->x;
        hittype[i].bposy = s->y;
        hittype[i].bposz = s->z;


        switch(s->picnum)
        {
            case RESPAWNMARKERRED:
            case RESPAWNMARKERYELLOW:
            case RESPAWNMARKERGREEN:
                T1++;
                if(T1 > respawnitemtime)
                {
                    KILLIT(i);
                }
                if( T1 >= (respawnitemtime>>1) && T1 < ((respawnitemtime>>1)+(respawnitemtime>>2)) )
                    PN = RESPAWNMARKERYELLOW;
                else if( T1 > ((respawnitemtime>>1)+(respawnitemtime>>2)) )
                    PN = RESPAWNMARKERGREEN;
                makeitfall(i);
                break;
            case RAT:
                makeitfall(i);
                IFMOVING
                {
#ifndef RRRA
                    if( (TRAND&255) == 0) spritesound(RATTY,i);
#endif
                    s->ang += (TRAND&31)-15+(sintable[(t[0]<<8)&2047]>>11);
                }
                else
                {
                    T1++;
                    if(T1 > 1) { KILLIT(i); }
                    else s->ang = (TRAND&2047);
                }
                if(s->xvel < 128)
                    s->xvel+=2;
                s->ang += (TRAND&3)-6;
                break;
            case RRTILE3190:
            case RRTILE3191:
            case RRTILE3192:
                if (!chickenplant) KILLIT(i);
                if (sector[SECT].lotag == 903)
                    makeitfall(i);
                j = movesprite(i,
                    (s->xvel*sintable[(s->ang+512)&2047])>>14,
                    (s->xvel*sintable[s->ang&2047])>>14,
                    s->zvel,CLIPMASK0);
                switch (sector[SECT].lotag)
                {
                    case 901:
                        sprite[i].picnum = RRTILE3191;
                        break;
                    case 902:
                        sprite[i].picnum = RRTILE3192;
                        break;
                    case 903:
                        if (SZ >= sector[SECT].floorz - (8<<8)) { KILLIT(i); }
                        break;
                    case 904:
                        KILLIT(i);
                        break;
                }
                if (j & 49152)
                {
                    if ((j & 49152) == 32768) { KILLIT(i); }
                    else if ((j & 49152) == 49152) { KILLIT(i); }
                }
                break;

            case RRTILE3120:
            case RRTILE3122:
            case RRTILE3123:
            case RRTILE3124:
                if (!chickenplant) KILLIT(i);
                makeitfall(i);
                j = movesprite(i,
                    (s->xvel*(sintable[(s->ang+512)&2047]))>>14,
                    (s->xvel*(sintable[s->ang&2047]))>>14,
                    s->zvel,CLIPMASK0);
                if (j & 49152)
                {
                    if ((j & 49152) == 32768) { KILLIT(i); }
                    else if ((j & 49152) == 49152) { KILLIT(i); }
                }
                if (sector[s->sectnum].lotag == 903)
                {
                    if (SZ >= sector[SECT].floorz - (4<<8))
                    {
                        KILLIT(i);
                    }
                }
                else if (sector[s->sectnum].lotag == 904)
                {
                    KILLIT(i);
                }
                break;

            case RRTILE3132:
                if (!chickenplant) KILLIT(i);
                makeitfall(i);
                j = movesprite(i,
                    (s->xvel*sintable[(s->ang+512)&2047])>>14,
                    (s->xvel*sintable[s->ang&2047])>>14,
                    s->zvel,CLIPMASK0);
                if (s->z >= sector[s->sectnum].floorz - (8<<8))
                {
                    if (sector[s->sectnum].lotag == 1)
                    {
                        j = spawn(i,WATERSPLASH2);
                        sprite[j].z = sector[sprite[j].sectnum].floorz;
                    }
                    KILLIT(i);
                }
                break;
            case BOWLINGBALL:
                if (s->xvel)
                {
                    if(Sound[356].num == 0)
                        spritesound(356,i);
                }
                else
                {
                    spawn(i,BOWLINGBALLSPRITE);
                    KILLIT(i);
                }
                if (sector[s->sectnum].lotag == 900)
                {
                    stopsound(356);
                }
            case RRTILE3440:
            case RRTILE3440+1:
            case HENSTAND:
            case HENSTAND+1:
                if (s->picnum == HENSTAND || s->picnum == HENSTAND+1)
                {
                    s->lotag--;
                    if (s->lotag == 0)
                    {
                        spawn(i,HEN);
                        KILLIT(i);
                    }
                }
                if (sector[s->sectnum].lotag == 900)
                    s->xvel = 0;
                if (s->xvel)
                {
                    makeitfall(i);
                    j = movesprite(i,
                        (sintable[(s->ang+512)&2047]*s->xvel)>>14,
                        (sintable[s->ang&2047]*s->xvel)>>14,
                        s->zvel,CLIPMASK0);
                    if (j & 49152)
                    {
                        if ((j & 49152) == 32768)
                        {
                            j &= (MAXWALLS-1);
                            k = getangle(
                                wall[wall[j].point2].x-wall[j].x,
                                wall[wall[j].point2].y-wall[j].y);
                            s->ang = ((k<<1) - s->ang)&2047;
                        }
                        else if ((j & 49152) == 49152)
                        {
                            j &= (MAXSPRITES-1);
                            checkhitsprite(i,j);
                            if (sprite[j].picnum == HEN)
                            {
                                ns = spawn(j,HENSTAND);
                                deletesprite(j);
                                sprite[ns].xvel = 32;
                                sprite[ns].lotag = 40;
                                sprite[ns].ang = s->ang;
                            }
                        }
                    }
                    s->xvel --;
                    if(s->xvel < 0) s->xvel = 0;
                    s->cstat = 257;
                    if( s->picnum == RRTILE3440 )
                    {
                        s->cstat |= 4&s->xvel;
                        s->cstat |= 8&s->xvel;
                        if (TRAND & 1)
                            s->picnum = RRTILE3440+1;
                    }
                    else if (s->picnum == HENSTAND)
                    {
                        s->cstat |= 4&s->xvel;
                        s->cstat |= 8&s->xvel;
                        if (TRAND & 1)
                            s->picnum = HENSTAND+1;
                        if (!s->xvel)
                            deletesprite(i);
                    }
                    if (s->picnum == RRTILE3440 || (s->picnum == RRTILE3440+1 && !s->xvel))
                        deletesprite(i);
                }
                else if (sector[s->sectnum].lotag == 900)
                {
                    if (s->picnum == BOWLINGBALL)
                        ballreturn(i);
                    KILLIT(i);
                }
                break;

            case QUEBALL:
            case STRIPEBALL:
                if(s->xvel)
                {
                    j = headspritestat[0];
                    while(j >= 0)
                    {
                        nextj = nextspritestat[j];
                        if( sprite[j].picnum == POCKET && ldist(&sprite[j],s) < 52 ) KILLIT(i);
                        j = nextj;
                    }

                    j = clipmove(&s->x,&s->y,&s->z,&s->sectnum,
                        (((s->xvel*(sintable[(s->ang+512)&2047]))>>14)*TICSPERFRAME)<<11,
                        (((s->xvel*(sintable[s->ang&2047]))>>14)*TICSPERFRAME)<<11,
                        24L,(4<<8),(4<<8),CLIPMASK1);

                    if(j&49152)
                    {
                        if( (j&49152) == 32768 )
                        {
                            j &= (MAXWALLS-1);
                            k = getangle(
                                wall[wall[j].point2].x-wall[j].x,
                                wall[wall[j].point2].y-wall[j].y);
                            s->ang = ((k<<1) - s->ang)&2047;
                        }
                        else if( (j&49152) == 49152 )
                        {
                            j &= (MAXSPRITES-1);
                            checkhitsprite(i,j);
                        }
                    }
                    s->xvel --;
                    if(s->xvel < 0) s->xvel = 0;
                    if( s->picnum == STRIPEBALL )
                    {
                        s->cstat = 257;
                        s->cstat |= 4&s->xvel;
                        s->cstat |= 8&s->xvel;
                    }
                }
                else
                {
                    p = findplayer(s,&x);

                    if( x < 1596)
                    {

//                        if(s->pal == 12)
                        {
                            j = getincangle(ps[p].ang,getangle(s->x-ps[p].posx,s->y-ps[p].posy));
                            if( j > -64 && j < 64 && (sync[p].bits&(1<<29)) )
                                if(ps[p].toggle_key_flag == 1)
                            {
                                a = headspritestat[1];
                                while(a >= 0)
                                {
                                    if(sprite[a].picnum == QUEBALL || sprite[a].picnum == STRIPEBALL)
                                    {
                                        j = getincangle(ps[p].ang,getangle(sprite[a].x-ps[p].posx,sprite[a].y-ps[p].posy));
                                        if( j > -64 && j < 64 )
                                        {
                                            findplayer(&sprite[a],&l);
                                            if(x > l) break;
                                        }
                                    }
                                    a = nextspritestat[a];
                                }
                                if(a == -1)
                                {
                                    if(s->pal == 12)
                                        s->xvel = 164;
                                    else s->xvel = 140;
                                    s->ang = ps[p].ang;
                                    ps[p].toggle_key_flag = 2;
                                }
                            }
                        }
                    }
                    if( x < 512 && s->sectnum == ps[p].cursectnum )
                    {
                        s->ang = getangle(s->x-ps[p].posx,s->y-ps[p].posy);
                        s->xvel = 48;
                    }
                }

                break;
            case FORCESPHERE:

                if(s->yvel == 0)
                {
                    s->yvel = 1;

                    for(l=512;l<(2048-512);l+= 128)
                        for(j=0;j<2048;j += 128)
                    {
                        k = spawn(i,FORCESPHERE);
                        sprite[k].cstat = 257+128;
                        sprite[k].clipdist = 64;
                        sprite[k].ang = j;
                        sprite[k].zvel = sintable[l&2047]>>5;
                        sprite[k].xvel = sintable[(l+512)&2047]>>9;
                        sprite[k].owner = i;
                    }
                }

                if(t[3] > 0)
                {
                    if(s->zvel < 6144)
                        s->zvel += 192;
                    s->z += s->zvel;
                    if(s->z > sector[sect].floorz)
                        s->z = sector[sect].floorz;
                    t[3]--;
                    if(t[3] == 0)
                        KILLIT(i);
                }
                else if(t[2] > 10)
                {
                    j = headspritestat[5];
                    while(j >= 0)
                    {
                        if(sprite[j].owner == i && sprite[j].picnum == FORCESPHERE)
                            hittype[j].temp_data[1] = 1+(TRAND&63);
                        j = nextspritestat[j];
                    }
                    t[3] = 64;
                }

                goto BOLT;

            case RECON:
            case UFO1:
            case UFO2:
            case UFO3:
            case UFO4:
            case UFO5:

                getglobalz(i);

                if (sector[s->sectnum].ceilingstat&1)
                   s->shade += (sector[s->sectnum].ceilingshade-s->shade)>>1;
                else s->shade += (sector[s->sectnum].floorshade-s->shade)>>1;

                if( s->z < sector[sect].ceilingz+(32<<8) )
                    s->z = sector[sect].ceilingz+(32<<8);

                if( ud.multimode < 2 )
                {
                    if( actor_tog == 1)
                    {
                        s->cstat = (short)32768;
                        goto BOLT;
                    }
                    else if(actor_tog == 2) s->cstat = 257;
                }
                IFHIT
                {
                    if( s->extra < 0 && t[0] != -1 )
                    {
                        t[0] = -1;
                        s->extra = 0;
                    }
                    RANDOMSCRAP;
                }

                if(t[0] == -1)
                {
                    s->z += 1024;
                    t[2]++;
                    if( (t[2]&3) == 0) spawn(i,EXPLOSION2);
                    getglobalz(i);
                    s->ang += 96;
                    s->xvel = 128;
                    j = ssp(i,CLIPMASK0);
                    if(j != 1 || s->z > hittype[i].floorz)
                    {
                        for(l=0;l<16;l++)
                            RANDOMSCRAP;
                        spritesound(LASERTRIP_EXPLODE,i);
#ifdef RRRA
                        if (ps[myconnectindex].raat5fd)
                            spawn(i,MINION);
                        else
#endif
                        if (s->picnum == UFO1)
                            spawn(i,HEN);
                        else if (s->picnum == UFO2)
                            spawn(i,COOT);
                        else if (s->picnum == UFO3)
                            spawn(i,COW);
                        else if (s->picnum == UFO4)
                            spawn(i,PIG);
                        else if (s->picnum == UFO5)
                            spawn(i,BILLYRAY);
                        ps[myconnectindex].actors_killed++;
                        KILLIT(i);
                    }
                    goto BOLT;
                }
                else
                {
                    if( s->z > hittype[i].floorz-(48<<8) )
                        s->z = hittype[i].floorz-(48<<8);
                }

                p = findplayer(s,&x);
                j = s->owner;

                // 3 = findplayerz, 4 = shoot

                if( t[0] >= 4 )
                {
                    t[2]++;
                    if( (t[2]&15) == 0 )
                    {
                        a = s->ang;
                        s->ang = hittype[i].tempang;
                        shoot(i,FIRELASER);
                        s->ang = a;
                    }
                    if( t[2] > (26*3) || !cansee(s->x,s->y,s->z-(16<<8),s->sectnum, ps[p].posx,ps[p].posy,ps[p].posz,ps[p].cursectnum ) )
                    {
                        t[0] = 0;
                        t[2] = 0;
                    }
                    else hittype[i].tempang +=
                        getincangle(hittype[i].tempang,getangle(ps[p].posx-s->x,ps[p].posy-s->y))/3;
                }
                else if(t[0] == 2 || t[0] == 3)
                {
                    t[3] = 0;
                    if(s->xvel > 0) s->xvel -= 16;
                    else s->xvel = 0;

                    if(t[0] == 2)
                    {
                        l = ps[p].posz-s->z;
                        if( klabs(l) < (48<<8) ) t[0] = 3;
                        else s->z += sgn(ps[p].posz-s->z)<<8;
                    }
                    else
                    {
                        t[2]++;
                        if( t[2] > (26*3) || !cansee(s->x,s->y,s->z-(16<<8),s->sectnum, ps[p].posx,ps[p].posy,ps[p].posz,ps[p].cursectnum ) )
                        {
                            t[0] = 1;
                            t[2] = 0;
                        }
                        else if( (t[2]&15) == 0 )
                        {
                            shoot(i,FIRELASER);
                        }
                    }
                    s->ang += getincangle(s->ang,getangle(ps[p].posx-s->x,ps[p].posy-s->y))>>2;
                }

                if( t[0] != 2 && t[0] != 3 )
                {
                    l = ldist(&sprite[j],s);
                    if(l <= 1524)
                    {
                        a = s->ang;
                        s->xvel >>= 1;
                    }
                    else a = getangle(sprite[j].x-s->x,sprite[j].y-s->y);

                    if(t[0] == 1 || t[0] == 4) // Found a locator and going with it
                    {
                        l = dist(&sprite[j],s);

                        if( l <= 1524 ) { if(t[0] == 1) t[0] = 0; else t[0] = 5; }
                        else
                        {
                            // Control speed here
                            if(l > 1524) { if( s->xvel < 256 ) s->xvel += 32; }
                            else
                            {
                                if(s->xvel > 0) s->xvel -= 16;
                                else s->xvel = 0;
                            }
                        }

                        if(t[0] < 2) t[2]++;

                        if( x < 6144 && t[0] < 2 && t[2] > (26*4) )
                        {
                            t[0] = 2+(TRAND&2);
                            t[2] = 0;
                            hittype[i].tempang = s->ang;
                        }
                    }

                    if(t[0] == 0 || t[0] == 5)
                    {
                        if(t[0] == 0)
                            t[0] = 1;
                        else t[0] = 4;
                        j = s->owner = LocateTheLocator(s->hitag,-1);
                        if(j == -1)
                        {
                            s->hitag = j = hittype[i].temp_data[5];
                            s->owner = LocateTheLocator(j,-1);
                            j = s->owner;
                            if(j == -1) KILLIT(i);
                        }
                        else s->hitag++;
                    }

                    t[3] = getincangle(s->ang,a);
                    s->ang += t[3]>>3;

                    if(s->z < sprite[j].z)
                        s->z += 1024;
                    else s->z -= 1024;
                }

                if(Sound[457].num < 2 )
                    spritesound(457,i);

                ssp(i,CLIPMASK0);

                goto BOLT;

            case OOZ:

                getglobalz(i);

                j = (hittype[i].floorz-hittype[i].ceilingz)>>9;
                if(j > 255) j = 255;

                x = 25-(j>>1);
                if(x < 8) x = 8;
                else if(x > 48) x = 48;

                s->yrepeat = j;
                s->xrepeat = x;
                s->z = hittype[i].floorz;

                goto BOLT;

#ifdef RRRA
            case EMPTYBIKE:
                makeitfall(i);
                getglobalz(i);
                if (sector[sect].lotag == 1)
                {
                    setsprite(i,s->x,s->y,hittype[i].floorz+(16<<8));
                }
                break;

            case EMPTYBOAT:
                makeitfall(i);
                getglobalz(i);
                break;
#endif

            case TRIPBOMBSPRITE:
#ifdef RRRA
                if (sector[sect].lotag != 1 && sector[sect].lotag != 160)
#endif
                if (s->xvel)
                {
                    j = movesprite(i,
                        (s->xvel*sintable[(s->ang+512)&2047])>>14,
                        (s->xvel*sintable[s->ang&2047])>>14,
                        s->zvel,CLIPMASK0);
                    s->xvel--;
                }
                break;

            case MORTER:
            case HEAVYHBOMB:
#ifdef RRRA
            case CHEERBOMB:
#endif

                if( (s->cstat&32768) )
                {
                    t[2]--;
                    if(t[2] <= 0)
                    {
                        spritesound(TELEPORTER,i);
                        spawn(i,TRANSPORTERSTAR);
                        s->cstat = 257;
                    }
                    goto BOLT;
                }

                p = findplayer(s,&x);

                if( x < 1220 ) s->cstat &= ~257;
                else s->cstat |= 257;

                if(t[3] == 0 )
                {
                    j = ifhitbyweapon(i);
                    if(j >= 0)
                    {
                        t[3] = 1;
                        t[4] = 0;
                        l = 0;
                        s->xvel = 0;
                        goto DETONATEB;
                    }
                }

                makeitfall(i);

#ifdef RRRA
                if( sector[sect].lotag != 1 && sector[sect].lotag != 160 && s->z >= hittype[i].floorz-(FOURSLEIGHT) && s->yvel < 3 )
#else
                if( sector[sect].lotag != 1 && s->z >= hittype[i].floorz-(FOURSLEIGHT) && s->yvel < 3 )
#endif
                {
                    if( s->yvel > 0 || (s->yvel == 0 && hittype[i].floorz == sector[sect].floorz ))
#ifdef RRRA
                    {
                        if (s->picnum != CHEERBOMB)
                            spritesound(PIPEBOMB_BOUNCE,i);
                        else
                        {
                            t[3] = 1;
                            t[4] = 1;
                            l = 0;
                            goto DETONATEB;
                        }
                    }
#else
                        spritesound(PIPEBOMB_BOUNCE,i);
#endif
                    s->zvel = -((4-s->yvel)<<8);
                    if(sector[s->sectnum].lotag== 2)
                        s->zvel >>= 2;
                    s->yvel++;
                }
#ifdef RRRA
                if (s->picnum != CHEERBOMB)
#endif
                if( s->z < hittype[i].ceilingz+(16<<8) && sector[sect].lotag != 2 )
                {
                    s->z = hittype[i].ceilingz+(16<<8);
                    s->zvel = 0;
                }

                j = movesprite(i,
                    (s->xvel*(sintable[(s->ang+512)&2047]))>>14,
                    (s->xvel*(sintable[s->ang&2047]))>>14,
                    s->zvel,CLIPMASK0);

                if(sector[SECT].lotag == 1 && s->zvel == 0)
                {
                    s->z += (32<<8);
                    if(t[5] == 0)
                    {
                        t[5] = 1;
                        spawn(i,WATERSPLASH2);
#ifdef RRRA
                        if (s->picnum == MORTER)
                            s->xvel = 0;
#endif
                    }
                }
                else t[5] = 0;

                if(t[3] == 0 && s->picnum == MORTER && (j || x < 844) )
                {
                    t[3] = 1;
                    t[4] = 0;
                    l = 0;
                    s->xvel = 0;
                    goto DETONATEB;
                }

#ifdef RRRA

                if(t[3] == 0 && s->picnum == CHEERBOMB && (j || x < 844) )
                {
                    t[3] = 1;
                    t[4] = 0;
                    l = 0;
                    s->xvel = 0;
                    goto DETONATEB;
                }
#endif

                if(sprite[s->owner].picnum == APLAYER)
                    l = sprite[s->owner].yvel;
                else l = -1;

                if(s->xvel > 0)
                {
                    s->xvel -= 5;
                    if(sector[sect].lotag == 2)
                        s->xvel -= 10;

                    if(s->xvel < 0)
                        s->xvel = 0;
                    if(s->xvel&8) s->cstat ^= 4;
                }

                if( (j&49152) == 32768 )
                {
                    j &= (MAXWALLS-1);

                    checkhitwall(i,j,s->x,s->y,s->z,s->picnum);

                    k = getangle(
                        wall[wall[j].point2].x-wall[j].x,
                        wall[wall[j].point2].y-wall[j].y);

#ifdef RRRA
                    if (s->picnum == CHEERBOMB)
                    {
                        t[3] = 1;
                        t[4] = 0;
                        l = 0;
                        s->xvel = 0;
                        goto DETONATEB;
                    }
#endif
                    s->ang = ((k<<1) - s->ang)&2047;
                    s->xvel >>= 1;
                }

                DETONATEB:

                if( ( l >= 0 && ps[l].hbomb_on == 0 ) || t[3] == 1)
                {
                    t[4]++;

                    if(t[4] == 2)
                    {
                        x = s->extra;
                        m = 0;
                        switch(s->picnum)
                        {
                            case TRIPBOMBSPRITE: m = powderkegblastradius;break;
                            case HEAVYHBOMB: m = pipebombblastradius;break;
                            case HBOMBAMMO: m = pipebombblastradius;break;
                            case MORTER: m = morterblastradius;break;
#ifdef RRRA
                            case CHEERBOMB: m = morterblastradius;break;
#endif
                        }

                        if(sector[s->sectnum].lotag != 800)
                        {
                            hitradius( i, m,x>>2,x>>1,x-(x>>2),x);
                            spawn(i,EXPLOSION2);
#ifdef RRRA
                            if (s->picnum == CHEERBOMB)
                                spawn(i,BURNING);
#endif
                            spritesound(PIPEBOMB_EXPLODE,i);
                            for(x=0;x<8;x++)
                                RANDOMSCRAP;
                        }
                    }

                    if(s->yrepeat)
                    {
                        s->yrepeat = 0;
                        goto BOLT;
                    }

                    if(t[4] > 20) KILLIT(i);
#ifdef RRRA
                    if (s->picnum == CHEERBOMB)
                    {
                        spawn(i,BURNING);
                        KILLIT(i);
                    }
#endif
                }
                else if(s->picnum == HEAVYHBOMB && x < 788 && t[0] > 7 && s->xvel == 0)
                    if( cansee(s->x,s->y,s->z-(8<<8),s->sectnum,ps[p].posx,ps[p].posy,ps[p].posz,ps[p].cursectnum) )
                        if(ps[p].ammo_amount[HANDBOMB_WEAPON] < max_ammo_amount[HANDBOMB_WEAPON])
                            if(s->pal == 0)
                {
                    if(ud.coop >= 1)
                    {
                        for(j=0;j<ps[p].weapreccnt;j++)
                            if(ps[p].weaprecs[j] == i)
                                goto BOLT;

                        if(ps[p].weapreccnt < 255)
                            ps[p].weaprecs[ps[p].weapreccnt++] = i;
                    }

                    addammo(HANDBOMB_WEAPON,&ps[p],1);
                    addammo(RPG_WEAPON,&ps[p],1);
                    spritesound(DUKE_GET,ps[p].i);

                    if( ps[p].gotweapon[HANDBOMB_WEAPON] == 0 || s->owner == ps[p].i )
                        addweapon(&ps[p],HANDBOMB_WEAPON);

                    if( sprite[s->owner].picnum != APLAYER )
                    {
                        ps[p].pals[0] = 0;
                        ps[p].pals[1] = 32;
                        ps[p].pals[2] = 0;
                        ps[p].pals_time = 32;
                    }

                    if( hittype[s->owner].picnum != HEAVYHBOMB || ud.respawn_items == 0 || sprite[s->owner].picnum == APLAYER )
                    {
                        if(s->picnum == HEAVYHBOMB &&
                        sprite[s->owner].picnum != APLAYER && ud.coop )
                            goto BOLT;
                        KILLIT(i);
                    }
                    else
                    {
                        t[2] = respawnitemtime;
                        spawn(i,RESPAWNMARKERRED);
                        s->cstat = (short) 32768;
                    }
                }

                if(t[0] < 8) t[0]++;
                goto BOLT;
                
            case REACTORBURNT:
            case REACTOR2BURNT:
                goto BOLT;

            case REACTOR:
            case REACTOR2:

                if( t[4] == 1 )
                {
                    j = headspritesect[sect];
                    while(j >= 0)
                    {
                        switch(sprite[j].picnum)
                        {
                            case SECTOREFFECTOR:
                                if(sprite[j].lotag == 1)
                                {
                                    sprite[j].lotag = (short) 65535;
                                    sprite[j].hitag = (short) 65535;
                                }
                                break;
                            case REACTOR:
                                sprite[j].picnum = REACTORBURNT;
                                break;
                            case REACTOR2:
                                sprite[j].picnum = REACTOR2BURNT;
                                break;
                            case REACTORSPARK:
                            case REACTOR2SPARK:
                                sprite[j].cstat = (short) 32768;
                                break;
                        }
                        j = nextspritesect[j];
                    }
                    goto BOLT;
                }

                if(t[1] >= 20)
                {
                    t[4] = 1;
                    goto BOLT;
                }

                p = findplayer(s,&x);

                t[2]++;
                if( t[2] == 4 ) t[2]=0;

                if( x < 4096 )
                {
                    if( (TRAND&255) < 16 )
                    {
                        if(Sound[DUKE_LONGTERM_PAIN].num < 1)
                            spritesound(DUKE_LONGTERM_PAIN,ps[p].i);

                        spritesound(SHORT_CIRCUIT,i);

                        sprite[ps[p].i].extra --;
                        ps[p].pals_time = 32;
                        ps[p].pals[0] = 32;
                        ps[p].pals[1] = 0;
                        ps[p].pals[2] = 0;
                    }
                    t[0] += 128;
                    if( t[3] == 0 )
                        t[3] = 1;
                }
                else t[3] = 0;

                if( t[1] )
                {
                    t[1]++;

                    t[4] = s->z;
                    s->z = sector[sect].floorz-(TRAND%(sector[sect].floorz-sector[sect].ceilingz));

                    switch( t[1] )
                    {
                        case 3:
                            //Turn on all of those flashing sectoreffector.
                            hitradius( i, 4096,
                                       impact_damage<<2,
                                       impact_damage<<2,
                                       impact_damage<<2,
                                       impact_damage<<2 );
/*
                            j = headspritestat[3];
                            while(j>=0)
                            {
                                if( sprite[j].lotag  == 3 )
                                    hittype[j].temp_data[4]=1;
                                else if(sprite[j].lotag == 12)
                                {
                                    hittype[j].temp_data[4] = 1;
                                    sprite[j].lotag = 3;
                                    sprite[j].owner = 0;
                                    hittype[j].temp_data[0] = s->shade;
                                }
                                j = nextspritestat[j];
                            }
*/
                            j = headspritestat[6];
                            while(j >= 0)
                            {
                                if(sprite[j].picnum == MASTERSWITCH)
                                    if(sprite[j].hitag == s->hitag)
                                        if(sprite[j].yvel == 0)
                                            sprite[j].yvel = 1;
                                j = nextspritestat[j];
                            }
                            break;

                        case 4:
                        case 7:
                        case 10:
                        case 15:
                            j = headspritesect[sect];
                            while(j >= 0)
                            {
                                l = nextspritesect[j];

                                if(j != i)
                                {
                                    deletesprite(j);
                                    break;
                                }
                                j = l;
                            }
                            break;
                    }
                    for(x=0;x<16;x++)
                        RANDOMSCRAP;

                    s->z = t[4];
                    t[4] = 0;

                }
                else
                {
                    IFHIT
                    {
                        for(x=0;x<32;x++)
                            RANDOMSCRAP;
                        if(s->extra < 0)
                            t[1] = 1;
                    }
                }
                goto BOLT;

            case CAMERA1:

                if( t[0] == 0 )
                {
                    t[1]+=8;
                    if(camerashitable)
                    {
                        IFHIT
                        {
                            t[0] = 1; // static
                            s->cstat = (short)32768;
                            for(x=0;x<5;x++) RANDOMSCRAP;
                            goto BOLT;
                        }
                    }

                    if(s->hitag > 0)
                    {
                        if(t[1]<s->hitag)
                            s->ang+=8;
                        else if(t[1]<(s->hitag*3))
                            s->ang-=8;
                        else if(t[1] < (s->hitag<<2) )
                            s->ang+=8;
                        else
                        {
                            t[1]=8;
                            s->ang+=16;
                        }
                    }
                }
                goto BOLT;
        }


// #ifndef VOLOMEONE
        if( ud.multimode < 2 && badguy(s) )
        {
            if( actor_tog == 1)
            {
                s->cstat = (short)32768;
                goto BOLT;
            }
            else if(actor_tog == 2) s->cstat = 257;
        }
// #endif

        p = findplayer(s,&x);

        execute(i,p,x);

        BOLT:

        i = nexti;
    }

}

void ballreturn(short spr)
{
    short j, i, nexti, nextj;
    i = headspritestat[105];
    while (i >= 0)
    {
        nexti = nextspritestat[i];
        if (sprite[i].picnum == RRTILE281)
            if (sprite[spr].sectnum == sprite[i].sectnum)
        {
            j = headspritestat[105];
            while (j >= 0)
            {
                nextj = nextspritestat[j];
                if (sprite[j].picnum == RRTILE282)
                    if (sprite[i].hitag == sprite[j].hitag)
                    spawn(j, BOWLINGBALLSPRITE);
                if (sprite[j].picnum == RRTILE280)
                    if (sprite[i].hitag == sprite[j].hitag)
                        if (sprite[j].lotag == 0)
                {
                    sprite[j].lotag = 100;
                    sprite[j].extra++;
                    pinsectorresetdown(sprite[j].sectnum);
                }
                j = nextj;
            }
        }

        i = nexti;
    }
}

short pinsectorresetdown(short sect)
{
    int vel, j;
    
    j = getanimationgoal(&sector[sect].ceilingz);

    if (j == -1)
    {
        j = sector[sect].floorz;
        vel = 64;
        setanimation(sect,&sector[sect].ceilingz,j,vel);
        return 1;
    }
    return 0;
}

short pinsectorresetup(short sect)
{
    int vel, j;
    
    j = getanimationgoal(&sector[sect].ceilingz);

    if (j == -1)
    {
        j = sector[nextsectorneighborz(sect,sector[sect].ceilingz,-1,-1)].ceilingz;
        vel = 64;
        setanimation(sect,&sector[sect].ceilingz,j,vel);
        return 1;
    }
    return 0;
}

short checkpins(short sect)
{
    short i, pin;
    int  x, y;
    short pins[10];
    short nexti, tag;
    
    pin = 0;
    for(i=0;i<10;i++) pins[i] = 0;

    i = headspritesect[sect];

    while (i >= 0)
    {
        nexti = nextspritesect[i];

        if (sprite[i].picnum == RRTILE3440)
        {
            pin++;
            pins[sprite[i].lotag] = 1;
        }
        if (sprite[i].picnum == RRTILE280)
        {
            tag = sprite[i].hitag;
        }

        i = nexti;
    }

    if (tag)
    {
        tag += 2024;
        copytilepiece(2024,0,0,128,64,tag,0,0);
        for(i=0;i<10;i++)
        {
            if (pins[i] == 1)
            {
                switch (i)
                {
                    case 0:
                        x = 64;
                        y = 48;
                        break;
                    case 1:
                        x = 56;
                        y = 40;
                        break;
                    case 2:
                        x = 72;
                        y = 40;
                        break;
                    case 3:
                        x = 48;
                        y = 32;
                        break;
                    case 4:
                        x = 64;
                        y = 32;
                        break;
                    case 5:
                        x = 80;
                        y = 32;
                        break;
                    case 6:
                        x = 40;
                        y = 24;
                        break;
                    case 7:
                        x = 56;
                        y = 24;
                        break;
                    case 8:
                        x = 72;
                        y = 24;
                        break;
                    case 9:
                        x = 88;
                        y = 24;
                        break;
                }
                copytilepiece(2023,0,0,8,8,tag,x-4,y-10);
            }
        }
    }

    return pin;
}

void resetpins(short sect)
{
    short i, j, nexti, tag;
    int x, y;
    i = headspritesect[sect];
    while (i >= 0)
    {
        nexti = headspritesect[i];
        if (sprite[i].picnum == 3440)
            deletesprite(i);
        i = nexti;
    }
    i = headspritesect[sect];
    while (i >= 0)
    {
        nexti = nextspritesect[i];
        if (sprite[i].picnum == 283)
        {
            j = spawn(i,3440);
            sprite[j].lotag = sprite[i].lotag;
            if (sprite[j].lotag == 3 || sprite[j].lotag == 5)
            {
                sprite[j].clipdist = (1+(TRAND%1))*16+32;
            }
            else
            {
                sprite[j].clipdist = (1+(TRAND%1))*16+32;
            }
            sprite[j].ang -= ((TRAND&32)-(TRAND&64))&2047;
        }
        if (sprite[i].picnum == 280)
            tag = sprite[i].hitag;
        i = nexti;
    }
    if (tag)
    {
        tag += LANEPICS+1;
        copytilepiece(LANEPICS+1,0,0,128,64,tag,0,0);
        for(i=0;i<10;i++)
        {
            switch (i)
            {
                case 0:
                    x = 64;
                    y = 48;
                    break;
                case 1:
                    x = 56;
                    y = 40;
                    break;
                case 2:
                    x = 72;
                    y = 40;
                    break;
                case 3:
                    x = 48;
                    y = 32;
                    break;
                case 4:
                    x = 64;
                    y = 32;
                    break;
                case 5:
                    x = 80;
                    y = 32;
                    break;
                case 6:
                    x = 40;
                    y = 24;
                    break;
                case 7:
                    x = 56;
                    y = 24;
                    break;
                case 8:
                    x = 72;
                    y = 24;
                    break;
                case 9:
                    x = 88;
                    y = 24;
                    break;
            }
            copytilepiece(LANEPICS,0,0,8,8,tag,x-4,y-10);
        }
    }
}

void resetlanepics(void)
{
    int x, y;
    short i;
    short tag, pic;
    for(tag=0;tag<4;tag++)
    {
        pic = tag + 1;
        if (pic == 0) continue;
        pic += LANEPICS+1;
        copytilepiece(LANEPICS+1,0,0,128,64, pic,0,0);
        for(i=0;i<10;i++)
        {
            switch (i)
            {
                case 0:
                    x = 64;
                    y = 48;
                    break;
                case 1:
                    x = 56;
                    y = 40;
                    break;
                case 2:
                    x = 72;
                    y = 40;
                    break;
                case 3:
                    x = 48;
                    y = 32;
                    break;
                case 4:
                    x = 64;
                    y = 32;
                    break;
                case 5:
                    x = 80;
                    y = 32;
                    break;
                case 6:
                    x = 40;
                    y = 24;
                    break;
                case 7:
                    x = 56;
                    y = 24;
                    break;
                case 8:
                    x = 72;
                    y = 24;
                    break;
                case 9:
                    x = 88;
                    y = 24;
                    break;
            }
            copytilepiece(LANEPICS,0,0,8,8,pic,x-4,y-10);
        }
    }
}
