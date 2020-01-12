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

// Savage Baggage Masters

#include "duke3d.h"

int32 turnheldtime; //MED
int32 lastcontroltime; //MED

void setpal(struct player_struct *p)
{
#ifdef RRRA
    if (p->DrugMode) p->palette = palette+1;
    else
#endif
    if(p->heat_on) p->palette = slimepal;
    else switch(sector[p->cursectnum].ceilingpicnum)
    {
        case FLOORSLIME:
        case FLOORSLIME+1:
        case FLOORSLIME+2:
            p->palette = slimepal;
            break;
        default:
            if(sector[p->cursectnum].lotag == 2) p->palette = waterpal;
            else p->palette = palette;
            break;
    }
    restorepalette = 1;
}

void incur_damage( struct player_struct *p )
{
    long  damage = 0L, unk = 0L, shield_damage = 0L;
    short i, damage_source, gut = 0L;

    sprite[p->i].extra -= p->extra_extra8>>8;

    damage = sprite[p->i].extra - p->last_extra;
    if ( damage < 0 )
    {
        p->extra_extra8 = 0;

        if ( p->steroids_amount > 0 && p->steroids_amount < 400)
        {
            shield_damage =  damage * (20 + (TRAND%30)) / 100;
            damage -= shield_damage;
        }
        if (p->drink_amt > 31 && p->drink_amt < 65)
            gut++;
        if (p->eat > 31 && p->eat < 65)
            gut++;

        switch (gut)
        {
            double ddamage;
            case 1:
                ddamage = damage;
                ddamage *= 0.75;
                damage = ddamage;
                break;
            case 2:
                ddamage = damage;
                ddamage *= 0.25;
                damage = ddamage;
                break;
        }

        sprite[p->i].extra = p->last_extra + damage;
    }
}

void quickkill(struct player_struct *p)
{
    p->pals[0] = 48;
    p->pals[1] = 48;
    p->pals[2] = 48;
    p->pals_time = 48;

    sprite[p->i].extra = 0;
    sprite[p->i].cstat |= 32768;
    if(ud.god == 0) guts(&sprite[p->i],JIBS6,8,myconnectindex);
    return;
}

void forceplayerangle(struct player_struct *p)
{
    short n;

    n = 128-(TRAND&255);

    p->horiz += 64;
    p->return_to_center = 9;
    p->look_ang = n>>1;
    p->rotscrnang = n>>1;
}

void tracers(long x1,long y1,long z1,long x2,long y2,long z2,long n)
{
     long i, xv, yv, zv;
     short sect = -1;

	 i = n+1;
	 xv = (x2-x1)/i;
	 yv = (y2-y1)/i;
	 zv = (z2-z1)/i;

     if( ( klabs(x1-x2)+klabs(y1-y2) ) < 3084 )
         return;

	 for(i=n;i>0;i--)
	 {
		  x1 += xv;
		  y1 += yv;
		  z1 += zv;
		  updatesector(x1,y1,&sect);
          if(sect >= 0)
          {
              if(sector[sect].lotag == 2)
                  EGS(sect,x1,y1,z1,WATERBUBBLE,-32,4+(TRAND&3),4+(TRAND&3),TRAND&2047,0,0,ps[0].i,5);
              else
                  EGS(sect,x1,y1,z1,SMALLSMOKE,-32,14,14,0,0,0,ps[0].i,5);
          }
	 }
}

long hits(short i)
{
    long sx,sy,sz;
    short sect,hw,hs;
    long zoff;

    if(PN == APLAYER) zoff = (40<<8);
    else zoff = 0;

    hitscan(SX,SY,SZ-zoff,SECT,
        sintable[(SA+512)&2047],
        sintable[SA&2047],
        0,&sect,&hw,&hs,&sx,&sy,&sz,CLIPMASK1);

    return ( FindDistance2D( sx-SX,sy-SY ) );
}

long hitasprite(short i,short *hitsp)
{
    long sx,sy,sz,zoff;
    short sect,hw;

    if(badguy(&sprite[i]) )
        zoff = (42<<8);
    else if(PN == APLAYER) zoff = (39<<8);
    else zoff = 0;

    hitscan(SX,SY,SZ-zoff,SECT,
        sintable[(SA+512)&2047],
        sintable[SA&2047],
        0,&sect,&hw,hitsp,&sx,&sy,&sz,CLIPMASK1);

    if(hw >= 0 && (wall[hw].cstat&16) && badguy(&sprite[i]) )
        return((1<<30));

    return ( FindDistance2D(sx-SX,sy-SY) );
}

/*
long hitaspriteandwall(short i,short *hitsp,short *hitw,short *x, short *y)
{
    long sz;
    short sect;

    hitscan(SX,SY,SZ,SECT,
        sintable[(SA+512)&2047],
        sintable[SA&2047],
        0,&sect,hitw,hitsp,x,y,&sz,CLIPMASK1);

    return ( FindDistance2D(*x-SX,*y-SY) );
}
*/


void hitawall(struct player_struct *p,short *hitw)
{
    long sx,sy,sz;
    short sect,hs;

    hitscan(p->posx,p->posy,p->posz,p->cursectnum,
        sintable[(p->ang+512)&2047],
        sintable[p->ang&2047],
        0,&sect,hitw,&hs,&sx,&sy,&sz,CLIPMASK0);
}

short aim(spritetype *s,short aang)
{
    char gotshrinker, gotfreezer;
    short i, j, a, k, cans;
    short aimstats[] = {10,13,1,2};
    long dx1, dy1, dx2, dy2, dx3, dy3, smax, sdist;
    long xv, yv;

    a = s->ang;

    j = -1;
    gotshrinker = 0;
    gotfreezer = 0;

    smax = 0x7fffffff;

    dx1 = sintable[(a+512-aang)&2047];
    dy1 = sintable[(a-aang)&2047];
    dx2 = sintable[(a+512+aang)&2047];
    dy2 = sintable[(a+aang)&2047];

    dx3 = sintable[(a+512)&2047];
    dy3 = sintable[a&2047];

    for(k=0;k<4;k++)
    {
        if( j >= 0 )
            break;
        for(i=headspritestat[aimstats[k]];i >= 0;i=nextspritestat[i])
            if( sprite[i].xrepeat > 0 && sprite[i].extra >= 0 && (sprite[i].cstat&(257+32768)) == 257)
                if( badguy(&sprite[i]) || k < 2 )
            {
                if(badguy(&sprite[i]) || PN == APLAYER || PN == SHARK)
                {
                    if( PN == APLAYER &&
                        ud.ffire == 0 &&
                        ud.coop == 1 &&
                        s->picnum == APLAYER &&
                        s != &sprite[i])
                            continue;
                    if(gotshrinker && sprite[i].xrepeat < 30 )
                    {
                        switch(PN)
                        {
                            case SHARK:
                                if(sprite[i].xrepeat < 20) continue;
                                    continue;
                            default:
                                continue;
                        }
                    }
                    if(gotfreezer && sprite[i].pal == 1) continue;
                }

                xv = (SX-s->x);
                yv = (SY-s->y);

                if( (dy1*xv) <= (dx1*yv) )
                    if( ( dy2*xv ) >= (dx2*yv) )
                {
                    sdist = mulscale(dx3,xv,14) + mulscale(dy3,yv,14);
                    if( sdist > 512 && sdist < smax )
                    {
                        if(s->picnum == APLAYER)
                            a = (klabs(scale(SZ-s->z,10,sdist)-(ps[s->yvel].horiz+ps[s->yvel].horizoff-100)) < 100);
                        else a = 1;

                        cans = cansee(SX,SY,SZ-(32<<8),SECT,s->x,s->y,s->z-(32<<8),s->sectnum);

                        if( a && cans )
                        {
                            smax = sdist;
                            j = i;
                        }
                    }
                }
            }
    }

    return j;
}




void shoot(short i,short atwith)
{
    short sect, hitsect, hitspr, hitwall, l, sa, p, j, k, scount;
    long sx, sy, sz, vel, zvel, hitx, hity, hitz, x, oldzvel, dal;
    unsigned char sizx,sizy;
    spritetype *s;

    s = &sprite[i];
    sect = s->sectnum;
    zvel = 0;

    if( s->picnum == APLAYER )
    {
        p = s->yvel;

        sx = ps[p].posx;
        sy = ps[p].posy;
        sz = ps[p].posz+ps[p].pyoff+(4<<8);
        sa = ps[p].ang;

#ifndef RRRA
        ps[p].crack_time = 777;
#endif

    }
    else
    {
        p = -1;
        sa = s->ang;
        sx = s->x;
        sy = s->y;
        sz = s->z-((s->yrepeat*tilesizy[s->picnum])<<1)+(4<<8);
        sz -= (7<<8);
        if(badguy(s))
        {
            sx += (sintable[(sa+1024+96)&2047]>>7);
            sy += (sintable[(sa+512+96)&2047]>>7);
        }
    }

    switch(atwith)
    {
        case BLOODSPLAT1:
        case BLOODSPLAT2:
        case BLOODSPLAT3:
        case BLOODSPLAT4:

            if(p >= 0)
                sa += 64 - (TRAND&127);
            else sa += 1024 + 64 - (TRAND&127);
            zvel = 1024-(TRAND&2047);
        case KNEE:
        case GROWSPARK:
#ifdef RRRA
        case SLINGBLADE:
            if(atwith == KNEE || atwith == GROWSPARK || atwith == SLINGBLADE)
#else
            if(atwith == KNEE || atwith == GROWSPARK)
#endif
            {
                if(p >= 0)
                {
                    zvel = (100-ps[p].horiz-ps[p].horizoff)<<5;
                    sz += (6<<8);
                    sa += 15;
                }
                else
                {
                    j = ps[findplayer(s,&x)].i;
                    zvel = ( (sprite[j].z-sz)<<8 ) / (x+1);
                    sa = getangle(sprite[j].x-sx,sprite[j].y-sy);
                }
            }

//            writestring(sx,sy,sz,sect,sintable[(sa+512)&2047],sintable[sa&2047],zvel<<6);

            hitscan(sx,sy,sz,sect,
                sintable[(sa+512)&2047],
                sintable[sa&2047],zvel<<6,
                &hitsect,&hitwall,&hitspr,&hitx,&hity,&hitz,CLIPMASK1);

#ifdef RRRA
            if (((sector[hitsect].lotag == 160 && zvel > 0) || (sector[hitsect].lotag == 161 && zvel < 0))
                && hitspr == -1 && hitwall == -1)
            {
                short ii;
                for (ii = 0; ii < MAXSPRITES; ii++)
                {
                    if (sprite[ii].sectnum == hitsect && sprite[ii].picnum == SECTOREFFECTOR
                        && sprite[ii].lotag == 7)
                    {
                        int nx, ny, nz;
                        nx = hitx + (sprite[sprite[ii].owner].x - sprite[ii].x);
                        ny = hity + (sprite[sprite[ii].owner].y - sprite[ii].y);
                        if (sector[hitsect].lotag == 161)
                        {
                            nz = sector[sprite[sprite[ii].owner].sectnum].floorz;
                        }
                        else
                        {
                            nz = sector[sprite[sprite[ii].owner].sectnum].ceilingz;
                        }
                        hitscan(nx,ny,nz,sprite[sprite[ii].owner].sectnum,
                            sintable[(sa+512)&2047],
                            sintable[sa&2047],zvel<<6,
                            &hitsect,&hitwall,&hitspr,&hitx,&hity,&hitz,CLIPMASK1);
                        break;
                    }
                }
            }
#endif

            if( atwith == BLOODSPLAT1 ||
                atwith == BLOODSPLAT2 ||
                atwith == BLOODSPLAT3 ||
                atwith == BLOODSPLAT4 )
            {
                if( FindDistance2D(sx-hitx,sy-hity) < 1024 )
                    if( hitwall >= 0 && wall[hitwall].overpicnum != BIGFORCE )
                        if( ( wall[hitwall].nextsector >= 0 && hitsect >= 0 &&
                            sector[wall[hitwall].nextsector].lotag == 0 &&
                                sector[hitsect].lotag == 0 &&
                                    sector[wall[hitwall].nextsector].lotag == 0 &&
                                        (sector[hitsect].floorz-sector[wall[hitwall].nextsector].floorz) > (16<<8) ) ||
                                            ( wall[hitwall].nextsector == -1 && sector[hitsect].lotag == 0 ) )
                                                if( (wall[hitwall].cstat&16) == 0)
                {
                    if(wall[hitwall].nextsector >= 0)
                    {
                        k = headspritesect[wall[hitwall].nextsector];
                        while(k >= 0)
                        {
                            if(sprite[k].statnum == 3 && sprite[k].lotag == 13)
                                return;
                            k = nextspritesect[k];
                        }
                    }

                    if( wall[hitwall].nextwall >= 0 &&
                        wall[wall[hitwall].nextwall].hitag != 0 )
                            return;

                    if(wall[hitwall].hitag == 0)
                    {
                        k = spawn(i,atwith);
                        sprite[k].xvel = -12;
                        sprite[k].ang = getangle(wall[hitwall].x-wall[wall[hitwall].point2].x,
                            wall[hitwall].y-wall[wall[hitwall].point2].y)+512;
                        sprite[k].x = hitx;
                        sprite[k].y = hity;
                        sprite[k].z = hitz;
                        sprite[k].cstat |= (TRAND&4);
                        ssp(k,CLIPMASK0);
                        setsprite(k,sprite[k].x,sprite[k].y,sprite[k].z);
                        if( PN == OOZFILTER )
                            sprite[k].pal = 6;
                    }
                }
                return;
            }

            if(hitsect < 0) break;

            if( ( klabs(sx-hitx)+klabs(sy-hity) ) < 1024 )
            {
                if(hitwall >= 0 || hitspr >= 0)
                {
#ifdef RRRA
                    if (atwith == SLINGBLADE)
                    {
                        j = EGS(hitsect,hitx,hity,hitz,SLINGBLADE,-15,0,0,sa,32,0,i,4);
                        sprite[j].extra += 50;
                    }
                    else
                    {
                        j = EGS(hitsect,hitx,hity,hitz,KNEE,-15,0,0,sa,32,0,i,4);
                        sprite[j].extra += (TRAND&7);
                    }
#else
                    j = EGS(hitsect,hitx,hity,hitz,KNEE,-15,0,0,sa,32,0,i,4);
                    sprite[j].extra += (TRAND&7);
#endif
                    if(p >= 0)
                    {
                        k = spawn(j,SMALLSMOKE);
                        sprite[k].z -= (8<<8);
                        if(atwith == KNEE)
                            spritesound(KICK_HIT,j);
#ifdef RRRA
                        else if (atwith == SLINGBLADE)
                            spritesound(260,j);
#endif
                    }

                    if ( p >= 0 && ps[p].steroids_amount > 0 && ps[p].steroids_amount < 400 )
                        sprite[j].extra += (max_player_health>>2);

                    if( hitspr >= 0 && sprite[hitspr].picnum != ACCESSSWITCH && sprite[hitspr].picnum != ACCESSSWITCH2 )
                    {
                        checkhitsprite(hitspr,j);
                        if(p >= 0) checkhitswitch(p,hitspr,1);
                    }
                    else if (hitwall >= 0)
                    {
                        if (wall[hitwall].cstat & 2)
                            if (wall[hitwall].nextsector >= 0)
                                if (hitz >= (sector[wall[hitwall].nextsector].floorz))
                                    hitwall = wall[hitwall].nextwall;

                        if( hitwall >= 0 && wall[hitwall].picnum != ACCESSSWITCH && wall[hitwall].picnum != ACCESSSWITCH2 )
                        {
                            checkhitwall(j,hitwall,hitx,hity,hitz,atwith);
                            if(p >= 0) checkhitswitch(p,hitwall,0);
                        }
                    }
                }
                else if(p >= 0 && zvel > 0 && sector[hitsect].lotag == 1)
                {
                    j = spawn(ps[p].i,WATERSPLASH2);
                    sprite[j].x = hitx;
                    sprite[j].y = hity;
                    sprite[j].ang = ps[p].ang; // Total tweek
                    sprite[j].xvel = 32;
                    ssp(i,0);
                    sprite[j].xvel = 0;

                }
            }

            break;

        case SHOTSPARK1:
        case SHOTGUN:
        case CHAINGUN:

            if( s->extra >= 0 ) s->shade = -96;

            if(p >= 0)
            {
                j = aim( s, AUTO_AIM_ANGLE );
                if(j >= 0)
                {
                    dal = ((sprite[j].xrepeat*tilesizy[sprite[j].picnum])<<1)+(5<<8);
                    zvel = ( ( sprite[j].z-sz-dal )<<8 ) / ldist(&sprite[ps[p].i], &sprite[j]) ;
                    sa = getangle(sprite[j].x-sx,sprite[j].y-sy);
                }

                if(atwith == SHOTSPARK1)
                {
                    if(j == -1)
                    {
                        sa += 16-(TRAND&31);
                        zvel = (100-ps[p].horiz-ps[p].horizoff)<<5;
                        zvel += 128-(TRAND&255);
                    }
                }
                else
                {
                    if (atwith == SHOTGUN)
                        sa += 64-(TRAND&127);
                    else
                        sa += 16-(TRAND&31);
                    if(j == -1) zvel = (100-ps[p].horiz-ps[p].horizoff)<<5;
                    zvel += 128-(TRAND&255);
                }
                sz -= (2<<8);
            }
            else
            {
                j = findplayer(s,&x);
                sz -= (4<<8);
                zvel = ( (ps[j].posz-sz) <<8 ) / (ldist(&sprite[ps[j].i], s ) );
                if(s->picnum != BOSS1)
                {
                    zvel += 128-(TRAND&255);
                    sa += 32-(TRAND&63);
                }
                else
                {
                    zvel += 128-(TRAND&255);
                    sa = getangle(ps[j].posx-sx,ps[j].posy-sy)+64-(TRAND&127);
                }
            }

            s->cstat &= ~257;
            hitscan(sx,sy,sz,sect,
                sintable[(sa+512)&2047],
                sintable[sa&2047],
                zvel<<6,&hitsect,&hitwall,&hitspr,&hitx,&hity,&hitz,CLIPMASK1);
#ifdef RRRA
            if (((sector[hitsect].lotag == 160 && zvel > 0) || (sector[hitsect].lotag == 161 && zvel < 0))
                && hitspr == -1 && hitwall == -1)
            {
                short ii;
                for (ii = 0; ii < MAXSPRITES; ii++)
                {
                    if (sprite[ii].sectnum == hitsect && sprite[ii].picnum == SECTOREFFECTOR
                        && sprite[ii].lotag == 7)
                    {
                        int nx, ny, nz;
                        nx = hitx + (sprite[sprite[ii].owner].x - sprite[ii].x);
                        ny = hity + (sprite[sprite[ii].owner].y - sprite[ii].y);
                        if (sector[hitsect].lotag == 161)
                        {
                            nz = sector[sprite[sprite[ii].owner].sectnum].floorz;
                        }
                        else
                        {
                            nz = sector[sprite[sprite[ii].owner].sectnum].ceilingz;
                        }
                        hitscan(nx,ny,nz,sprite[sprite[ii].owner].sectnum,
                            sintable[(sa+512)&2047],
                            sintable[sa&2047],zvel<<6,
                            &hitsect,&hitwall,&hitspr,&hitx,&hity,&hitz,CLIPMASK1);
                        break;
                    }
                }
            }
#endif
            s->cstat |= 257;

            if(hitsect < 0) return;

            if (atwith == SHOTGUN)
                if (sector[hitsect].lotag == 1)
                    if (TRAND&1)
                        return;

            if( (TRAND&15) == 0 && sector[hitsect].lotag == 2 )
                tracers(hitx,hity,hitz,sx,sy,sz,8-(ud.multimode>>1));

            if(p >= 0)
            {
                k = EGS(hitsect,hitx,hity,hitz,SHOTSPARK1,-15,10,10,sa,0,0,i,4);
                sprite[k].extra = *actorscrptr[atwith];
                sprite[k].extra += (TRAND%6);

                if( hitwall == -1 && hitspr == -1)
                {
                    if( zvel < 0 )
                    {
                        if( sector[hitsect].ceilingstat&1 )
                        {
                            sprite[k].xrepeat = 0;
                            sprite[k].yrepeat = 0;
                            return;
                        }
                        else
                            checkhitceiling(hitsect);
                    }
                    if (sector[hitsect].lotag != 1)
                        spawn(k,SMALLSMOKE);
                }

                if(hitspr >= 0)
                {
                    if (sprite[hitspr].picnum == 1930)
                        return;
                    checkhitsprite(hitspr,k);
                    if( sprite[hitspr].picnum == APLAYER && (ud.coop != 1 || ud.ffire == 1) )
                    {
                        l = spawn(k,JIBS6);
                        sprite[k].xrepeat = sprite[k].yrepeat = 0;
                        sprite[l].z += (4<<8);
                        sprite[l].xvel = 16;
                        sprite[l].xrepeat = sprite[l].yrepeat = 24;
                        sprite[l].ang += 64-(TRAND&127);
                    }
                    else spawn(k,SMALLSMOKE);

                    if(p >= 0 && (
                        sprite[hitspr].picnum == DIPSWITCH ||
                        sprite[hitspr].picnum == DIPSWITCH+1 ||
                        sprite[hitspr].picnum == DIPSWITCH2 ||
                        sprite[hitspr].picnum == DIPSWITCH2+1 ||
                        sprite[hitspr].picnum == DIPSWITCH3 ||
                        sprite[hitspr].picnum == DIPSWITCH3+1 ||
#ifdef RRRA
                        sprite[hitspr].picnum == RRTILE8660 ||
#endif
                        sprite[hitspr].picnum == HANDSWITCH ||
                        sprite[hitspr].picnum == HANDSWITCH+1) )
                    {
                        checkhitswitch(p,hitspr,1);
                        return;
                    }
                }
                else if( hitwall >= 0 )
                {
                    spawn(k,SMALLSMOKE);

                    if( isadoorwall(wall[hitwall].picnum) == 1 )
                        goto SKIPBULLETHOLE;
                    if (isablockdoor(wall[hitwall].picnum) == 1)
                        goto SKIPBULLETHOLE;
                    if(p >= 0 && (
                        wall[hitwall].picnum == DIPSWITCH ||
                        wall[hitwall].picnum == DIPSWITCH+1 ||
                        wall[hitwall].picnum == DIPSWITCH2 ||
                        wall[hitwall].picnum == DIPSWITCH2+1 ||
                        wall[hitwall].picnum == DIPSWITCH3 ||
                        wall[hitwall].picnum == DIPSWITCH3+1 ||
#ifdef RRRA
                        wall[hitwall].picnum == RRTILE8660 ||
#endif
                        wall[hitwall].picnum == HANDSWITCH ||
                        wall[hitwall].picnum == HANDSWITCH+1) )
                    {
                        checkhitswitch(p,hitwall,0);
                        return;
                    }

                    if(wall[hitwall].hitag != 0 || ( wall[hitwall].nextwall >= 0 && wall[wall[hitwall].nextwall].hitag != 0 ) )
                        goto SKIPBULLETHOLE;

                    if( hitsect >= 0 && sector[hitsect].lotag == 0 )
                        if( wall[hitwall].overpicnum != BIGFORCE )
                            if( (wall[hitwall].nextsector >= 0 && sector[wall[hitwall].nextsector].lotag == 0 ) ||
                                ( wall[hitwall].nextsector == -1 && sector[hitsect].lotag == 0 ) )
                                    if( (wall[hitwall].cstat&16) == 0)
                    {
                        if(wall[hitwall].nextsector >= 0)
                        {
                            l = headspritesect[wall[hitwall].nextsector];
                            while(l >= 0)
                            {
                                if(sprite[l].statnum == 3 && sprite[l].lotag == 13)
                                    goto SKIPBULLETHOLE;
                                l = nextspritesect[l];
                            }
                        }

                        l = headspritestat[5];
                        while(l >= 0)
                        {
                            if(sprite[l].picnum == BULLETHOLE)
                                if(dist(&sprite[l],&sprite[k]) < (12+(TRAND&7)) )
                                    goto SKIPBULLETHOLE;
                            l = nextspritestat[l];
                        }
                        l = spawn(k,BULLETHOLE);
                        sprite[l].xvel = -1;
                        sprite[l].ang = getangle(wall[hitwall].x-wall[wall[hitwall].point2].x,
                            wall[hitwall].y-wall[wall[hitwall].point2].y)+512;
                        ssp(l,CLIPMASK0);
                    }

                    SKIPBULLETHOLE:

                    if( wall[hitwall].cstat&2 )
                        if(wall[hitwall].nextsector >= 0)
                            if(hitz >= (sector[wall[hitwall].nextsector].floorz) )
                                hitwall = wall[hitwall].nextwall;

                    checkhitwall(k,hitwall,hitx,hity,hitz,SHOTSPARK1);
                }
            }
            else
            {
                k = EGS(hitsect,hitx,hity,hitz,SHOTSPARK1,-15,24,24,sa,0,0,i,4);
                sprite[k].extra = *actorscrptr[atwith];

                if( hitspr >= 0 )
                {
                    checkhitsprite(hitspr,k);
                    if( sprite[hitspr].picnum != APLAYER )
                        spawn(k,SMALLSMOKE);
                    else sprite[k].xrepeat = sprite[k].yrepeat = 0;
                }
                else if( hitwall >= 0 )
                    checkhitwall(k,hitwall,hitx,hity,hitz,SHOTSPARK1);
            }

            if( (TRAND&255) < 10 )
                xyzsound(PISTOL_RICOCHET,k,hitx,hity,hitz);

            return;

        case TRIPBOMBSPRITE:
            j = spawn(i,atwith);
            sprite[j].xvel = 32;
            sprite[j].ang = sprite[i].ang;
            sprite[j].z -= (5<<8);
            break;

        case BOWLINGBALL:
            j = spawn(i,atwith);
            sprite[j].xvel = 250;
            sprite[j].ang = sprite[i].ang;
            sprite[j].z -= (15<<8);
            break;

        case OWHIP:
        case UWHIP:

            if( s->extra >= 0 ) s->shade = -96;

            scount = 1;
            if (atwith == 3471)
            {
                vel = 300;
                sz -= (15<<8);
                scount = 1;
            }
            else if (atwith == 3475)
            {
                vel = 300;
                sz += (4<<8);
                scount = 1;
            }

            if(p >= 0)
            {
                j = aim( s, AUTO_AIM_ANGLE );

                if(j >= 0)
                {
                    dal = ((sprite[j].xrepeat*tilesizy[sprite[j].picnum])<<1)-(12<<8);
                    zvel = ((sprite[j].z-sz-dal)*vel ) / ldist(&sprite[ps[p].i], &sprite[j]) ;
                    sa = getangle(sprite[j].x-sx,sprite[j].y-sy);
                }
                else
                    zvel = (100-ps[p].horiz-ps[p].horizoff)*98;
            }
            else
            {
                j = findplayer(s,&x);
//                sa = getangle(ps[j].oposx-sx,ps[j].oposy-sy);
                if (s->picnum == VIXEN)
                    sa -= (TRAND&16);
                else
                    sa += 16-(TRAND&31);
                zvel = ( ( (ps[j].oposz - sz + (3<<8) ) )*vel ) / ldist(&sprite[ps[j].i],s);
            }

            oldzvel = zvel;
            sizx = 18; sizy = 18;

            if(p >= 0) sizx = 7,sizy = 7;
            else sizx = 8,sizy = 8;

            while(scount > 0)
            {
                j = EGS(sect,sx,sy,sz,atwith,-127,sizx,sizy,sa,vel,zvel,i,4);
                sprite[j].extra += (TRAND&7);

                sprite[j].cstat = 128;
                sprite[j].clipdist = 4;

                sa = s->ang+32-(TRAND&63);
                zvel = oldzvel+512-(TRAND&1023);

                scount--;
            }

            return;
            
        case FIRELASER:
        case SPIT:
        case COOLEXPLOSION1:

#ifdef RRRA
            if( atwith != SPIT && s->extra >= 0 ) s->shade = -96;

            scount = 1;
            if(atwith == SPIT)
            {
                if (s->picnum == 8705)
                    vel = 600;
                else
                    vel = 400;
            }
#else
            if( s->extra >= 0 ) s->shade = -96;

            scount = 1;
            if(atwith == SPIT) vel = 400;
#endif
            else
            {
                vel = 840;
                sz -= (4<<7);
                if (s->picnum == 4649)
                {
                    sx += sintable[(s->ang+512+256)&2047]>>6;
                    sy += sintable[(s->ang+256)&2047]>>6;
                    sz += (12<<8);
                }
                if (s->picnum == VIXEN)
                {
                    sz -= (12<<8);
                }
            }

            if(p >= 0)
            {
                j = aim( s, AUTO_AIM_ANGLE );

                if(j >= 0)
                {
                    sx += sintable[(s->ang+512+160)&2047]>>7;
                    sy += sintable[(s->ang+160)&2047]>>7;
                    dal = ((sprite[j].xrepeat*tilesizy[sprite[j].picnum])<<1)-(12<<8);
                    zvel = ((sprite[j].z-sz-dal)*vel ) / ldist(&sprite[ps[p].i], &sprite[j]) ;
                    sa = getangle(sprite[j].x-sx,sprite[j].y-sy);
                }
                else
                {
                    sx += sintable[(s->ang+512+160)&2047]>>7;
                    sy += sintable[(s->ang+160)&2047]>>7;
                    zvel = (100-ps[p].horiz-ps[p].horizoff)*98;
                }
            }
            else
            {
                j = findplayer(s,&x);
//                sa = getangle(ps[j].oposx-sx,ps[j].oposy-sy);
                if (s->picnum == HULK)
                    sa -= (TRAND&31);
                else if (s->picnum == VIXEN)
                    sa -= (TRAND&16);
                else if (s->picnum != UFOBEAM)
#ifdef RRRA
                {
                    if (s->picnum == MAMA)
                        sa += 16-(TRAND&31);
                    else
                        sa += 16-(TRAND&31);
                }
#else
                    sa += 16-(TRAND&31);
#endif
                zvel = ( ( (ps[j].oposz - sz + (3<<8) ) )*vel ) / ldist(&sprite[ps[j].i],s);
            }

            oldzvel = zvel;

#ifdef RRRA
            if(atwith == SPIT) { sizx = 18;sizy = 18;
                if (s->picnum != MAMA) sz -= (10<<8); else sz -= (20<<8); }
#else
            if(atwith == SPIT) { sizx = 18;sizy = 18,sz -= (10<<8); }
#endif
            else
            {
                if( atwith == COOLEXPLOSION1 )
                {
                    sizx = 8;
                    sizy = 8;
                }
                else if( atwith == FIRELASER )
                {
                    if(p >= 0)
                    {
                        
                        sizx = 34;
                        sizy = 34;
                    }
                    else
                    {
                        sizx = 18;
                        sizy = 18;
                    }
                }
                else
                {
                    sizx = 18;
                    sizy = 18;
                }
            }

            if(p >= 0) sizx = 7,sizy = 7;

            while(scount > 0)
            {
                j = EGS(sect,sx,sy,sz,atwith,-127,sizx,sizy,sa,vel,zvel,i,4);
                sprite[j].extra += (TRAND&7);
                sprite[j].cstat = 128;
                sprite[j].clipdist = 4;

                sa = s->ang+32-(TRAND&63);
                zvel = oldzvel+512-(TRAND&1023);

                if (atwith == FIRELASER)
                {
                    sprite[j].xrepeat = 8;
                    sprite[j].yrepeat = 8;
                }

                scount--;
            }

            return;

        case FREEZEBLAST:
            sz += (3<<8);
        case RPG:
        case SHRINKSPARK:
#ifdef RRRA
        case RPG2:
        case RRTILE1790:
            {
            short var90 = 0;
#endif
            if( s->extra >= 0 ) s->shade = -96;

            scount = 1;
            vel = 644;

            j = -1;

            if(p >= 0)
            {
                j = aim( s, 48 );
                if(j >= 0)
                {
#ifdef RRRA
                    if (atwith == RPG2)
                    {
                        if (sprite[j].picnum == HEN || sprite[j].picnum == HENSTAYPUT)
                            var90 = ps[screenpeek].i;
                        else
                            var90 = j;
                    }
#endif
                    dal = ((sprite[j].xrepeat*tilesizy[sprite[j].picnum])<<1)+(8<<8);
                    zvel = ( (sprite[j].z-sz-dal)*vel ) / ldist(&sprite[ps[p].i], &sprite[j]);
                    if( sprite[j].picnum != RECON )
                        sa = getangle(sprite[j].x-sx,sprite[j].y-sy);
                }
                else zvel = (100-ps[p].horiz-ps[p].horizoff)*81;
                if(atwith == RPG)
                    spritesound(RPG_SHOOT,i);
#ifdef RRRA
                else if (atwith == RPG2)
                    spritesound(244,i);
                else if (atwith == RRTILE1790)
                    spritesound(94,i);
#endif

            }
            else
            {
                j = findplayer(s,&x);
                sa = getangle(ps[j].oposx-sx,ps[j].oposy-sy);
                if(PN == BOSS3)
                    sz -= (32<<8);
                else if(PN == BOSS2)
                {
                    vel += 128;
                    sz += 24<<8;
                }

                l = ldist(&sprite[ps[j].i],s);
                zvel = ( (ps[j].oposz-sz)*vel) / l;

                if( badguy(s) && (s->hitag&face_player_smart) )
                    sa = s->ang+(TRAND&31)-16;
            }

            if( p >= 0 && j >= 0)
               l = j;
            else l = -1;

#ifdef RRRA
            if (atwith == RRTILE1790)
            {
                zvel = -(10<<8);
                vel <<= 1;
            }
#endif

            j = EGS(sect,
                sx+(sintable[(348+sa+512)&2047]/448),
                sy+(sintable[(sa+348)&2047]/448),
                sz-(1<<8),atwith,0,14,14,sa,vel,zvel,i,4);

#ifdef RRRA
            if (atwith == RRTILE1790)
            {
                sprite[j].extra = 10;
                sprite[j].zvel = -(10<<8);
            }
            else if (atwith == RPG2)
            {
                sprite[j].lotag = var90;
                sprite[j].hitag = 0;
                lotsofmoney(&sprite[j],(TRAND&3)+1);
            }
#endif

            sprite[j].extra += (TRAND&7);
            if(atwith != FREEZEBLAST)
                sprite[j].yvel = l;
            else
            {
                sprite[j].yvel = numfreezebounces;
                sprite[j].xrepeat >>= 1;
                sprite[j].yrepeat >>= 1;
                sprite[j].zvel -= (2<<4);
            }

            if(p == -1)
            {
                if(PN == HULK)
                {
                    sprite[j].xrepeat = 8;
                    sprite[j].yrepeat = 8;
                }
                else if(atwith != FREEZEBLAST)
                {
                    sprite[j].xrepeat = 30;
                    sprite[j].yrepeat = 30;
                    sprite[j].extra >>= 2;
                }
            }
            else if(ps[p].curr_weapon == DEVISTATOR_WEAPON)
            {
                sprite[j].extra >>= 2;
                sprite[j].ang += 16-(TRAND&31);
                sprite[j].zvel += 256-(TRAND&511);

                if( ps[p].hbomb_hold_delay )
                {
                    sprite[j].x -= sintable[sa&2047]/644;
                    sprite[j].y -= sintable[(sa+1024+512)&2047]/644;
                }
                else
                {
                    sprite[j].x += sintable[sa&2047]>>8;
                    sprite[j].y += sintable[(sa+1024+512)&2047]>>8;
                }
                sprite[j].xrepeat >>= 1;
                sprite[j].yrepeat >>= 1;
            }

            sprite[j].cstat = 128;
            if(atwith == RPG)
                sprite[j].clipdist = 4;
#ifdef RRRA
            else if(atwith == RPG2)
                sprite[j].clipdist = 4;
            else if(atwith == RPG)
                sprite[j].clipdist = 4;
#endif
            else
                sprite[j].clipdist = 40;

#ifdef RRRA
            }
#endif
            break;

        /*case HANDHOLDINGLASER:

            if(p >= 0)
                zvel = (100-ps[p].horiz-ps[p].horizoff)*32;
            else zvel = 0;

            hitscan(sx,sy,sz-ps[p].pyoff,sect,
                sintable[(sa+512)&2047],
                sintable[sa&2047],
                zvel<<6,&hitsect,&hitwall,&hitspr,&hitx,&hity,&hitz,CLIPMASK1);

            j = 0;
            if(hitspr >= 0) break;

            if(hitwall >= 0 && hitsect >= 0)
                if( ((hitx-sx)*(hitx-sx)+(hity-sy)*(hity-sy)) < (290*290) )
            {
                if( wall[hitwall].nextsector >= 0)
                {
                    if( sector[wall[hitwall].nextsector].lotag <= 2 && sector[hitsect].lotag <= 2 )
                        j = 1;
                }
                else if( sector[hitsect].lotag <= 2 )
                    j = 1;
            }

            if(j == 1)
            {
                k = EGS(hitsect,hitx,hity,hitz,TRIPBOMB,-16,4,5,sa,0,0,i,6);

                sprite[k].hitag = k;
                spritesound(LASERTRIP_ONWALL,k);
                sprite[k].xvel = -20;
                ssp(k,CLIPMASK0);
                sprite[k].cstat = 16;
                hittype[k].temp_data[5] = sprite[k].ang = getangle(wall[hitwall].x-wall[wall[hitwall].point2].x,wall[hitwall].y-wall[wall[hitwall].point2].y)-512;

                if(p >= 0)
                    ps[p].ammo_amount[TRIPBOMB_WEAPON]--;

            }
            return;*/

        //case BOUNCEMINE:
        case MORTER:
#ifdef RRRA
        case CHEERBOMB:
#endif

            if( s->extra >= 0 ) s->shade = -96;

            j = ps[findplayer(s,&x)].i;
            x = ldist(&sprite[j],s);

            zvel = -x>>1;

            if(zvel < -4096)
                zvel = -2048;
            vel = x>>4;

#ifdef RRRA
            if (atwith == CHEERBOMB)
                EGS(sect,
                    sx+(sintable[(512+sa+512)&2047]>>8),
                    sy+(sintable[(sa+512)&2047]>>8),
                    sz+(6<<8),atwith,-64,16,16,sa,vel,zvel,i,1);
            else
#endif

            EGS(sect,
                sx+(sintable[(512+sa+512)&2047]>>8),
                sy+(sintable[(sa+512)&2047]>>8),
                sz+(6<<8),atwith,-64,32,32,sa,vel,zvel,i,1);
            break;
    }
    return;
}
