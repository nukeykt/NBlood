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

void operateactivators(short low,short snum)
{
    short i, j, k, *p, nexti;
    walltype *wal;

    for(i=numcyclers-1;i>=0;i--)
    {
        p = &cyclers[i][0];

        if(p[4] == low)
        {
            p[5] = !p[5];

            sector[p[0]].floorshade = sector[p[0]].ceilingshade = p[3];
            wal = &wall[sector[p[0]].wallptr];
            for(j=sector[p[0]].wallnum;j > 0;j--,wal++)
                wal->shade = p[3];
        }
    }

    i = headspritestat[8];
    k = -1;
    while(i >= 0)
    {
        if(sprite[i].lotag == low)
        {
            if( sprite[i].picnum == ACTIVATORLOCKED )
            {
                sector[SECT].lotag ^= 16384;

                if(snum >= 0)
                {
                    if(sector[SECT].lotag&16384)
                        FTA(4,&ps[snum]);
                    else FTA(8,&ps[snum]);
                }
            }
            else
            {
                switch(SHT)
                {
                    case 0:
                        break;
                    case 1:
                        if(sector[SECT].floorz != sector[SECT].ceilingz)
                        {
                            i = nextspritestat[i];
                            continue;
                        }
                        break;
                    case 2:
                        if(sector[SECT].floorz == sector[SECT].ceilingz)
                        {
                            i = nextspritestat[i];
                            continue;
                        }
                        break;
                }

                if( sector[sprite[i].sectnum].lotag < 3 )
                {
                    j = headspritesect[sprite[i].sectnum];
                    while(j >= 0)
                    {
                        if( sprite[j].statnum == 3 ) switch(sprite[j].lotag)
                        {
                            case 36:
                            case 31:
                            case 32:
#ifndef RRRA
                            case 18:
#endif
                                hittype[j].temp_data[0] = 1-hittype[j].temp_data[0];
                                callsound(SECT,j);
                                break;
                        }
                        j = nextspritesect[j];
                    }
                }

                if( k == -1 && (sector[SECT].lotag&0xff) == 22 )
                    k = callsound(SECT,i);

                operatesectors(SECT,i);
            }
        }
        i = nextspritestat[i];
     }

    operaterespawns(low);
}

void operatemasterswitches(short low)
{
    short i;

    i = headspritestat[6];
    while(i >= 0)
    {
        if( PN == MASTERSWITCH && SLT == low && SP == 0 )
            SP = 1;
        i = nextspritestat[i];
    }
}



void operateforcefields(short s, short low)
{
    short i, p;

    for(p=numanimwalls;p>=0;p--)
    {
        i = animwall[p].wallnum;

        if(low == wall[i].lotag || low == -1)
            switch(wall[i].overpicnum)
        {
            case BIGFORCE:

                animwall[p].tag = 0;

                if( wall[i].cstat )
                {
                    wall[i].cstat   = 0;

                    if( s >= 0 && sprite[s].picnum == SECTOREFFECTOR &&
                        sprite[s].lotag == 30)
                            wall[i].lotag = 0;
                }
                else
                    wall[i].cstat = 85;
                break;
        }
    }
}


char checkhitswitch(short snum,long w,char switchtype)
{
    char switchpal;
    short i, x, lotag,hitag,picnum,correctdips,numdips;
    long sx,sy;

    if(w < 0) return 0;
    correctdips = 1;
    numdips = 0;

    if(switchtype == 1) // A wall sprite
    {
        lotag = sprite[w].lotag; if(lotag == 0) return 0;
        hitag = sprite[w].hitag;
        sx = sprite[w].x;
        sy = sprite[w].y;
        picnum = sprite[w].picnum;
        switchpal = sprite[w].pal;
    }
    else
    {
        lotag = wall[w].lotag; if(lotag == 0) return 0;
        hitag = wall[w].hitag;
        sx = wall[w].x;
        sy = wall[w].y;
        picnum = wall[w].picnum;
        switchpal = wall[w].pal;
    }

    switch(picnum)
    {
        case DIPSWITCH:
        case DIPSWITCH+1:
        case TECHSWITCH:
        case TECHSWITCH+1:
        case ALIENSWITCH:
        case ALIENSWITCH+1:
            break;
        case ACCESSSWITCH:
        case ACCESSSWITCH2:
            if(ps[snum].access_incs == 0)
            {
                if( switchpal == 0 )
                {
                    if( ps[snum].keys[1] )
                        ps[snum].access_incs = 1;
#ifdef RRRA
                    else
                    {
                        FTA(70,&ps[snum]);
                        spritesound(99,w);
                    }
#else
                    else FTA(70,&ps[snum]);
#endif
                }

                else if( switchpal == 21 )
                {
                    if( ps[snum].keys[2] )
                        ps[snum].access_incs = 1;
#ifdef RRRA
                    else
                    {
                        FTA(71,&ps[snum]);
                        spritesound(99,w);
                    }
#else
                    else FTA(71,&ps[snum]);
#endif
                }

                else if( switchpal == 23 )
                {
                    if( ps[snum].keys[3] )
                        ps[snum].access_incs = 1;
#ifdef RRRA
                    else
                    {
                        FTA(72,&ps[snum]);
                        spritesound(99,w);
                    }
#else
                    else FTA(72,&ps[snum]);
#endif
                }

                if( ps[snum].access_incs == 1 )
                {
                    if(switchtype == 0)
                        ps[snum].access_wallnum = w;
                    else
                        ps[snum].access_spritenum = w;
                }

                return 0;
            }
        case DIPSWITCH2:
        case DIPSWITCH2+1:
        case DIPSWITCH3:
        case DIPSWITCH3+1:
        case MULTISWITCH:
        case MULTISWITCH+1:
        case MULTISWITCH+2:
        case MULTISWITCH+3:
        case PULLSWITCH:
        case PULLSWITCH+1:
        case HANDSWITCH:
        case HANDSWITCH+1:
        case SLOTDOOR:
        case SLOTDOOR+1:
        case LIGHTSWITCH:
        case LIGHTSWITCH+1:
        case SPACELIGHTSWITCH:
        case SPACELIGHTSWITCH+1:
        case SPACEDOORSWITCH:
        case SPACEDOORSWITCH+1:
        case FRANKENSTINESWITCH:
        case FRANKENSTINESWITCH+1:
        case LIGHTSWITCH2:
        case LIGHTSWITCH2+1:
        case POWERSWITCH1:
        case POWERSWITCH1+1:
        case LOCKSWITCH1:
        case LOCKSWITCH1+1:
        case POWERSWITCH2:
        case POWERSWITCH2+1:
        case NUKEBUTTON:
        case NUKEBUTTON+1:
        case RRTILE2214:
        case RRTILE2697:
        case RRTILE2697+1:
        case RRTILE2707:
        case RRTILE2707+1:
#ifdef RRRA
        case MULTISWITCH2:
        case MULTISWITCH2+1:
        case MULTISWITCH2+2:
        case MULTISWITCH2+3:
        case RRTILE8464:
        case RRTILE8660:
#endif
            if( check_activator_motion( lotag ) ) return 0;
            break;
        default:
            if( isadoorwall(picnum) == 0 ) return 0;
            break;
    }

    i = headspritestat[0];
    while(i >= 0)
    {
        if( lotag == SLT ) switch(PN)
        {
            case DIPSWITCH:
            case TECHSWITCH:
            case ALIENSWITCH:
                if( switchtype == 1 && w == i ) PN++;
                else if( SHT == 0 ) correctdips++;
                numdips++;
                break;
            case TECHSWITCH+1:
            case DIPSWITCH+1:
            case ALIENSWITCH+1:
                if( switchtype == 1 && w == i ) PN--;
                else if( SHT == 1 ) correctdips++;
                numdips++;
                break;
            case MULTISWITCH:
            case MULTISWITCH+1:
            case MULTISWITCH+2:
            case MULTISWITCH+3:
                sprite[i].picnum++;
                if( sprite[i].picnum > (MULTISWITCH+3) )
                    sprite[i].picnum = MULTISWITCH;
                break;
#ifdef RRRA
            case MULTISWITCH2:
            case MULTISWITCH2+1:
            case MULTISWITCH2+2:
            case MULTISWITCH2+3:
                sprite[i].picnum++;
                if( sprite[i].picnum > (MULTISWITCH2+3) )
                    sprite[i].picnum = MULTISWITCH2;
                break;
#endif
            case RRTILE2214:
                if (ud.level_number > 6)
                    ud.level_number = 0;
                sprite[i].picnum++;
                break;
            case ACCESSSWITCH:
            case ACCESSSWITCH2:
            case SLOTDOOR:
            case LIGHTSWITCH:
            case SPACELIGHTSWITCH:
            case SPACEDOORSWITCH:
            case FRANKENSTINESWITCH:
            case LIGHTSWITCH2:
            case POWERSWITCH1:
            case LOCKSWITCH1:
            case POWERSWITCH2:
            case HANDSWITCH:
            case PULLSWITCH:
            case DIPSWITCH2:
            case DIPSWITCH3:
            case NUKEBUTTON:
            case RRTILE2697:
            case RRTILE2707:
#ifdef RRRA
            case RRTILE8660:
#endif
                if (PN == DIPSWITCH3)
                    if(SHT == 999)
                {
                    short j, nextj;
                    j = headspritestat[107];
                    while (j >= 0)
                    {
                        nextj = nextspritestat[j];
                        if (sprite[j].picnum == RRTILE3410)
                        {
                            sprite[j].picnum++;
                            sprite[j].hitag = 100;
                            sprite[j].extra = 0;
                            spritesound(474,j);
                        }
                        else if (sprite[j].picnum == RRTILE295)
                            deletesprite(j);
                        j = nextj;
                    }
                    sprite[i].picnum++;
                    break;
                }
                if (PN == NUKEBUTTON)
                    chickenplant = 0;
#ifdef RRRA
                if (PN == RRTILE8660)
                {
                    BellTime = 132;
                    word_119BE0 = i;
                }
#endif
                sprite[i].picnum++;
                break;
            case PULLSWITCH+1:
            case HANDSWITCH+1:
            case LIGHTSWITCH2+1:
            case POWERSWITCH1+1:
            case LOCKSWITCH1+1:
            case POWERSWITCH2+1:
            case SLOTDOOR+1:
            case LIGHTSWITCH+1:
            case SPACELIGHTSWITCH+1:
            case SPACEDOORSWITCH+1:
            case FRANKENSTINESWITCH+1:
            case DIPSWITCH2+1:
            case DIPSWITCH3+1:
            case NUKEBUTTON+1:
            case RRTILE2697+1:
            case RRTILE2707+1:
                if (PN == NUKEBUTTON+1)
                    chickenplant = 1;
                if (SHT != 999)
                    sprite[i].picnum--;
                break;
        }
        i = nextspritestat[i];
    }

    for(i=0;i<numwalls;i++)
    {
        x = i;
        if(lotag == wall[x].lotag)
            switch(wall[x].picnum)
        {
            case DIPSWITCH:
            case TECHSWITCH:
            case ALIENSWITCH:
                if( switchtype == 0 && i == w ) wall[x].picnum++;
                else if( wall[x].hitag == 0 ) correctdips++;
                numdips++;
                break;
            case DIPSWITCH+1:
            case TECHSWITCH+1:
            case ALIENSWITCH+1:
                if( switchtype == 0 && i == w ) wall[x].picnum--;
                else if( wall[x].hitag == 1 ) correctdips++;
                numdips++;
                break;
            case MULTISWITCH:
            case MULTISWITCH+1:
            case MULTISWITCH+2:
            case MULTISWITCH+3:
                wall[x].picnum++;
                if(wall[x].picnum > (MULTISWITCH+3) )
                    wall[x].picnum = MULTISWITCH;
                break;
#ifdef RRRA
            case MULTISWITCH2:
            case MULTISWITCH2+1:
            case MULTISWITCH2+2:
            case MULTISWITCH2+3:
                wall[x].picnum++;
                if(wall[x].picnum > (MULTISWITCH2+3) )
                    wall[x].picnum = MULTISWITCH2;
                break;
#endif
            case ACCESSSWITCH:
            case ACCESSSWITCH2:
            case SLOTDOOR:
            case LIGHTSWITCH:
            case SPACELIGHTSWITCH:
            case SPACEDOORSWITCH:
            case LIGHTSWITCH2:
            case POWERSWITCH1:
            case LOCKSWITCH1:
            case POWERSWITCH2:
            case PULLSWITCH:
            case HANDSWITCH:
            case DIPSWITCH2:
            case DIPSWITCH3:
            case RRTILE2697:
            case RRTILE2707:
#ifdef RRRA
            case RRTILE8660:
#endif
                wall[x].picnum++;
                break;
            case HANDSWITCH+1:
            case PULLSWITCH+1:
            case LIGHTSWITCH2+1:
            case POWERSWITCH1+1:
            case LOCKSWITCH1+1:
            case POWERSWITCH2+1:
            case SLOTDOOR+1:
            case LIGHTSWITCH+1:
            case SPACELIGHTSWITCH+1:
            case SPACEDOORSWITCH+1:
            case DIPSWITCH2+1:
            case DIPSWITCH3+1:
            case RRTILE2697+1:
            case RRTILE2707+1:
                wall[x].picnum--;
                break;
        }
    }

    if(lotag == (short) 65535)
    {
        ps[myconnectindex].gm = MODE_EOL;
        if(ud.from_bonus)
        {
            ud.level_number = ud.from_bonus;
            ud.m_level_number = ud.level_number;
            ud.from_bonus = 0;
        }
        else
        {
            ud.level_number++;
            if( (ud.volume_number && ud.level_number > 6 ) || ( ud.volume_number == 0 && ud.level_number > 6 ) )
                ud.level_number = 0;
            ud.m_level_number = ud.level_number;
        }
    }

    switch(picnum)
    {
        default:
            if(isadoorwall(picnum) == 0) break;
        case DIPSWITCH:
        case DIPSWITCH+1:
        case TECHSWITCH:
        case TECHSWITCH+1:
        case ALIENSWITCH:
        case ALIENSWITCH+1:
            if( picnum == DIPSWITCH  || picnum == DIPSWITCH+1 ||
                picnum == ALIENSWITCH || picnum == ALIENSWITCH+1 ||
                picnum == TECHSWITCH || picnum == TECHSWITCH+1 )
            {
                if( picnum == ALIENSWITCH || picnum == ALIENSWITCH+1)
                {
                    if(switchtype == 1)
                        xyzsound(ALIEN_SWITCH1,w,sx,sy,ps[snum].posz);
                    else xyzsound(ALIEN_SWITCH1,ps[snum].i,sx,sy,ps[snum].posz);
                }
                else
                {
                    if(switchtype == 1)
                        xyzsound(SWITCH_ON,w,sx,sy,ps[snum].posz);
                    else xyzsound(SWITCH_ON,ps[snum].i,sx,sy,ps[snum].posz);
                }
                if(numdips != correctdips) break;
                xyzsound(END_OF_LEVEL_WARN,ps[snum].i,sx,sy,ps[snum].posz);
            }
        case DIPSWITCH2:
        case DIPSWITCH2+1:
        case DIPSWITCH3:
        case DIPSWITCH3+1:
        case MULTISWITCH:
        case MULTISWITCH+1:
        case MULTISWITCH+2:
        case MULTISWITCH+3:
        case ACCESSSWITCH:
        case ACCESSSWITCH2:
        case SLOTDOOR:
        case SLOTDOOR+1:
        case LIGHTSWITCH:
        case LIGHTSWITCH+1:
        case SPACELIGHTSWITCH:
        case SPACELIGHTSWITCH+1:
        case SPACEDOORSWITCH:
        case SPACEDOORSWITCH+1:
        case FRANKENSTINESWITCH:
        case FRANKENSTINESWITCH+1:
        case LIGHTSWITCH2:
        case LIGHTSWITCH2+1:
        case POWERSWITCH1:
        case POWERSWITCH1+1:
        case LOCKSWITCH1:
        case LOCKSWITCH1+1:
        case POWERSWITCH2:
        case POWERSWITCH2+1:
        case HANDSWITCH:
        case HANDSWITCH+1:
        case PULLSWITCH:
        case PULLSWITCH+1:
        case RRTILE2697:
        case RRTILE2697+1:
        case RRTILE2707:
        case RRTILE2707+1:
#ifdef RRRA
        case MULTISWITCH2:
        case MULTISWITCH2+1:
        case MULTISWITCH2+2:
        case MULTISWITCH2+3:
        case RRTILE8464:
        case RRTILE8660:
#endif
#ifdef RRRA
            if (picnum == RRTILE8660)
            {
                BellTime = 132;
                word_119BE0 = w;
                sprite[w].picnum++;
            }
            else if (picnum == RRTILE8464)
            {
                sprite[w].picnum = sprite[w].picnum+1;
                if (hitag == 10001)
                {
                    if (ps[snum].SeaSick == 0)
                        ps[snum].SeaSick = 350;
                    operateactivators(668,ps[snum].i);
                    operatemasterswitches(668);
                    spritesound(328,ps[snum].i);
                    return 1;
                }
            }
            else if (hitag == 10000)
            {
                if( picnum == MULTISWITCH || picnum == (MULTISWITCH+1) ||
                    picnum == (MULTISWITCH+2) || picnum == (MULTISWITCH+3) ||
                    picnum == MULTISWITCH2 || picnum == (MULTISWITCH2+1) ||
                    picnum == (MULTISWITCH2+2) || picnum == (MULTISWITCH2+3) )
                {
                    int var6c[3], var54, j;
                    short jpn, jht;
                    var54 = 0;
                    xyzsound(SWITCH_ON,w,sx,sy,ps[snum].posz);
                    for (j = 0; j < MAXSPRITES; j++)
                    {
                        jpn = sprite[j].picnum;
                        jht = sprite[j].hitag;
                        if ((jpn == MULTISWITCH || jpn == MULTISWITCH2) && jht == 10000)
                        {
                            if (var54 < 3)
                            {
                                var6c[var54] = j;
                                var54++;
                            }
                        }
                    }
                    if (var54 == 3)
                    {
                        xyzsound(78,w,sx,sy,ps[snum].posz);
                        for (j = 0; j < var54; j++)
                        {
                            sprite[var6c[j]].hitag = 0;
                            if (picnum >= MULTISWITCH2)
                                sprite[var6c[j]].picnum = MULTISWITCH2+3;
                            else
                                sprite[var6c[j]].picnum = MULTISWITCH+3;
                            checkhitswitch(snum,var6c[j],1);
                        }
                    }
                    return 1;
                }
            }
#endif
                if( picnum == MULTISWITCH || picnum == (MULTISWITCH+1) ||
                    picnum == (MULTISWITCH+2) || picnum == (MULTISWITCH+3) )
                        lotag += picnum-MULTISWITCH;
#ifdef RRRA
                if( picnum == MULTISWITCH2 || picnum == (MULTISWITCH2+1) ||
                    picnum == (MULTISWITCH2+2) || picnum == (MULTISWITCH2+3) )
                        lotag += picnum-MULTISWITCH2;
#endif

                x = headspritestat[3];
                while(x >= 0)
                {
                   if( ((sprite[x].hitag) == lotag) )
                   {
                       switch(sprite[x].lotag)
                       {
                           case 12:
#ifdef RRRA
                           case 46:
                           case 47:
                           case 48:
#endif
                                sector[sprite[x].sectnum].floorpal = 0;
                                hittype[x].temp_data[0]++;
                                if(hittype[x].temp_data[0] == 2)
                                   hittype[x].temp_data[0]++;

                                break;
                           case 24:
                           case 34:
                           case 25:
                               hittype[x].temp_data[4] = !hittype[x].temp_data[4];
                               if(hittype[x].temp_data[4])
                                   FTA(15,&ps[snum]);
                               else FTA(2,&ps[snum]);
                               break;
                           case 21:
                               FTA(2,&ps[screenpeek]);
                               break;
                       }
                   }
                   x = nextspritestat[x];
                }

                operateactivators(lotag,snum);
                operateforcefields(ps[snum].i,lotag);
                operatemasterswitches(lotag);

                if( picnum == DIPSWITCH || picnum == DIPSWITCH+1 ||
                    picnum == ALIENSWITCH || picnum == ALIENSWITCH+1 ||
                    picnum == TECHSWITCH || picnum == TECHSWITCH+1 ) return 1;

                if( hitag == 0 && isadoorwall(picnum) == 0 )
                {
                    if(switchtype == 1)
                        xyzsound(SWITCH_ON,w,sx,sy,ps[snum].posz);
                    else xyzsound(SWITCH_ON,ps[snum].i,sx,sy,ps[snum].posz);
                }
                else if(hitag != 0)
                {
                    if(switchtype == 1 && (soundm[hitag]&4) == 0)
                        xyzsound(hitag,w,sx,sy,ps[snum].posz);
                    else spritesound(hitag,ps[snum].i);
                }

               return 1;
    }
    return 0;
}


void activatebysector(short sect,short j)
{
    short i;

    i = headspritesect[sect];
    while(i >= 0)
    {
        if(PN == ACTIVATOR)
        {
            operateactivators(SLT,-1);
//            return;
        }
        i = nextspritesect[i];
    }

    if(sector[sect].lotag != 22)
        operatesectors(sect,j);
}

void breakwall(short newpn,short spr,short dawallnum)
{
    wall[dawallnum].picnum = newpn;
    spritesound(VENT_BUST,spr);
    spritesound(GLASS_HEAVYBREAK,spr);
    lotsofglass(spr,dawallnum,10);
}

void checkhitwall(short spr,short dawallnum,long x,long y,long z,short atwith)
{
    short j, i, sn = -1, darkestwall;
    signed char nfloors,nceilings;
    short nextj;
    walltype *wal;
    spritetype *s;

    wal = &wall[dawallnum];

    if(wal->overpicnum == MIRROR)
    {
        switch(atwith)
        {
            case HEAVYHBOMB:
            case RADIUSEXPLOSION:
            case RPG:
            case HYDRENT:
            case SEENINE:
            case OOZFILTER:
            case EXPLODINGBARREL:
#ifdef RRRA
            case RPG2:
#endif
                lotsofglass(spr,dawallnum,70);
                wal->cstat &= ~16;
                wal->overpicnum = MIRRORBROKE;
                spritesound(GLASS_HEAVYBREAK,spr);
                return;
        }
    }

   if( ( (wal->cstat&16) || wal->overpicnum == BIGFORCE ) && wal->nextsector >= 0 )
       if( sector[wal->nextsector].floorz > z )
           if( sector[wal->nextsector].floorz-sector[wal->nextsector].ceilingz )
               switch(wal->overpicnum)
    {
        case FANSPRITE:
            wal->overpicnum = FANSPRITEBROKE;
            wal->cstat &= 65535-65;
            if(wal->nextwall >= 0)
            {
                wall[wal->nextwall].overpicnum = FANSPRITEBROKE;
                wall[wal->nextwall].cstat &= 65535-65;
            }
            spritesound(VENT_BUST,spr);
            spritesound(GLASS_BREAKING,spr);
            return;

        case RRTILE1973:
            updatesector(x,y,&sn); if( sn < 0 ) return;
            wal->overpicnum=GLASS2;
            lotsofpopcorn(spr,dawallnum,64);
            wal->cstat = 0;

            if(wal->nextwall >= 0)
                wall[wal->nextwall].cstat = 0;

            i = EGS(sn,x,y,z,SECTOREFFECTOR,0,0,0,ps[0].ang,0,0,spr,3);
            SLT = 128; T2 = 2; T3 = dawallnum;
            spritesound(GLASS_BREAKING,i);
            return;

        case GLASS:
            updatesector(x,y,&sn); if( sn < 0 ) return;
            wal->overpicnum=GLASS2;
            lotsofglass(spr,dawallnum,10);
            wal->cstat = 0;

            if(wal->nextwall >= 0)
                wall[wal->nextwall].cstat = 0;

            i = EGS(sn,x,y,z,SECTOREFFECTOR,0,0,0,ps[0].ang,0,0,spr,3);
            SLT = 128; T2 = 2; T3 = dawallnum;
            spritesound(GLASS_BREAKING,i);
            return;
        case STAINGLASS1:
            updatesector(x,y,&sn); if( sn < 0 ) return;
            lotsofcolourglass(spr,dawallnum,80);
            wal->cstat = 0;
            if(wal->nextwall >= 0)
                wall[wal->nextwall].cstat = 0;
            spritesound(VENT_BUST,spr);
            spritesound(GLASS_BREAKING,spr);
            return;
    }

    switch(wal->picnum)
    {
#ifdef RRRA
            case RRTILE8464:
                break;
#endif
            case RRTILE3643:
            case RRTILE3643+1:
            case RRTILE3643+2:
            case RRTILE3643+3:
            {
                short sect;
                short unk = 0;
                short jj;
                short nextjj;
                short startwall, endwall;
                sect = wall[wal->nextwall].nextsector;
                jj = headspritesect[sect];
                while (jj != -1)
                {
                    nextjj = nextspritesect[jj];
                    s = &sprite[jj];
                    if (s->lotag == 6)
                    {
                        for (j = 0; j < 16; j++) RANDOMSCRAP;
                        s->filler++;
                        if (s->filler == 25)
                        {
                            startwall = sector[s->sectnum].wallptr;
                            endwall = startwall+sector[s->sectnum].wallnum;
                            for(i=startwall;i<endwall;i++)
                                sector[wall[i].nextsector].lotag = 0;
                            sector[s->sectnum].lotag = 0;
                            stopsound(sprite[jj].lotag);
                            spritesound(400,jj);
                            deletesprite(jj);
                        }
                    }
                    jj = nextjj;
                }
                return;
            }
#ifdef RRRA
            case RRTILE7555:
                wal->picnum = SBMOVE;
                spritesound(GLASS_HEAVYBREAK,spr);
                return;
            case RRTILE7441:
                wal->picnum = RRTILE5016;
                spritesound(GLASS_HEAVYBREAK,spr);
                return;
            case RRTILE7559:
                wal->picnum = RRTILE5017;
                spritesound(GLASS_HEAVYBREAK,spr);
                return;
            case RRTILE7433:
                wal->picnum = RRTILE5018;
                spritesound(GLASS_HEAVYBREAK,spr);
                return;
            case RRTILE7557:
                wal->picnum = RRTILE5019;
                spritesound(GLASS_HEAVYBREAK,spr);
                return;
            case RRTILE7553:
                wal->picnum = RRTILE5020;
                spritesound(GLASS_HEAVYBREAK,spr);
                return;
            case RRTILE7552:
                wal->picnum = RRTILE5021;
                spritesound(GLASS_HEAVYBREAK,spr);
                return;
            case RRTILE7568:
                wal->picnum = RRTILE5022;
                spritesound(GLASS_HEAVYBREAK,spr);
                return;
            case RRTILE7540:
                wal->picnum = RRTILE5023;
                spritesound(GLASS_HEAVYBREAK,spr);
                return;
            case RRTILE7558:
                wal->picnum = RRTILE5024;
                spritesound(GLASS_HEAVYBREAK,spr);
                return;
            case RRTILE7554:
                wal->picnum = RRTILE5025;
                spritesound(GLASS_HEAVYBREAK,spr);
                return;
            case RRTILE7579:
                wal->picnum = RRTILE5026;
                spritesound(GLASS_HEAVYBREAK,spr);
                return;
            case RRTILE7561:
                wal->picnum = RRTILE5027;
                spritesound(GLASS_HEAVYBREAK,spr);
                return;
            case RRTILE7580:
                wal->picnum = RRTILE5037;
                spritesound(GLASS_HEAVYBREAK,spr);
                return;
            case RRTILE8227:
                wal->picnum = RRTILE5070;
                spritesound(GLASS_HEAVYBREAK,spr);
                return;
            case RRTILE8503:
                wal->picnum = RRTILE5079;
                spritesound(GLASS_HEAVYBREAK,spr);
                return;
            case RRTILE8567:
            case RRTILE8568:
            case RRTILE8569:
            case RRTILE8570:
            case RRTILE8571:
                wal->picnum = RRTILE5082;
                spritesound(GLASS_HEAVYBREAK,spr);
                return;
            case RRTILE7859:
                wal->picnum = RRTILE5081;
                spritesound(GLASS_HEAVYBREAK,spr);
                return;
            case RRTILE8496:
                wal->picnum = RRTILE5061;
                spritesound(GLASS_HEAVYBREAK,spr);
                return;
            case RRTILE8617:
                if (numplayers < 2)
                {
                    wal->picnum = RRTILE8618;
                    spritesound(47,spr);
                }
                return;
            case RRTILE8620:
                wal->picnum = RRTILE8621;
                spritesound(47,spr);
                return;
            case RRTILE8622:
                wal->picnum = RRTILE8623;
                spritesound(495,spr);
                return;
            case RRTILE7657:
                wal->picnum = RRTILE7659;
                spritesound(GLASS_HEAVYBREAK,spr);
                return;
            case RRTILE8497:
                wal->picnum = RRTILE5076;
                spritesound(495,spr);
                return;
            case RRTILE7533:
                wal->picnum = RRTILE5035;
                spritesound(495,spr);
                return;
#endif

            case COLAMACHINE:
            case VENDMACHINE:
                breakwall(wal->picnum+2,spr,dawallnum);
                spritesound(GLASS_BREAKING,spr);
                return;

            case OJ:

            case SCREENBREAK6:
            case SCREENBREAK7:
            case SCREENBREAK8:

                lotsofglass(spr,dawallnum,30);
#ifdef RRRA
                wal->picnum=W_SCREENBREAK+(TRAND%2);
#else
                wal->picnum=W_SCREENBREAK+(TRAND%3);
#endif
                spritesound(GLASS_HEAVYBREAK,spr);
                return;

            case ATM:
                wal->picnum = ATMBROKE;
                lotsofmoney(&sprite[spr],1+(TRAND&7));
                spritesound(GLASS_HEAVYBREAK,spr);
                break;

            case WALLLIGHT1:
            case WALLLIGHT3:
            case WALLLIGHT4:
            case TECHLIGHT2:
            case TECHLIGHT4:
            case RRTILE1814:
            case RRTILE1939:
            case RRTILE1986:
            case RRTILE1988:
            case RRTILE2123:
            case RRTILE2125:
            case RRTILE2636:
            case RRTILE2878:
            case RRTILE2898:
            case RRTILE3200:
            case RRTILE3202:
            case RRTILE3204:
            case RRTILE3206:
            case RRTILE3208:

                if( rnd(128) )
                    spritesound(GLASS_HEAVYBREAK,spr);
                else spritesound(GLASS_BREAKING,spr);
                lotsofglass(spr,dawallnum,30);

                if(wal->picnum == RRTILE1814)
                    wal->picnum = RRTILE1817;

                if(wal->picnum == RRTILE1986)
                    wal->picnum = RRTILE1987;

                if(wal->picnum == RRTILE1939)
                    wal->picnum = RRTILE2004;

                if(wal->picnum == RRTILE1988)
                    wal->picnum = RRTILE2005;

                if(wal->picnum == RRTILE2898)
                    wal->picnum = RRTILE2899;

                if(wal->picnum == RRTILE2878)
                    wal->picnum = RRTILE2879;

                if(wal->picnum == RRTILE2123)
                    wal->picnum = RRTILE2124;

                if(wal->picnum == RRTILE2125)
                    wal->picnum = RRTILE2126;

                if(wal->picnum == RRTILE3200)
                    wal->picnum = RRTILE3201;

                if(wal->picnum == RRTILE3202)
                    wal->picnum = RRTILE3203;

                if(wal->picnum == RRTILE3204)
                    wal->picnum = RRTILE3205;

                if(wal->picnum == RRTILE3206)
                    wal->picnum = RRTILE3207;

                if(wal->picnum == RRTILE3208)
                    wal->picnum = RRTILE3209;

                if(wal->picnum == RRTILE2636)
                    wal->picnum = RRTILE2637;

                if(wal->picnum == WALLLIGHT1)
                    wal->picnum = WALLLIGHTBUST1;

                if(wal->picnum == WALLLIGHT3)
                    wal->picnum = WALLLIGHTBUST3;

                if(wal->picnum == WALLLIGHT4)
                    wal->picnum = WALLLIGHTBUST4;

                if(wal->picnum == TECHLIGHT2)
                    wal->picnum = TECHLIGHTBUST2;

                if(wal->picnum == TECHLIGHT4)
                    wal->picnum = TECHLIGHTBUST4;

                if(!wal->lotag) return;

                sn = wal->nextsector;
                if(sn < 0) return;
                darkestwall = 0;

                wal = &wall[sector[sn].wallptr];
                for(i=sector[sn].wallnum;i > 0;i--,wal++)
                    if(wal->shade > darkestwall)
                        darkestwall=wal->shade;

                j = TRAND&1;
                i= headspritestat[3];
                while(i >= 0)
                {
                    if(SHT == wall[dawallnum].lotag && SLT == 3 )
                    {
                        T3 = j;
                        T4 = darkestwall;
                        T5 = 1;
                    }
                    i = nextspritestat[i];
                }
                break;
    }
}


void checkplayerhurt(struct player_struct *p,short j)
{
    if( (j&49152) == 49152 )
    {
        j &= (MAXSPRITES-1);

        switch(sprite[j].picnum)
        {
#ifdef RRRA
            case RRTILE2430:
            case RRTILE2431:
            case RRTILE2432:
            case RRTILE2443:
            case RRTILE2446:
            case RRTILE2451:
            case RRTILE2455:
                if(p->hurt_delay2 < 8 )
                {
                    sprite[p->i].extra -= 2;
                    p->hurt_delay2 = 16;
#else
            case CACTUS:
                if(p->hurt_delay < 8 )
                {
                    sprite[p->i].extra -= 5;
                    p->hurt_delay = 16;
#endif
                    p->pals_time = 32;
                    p->pals[0] = 32;
                    p->pals[1] = 0;
                    p->pals[2] = 0;
                    spritesound(DUKE_LONGTERM_PAIN,p->i);
                }
                break;
        }
        return;
    }

    if( (j&49152) != 32768) return;
    j &= (MAXWALLS-1);

    if( p->hurt_delay > 0 ) p->hurt_delay--;
    else if( wall[j].cstat&85 ) switch(wall[j].overpicnum)
    {
        case BIGFORCE:
            p->hurt_delay = 26;
            checkhitwall(p->i,j,
                p->posx+(sintable[(p->ang+512)&2047]>>9),
                p->posy+(sintable[p->ang&2047]>>9),
                p->posz,-1);
            break;

    }
}


char checkhitceiling(short sn)
{
    short i, j, q, darkestwall, darkestceiling;
    signed char nfloors,nceilings;
    walltype *wal;

    switch(sector[sn].ceilingpicnum)
    {
        case WALLLIGHT1:
        case WALLLIGHT3:
        case WALLLIGHT4:
        case TECHLIGHT2:
        case TECHLIGHT4:
        case RRTILE1939:
        case RRTILE1986:
        case RRTILE1988:
        case RRTILE2123:
        case RRTILE2125:
        case RRTILE2878:
        case RRTILE2898:


                ceilingglass(ps[myconnectindex].i,sn,10);
                spritesound(GLASS_BREAKING,ps[screenpeek].i);

                if(sector[sn].ceilingpicnum == WALLLIGHT1)
                    sector[sn].ceilingpicnum = WALLLIGHTBUST1;

                if(sector[sn].ceilingpicnum == WALLLIGHT3)
                    sector[sn].ceilingpicnum = WALLLIGHTBUST3;

                if(sector[sn].ceilingpicnum == WALLLIGHT4)
                    sector[sn].ceilingpicnum = WALLLIGHTBUST4;

                if(sector[sn].ceilingpicnum == TECHLIGHT2)
                    sector[sn].ceilingpicnum = TECHLIGHTBUST2;

                if(sector[sn].ceilingpicnum == TECHLIGHT4)
                    sector[sn].ceilingpicnum = TECHLIGHTBUST4;

                if(sector[sn].ceilingpicnum == RRTILE1986)
                    sector[sn].ceilingpicnum = RRTILE1987;

                if(sector[sn].ceilingpicnum == RRTILE1939)
                    sector[sn].ceilingpicnum = RRTILE2004;

                if(sector[sn].ceilingpicnum == RRTILE1988)
                    sector[sn].ceilingpicnum = RRTILE2005;

                if(sector[sn].ceilingpicnum == RRTILE2898)
                    sector[sn].ceilingpicnum = RRTILE2899;

                if(sector[sn].ceilingpicnum == RRTILE2878)
                    sector[sn].ceilingpicnum = RRTILE2879;

                if(sector[sn].ceilingpicnum == RRTILE2123)
                    sector[sn].ceilingpicnum = RRTILE2124;

                if(sector[sn].ceilingpicnum == RRTILE2125)
                    sector[sn].ceilingpicnum = RRTILE2126;


                if(!sector[sn].hitag)
                {
                    i = headspritesect[sn];
                    while(i >= 0)
                    {
#ifdef RRRA
                        if( PN == SECTOREFFECTOR && (SLT == 12 || SLT == 47 || SLT == 48) )
#else
                        if( PN == SECTOREFFECTOR && SLT == 12 )
#endif
                        {
                            j = headspritestat[3];
                            while(j >= 0)
                            {
                                if( sprite[j].hitag == SHT )
                                    hittype[j].temp_data[3] = 1;
                                j = nextspritestat[j];
                            }
                            break;
                        }
                        i = nextspritesect[i];
                    }
                }

                i = headspritestat[3];
                j = TRAND&1;
                while(i >= 0)
                {
                    if(SHT == (sector[sn].hitag) && SLT == 3 )
                    {
                        T3 = j;
                        T5 = 1;
                    }
                    i = nextspritestat[i];
                }

                return 1;
    }

    return 0;
}                                     
