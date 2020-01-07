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

extern char actor_tog;

void moveeffectors(void)   //STATNUM 3
{
    long q, l, m, x, st, j, *t;
    short i, k, nexti, nextk, p, sh, nextj, ns, pn;
    spritetype *s;
    sectortype *sc;
    walltype *wal;

    fricxv = fricyv = 0;

    i = headspritestat[3];
    while(i >= 0)
    {
        nexti = nextspritestat[i];
        s = &sprite[i];

        sc = &sector[s->sectnum];
        st = s->lotag;
        sh = s->hitag;

        t = &hittype[i].temp_data[0];

        switch(st)
        {
            case 0:
            {
                long zchange = 0;

                zchange = 0;

                j = s->owner;

                if( sprite[j].lotag == (short) 65535 )
                    KILLIT(i);

                q = sc->extra>>3;
                l = 0;

                if(sc->lotag == 30)
                {
                    q >>= 2;

                    if( sprite[i].extra == 1 )
                    {
                        if(hittype[i].tempang < 256)
                        {
                            hittype[i].tempang += 4;
                            if(hittype[i].tempang >= 256)
                                callsound(s->sectnum,i);
                            if(s->clipdist) l = 1;
                            else l = -1;
                        }
                        else hittype[i].tempang = 256;

                        if( sc->floorz > s->z ) //z's are touching
                        {
                            sc->floorz -= 512;
                            zchange = -512;
                            if( sc->floorz < s->z )
                                sc->floorz = s->z;
                        }

                        else if( sc->floorz < s->z ) //z's are touching
                        {
                            sc->floorz += 512;
                            zchange = 512;
                            if( sc->floorz > s->z )
                                sc->floorz = s->z;
                        }
                    }
                    else if(sprite[i].extra == 3)
                    {
                        if(hittype[i].tempang > 0)
                        {
                            hittype[i].tempang -= 4;
                            if(hittype[i].tempang <= 0)
                                callsound(s->sectnum,i);
                            if( s->clipdist ) l = -1;
                            else l = 1;
                        }
                        else hittype[i].tempang = 0;

                        if( sc->floorz > T4 ) //z's are touching
                        {
                            sc->floorz -= 512;
                            zchange = -512;
                            if( sc->floorz < T4 )
                                sc->floorz = T4;
                        }

                        else if( sc->floorz < T4 ) //z's are touching
                        {
                            sc->floorz += 512;
                            zchange = 512;
                            if( sc->floorz > T4 )
                                sc->floorz = T4;
                        }
                    }

                    s->ang += (l*q);
                    t[2] += (l*q);
                }
                else
                {
                    if( hittype[j].temp_data[0] == 0 ) break;
                    if( hittype[j].temp_data[0] == 2 ) KILLIT(i);

                    if( sprite[j].ang > 1024 )
                        l = -1;
                    else l = 1;
                    if( t[3] == 0 )
                        t[3] = ldist(s,&sprite[j]);
                    s->xvel = t[3];
                    s->x = sprite[j].x;
                    s->y = sprite[j].y;
                    s->ang += (l*q);
                    t[2] += (l*q);
                }

                if( l && (sc->floorstat&64) )
                {
                    for(p=connecthead;p>=0;p=connectpoint2[p])
                    {
                        if( ps[p].cursectnum == s->sectnum && ps[p].on_ground == 1)
                        {

                            ps[p].ang += (l*q);
                            ps[p].ang &= 2047;

                            ps[p].posz += zchange;

                            rotatepoint( sprite[j].x,sprite[j].y,
                                ps[p].posx,ps[p].posy,(q*l),
                                &m,&x);

                            ps[p].bobposx += m-ps[p].posx;
                            ps[p].bobposy += x-ps[p].posy;

                            ps[p].posx = m;
                            ps[p].posy = x;

                            if(sprite[ps[p].i].extra <= 0)
                            {
                                sprite[ps[p].i].x = m;
                                sprite[ps[p].i].y = x;
                            }
                        }
                    }

                    p = headspritesect[s->sectnum];
                    while(p >= 0)
                    {
                        if(sprite[p].statnum != 3 && sprite[p].statnum != 4)
                        {
                            if(sprite[p].picnum == APLAYER && sprite[p].owner >= 0)
                            {
                                p = nextspritesect[p];
                                continue;
                            }

                            sprite[p].ang += (l*q);
                            sprite[p].ang &= 2047;

                            sprite[p].z += zchange;

                            rotatepoint(sprite[j].x,sprite[j].y,
                                sprite[p].x,sprite[p].y,(q*l),
                                &sprite[p].x,&sprite[p].y);

                        }
                        p = nextspritesect[p];
                    }

                }

                ms(i);
            }

            break;
            case 1: //Nothing for now used as the pivot
                if(s->owner == -1) //Init
                {
                    s->owner = i;

                    j = headspritestat[3];
                    while(j >= 0)
                    {
                        if( sprite[j].lotag == 19 && sprite[j].hitag == sh )
                        {
                            t[0] = 0;
                            break;
                        }
                        j = nextspritestat[j];
                    }
                }

                break;
            case 6:
                k = sc->extra;

                if(t[4] > 0)
                {
                    t[4]--;
                    if( t[4] >= (k-(k>>3)) )
                        s->xvel -= (k>>5);
                    if( t[4] > ((k>>1)-1) && t[4] < (k-(k>>3)) )
                        s->xvel = 0;
                    if( t[4] < (k>>1) )
                        s->xvel += (k>>5);
                    if( t[4] < ((k>>1)-(k>>3)) )
                    {
                        t[4] = 0;
                        s->xvel = k;
#ifdef RRRA
                        if (lastlevel && hulkspawn)
#else
                        if (hulkspawn)
#endif
                        {
                            hulkspawn--;
                            ns = spawn(i,HULK);
                            sprite[ns].z = sector[sprite[ns].sectnum].ceilingz;
                            sprite[ns].pal = 33;
                            if (!hulkspawn)
                            {
                                ns = EGS(s->sectnum,s->x,s->y,sector[s->sectnum].ceilingz+119428,3677,-8,16,16,0,0,0,i,5);
                                sprite[ns].cstat = 514;
                                sprite[ns].pal = 7;
                                sprite[ns].xrepeat = 80;
                                sprite[ns].yrepeat = 255;
                                ns = spawn(i,296);
                                sprite[ns].cstat = 0;
                                sprite[ns].cstat |= 32768;
                                sprite[ns].z = sector[s->sectnum].floorz - 6144;
                                KILLIT(i);
                            }
                        }
                    }
                }
                else
                {
                    s->xvel = k;
                    j = headspritesect[s->sectnum];
                    while (j >= 0)
                    {
                        nextj = nextspritesect[j];
                        if (sprite[j].picnum == UFOBEAM)
                            if (ufospawn)
                                if (++ufocnt == 64)
                        {
                            ufocnt = 0;
                            ufospawn--;
#ifdef RRRA
                            switch (TRAND&3)
                            {
                                case 0:
                                    pn = UFO1;
                                    break;
                                case 1:
                                    pn = UFO1;
                                    break;
                                case 2:
                                    pn = UFO1;
                                    break;
                                case 3:
                                    pn = UFO1;
                                    break;
                            }
#else
                            switch (TRAND&3)
                            {
                                case 0:
                                    pn = UFO1;
                                    break;
                                case 1:
                                    pn = UFO2;
                                    break;
                                case 2:
                                    pn = UFO3;
                                    break;
                                case 3:
                                    pn = UFO4;
                                    break;
                            }
#endif
                            ns = spawn(i,pn);
                            sprite[ns].z = sector[sprite[ns].sectnum].ceilingz;
                        }
                        j = nextj;
                    }
                }

                j = headspritestat[3];
                while( j >= 0)
                {
                    if( (sprite[j].lotag == 14) && (sh == sprite[j].hitag) && (hittype[j].temp_data[0] == t[0]) )
                    {
                        sprite[j].xvel = s->xvel;
//                        if( t[4] == 1 )
                        {
                            if(hittype[j].temp_data[5] == 0)
                                hittype[j].temp_data[5] = dist(&sprite[j],s);
                            x = sgn( dist(&sprite[j],s)-hittype[j].temp_data[5] );
                            if(sprite[j].extra)
                                x = -x;
                            s->xvel += x;
                        }
                        hittype[j].temp_data[4] = t[4];
                    }
                    j = nextspritestat[j];
                }
                x = 0;


            case 14:
                if(s->owner==-1)
                    s->owner = LocateTheLocator((short)t[3],(short)t[0]);

                if(s->owner == -1)
                {
                    sprintf(tempbuf,"Could not find any locators for SE# 6 and 14 with a hitag of %ld.\n",t[3]);
                    gameexit(tempbuf);
                }

                j = ldist(&sprite[s->owner],s);

                if( j < 1024L )
                {
                    if(st==6)
                        if(sprite[s->owner].hitag&1)
                            t[4]=sc->extra; //Slow it down
                    t[3]++;
                    s->owner = LocateTheLocator(t[3],t[0]);
                    if(s->owner==-1)
                    {
                        t[3]=0;
                        s->owner = LocateTheLocator(0,t[0]);
                    }
                }

                if(s->xvel)
                {
                    x = getangle(sprite[s->owner].x-s->x,sprite[s->owner].y-s->y);
                    q = getincangle(s->ang,x)>>3;

                    t[2] += q;
                    s->ang += q;

                    if(s->xvel == sc->extra )
                    {
                        if( Sound[hittype[i].lastvx].num == 0 )
                            spritesound(hittype[i].lastvx,i);
                        if( ud.monsters_off == 0 && sc->floorpal == 0 && (sc->floorstat&1) && rnd(8) )
                        {
                            p = findplayer(s,&x);
                            if(x < 20480)
                            {
                                j = s->ang;
                                s->ang = getangle(s->x-ps[p].posx,s->y-ps[p].posy);
                                shoot(i,RPG);
                                s->ang = j;
                            }
                        }
                    }

                    if(s->xvel <= 64)
                        stopsound(hittype[i].lastvx);

                    if( (sc->floorz-sc->ceilingz) < (108<<8) )
                    {
                        if(ud.clipping == 0 && s->xvel >= 192)
                            for(p=connecthead;p>=0;p=connectpoint2[p])
                                if(sprite[ps[p].i].extra > 0)
                        {
                            k = ps[p].cursectnum;
                            updatesector(ps[p].posx,ps[p].posy,&k);
                            if( ( k == -1 && ud.clipping == 0 ) || ( k == s->sectnum && ps[p].cursectnum != s->sectnum ) )
                            {
                                ps[p].posx = s->x;
                                ps[p].posy = s->y;
                                ps[p].cursectnum = s->sectnum;

                                setsprite(ps[p].i,s->x,s->y,s->z);
                                quickkill(&ps[p]);
                            }
                        }
                    }

                    m = (s->xvel*sintable[(s->ang+512)&2047])>>14;
                    x = (s->xvel*sintable[s->ang&2047])>>14;

                    for(p = connecthead;p >= 0;p=connectpoint2[p])
                       if(sector[ps[p].cursectnum].lotag != 2)
                    {
                        if(po[p].os == s->sectnum)
                        {
                            po[p].ox += m;
                            po[p].oy += x;
                        }

                        if(s->sectnum == sprite[ps[p].i].sectnum)
                        {
                            rotatepoint(s->x,s->y,ps[p].posx,ps[p].posy,q,&ps[p].posx,&ps[p].posy);

                            ps[p].posx += m;
                            ps[p].posy += x;

                            ps[p].bobposx += m;
                            ps[p].bobposy += x;

                            ps[p].ang += q;

                            if(numplayers > 1)
                            {
                                ps[p].oposx = ps[p].posx;
                                ps[p].oposy = ps[p].posy;
                            }
                            if( sprite[ps[p].i].extra <= 0 )
                            {
                                sprite[ps[p].i].x = ps[p].posx;
                                sprite[ps[p].i].y = ps[p].posy;
                            }
                        }
                    }
                    j = headspritesect[s->sectnum];
                    while(j >= 0)
                    {
                        if (sprite[j].statnum != 10 && sector[sprite[j].sectnum].lotag != 2 && sprite[j].picnum != SECTOREFFECTOR && sprite[j].picnum != LOCATORS )
                        {
                            rotatepoint(s->x,s->y,
                                sprite[j].x,sprite[j].y,q,
                                &sprite[j].x,&sprite[j].y);

                            sprite[j].x+= m;
                            sprite[j].y+= x;

                            sprite[j].ang+=q;

                            if(numplayers > 1)
                            {
                                hittype[j].bposx = sprite[j].x;
                                hittype[j].bposy = sprite[j].y;
                            }
                        }
                        j = nextspritesect[j];
                    }

                    ms(i);
                    setsprite(i,s->x,s->y,s->z);

                    if( (sc->floorz-sc->ceilingz) < (108<<8) )
                    {
                        if(ud.clipping == 0 && s->xvel >= 192)
                            for(p=connecthead;p>=0;p=connectpoint2[p])
                                if(sprite[ps[p].i].extra > 0)
                        {
                            k = ps[p].cursectnum;
                            updatesector(ps[p].posx,ps[p].posy,&k);
                            if( ( k == -1 && ud.clipping == 0 ) || ( k == s->sectnum && ps[p].cursectnum != s->sectnum ) )
                            {
                                ps[p].oposx = ps[p].posx = s->x;
                                ps[p].oposy = ps[p].posy = s->y;
                                ps[p].cursectnum = s->sectnum;

                                setsprite(ps[p].i,s->x,s->y,s->z);
                                quickkill(&ps[p]);
                            }
                        }

                        j = headspritesect[sprite[OW].sectnum];
                        while(j >= 0)
                        {
                            l = nextspritesect[j];
                            if (sprite[j].statnum == 1 && badguy(&sprite[j]) && sprite[j].picnum != SECTOREFFECTOR && sprite[j].picnum != LOCATORS )
                            {
                                k = sprite[j].sectnum;
                                updatesector(sprite[j].x,sprite[j].y,&k);
                                if( sprite[j].extra >= 0 && k == s->sectnum )
                                {
                                    gutsdir(&sprite[j],JIBS6,72,myconnectindex);
                                    spritesound(SQUISHED,i);
                                    deletesprite(j);
                                }
                            }
                            j = l;
                        }
                    }
                }

                break;

            case 30:
                if(s->owner == -1)
                {
                    t[3] = !t[3];
                    s->owner = LocateTheLocator(t[3],t[0]);
                }
                else
                {

                    if(t[4] == 1) // Starting to go
                    {
                        if( ldist( &sprite[s->owner],s ) < (2048-128) )
                            t[4] = 2;
                        else
                        {
                            if(s->xvel == 0)
                                operateactivators(s->hitag+(!t[3]),-1);
                            if(s->xvel < 256)
                                s->xvel += 16;
                        }
                    }
                    if(t[4] == 2)
                    {
                        l = FindDistance2D(sprite[s->owner].x-s->x,sprite[s->owner].y-s->y);

                        if(l <= 128)
                            s->xvel = 0;

                        if( s->xvel > 0 )
                            s->xvel -= 16;
                        else
                        {
                            s->xvel = 0;
                            operateactivators(s->hitag+(short)t[3],-1);
                            s->owner = -1;
                            s->ang += 1024;
                            t[4] = 0;
                            operateforcefields(i,s->hitag);

                            j = headspritesect[s->sectnum];
                            while(j >= 0)
                            {
                                if(sprite[j].picnum != SECTOREFFECTOR && sprite[j].picnum != LOCATORS )
                                {
                                    hittype[j].bposx = sprite[j].x;
                                    hittype[j].bposy = sprite[j].y;
                                }
                                j = nextspritesect[j];
                            }

                        }
                    }
                }

                if(s->xvel)
                {
                    l = (s->xvel*sintable[(s->ang+512)&2047])>>14;
                    x = (s->xvel*sintable[s->ang&2047])>>14;

                    if( (sc->floorz-sc->ceilingz) < (108<<8) )
                        if(ud.clipping == 0)
                            for(p=connecthead;p>=0;p=connectpoint2[p])
                                if(sprite[ps[p].i].extra > 0)
                    {
                        k = ps[p].cursectnum;
                        updatesector(ps[p].posx,ps[p].posy,&k);
                        if( ( k == -1 && ud.clipping == 0 ) || ( k == s->sectnum && ps[p].cursectnum != s->sectnum ) )
                        {
                            ps[p].posx = s->x;
                            ps[p].posy = s->y;
                            ps[p].cursectnum = s->sectnum;

                            setsprite(ps[p].i,s->x,s->y,s->z);
                            quickkill(&ps[p]);
                        }
                    }

                    for(p = connecthead;p >= 0;p = connectpoint2[p])
                    {
                        if( sprite[ps[p].i].sectnum == s->sectnum )
                        {
                            ps[p].posx += l;
                            ps[p].posy += x;

                            if(numplayers > 1)
                            {
                                ps[p].oposx = ps[p].posx;
                                ps[p].oposy = ps[p].posy;
                            }

                            ps[p].bobposx += l;
                            ps[p].bobposy += x;
                        }

                        if( po[p].os == s->sectnum )
                        {
                            po[p].ox += l;
                            po[p].oy += x;
                        }
                    }

                    j = headspritesect[s->sectnum];
                    while(j >= 0)
                    {
                        if(sprite[j].picnum != SECTOREFFECTOR && sprite[j].picnum != LOCATORS )
                        {
                            if(numplayers < 2)
                            {
                                hittype[j].bposx = sprite[j].x;
                                hittype[j].bposy = sprite[j].y;
                            }

                            sprite[j].x += l;
                            sprite[j].y += x;

                            if(numplayers > 1)
                            {
                                hittype[j].bposx = sprite[j].x;
                                hittype[j].bposy = sprite[j].y;
                            }
                        }
                        j = nextspritesect[j];
                    }

                    ms(i);
                    setsprite(i,s->x,s->y,s->z);

                    if( (sc->floorz-sc->ceilingz) < (108<<8) )
                    {
                        if(ud.clipping == 0)
                            for(p=connecthead;p>=0;p=connectpoint2[p])
                                if(sprite[ps[p].i].extra > 0)
                        {
                            k = ps[p].cursectnum;
                            updatesector(ps[p].posx,ps[p].posy,&k);
                            if( ( k == -1 && ud.clipping == 0 ) || ( k == s->sectnum && ps[p].cursectnum != s->sectnum ) )
                            {
                                ps[p].posx = s->x;
                                ps[p].posy = s->y;

                                ps[p].oposx = ps[p].posx;
                                ps[p].oposy = ps[p].posy;

                                ps[p].cursectnum = s->sectnum;

                                setsprite(ps[p].i,s->x,s->y,s->z);
                                quickkill(&ps[p]);
                            }
                        }

                        j = headspritesect[sprite[OW].sectnum];
                        while(j >= 0)
                        {
                            l = nextspritesect[j];
                            if (sprite[j].statnum == 1 && badguy(&sprite[j]) && sprite[j].picnum != SECTOREFFECTOR && sprite[j].picnum != LOCATORS )
                            {
            //                    if(sprite[j].sectnum != s->sectnum)
                                {
                                    k = sprite[j].sectnum;
                                    updatesector(sprite[j].x,sprite[j].y,&k);
                                    if( sprite[j].extra >= 0 && k == s->sectnum )
                                    {
                                        gutsdir(&sprite[j],JIBS6,24,myconnectindex);
                                        spritesound(SQUISHED,j);
                                        deletesprite(j);
                                    }
                                }

                            }
                            j = l;
                        }
                    }
                }

                break;


            case 2://Quakes
                if(t[4] > 0 && t[0] == 0 )
                {
                    if( t[4] < sh )
                        t[4]++;
                    else t[0] = 1;
                }

                if(t[0] > 0)
                {
                    t[0]++;

                    s->xvel = 3;

                    if(t[0] > 96)
                    {
                        t[0] = -1; //Stop the quake
                        t[4] = -1;
                        KILLIT(i);
                    }
                    else
                    {
                        if( (t[0]&31) ==  8 )
                        {
                            earthquaketime = 48;
                            spritesound(EARTHQUAKE,ps[screenpeek].i);
                        }

                        if( klabs( sc->floorheinum-t[5] ) < 8 )
                            sc->floorheinum = t[5];
                        else sc->floorheinum += ( sgn(t[5]-sc->floorheinum)<<4 );
                    }

                    m = (s->xvel*sintable[(s->ang+512)&2047])>>14;
                    x = (s->xvel*sintable[s->ang&2047])>>14;


                    for(p=connecthead;p>=0;p=connectpoint2[p])
                        if(ps[p].cursectnum == s->sectnum && ps[p].on_ground)
                        {
                            ps[p].posx += m;
                            ps[p].posy += x;

                            ps[p].bobposx += m;
                            ps[p].bobposy += x;
                        }

                    j = headspritesect[s->sectnum];
                    while(j >= 0)
                    {
                        nextj = nextspritesect[j];

                        if (sprite[j].picnum != SECTOREFFECTOR)
                        {
                            sprite[j].x+=m;
                            sprite[j].y+=x;
                            setsprite(j,sprite[j].x,sprite[j].y,sprite[j].z);
                        }
                        j = nextj;
                    }
                    ms(i);
                    setsprite(i,s->x,s->y,s->z);
                }
                break;

            //Flashing sector lights after reactor EXPLOSION2

            case 3:

                if( t[4] == 0 ) break;
                p = findplayer(s,&x);

            //    if(t[5] > 0) { t[5]--; break; }

                if( (global_random/(sh+1)&31) < 4 && !t[2])
                {
             //       t[5] = 4+(global_random&7);
                    sc->ceilingpal = s->owner>>8;
                    sc->floorpal = s->owner&0xff;
                    t[0] = s->shade + (global_random&15);
                }
                else
                {
             //       t[5] = 4+(global_random&3);
                    sc->ceilingpal = s->pal;
                    sc->floorpal = s->pal;
                    t[0] = t[3];
                }

                sc->ceilingshade = t[0];
                sc->floorshade = t[0];

                wal = &wall[sc->wallptr];

                for(x=sc->wallnum;x > 0;x--,wal++)
                {
                    if( wal->hitag != 1 )
                    {
                        wal->shade = t[0];
                        if((wal->cstat&2) && wal->nextwall >= 0)
                        {
                            wall[wal->nextwall].shade = wal->shade;
                        }
                    }
                }

                break;

            case 4:

                if((global_random/(sh+1)&31) < 4 )
                {
                    t[1] = s->shade + (global_random&15);//Got really bright
                    t[0] = s->shade + (global_random&15);
                    sc->ceilingpal = s->owner>>8;
                    sc->floorpal = s->owner&0xff;
                    j = 1;
                }
                else
                {
                    t[1] = t[2];
                    t[0] = t[3];

                    sc->ceilingpal = s->pal;
                    sc->floorpal = s->pal;

                    j = 0;
                }

                sc->floorshade = t[1];
                sc->ceilingshade = t[1];

                wal = &wall[sc->wallptr];

                for(x=sc->wallnum;x > 0; x--,wal++)
                {
                    if(j) wal->pal = (s->owner&0xff);
                    else wal->pal = s->pal;

                    if( wal->hitag != 1 )
                    {
                        wal->shade = t[0];
                        if((wal->cstat&2) && wal->nextwall >= 0)
                            wall[wal->nextwall].shade = wal->shade;
                    }
                }

                j = headspritesect[SECT];
                while(j >= 0)
                {
                    if(sprite[j].cstat&16)
                    {
                        if (sc->ceilingstat&1)
                            sprite[j].shade = sc->ceilingshade;
                        else sprite[j].shade = sc->floorshade;
                    }

                    j = nextspritesect[j];
                }

                if(t[4]) KILLIT(i);

                break;

            //BOSS
            case 5:
                p = findplayer(s,&x);
                if(x < 8192)
                {
                    j = s->ang;
                    s->ang = getangle(s->x-ps[p].posx,s->y-ps[p].posy);
                    shoot(i,FIRELASER);
                    s->ang = j;
                }

                if(s->owner==-1) //Start search
                {
                    t[4]=0;
                    l = 0x7fffffff;
                    while(1) //Find the shortest dist
                    {
                        s->owner = LocateTheLocator((short)t[4],-1); //t[0] hold sectnum

                        if(s->owner==-1) break;

                        m = ldist(&sprite[ps[p].i],&sprite[s->owner]);

                        if(l > m)
                        {
                            q = s->owner;
                            l = m;
                        }

                        t[4]++;
                    }

                    s->owner = q;
                    s->zvel = ksgn(sprite[q].z-s->z)<<4;
                }

                if(ldist(&sprite[s->owner],s) < 1024)
                {
                    short ta;
                    ta = s->ang;
                    s->ang = getangle(ps[p].posx-s->x,ps[p].posy-s->y);
                    s->ang = ta;
                    s->owner = -1;
                    goto BOLT;

                }
                else s->xvel=256;

                x = getangle(sprite[s->owner].x-s->x,sprite[s->owner].y-s->y);
                q = getincangle(s->ang,x)>>3;
                s->ang += q;
                
                if(rnd(32))
                {
                    t[2]+=q;
                    sc->ceilingshade = 127;
                }
                else
                {
                    t[2] +=
                        getincangle(t[2]+512,getangle(ps[p].posx-s->x,ps[p].posy-s->y))>>2;
                    sc->ceilingshade = 0;
                }
                IFHIT
                {
                    t[3]++;
                    if(t[3] == 5)
                    {
                        s->zvel += 1024;
                        FTA(7,&ps[myconnectindex]);
                    }
                }

                s->z += s->zvel;
                sc->ceilingz += s->zvel;
                sector[t[0]].ceilingz += s->zvel;
                ms(i);
                setsprite(i,s->x,s->y,s->z);
                break;

            
            case 8:
            case 9:

                // work only if its moving

                j = -1;

                if(hittype[i].temp_data[4])
                {
                    hittype[i].temp_data[4]++;
                    if( hittype[i].temp_data[4] > 8 ) KILLIT(i);
                    j = 1;
                }
                else j = getanimationgoal(&sc->ceilingz);

                if( j >= 0 )
                {
                    short sn;

                    if( (sc->lotag&0x8000) || hittype[i].temp_data[4] )
                        x = -t[3];
                    else
                        x = t[3];

                    if ( st == 9 ) x = -x;

                    j = headspritestat[3];
                    while(j >= 0)
                    {
                        if( ((sprite[j].lotag) == st ) && (sprite[j].hitag) == sh )
                        {
                            sn = sprite[j].sectnum;
                            m = sprite[j].shade;

                            wal = &wall[sector[sn].wallptr];

                            for(l=sector[sn].wallnum;l>0;l--,wal++)
                            {
                                if( wal->hitag != 1 )
                                {
                                    wal->shade+=x;

                                    if(wal->shade < m)
                                        wal->shade = m;
                                    else if(wal->shade > hittype[j].temp_data[2])
                                        wal->shade = hittype[j].temp_data[2];

                                    if(wal->nextwall >= 0)
                                        if(wall[wal->nextwall].hitag != 1)
                                            wall[wal->nextwall].shade = wal->shade;
                                }
                            }

                            sector[sn].floorshade   += x;
                            sector[sn].ceilingshade += x;

                            if(sector[sn].floorshade < m)
                                sector[sn].floorshade = m;
                            else if(sector[sn].floorshade > hittype[j].temp_data[0])
                                sector[sn].floorshade = hittype[j].temp_data[0];

                            if(sector[sn].ceilingshade < m)
                                sector[sn].ceilingshade = m;
                            else if(sector[sn].ceilingshade > hittype[j].temp_data[1])
                                sector[sn].ceilingshade = hittype[j].temp_data[1];

                            if (sector[sn].hitag == 1)
                                sector[sn].ceilingshade = hittype[j].temp_data[1];

                        }
                        j = nextspritestat[j];
                    }
                }
                break;
            case 10:

                if( (sc->lotag&0xff) == 27 || ( sc->floorz > sc->ceilingz && (sc->lotag&0xff) != 23 ) || sc->lotag == (short) 32791 )
                {
                    j = 1;

                    if( (sc->lotag&0xff) != 27)
                        for(p=connecthead;p>=0;p=connectpoint2[p])
                            if( sc->lotag != 30 && sc->lotag != 31 && sc->lotag != 0 )
                                if(s->sectnum == sprite[ps[p].i].sectnum)
                                    j = 0;

                    if(j == 1)
                    {
                        if(t[0] > sh )
                        {
                            activatebysector(s->sectnum,i);
                            t[0] = 0;
                        }
                        else t[0]++;
                    }
                }
                else t[0]=0;
                break;
            case 11: //Swingdoor

                if( t[5] > 0)
                {
                    t[5]--;
                    break;
                }

                if( t[4] )
                {
                    short startwall,endwall;

                    startwall = sc->wallptr;
                    endwall = startwall+sc->wallnum;

                    for(j=startwall;j<endwall;j++)
                    {
                        k = headspritestat[1];
                        while(k >= 0)
                        {
                            if( sprite[k].extra > 0 && badguy(&sprite[k]) && clipinsidebox(sprite[k].x,sprite[k].y,j,256L) == 1 )
                                goto BOLT;
                            k = nextspritestat[k];
                        }

                        k = headspritestat[10];
                        while(k >= 0)
                        {
                            if( sprite[k].owner >= 0 && clipinsidebox(sprite[k].x,sprite[k].y,j,144L) == 1 )
                            {
                                t[5] = 8; // Delay
                                k = (SP>>3)*t[3];
                                t[2]-=k;
                                t[4]-=k;
                                ms(i);
                                setsprite(i,s->x,s->y,s->z);
                                goto BOLT;
                            }
                            k = nextspritestat[k];
                        }
                    }

                    k = (SP>>3)*t[3];
                    t[2]+=k;
                    t[4]+=k;
                    ms(i);
                    setsprite(i,s->x,s->y,s->z);

                    if(t[4] <= -511 || t[4] >= 512)
                    {
                        t[4] = 0;
                        t[2] &= 0xffffff00;
                        ms(i);
                        setsprite(i,s->x,s->y,s->z);
                        break;
                    }
                }
                break;
            case 12:
                if( t[0] == 3 || t[3] == 1 ) //Lights going off
                {
                    sc->floorpal = 0;
                    sc->ceilingpal = 0;

                    wal = &wall[sc->wallptr];
                    for(j = sc->wallnum;j > 0; j--, wal++)
                        if(wal->hitag != 1)
                        {
                            wal->shade = t[1];
                            wal->pal = 0;
                        }

                    sc->floorshade = t[1];
                    sc->ceilingshade = t[2];
                    t[0]=0;

                    j = headspritesect[SECT];
                    while(j >= 0)
                    {
                        if(sprite[j].cstat&16)
                        {
                            if (sc->ceilingstat&1)
                                sprite[j].shade = sc->ceilingshade;
                            else sprite[j].shade = sc->floorshade;
                        }
                        j = nextspritesect[j];

                    }

                    if(t[3] == 1) KILLIT(i);
                }
                if( t[0] == 1 ) //Lights flickering on
                {
                    if( sc->floorshade > s->shade )
                    {
                        sc->floorpal = s->pal;
                        sc->ceilingpal = s->pal;

                        sc->floorshade -= 2;
                        sc->ceilingshade -= 2;

                        wal = &wall[sc->wallptr];
                        for(j=sc->wallnum;j>0;j--,wal++)
                            if(wal->hitag != 1)
                            {
                                wal->pal = s->pal;
                                wal->shade -= 2;
                            }
                    }
                    else t[0] = 2;

                    j = headspritesect[SECT];
                    while(j >= 0)
                    {
                        if(sprite[j].cstat&16)
                        {
                            if (sc->ceilingstat&1)
                                sprite[j].shade = sc->ceilingshade;
                            else sprite[j].shade = sc->floorshade;
                        }
                        j = nextspritesect[j];
                    }
                }
                break;
#ifdef RRRA
            case 47:
                if( t[0] == 3 || t[3] == 1 ) //Lights going off
                {
                    sc->floorpal = 0;
                    sc->ceilingpal = 0;

                    wal = &wall[sc->wallptr];
                    for(j = sc->wallnum;j > 0; j--, wal++)
                        if(wal->hitag != 1)
                        {
                            wal->shade = t[1];
                            wal->pal = 0;
                        }

                    sc->floorshade = t[1];
                    sc->ceilingshade = t[2];
                    t[0]=0;

                    j = headspritesect[SECT];
                    while(j >= 0)
                    {
                        if(sprite[j].cstat&16)
                        {
                            if (sc->ceilingstat&1)
                                sprite[j].shade = sc->ceilingshade;
                            else sprite[j].shade = sc->floorshade;
                        }
                        j = nextspritesect[j];

                    }

                    if(t[3] == 1) KILLIT(i);
                }
                if( t[0] == 1 ) //Lights flickering on
                {
                    if( sc->floorshade > s->shade )
                    {
                        sc->floorpal = s->pal;

                        sc->floorshade -= 2;

                        wal = &wall[sc->wallptr];
                        for(j=sc->wallnum;j>0;j--,wal++)
                            if(wal->hitag != 1)
                            {
                                wal->pal = s->pal;
                                wal->shade -= 2;
                            }
                    }
                    else t[0] = 2;

                    j = headspritesect[SECT];
                    while(j >= 0)
                    {
                        if(sprite[j].cstat&16)
                        {
                            if (sc->ceilingstat&1)
                                sprite[j].shade = sc->ceilingshade;
                            else sprite[j].shade = sc->floorshade;
                        }
                        j = nextspritesect[j];
                    }
                }
                break;
            case 48:
                if( t[0] == 3 || t[3] == 1 ) //Lights going off
                {
                    sc->floorpal = 0;
                    sc->ceilingpal = 0;

                    wal = &wall[sc->wallptr];
                    for(j = sc->wallnum;j > 0; j--, wal++)
                        if(wal->hitag != 1)
                        {
                            wal->shade = t[1];
                            wal->pal = 0;
                        }

                    sc->floorshade = t[1];
                    sc->ceilingshade = t[2];
                    t[0]=0;

                    j = headspritesect[SECT];
                    while(j >= 0)
                    {
                        if(sprite[j].cstat&16)
                        {
                            if (sc->ceilingstat&1)
                                sprite[j].shade = sc->ceilingshade;
                            else sprite[j].shade = sc->floorshade;
                        }
                        j = nextspritesect[j];

                    }

                    if(t[3] == 1) KILLIT(i);
                }
                if( t[0] == 1 ) //Lights flickering on
                {
                    if( sc->ceilingshade > s->shade )
                    {
                        sc->ceilingpal = s->pal;

                        sc->ceilingshade -= 2;

                        wal = &wall[sc->wallptr];
                        for(j=sc->wallnum;j>0;j--,wal++)
                            if(wal->hitag != 1)
                            {
                                wal->pal = s->pal;
                                wal->shade -= 2;
                            }
                    }
                    else t[0] = 2;

                    j = headspritesect[SECT];
                    while(j >= 0)
                    {
                        if(sprite[j].cstat&16)
                        {
                            if (sc->ceilingstat&1)
                                sprite[j].shade = sc->ceilingshade;
                            else sprite[j].shade = sc->floorshade;
                        }
                        j = nextspritesect[j];
                    }
                }
                break;
#endif


            case 13:
                if( t[2] )
                {
                    j = (SP<<5)|1;

                    if( s->ang == 512 )
                    {
                        if( s->owner )
                        {
                            if( klabs(t[0]-sc->ceilingz) >= j )
                                sc->ceilingz += sgn(t[0]-sc->ceilingz)*j;
                            else sc->ceilingz = t[0];
                        }
                        else
                        {
                            if( klabs(t[1]-sc->floorz) >= j )
                                sc->floorz += sgn(t[1]-sc->floorz)*j;
                            else sc->floorz = t[1];
                        }
                    }
                    else
                    {
                        if( klabs(t[1]-sc->floorz) >= j )
                            sc->floorz += sgn(t[1]-sc->floorz)*j;
                        else sc->floorz = t[1];
                        if( klabs(t[0]-sc->ceilingz) >= j )
                            sc->ceilingz += sgn(t[0]-sc->ceilingz)*j;
                        sc->ceilingz = t[0];
                    }

                    if( t[3] == 1 )
                    {
                        //Change the shades

                        t[3]++;
                        sc->ceilingstat ^= 1;

                        if(s->ang == 512)
                        {
                            wal = &wall[sc->wallptr];
                            for(j=sc->wallnum;j>0;j--,wal++)
                                wal->shade = s->shade;

                            sc->floorshade = s->shade;

                            if(ps[0].one_parallax_sectnum >= 0)
                            {
                                sc->ceilingpicnum =
                                    sector[ps[0].one_parallax_sectnum].ceilingpicnum;
                                sc->ceilingshade  =
                                    sector[ps[0].one_parallax_sectnum].ceilingshade;
                            }
                        }
                    }
                    t[2]++;
                    if(t[2] > 256)
                        KILLIT(i);
                }


                if( t[2] == 4 && s->ang != 512)
                    for(x=0;x<7;x++) RANDOMSCRAP;
                break;


            case 15:

                if(t[4])
                {
                    s->xvel = 16;

                    if(t[4] == 1) //Opening
                    {
                        if( t[3] >= (SP>>3) )
                        {
                            t[4] = 0; //Turn off the sliders
                            callsound(s->sectnum,i);
                            break;
                        }
                        t[3]++;
                    }
                    else if(t[4] == 2)
                    {
                        if(t[3]<1)
                        {
                            t[4] = 0;
                            callsound(s->sectnum,i);
                            break;
                        }
                        t[3]--;
                    }

                    ms(i);
                    setsprite(i,s->x,s->y,s->z);
                }
                break;

            case 16: //Reactor

                t[2]+=32;
                if(sc->floorz<sc->ceilingz) s->shade=0;

                else if( sc->ceilingz < t[3] )
                {

                    //The following code check to see if
                    //there is any other sprites in the sector.
                    //If there isn't, then kill this sectoreffector
                    //itself.....

                    j = headspritesect[s->sectnum];
                    while(j >= 0)
                    {
                        if(sprite[j].picnum == REACTOR || sprite[j].picnum == REACTOR2)
                            break;
                        j = nextspritesect[j];
                    }
                    if(j == -1) { KILLIT(i); }
                    else s->shade=1;
                }

                if(s->shade) sc->ceilingz+=1024;
                else sc->ceilingz-=512;

                ms(i);
                setsprite(i,s->x,s->y,s->z);

                break;

            case 17:

                q = t[0]*(SP<<2);

                sc->ceilingz += q;
                sc->floorz += q;

                j = headspritesect[s->sectnum];
                while(j >= 0)
                {
                    if(sprite[j].statnum == 10 && sprite[j].owner >= 0)
                    {
                        p = sprite[j].yvel;
                        if(numplayers < 2)
                            ps[p].oposz = ps[p].posz;
                        ps[p].posz += q;
                        ps[p].truefz += q;
                        ps[p].truecz += q;
                        if(numplayers > 1)
                            ps[p].oposz = ps[p].posz;
                    }
                    if( sprite[j].statnum != 3 )
                    {
                        hittype[j].bposz = sprite[j].z;
                        sprite[j].z += q;
                    }

                    hittype[j].floorz = sc->floorz;
                    hittype[j].ceilingz = sc->ceilingz;

                    j = nextspritesect[j];
                }

                if( t[0] )                if(t[0]) //If in motion
                {
                    if( klabs(sc->floorz-t[2]) <= SP)
                    {
                        activatewarpelevators(i,0);
                        break;
                    }

                    if(t[0]==-1)
                    {
                        if( sc->floorz > t[3] )
                            break;
                    }
                    else if( sc->ceilingz < t[4] ) break;

                    if( t[1] == 0 ) break;
                    t[1] = 0;

                    j = headspritestat[3];
                    while(j >= 0)
                    {
                                if( i != j && (sprite[j].lotag) == 17)
                                    if( (sc->hitag-t[0]) ==
                                        (sector[sprite[j].sectnum].hitag)
                                        && sh == (sprite[j].hitag))
                                            break;
                                j = nextspritestat[j];
                    }

                    if(j == -1) break;

                    k = headspritesect[s->sectnum];
                    while(k >= 0)
                    {
                        nextk = nextspritesect[k];

                        if(sprite[k].statnum == 10 && sprite[k].owner >= 0)
                        {
                            p = sprite[k].yvel;

                            ps[p].posx += sprite[j].x-s->x;
                            ps[p].posy += sprite[j].y-s->y;
                            ps[p].posz = sector[sprite[j].sectnum].floorz-(sc->floorz-ps[p].posz);

                            hittype[k].floorz = sector[sprite[j].sectnum].floorz;
                            hittype[k].ceilingz = sector[sprite[j].sectnum].ceilingz;

                            ps[p].bobposx = ps[p].oposx = ps[p].posx;
                            ps[p].bobposy = ps[p].oposy = ps[p].posy;
                            ps[p].oposz = ps[p].posz;

                            ps[p].truefz = hittype[k].floorz;
                            ps[p].truecz = hittype[k].ceilingz;
                            ps[p].bobcounter = 0;

                            changespritesect(k,sprite[j].sectnum);
                            ps[p].cursectnum = sprite[j].sectnum;
                        }
                        else if( sprite[k].statnum != 3 )
                        {
                            sprite[k].x +=
                                sprite[j].x-s->x;
                            sprite[k].y +=
                                sprite[j].y-s->y;
                            sprite[k].z = sector[sprite[j].sectnum].floorz-
                                (sc->floorz-sprite[k].z);

                            hittype[k].bposx = sprite[k].x;
                            hittype[k].bposy = sprite[k].y;
                            hittype[k].bposz = sprite[k].z;

                            changespritesect(k,sprite[j].sectnum);
                            setsprite(k,sprite[k].x,sprite[k].y,sprite[k].z);

                            hittype[k].floorz = sector[sprite[j].sectnum].floorz;
                            hittype[k].ceilingz = sector[sprite[j].sectnum].ceilingz;

                        }
                        k = nextk;
                    }
                }
                break;

            case 18:
                if(t[0])
                {
                    if(s->pal)
                    {
                        if(s->ang == 512)
                        {
                            sc->ceilingz -= sc->extra;
                            if(sc->ceilingz <= t[1])
                            {
                                sc->ceilingz = t[1];
                                KILLIT(i);
                            }
                        }
                        else
                        {
                            sc->floorz += sc->extra;
                            if(sc->floorz >= t[1])
                            {
                                sc->floorz = t[1];
                                KILLIT(i);
                            }
                        }
                    }
                    else
                    {
                        if(s->ang == 512)
                        {
                            sc->ceilingz += sc->extra;
                            if(sc->ceilingz >= s->z)
                            {
                                sc->ceilingz = s->z;
                                KILLIT(i);
                            }
                        }
                        else
                        {
                            sc->floorz -= sc->extra;
                            if(sc->floorz <= s->z)
                            {
                                sc->floorz = s->z;
                                KILLIT(i);
                            }
                        }
                    }

                    t[2]++;
                    if(t[2] >= s->hitag)
                    {
                        t[2] = 0;
                        t[0] = 0;
                    }
                }
                break;

            case 19: //Battlestar galactia shields

                if(t[0])
                {
                    if(t[0] == 1)
                    {
                        t[0]++;
                        x = sc->wallptr;
                        q = x+sc->wallnum;
                        for(j=x;j<q;j++)
                            if(wall[j].overpicnum == BIGFORCE)
                            {
                                wall[j].cstat &= (128+32+8+4+2);
                                wall[j].overpicnum = 0;
                                if(wall[j].nextwall >= 0)
                                {
                                    wall[wall[j].nextwall].overpicnum = 0;
                                    wall[wall[j].nextwall].cstat &= (128+32+8+4+2);
                                }
                            }
                    }

                    if(sc->ceilingz < sc->floorz)
                        sc->ceilingz += SP;
                    else
                    {
                        sc->ceilingz = sc->floorz;

                        j = headspritestat[3];
                        while(j >= 0)
                        {
                            if(sprite[j].lotag == 0 && sprite[j].hitag==sh)
                            {
                                q = sprite[sprite[j].owner].sectnum;
                                sector[sprite[j].sectnum].floorpal = sector[sprite[j].sectnum].ceilingpal =
                                        sector[q].floorpal;
                                sector[sprite[j].sectnum].floorshade = sector[sprite[j].sectnum].ceilingshade =
                                    sector[q].floorshade;

                                hittype[sprite[j].owner].temp_data[0] = 2;
                            }
                            j = nextspritestat[j];
                        }
                        KILLIT(i);
                    }
                }
                else //Not hit yet
                {
                    IFHITSECT
                    {
                        FTA(8,&ps[myconnectindex]);

                        l = headspritestat[3];
                        while(l >= 0)
                        {
                            x = sprite[l].lotag&0x7fff;
                            switch( x )
                            {
                                case 0:
                                    if(sprite[l].hitag == sh)
                                    {
                                        q = sprite[l].sectnum;
                                        sector[q].floorshade =
                                            sector[q].ceilingshade =
                                                sprite[sprite[l].owner].shade;
                                        sector[q].floorpal =
                                            sector[q].ceilingpal =
                                                sprite[sprite[l].owner].pal;
                                    }
                                    break;

                                case 1:
                                case 12:
//                                case 18:
                                case 19:

                                    if( sh == sprite[l].hitag )
                                        if( hittype[l].temp_data[0] == 0 )
                                        {
                                            hittype[l].temp_data[0] = 1; //Shut them all on
                                            sprite[l].owner = i;
                                        }

                                    break;
                            }
                            l = nextspritestat[l];
                        }
                    }
                }

                break;

            case 20: //Extend-o-bridge

                if( t[0] == 0 ) break;
                if( t[0] == 1 ) s->xvel = 8;
                else s->xvel = -8;

                if( s->xvel ) //Moving
                {
                    x = (s->xvel*sintable[(s->ang+512)&2047])>>14;
                    l = (s->xvel*sintable[s->ang&2047])>>14;

                    t[3] += s->xvel;

                    s->x += x;
                    s->y += l;

                    if( t[3] <= 0 || (t[3]>>6) >= (SP>>6) )
                    {
                        s->x -= x;
                        s->y -= l;
                        t[0] = 0;
                        callsound(s->sectnum,i);
                        break;
                    }

                    j = headspritesect[s->sectnum];
                    while(j >= 0)
                    {
                        nextj = nextspritesect[j];

                        if( sprite[j].statnum != 3 && sprite[j].zvel == 0)
                        {
                            sprite[j].x += x;
                            sprite[j].y += l;
                            setsprite(j,sprite[j].x,sprite[j].y,sprite[j].z);
                            if( sector[sprite[j].sectnum].floorstat&2 )
                                if(sprite[j].statnum == 2)
                                    makeitfall(j);
                        }
                        j = nextj;
                    }

                    dragpoint((short)t[1],wall[t[1]].x+x,wall[t[1]].y+l);
                    dragpoint((short)t[2],wall[t[2]].x+x,wall[t[2]].y+l);

                    for(p=connecthead;p>=0;p=connectpoint2[p])
                        if(ps[p].cursectnum == s->sectnum && ps[p].on_ground)
                        {
                            ps[p].posx += x;
                            ps[p].posy += l;

                            ps[p].oposx = ps[p].posx;
                            ps[p].oposy = ps[p].posy;

                            setsprite(ps[p].i,ps[p].posx,ps[p].posy,ps[p].posz+PHEIGHT);
                        }

                    sc->floorxpanning-=x>>3;
                    sc->floorypanning-=l>>3;

                    sc->ceilingxpanning-=x>>3;
                    sc->ceilingypanning-=l>>3;
                }

                break;

            case 21: // Cascading effect

                if( t[0] == 0 ) break;

                if( s->ang == 1536 )
                    l = (long) &sc->ceilingz;
                else
                    l = (long) &sc->floorz;

                if( t[0] == 1 ) //Decide if the s->sectnum should go up or down
                {
                    s->zvel = ksgn(s->z-*(long *)l) * (SP<<4);
                    t[0]++;
                }

                if( sc->extra == 0 )
                {
                    *(long *)l += s->zvel;

                    if(klabs(*(long *)l-s->z) < 1024)
                    {
                        *(long *)l = s->z;
                        KILLIT(i); //All done
                    }
                }
                else sc->extra--;
                break;

            case 22:

                if( t[1] )
                {
                    if(getanimationgoal(&sector[t[0]].ceilingz) >= 0)
                        sc->ceilingz += sc->extra*9;
                    else t[1] = 0;
                }
                break;

            case 24:
            case 34:
#ifdef RRRA
            case 156:
#endif

                if(t[4]) break;

                x = (SP*sintable[(s->ang+512)&2047])>>18;
                l = (SP*sintable[s->ang&2047])>>18;

                k = 0;

                j = headspritesect[s->sectnum];
                while(j >= 0)
                {
                    nextj = nextspritesect[j];
                    if(sprite[j].zvel >= 0)
                        switch(sprite[j].statnum)
                    {
                        case 5:
                            switch(sprite[j].picnum)
                            {
                                case BLOODPOOL:
                                case FOOTPRINTS:
                                case FOOTPRINTS2:
                                case FOOTPRINTS3:
                                    sprite[j].xrepeat = sprite[j].yrepeat = 0;
                                    k = 1;
                                    break;
                                case BULLETHOLE:
                                    j = nextj;
                                    continue;
                            }
                        case 6:
                        case 1:
                        case 0:
                            if(
                                sprite[j].picnum == BOLT1 ||
                                sprite[j].picnum == BOLT1+1 ||
                                sprite[j].picnum == BOLT1+2 ||
                                sprite[j].picnum == BOLT1+3 ||
                                wallswitchcheck(j)
                              )
                              break;

                            if( !(sprite[j].picnum >= CRANE && sprite[j].picnum <= (CRANE+3)))
                            {
                                if( sprite[j].z > (hittype[j].floorz-(16<<8)) )
                                {
                                    hittype[j].bposx = sprite[j].x;
                                    hittype[j].bposy = sprite[j].y;

                                    sprite[j].x += x>>1;
                                    sprite[j].y += l>>1;

                                    setsprite(j,sprite[j].x,sprite[j].y,sprite[j].z);

                                    if( sector[sprite[j].sectnum].floorstat&2 )
                                        if(sprite[j].statnum == 2)
                                            makeitfall(j);
                                }
                            }
                            break;
                    }
                    j = nextj;
                }

                p = myconnectindex;
                if(ps[p].cursectnum == s->sectnum && ps[p].on_ground)
                    if( klabs(ps[p].posz-ps[p].truefz) < PHEIGHT+(9<<8) )
                {
                    fricxv += x<<3;
                    fricyv += l<<3;
                }

#ifdef RRRA
                if (st != 156)
#endif
                sc->floorxpanning += SP>>7;

            break;

            case 35:
                if(sc->ceilingz > s->z)
                    for(j = 0;j < 8;j++)
                {
                    s->ang += TRAND&511;
                    k = spawn(i,SMALLSMOKE);
                    sprite[k].xvel = 96+(TRAND&127);
                    ssp(k,CLIPMASK0);
                    setsprite(k,sprite[k].x,sprite[k].y,sprite[k].z);
                    if( rnd(16) )
                        spawn(i,EXPLOSION2);
                }

                switch(t[0])
                {
                    case 0:
                        sc->ceilingz += s->yvel;
                        if(sc->ceilingz > sc->floorz)
                            sc->floorz = sc->ceilingz;
                        if(sc->ceilingz > s->z+(32<<8))
                            t[0]++;
                        break;
                    case 1:
                        sc->ceilingz-=(s->yvel<<2);
                        if(sc->ceilingz < t[4])
                        {
                            sc->ceilingz = t[4];
                            t[0] = 0;
                        }
                        break;
                }
                break;

            case 25: //PISTONS

                if( t[4] == 0 ) break;

                if(sc->floorz <= sc->ceilingz)
                    s->shade = 0;
                else if( sc->ceilingz <= t[4])
                    s->shade = 1;

#ifdef RRRA
                if(s->shade)
                {
                    sc->ceilingz += SP<<4;
                    if(sc->ceilingz > sc->floorz)
                    {
                        sc->ceilingz = sc->floorz;
                        if (ps[screenpeek].raat601)
                            spritesound(371, i);
                    }
                }
                else
                {
                    sc->ceilingz   -= SP<<4;
                    if(sc->ceilingz < t[4])
                    {
                        sc->ceilingz = t[4];
                        if (ps[screenpeek].raat601)
                            spritesound(167, i);
                    }
                }
#else
                if(s->shade)
                {
                    sc->ceilingz += SP<<4;
                    if(sc->ceilingz > sc->floorz)
                        sc->ceilingz = sc->floorz;
                }
                else
                {
                    sc->ceilingz   -= SP<<4;
                    if(sc->ceilingz < t[4])
                        sc->ceilingz = t[4];
                }
#endif

                break;

            case 26:

                s->xvel = 32;
                l = (s->xvel*sintable[(s->ang+512)&2047])>>14;
                x = (s->xvel*sintable[s->ang&2047])>>14;

                s->shade++;
                if( s->shade > 7 )
                {
                    s->x = t[3];
                    s->y = t[4];
                    sc->floorz -= ((s->zvel*s->shade)-s->zvel);
                    s->shade = 0;
                }
                else
                    sc->floorz += s->zvel;

                j = headspritesect[s->sectnum];
                while( j >= 0 )
                {
                    nextj = nextspritesect[j];
                    if(sprite[j].statnum != 3 && sprite[j].statnum != 10)
                    {
                        hittype[j].bposx = sprite[j].x;
                        hittype[j].bposy = sprite[j].y;

                        sprite[j].x += l;
                        sprite[j].y += x;

                        sprite[j].z += s->zvel;
                        setsprite(j,sprite[j].x,sprite[j].y,sprite[j].z);
                    }
                    j = nextj;
                }

                p = myconnectindex;
                if(sprite[ps[p].i].sectnum == s->sectnum && ps[p].on_ground)
                {
                    fricxv += l<<5;
                    fricyv += x<<5;
                }

                for(p = connecthead;p >= 0;p = connectpoint2[p])
                    if(sprite[ps[p].i].sectnum == s->sectnum && ps[p].on_ground)
                        ps[p].posz += s->zvel;

                ms(i);
                setsprite(i,s->x,s->y,s->z);

                break;


            case 27:

                if(ud.recstat == 0) break;

                hittype[i].tempang = s->ang;

                p = findplayer(s,&x);
                if( sprite[ps[p].i].extra > 0 && myconnectindex == screenpeek)
                {
                    if( t[0] < 0 )
                    {
                        ud.camerasprite = i;
                        t[0]++;
                    }
                    else if(ud.recstat == 2 && ps[p].newowner == -1)
                    {
                        if(cansee(s->x,s->y,s->z,SECT,ps[p].posx,ps[p].posy,ps[p].posz,ps[p].cursectnum))
                        {
                            if(x < (unsigned)sh)
                            {
                                ud.camerasprite = i;
                                t[0] = 999;
                                s->ang += getincangle(s->ang,getangle(ps[p].posx-s->x,ps[p].posy-s->y))>>3;
                                SP = 100+((s->z-ps[p].posz)/257);

                            }
                            else if(t[0] == 999)
                            {
                                if(ud.camerasprite == i)
                                    t[0] = 0;
                                else t[0] = -10;
                                ud.camerasprite = i;

                            }
                        }
                        else
                        {
                            s->ang = getangle(ps[p].posx-s->x,ps[p].posy-s->y);

                            if(t[0] == 999)
                            {
                                if(ud.camerasprite == i)
                                    t[0] = 0;
                                else t[0] = -20;
                                ud.camerasprite = i;
                            }
                        }
                    }
                }
                break;
            case 29:
                s->hitag += 64;
                l = mulscale12((long)s->yvel,sintable[s->hitag&2047]);
                sc->floorz = s->z + l;
                break;
            case 31: // True Drop Floor
                if(t[0] == 1)
                {
                    if(t[2] == 1) // Retract
                    {
                        if(SA != 1536)
                        {
                            if( klabs( sc->floorz - s->z ) < SP )
                            {
                                sc->floorz = s->z;
                                t[2] = 0;
                                t[0] = 0;
                                callsound(s->sectnum,i);
                            }
                            else
                            {
                                l = sgn(s->z-sc->floorz)*SP;
                                sc->floorz += l;

                                j = headspritesect[s->sectnum];
                                while(j >= 0)
                                {
                                    if(sprite[j].picnum == APLAYER && sprite[j].owner >= 0)
                                        if( ps[sprite[j].yvel].on_ground == 1 )
                                            ps[sprite[j].yvel].posz += l;
                                    if( sprite[j].zvel == 0 && sprite[j].statnum != 3)
                                    {
                                        hittype[j].bposz = sprite[j].z += l;
                                        hittype[j].floorz = sc->floorz;
                                    }
                                    j = nextspritesect[j];
                                }
                            }
                        }
                        else
                        {
                            if( klabs( sc->floorz - t[1] ) < SP )
                            {
                                sc->floorz = t[1];
                                callsound(s->sectnum,i);
                                t[2] = 0;
                                t[0] = 0;
                            }
                            else
                            {
                                l = sgn(t[1]-sc->floorz)*SP;
                                sc->floorz += l;

                                j = headspritesect[s->sectnum];
                                while(j >= 0)
                                {
                                    if(sprite[j].picnum == APLAYER && sprite[j].owner >= 0)
                                        if( ps[sprite[j].yvel].on_ground == 1 )
                                            ps[sprite[j].yvel].posz += l;
                                    if( sprite[j].zvel == 0 && sprite[j].statnum != 3)
                                    {
                                        hittype[j].bposz = sprite[j].z += l;
                                        hittype[j].floorz = sc->floorz;
                                    }
                                    j = nextspritesect[j];
                                }
                            }
                        }
                        break;
                    }

                    if( (s->ang&2047) == 1536)
                    {
                        if( klabs( s->z-sc->floorz ) < SP )
                        {
                            callsound(s->sectnum,i);
                            t[0] = 0;
                            t[2] = 1;
                        }
                        else
                        {
                            l = sgn(s->z-sc->floorz)*SP;
                            sc->floorz += l;

                            j = headspritesect[s->sectnum];
                            while(j >= 0)
                            {
                                if(sprite[j].picnum == APLAYER && sprite[j].owner >= 0)
                                    if( ps[sprite[j].yvel].on_ground == 1 )
                                        ps[sprite[j].yvel].posz += l;
                                if( sprite[j].zvel == 0 && sprite[j].statnum != 3)
                                {
                                    hittype[j].bposz = sprite[j].z += l;
                                    hittype[j].floorz = sc->floorz;
                                }
                                j = nextspritesect[j];
                            }
                        }
                    }
                    else
                    {
                        if( klabs( sc->floorz-t[1] ) < SP )
                        {
                            t[0] = 0;
                            callsound(s->sectnum,i);
                            t[2] = 1;
                        }
                        else
                        {
                            l = sgn(s->z-t[1])*SP;
                            sc->floorz -= l;

                            j = headspritesect[s->sectnum];
                            while(j >= 0)
                            {
                                if(sprite[j].picnum == APLAYER && sprite[j].owner >= 0)
                                    if( ps[sprite[j].yvel].on_ground == 1 )
                                        ps[sprite[j].yvel].posz -= l;
                                if(sprite[j].zvel == 0 && sprite[j].statnum != 3)
                                {
                                    hittype[j].bposz = sprite[j].z -= l;
                                    hittype[j].floorz = sc->floorz;
                                }
                                j = nextspritesect[j];
                            }
                        }
                    }
                }
                break;

           case 32: // True Drop Ceiling
                if(t[0] == 1)
                {
                    // Choose dir

                    if(t[2] == 1) // Retract
                    {
                        if(SA != 1536)
                        {
                            if( klabs( sc->ceilingz - s->z ) <
                                (SP<<1) )
                            {
                                sc->ceilingz = s->z;
                                callsound(s->sectnum,i);
                                t[2] = 0;
                                t[0] = 0;
                            }
                            else sc->ceilingz +=
                                sgn(s->z-sc->ceilingz)*SP;
                        }
                        else
                        {
                            if( klabs( sc->ceilingz - t[1] ) <
                                (SP<<1) )
                            {
                                sc->ceilingz = t[1];
                                callsound(s->sectnum,i);
                                t[2] = 0;
                                t[0] = 0;
                            }
                            else sc->ceilingz +=
                                sgn(t[1]-sc->ceilingz)*SP;
                        }
                        break;
                    }

                    if( (s->ang&2047) == 1536)
                    {
                        if( klabs(sc->ceilingz-s->z ) <
                            (SP<<1) )
                        {
                            t[0] = 0;
                            t[2] = !t[2];
                            callsound(s->sectnum,i);
                            sc->ceilingz = s->z;
                        }
                        else sc->ceilingz +=
                            sgn(s->z-sc->ceilingz)*SP;
                    }
                    else
                    {
                        if( klabs(sc->ceilingz-t[1] ) < (SP<<1) )
                        {
                            t[0] = 0;
                            t[2] = !t[2];
                            callsound(s->sectnum,i);
                        }
                        else sc->ceilingz -= sgn(s->z-t[1])*SP;
                    }
                }
                break;

            case 33:
                if( earthquaketime > 0 && (TRAND&7) == 0 )
                    RANDOMSCRAP;
                break;
            case 36:

                if( t[0] )
                {
                    if( t[0] == 1 )
                        shoot(i,sc->extra);
                    else if( t[0] == 26*5 )
                        t[0] = 0;
                    t[0]++;
                }
                break;

            case 128: //SE to control glass breakage

                wal = &wall[t[2]];

                if(wal->cstat|32)
                {
                    wal->cstat &= (255-32);
                    wal->cstat |= 16;
                    if(wal->nextwall >= 0)
                    {
                        wall[wal->nextwall].cstat &= (255-32);
                        wall[wal->nextwall].cstat |= 16;
                    }
                }
                else break;

                wal->overpicnum++;
                if(wal->nextwall >= 0)
                    wall[wal->nextwall].overpicnum++;

                if(t[0] < t[1]) t[0]++;
                else
                {
                    wal->cstat &= (128+32+8+4+2);
                    if(wal->nextwall >= 0)
                        wall[wal->nextwall].cstat &= (128+32+8+4+2);
                    KILLIT(i);
                }
                break;

            case 130:
                if(t[0] > 80) { KILLIT(i); }
                else t[0]++;

                x = sc->floorz-sc->ceilingz;

                if( rnd(64) )
                {
                    k = spawn(i,EXPLOSION2);
                    sprite[k].xrepeat = sprite[k].yrepeat = 2+(TRAND&7);
                    sprite[k].z = sc->floorz-(TRAND%x);
                    sprite[k].ang += 256-(TRAND%511);
                    sprite[k].xvel = TRAND&127;
                    ssp(k,CLIPMASK0);
                }
                break;
            case 131:
                if(t[0] > 40) { KILLIT(i); }
                else t[0]++;

                x = sc->floorz-sc->ceilingz;

                if( rnd(32) )
                {
                    k = spawn(i,EXPLOSION2);
                    sprite[k].xrepeat = sprite[k].yrepeat = 2+(TRAND&3);
                    sprite[k].z = sc->floorz-(TRAND%x);
                    sprite[k].ang += 256-(TRAND%511);
                    sprite[k].xvel = TRAND&127;
                    ssp(k,CLIPMASK0);
                }
                break;
        }
        BOLT:
        i = nexti;
    }

         //Sloped sin-wave floors!
     for(i=headspritestat[3];i>=0;i=nextspritestat[i])
     {
          s = &sprite[i];
          if (s->lotag != 29) continue;
          sc = &sector[s->sectnum];
          if (sc->wallnum != 4) continue;
          wal = &wall[sc->wallptr+2];
          alignflorslope(s->sectnum,wal->x,wal->y,sector[wal->nextsector].floorz);
     }
}
