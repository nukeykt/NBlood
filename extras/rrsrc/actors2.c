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

extern long numenvsnds;

short ifhitbyweapon(short sn)
{
    short j, k, p;
    spritetype *npc;

    if( hittype[sn].extra >= 0 )
    {
        if(sprite[sn].extra >= 0 )
        {
            npc = &sprite[sn];

            if(npc->picnum == APLAYER)
            {
                if(ud.god) return -1;

                p = npc->yvel;
                j = hittype[sn].owner;

                if( j >= 0 &&
                    sprite[j].picnum == APLAYER &&
                    ud.coop == 1 &&
                    ud.ffire == 0 )
                        return -1;

                npc->extra -= hittype[sn].extra;

                if(j >= 0)
                {
                    if(npc->extra <= 0 && hittype[sn].picnum != FREEZEBLAST)
                    {
                        npc->extra = 0;

                        ps[p].wackedbyactor = j;

                        if( sprite[hittype[sn].owner].picnum == APLAYER && p != sprite[hittype[sn].owner].yvel )
                            ps[p].frag_ps = sprite[j].yvel;

                        hittype[sn].owner = ps[p].i;
                    }
                }

                switch(hittype[sn].picnum)
                {
                    case RADIUSEXPLOSION:
                    case RPG:
                    case HYDRENT:
                    case HEAVYHBOMB:
                    case SEENINE:
                    case OOZFILTER:
                    case EXPLODINGBARREL:
                    case TRIPBOMBSPRITE:
#ifdef RRRA
                    case RPG2:
#endif
                        ps[p].posxv +=
                            hittype[sn].extra*(sintable[(hittype[sn].ang+512)&2047])<<2;
                        ps[p].posyv +=
                            hittype[sn].extra*(sintable[hittype[sn].ang&2047])<<2;
                        break;
                    default:
                        ps[p].posxv +=
                            hittype[sn].extra*(sintable[(hittype[sn].ang+512)&2047])<<1;
                        ps[p].posyv +=
                            hittype[sn].extra*(sintable[hittype[sn].ang&2047])<<1;
                        break;
                }
            }
            else
            {
                if(hittype[sn].extra == 0 )
                    if( npc->xrepeat < 24 )
                        return -1;

                npc->extra -= hittype[sn].extra;
                if(npc->picnum != RECON && npc->owner >= 0 && sprite[npc->owner].statnum < MAXSTATUS )
                    npc->owner = hittype[sn].owner;
            }

            hittype[sn].extra = -1;
            return hittype[sn].picnum;
        }
    }

    hittype[sn].extra = -1;
    return -1;
}

void movecyclers(void)
{
    short q, j, x, t, s, *c;
    walltype *wal;
    char cshade;

    for(q=numcyclers-1;q>=0;q--)
    {

        c = &cyclers[q][0];
        s = c[0];
        
        t = c[3];
        j = t+(sintable[c[1]&2047]>>10);
        cshade = c[2];

        if( j < cshade ) j = cshade;
        else if( j > t )  j = t;

        c[1] += sector[s].extra;
        if(c[5])
        {
            wal = &wall[sector[s].wallptr];
            for(x = sector[s].wallnum;x>0;x--,wal++)
                if( wal->hitag != 1 )
            {
                wal->shade = j;

                if( (wal->cstat&2) && wal->nextwall >= 0)
                    wall[wal->nextwall].shade = j;

            }
            sector[s].floorshade = sector[s].ceilingshade = j;
        }
    }
}

void movedummyplayers(void)
{
    short i, p, nexti;

    i = headspritestat[13];
    while(i >= 0)
    {
        nexti = nextspritestat[i];

        p = sprite[OW].yvel;

        if( sector[ps[p].cursectnum].lotag != 1 || sprite[ps[p].i].extra <= 0 )
        {
            ps[p].dummyplayersprite = -1;
            KILLIT(i);
        }
        else
        {
            if(ps[p].on_ground && ps[p].on_warping_sector == 1 && sector[ps[p].cursectnum].lotag == 1 )
            {
                CS = 257;
                SZ = sector[SECT].ceilingz+(27<<8);
                SA = ps[p].ang;
                if(T1 == 8)
                    T1 = 0;
                else T1++;
            }
            else
            {
                if(sector[SECT].lotag != 2) SZ = sector[SECT].floorz;
                CS = (short) 32768;
            }
        }

        SX += (ps[p].posx-ps[p].oposx);
        SY += (ps[p].posy-ps[p].oposy);
        setsprite(i,SX,SY,SZ);

        BOLT:

        i = nexti;
    }
}


short otherp;
void moveplayers(void) //Players
{
    short i , nexti;
    long otherx;
    spritetype *s;
    struct player_struct *p;

    i = headspritestat[10];
    while(i >= 0)
    {
        nexti = nextspritestat[i];

        s = &sprite[i];
        p = &ps[s->yvel];
        if(s->owner >= 0)
        {
            if(p->newowner >= 0 ) //Looking thru the camera
            {
                s->x = p->oposx;
                s->y = p->oposy;
                hittype[i].bposz = s->z = p->oposz+PHEIGHT;
                s->ang = p->oang;
                setsprite(i,s->x,s->y,s->z);
            }
            else
            {
                if(ud.multimode > 1)
                    otherp = findotherplayer(s->yvel,&otherx);
                else
                {
                    otherp = s->yvel;
                    otherx = 0;
                }

                execute(i,s->yvel,otherx);

                if(ud.multimode > 1)
                    if( sprite[ps[otherp].i].extra > 0 )
                {
                    if( s->yrepeat > 32 && sprite[ps[otherp].i].yrepeat < 32)
                    {
                        if( otherx < 1400 && p->knee_incs == 0 )
                        {
                            p->knee_incs = 1;
                            p->weapon_pos = -1;
                            p->actorsqu = ps[otherp].i;
                        }
                    }
                }
                if(ud.god)
                {
                    s->extra = max_player_health;
                    s->cstat = 257;
                }


                if( s->extra > 0 )
                {
                    hittype[i].owner = i;

                    if(ud.god == 0)
                        if( ceilingspace(s->sectnum) || floorspace(s->sectnum) )
                            quickkill(p);
                }
                else
                {
                    
                    p->posx = s->x;
                    p->posy = s->y;
                    p->posz = s->z-(20<<8);

                    p->newowner = -1;

                    if( p->wackedbyactor >= 0 && sprite[p->wackedbyactor].statnum < MAXSTATUS )
                    {
                        p->ang += getincangle(p->ang,getangle(sprite[p->wackedbyactor].x-p->posx,sprite[p->wackedbyactor].y-p->posy))>>1;
                        p->ang &= 2047;
                    }

                }
                s->ang = p->ang;
            }
        }
        else
        {
            if(p->holoduke_on == -1)
                KILLIT(i);

            hittype[i].bposx = s->x;
            hittype[i].bposy = s->y;
            hittype[i].bposz = s->z;

            s->cstat = 0;

            if(s->xrepeat < 42)
            {
                s->xrepeat += 4;
                s->cstat |= 2;
            }
            else s->xrepeat = 42;
            if(s->yrepeat < 36)
                s->yrepeat += 4;
            else
            {
                s->yrepeat = 36;
                if(sector[s->sectnum].lotag != 2)
                    makeitfall(i);
                if(s->zvel == 0 && sector[s->sectnum].lotag == 1)
                    s->z += (32<<8);
            }

            if(s->extra < 8)
            {
                s->xvel = 128;
                s->ang = p->ang;
                s->extra++;
                IFMOVING;
            }
            else
            {
                s->ang = 2047-p->ang;
                setsprite(i,s->x,s->y,s->z);
            }
        }

        if (sector[s->sectnum].ceilingstat&1)
            s->shade += (sector[s->sectnum].ceilingshade-s->shade)>>1;
        else
            s->shade += (sector[s->sectnum].floorshade-s->shade)>>1;

        BOLT:
        i = nexti;
    }
}


void movefx(void)
{
    short i, j, nexti, p;
    long x, ht;
    spritetype *s;

    i = headspritestat[11];
    while(i >= 0)
    {
        s = &sprite[i];

        nexti = nextspritestat[i];

        switch(s->picnum)
        {
            case RESPAWN:
                if(sprite[i].extra == 66)
                {
                    j = spawn(i,SHT);
#ifdef RRRA
                    sprite[j].pal = sprite[i].pal;
                    if (sprite[j].picnum == MAMA)
                    {
                        if (sprite[j].pal == 30)
                        {
                            sprite[j].xrepeat = 26;
                            sprite[j].yrepeat = 26;
                            sprite[j].clipdist = 75;
                        }
                        else if (sprite[j].pal == 31)
                        {
                            sprite[j].xrepeat = 36;
                            sprite[j].yrepeat = 36;
                            sprite[j].clipdist = 100;
                        }
                        else if (sprite[j].pal == 32)
                        {
                            sprite[j].xrepeat = 50;
                            sprite[j].yrepeat = 50;
                            sprite[j].clipdist = 100;
                        }
                        else
                        {
                            sprite[j].xrepeat = 50;
                            sprite[j].yrepeat = 50;
                            sprite[j].clipdist = 100;
                        }
                    }

                    if (sprite[j].pal == 8)
                    {
                        sprite[j].cstat |= 2;
                    }

                    if (sprite[j].pal != 6)
                        KILLIT(i);
                    sprite[i].extra = (66-13);
                    sprite[j].pal = 0;
#else
//                    sprite[j].pal = sprite[i].pal;
                    KILLIT(i);
#endif
                }
                else if(sprite[i].extra > (66-13))
                    sprite[i].extra++;
                break;

            case MUSICANDSFX:

                ht = s->hitag;

                if(T2 != SoundToggle)
                {
                    T2 = SoundToggle;
                    T1 = 0;
                }

                if(s->lotag >= 1000 && s->lotag < 2000)
                {
                    x = ldist(&sprite[ps[screenpeek].i],s);
                    if( x < ht && T1 == 0 )
                    {
                        FX_SetReverb( s->lotag - 1000 );
                        T1 = 1;
                    }
                    if( x >= ht && T1 == 1 )
                    {
                        FX_SetReverb(0);
                        FX_SetReverbDelay(0);
                        T1 = 0;
                    }
                }
                else if(s->lotag < 999 && (unsigned)sector[s->sectnum].lotag < 9 && AmbienceToggle && sector[SECT].floorz != sector[SECT].ceilingz)
                {
                    if( (soundm[s->lotag]&2) )
                    {
                        x = dist(&sprite[ps[screenpeek].i],s);
                        if( x < ht && T1 == 0 && FX_VoiceAvailable(soundpr[s->lotag]-1) )
                        {
                            if(numenvsnds == NumVoices)
                            {
                                j = headspritestat[11];
                                while(j >= 0)
                                {
                                    if( PN == MUSICANDSFX && j != i && sprite[j].lotag < 999 && hittype[j].temp_data[0] == 1 && dist(&sprite[j],&sprite[ps[screenpeek].i]) > x )
                                    {
                                        stopenvsound(sprite[j].lotag,j);
                                        break;
                                    }
                                    j = nextspritestat[j];
                                }
                                if(j == -1) goto BOLT;
                            }
                            spritesound(s->lotag,i);
                            T1 = 1;
                        }
                        if( x >= ht && T1 == 1 )
                        {
                            T1 = 0;
                            stopenvsound(s->lotag,i);
                        }
                    }
                    if( (soundm[s->lotag]&16) )
                    {
                        if(T5 > 0) T5--;
                        else for(p=connecthead;p>=0;p=connectpoint2[p])
                            if( p == myconnectindex && ps[p].cursectnum == s->sectnum )
                        {
                            j = s->lotag+((unsigned)global_random%(s->hitag+1));
                            sound(j);
                            T5 =  26*40 + (global_random%(26*40));
                        }
                    }
                }
                break;
        }
        BOLT:
        i = nexti;
    }
}



void movefallers(void)
{
    short i, nexti, sect, j;
    spritetype *s;
    long x;

    i = headspritestat[12];
    while(i >= 0)
    {
        nexti = nextspritestat[i];
        s = &sprite[i];

        sect = s->sectnum;

        if( T1 == 0 )
        {
            s->z -= (16<<8);
            T2 = s->ang;
            x = s->extra;
            IFHIT
            {
#ifdef RRRA
                if( j == RPG || j == RPG2 || j == RADIUSEXPLOSION || j == SEENINE || j == OOZFILTER )
#else
                if( j == RPG || j == RADIUSEXPLOSION || j == SEENINE || j == OOZFILTER )
#endif
                {
                    if(s->extra <= 0)
                    {
                        T1 = 1;
                        j = headspritestat[12];
                        while(j >= 0)
                        {
                            if(sprite[j].hitag == SHT)
                            {
                                hittype[j].temp_data[0] = 1;
                                sprite[j].cstat &= (65535-64);
                                if(sprite[j].picnum == CEILINGSTEAM || sprite[j].picnum == STEAM)
                                    sprite[j].cstat |= 32768;
                            }
                            j = nextspritestat[j];
                        }
                    }
                }
                else
                {
                    hittype[i].extra = 0;
                    s->extra = x;
                }
            }
            s->ang = T2;
            s->z += (16<<8);
        }
        else if(T1 == 1)
        {
            if(s->lotag > 0)
            {
                s->lotag-=3;
                s->xvel = ((64+TRAND)&127);
                s->zvel = -(1024+(TRAND&1023));
            }
            else
            {
                if( s->xvel > 0)
                {
                    s->xvel -= 2;
                    ssp(i,CLIPMASK0);
                }

                if( floorspace(s->sectnum) ) x = 0;
                else
                {
                    if(ceilingspace(s->sectnum))
                        x = gc/6;
                    else
                        x = gc;
                }

                if( s->z < (sector[sect].floorz-FOURSLEIGHT) )
                {
                    s->zvel += x;
                    if(s->zvel > 6144)
                        s->zvel = 6144;
                    s->z += s->zvel;
                }
                if( (sector[sect].floorz-s->z) < (16<<8) )
                {
                    j = 1+(TRAND&7);
                    for(x=0;x<j;x++) RANDOMSCRAP;
                    KILLIT(i);
                }
            }
        }

        BOLT:
        i = nexti;
    }
}

void movestandables(void)
{
    short i, j, k, m, nexti, nextj, nextk, p, q, sect;
    long l, x, *t, x1, y1;
    spritetype *s;

    i = headspritestat[6];
    while(i >= 0)
    {
        nexti = nextspritestat[i];

        t = &hittype[i].temp_data[0];
        s = &sprite[i];
        sect = s->sectnum;

        if( sect < 0 ) KILLIT(i);

        hittype[i].bposx = s->x;
        hittype[i].bposy = s->y;
        hittype[i].bposz = s->z;

        IFWITHIN(CRANE,CRANE+3)
        {
            //t[0] = state
            //t[1] = checking sector number

            if(s->xvel) getglobalz(i);

            if( t[0] == 0 ) //Waiting to check the sector
            {
                j = headspritesect[t[1]];
                while(j>=0)
                {
                    nextj = nextspritesect[j];
                    switch( sprite[j].statnum )
                    {
                        case 1:
                        case 2:
                        case 6:
                        case 10:
                            s->ang = getangle(msx[t[4]+1]-s->x,msy[t[4]+1]-s->y);
                            setsprite(j,msx[t[4]+1],msy[t[4]+1],sprite[j].z);
                            t[0]++;
                            goto BOLT;
                    }
                    j = nextj;
                }
            }

            else if(t[0]==1)
            {
                if( s->xvel < 184 )
                {
                    s->picnum = CRANE+1;
                    s->xvel += 8;
                }
                IFMOVING;
                if(sect == t[1])
                    t[0]++;
            }
            else if(t[0]==2 || t[0]==7)
            {
                s->z += (1024+512);

                if(t[0]==2)
                {
                    if( (sector[sect].floorz - s->z) < (64<<8) )
                        if(s->picnum > CRANE) s->picnum--;

                    if( (sector[sect].floorz - s->z) < (4096+1024))
                        t[0]++;
                }
                if(t[0]==7)
                {
                    if( (sector[sect].floorz - s->z) < (64<<8) )
                    {
                        if(s->picnum > CRANE) s->picnum--;
                        else
                        {
                            if(s->owner==-2)
                            {
                                spritesound(390,ps[p].i);
                                p = findplayer(s,&x);
                                if(ps[p].on_crane == i)
                                    ps[p].on_crane = -1;
                            }
                            t[0]++;
                            s->owner = -1;
                        }
                    }
                }
            }
            else if(t[0]==3)
            {
                s->picnum++;
                if( s->picnum == (CRANE+2) )
                {
                    p = checkcursectnums(t[1]);
                    if(p >= 0 && ps[p].on_ground)
                    {
                        s->owner = -2;
                        ps[p].on_crane = i;
                        spritesound(390,ps[p].i);
                        ps[p].ang = s->ang+1024;
                    }
                    else
                    {
                        j = headspritesect[t[1]];
                        while(j>=0)
                        {
                            switch( sprite[j].statnum )
                            {
                                case 1:
                                case 6:
                                    s->owner = j;
                                    break;
                            }
                            j = nextspritesect[j];
                        }
                    }

                    t[0]++;//Grabbed the sprite
                    t[2]=0;
                    goto BOLT;
                }
            }
            else if(t[0]==4) //Delay before going up
            {
                t[2]++;
                if(t[2] > 10)
                    t[0]++;
            }
            else if(t[0]==5 || t[0] == 8)
            {
                if(t[0]==8 && s->picnum < (CRANE+2))
                    if( (sector[sect].floorz-s->z) > 8192)
                        s->picnum++;

                if(s->z < msx[t[4]+2])
                {
                    t[0]++;
                    s->xvel = 0;
                }
                else
                    s->z -= (1024+512);
            }
            else if(t[0]==6)
            {
                if( s->xvel < 192 )
                    s->xvel += 8;
                s->ang = getangle(msx[t[4]]-s->x,msy[t[4]]-s->y);
                IFMOVING;
                if( ((s->x-msx[t[4]])*(s->x-msx[t[4]])+(s->y-msy[t[4]])*(s->y-msy[t[4]]) ) < (128*128) )
                    t[0]++;
            }

            else if(t[0]==9)
                t[0] = 0;

            setsprite(msy[t[4]+2],s->x,s->y,s->z-(34<<8));

            if(s->owner != -1)
            {
                p = findplayer(s,&x);

                IFHIT
                {
                    if(s->owner == -2)
                        if(ps[p].on_crane == i)
                            ps[p].on_crane = -1;
                    s->owner = -1;
                    s->picnum = CRANE;
                    goto BOLT;
                }

                if(s->owner >= 0)
                {
                    setsprite(s->owner,s->x,s->y,s->z);

                    hittype[s->owner].bposx = s->x;
                    hittype[s->owner].bposy = s->y;
                    hittype[s->owner].bposz = s->z;

                    s->zvel = 0;
                }
                else if(s->owner == -2)
                {
                    ps[p].oposx = ps[p].posx = s->x-(sintable[(ps[p].ang+512)&2047]>>6);
                    ps[p].oposy = ps[p].posy = s->y-(sintable[ps[p].ang&2047]>>6);
                    ps[p].oposz = ps[p].posz = s->z+(2<<8);
                    setsprite(ps[p].i,ps[p].posx,ps[p].posy,ps[p].posz);
                    ps[p].cursectnum = sprite[ps[p].i].sectnum;
                }
            }

            goto BOLT;
        }

        IFWITHIN(WATERFOUNTAIN,WATERFOUNTAIN+3)
        {
            if(t[0] > 0)
            {
                if( t[0] < 20 )
                {
                    t[0]++;

                    s->picnum++;

                    if( s->picnum == ( WATERFOUNTAIN+3 ) )
                        s->picnum = WATERFOUNTAIN+1;
                }
                else
                {
                    p = findplayer(s,&x);

                    if(x > 512)
                    {
                        t[0] = 0;
                        s->picnum = WATERFOUNTAIN;
                    }
                    else t[0] = 1;
                }
            }
            goto BOLT;
        }

        if( AFLAMABLE(s->picnum) )
        {
            if(T1 == 1)
            {
                T2++;
                if( (T2&3) > 0) goto BOLT;

                if( s->picnum == TIRE && T2 == 32 )
                {
                    s->cstat = 0;
                    j = spawn(i,BLOODPOOL);
                    sprite[j].shade = 127;
                }
                else
                {
                    if(s->shade < 64) s->shade++;
                    else KILLIT(i);
                }

                j = s->xrepeat-(TRAND&7);
                if(j < 10)
                {
                    KILLIT(i);
                }

                s->xrepeat = j;

                j = s->yrepeat-(TRAND&7);
                if(j < 4) { KILLIT(i); }
                s->yrepeat = j;
            }
            goto BOLT;
        }


        if( s->picnum >= CRACK1 && s->picnum <= CRACK4 )
        {
            if(s->hitag > 0)
            {
                t[0] = s->cstat;
                t[1] = s->ang;
                j = ifhitbyweapon(i);
#ifdef RRRA
                if(j == RPG || j == RPG2 || j == RADIUSEXPLOSION || j == SEENINE || j == OOZFILTER )
#else
                if(j == RPG || j == RADIUSEXPLOSION || j == SEENINE || j == OOZFILTER )
#endif
                {
                    j = headspritestat[6];
                    while(j >= 0)
                    {
                        if(s->hitag == sprite[j].hitag && ( sprite[j].picnum == OOZFILTER || sprite[j].picnum == SEENINE ) )
                            if(sprite[j].shade != -32)
                                sprite[j].shade = -32;
                        j = nextspritestat[j];
                    }

                    goto DETONATE;
                }
                else
                {
                    s->cstat = t[0];
                    s->ang = t[1];
                    s->extra = 0;
                }
            }
            goto BOLT;
        }

        if(s->picnum == OOZFILTER || s->picnum == SEENINE || s->picnum == SEENINEDEAD || s->picnum == (SEENINEDEAD+1) )
        {
            if(s->shade != -32 && s->shade != -33)
            {
                if(s->xrepeat)
                    j = (ifhitbyweapon(i) >= 0);
                else
                    j = 0;

                if( j || s->shade == -31 )
                {
                    if(j) s->lotag = 0;

                    t[3] = 1;

                    j = headspritestat[6];
                    while(j >= 0)
                    {
                        if(s->hitag == sprite[j].hitag && ( sprite[j].picnum == SEENINE || sprite[j].picnum == OOZFILTER ) )
                            sprite[j].shade = -32;
                        j = nextspritestat[j];
                    }
                }
            }
            else
            {
                if(s->shade == -32)
                {
                    if(s->lotag > 0)
                    {
                        s->lotag-=3;
                        if(s->lotag <= 0) s->lotag = -99;
                    }
                    else
                        s->shade = -33;
                }
                else
                {
                    if( s->xrepeat > 0 )
                    {
                        T3++;
                        if(T3 == 3)
                        {
                            if( s->picnum == OOZFILTER )
                            {
                                T3 = 0;
                                goto DETONATE;
                            }
                            if( s->picnum != (SEENINEDEAD+1) )
                            {
                                T3 = 0;

                                if(s->picnum == SEENINEDEAD) s->picnum++;
                                else if(s->picnum == SEENINE)
                                    s->picnum = SEENINEDEAD;
                            }
                            else goto DETONATE;
                        }
                        goto BOLT;
                    }

                    DETONATE:

                    earthquaketime = 16;

                    j = headspritestat[3];
                    while(j >= 0)
                    {
                        if( s->hitag == sprite[j].hitag )
                        {
                            if(sprite[j].lotag == 13)
                            {
                                if( hittype[j].temp_data[2] == 0 )
                                    hittype[j].temp_data[2] = 1;
                            }
                            else if(sprite[j].lotag == 8)
                                hittype[j].temp_data[4] = 1;
                            else if(sprite[j].lotag == 18)
                            {
                                if(hittype[j].temp_data[0] == 0)
                                    hittype[j].temp_data[0] = 1;
                            }
                            else if(sprite[j].lotag == 21)
                                hittype[j].temp_data[0] = 1;
                        }
                        j = nextspritestat[j];
                    }

                    s->z -= (32<<8);

                    if( ( t[3] == 1 && s->xrepeat ) || s->lotag == -99 )
                    {
                        x = s->extra;
                        spawn(i,EXPLOSION2);
                        hitradius( i,seenineblastradius,x>>2, x-(x>>1),x-(x>>2), x);
                        spritesound(PIPEBOMB_EXPLODE,i);
                    }

                    if(s->xrepeat)
                        for(x=0;x<8;x++) RANDOMSCRAP;

                    KILLIT(i);
                }
            }
            goto BOLT;
        }

        if(s->picnum == MASTERSWITCH)
        {
            if(s->yvel == 1)
                {
                    s->hitag--;
                    if(s->hitag <= 0)
                    {
                        operatesectors(sect,i);

                        j = headspritesect[sect];
                        while(j >= 0)
                        {
                            if(sprite[j].statnum == 3)
                            {
                                switch(sprite[j].lotag)
                                {
                                    case 2:
                                    case 21:
                                    case 31:
                                    case 32:
                                    case 36:
                                        hittype[j].temp_data[0] = 1;
                                        break;
                                    case 3:
                                        hittype[j].temp_data[4] = 1;
                                        break;
                                }
                            }
                            else if(sprite[j].statnum == 6)
                            {
                                switch(sprite[j].picnum)
                                {
                                    case SEENINE:
                                    case OOZFILTER:
                                        sprite[j].shade = -31;
                                        break;
                                }
                            }
                            j = nextspritesect[j];
                        }
                        KILLIT(i);
                    }
                }
                goto BOLT;
        }

        switch(s->picnum)
        {
            case TRASH:

                if(s->xvel == 0) s->xvel = 1;
                IFMOVING
                {
                    makeitfall(i);
                    if(TRAND&1) s->zvel -= 256;
                    if( klabs(s->xvel) < 48 )
                        s->xvel += (TRAND&3);
                }
                else KILLIT(i);
                break;

            case BOLT1:
            case BOLT1+1:
            case BOLT1+2:
            case BOLT1+3:
                p = findplayer(s, &x);
                if( x > 20480 ) goto BOLT;

                if( t[3] == 0 )
                    t[3]=sector[sect].floorshade;

                CLEAR_THE_BOLT:
                if(t[2])
                {
                    t[2]--;
                    sector[sect].floorshade = 20;
                    sector[sect].ceilingshade = 20;
                    goto BOLT;
                }
                if( (s->xrepeat|s->yrepeat) == 0 )
                {
                    s->xrepeat=t[0];
                    s->yrepeat=t[1];
                }
                else if( (TRAND&8) == 0 )
                {
                    t[0]=s->xrepeat;
                    t[1]=s->yrepeat;
                    t[2] = global_random&4;
                    s->xrepeat=s->yrepeat=0;
                    goto CLEAR_THE_BOLT;
                }
                s->picnum++;

                l = global_random&7;
                s->xrepeat=l+8;

                if(l&1) s->cstat ^= 2;

                if( s->picnum == (BOLT1+1) && (TRAND&1) && sector[sect].floorpicnum == HURTRAIL )
                    spritesound(SHORT_CIRCUIT,i);

                if(s->picnum==BOLT1+4) s->picnum=BOLT1;

                if(s->picnum&1)
                {
                    sector[sect].floorshade = 0;
                    sector[sect].ceilingshade = 0;
                }
                else
                {
                    sector[sect].floorshade = 20;
                    sector[sect].ceilingshade = 20;
                }
                goto BOLT;
                
            case WATERDRIP:

                if( t[1] )
                {
                    t[1]--;
                    if(t[1] == 0)
                        s->cstat &= 32767;
                }
                else
                {
                    makeitfall(i);
                    ssp(i,CLIPMASK0);
                    if(s->xvel > 0) s->xvel -= 2;

                    if(s->zvel == 0)
                    {
                        s->cstat |= 32768;

                        if(s->pal != 2)
                            spritesound(SOMETHING_DRIPPING,i);

                        if(sprite[s->owner].picnum != WATERDRIP)
                        {
                            KILLIT(i);
                        }
                        else
                        {
                            hittype[i].bposz = s->z = t[0];
                            t[1] = 48+(TRAND&31);
                        }
                    }
                }


                goto BOLT;

            case DOORSHOCK:
                j = klabs(sector[sect].ceilingz-sector[sect].floorz)>>9;
                s->yrepeat = j+4;
                s->xrepeat = 16;
                s->z = sector[sect].floorz;
                goto BOLT;

            case TOUCHPLATE:
                if( t[1] == 1 && s->hitag >= 0) //Move the sector floor
                {
                    x = sector[sect].floorz;

                    if(t[3] == 1)
                    {
                        if(x >= t[2])
                        {
                            sector[sect].floorz = x;
                            t[1] = 0;
                        }
                        else
                        {
                            sector[sect].floorz += sector[sect].extra;
                            p = checkcursectnums(sect);
                            if(p >= 0) ps[p].posz += sector[sect].extra;
                        }
                    }
                    else
                    {
                        if(x <= s->z)
                        {
                            sector[sect].floorz = s->z;
                            t[1] = 0;
                        }
                        else
                        {
                            sector[sect].floorz -= sector[sect].extra;
                            p = checkcursectnums(sect);
                            if(p >= 0)
                                ps[p].posz -= sector[sect].extra;
                        }
                    }
                    goto BOLT;
                }

                if(t[5] == 1) goto BOLT;

                p = checkcursectnums(sect);
                if( p >= 0 && ( ps[p].on_ground || s->ang == 512) )
                {
                    if( t[0] == 0 && !check_activator_motion(s->lotag) )
                    {
                        t[0] = 1;
                        t[1] = 1;
                        t[3] = !t[3];
                        operatemasterswitches(s->lotag);
                        operateactivators(s->lotag,p);
                        if(s->hitag > 0)
                        {
                            s->hitag--;
                            if(s->hitag == 0) t[5] = 1;
                        }
                    }
                }
                else t[0] = 0;

                if(t[1] == 1)
                {
                    j = headspritestat[6];
                    while(j >= 0)
                    {
                        if(j != i && sprite[j].picnum == TOUCHPLATE && sprite[j].lotag == s->lotag)
                        {
                            hittype[j].temp_data[1] = 1;
                            hittype[j].temp_data[3] = t[3];
                        }
                        j = nextspritestat[j];
                    }
                }
                goto BOLT;

            case CANWITHSOMETHING:
                makeitfall(i);
                IFHIT
                {
                    spritesound(VENT_BUST,i);
                    for(j=0;j<10;j++)
                        RANDOMSCRAP;

                    if(s->lotag) spawn(i,s->lotag);

                    KILLIT(i);
                }
                goto BOLT;

            case EXPLODINGBARREL:
            case WOODENHORSE:
            case HORSEONSIDE:
            case FIREBARREL:
            case FIREVASE:
            case NUKEBARREL:
            case NUKEBARRELDENTED:
            case NUKEBARRELLEAKED:
            case TOILETWATER:
            case RUBBERCAN:
            case STEAM:
            case CEILINGSTEAM:
                p = findplayer(s, &x);
                execute(i,p,x);
                goto BOLT;
            case WATERBUBBLEMAKER:
                p = findplayer(s, &x);
                execute(i,p,x);
                goto BOLT;
        }

        BOLT:
        i = nexti;
    }
}
