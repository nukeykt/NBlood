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

// PRIMITIVE


char haltsoundhack;

short callsound2(short sn, short snum)
{
    short i;
    struct player_struct *p;
    p = &ps[snum];
    i = p->i;
    spritesound(sn,i);
    return 1;
}
short callsound(short sn,short whatsprite)
{
    short i;

    i = headspritesect[sn];
    while(i >= 0)
    {
        if( PN == MUSICANDSFX && SLT < 1000 )
        {
            if(whatsprite == -1) whatsprite = i;

            if(T1 == 0)
            {
                if( (soundm[SLT]&16) == 0)
                {
                    if(SLT)
                    {
                        spritesound(SLT,whatsprite);
                        if(SHT && SLT != SHT && SHT < NUM_SOUNDS)
                            stopsound(SHT);
                    }

                    if( (sector[SECT].lotag&0xff) != 22)
                        T1 = 1;
                }
            }
            else if(SHT < NUM_SOUNDS)
            {
                if(SHT) spritesound(SHT,whatsprite);
                if( (soundm[SLT]&1) || ( SHT && SHT != SLT ) )
                    stopsound(SLT);
                T1 = 0;
            }
            return SLT;
        }
        i = nextspritesect[i];
    }
    return -1;
}


short check_activator_motion( short lotag )
{
    short i, j;
    spritetype *s;

    i = headspritestat[8];
    while ( i >= 0 )
    {
        if ( sprite[i].lotag == lotag )
        {
            s = &sprite[i];

            for ( j = animatecnt-1; j >= 0; j-- )
                if ( s->sectnum == animatesect[j] )
                    return( 1 );

            j = headspritestat[3];
            while ( j >= 0 )
            {
                if(s->sectnum == sprite[j].sectnum)
                    switch(sprite[j].lotag)
                    {
                        case 11:
                        case 30:
                            if ( hittype[j].temp_data[4] )
                                return( 1 );
                            break;
                        case 20:
                        case 31:
                        case 32:
#ifndef RRRA
                        case 18:
#endif
                            if ( hittype[j].temp_data[0] )
                                return( 1 );
                            break;
                    }

                j = nextspritestat[j];
            }
        }
        i = nextspritestat[i];
    }
    return( 0 );
}

char isadoorwall(short dapic)
{
    switch(dapic)
    {
        case DOORTILE1:
        case DOORTILE2:
        case DOORTILE3:
        case DOORTILE4:
        case DOORTILE5:
        case DOORTILE6:
        case DOORTILE7:
        case DOORTILE8:
        case DOORTILE9:
        case DOORTILE10:
        case DOORTILE11:
        case DOORTILE12:
        case DOORTILE14:
        case DOORTILE15:
        case DOORTILE16:
        case DOORTILE17:
        case DOORTILE18:
        case DOORTILE19:
        case DOORTILE20:
        case DOORTILE21:
        case DOORTILE22:
        case RRTILE1856:
        case RRTILE1877:
            return 1;
    }
    return 0;
}

char isablockdoor(short dapic)
{
    switch (dapic)
    {
        case RRTILE1792:
        case RRTILE1801:
        case RRTILE1805:
        case RRTILE1807:
        case RRTILE1808:
        case RRTILE1812:
        case RRTILE1821:
        case RRTILE1826:
        case RRTILE1850:
        case RRTILE1851:
        case RRTILE1856:
        case RRTILE1877:
        case RRTILE1938:
        case RRTILE1942:
        case RRTILE1944:
        case RRTILE1945:
        case RRTILE1951:
        case RRTILE1961:
        case RRTILE1964:
        case RRTILE1985:
        case RRTILE1995:
        case RRTILE2022:
        case RRTILE2052:
        case RRTILE2053:
        case RRTILE2060:
        case RRTILE2074:
        case RRTILE2132:
        case RRTILE2136:
        case RRTILE2139:
        case RRTILE2150:
        case RRTILE2178:
        case RRTILE2186:
        case RRTILE2319:
        case RRTILE2321:
        case RRTILE2326:
        case RRTILE2329:
        case RRTILE2578:
        case RRTILE2581:
        case RRTILE2610:
        case RRTILE2613:
        case RRTILE2621:
        case RRTILE2622:
        case RRTILE2676:
        case RRTILE2732:
        case RRTILE2831:
        case RRTILE2832:
        case RRTILE2842:
        case RRTILE2940:
        case RRTILE2970:
        case RRTILE3083:
        case RRTILE3100:
        case RRTILE3155:
        case RRTILE3195:
        case RRTILE3232:
        case RRTILE3600:
        case RRTILE3631:
        case RRTILE3635:
        case RRTILE3637:
        case RRTILE3643+2:
        case RRTILE3643+3:
        case RRTILE3647:
        case RRTILE3652:
        case RRTILE3653:
        case RRTILE3671:
        case RRTILE3673:
        case RRTILE3684:
        case RRTILE3708:
        case RRTILE3714:
        case RRTILE3716:
        case RRTILE3723:
        case RRTILE3725:
        case RRTILE3737:
        case RRTILE3754:
        case RRTILE3762:
        case RRTILE3763:
        case RRTILE3764:
        case RRTILE3765:
        case RRTILE3767:
        case RRTILE3793:
        case RRTILE3814:
        case RRTILE3815:
        case RRTILE3819:
        case RRTILE3827:
        case RRTILE3837:
#ifdef RRRA
        case RRTILE1996:
        case RRTILE2382:
        case RRTILE2961:
        case RRTILE3804:
        case RRTILE7430:
        case RRTILE7467:
        case RRTILE7469:
        case RRTILE7470:
        case RRTILE7475:
        case RRTILE7566:
        case RRTILE7576:
        case RRTILE7716:
        case RRTILE8063:
        case RRTILE8067:
        case RRTILE8076:
        case RRTILE8106:
        case RRTILE8379:
        case RRTILE8380:
        case RRTILE8565:
        case RRTILE8605:
#endif
            return 1;
    }
    return 0;
}


char isanunderoperator(short lotag)
{
    switch(lotag&0xff)
    {
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 26:
            return 1;
    }
    return 0;
}

char isanearoperator(short lotag)
{
    switch(lotag&0xff)
    {
        case 9:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 25:
        case 26:
        case 29://Toothed door
        case 41:
            return 1;
    }
    return 0;
}

short checkcursectnums(short sect)
{
    short i;
    for(i=connecthead;i>=0;i=connectpoint2[i])
        if( sprite[ps[i].i].sectnum == sect ) return i;
    return -1;
}

long ldist(spritetype *s1,spritetype *s2)
{
    long vx,vy;
    vx = s1->x - s2->x;
    vy = s1->y - s2->y;
    return(FindDistance2D(vx,vy) + 1);
}

long dist(spritetype *s1,spritetype *s2)
{
    long vx,vy,vz;
    vx = s1->x - s2->x;
    vy = s1->y - s2->y;
    vz = s1->z - s2->z;
    return(FindDistance3D(vx,vy,vz>>4));
}

short findplayer(spritetype *s,long *d)
{
    short j, closest_player;
    long x, closest;

    if(ud.multimode < 2)
    {
        *d = klabs(ps[myconnectindex].oposx-s->x) + klabs(ps[myconnectindex].oposy-s->y) + ((klabs(ps[myconnectindex].oposz-s->z+(28<<8)))>>4);
        return myconnectindex;
    }

    closest = 0x7fffffff;
    closest_player = 0;

    for(j=connecthead;j>=0;j=connectpoint2[j])
    {
        x = klabs(ps[j].oposx-s->x) + klabs(ps[j].oposy-s->y) + ((klabs(ps[j].oposz-s->z+(28<<8)))>>4);
        if( x < closest && sprite[ps[j].i].extra > 0 )
        {
            closest_player = j;
            closest = x;
        }
    }

    *d = closest;
    return closest_player;
}

short findotherplayer(short p,long *d)
{
    short j, closest_player;
    long x, closest;

    closest = 0x7fffffff;
    closest_player = p;

    for(j=connecthead;j>=0;j=connectpoint2[j])
        if(p != j && sprite[ps[j].i].extra > 0)
    {
        x = klabs(ps[j].oposx-ps[p].posx) + klabs(ps[j].oposy-ps[p].posy) + (klabs(ps[j].oposz-ps[p].posz)>>4);

        if( x < closest )
        {
            closest_player = j;
            closest = x;
        }
    }

    *d = closest;
    return closest_player;
}



void doanimations(void)
{
	long i, j, a, p, v, dasect;

	for(i=animatecnt-1;i>=0;i--)
	{
		a = *animateptr[i];
		v = animatevel[i]*TICSPERFRAME;
		dasect = animatesect[i];

        if (a == animategoal[i])
        {
            stopinterpolation(animateptr[i]);

            animatecnt--;
            animateptr[i] = animateptr[animatecnt];
            animategoal[i] = animategoal[animatecnt];
            animatevel[i] = animatevel[animatecnt];
            animatesect[i] = animatesect[animatecnt];
            if( sector[animatesect[i]].lotag == 18 || sector[animatesect[i]].lotag == 19 )
                if(animateptr[i] == &sector[animatesect[i]].ceilingz)
                    continue;

           if( (sector[dasect].lotag&0xff) != 22 )
                callsound(dasect,-1);

            continue;
        }

        if (v > 0) { a = min(a+v,animategoal[i]); }
              else { a = max(a+v,animategoal[i]); }

        if( animateptr[i] == &sector[animatesect[i]].floorz)
        {
            for(p=connecthead;p>=0;p=connectpoint2[p])
                if (ps[p].cursectnum == dasect)
                    if ((sector[dasect].floorz-ps[p].posz) < (64<<8))
                        if (sprite[ps[p].i].owner >= 0)
            {
                ps[p].posz += v;
                ps[p].poszv = 0;
                if (p == myconnectindex)
                {
                    myz += v;
                    myzvel = 0;
                    myzbak[((movefifoplc-1)&(MOVEFIFOSIZ-1))] = ps[p].posz;
                }
            }

            for(j=headspritesect[dasect];j>=0;j=nextspritesect[j])
                if (sprite[j].statnum != 3)
                {
                    hittype[j].bposz = sprite[j].z;
                    sprite[j].z += v;
                    hittype[j].floorz = sector[dasect].floorz+v;
                }
        }

		*animateptr[i] = a;
	}
}

getanimationgoal(long *animptr)
{
	long i, j;

	j = -1;
    for(i=animatecnt-1;i>=0;i--)
        if (animptr == (long *)animateptr[i])
		{
			j = i;
			break;
		}
	return(j);
}

setanimation(short animsect,long *animptr, long thegoal, long thevel)
{
	long i, j;

	if (animatecnt >= MAXANIMATES-1)
		return(-1);

	j = animatecnt;
    for(i=0;i<animatecnt;i++)
		if (animptr == animateptr[i])
		{
			j = i;
			break;
		}

    animatesect[j] = animsect;
	animateptr[j] = animptr;
	animategoal[j] = thegoal;
    if (thegoal >= *animptr)
       animatevel[j] = thevel;
    else
       animatevel[j] = -thevel;
    
    if (j == animatecnt) animatecnt++;

    setinterpolation(animptr);

    return(j);
}




void animatecamsprite(void)
{
    short i;

    if(camsprite <= 0) return;

    i = camsprite;

    if(T1 >= 11)
    {
        T1 = 0;

        if(ps[screenpeek].newowner >= 0)
            OW = ps[screenpeek].newowner;

        else if(OW >= 0 && dist(&sprite[ps[screenpeek].i],&sprite[i]) < 2048)
            xyzmirror(OW,PN);
    }
    else T1++;
}

void animatewalls(void)
{
    long i, j, p, t;

#ifdef RRRA
    if (ps[screenpeek].raat5dd == 1)
    {
        for (i = 0; i < MAXWALLS; i++)
        {
            if (wall[i].picnum == RRTILE7873)
                wall[i].xpanning += 6;
            else if (wall[i].picnum == RRTILE7870)
                wall[i].xpanning += 6;
        }
    }
#endif

    for(p=0;p < numanimwalls ;p++)
//    for(p=numanimwalls-1;p>=0;p--)
    {
        i = animwall[p].wallnum;
        j = wall[i].picnum;

            switch(j)
            {
                case SCREENBREAK1:
                case SCREENBREAK2:
                case SCREENBREAK3:
                case SCREENBREAK4:
                case SCREENBREAK5:

                case SCREENBREAK9:
                case SCREENBREAK10:
                case SCREENBREAK11:
                case SCREENBREAK12:
                case SCREENBREAK13:

                    if( (TRAND&255) < 16)
                    {
                        animwall[p].tag = wall[i].picnum;
                        wall[i].picnum = SCREENBREAK6;
                    }

                    continue;

                case SCREENBREAK6:
                case SCREENBREAK7:
                case SCREENBREAK8:

                    if(animwall[p].tag >= 0)
                        wall[i].picnum = animwall[p].tag;
                    else
                    {
                        wall[i].picnum++;
                        if(wall[i].picnum == (SCREENBREAK6+3) )
                            wall[i].picnum = SCREENBREAK6;
                    }
                    continue;

            }

        if(wall[i].cstat&16)
            switch(wall[i].overpicnum)
        {
            case W_FORCEFIELD:
            case W_FORCEFIELD+1:
            case W_FORCEFIELD+2:

                t = animwall[p].tag;

                if(wall[i].cstat&254)
                {
                    wall[i].xpanning -= t>>10; // sintable[(t+512)&2047]>>12;
                    wall[i].ypanning -= t>>10; // sintable[t&2047]>>12;

                    if(wall[i].extra == 1)
                    {
                        wall[i].extra = 0;
                        animwall[p].tag = 0;
                    }
                    else
                        animwall[p].tag+=128;

                    if( animwall[p].tag < (128<<4) )
                    {
                        if( animwall[p].tag&128 )
                            wall[i].overpicnum = W_FORCEFIELD;
                        else wall[i].overpicnum = W_FORCEFIELD+1;
                    }
                    else
                    {
                        if( (TRAND&255) < 32 )
                            animwall[p].tag = 128<<(TRAND&3);
                        else wall[i].overpicnum = W_FORCEFIELD+1;
                    }
                }

                break;
        }
    }
}

char activatewarpelevators(short s,short d) //Parm = sectoreffectornum
{
    short i, sn;

    sn = sprite[s].sectnum;

    // See if the sector exists

    i = headspritestat[3];
    while(i >= 0)
    {
#ifdef RRRA
        if( SLT == 17 || SLT == 18 )
#else
        if( SLT == 17 )
#endif
            if( SHT == sprite[s].hitag )
                if( (klabs(sector[sn].floorz-hittype[s].temp_data[2]) > SP) ||
                    (sector[SECT].hitag == (sector[sn].hitag-d) ) )
                        break;
        i = nextspritestat[i];
    }

    if(i==-1)
    {
        d = 0;
        return 1; // No find
    }
    else
    {
        if(d == 0)
            spritesound(ELEVATOR_OFF,s);
        else spritesound(ELEVATOR_ON,s);
    }


    i = headspritestat[3];
    while(i >= 0)
    {
#ifdef RRRA
        if( SLT == 17 || SLT == 18 )
#else
        if( SLT == 17 )
#endif
            if( SHT == sprite[s].hitag )
            {
                T1 = d;
                T2 = d; //Make all check warp
            }
        i = nextspritestat[i];
    }
    return 0;
}



void operatesectors(short sn,short ii)
{
    long j, l, q, startwall, endwall;
    long u6,u7;
    short i;
    char sect_error;
    sectortype *sptr;

    long u1,u2,u3,u4,u5;

    sect_error = 0;
    sptr = &sector[sn];

    switch(sptr->lotag&(0xffff-49152))
    {

        case 41:
            for (i = 0; i < jaildoorcnt; i++)
            {
                if (jaildoorsecthtag[i] == sptr->hitag)
                {
                    if (jaildooropen[i] == 0)
                    {
                        jaildooropen[i] = 1;
                        jaildoordrag[i] = jaildoordist[i];
#ifdef RRRA
                        if (jaildoorsound[i] != 0)
#endif
                        callsound2(jaildoorsound[i],screenpeek);
                    }
                    if (jaildooropen[i] == 2)
                    {
                        jaildooropen[i] = 3;
                        jaildoordrag[i] = jaildoordist[i];
#ifdef RRRA
                        if (jaildoorsound[i] != 0)
#endif
                        callsound2(jaildoorsound[i],screenpeek);
                    }
                }
            }
            break;

        case 7:
            startwall = sptr->wallptr;
            endwall = startwall+sptr->wallnum;
            for (j = startwall; j < endwall; j++)
            {
                setanimation(sn,&wall[j].x,wall[j].x+1024,4);
                setanimation(sn,&wall[wall[j].nextwall].x,wall[wall[j].nextwall].x+1024,4);
            }
            break;

        case 30:
            j = sector[sn].hitag;
            if( hittype[j].tempang == 0 ||
                hittype[j].tempang == 256)
                    callsound(sn,ii);
            if(sprite[j].extra == 1)
                sprite[j].extra = 3;
            else sprite[j].extra = 1;
            break;

        case 31:

            j = sector[sn].hitag;
            if(hittype[j].temp_data[4] == 0)
                hittype[j].temp_data[4] = 1;
            
            callsound(sn,ii);
            break;

        case 26: //The split doors
            i = getanimationgoal(&sptr->ceilingz);
            if(i == -1) //if the door has stopped
            {
                haltsoundhack = 1;
                sptr->lotag &= 0xff00;
                sptr->lotag |= 22;
                operatesectors(sn,ii);
                sptr->lotag &= 0xff00;
                sptr->lotag |= 9;
                operatesectors(sn,ii);
                sptr->lotag &= 0xff00;
                sptr->lotag |= 26;
            }
            return;

        case 9:
        {
            long dax,day,dax2,day2,sp;
            long wallfind[2];

            startwall = sptr->wallptr;
            endwall = startwall+sptr->wallnum-1;

            sp = sptr->extra>>4;

            //first find center point by averaging all points
            dax = 0L, day = 0L;
            for(i=startwall;i<=endwall;i++)
            {
                dax += wall[i].x;
                day += wall[i].y;
            }
            dax /= (endwall-startwall+1);
            day /= (endwall-startwall+1);

            //find any points with either same x or same y coordinate
            //  as center (dax, day) - should be 2 points found.
            wallfind[0] = -1;
            wallfind[1] = -1;
            for(i=startwall;i<=endwall;i++)
                if ((wall[i].x == dax) || (wall[i].y == day))
                {
                    if (wallfind[0] == -1)
                        wallfind[0] = i;
                    else wallfind[1] = i;
                }

            for(j=0;j<2;j++)
            {
                if ((wall[wallfind[j]].x == dax) && (wall[wallfind[j]].y == day))
                {
                    //find what direction door should open by averaging the
                    //  2 neighboring points of wallfind[0] & wallfind[1].
                    i = wallfind[j]-1; if (i < startwall) i = endwall;
                    dax2 = ((wall[i].x+wall[wall[wallfind[j]].point2].x)>>1)-wall[wallfind[j]].x;
                    day2 = ((wall[i].y+wall[wall[wallfind[j]].point2].y)>>1)-wall[wallfind[j]].y;
                    if (dax2 != 0)
                    {
                        dax2 = wall[wall[wall[wallfind[j]].point2].point2].x;
                        dax2 -= wall[wall[wallfind[j]].point2].x;
                        setanimation(sn,&wall[wallfind[j]].x,wall[wallfind[j]].x+dax2,sp);
                        setanimation(sn,&wall[i].x,wall[i].x+dax2,sp);
                        setanimation(sn,&wall[wall[wallfind[j]].point2].x,wall[wall[wallfind[j]].point2].x+dax2,sp);
                        callsound(sn,ii);
                    }
                    else if (day2 != 0)
                    {
                        day2 = wall[wall[wall[wallfind[j]].point2].point2].y;
                        day2 -= wall[wall[wallfind[j]].point2].y;
                        setanimation(sn,&wall[wallfind[j]].y,wall[wallfind[j]].y+day2,sp);
                        setanimation(sn,&wall[i].y,wall[i].y+day2,sp);
                        setanimation(sn,&wall[wall[wallfind[j]].point2].y,wall[wall[wallfind[j]].point2].y+day2,sp);
                        callsound(sn,ii);
                    }
                }
                else
                {
                    i = wallfind[j]-1; if (i < startwall) i = endwall;
                    dax2 = ((wall[i].x+wall[wall[wallfind[j]].point2].x)>>1)-wall[wallfind[j]].x;
                    day2 = ((wall[i].y+wall[wall[wallfind[j]].point2].y)>>1)-wall[wallfind[j]].y;
                    if (dax2 != 0)
                    {
                        setanimation(sn,&wall[wallfind[j]].x,dax,sp);
                        setanimation(sn,&wall[i].x,dax+dax2,sp);
                        setanimation(sn,&wall[wall[wallfind[j]].point2].x,dax+dax2,sp);
                        callsound(sn,ii);
                    }
                    else if (day2 != 0)
                    {
                        setanimation(sn,&wall[wallfind[j]].y,day,sp);
                        setanimation(sn,&wall[i].y,day+day2,sp);
                        setanimation(sn,&wall[wall[wallfind[j]].point2].y,day+day2,sp);
                        callsound(sn,ii);
                    }
                }
            }

        }
        return;

        case 15://Warping elevators

            if(sprite[ii].picnum != APLAYER) return;
//            if(ps[sprite[ii].yvel].select_dir == 1) return;

            i = headspritesect[sn];
            while(i >= 0)
            {
                if(PN==SECTOREFFECTOR && SLT == 17 ) break;
                i = nextspritesect[i];
            }

            if(sprite[ii].sectnum == sn)
            {
                if( activatewarpelevators(i,-1) )
                    activatewarpelevators(i,1);
                else if( activatewarpelevators(i,1) )
                    activatewarpelevators(i,-1);
                return;
            }
            else
            {
                if(sptr->floorz > SZ)
                    activatewarpelevators(i,-1);
                else
                    activatewarpelevators(i,1);
            }

            return;

        case 16:
        case 17:

            i = getanimationgoal(&sptr->floorz);

            if(i == -1)
            {
                i = nextsectorneighborz(sn,sptr->floorz,1,1);
                if( i == -1 )
                {
                    i = nextsectorneighborz(sn,sptr->floorz,1,-1);
                    if( i == -1 ) return;
                    j = sector[i].floorz;
                    setanimation(sn,&sptr->floorz,j,sptr->extra);
                }
                else
                {
                    j = sector[i].floorz;
                    setanimation(sn,&sptr->floorz,j,sptr->extra);
                }
                callsound(sn,ii);
            }

            return;

        case 18:
        case 19:

            i = getanimationgoal(&sptr->floorz);

            if(i==-1)
            {
                i = nextsectorneighborz(sn,sptr->floorz,1,-1);
                if(i==-1) i = nextsectorneighborz(sn,sptr->floorz,1,1);
                if(i==-1) return;
                j = sector[i].floorz;
                q = sptr->extra;
                l = sptr->ceilingz-sptr->floorz;
                setanimation(sn,&sptr->floorz,j,q);
                setanimation(sn,&sptr->ceilingz,j+l,q);
                callsound(sn,ii);
            }
            return;

        case 29:

            if(sptr->lotag&0x8000)
                j = sector[nextsectorneighborz(sn,sptr->ceilingz,1,1)].floorz;
            else
                j = sector[nextsectorneighborz(sn,sptr->ceilingz,-1,-1)].ceilingz;

            i = headspritestat[3]; //Effectors
            while(i >= 0)
            {
                if( (SLT == 22) &&
                    (SHT == sptr->hitag) )
                {
                    sector[SECT].extra = -sector[SECT].extra;

                    T1 = sn;
                    T2 = 1;
                }
                i = nextspritestat[i];
            }

            sptr->lotag ^= 0x8000;

            setanimation(sn,&sptr->ceilingz,j,sptr->extra);

            callsound(sn,ii);

            return;

        case 20:

            REDODOOR:

            if(sptr->lotag&0x8000)
            {
                i = headspritesect[sn];
                while(i >= 0)
                {
                    if(sprite[i].statnum == 3 && SLT==9)
                    {
                        j = SZ;
                        break;
                    }
                    i = nextspritesect[i];
                }
                if(i==-1)
                    j = sptr->floorz;
            }
            else
            {
                j = nextsectorneighborz(sn,sptr->ceilingz,-1,-1);

                if(j >= 0) j = sector[j].ceilingz;
                else
                {
                    sptr->lotag |= 32768;
                    goto REDODOOR;
                }
            }

            sptr->lotag ^= 0x8000;

            setanimation(sn,&sptr->ceilingz,j,sptr->extra);
            callsound(sn,ii);

            return;

        case 21:
            i = getanimationgoal(&sptr->floorz);
            if (i >= 0)
            {
                if (animategoal[sn] == sptr->ceilingz)
                    animategoal[i] = sector[nextsectorneighborz(sn,sptr->ceilingz,1,1)].floorz;
                else animategoal[i] = sptr->ceilingz;
                j = animategoal[i];
            }
            else
            {
                if (sptr->ceilingz == sptr->floorz)
                    j = sector[nextsectorneighborz(sn,sptr->ceilingz,1,1)].floorz;
                else j = sptr->ceilingz;

                sptr->lotag ^= 0x8000;

                if(setanimation(sn,&sptr->floorz,j,sptr->extra) >= 0)
                    callsound(sn,ii);
            }
            return;

        case 22:

            // REDODOOR22:

            if ( (sptr->lotag&0x8000) )
            {
                q = (sptr->ceilingz+sptr->floorz)>>1;
                j = setanimation(sn,&sptr->floorz,q,sptr->extra);
                j = setanimation(sn,&sptr->ceilingz,q,sptr->extra);
            }
            else
            {
                q = sector[nextsectorneighborz(sn,sptr->floorz,1,1)].floorz;
                j = setanimation(sn,&sptr->floorz,q,sptr->extra);
                q = sector[nextsectorneighborz(sn,sptr->ceilingz,-1,-1)].ceilingz;
                j = setanimation(sn,&sptr->ceilingz,q,sptr->extra);
            }

            sptr->lotag ^= 0x8000;

            callsound(sn,ii);

            return;

        case 23: //Swingdoor

            j = -1;
            q = 0;

            i = headspritestat[3];
            while(i >= 0)
            {
                if( SLT == 11 && SECT == sn && !T5)
                {
                    j = i;
                    break;
                }
                i = nextspritestat[i];
            }

            l = sector[SECT].lotag&0x8000;

            if(j >= 0)
            {
                i = headspritestat[3];
                while(i >= 0)
                {
                    if( l == (sector[SECT].lotag&0x8000) && SLT == 11 && sprite[j].hitag == SHT && !T5 )
                    {
                        if(sector[SECT].lotag&0x8000) sector[SECT].lotag &= 0x7fff;
                        else sector[SECT].lotag |= 0x8000;
                        T5 = 1;
                        T4 = -T4;
                        if(q == 0)
                        {
                            callsound(sn,i);
                            q = 1;
                        }
                    }
                    i = nextspritestat[i];
                }
            }
            return;

        case 25: //Subway type sliding doors

            j = headspritestat[3];
            while(j >= 0)//Find the sprite
            {
                if( (sprite[j].lotag) == 15 && sprite[j].sectnum == sn )
                    break; //Found the sectoreffector.
                j = nextspritestat[j];
            }

            if(j < 0)
                return;

            i = headspritestat[3];
            while(i >= 0)
            {
                if( SHT==sprite[j].hitag )
                {
                    if( SLT == 15 )
                    {
                        sector[SECT].lotag ^= 0x8000; // Toggle the open or close
                        SA += 1024;
                        if(T5) callsound(SECT,i);
                        callsound(SECT,i);
                        if(sector[SECT].lotag&0x8000) T5 = 1;
                        else T5 = 2;
                    }
                }
                i = nextspritestat[i];
            }
            return;

        case 27:  //Extended bridge

            j = headspritestat[3];
            while(j >= 0)
            {
                if( (sprite[j].lotag&0xff)==20 && sprite[j].sectnum == sn) //Bridge
                {

                    sector[sn].lotag ^= 0x8000;
                    if(sector[sn].lotag&0x8000) //OPENING
                        hittype[j].temp_data[0] = 1;
                    else hittype[j].temp_data[0] = 2;
                    callsound(sn,ii);
                    break;
                }
                j = nextspritestat[j];
            }
            return;


        case 28:
            //activate the rest of them

            j = headspritesect[sn];
            while(j >= 0)
            {
                if(sprite[j].statnum==3 && (sprite[j].lotag&0xff)==21)
                    break; //Found it
                j = nextspritesect[j];
            }

            j = sprite[j].hitag;

            l = headspritestat[3];
            while(l >= 0)
            {
                if( (sprite[l].lotag&0xff)==21 && !hittype[l].temp_data[0] &&
                    (sprite[l].hitag) == j )
                    hittype[l].temp_data[0] = 1;
                l = nextspritestat[l];
            }
            callsound(sn,ii);

            return;
    }
}



void operaterespawns(short low)
{
    short i, j, nexti;

    i = headspritestat[11];
    while(i >= 0)
    {
        nexti = nextspritestat[i];
        if(SLT == low) switch(PN)
        {
            case RESPAWN:
                if( badguypic(SHT) && ud.monsters_off ) break;

                j = spawn(i,TRANSPORTERSTAR);
                sprite[j].z -= (32<<8);

                sprite[i].extra = 66-12;   // Just a way to killit
                break;
#ifdef RRRA
            case RRTILE7424:
                if (!ud.monsters_off)
                    changespritestat(i,119);
                break;

#endif
        }
        i = nexti;
    }
}
