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

void moveexplosions(void)  // STATNUM 5
{
    short i, j, k, nexti, sect, p;
    long l, x, *t;
    spritetype *s;

    i = headspritestat[5];
    while(i >= 0)
    {
        nexti = nextspritestat[i];

        t = &hittype[i].temp_data[0];
        s = &sprite[i];
        sect = s->sectnum;

        if( sect < 0 || s->xrepeat == 0 ) KILLIT(i);

        hittype[i].bposx = s->x;
        hittype[i].bposy = s->y;
        hittype[i].bposz = s->z;

        switch(s->picnum)
        {
            case SHOTGUNSPRITE:
                if (sector[s->sectnum].lotag == 800)
                    if (s->z >= sector[s->sectnum].floorz - (8<<8))
                {
                    KILLIT(i);
                }
                break;
            case NEON1:
            case NEON2:
            case NEON3:
            case NEON4:
            case NEON5:
            case NEON6:

                if( (global_random/(s->lotag+1)&31) > 4) s->shade = -127;
                else s->shade = 127;
                goto BOLT;

            case BLOODSPLAT1:
            case BLOODSPLAT2:
            case BLOODSPLAT3:
            case BLOODSPLAT4:

                if( t[0] == 7*26 ) goto BOLT;
                s->z += 16+(TRAND&15);
                t[0]++;
                if( (t[0]%9) == 0 ) s->yrepeat++;
                goto BOLT;

            /*case NUKEBUTTON:
            case NUKEBUTTON+1:
            case NUKEBUTTON+2:
            case NUKEBUTTON+3:

                if(t[0])
                {
                    t[0]++;
                    if(t[0] == 8) s->picnum = NUKEBUTTON+1;
                    else if(t[0] == 16)
                    {
                        s->picnum = NUKEBUTTON+2;
                        ps[sprite[s->owner].yvel].fist_incs = 1;
                    }
                    if( ps[sprite[s->owner].yvel].fist_incs == 26 )
                        s->picnum = NUKEBUTTON+3;
                }
                goto BOLT;*/

            case FORCESPHERE:

                l = s->xrepeat;
                if(t[1] > 0)
                {
                    t[1]--;
                    if(t[1] == 0)
                    {
                        KILLIT(i);
                    }
                }
                if(hittype[s->owner].temp_data[1] == 0)
                {
                    if(t[0] < 64)
                    {
                        t[0]++;
                        l += 3;
                    }
                }
                else
                    if(t[0] > 64)
                    {
                        t[0]--;
                        l -= 3;
                    }

                s->x = sprite[s->owner].x;
                s->y = sprite[s->owner].y;
                s->z = sprite[s->owner].z;
                s->ang += hittype[s->owner].temp_data[0];

                if(l > 64) l = 64;
                else if(l < 1) l = 1;

                s->xrepeat = l;
                s->yrepeat = l;
                s->shade = (l>>1)-48;

                for(j=t[0];j > 0;j--)
                    ssp(i,CLIPMASK0);
                goto BOLT;

            case MUD:

                t[0]++;
                if(t[0] == 1 )
                {
                    if(sector[sect].floorpicnum != 3073)
                        KILLIT(i);
                    if(Sound[22].num == 0)
                        spritesound(22,i);
                }
                if(t[0] == 3)
                {
                    t[0] = 0;
                    t[1]++;
                }
                if(t[1] == 5)
                    deletesprite(i);
                goto BOLT;

            case WATERSPLASH2:

                t[0]++;
                if(t[0] == 1 )
                {
                    if(sector[sect].lotag != 1 && sector[sect].lotag != 2)
                        KILLIT(i);
/*                    else
                    {
                        l = getflorzofslope(sect,s->x,s->y)-s->z;
                        if( l > (16<<8) ) KILLIT(i);
                    }
                    else */ if(Sound[ITEM_SPLASH].num == 0)
                        spritesound(ITEM_SPLASH,i);
                }
                if(t[0] == 3)
                {
                    t[0] = 0;
                    t[1]++;
                }
                if(t[1] == 5)
                    deletesprite(i);
                goto BOLT;

            case FRAMEEFFECT1:

                if(s->owner >= 0)
                {
                    t[0]++;

                    if( t[0] > 7 )
                    {
                        KILLIT(i);
                    }
                    else if( t[0] > 4 )
                        s->cstat |= 512+2;
                    else if( t[0] > 2 )
                        s->cstat |= 2;
                    s->xoffset = sprite[s->owner].xoffset;
                    s->yoffset = sprite[s->owner].yoffset;
                }
                goto BOLT;
            case INNERJAW:
            case INNERJAW+1:

                p = findplayer(s,&x);
                if(x < 512)
                {
                    ps[p].pals_time = 32;
                    ps[p].pals[0] = 32;
                    ps[p].pals[1] = 0;
                    ps[p].pals[2] = 0;
                    sprite[ps[p].i].extra -= 4;
                }

            case COOLEXPLOSION1:
            case FIRELASER:
            case OWHIP:
            case UWHIP:
                if(s->extra != 999)
                    s->extra = 999;
                else KILLIT(i);
                break;
            case TONGUE:
                KILLIT(i);
            case MONEY+1:
                hittype[i].floorz = s->z = getflorzofslope(s->sectnum,s->x,s->y);
                if (sector[s->sectnum].lotag == 800)
                    KILLIT(i);
                break;
            case MONEY:

                s->xvel = (TRAND&7)+(sintable[T1&2047]>>9);
                T1 += (TRAND&63);
                if( (T1&2047) > 512 && (T1&2047) < 1596)
                {
                    if(sector[sect].lotag == 2)
                    {
                        if(s->zvel < 64)
                            s->zvel += (gc>>5)+(TRAND&7);
                    }
                    else
                        if(s->zvel < 144)
                            s->zvel += (gc>>5)+(TRAND&7);
                }

                ssp(i,CLIPMASK0);

                if( (TRAND&3) == 0 )
                    setsprite(i,s->x,s->y,s->z);

                if(s->sectnum == -1) KILLIT(i);
                l = getflorzofslope(s->sectnum,s->x,s->y);

                if( s->z > l )
                {
                    s->z = l;

                    insertspriteq(i);
                    PN ++;

                    j = headspritestat[5];
                    while(j >= 0)
                    {
                        if(sprite[j].picnum == BLOODPOOL)
                            if(ldist(s,&sprite[j]) < 348)
                        {
                            s->pal = 2;
                            break;
                        }
                        j = nextspritestat[j];
                    }
                }

                if (sector[s->sectnum].lotag == 800)
                    if (s->z >= sector[s->sectnum].floorz - (8<<8))
                {
                    KILLIT(i);
                }

                break;

            case BILLYJIBA:
            case BILLYJIBB:
            case HULKJIBA:
            case HULKJIBB:
            case HULKJIBC:
            case MINJIBA:
            case MINJIBB:
            case MINJIBC:
            case COOTJIBA:
            case COOTJIBB:
            case COOTJIBC:
            case JIBS1:
            case JIBS2:
            case JIBS3:
            case JIBS4:
            case JIBS5:
            case JIBS6:
            case DUKETORSO:
            case DUKEGUN:
            case DUKELEG:
#ifdef RRRA
            case RRTILE2460:
            case RRTILE2465:
            case BIKEJIBA:
            case BIKEJIBB:
            case BIKEJIBC:
            case BIKERJIBA:
            case BIKERJIBB:
            case BIKERJIBC:
            case BIKERJIBD:
            case CHEERJIBA:
            case CHEERJIBB:
            case CHEERJIBC:
            case CHEERJIBD:
            case FBOATJIBA:
            case FBOATJIBB:
            case RABBITJIBA:
            case RABBITJIBB:
            case RABBITJIBC:
            case MAMAJIBA:
            case MAMAJIBB:
#endif

                if(s->xvel > 0) s->xvel--;
                else s->xvel = 0;


                if(s->zvel > 1024 && s->zvel < 1280)
                {
                    setsprite(i,s->x,s->y,s->z);
                    sect = s->sectnum;
                }

                setsprite(i,s->x,s->y,s->z);

                l = getflorzofslope(sect,s->x,s->y);
                x = getceilzofslope(sect,s->x,s->y);
                if(x == l || sect < 0 || sect >= MAXSECTORS) KILLIT(i);

                if( s->z < l-(2<<8) )
                {
                    if(t[1] < 2) t[1]++;
                    else if(sector[sect].lotag != 2)
                    {
                        t[1] = 0;
                        if( s->picnum == DUKELEG || s->picnum == DUKETORSO || s->picnum == DUKEGUN )
                        {
                            if(t[0] > 6) t[0] = 0;
                            else t[0]++;
                        }
                        else
                        {
                            if(t[0] > 2)
                                t[0] = 0;
                            else t[0]++;
                        }
                    }

                    if(s->zvel < 6144)
                    {
                        if(sector[sect].lotag == 2)
                        {
                            if(s->zvel < 1024)
                                s->zvel += 48;
                            else s->zvel = 1024;
                        }
                        else s->zvel += gc-50;
                    }

                    s->x += (s->xvel*sintable[(s->ang+512)&2047])>>14;
                    s->y += (s->xvel*sintable[s->ang&2047])>>14;
                    s->z += s->zvel;

                    if (s->z >= sector[s->sectnum].floorz)
                        KILLIT(i);
                }
                else
                {
#ifdef RRRA
                    if (s->picnum == RRTILE2465 || s->picnum == RRTILE2560)
                        KILLIT(i);
#endif
                    if(t[2] == 0)
                    {
                        if( s->sectnum == -1) { KILLIT(i); }
                        if( (sector[s->sectnum].floorstat&2) ) { KILLIT(i); }
                        t[2]++;
                    }
                    l = getflorzofslope(s->sectnum,s->x,s->y);

                    s->z = l-(2<<8);
                    s->xvel = 0;

                    if(s->picnum == JIBS6)
                    {
                        t[1]++;
                        if( (t[1]&3) == 0 && t[0] < 7)
                            t[0]++;
                        if(t[1] > 20) KILLIT(i);
                    }
                    else { s->picnum = JIBS6; t[0] = 0; t[1] = 0; }
                }

                if (sector[s->sectnum].lotag == 800)
                    if (s->z >= sector[s->sectnum].floorz - (8<<8))
                    KILLIT(i);

                goto BOLT;

            case BLOODPOOL:

                if(t[0] == 0)
                {
                    t[0] = 1;
                    if(sector[sect].floorstat&2) { KILLIT(i); }
                    else insertspriteq(i);
                }

                makeitfall(i);

                p = findplayer(s,&x);

                s->z = hittype[i].floorz-(FOURSLEIGHT);

                if(t[2] < 32)
                {
                    t[2]++;
                    if(hittype[i].picnum == TIRE)
                    {
                        if(s->xrepeat < 64 && s->yrepeat < 64)
                        {
                            s->xrepeat += TRAND&3;
                            s->yrepeat += TRAND&3;
                        }
                    }
                    else
                    {
                        if(s->xrepeat < 32 && s->yrepeat < 32)
                        {
                            s->xrepeat += TRAND&3;
                            s->yrepeat += TRAND&3;
                        }
                    }
                }

                if(x < 844 && s->xrepeat > 6 && s->yrepeat > 6)
                {
                    if( s->pal == 0 && (TRAND&255) < 16)
                    {
                        if(ps[p].boot_amount > 0)
                            ps[p].boot_amount--;
                        else
                        {
                            if(Sound[DUKE_LONGTERM_PAIN].num < 1)
                                spritesound(DUKE_LONGTERM_PAIN,ps[p].i);
                            sprite[ps[p].i].extra --;
                            ps[p].pals_time = 32;
                            ps[p].pals[0] = 16;
                            ps[p].pals[1] = 0;
                            ps[p].pals[2] = 0;
                        }
                    }

                    if(t[1] == 1) goto BOLT;
                    t[1] = 1;

                    if(hittype[i].picnum == TIRE)
                        ps[p].footprintcount = 10;
                    else ps[p].footprintcount = 3;

                    ps[p].footprintpal = s->pal;
                    ps[p].footprintshade = s->shade;

                    if(t[2] == 32)
                    {
                        s->xrepeat -= 6;
                        s->yrepeat -= 6;
                    }
                }
                else t[1] = 0;

                if (sector[s->sectnum].lotag == 800)
                    if (s->z >= sector[s->sectnum].floorz - (8<<8))
                    KILLIT(i);
                goto BOLT;

            case BURNING:
            case WATERBUBBLE:
            case SMALLSMOKE:
            case EXPLOSION2:
            case EXPLOSION3:
            case BLOOD:
            case FORCERIPPLE:
            case TRANSPORTERSTAR:
            case TRANSPORTERBEAM:
                p = findplayer(s,&x);
                execute(i,p,x);
                goto BOLT;

            case SHELL:
            case SHOTGUNSHELL:

                ssp(i,CLIPMASK0);

                if(sect < 0) KILLIT(i);

                if(sector[sect].lotag == 2)
                {
                    t[1]++;
                    if(t[1] > 8)
                    {
                        t[1] = 0;
                        t[0]++;
                        t[0] &= 3;
                    }
                    if(s->zvel < 128) s->zvel += (gc/13); // 8
                    else s->zvel -= 64;
                    if(s->xvel > 0)
                        s->xvel -= 4;
                    else s->xvel = 0;
                }
                else
                {
                    t[1]++;
                    if(t[1] > 3)
                    {
                        t[1] = 0;
                        t[0]++;
                        t[0] &= 3;
                    }
                    if(s->zvel < 512) s->zvel += (gc/3); // 52;
                    if(s->xvel > 0)
                        s->xvel --;
                    else KILLIT(i);
                }

                goto BOLT;

            case GLASSPIECES:
            case GLASSPIECES+1:
            case GLASSPIECES+2:
            case POPCORN:

                makeitfall(i);

                if(s->zvel > 4096) s->zvel = 4096;
                if(sect < 0) KILLIT(i);

                if( s->z == hittype[i].floorz-(FOURSLEIGHT) && t[0] < 3)
                {
                    s->zvel = -((3-t[0])<<8)-(TRAND&511);
                    if(sector[sect].lotag == 2)
                        s->zvel >>= 1;
                    s->xrepeat >>= 1;
                    s->yrepeat >>= 1;
                    if( rnd(96) )
                      setsprite(i,s->x,s->y,s->z);
                    t[0]++;//Number of bounces
                }
                else if( t[0] == 3 ) KILLIT(i);

                if(s->xvel > 0)
                {
                    s->xvel -= 2;
                    s->cstat = ((s->xvel&3)<<2);
                }
                else s->xvel = 0;

                ssp(i,CLIPMASK0);

                goto BOLT;
        }

        IFWITHIN(SCRAP6,SCRAP5+3)
        {
                if(s->xvel > 0)
                    s->xvel--;
                else s->xvel = 0;

                if(s->zvel > 1024 && s->zvel < 1280)
                {
                    setsprite(i,s->x,s->y,s->z);
                    sect = s->sectnum;
                }

                if( s->z < sector[sect].floorz-(2<<8) )
                {
                    if(t[1] < 1) t[1]++;
                    else
                    {
                        t[1] = 0;

                        if(s->picnum < SCRAP6+8)
                        {
                            if(t[0] > 6)
                                t[0] = 0;
                            else t[0]++;
                        }
                        else
                        {
                            if(t[0] > 2)
                                t[0] = 0;
                            else t[0]++;
                        }
                    }
                    if(s->zvel < 4096) s->zvel += gc-50;
                    s->x += (s->xvel*sintable[(s->ang+512)&2047])>>14;
                    s->y += (s->xvel*sintable[s->ang&2047])>>14;
                    s->z += s->zvel;
                }
                else
                {
                    if(s->picnum == SCRAP1 && s->yvel > 0)
                    {
                        j = spawn(i,s->yvel);
                        setsprite(j,s->x,s->y,s->z);
                        getglobalz(j);
                        sprite[j].hitag = sprite[j].lotag = 0;
                    }
                    KILLIT(i);
                }
                goto BOLT;
        }

        BOLT:
        i = nexti;
    }
}
