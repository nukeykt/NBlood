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

void bounce(short i)
{
    long k, l, daang, dax, day, daz, xvect, yvect, zvect;
    short hitsect;
    spritetype *s = &sprite[i];

    xvect = mulscale10(s->xvel,sintable[(s->ang+512)&2047]);
    yvect = mulscale10(s->xvel,sintable[s->ang&2047]);
    zvect = s->zvel;

    hitsect = s->sectnum;

    k = sector[hitsect].wallptr; l = wall[k].point2;
    daang = getangle(wall[l].x-wall[k].x,wall[l].y-wall[k].y);

    if ( s->z < (hittype[i].floorz+hittype[i].ceilingz)>>1)
        k = sector[hitsect].ceilingheinum;
    else
        k = sector[hitsect].floorheinum;

    dax = mulscale14(k,sintable[(daang)&2047]);
    day = mulscale14(k,sintable[(daang+1536)&2047]);
    daz = 4096;

    k = xvect*dax+yvect*day+zvect*daz;
    l = dax*dax+day*day+daz*daz;
    if ((klabs(k)>>14) < l)
    {
        k = divscale17(k,l);
        xvect -= mulscale16(dax,k);
        yvect -= mulscale16(day,k);
        zvect -= mulscale16(daz,k);
    }

    s->zvel = zvect;
    s->xvel = ksqrt(dmulscale8(xvect,xvect,yvect,yvect));
    s->ang = getangle(xvect,yvect);
}
     
void moveweapons(void)
{
    short i, j, k, nexti, p, q, tempsect;
    long dax,day,daz, x, l, ll, x1, y1;
    unsigned long qq;
    spritetype *s;

    i = headspritestat[4];
    while(i >= 0)
    {
        nexti = nextspritestat[i];
        s = &sprite[i];

        if(s->sectnum < 0) KILLIT(i);

        hittype[i].bposx = s->x;
        hittype[i].bposy = s->y;
        hittype[i].bposz = s->z;

        switch(s->picnum)
        {
            case RADIUSEXPLOSION:
                KILLIT(i);
            case TONGUE:
                T1 = sintable[(T2)&2047]>>9;
                T2 += 32;
                if(T2 > 2047) KILLIT(i);

                if(sprite[s->owner].statnum == MAXSTATUS)
                    if(badguy(&sprite[s->owner]) == 0)
                        KILLIT(i);

                s->ang = sprite[s->owner].ang;
                s->x = sprite[s->owner].x;
                s->y = sprite[s->owner].y;
                if(sprite[s->owner].picnum == APLAYER)
                    s->z = sprite[s->owner].z-(34<<8);
                for(k=0;k<T1;k++)
                {
                    q = EGS(s->sectnum,
                        s->x+((k*sintable[(s->ang+512)&2047])>>9),
                        s->y+((k*sintable[s->ang&2047])>>9),
                        s->z+((k*ksgn(s->zvel))*klabs(s->zvel/12)),TONGUE,-40+(k<<1),
                        8,8,0,0,0,i,5);
                    sprite[q].cstat = 128;
                    sprite[q].pal = 8;
                }
                q = EGS(s->sectnum,
                    s->x+((k*sintable[(s->ang+512)&2047])>>9),
                    s->y+((k*sintable[s->ang&2047])>>9),
                    s->z+((k*ksgn(s->zvel))*klabs(s->zvel/12)),INNERJAW,-40,
                    32,32,0,0,0,i,5);
                sprite[q].cstat = 128;
                if( T2 > 512 && T2 < (1024) )
                    sprite[q].picnum = INNERJAW+1;

                goto BOLT;

            case FREEZEBLAST:
                if(s->yvel < 1 || s->extra < 2 || (s->xvel|s->zvel) == 0)
                {
                    j = spawn(i,TRANSPORTERSTAR);
                    sprite[j].pal = 1;
                    sprite[j].xrepeat = 32;
                    sprite[j].yrepeat = 32;
                    KILLIT(i);
                }
            case SHRINKSPARK:
            case RPG:
            case FIRELASER:
            case SPIT:
            case COOLEXPLOSION1:
            case OWHIP:
            case UWHIP:
#ifdef RRRA
            case RPG2:
            case RRTILE1790:
#endif
                p = -1;

                if(s->picnum == RPG && sector[s->sectnum].lotag == 2)
                {
                    k = s->xvel>>1;
                    ll = s->zvel>>1;
                }
#ifdef RRRA
                else if(s->picnum == RPG2 && sector[s->sectnum].lotag == 2)
                {
                    k = s->xvel>>1;
                    ll = s->zvel>>1;
                }
#endif
                else
                {
                    k = s->xvel;
                    ll = s->zvel;
                }

                dax = s->x; day = s->y; daz = s->z;

                getglobalz(i);
                qq = CLIPMASK1;

                switch(s->picnum)
                {
                    case RPG:
                        if(hittype[i].picnum != BOSS2 && s->xrepeat >= 10 && sector[s->sectnum].lotag != 2)
                        {
                            j = spawn(i,SMALLSMOKE);
                            sprite[j].z += (1<<8);
                        }
                        break;
#ifdef RRRA
                    case RPG2:
                        s->hitag++;
                        if(hittype[i].picnum != BOSS2 && s->xrepeat >= 10 && sector[s->sectnum].lotag != 2)
                        {
                            j = spawn(i,SMALLSMOKE);
                            sprite[j].z += (1<<8);
                            if ((TRAND & 15) == 2)
                            {
                                j = spawn(i,1310);
                            }
                        }
                        if (sprite[s->lotag].extra <= 0)
                            s->lotag = 0;
                        if (s->lotag != 0 && s->hitag > 5)
                        {
                            spritetype *ts;
                            int ang, ang2, ang3;
                            ts = &sprite[s->lotag];
                            ang = getangle(ts->x - s->x, ts->y - s->y);
                            ang2 = ang - s->ang;
                            ang3 = abs(ang2);
                            if (ang2 < 100)
                            {
                                if (ang3 > 1023)
                                    s->ang += 51;
                                else
                                    s->ang -= 51;
                            }
                            else if (ang2 > 100)
                            {
                                if (ang3 > 1023)
                                    s->ang -= 51;
                                else
                                    s->ang += 51;
                            }
                            else
                                s->ang = ang;

                            if (s->hitag > 180)
                                if (s->zvel <= 0)
                                s->zvel += 200;
                        }
                        break;
                    case RRTILE1790:
                        if (s->extra)
                        {
                            s->zvel = -(s->extra * 250);
                            s->extra--;
                        }
                        else
                            makeitfall(i);
                        if (s->xrepeat >= 10 && sector[s->sectnum].lotag != 2)
                        {
                            j = spawn(i,SMALLSMOKE);
                            sprite[j].z += (1<<8);
                        }
                        break;
#endif
                }

                j = movesprite(i,
                    (k*(sintable[(s->ang+512)&2047]))>>14,
                    (k*(sintable[s->ang&2047]))>>14,ll,qq);

#ifdef RRRA
                if(s->picnum == RPG && s->yvel >= 0)
                {
                    if( FindDistance2D(s->x-sprite[s->yvel].x,s->y-sprite[s->yvel].y) < 256 )
                        j = 49152|s->yvel;
                }
                else if(s->picnum == RPG2 && s->yvel >= 0)
                {
                    if( FindDistance2D(s->x-sprite[s->yvel].x,s->y-sprite[s->yvel].y) < 256 )
                        j = 49152|s->yvel;
                }
                else if(s->picnum == RRTILE1790 && s->yvel >= 0)
                {
                    if( FindDistance2D(s->x-sprite[s->yvel].x,s->y-sprite[s->yvel].y) < 256 )
                        j = 49152|s->yvel;
                }
#else
                if(s->picnum == RPG && s->yvel >= 0)
                    if( FindDistance2D(s->x-sprite[s->yvel].x,s->y-sprite[s->yvel].y) < 256 )
                        j = 49152|s->yvel;
#endif

                if(s->sectnum < 0) { KILLIT(i); }

                if(sector[s->sectnum].filler == 800) { KILLIT(i); }

                if( (j&49152) != 49152)
                    if(s->picnum != FREEZEBLAST)
                {
                    if(s->z < hittype[i].ceilingz)
                    {
                        j = 16384|(s->sectnum);
                        s->zvel = -1;
                    }
                    else
                        if(s->z > hittype[i].floorz)
                    {
                        j = 16384|(s->sectnum);
                        if(sector[s->sectnum].lotag != 1)
                            s->zvel = 1;
                    }
                }

                if(s->picnum == FIRELASER)
                {
                    for(k=-3;k<2;k++)
                    {
                        x = EGS(s->sectnum,
                            s->x+((k*sintable[(s->ang+512)&2047])>>9),
                            s->y+((k*sintable[s->ang&2047])>>9),
                            s->z+((k*ksgn(s->zvel))*klabs(s->zvel/24)),FIRELASER,-40+(k<<2),
                            s->xrepeat,s->yrepeat,0,0,0,s->owner,5);

                        sprite[x].cstat = 128;
                        sprite[x].pal = s->pal;
                    }
                }
                else if(s->picnum == SPIT) if(s->zvel < 6144)
                    s->zvel += gc-112;

                if( j != 0 )
                {
                    if( (j&49152) == 49152 )
                    {
                        j &= (MAXSPRITES-1);

#ifdef RRRA
                        if (sprite[j].picnum == MINION && (s->picnum == RPG || s->picnum == RPG2) && sprite[j].pal == 19)
                        {
                            spritesound(RPG_EXPLODE,i);
                            k = spawn(i,EXPLOSION2);
                            sprite[k].x = dax;
                            sprite[k].y = day;
                            sprite[k].z = daz;
                            goto BOLT;
                        }

#else
                        if(s->picnum == FREEZEBLAST && sprite[j].pal == 1 )
                            if( badguy(&sprite[j]) || sprite[j].picnum == APLAYER )
                        {
                            j = spawn(i,TRANSPORTERSTAR);
                            sprite[j].pal = 1;
                            sprite[j].xrepeat = 32;
                            sprite[j].yrepeat = 32;

                            KILLIT(i);
                        }
#endif

                        checkhitsprite(j,i);

                        if(sprite[j].picnum == APLAYER)
                        {
                            p = sprite[j].yvel;
                            spritesound(PISTOL_BODYHIT,j);

                            if(s->picnum == SPIT)
                            {
#ifdef RRRA
                                if (sprite[s->owner].picnum == MAMA)
                                {
                                    guts(s,RABBITJIBA,2,myconnectindex);
                                    guts(s,RABBITJIBB,2,myconnectindex);
                                    guts(s,RABBITJIBC,2,myconnectindex);
                                }
#endif
                                ps[p].horiz += 32;
                                ps[p].return_to_center = 8;

                                if(ps[p].loogcnt == 0)
                                {
                                    if(Sound[DUKE_LONGTERM_PAIN].num < 1)
                                        spritesound(DUKE_LONGTERM_PAIN,ps[p].i);

                                    j = 3+(TRAND&3);
                                    ps[p].numloogs = j;
                                    ps[p].loogcnt = 24*4;
                                    for(x=0;x < j;x++)
                                    {
                                        ps[p].loogiex[x] = TRAND%xdim;
                                        ps[p].loogiey[x] = TRAND%ydim;
                                    }
                                }
                            }
                        }
                    }
                    else if( (j&49152) == 32768 )
                    {
                        j &= (MAXWALLS-1);

#ifdef RRRA
                        if (sprite[s->owner].picnum == MAMA)
                        {
                            guts(s,RABBITJIBA,2,myconnectindex);
                            guts(s,RABBITJIBB,2,myconnectindex);
                            guts(s,RABBITJIBC,2,myconnectindex);
                        }

                        if(s->picnum != RPG && s->picnum != RPG2 && s->picnum != FREEZEBLAST && s->picnum != SPIT && s->picnum != SHRINKSPARK && ( wall[j].overpicnum == MIRROR || wall[j].picnum == MIRROR ) )
#else
                        if(s->picnum != RPG && s->picnum != FREEZEBLAST && s->picnum != SPIT && s->picnum != SHRINKSPARK && ( wall[j].overpicnum == MIRROR || wall[j].picnum == MIRROR ) )
#endif
                        {
                            k = getangle(
                                    wall[wall[j].point2].x-wall[j].x,
                                    wall[wall[j].point2].y-wall[j].y);
                            s->ang = ((k<<1) - s->ang)&2047;
                            s->owner = i;
                            spawn(i,TRANSPORTERSTAR);
                            goto BOLT;
                        }
                        else
                        {
                            setsprite(i,dax,day,daz);
                            checkhitwall(i,j,s->x,s->y,s->z,s->picnum);

#ifndef RRRA
                            if(s->picnum == FREEZEBLAST)
                            {
                                if( wall[j].overpicnum != MIRROR && wall[j].picnum != MIRROR )
                                {
                                    s->extra >>= 1;
                                    if (s->xrepeat > 8)
                                        s->xrepeat -= 2;
                                    if (s->yrepeat > 8)
                                        s->yrepeat -= 2;
                                    s->yvel--;
                                }

                                k = getangle(
                                    wall[wall[j].point2].x-wall[j].x,
                                    wall[wall[j].point2].y-wall[j].y);
                                s->ang = ((k<<1) - s->ang)&2047;
                                goto BOLT;
                            }
#endif
                            if (s->picnum == SHRINKSPARK)
                            {
                                if (wall[j].picnum >= RRTILE3643 && wall[j].picnum < RRTILE3643+3) KILLIT(i);
                                if (s->extra <= 0)
                                {
                                    s->x += sintable[(s->ang+512)&2047]>>7;
                                    s->y += sintable[s->ang&2047]>>7;
#ifdef RRRA
                                    if (sprite[s->owner].picnum != CHEER && sprite[s->owner].picnum != CHEERSTAYPUT)
                                    {
                                        j = spawn(i,CIRCLESTUCK);
                                        sprite[j].xrepeat = 8;
                                        sprite[j].yrepeat = 8;
                                        sprite[j].cstat = 16;
                                        sprite[j].ang = (sprite[j].ang+512)&2047;
                                        sprite[j].clipdist = mulscale7(s->xrepeat,tilesizx[s->picnum]);
                                    }
#else
                                    j = spawn(i,CIRCLESTUCK);
                                    sprite[j].xrepeat = 8;
                                    sprite[j].yrepeat = 8;
                                    sprite[j].cstat = 16;
                                    sprite[j].ang = (sprite[j].ang+512)&2047;
                                    sprite[j].clipdist = mulscale7(s->xrepeat,tilesizx[s->picnum]);
#endif
                                    KILLIT(i);
                                }
                                if( wall[j].overpicnum != MIRROR && wall[j].picnum != MIRROR )
                                {
                                    s->extra -= 20;
                                    s->yvel--;
                                }

                                k = getangle(
                                    wall[wall[j].point2].x-wall[j].x,
                                    wall[wall[j].point2].y-wall[j].y);
                                s->ang = ((k<<1) - s->ang)&2047;
                                goto BOLT;
                            }
                        }
                    }
                    else if( (j&49152) == 16384)
                    {
                        setsprite(i,dax,day,daz);

#ifdef RRRA
                        if (sprite[s->owner].picnum == MAMA)
                        {
                            guts(s,RABBITJIBA,2,myconnectindex);
                            guts(s,RABBITJIBB,2,myconnectindex);
                            guts(s,RABBITJIBC,2,myconnectindex);
                        }
#endif

                        if(s->zvel < 0)
                        {
                            if( sector[s->sectnum].ceilingstat&1 )
                                if(sector[s->sectnum].ceilingpal == 0)
                                    KILLIT(i);

                            checkhitceiling(s->sectnum);
                        }

#ifndef RRRA
                        if(s->picnum == FREEZEBLAST)
                        {
                            bounce(i);
                            ssp(i,qq);
                            s->extra >>= 1;
                            if(s->xrepeat > 8)
                                s->xrepeat -= 2;
                            if(s->yrepeat > 8)
                                s->yrepeat -= 2;
                            s->yvel--;
                            goto BOLT;
                        }
#endif
                    }

                    if(s->picnum != SPIT)
                    {
                        if(s->picnum == RPG)
                        {
                            k = spawn(i,EXPLOSION2);
                            sprite[k].x = dax;
                            sprite[k].y = day;
                            sprite[k].z = daz;

                            if(s->xrepeat < 10)
                            {
                                sprite[k].xrepeat = 6;
                                sprite[k].yrepeat = 6;
                            }
                            else if( (j&49152) == 16384)
                            {
                                sprite[k].cstat |= 8;
                                sprite[k].z += (48<<8);
                            }
                        }
#ifdef RRRA
                        else if(s->picnum == RPG2)
                        {
                            k = spawn(i,EXPLOSION2);
                            sprite[k].x = dax;
                            sprite[k].y = day;
                            sprite[k].z = daz;

                            if(s->xrepeat < 10)
                            {
                                sprite[k].xrepeat = 6;
                                sprite[k].yrepeat = 6;
                            }
                            else if( (j&49152) == 16384)
                            {
                                sprite[k].cstat |= 8;
                                sprite[k].z += (48<<8);
                            }
                        }
                        else if(s->picnum == RRTILE1790)
                        {
                            s->extra = 160;
                            k = spawn(i,EXPLOSION2);
                            sprite[k].x = dax;
                            sprite[k].y = day;
                            sprite[k].z = daz;

                            if(s->xrepeat < 10)
                            {
                                sprite[k].xrepeat = 6;
                                sprite[k].yrepeat = 6;
                            }
                            else if( (j&49152) == 16384)
                            {
                                sprite[k].cstat |= 8;
                                sprite[k].z += (48<<8);
                            }
                        }
#endif
                        else if( s->picnum != FREEZEBLAST && s->picnum != FIRELASER && s->picnum != SHRINKSPARK)
                        {
                            k = spawn(i,1441);
                            sprite[k].xrepeat = sprite[k].yrepeat = s->xrepeat>>1;
                            if( (j&49152) == 16384)
                            {
                                if( s->zvel < 0)
                                    { sprite[k].cstat |= 8; sprite[k].z += (72<<8); }
                            }
                        }
                        if( s->picnum == RPG )
                        {
                            spritesound(RPG_EXPLODE,i);

                            if(s->xrepeat >= 10)
                            {
                                x = s->extra;
                                hitradius( i,rpgblastradius, x>>2,x>>1,x-(x>>2),x);
                            }
                            else
                            {
                                x = s->extra+(global_random&3);
                                hitradius( i,(rpgblastradius>>1),x>>2,x>>1,x-(x>>2),x);
                            }
                        }
#ifdef RRRA
                        else if (s->picnum == RPG2)
                        {
                            s->extra = 150;
                            spritesound(247,i);

                            if(s->xrepeat >= 10)
                            {
                                x = s->extra;
                                hitradius( i,rpgblastradius, x>>2,x>>1,x-(x>>2),x);
                            }
                            else
                            {
                                x = s->extra+(global_random&3);
                                hitradius( i,(rpgblastradius>>1),x>>2,x>>1,x-(x>>2),x);
                            }
                        }
                        else if (s->picnum == RRTILE1790)
                        {
                            s->extra = 160;
                            spritesound(RPG_EXPLODE,i);

                            if(s->xrepeat >= 10)
                            {
                                x = s->extra;
                                hitradius( i,rpgblastradius, x>>2,x>>1,x-(x>>2),x);
                            }
                            else
                            {
                                x = s->extra+(global_random&3);
                                hitradius( i,(rpgblastradius>>1),x>>2,x>>1,x-(x>>2),x);
                            }
                        }
#endif
                    }
                    KILLIT(i);
                }
#ifdef RRRA
                if((s->picnum == RPG || s->picnum == RPG2) && sector[s->sectnum].lotag == 2 && s->xrepeat >= 10 && rnd(184))
#else
                if(s->picnum == RPG && sector[s->sectnum].lotag == 2 && s->xrepeat >= 10 && rnd(184))
#endif
                    spawn(i,WATERBUBBLE);

                goto BOLT;


            case SHOTSPARK1:
                p = findplayer(s,&x);
                execute(i,p,x);
                goto BOLT;
        }
        BOLT:
        i = nexti;
    }
}


void movetransports(void)
{
#ifdef RRRA
    char warpdir, warpspriteto;
#else
    char warpspriteto;
#endif
    short i, j, k, l, p, sect, sectlotag, nexti, nextj, nextk;
#ifdef RRRA
    long ll2,ll,onfloorz,q;
#else
    long ll,onfloorz,q;
#endif

    i = headspritestat[9]; //Transporters

    while(i >= 0)
    {
        sect = SECT;
        sectlotag = sector[sect].lotag;

        nexti = nextspritestat[i];

        if(OW == i)
        {
            i = nexti;
            continue;
        }

        onfloorz = T5;

        if(T1 > 0) T1--;

        j = headspritesect[sect];
        while(j >= 0)
        {
            nextj = nextspritesect[j];

            switch(sprite[j].statnum)
            {
                case 10:    // Player

                    if( sprite[j].owner != -1 )
                    {
                        p = sprite[j].yvel;

                        ps[p].on_warping_sector = 1;

                        if( ps[p].transporter_hold == 0 && ps[p].jumping_counter == 0 )
                        {
                            if(ps[p].on_ground && sectlotag == 0 && onfloorz && ps[p].jetpack_on == 0 )
                            {
                                spawn(i,TRANSPORTERBEAM);
                                spritesound(TELEPORTER,i);

                                for(k=connecthead;k>=0;k=connectpoint2[k])
                                    if(ps[k].cursectnum == sprite[OW].sectnum)
                                {
                                    ps[k].frag_ps = p;
                                    sprite[ps[k].i].extra = 0;
                                }

                                ps[p].ang = sprite[OW].ang;

                                if(sprite[OW].owner != OW)
                                {
                                    T1 = 13;
                                    hittype[OW].temp_data[0] = 13;
                                    ps[p].transporter_hold = 13;
                                }

                                ps[p].bobposx = ps[p].oposx = ps[p].posx = sprite[OW].x;
                                ps[p].bobposy = ps[p].oposy = ps[p].posy = sprite[OW].y;
                                ps[p].oposz = ps[p].posz = sprite[OW].z-(PHEIGHT-(4<<8));

                                changespritesect(j,sprite[OW].sectnum);
                                ps[p].cursectnum = sprite[j].sectnum;

                                k = spawn(OW,TRANSPORTERBEAM);
                                spritesound(TELEPORTER,k);

                                break;
                            }
                        }
                        else break;

                        if(onfloorz == 0 && klabs(SZ-ps[p].posz) < 6144 )
                            if( (ps[p].jetpack_on == 0 ) || (ps[p].jetpack_on && (sync[p].bits&1) ) ||
                                (ps[p].jetpack_on && (sync[p].bits&2) ) )
                        {
                            ps[p].oposx = ps[p].posx += sprite[OW].x-SX;
                            ps[p].oposy = ps[p].posy += sprite[OW].y-SY;

                            if( ps[p].jetpack_on && ( (sync[p].bits&1) || ps[p].jetpack_on < 11 ) )
                                ps[p].posz = sprite[OW].z-6144;
                            else ps[p].posz = sprite[OW].z+6144;
                            ps[p].oposz = ps[p].posz;

                            changespritesect(j,sprite[OW].sectnum);
                            ps[p].cursectnum = sprite[OW].sectnum;

                            break;
                        }

                        k = 0;

#ifdef RRRA
                        if( onfloorz && sectlotag == 160 && ps[p].posz > (sector[sect].floorz-(48<<8)) )
                        {
                            k = 2;
                            ps[p].oposz = ps[p].posz =
                                sector[sprite[OW].sectnum].ceilingz+(7<<8);
                        }

                        if( onfloorz && sectlotag == 161 && ps[p].posz < (sector[sect].ceilingz+(6<<8)) )
                        {
                            k = 2;
                            if (sprite[ps[p].i].extra <= 0) break;
                            ps[p].oposz = ps[p].posz =
                                sector[sprite[OW].sectnum].floorz-(49<<8);
                        }

                        if( (onfloorz && sectlotag == 1 && ps[p].posz > (sector[sect].floorz-(6<<8))) ||
                            (onfloorz && sectlotag == 1 && ps[p].OnMotorcycle) )
                        {
                            if (ps[p].OnBoat) break;
                            k = 1;
                            if(screenpeek == p)
                            {
                                FX_StopAllSounds();
                                clearsoundlocks();
                            }
                            spritesound(DUKE_UNDERWATER,ps[p].i);
                            ps[p].oposz = ps[p].posz =
                                sector[sprite[OW].sectnum].ceilingz+(7<<8);
                            if (ps[p].OnMotorcycle)
                                ps[p].raat5b9 = 1;
                        }

                        if( onfloorz && sectlotag == 2 && ps[p].posz < (sector[sect].ceilingz+(6<<8)) )
                        {
                            k = 1;
                            if( sprite[ps[p].i].extra <= 0) break;
                            if(screenpeek == p)
                            {
                                FX_StopAllSounds();
                                clearsoundlocks();
                            }
                            spritesound(DUKE_GASP,ps[p].i);

                            ps[p].oposz = ps[p].posz =
                                sector[sprite[OW].sectnum].floorz-(7<<8);
                        }
#else
                        if( onfloorz && sectlotag == 1 && ps[p].posz > (sector[sect].floorz-(6<<8)) )
                        {
                            k = 1;
                            if(screenpeek == p)
                            {
                                FX_StopAllSounds();
                                clearsoundlocks();
                            }
                            spritesound(48,ps[p].i);
                            ps[p].oposz = ps[p].posz =
                                sector[sprite[OW].sectnum].ceilingz+(7<<8);
                        }

                        if( onfloorz && sectlotag == 2 && ps[p].posz < (sector[sect].ceilingz+(6<<8)) )
                        {
                            k = 1;
                            if( sprite[ps[p].i].extra <= 0) break;
                            if(screenpeek == p)
                            {
                                FX_StopAllSounds();
                                clearsoundlocks();
                            }
                            spritesound(25,ps[p].i);

                            ps[p].oposz = ps[p].posz =
                                sector[sprite[OW].sectnum].floorz-(7<<8);
                        }
#endif

                        if(k == 1)
                        {
                            ps[p].oposx = ps[p].posx += sprite[OW].x-SX;
                            ps[p].oposy = ps[p].posy += sprite[OW].y-SY;

                            if(sprite[OW].owner != OW)
                                ps[p].transporter_hold = -2;
                            ps[p].cursectnum = sprite[OW].sectnum;

                            changespritesect(j,sprite[OW].sectnum);

                            setpal(&ps[p]);

                            if( (TRAND&255) < 32 )
                                spawn(ps[p].i,WATERSPLASH2);
                        }
#ifdef RRRA
                        else if(k == 2)
                        {
                            ps[p].oposx = ps[p].posx += sprite[OW].x-SX;
                            ps[p].oposy = ps[p].posy += sprite[OW].y-SY;

                            if(sprite[OW].owner != OW)
                                ps[p].transporter_hold = -2;
                            ps[p].cursectnum = sprite[OW].sectnum;

                            changespritesect(j,sprite[OW].sectnum);
                        }
#endif
                    }
                    break;

                case 1:
                    if (PN == SHARK) goto JBOLT;
#ifdef RRRA
                    if (PN == CHEERBOAT || PN == HULKBOAT || PN == MINIONBOAT || PN == UFO1)
#else
                    if (PN == UFO1 || PN == UFO2 || PN == UFO3 || PN == UFO4 || PN == UFO5)
#endif
                        goto JBOLT;
                case 4:
                case 5:
                case 13:

                    ll = klabs(sprite[j].zvel);
#ifdef RRRA
                    if (sprite[j].zvel >= 0)
                        warpdir = 2;
                    else
                        warpdir = 1;
#endif

                    {
                        warpspriteto = 0;
                        if( ll && sectlotag == 2 && sprite[j].z < (sector[sect].ceilingz+ll) )
                            warpspriteto = 1;
                        
                        if( ll && sectlotag == 1 && sprite[j].z > (sector[sect].floorz-ll) )
#ifdef RRRA
                            if (sprite[j].picnum != CHEERBOAT && sprite[j].picnum != HULKBOAT && sprite[j].picnum != MINIONBOAT)
#endif
                            warpspriteto = 1;

#ifdef RRRA
                        if( ll && sectlotag == 161 && sprite[j].z < (sector[sect].ceilingz+ll) && warpdir == 1)
                        {
                            warpspriteto = 1;
                            ll2 = ll - klabs(sprite[j].z-sector[sect].ceilingz);
                        }
                        else if (sectlotag == 161 && sprite[j].z < (sector[sect].ceilingz+1000) && warpdir == 1)
                        {
                            warpspriteto = 1;
                            ll2 = 1;
                        }
                        if( ll && sectlotag == 160 && sprite[j].z > (sector[sect].floorz-ll) && warpdir == 2)
                        {
                            warpspriteto = 1;
                            ll2 = ll - klabs(sector[sect].floorz-sprite[j].z);
                        }
                        else if (sectlotag == 160 && sprite[j].z > (sector[sect].floorz-1000) && warpdir == 2)
                        {
                            warpspriteto = 1;
                            ll2 = 1;
                        }
#endif

                        if( sectlotag == 0 && ( onfloorz || klabs(sprite[j].z-SZ) < 4096) )
                        {
                            if( sprite[OW].owner != OW && onfloorz && T1 > 0 && sprite[j].statnum != 5 )
                            {
                                T1++;
                                goto BOLT;
                            }
                            warpspriteto = 1;
                        }

                        if( warpspriteto ) switch(sprite[j].picnum)
                        {
#ifndef RRRA
                            case TRIPBOMBSPRITE:
#endif
                            case TRANSPORTERSTAR:
                            case TRANSPORTERBEAM:
                            case BULLETHOLE:
                            case WATERSPLASH2:
                            case BURNING:
                            case FIRE:
                            case MUD:
                                goto JBOLT;
                            case PLAYERONWATER:
                                if(sectlotag == 2)
                                {
                                    sprite[j].cstat &= 32767;
                                    break;
                                }
                            default:
#ifdef RRRA
                                if(sprite[j].statnum == 5 && !(sectlotag == 1 || sectlotag == 2 || sectlotag == 160 || sectlotag == 161) )
#else
                                if(sprite[j].statnum == 5 && !(sectlotag == 1 || sectlotag == 2) )
#endif
                                    break;

                            case WATERBUBBLE:
                                if( rnd(192) && sprite[j].picnum == WATERBUBBLE)
                                   break;

                                if(sectlotag > 0)
                                {
                                    k = spawn(j,WATERSPLASH2);
                                    if( sectlotag == 1 && sprite[j].statnum == 4 )
                                    {
                                        sprite[k].xvel = sprite[j].xvel>>1;
                                        sprite[k].ang = sprite[j].ang;
                                        ssp(k,CLIPMASK0);
                                    }
                                }
#ifdef RRRA
                                if (sectlotag != 160 && sectlotag != 161)
                                {
                                    switch (sprite[j].picnum)
                                    {
                                        case TRIPBOMBSPRITE:
                                            break;
                                    }
                                }
#endif

                                switch(sectlotag)
                                {
                                    case 0:
                                        if(onfloorz)
                                        {
                                            if( checkcursectnums(sect) == -1 && checkcursectnums(sprite[OW].sectnum)  == -1 )
                                            {
                                                sprite[j].x += (sprite[OW].x-SX);
                                                sprite[j].y += (sprite[OW].y-SY);
                                                sprite[j].z -= SZ - sector[sprite[OW].sectnum].floorz;
                                                sprite[j].ang = sprite[OW].ang;

                                                hittype[j].bposx = sprite[j].x;
                                                hittype[j].bposy = sprite[j].y;
                                                hittype[j].bposz = sprite[j].z;

                                                k = spawn(i,TRANSPORTERBEAM);
                                                spritesound(TELEPORTER,k);

                                                k = spawn(OW,TRANSPORTERBEAM);
                                                spritesound(TELEPORTER,k);

                                                if( sprite[OW].owner != OW )
                                                {
                                                    T1 = 13;
                                                    hittype[OW].temp_data[0] = 13;
                                                }

                                                changespritesect(j,sprite[OW].sectnum);
                                            }
                                        }
                                        else
                                        {
                                            sprite[j].x += (sprite[OW].x-SX);
                                            sprite[j].y += (sprite[OW].y-SY);
                                            sprite[j].z = sprite[OW].z+4096;

                                            hittype[j].bposx = sprite[j].x;
                                            hittype[j].bposy = sprite[j].y;
                                            hittype[j].bposz = sprite[j].z;

                                            changespritesect(j,sprite[OW].sectnum);
                                        }
                                        break;
                                    case 1:
                                        sprite[j].x += (sprite[OW].x-SX);
                                        sprite[j].y += (sprite[OW].y-SY);
                                        sprite[j].z = sector[sprite[OW].sectnum].ceilingz+ll;

                                        hittype[j].bposx = sprite[j].x;
                                        hittype[j].bposy = sprite[j].y;
                                        hittype[j].bposz = sprite[j].z;

                                        changespritesect(j,sprite[OW].sectnum);

                                        break;
                                    case 2:
                                        sprite[j].x += (sprite[OW].x-SX);
                                        sprite[j].y += (sprite[OW].y-SY);
                                        sprite[j].z = sector[sprite[OW].sectnum].floorz-ll;

                                        hittype[j].bposx = sprite[j].x;
                                        hittype[j].bposy = sprite[j].y;
                                        hittype[j].bposz = sprite[j].z;

                                        changespritesect(j,sprite[OW].sectnum);

                                        break;
#ifdef RRRA
                                    case 160:
                                        sprite[j].x += (sprite[OW].x-SX);
                                        sprite[j].y += (sprite[OW].y-SY);
                                        sprite[j].z = sector[sprite[OW].sectnum].ceilingz+ll2;

                                        hittype[j].bposx = sprite[j].x;
                                        hittype[j].bposy = sprite[j].y;
                                        hittype[j].bposz = sprite[j].z;

                                        changespritesect(j,sprite[OW].sectnum);

                                        movesprite(j,(sprite[j].xvel*sintable[(sprite[j].ang+512)&2047])>>14,
                                            (sprite[j].xvel*sintable[sprite[j].ang&2047])>>14,0,CLIPMASK1);

                                        break;
                                    case 161:
                                        sprite[j].x += (sprite[OW].x-SX);
                                        sprite[j].y += (sprite[OW].y-SY);
                                        sprite[j].z = sector[sprite[OW].sectnum].floorz-ll2;

                                        hittype[j].bposx = sprite[j].x;
                                        hittype[j].bposy = sprite[j].y;
                                        hittype[j].bposz = sprite[j].z;

                                        changespritesect(j,sprite[OW].sectnum);

                                        movesprite(j,(sprite[j].xvel*sintable[(sprite[j].ang+512)&2047])>>14,
                                            (sprite[j].xvel*sintable[sprite[j].ang&2047])>>14,0,CLIPMASK1);

                                        break;
#endif
                                }

                                break;
                        }
                }
                break;

            }
            JBOLT:
            j = nextj;
        }
        BOLT:
        i = nexti;
    }
}
