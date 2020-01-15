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

#include "types.h"
#include "develop.h"
#include "scriplib.h"
#include "file_lib.h"
#include "gamedefs.h"
#include "keyboard.h"
#include "util_lib.h"
#include "function.h"
#include "control.h"
#include "fx_man.h"
#include "sounds.h"
#include "config.h"
#include "sndcards.h"

#include "duke3d.h"


long tempwallptr;
short spawn( short j, short pn )
{
    short i, s, startwall, endwall, sect, clostest;
    long x, y, d;
    spritetype *sp;

    if(j >= 0)
    {
        i = EGS(sprite[j].sectnum,sprite[j].x,sprite[j].y,sprite[j].z
            ,pn,0,0,0,0,0,0,j,0);
        hittype[i].picnum = sprite[j].picnum;
    }
    else
    {
        i = pn;

        hittype[i].picnum = PN;
        hittype[i].timetosleep = 0;
        hittype[i].extra = -1;

        hittype[i].bposx = SX;
        hittype[i].bposy = SY;
        hittype[i].bposz = SZ;

        OW = hittype[i].owner = i;
        hittype[i].cgg = 0;
        hittype[i].movflag = 0;
        hittype[i].tempang = 0;
        hittype[i].dispicnum = 0;
        hittype[i].floorz = sector[SECT].floorz;
        hittype[i].ceilingz = sector[SECT].ceilingz;

        hittype[i].lastvx = 0;
        hittype[i].lastvy = 0;
        hittype[i].actorstayput = -1;

        T1 = T2 = T3 = T4 = T5 = T6 = 0;

        if(CS&48)
            if( !(PN >= CRACK1 && PN <= CRACK4) )
        {
            if(SS == 127) return i;
            if( wallswitchcheck(i) == 1 && (CS&16) )
            {
                if( PN != ACCESSSWITCH && PN != ACCESSSWITCH2 && sprite[i].pal)
                {
                    if( (ud.multimode < 2) || (ud.multimode > 1 && ud.coop==1) )
                    {
                        sprite[i].xrepeat = sprite[i].yrepeat = 0;
                        sprite[i].cstat = SLT = SHT = 0;
                        return i;
                    }
                }
                CS |= 257;
                if( sprite[i].pal && PN != ACCESSSWITCH && PN != ACCESSSWITCH2)
                    sprite[i].pal = 0;
                return i;
            }

            if( SHT )
            {
                changespritestat(i,12);
                CS |=  257;
                SH = impact_damage;
                return i;
            }
        }

        s = PN;

        if( CS&1 ) CS |= 256;

        if( actorscrptr[s] )
        {
            SH = *(actorscrptr[s]);
            T5 = *(actorscrptr[s]+1);
            T2 = *(actorscrptr[s]+2);
            if( *(actorscrptr[s]+3) && SHT == 0 )
                SHT = *(actorscrptr[s]+3);
        }
        else T2 = T5 = 0;
    }

    sp = &sprite[i];
    sect = sp->sectnum;

    switch(sp->picnum)
    {
            default:

                if( actorscrptr[sp->picnum] )
                {
                    if( j == -1 && sp->lotag > ud.player_skill )
                    {
                        sp->xrepeat=sp->yrepeat=0;
                        changespritestat(i,5);
                        break;
                    }

                        //  Init the size
                    if(sp->xrepeat == 0 || sp->yrepeat == 0)
                        sp->xrepeat = sp->yrepeat = 1;

                    if( actortype[sp->picnum] & 3)
                    {
                        if( ud.monsters_off == 1 )
                        {
                            sp->xrepeat=sp->yrepeat=0;
                            changespritestat(i,5);
                            break;
                        }

                        makeitfall(i);

                        if( actortype[sp->picnum] & 2)
                            hittype[i].actorstayput = sp->sectnum;

                        if (actorfella(sp))
                            ps[myconnectindex].max_actors_killed++;
                        sp->clipdist = 80;
                        if(j >= 0)
                        {
                            if(sprite[j].picnum == RESPAWN)
                                hittype[i].tempang = sprite[i].pal = sprite[j].pal;
                            changespritestat(i,1);
                        }
                        else changespritestat(i,2);
                    }
                    else
                    {
                        sp->clipdist = 40;
                        sp->owner = i;
                        changespritestat(i,1);
                    }

                    hittype[i].timetosleep = 0;

                    if(j >= 0)
                        sp->ang = sprite[j].ang;
                }
                break;
            case RRTILE280:
            case RRTILE281:
            case RRTILE282:
            case RRTILE283:
            case RRTILE2025:
            case RRTILE2026:
            case RRTILE2027:
            case RRTILE2028:
                sp->cstat = 0;
                sp->cstat |= 32768;
                sp->xrepeat = 0;
                sp->yrepeat = 0;
                sp->clipdist = 0;
                sp->extra = 0;
                changespritestat(i,105);
                break;
            case RRTILE3410:
                sp->extra = 0;
                changespritestat(i,107);
                break;
#ifdef RRRA
            case RRTILE8450:
                sp->xrepeat = 64;
                sp->yrepeat = 64;
                sp->extra = sp->lotag;
                sp->cstat |= 257;
                changespritestat(i,116);
                break;
            case PIG+11:
                sp->xrepeat = 16;
                sp->yrepeat = 16;
                sp->clipdist = 0;
                sp->extra = 0;
                sp->cstat = 0;
                changespritestat(i,121);
                break;
            case RRTILE8487:
            case RRTILE8489:
                sp->xrepeat = 32;
                sp->yrepeat = 32;
                sp->extra = 0;
                sp->cstat |= 257;
                sp->hitag = 0;
                changespritestat(i,117);
                break;
            case RRTILE7424:
                sp->extra = 0;
                sp->xrepeat = 0;
                sp->yrepeat = 0;
                changespritestat(i,11);
                break;
            case RRTILE7936:
                sp->xrepeat = 0;
                sp->yrepeat = 0;
                sub_86730(2);
                ps[screenpeek].fogtype = 1;
                break;
            case RRTILE6144:
                sp->xrepeat = 0;
                sp->yrepeat = 0;
                ps[screenpeek].raat5dd = 1;
                break;
            case RRTILE8448:
                sp->lotag = 1;
                sp->clipdist = 0;
                break;
            case RRTILE8099:
                sp->lotag = 5;
                sp->clipdist = 0;
                changespritestat(i,123);
                break;
            case RRTILE8704:
                sp->lotag = 1;
                sp->clipdist = 0;
                break;
            case RRTILE8192:
                sp->xrepeat = 0;
                sp->yrepeat = 0;
                ps[screenpeek].raat5fd = 1;
                break;
            case RRTILE8193:
                sp->xrepeat = 0;
                sp->yrepeat = 0;
                ps[screenpeek].raat601 = 1;
                break;
            case RRTILE8165:
                sp->lotag = 1;
                sp->clipdist = 0;
                sp->owner = i;
                sp->extra = 0;
                changespritestat(i,115);
                break;
            case RRTILE8593:
                sp->lotag = 1;
                sp->clipdist = 0;
                sp->owner = i;
                sp->extra = 0;
                changespritestat(i,122);
                break;
#endif
            case RRTILE285:
            case RRTILE286:
            case RRTILE287:
            case RRTILE288:
            case RRTILE289:
            case RRTILE290:
            case RRTILE291:
            case RRTILE292:
            case RRTILE293:
                sp->cstat = 0;
                sp->cstat |= 32768;
                sp->xrepeat = 0;
                sp->yrepeat = 0;
                sp->clipdist = 0;
                sp->lotag = 0;
                changespritestat(i,106);
                break;
            case WATERSPLASH2:
            case MUD:
                if (j >= 0)
                {
                    setsprite(i,sprite[j].x,sprite[j].y,sprite[j].z);
                    sp->xrepeat = sp->yrepeat = 8+(TRAND&7);
                }
                else sp->xrepeat = sp->yrepeat = 16+(TRAND&15);

                sp->shade = -16;
                sp->cstat |= 128;
                if(j >= 0)
                {
                    if(sector[sprite[j].sectnum].lotag == 2)
                    {
                        sp->z = getceilzofslope(SECT,SX,SY)+(16<<8);
                        sp->cstat |= 8;
                    }
                    else if( sector[sprite[j].sectnum].lotag == 1)
                        sp->z = getflorzofslope(SECT,SX,SY);
                }

                if(sector[sect].floorpicnum == FLOORSLIME ||
                    sector[sect].ceilingpicnum == FLOORSLIME)
                        sp->pal = 7;
            case NEON1:
            case NEON2:
            case NEON3:
            case NEON4:
            case NEON5:
            case NEON6:
            case DOMELITE:
                if(sp->picnum != WATERSPLASH2)
                    sp->cstat |= 257;
                if(sp->picnum == DOMELITE)
                    sp->cstat |= 257;
            case JIBS1:
            case JIBS2:
            case JIBS3:
            case JIBS4:
            case JIBS5:
            case JIBS6:
            case DUKETORSO:
            case DUKEGUN:
            case DUKELEG:
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
                if (sp->picnum == JIBS6)
                {
                    sp->xrepeat >>= 1;
                    sp->yrepeat >>= 1;
                }
#ifdef RRRA
                else if (sp->picnum == RABBITJIBA)
                {
                    sp->xrepeat = 18;
                    sp->yrepeat = 18;
                }
                else if (sp->picnum == RABBITJIBB)
                {
                    sp->xrepeat = 36;
                    sp->yrepeat = 36;
                }
                else if (sp->picnum == RABBITJIBC)
                {
                    sp->xrepeat = 54;
                    sp->yrepeat = 54;
                }
#endif
                changespritestat(i,5);
                break;
            case TONGUE:
                if(j >= 0)
                    sp->ang = sprite[j].ang;
                sp->z -= 38<<8;
                sp->zvel = 256-(TRAND&511);
                sp->xvel = 64-(TRAND&127);
                changespritestat(i,4);
                break;
            case TRANSPORTERSTAR:
            case TRANSPORTERBEAM:
                if(j == -1) break;
                if(sp->picnum == TRANSPORTERBEAM)
                {
                    sp->xrepeat = 31;
                    sp->yrepeat = 1;
                    sp->z = sector[sprite[j].sectnum].floorz-(40<<8);
                }
                else
                {
                    if(sprite[j].statnum == 4)
                    {
                        sp->xrepeat = 8;
                        sp->yrepeat = 8;
                    }
                    else
                    {
                        sp->xrepeat = 48;
                        sp->yrepeat = 64;
                        if(sprite[j].statnum == 10 || badguy(&sprite[j]) )
                            sp->z -= (32<<8);
                    }
                }

                sp->shade = -127;
                sp->cstat = 128|2;
                sp->ang = sprite[j].ang;

                sp->xvel = 128;
                changespritestat(i,5);
                ssp(i,CLIPMASK0);
                setsprite(i,sp->x,sp->y,sp->z);
                break;

            case FRAMEEFFECT1:
                if (j >= 0)
                {
                    sp->xrepeat = sprite[j].xrepeat;
                    sp->yrepeat = sprite[j].yrepeat;
                    if (sprite[j].picnum == APLAYER)
                        T2 = SMALLSMOKE;
                    else
                        T2 = sprite[j].picnum;
                }
                else sp->xrepeat = sp->yrepeat = 0;

                changespritestat(i,5);
                break;

            case FORCESPHERE:
                if (j == -1)
                {
                    sp->cstat = (short)32768;
                    changespritestat(i,2);
                }
                else
                {
                    sp->xrepeat = sp->yrepeat = 1;
                    changespritestat(i,5);
                }
                break;

            case BLOOD:
                sp->xrepeat = sp->yrepeat = 4;
                sp->z -= (26<<8);
                changespritestat(i,5);
                break;
            case BLOODPOOL:
                {
                    short s1;
                    s1 = sp->sectnum;

                    updatesector(sp->x+108,sp->y+108,&s1);
                    if(s1 >= 0 && sector[s1].floorz == sector[sp->sectnum].floorz)
                    {
                        updatesector(sp->x-108,sp->y-108,&s1);
                        if(s1 >= 0 && sector[s1].floorz == sector[sp->sectnum].floorz)
                        {
                            updatesector(sp->x+108,sp->y-108,&s1);
                            if(s1 >= 0 && sector[s1].floorz == sector[sp->sectnum].floorz)
                            {
                                updatesector(sp->x-108,sp->y+108,&s1);
                                if(s1 >= 0 && sector[s1].floorz != sector[sp->sectnum].floorz)
                                { sp->xrepeat = sp->yrepeat = 0;changespritestat(i,5);break;}
                            }
                            else { sp->xrepeat = sp->yrepeat = 0;changespritestat(i,5);break;}
                        }
                        else { sp->xrepeat = sp->yrepeat = 0;changespritestat(i,5);break;}
                    }
                    else { sp->xrepeat = sp->yrepeat = 0;changespritestat(i,5);break;}
                }

                if( sector[SECT].lotag == 1 )
                {
                    changespritestat(i,5);
                    break;
                }

                if(j >= 0)
                {
                    if( sprite[j].pal == 1)
                        sp->pal = 1;
                    else if( sprite[j].pal != 6 && sprite[j].picnum != NUKEBARREL && sprite[j].picnum != TIRE )
                    {
                        sp->pal = 2; // Red
                    }
                    else sp->pal = 0;  // green

                    if(sprite[j].picnum == TIRE)
                        sp->shade = 127;
                }
                sp->cstat |= 32;

            case BLOODSPLAT1:
            case BLOODSPLAT2:
            case BLOODSPLAT3:
            case BLOODSPLAT4:
                sp->cstat |= 16;
                sp->xrepeat = 7+(TRAND&7);
                sp->yrepeat = 7+(TRAND&7);
                sp->z -= (16<<8);
                if(j >= 0 && sprite[j].pal == 6)
                    sp->pal = 6;
                insertspriteq(i);
                changespritestat(i,5);
                break;

            case HYDRENT:
            case SATELITE:
            case FUELPOD:
            case SOLARPANNEL:
            case ANTENNA:
            case GRATE1:
            case CHAIR1:
            case CHAIR2:
            case CHAIR3:
            case BOTTLE1:
            case BOTTLE2:
            case BOTTLE3:
            case BOTTLE4:
            case BOTTLE5:
            case BOTTLE6:
            case BOTTLE7:
            case BOTTLE8:
            case BOTTLE10:
            case BOTTLE11:
            case BOTTLE12:
            case BOTTLE13:
            case BOTTLE14:
            case BOTTLE15:
            case BOTTLE16:
            case BOTTLE17:
            case BOTTLE18:
            case BOTTLE19:
            case SCALE:
            case VACUUM:
            case FANSPRITE:
            case CACTUS:
            case CACTUSBROKE:
            case CAMERALIGHT:
            case MOVIECAMERA:
            case IVUNIT:
            case POT1:
            case POT2:
            case POT3:
            case SUSHIPLATE1:
            case SUSHIPLATE2:
            case SUSHIPLATE3:
            case SUSHIPLATE4:
            case SUSHIPLATE5:
            case WAITTOBESEATED:
            case VASE:
            case PIPE1:
            case PIPE2:
            case PIPE3:
            case PIPE4:
            case PIPE5:
            case PIPE6:
                sp->clipdist = 32;
                sp->cstat |= 257;
                changespritestat(i,0);
                break;
            case FEMMAG1:
            case FEMMAG2:
                sp->cstat &= ~257;
                changespritestat(i,0);
                break;

            case MASKWALL7:
                j = sp->cstat&60;
                sp->cstat = j|1;
                changespritestat(i,0);
                break;
            case FOOTPRINTS:
            case FOOTPRINTS2:
            case FOOTPRINTS3:
            case FOOTPRINTS4:
                if(j >= 0)
                {
                    short s1;
                    s1 = sp->sectnum;

                    updatesector(sp->x+84,sp->y+84,&s1);
                    if(s1 >= 0 && sector[s1].floorz == sector[sp->sectnum].floorz)
                    {
                        updatesector(sp->x-84,sp->y-84,&s1);
                        if(s1 >= 0 && sector[s1].floorz == sector[sp->sectnum].floorz)
                        {
                            updatesector(sp->x+84,sp->y-84,&s1);
                            if(s1 >= 0 && sector[s1].floorz == sector[sp->sectnum].floorz)
                            {
                                updatesector(sp->x-84,sp->y+84,&s1);
                                if(s1 >= 0 && sector[s1].floorz != sector[sp->sectnum].floorz)
                                { sp->xrepeat = sp->yrepeat = 0;changespritestat(i,5);break;}
                            }
                            else { sp->xrepeat = sp->yrepeat = 0;break;}
                        }
                        else { sp->xrepeat = sp->yrepeat = 0;break;}
                    }
                    else { sp->xrepeat = sp->yrepeat = 0;break;}

                    sp->cstat = 32+((ps[sprite[j].yvel].footprintcount&1)<<2);
                    sp->ang = sprite[j].ang;
                }

                sp->z = sector[sect].floorz;
                if(sector[sect].lotag != 1 && sector[sect].lotag != 2)
                    sp->xrepeat = sp->yrepeat = 32;

                insertspriteq(i);
                changespritestat(i,5);
                break;
            case FEM10:
            case NAKED1:
            case STATUE:
            case TOUGHGAL:
                sp->yvel = sp->hitag;
                sp->hitag = -1;
            case QUEBALL:
            case STRIPEBALL:
                if (sp->picnum == QUEBALL || sp->picnum == STRIPEBALL)
                {
                    sp->cstat = 256;
                    sp->clipdist = 8;
                }
                else
                {
                    sp->cstat |= 257;
                    sp->clipdist = 32;
                }
                changespritestat(i,2);
                break;
            case BOWLINGBALL:
                sp->cstat = 256;
                sp->clipdist = 64;
                sp->xrepeat = 11;
                sp->yrepeat = 9;
                changespritestat(i,2);
                break;
            case HENSTAND:
                sp->cstat = 257;
                sp->clipdist = 48;
                sp->xrepeat = 21;
                sp->yrepeat = 15;
                changespritestat(i,2);
                break;
            case RRTILE295:
                sp->cstat |= 32768;
                changespritestat(i,107);
                break;
            case RRTILE296:
            case RRTILE297:
                sp->xrepeat = 64;
                sp->yrepeat = 64;
                sp->clipdist = 64;
                changespritestat(i,108);
                break;
            case RRTILE3190:
            case RRTILE3191:
            case RRTILE3192:
                sp->cstat = 257;
                sp->clipdist = 8;
                sp->xrepeat = 32;
                sp->yrepeat = 26;
                sp->xvel = 32;
                changespritestat(i,1);
                break;
            case RRTILE3120:
                sp->cstat = 257;
                sp->clipdist = 8;
                sp->xrepeat = 12;
                sp->yrepeat = 10;
                sp->xvel = 32;
                changespritestat(i,1);
                break;
            case RRTILE3122:
                sp->cstat = 257;
                sp->clipdist = 2;
                sp->xrepeat = 8;
                sp->yrepeat = 6;
                sp->xvel = 16;
                changespritestat(i,1);
                break;
            case RRTILE3123:
                sp->cstat = 257;
                sp->clipdist = 8;
                sp->xrepeat = 13;
                sp->yrepeat = 13;
                sp->xvel = 16;
                changespritestat(i,1);
                break;
            case RRTILE3124:
                sp->cstat = 257;
                sp->clipdist = 8;
                sp->xrepeat = 17;
                sp->yrepeat = 12;
                sp->xvel = 32;
                changespritestat(i,1);
                break;
            case RRTILE3132:
                sp->cstat = 257;
                sp->clipdist = 8;
                sp->xrepeat = 13;
                sp->yrepeat = 10;
                sp->xvel = 0;
                changespritestat(i,1);
                break;
            case RRTILE3440:
                sp->cstat = 257;
                sp->clipdist = 48;
                sp->xrepeat = 23;
                sp->yrepeat = 23;
                changespritestat(i,2);
                break;
            case DUKELYINGDEAD:
                if(j >= 0 && sprite[j].picnum == APLAYER)
                {
                    sp->xrepeat = sprite[j].xrepeat;
                    sp->yrepeat = sprite[j].yrepeat;
                    sp->shade = sprite[j].shade;
                    sp->pal = ps[sprite[j].yvel].palookup;
                }
                sp->cstat = 0;
                sp->extra = 1;
                sp->xvel = 292;
                sp->zvel = 360;
            case RESPAWNMARKERRED:
                if(sp->picnum == RESPAWNMARKERRED)
                {
                    sp->xrepeat = sp->yrepeat = 8;
                    if(j >= 0) sp->z = hittype[j].floorz;
                }
                else
                {
                    sp->cstat |= 257;
                    sp->clipdist = 128;
                }
            case MIKE:
                if(sp->picnum == MIKE)
                    sp->yvel = sp->hitag;
                changespritestat(i,1);
                break;

            case SPOTLITE:
                T1 = sp->x;
                T2 = sp->y;
                break;
            case BULLETHOLE:
                sp->xrepeat = sp->yrepeat = 3;
                sp->cstat = 16+(krand()&12);
                insertspriteq(i);
            case MONEY:
                if(sp->picnum == MONEY)
                {
                    hittype[i].temp_data[0] = TRAND&2047;
                    sp->cstat = TRAND&12;
                    sp->xrepeat = sp->yrepeat = 8;
                    sp->ang = TRAND&2047;
                }
                changespritestat(i,5);
                break;

            case SHELL: //From the player
            case SHOTGUNSHELL:
                if( j >= 0 )
                {
                    short snum,a;

                    if(sprite[j].picnum == APLAYER)
                    {
                        snum = sprite[j].yvel;
                        a = ps[snum].ang-(TRAND&63)+8;  //Fine tune

                        T1 = TRAND&1;
                        if(sp->picnum == SHOTGUNSHELL)
                            sp->z = (6<<8)+ps[snum].pyoff+ps[snum].posz-((ps[snum].horizoff+ps[snum].horiz-100)<<4);
                        else sp->z = (3<<8)+ps[snum].pyoff+ps[snum].posz-((ps[snum].horizoff+ps[snum].horiz-100)<<4);
                        sp->zvel = -(TRAND&255);
                    }
                    else
                    {
                        a = sp->ang;
                        sp->z = sprite[j].z-PHEIGHT+(7<<8);
                    }

                    sp->x = sprite[j].x+(sintable[(a+512)&2047]>>7);
                    sp->y = sprite[j].y+(sintable[a&2047]>>7);

                    sp->shade = -8;

                    sp->ang = a-512;
                    sp->xvel = 20;

                    if (sp->picnum == SHELL)
                        sp->xrepeat=sp->yrepeat=2;
                    else
                        sp->xrepeat=sp->yrepeat=4;

                    changespritestat(i,5);
                }
                break;
            case RESPAWN:
                sp->extra = 66-13;
            case MUSICANDSFX:
                if (ud.multimode < 2 && sp->pal == 1)
                {
                    sp->xrepeat = sp->yrepeat = 0;
                    changespritestat(i,5);
                    break;
                }
                sp->cstat = (short)32768;
                changespritestat(i,11);
                break;
            case SOUNDFX:
                {
                    short tg;
                    sp->cstat |= 32768;
                    changespritestat(i,2);
                    tg = sp->hitag;
                    tg = sp->lotag;
                }
                break;
            case EXPLOSION2:
            case EXPLOSION3:
            case BURNING:
            case SMALLSMOKE:
                if(j >= 0)
                {
                    sp->ang = sprite[j].ang;
                    sp->shade = -64;
                    sp->cstat = 128|(TRAND&4);
                }

                if(sp->picnum == EXPLOSION2)
                {
                    sp->xrepeat = 48;
                    sp->yrepeat = 48;
                    sp->shade = -127;
                    sp->cstat |= 128;
                }
                else if(sp->picnum == EXPLOSION3)
                {
                    sp->xrepeat = 128;
                    sp->yrepeat = 128;
                    sp->shade = -127;
                    sp->cstat |= 128;
                }
                else if(sp->picnum == SMALLSMOKE)
                {
                    sp->xrepeat = 12;
                    sp->yrepeat = 12;
                }
                else if(sp->picnum == BURNING)
                {
                    sp->xrepeat = 4;
                    sp->yrepeat = 4;
                }

                if(j >= 0)
                {
                    x = getflorzofslope(sp->sectnum,sp->x,sp->y);
                    if(sp->z > x-(12<<8) )
                        sp->z = x-(12<<8);
                }

                changespritestat(i,5);

                break;

            case PLAYERONWATER:
                if(j >= 0)
                {
                    sp->xrepeat = sprite[j].xrepeat;
                    sp->yrepeat = sprite[j].yrepeat;
                    sp->zvel = 128;
                    if(sector[sp->sectnum].lotag != 2)
                        sp->cstat |= 32768;
                }
                changespritestat(i,13);
                break;

            case APLAYER:
                sp->xrepeat = sp->yrepeat = 0;
                j = ud.coop;
                if(j == 2) j = 0;

                if( ud.multimode < 2 || (ud.multimode > 1 && j != sp->lotag) )
                    changespritestat(i,5);
                else
                    changespritestat(i,10);
                break;

            case WATERBUBBLE:
                if(j >= 0 && sprite[j].picnum == APLAYER)
                    sp->z-= (16<<8);
                if(sp->picnum == WATERBUBBLE)
                {
                    if(j >= 0)
                        sp->ang = sprite[j].ang;
                    sp->xrepeat = sp->yrepeat = 1+(TRAND&7);
                }
                else
                    sp->xrepeat = sp->yrepeat = 32;
                changespritestat(i,5);
                break;
            case CRANE:

                sp->cstat |= 64|257;

                sp->picnum += 2;
                sp->z = sector[sect].ceilingz+(48<<8);
                T5 = tempwallptr;

                msx[tempwallptr] = sp->x;
                msy[tempwallptr] = sp->y;
                msx[tempwallptr+2] = sp->z;

                s = headspritestat[0];
                while(s >= 0)
                {
                    if( sprite[s].picnum == CRANEPOLE && SHT == (sprite[s].hitag) )
                    {
                        msy[tempwallptr+2] = s;

                        T2 = sprite[s].sectnum;

                        sprite[s].xrepeat = 48;
                        sprite[s].yrepeat = 128;

                        msx[tempwallptr+1] = sprite[s].x;
                        msy[tempwallptr+1] = sprite[s].y;

                        sprite[s].x = sp->x;
                        sprite[s].y = sp->y;
                        sprite[s].z = sp->z;
                        sprite[s].shade = sp->shade;

                        setsprite(s,sprite[s].x,sprite[s].y,sprite[s].z);
                        break;
                    }
                    s = nextspritestat[s];
                }

                tempwallptr += 3;
                sp->owner = -1;
                sp->extra = 8;
                changespritestat(i,6);
                break;
            case WATERDRIP:
                // TODO: array underflow
                if(j >= 0 && sprite[j].statnum == 10 || sprite[j].statnum == 1)
                {
                    sp->shade = 32;
                    if(sprite[j].pal != 1)
                    {
                        sp->pal = 2;
                        sp->z -= (18<<8);
                    }
                    else sp->z -= (13<<8);
                    sp->ang = getangle(ps[connecthead].posx-sp->x,ps[connecthead].posy-sp->y);
                    sp->xvel = 48-(TRAND&31);
                    ssp(i,CLIPMASK0);
                }
                else if(j == -1)
                {
                    sp->z += (4<<8);
                    T1 = sp->z;
                }
            case TRASH:

                if(sp->picnum != WATERDRIP) sp->ang = TRAND&2047;

                sp->xrepeat = 24;
                sp->yrepeat = 24;
                changespritestat(i,6);
                break;

            case PLUG:
                sp->lotag = 9999;
                changespritestat(i,6);
                break;
            case TOUCHPLATE:
                T3 = sector[sect].floorz;
                if(sector[sect].lotag != 1 && sector[sect].lotag != 2)
                    sector[sect].floorz = sp->z;
                if(sp->pal && ud.multimode > 1)
                {
                    sp->xrepeat=sp->yrepeat=0;
                    changespritestat(i,5);
                    break;
                }
            case WATERBUBBLEMAKER:
                sp->cstat |= 32768;
                changespritestat(i,6);
                break;
            case BOLT1:
            case BOLT1+1:
            case BOLT1+2:
            case BOLT1+3:
                T1 = sp->xrepeat;
                T2 = sp->yrepeat;
            case MASTERSWITCH:
                if(sp->picnum == 8)
                    sp->cstat |= 32768;
                sp->yvel = 0;
                changespritestat(i,6);
                break;
            case BILLYRAYSTAYPUT:
            case BRAYSNIPER:
            case BUBBASTAND:
            case HULKSTAYPUT:
            case HENSTAYPUT:
            case PIGSTAYPUT:
            case MINIONSTAYPUT:
            case COOTSTAYPUT:
#ifdef RRRA
            case SBSWIPE:
            case CHEERSTAYPUT:
#else
            case SBMOVE:
#endif
                hittype[i].actorstayput = sp->sectnum;
            case BOULDER:
            case BOULDER1:
            case RAT:
            case TORNADO:
            case BILLYCOCK:
            case BILLYRAY:
            case DOGRUN:
            case LTH:
            case HULK:
            case HEN:
            case DRONE:
            case PIG:
            case MINION:
            case UFO1:
            case UFO2:
            case UFO3:
            case UFO4:
            case UFO5:
            case COW:
            case COOT:
            case SHARK:
            case VIXEN:
#ifdef RRRA
            case BIKERB:
            case BIKERBV2:
            case BIKER:
            case MAKEOUT:
            case CHEERB:
            case CHEER:
            case COOTPLAY:
            case BILLYPLAY:
            case MINIONBOAT:
            case HULKBOAT:
            case CHEERBOAT:
            case RABBIT:
            case ROCK:
            case ROCK2:
            case MAMACLOUD:
            case MAMA:
#endif
                sp->xrepeat = 40;
                sp->yrepeat = 40;
                switch (sp->picnum)
                {
                    case VIXEN:
                        if (sp->pal == 34)
                        {
                            sp->xrepeat = 22;
                            sp->yrepeat = 21;
                        }
                        else
                        {
                            sp->xrepeat = 22;
                            sp->yrepeat = 20;
                        }
                        sp->clipdist = mulscale7(sp->xrepeat,tilesizx[sp->picnum]);
                        break;
                    case HULKHANG:
                    case HULKHANGDEAD:
                    case HULKJUMP:
                    case HULK:
                    case HULKSTAYPUT:
                        sp->xrepeat = 32;
                        sp->yrepeat = 32;
                        sp->clipdist = mulscale7(sp->xrepeat,tilesizx[sp->picnum]);
                        break;
                    case COOT:
                    case COOTSTAYPUT:
#ifdef RRRA
                    case COOTPLAY:
#endif
                        sp->xrepeat = 24;
                        sp->yrepeat = 18;
                        sp->clipdist = mulscale7(sp->xrepeat,tilesizx[sp->picnum]);
                        sp->clipdist <<= 2;
                        break;
                    case DRONE:
                        sp->xrepeat = 14;
                        sp->yrepeat = 7;
                        sp->clipdist = 128;
                        break;
                    case BILLYCOCK:
                    case BILLYRAY:
                    case BILLYRAYSTAYPUT:
                    case BRAYSNIPER:
                    case BUBBASTAND:
#ifdef RRRA
                    case SBSWIPE:
                    case BILLYPLAY:
#endif
                        sp->xrepeat = 25;
                        sp->yrepeat = 21;
                        sp->clipdist = mulscale7(sp->xrepeat,tilesizx[sp->picnum]);
                        break;
                    case COW:
                        sp->xrepeat = 32;
                        sp->yrepeat = 32;
                        sp->clipdist = mulscale7(sp->xrepeat,tilesizx[sp->picnum]);
                        break;
                    case HEN:
                    case HENSTAYPUT:
                    case HENSTAND:
                        if(sp->pal == 35)
                        {
                            sp->xrepeat = 42;
                            sp->yrepeat = 30;
                            sp->clipdist = mulscale7(sp->xrepeat,tilesizx[sp->picnum]);
                        }
                        else
                        {
                            sp->xrepeat = 21;
                            sp->yrepeat = 15;
                            sp->clipdist = 64;
                        }
                        break;
                    case MINION:
                    case MINIONSTAYPUT:
                        sp->xrepeat = 16;
                        sp->yrepeat = 16;
                        sp->clipdist = mulscale7(sp->xrepeat,tilesizx[sp->picnum]);
#ifdef RRRA
                        if (ps[screenpeek].raat5fd)
                            sp->pal = 8;
#endif
                        break;
                    case DOGRUN:
                    case PIG:
                        sp->xrepeat = 16;
                        sp->yrepeat = 16;
                        sp->clipdist = mulscale7(sp->xrepeat,tilesizx[sp->picnum]);
                        break;
#ifdef RRRA
                    case RABBIT:
                        sp->xrepeat = 18;
                        sp->yrepeat = 18;
                        sp->clipdist = mulscale7(sp->xrepeat,tilesizx[sp->picnum]);
                        break;
                    case MAMACLOUD:
                        sp->xrepeat = 64;
                        sp->yrepeat = 64;
                        sp->cstat = 2;
                        sp->cstat |= 512;
                        sp->x += (TRAND&2047)-1024;
                        sp->y += (TRAND&2047)-1024;
                        sp->z += (TRAND&2047)-1024;
                        break;
                    case MAMA:
                        if (sp->pal == 30)
                        {
                            sp->xrepeat = 26;
                            sp->yrepeat = 26;
                            sp->clipdist = 75;
                        }
                        else if (sp->pal == 31)
                        {
                            sp->xrepeat = 36;
                            sp->yrepeat = 36;
                            sp->clipdist = 100;
                        }
                        else if (sp->pal == 32)
                        {
                            sp->xrepeat = 50;
                            sp->yrepeat = 50;
                            sp->clipdist = 100;
                        }
                        else
                        {
                            sp->xrepeat = 50;
                            sp->yrepeat = 50;
                            sp->clipdist = 100;
                        }
                        break;
                    case BIKERB:
                        sp->xrepeat = 28;
                        sp->yrepeat = 22;
                        sp->clipdist = 72;
                        break;
                    case BIKERBV2:
                        sp->xrepeat = 28;
                        sp->yrepeat = 22;
                        sp->clipdist = 72;
                        break;
                    case BIKER:
                        sp->xrepeat = 28;
                        sp->yrepeat = 22;
                        sp->clipdist = mulscale7(sp->xrepeat,tilesizx[sp->picnum]);
                        break;
                    case CHEERB:
                        sp->xrepeat = 28;
                        sp->yrepeat = 22;
                        sp->clipdist = 72;
                        break;
                    case CHEER:
                    case CHEERSTAYPUT:
                        sp->xrepeat = 20;
                        sp->yrepeat = 20;
                        sp->clipdist = mulscale7(sp->xrepeat,tilesizx[sp->picnum]);
                        break;
                    case MAKEOUT:
                        sp->xrepeat = 26;
                        sp->yrepeat = 26;
                        sp->clipdist = mulscale7(sp->xrepeat,tilesizx[sp->picnum]);
                        break;
                    case MINIONBOAT:
                        sp->xrepeat = 16;
                        sp->yrepeat = 16;
                        sp->clipdist = mulscale7(sp->xrepeat,tilesizx[sp->picnum]);
                        break;
                    case HULKBOAT:
                        sp->xrepeat = 48;
                        sp->yrepeat = 48;
                        sp->clipdist = mulscale7(sp->xrepeat,tilesizx[sp->picnum]);
                        break;
                    case CHEERBOAT:
                        sp->xrepeat = 32;
                        sp->yrepeat = 32;
                        sp->clipdist = mulscale7(sp->xrepeat,tilesizx[sp->picnum]);
                        break;
#endif
                    case TORNADO:
                        sp->xrepeat = 64;
                        sp->yrepeat = 128;
                        sp->clipdist = mulscale7(sp->xrepeat,tilesizx[sp->picnum]);
                        sp->clipdist >>= 2;
                        sp->cstat = 2;
                        break;
                    case LTH:
                        sp->xrepeat = 24;
                        sp->yrepeat = 22;
                        sp->clipdist = mulscale7(sp->xrepeat,tilesizx[sp->picnum]);
                        break;
#ifdef RRRA
                    case ROCK:
                    case ROCK2:
                        sp->xrepeat = 64;
                        sp->yrepeat = 64;
                        sp->clipdist = mulscale7(sp->xrepeat,tilesizx[sp->picnum]);
                        break;
#endif
                    case UFO1:
                    case UFO2:
                    case UFO3:
                    case UFO4:
                    case UFO5:
                        sp->xrepeat = 32;
                        sp->yrepeat = 32;
                        sp->clipdist = mulscale7(sp->xrepeat,tilesizx[sp->picnum]);
                        sp->extra = 50;
                        break;
#ifndef RRRA
                    case SBMOVE:
                        sp->xrepeat = 48;
                        sp->yrepeat = 48;
                        sp->clipdist = mulscale7(sp->xrepeat,tilesizx[sp->picnum]);
                        break;
#endif

                    default:
                        break;
                }

                if(j >= 0) sp->lotag = 0;

                if( ( sp->lotag > ud.player_skill ) || ud.monsters_off == 1 )
                {
                    sp->xrepeat=sp->yrepeat=0;
                    changespritestat(i,5);
                    break;
                }
                else
                {
                    makeitfall(i);

                    if(sp->picnum == RAT)
                    {
                        sp->ang = TRAND&2047;
                        sp->xrepeat = sp->yrepeat = 48;
                        sp->cstat = 0;
                    }
                    else
                    {
                        sp->cstat |= 257;

                        if(sp->picnum != 5501)
                            if (actorfella(sp))
                                ps[myconnectindex].max_actors_killed++;
                    }

                    if(j >= 0)
                    {
                        hittype[i].timetosleep = 0;
                        check_fta_sounds(i);
                        changespritestat(i,1);
                    }
                    else changespritestat(i,2);

                    sp->shade = sprite[j].shade;
                }

                break;
            case LOCATORS:
//                sp->xrepeat=sp->yrepeat=0;
                sp->cstat |= 32768;
                changespritestat(i,7);
                break;
                
            case ACTIVATORLOCKED:
            case ACTIVATOR:
//                sp->xrepeat=sp->yrepeat=0;
                sp->cstat |= 32768;
                if (sp->picnum == ACTIVATORLOCKED)
                    sector[sect].lotag ^= 16384;
                changespritestat(i,8);
                break;
            case DOORSHOCK:
                sp->cstat |= 1+256;
                sp->shade = -12;

                changespritestat(i,6);
                break;

            case OOZ:
                sp->shade = -12;

                if(j >= 0)
                    if( sprite[j].picnum == NUKEBARREL )
                        sp->pal = 8;

                changespritestat(i,1);

                getglobalz(i);

                j = (hittype[i].floorz-hittype[i].ceilingz)>>9;

                sp->yrepeat = j;
                sp->xrepeat = 25-(j>>1);
                sp->cstat |= (TRAND&4);
                break;

            case HEAVYHBOMB:
                sp->owner = i;
                sp->xrepeat = sp->yrepeat = 9;
                sp->yvel = 4;
            case REACTOR2:
            case REACTOR:
            case RECON:

                if (sp->picnum == RECON)
                {
                    if( sp->lotag > ud.player_skill )
                    {
                        sp->xrepeat = sp->yrepeat = 0;
                        changespritestat(i,5);
                        return i;
                    }
                    if (actorfella(sp))
                        ps[myconnectindex].max_actors_killed++;
                    hittype[i].temp_data[5] = 0;
                    if(ud.monsters_off == 1)
                    {
                        sp->xrepeat = sp->yrepeat = 0;
                        changespritestat(i,5);
                        break;
                    }
                    sp->extra = 130;
                }
                
                if(sp->picnum == REACTOR || sp->picnum == REACTOR2)
                    sp->extra = impact_damage;

                CS |= 257; // Make it hitable

                if( ud.multimode < 2 && sp->pal != 0)
                {
                    sp->xrepeat = sp->yrepeat = 0;
                    changespritestat(i,5);
                    break;
                }
                sp->pal = 0;
                SS = -17;

                changespritestat(i,2);
                break;

            case ATOMICHEALTH:
            case STEROIDS:
            case HEATSENSOR:
            case SHIELD:
            case AIRTANK:
            case TRIPBOMBSPRITE:
            case JETPACK:
            case HOLODUKE:

            case FIRSTGUNSPRITE:
            case CHAINGUNSPRITE:
            case SHOTGUNSPRITE:
            case RPGSPRITE:
            case SHRINKERSPRITE:
            case FREEZESPRITE:
            case DEVISTATORSPRITE:

            case SHOTGUNAMMO:
            case FREEZEAMMO:
            case HBOMBAMMO:
            case CRYSTALAMMO:
            case GROWAMMO:
            case BATTERYAMMO:
            case DEVISTATORAMMO:
            case RPGAMMO:
            case BOOTS:
            case AMMO:
            case AMMOLOTS:
            case COLA:
            case FIRSTAID:
            case SIXPAK:

            case RRTILE43:
            case BOWLINGBALLSPRITE:
#ifdef RRRA
            case RPG2SPRITE:
            case MOTOAMMO:
            case BOATAMMO:
#endif
                if (j >= 0)
                {
                    sp->lotag = 0;
                    if (sp->picnum != BOWLINGBALLSPRITE)
                    {
                        sp->z -= (32 << 8);
                        sp->zvel = -(4 << 8);
                    }
                    else
                    {
                        sp->zvel = 0;
                    }
                    ssp(i,CLIPMASK0);
                    sp->cstat = TRAND&4;
                }
                else
                {
                    sp->owner = i;
                    sp->cstat = 0;
                }

                if( ( ud.multimode < 2 && sp->pal != 0) || (sp->lotag > ud.player_skill) )
                {
                    sp->xrepeat = sp->yrepeat = 0;
                    changespritestat(i,5);
                    break;
                }

                sp->pal = 0;

            case ACCESSCARD:

                if (sp->picnum == ATOMICHEALTH)
                    sp->cstat |= 128;

                if(ud.multimode > 1 && ud.coop != 1 && sp->picnum == ACCESSCARD)
                {
                    sp->xrepeat = sp->yrepeat = 0;
                    changespritestat(i,5);
                    break;
                }
                else
                {
                    if(sp->picnum == AMMO)
                        sp->xrepeat = sp->yrepeat = 16;
                    else sp->xrepeat = sp->yrepeat = 32;
                }

                sp->shade = -17;

                if(j >= 0) changespritestat(i,1);
                else
                {
                    changespritestat(i,2);
                    makeitfall(i);
                }
                switch (sp->picnum)
                {
                case FIRSTGUNSPRITE:
                    sp->xrepeat = 16;
                    sp->yrepeat = 16;
                    break;
                case SHOTGUNAMMO:
                    sp->xrepeat = 18;
                    sp->yrepeat = 17;
#ifdef RRRA
                    sp->cstat = 256;
#endif
                    break;
                case SIXPAK:
                    sp->xrepeat = 13;
                    sp->yrepeat = 9;
#ifdef RRRA
                    sp->cstat = 256;
#endif
                    break;
                case FIRSTAID:
                    sp->xrepeat = 8;
                    sp->yrepeat = 8;
                    break;
                case COLA:
                    sp->xrepeat = 5;
                    sp->yrepeat = 4;
                    break;
                case AMMO:
                    sp->xrepeat = 9;
                    sp->yrepeat = 9;
                    break;
#ifdef RRRA
                case MOTOAMMO:
                    sp->xrepeat = 23;
                    sp->yrepeat = 23;
                    break;
                case BOATAMMO:
                    sp->xrepeat = 16;
                    sp->yrepeat = 16;
                    break;
#endif
                case JETPACK:
                    sp->xrepeat = 8;
                    sp->yrepeat = 6;
                    break;
                case STEROIDS:
                    sp->xrepeat = 13;
                    sp->yrepeat = 9;
                    break;
                case ACCESSCARD:
                    sp->xrepeat = 11;
                    sp->yrepeat = 12;
                    break;
                case HEATSENSOR:
                    sp->xrepeat = 6;
                    sp->yrepeat = 4;
                    break;
                case AIRTANK:
                    sp->xrepeat = 19;
                    sp->yrepeat = 16;
                    break;
                case BATTERYAMMO:
                    sp->xrepeat = 15;
                    sp->yrepeat = 15;
                    break;
                case BOWLINGBALLSPRITE:
                    sp->xrepeat = 11;
                    sp->yrepeat = 11;
                    break;
                case TRIPBOMBSPRITE:
                    sp->xrepeat = 11;
                    sp->yrepeat = 11;
                    sp->yvel = 4;
                    sp->xvel = 32;
                    break;
                case RPGSPRITE:
                    sp->xrepeat = 16;
                    sp->yrepeat = 14;
                    break;
#ifdef RRRA
                case RPG2SPRITE:
                    sp->xrepeat = 20;
                    sp->yrepeat = 20;
                    break;
#endif
                case SHRINKERSPRITE:
                    sp->xrepeat = 22;
                    sp->yrepeat = 13;
                    break;
                case DEVISTATORSPRITE:
                    sp->xrepeat = 18;
                    sp->yrepeat = 17;
                    break;
                case RRTILE43:
                    sp->xrepeat = 12;
                    sp->yrepeat = 7;
                    break;
                case GROWSPRITEICON:
                    sp->xrepeat = 10;
                    sp->yrepeat = 9;
                    break;
                case DEVISTATORAMMO:
                    sp->xrepeat = 10;
                    sp->yrepeat = 9;
                    break;
                case ATOMICHEALTH:
                    sp->xrepeat = 8;
                    sp->yrepeat = 8;
                    break;
                case FREEZESPRITE:
                    sp->xrepeat = 17;
                    sp->yrepeat = 16;
                    break;
                }
                sp->shade = sector[sp->sectnum].floorshade;
                break;
            case WATERFOUNTAIN:
                SLT = 1;
            case TREE1:
            case TREE2:
            case TIRE:
                CS = 257; // Make it hitable
                sprite[i].extra = 1;
                changespritestat(i,6);
                break;

            case CAMERA1:
            case CAMERA1+1:
            case CAMERA1+2:
            case CAMERA1+3:
            case CAMERA1+4:
            case CAMERAPOLE:
                sp->extra = 1;

                if(camerashitable) sp->cstat = 257;
                else sp->cstat = 0;

                if( ud.multimode < 2 && sp->pal != 0 )
                {
                    sp->xrepeat = sp->yrepeat = 0;
                    changespritestat(i,5);
                    break;
                }
                else sp->pal = 0;
                if(sp->picnum == CAMERAPOLE) break;
                sp->picnum = CAMERA1;
                changespritestat(i,1);
                break;
            case STEAM:
                if(j >= 0)
                {
                    sp->ang = sprite[j].ang;
                    sp->cstat = 16+128+2;
                    sp->xrepeat=sp->yrepeat=1;
                    sp->xvel = -8;
                    ssp(i,CLIPMASK0);
                }
            case CEILINGSTEAM:
                changespritestat(i,6);
                break;
            case SECTOREFFECTOR:
                sp->yvel = sector[sect].extra;
                sp->cstat |= 32768;
                sp->xrepeat = sp->yrepeat = 0;

                switch(sp->lotag)
                {
                    case 7: // Transporters!!!!
                    case 23:// XPTR END
                        if(sp->lotag != 23)
                        {
                            for(j=0;j<MAXSPRITES;j++)
                                if(sprite[j].statnum < MAXSTATUS && sprite[j].picnum == SECTOREFFECTOR && ( sprite[j].lotag == 7 || sprite[j].lotag == 23 ) && i != j && sprite[j].hitag == SHT)
                                {
                                    OW = j;
                                    break;
                                }
                        }
                        else OW = i;

                        T5 = sector[sect].floorz == SZ;
                        sp->cstat = 0;
                        changespritestat(i,9);
                        return i;
                    case 1:
                        sp->owner = -1;
                        T1 = 1;
                        break;
                    case 18:

                        if(sp->ang == 512)
                        {
                            T2 = sector[sect].ceilingz;
                            if(sp->pal)
                                sector[sect].ceilingz = sp->z;
                        }
                        else
                        {
                            T2 = sector[sect].floorz;
                            if(sp->pal)
                                sector[sect].floorz = sp->z;
                        }

                        sp->hitag <<= 2;
                        break;

                    case 19:
                        sp->owner = -1;
                        break;
                    case 25: // Pistons
                        T5 = sector[sect].ceilingz;
                        sector[sect].ceilingz = sp->z;
                        setinterpolation(&sector[sect].ceilingz);
                        break;
                    case 35:
                        sector[sect].ceilingz = sp->z;
                        break;
                    case 27:
                        if(ud.recstat == 1)
                        {
                            sp->xrepeat=sp->yrepeat=64;
                            sp->cstat &= 32767;
                        }
                        break;
#ifdef RRRA
                    case 47:
                    case 48:
#endif
                    case 12:

                        T2 = sector[sect].floorshade;
                        T3 = sector[sect].ceilingshade;
                        break;

                    case 13:

                        T1 = sector[sect].ceilingz;
                        T2 = sector[sect].floorz;

                        if( klabs(T1-sp->z) < klabs(T2-sp->z) )
                            sp->owner = 1;
                        else sp->owner = 0;

                        if(sp->ang == 512)
                        {
                            if(sp->owner)
                                sector[sect].ceilingz = sp->z;
                            else
                                sector[sect].floorz = sp->z;
                        }
                        else
                            sector[sect].ceilingz = sector[sect].floorz = sp->z;

                        if( sector[sect].ceilingstat&1 )
                        {
                            sector[sect].ceilingstat ^= 1;
                            T4 = 1;

                            if(!sp->owner && sp->ang==512)
                            {
                                sector[sect].ceilingstat ^= 1;
                                T4 = 0;
                            }

                            sector[sect].ceilingshade =
                                sector[sect].floorshade;

                            if(sp->ang==512)
                            {
                                startwall = sector[sect].wallptr;
                                endwall = startwall+sector[sect].wallnum;
                                for(j=startwall;j<endwall;j++)
                                {
                                    x = wall[j].nextsector;
                                    if(x >= 0)
                                        if( !(sector[x].ceilingstat&1) )
                                    {
                                        sector[sect].ceilingpicnum =
                                            sector[x].ceilingpicnum;
                                        sector[sect].ceilingshade =
                                            sector[x].ceilingshade;
                                        break; //Leave earily
                                    }
                                }
                            }
                        }

                        break;

                    case 17:

                        T3 = sector[sect].floorz; //Stopping loc

                        j = nextsectorneighborz(sect,sector[sect].floorz,-1,-1);
                        T4 = sector[j].ceilingz;

                        j = nextsectorneighborz(sect,sector[sect].ceilingz,1,1);
                        T5 = sector[j].floorz;

                        if(numplayers < 2)
                        {
                            setinterpolation(&sector[sect].floorz);
                            setinterpolation(&sector[sect].ceilingz);
                        }

                        break;

                    case 24:
                        sp->yvel <<= 1;
                    case 36:
                        break;

                    case 20:
                    {
                        long q;

                        startwall = sector[sect].wallptr;
                        endwall = startwall+sector[sect].wallnum;

                        //find the two most clostest wall x's and y's
                        q = 0x7fffffff;

                        for(s=startwall;s<endwall;s++)
                        {
                            x = wall[s].x;
                            y = wall[s].y;

                            d = FindDistance2D(sp->x-x,sp->y-y);
                            if( d < q )
                            {
                                q = d;
                                clostest = s;
                            }
                        }

                        T2 = clostest;

                        q = 0x7fffffff;

                        for(s=startwall;s<endwall;s++)
                        {
                            x = wall[s].x;
                            y = wall[s].y;

                            d = FindDistance2D(sp->x-x,sp->y-y);
                            if(d < q && s != T2)
                            {
                                q = d;
                                clostest = s;
                            }
                        }

                        T3 = clostest;
                    }

                    break;

                    case 3:

                        T4=sector[sect].floorshade;

                        sector[sect].floorshade = sp->shade;
                        sector[sect].ceilingshade = sp->shade;

                        sp->owner = sector[sect].ceilingpal<<8;
                        sp->owner |= sector[sect].floorpal;

                        //fix all the walls;

                        startwall = sector[sect].wallptr;
                        endwall = startwall+sector[sect].wallnum;

                        for(s=startwall;s<endwall;s++)
                        {
                            if(!(wall[s].hitag&1))
                                wall[s].shade=sp->shade;
                            if( (wall[s].cstat&2) && wall[s].nextwall >= 0)
                                wall[wall[s].nextwall].shade = sp->shade;
                        }
                        break;

                    case 31:
                        T2 = sector[sect].floorz;
                    //    T3 = sp->hitag;
                        if(sp->ang != 1536) sector[sect].floorz = sp->z;

                        startwall = sector[sect].wallptr;
                        endwall = startwall+sector[sect].wallnum;

                        for(s=startwall;s<endwall;s++)
                            if(wall[s].hitag == 0) wall[s].hitag = 9999;

                        setinterpolation(&sector[sect].floorz);

                        break;
                    case 32:
                        T2 = sector[sect].ceilingz;
                        T3 = sp->hitag;
                        if(sp->ang != 1536) sector[sect].ceilingz = sp->z;

                        startwall = sector[sect].wallptr;
                        endwall = startwall+sector[sect].wallnum;

                        for(s=startwall;s<endwall;s++)
                            if(wall[s].hitag == 0) wall[s].hitag = 9999;

                        setinterpolation(&sector[sect].ceilingz);

                        break;

                    case 4: //Flashing lights

                        T3 = sector[sect].floorshade;

                        startwall = sector[sect].wallptr;
                        endwall = startwall+sector[sect].wallnum;

                        sp->owner = sector[sect].ceilingpal<<8;
                        sp->owner |= sector[sect].floorpal;

                        for(s=startwall;s<endwall;s++)
                            if(wall[s].shade > T4)
                                T4 = wall[s].shade;

                        break;

                    case 9:
                        if( sector[sect].lotag &&
                            labs(sector[sect].ceilingz-sp->z) > 1024)
                                sector[sect].lotag |= 32768; //If its open
                    case 8:
                        //First, get the ceiling-floor shade

                        T1 = sector[sect].floorshade;
                        T2 = sector[sect].ceilingshade;

                        startwall = sector[sect].wallptr;
                        endwall = startwall+sector[sect].wallnum;

                        for(s=startwall;s<endwall;s++)
                            if(wall[s].shade > T3)
                                T3 = wall[s].shade;

                        T4 = 1; //Take Out;

                        break;

                    case 88:
                        //First, get the ceiling-floor shade

                        T1 = sector[sect].floorshade;
                        T2 = sector[sect].ceilingshade;

                        startwall = sector[sect].wallptr;
                        endwall = startwall+sector[sect].wallnum;

                        for(s=startwall;s<endwall;s++)
                            if(wall[s].shade > T3)
                                T3 = wall[s].shade;

                        T4 = 1; //Take Out;
                        break;

                    case 11://Pivitor rotater
                        if(sp->ang>1024) T4 = 2;
                        else T4 = -2;
                    case 0:
                    case 2://Earthquakemakers
                    case 5://Boss Creature
                    case 6://Subway
                    case 14://Caboos
                    case 15://Subwaytype sliding door
                    case 16://That rotating blocker reactor thing
                    case 26://ESCELATOR
                    case 30://No rotational subways

                        if(sp->lotag == 0)
                        {
                            if( sector[sect].lotag == 30 )
                            {
                                if(sp->pal) sprite[i].clipdist = 1;
                                else sprite[i].clipdist = 0;
                                T4 = sector[sect].floorz;
                                sector[sect].hitag = i;
                            }

                            for(j = 0;j < MAXSPRITES;j++)
                            {
                                if( sprite[j].statnum < MAXSTATUS )
                                if( sprite[j].picnum == SECTOREFFECTOR &&
                                    sprite[j].lotag == 1 &&
                                    sprite[j].hitag == sp->hitag)
                                {
                                    if( sp->ang == 512 )
                                    {
                                        sp->x = sprite[j].x;
                                        sp->y = sprite[j].y;
                                    }
                                    break;
                                }
                            }
                            if(j == MAXSPRITES)
                            {
                                sprintf(tempbuf,"Found lonely Sector Effector (lotag 0) at (%ld,%ld)\n",sp->x,sp->y);
                                gameexit(tempbuf);
                            }
                            sp->owner = j;
                        }

                        startwall = sector[sect].wallptr;
                        endwall = startwall+sector[sect].wallnum;

                        T2 = tempwallptr;
                        for(s=startwall;s<endwall;s++)
                        {
                            msx[tempwallptr] = wall[s].x-sp->x;
                            msy[tempwallptr] = wall[s].y-sp->y;
                            tempwallptr++;
                            if(tempwallptr > 2047)
                            {
                                sprintf(tempbuf,"Too many moving sectors at (%ld,%ld).\n",wall[s].x,wall[s].y);
                                gameexit(tempbuf);
                            }
                        }
                        if( sp->lotag == 30 || sp->lotag == 6 || sp->lotag == 14 || sp->lotag == 5 )
                        {

                            startwall = sector[sect].wallptr;
                            endwall = startwall+sector[sect].wallnum;

                            if(sector[sect].hitag == -1)
                                sp->extra = 0;
                            else sp->extra = 1;

                            sector[sect].hitag = i;

                            j = 0;

                            for(s=startwall;s<endwall;s++)
                            {
                                if( wall[ s ].nextsector >= 0 &&
                                    sector[ wall[ s ].nextsector].hitag == 0 &&
#ifdef RRRA
                                        (sector[ wall[ s ].nextsector].lotag < 3 || sector[ wall[ s ].nextsector].lotag == 160) )
#else
                                        sector[ wall[ s ].nextsector].lotag < 3 )
#endif
                                    {
                                        s = wall[s].nextsector;
                                        j = 1;
                                        break;
                                    }
                            }

                            if(j == 0)
                            {
                                sprintf(tempbuf,"Subway found no zero'd sectors with locators\nat (%ld,%ld).\n",sp->x,sp->y);
                                gameexit(tempbuf);
                            }

                            sp->owner = -1;
                            T1 = s;

                            if(sp->lotag != 30)
                                T4 = sp->hitag;
                        }

                        else if(sp->lotag == 16)
                            T4 = sector[sect].ceilingz;

                        else if( sp->lotag == 26 )
                        {
                            T4 = sp->x;
                            T5 = sp->y;
                            if(sp->shade==sector[sect].floorshade) //UP
                                sp->zvel = -256;
                            else
                                sp->zvel = 256;

                            sp->shade = 0;
                        }
                        else if( sp->lotag == 2)
                        {
                            T6 = sector[sp->sectnum].floorheinum;
                            sector[sp->sectnum].floorheinum = 0;
                        }
                }

                switch(sp->lotag)
                {
                    case 6:
                    case 14:
                        j = callsound(sect,i);
                        if(j == -1)
                        {
                            if (sector[sp->sectnum].floorpal == 7)
                                j = 456;
                            else
                                j = 75;
                        }
                        hittype[i].lastvx = j;
                    case 30:
                        if(numplayers > 1) break;
                    case 0:
                    case 1:
                    case 5:
                    case 11:
                    case 15:
                    case 16:
                    case 26:
                        setsectinterpolate(i);
                        break;
                }

#ifdef RRRA
                switch (sprite[i].lotag)
                {
                case 150:
                case 151:
                case 152:
                case 153:
                case 154:
                case 155:
                    changespritestat(i,15);
                    break;
                default:
                    changespritestat(i,3);
                }
#else
                changespritestat(i,3);
#endif

                break;

            case SEENINE:
            case OOZFILTER:

                sp->shade = -16;
                if(sp->xrepeat <= 8)
                {
                    sp->cstat = (short)32768;
                    sp->xrepeat=sp->yrepeat=0;
                }
                else sp->cstat = 1+256;
                sp->extra = impact_damage<<2;
                sp->owner = i;

                changespritestat(i,6);
                break;

            case CRACK1:
            case CRACK2:
            case CRACK3:
            case CRACK4:
                sp->cstat |= 17;
                sp->extra = 1;
                if( ud.multimode < 2 && sp->pal != 0)
                {
                    sp->xrepeat = sp->yrepeat = 0;
                    changespritestat(i,5);
                    break;
                }

                sp->pal = 0;
                sp->owner = i;
                changespritestat(i,6);
                sp->xvel = 8;
                ssp(i,CLIPMASK0);
                break;

#ifdef RRRA
            case EMPTYBIKE:
                if (ud.multimode < 2 && sp->pal == 1)
                {
                    sp->xrepeat = sp->yrepeat = 0;
                    break;
                }
                sp->pal = 0;
                sp->xrepeat = 18;
                sp->yrepeat = 18;
                sp->clipdist = mulscale7(sp->xrepeat,tilesizx[sp->picnum]);
                sp->owner = 100;
                sp->cstat = 257;
                sp->lotag = 1;
                changespritestat(i,1);
                break;
            case EMPTYBOAT:
                if (ud.multimode < 2 && sp->pal == 1)
                {
                    sp->xrepeat = sp->yrepeat = 0;
                    break;
                }
                sp->pal = 0;
                sp->xrepeat = 32;
                sp->yrepeat = 32;
                sp->clipdist = mulscale7(sp->xrepeat,tilesizx[sp->picnum]);
                sp->owner = 20;
                sp->cstat = 257;
                sp->lotag = 1;
                changespritestat(i,1);
                break;
#endif

            case TOILET:
            case STALL:
            case RRTILE2121:
            case RRTILE2122:
                sp->lotag = 1;
                sp->cstat |= 257;
                sp->clipdist = 8;
                sp->owner = i;
                break;
            case CANWITHSOMETHING:
            case RUBBERCAN:
                sp->extra = 0;
            case EXPLODINGBARREL:
            case HORSEONSIDE:
            case FIREBARREL:
            case NUKEBARREL:
            case FIREVASE:
            case NUKEBARRELDENTED:
            case NUKEBARRELLEAKED:
            case WOODENHORSE:

                if(j >= 0)
                    sp->xrepeat = sp->yrepeat = 32;
                sp->clipdist = 72;
                makeitfall(i);
                if(j >= 0)
                    sp->owner = j;
                else sp->owner = i;
            case EGG:
                if( ud.monsters_off == 1 && sp->picnum == EGG )
                {
                    sp->xrepeat = sp->yrepeat = 0;
                    changespritestat(i,5);
                }
                else
                {
                    if(sp->picnum == EGG)
                        sp->clipdist = 24;
                    sp->cstat = 257|(TRAND&4);
                    changespritestat(i,2);
                }
                break;
            case TOILETWATER:
                sp->shade = -16;
                changespritestat(i,6);
                break;
            case RRTILE63:
                sp->cstat |= 32768;
                sp->xrepeat = 1;
                sp->yrepeat = 1;
                sp->clipdist = 1;
                changespritestat(i,100);
                break;
    }
    return i;
}
