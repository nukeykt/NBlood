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

extern FILE *frecfilep;

#define patchstatusbar(x1,y1,x2,y2)                                        \
{                                                                          \
    rotatesprite(0,(200-34)<<16,32768L,0,BOTTOMSTATUSBAR,4,0,10+16+64+128, \
        scale(x1,xdim,320),scale(y1,ydim,200),                             \
        scale(x2,xdim,320)-1,scale(y2,ydim,200)-1);                        \
}

#define patchstatusbar2(x1,y1,x2,y2)                                       \
{                                                                          \
    rotatesprite(0,158<<16,32768L,0,WEAPONBAR,4,0,10+16+64+128,            \
        scale(x1,xdim,320),scale(y1,ydim,200),                             \
        scale(x2,xdim,320)-1,scale(y2,ydim,200)-1);                        \
}

short actorfella(spritetype *s)
{
    switch (s->picnum)
    {
        case BOULDER:
        case BOULDER1:
        case EGG:
        case RAT:
        case TORNADO:
        case BILLYCOCK:
        case BILLYRAY:
        case BILLYRAYSTAYPUT:
        case BRAYSNIPER:
        case DOGRUN:
        case LTH:
        case HULKJUMP:
        case BUBBASTAND:
        case HULK:
        case HULKSTAYPUT:
        case DRONE:
        case PIG:
        case RECON:
        case MINION:
        case MINIONSTAYPUT:
        case UFO1:
        case COOT:
        case COOTSTAYPUT:
        case SHARK:
        case VIXEN:
#ifdef RRRA
        case SBSWIPE:
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
        case ROCK:
        case ROCK2:
        case MAMA:
#else
        case SBMOVE:
        case UFO2:
        case UFO3:
        case UFO4:
        case UFO5:
#endif
            return 1;
    }
    return 0;
}


short badguy(spritetype *s)
{

    switch(s->picnum)
    {
            case BOULDER:
            case BOULDER1:
            case EGG:
            case RAT:
            case TORNADO:
            case BILLYCOCK:
            case BILLYRAY:
            case BILLYRAYSTAYPUT:
            case BRAYSNIPER:
            case DOGRUN:
            case LTH:
            case HULKJUMP:
            case BUBBASTAND:
            case HULK:
            case HULKSTAYPUT:
            case HEN:
            case DRONE:
            case PIG:
            case RECON:
            case MINION:
            case MINIONSTAYPUT:
            case UFO1:
            case COOT:
            case COOTSTAYPUT:
            case SHARK:
            case VIXEN:
#ifdef RRRA
            case SBSWIPE:
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
#else
            case SBMOVE:
            case UFO2:
            case UFO3:
            case UFO4:
            case UFO5:
#endif
                return 1;
    }
    if( actortype[s->picnum] ) return 1;

    return 0;
}


short badguypic(short pn)
{

    switch(pn)
    {
            case BOULDER:
            case BOULDER1:
            case EGG:
            case RAT:
            case TORNADO:
            case BILLYCOCK:
            case BILLYRAY:
            case BILLYRAYSTAYPUT:
            case BRAYSNIPER:
            case DOGRUN:
            case LTH:
            case HULKJUMP:
            case BUBBASTAND:
            case HULK:
            case HULKSTAYPUT:
            case DRONE:
            case PIG:
            case RECON:
            case MINION:
            case MINIONSTAYPUT:
            case UFO1:
            case COOT:
            case COOTSTAYPUT:
            case SHARK:
            case VIXEN:
#ifdef RRRA
            case SBSWIPE:
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
#else
            case SBMOVE:
            case UFO2:
            case UFO3:
            case UFO4:
            case UFO5:
#endif
                return 1;
    }

    if( actortype[pn] ) return 1;

    return 0;
}

#ifdef RRRA
void ShowMotorcycle(long x, long y, short tilenum, signed char shade, char orientation, char p, short a)
{
    char fp;

    fp = sector[ps[screenpeek].cursectnum].floorpal;
    rotatesprite(x<<16,y<<16,34816L,a,tilenum,shade,p,2|orientation,windowx1,windowy1,windowx2,windowy2);
}

void OnMotorcycle(struct player_struct *p, int unk)
{
    if (!p->OnMotorcycle && !(sector[p->cursectnum].lotag == 2))
    {
        if (unk)
        {
            p->posx = sprite[unk].x;
            p->posy = sprite[unk].y;
            p->ang = sprite[unk].ang;
            p->ammo_amount[RA13_WEAPON] = sprite[unk].owner;
            deletesprite(unk);
        }
        p->over_shoulder_on = 0;
        p->OnMotorcycle = 1;
        p->last_full_weapon = p->curr_weapon;
        p->curr_weapon = RA13_WEAPON;
        p->gotweapon[RA13_WEAPON] = 1;
        p->posxv = 0;
        p->posyv = 0;
        p->horiz = 100;
    }
    if (!Sound[186].num)
        spritesound(186, p->i);
}

void OffMotorcycle(struct player_struct *p)
{
    short j;
    if (p->OnMotorcycle)
    {
        if (Sound[188].num > 0)
            stopsound(Sound[188].num);
        if (Sound[187].num > 0)
            stopsound(Sound[187].num);
        if (Sound[186].num > 0)
            stopsound(Sound[186].num);
        if (Sound[214].num > 0)
            stopsound(Sound[214].num);
        if (Sound[42].num == 0)
            spritesound(42, p->i);
        p->OnMotorcycle = 0;
        p->gotweapon[RA13_WEAPON] = 0;
        p->curr_weapon = p->last_full_weapon;
        checkavailweapon(p);
        p->horiz = 100;
        p->raat5b5 = 0;
        p->MotoSpeed = 0;
        p->TiltStatus = 0;
        p->raat5c1 = 0;
        p->VBumpTarget = 0;
        p->VBumpNow = 0;
        p->TurbCount = 0;
        p->posxv = 0;
        p->posyv = 0;
        p->posxv -= sintable[(p->ang+512)&2047]<<7;
        p->posyv -= sintable[p->ang&2047]<<7;
        p->raat5b9 = 0;
        j = spawn(p->i, EMPTYBIKE);
        sprite[j].ang = p->ang;
        sprite[j].xvel += sintable[(p->ang+512)&2047]<<7;
        sprite[j].yvel += sintable[p->ang&2047]<<7;
        sprite[j].owner = p->ammo_amount[RA13_WEAPON];
    }
}

void ShowBoat(long x, long y, short tilenum, signed char shade, char orientation, char p, short a)
{
    char fp;

    fp = sector[ps[screenpeek].cursectnum].floorpal;
    rotatesprite(x<<16,y<<16,66048L,a,tilenum,shade,p,2|orientation,windowx1,windowy1,windowx2,windowy2);
}

void OnBoat(struct player_struct *p, int unk)
{
    if (!p->OnBoat)
    {
        if (unk)
        {
            p->posx = sprite[unk].x;
            p->posy = sprite[unk].y;
            p->ang = sprite[unk].ang;
            p->ammo_amount[RA14_WEAPON] = sprite[unk].owner;
            deletesprite(unk);
        }
        p->over_shoulder_on = 0;
        p->OnBoat = 1;
        p->last_full_weapon = p->curr_weapon;
        p->curr_weapon = RA14_WEAPON;
        p->gotweapon[RA14_WEAPON] = 1;
        p->posxv = 0;
        p->posyv = 0;
        p->horiz = 100;
    }
}

void OffBoat(struct player_struct *p)
{
    short j;
    if (p->OnBoat)
    {
        p->OnBoat = 0;
        p->gotweapon[RA14_WEAPON] = 0;
        p->curr_weapon = p->last_full_weapon;
        checkavailweapon(p);
        p->horiz = 100;
        p->raat5b5 = 0;
        p->MotoSpeed = 0;
        p->TiltStatus = 0;
        p->raat5c1 = 0;
        p->VBumpTarget = 0;
        p->VBumpNow = 0;
        p->TurbCount = 0;
        p->posxv = 0;
        p->posyv = 0;
        p->posxv -= sintable[(p->ang+512)&2047]<<7;
        p->posyv -= sintable[p->ang&2047]<<7;
        p->raat5b9 = 0;
        j = spawn(p->i, EMPTYBOAT);
        sprite[j].ang = p->ang;
        sprite[j].xvel += sintable[(p->ang+512)&2047]<<7;
        sprite[j].yvel += sintable[p->ang&2047]<<7;
        sprite[j].owner = p->ammo_amount[RA14_WEAPON];
    }
}

#endif

void myos(long x, long y, short tilenum, signed char shade, char orientation)
{
    char p;
    short a;

    if(orientation&4)
        a = 1024;
    else a = 0;

    p = sector[ps[screenpeek].cursectnum].floorpal;
    rotatesprite(x<<16,y<<16,65536L,a,tilenum,shade,p,2|orientation,windowx1,windowy1,windowx2,windowy2);
}

void myospal(long x, long y, short tilenum, signed char shade, char orientation, char p)
{
    char fp;
    short a;

    if(orientation&4)
        a = 1024;
    else a = 0;

    fp = sector[ps[screenpeek].cursectnum].floorpal;

    rotatesprite(x<<16,y<<16,32768L,a,tilenum,shade,p,2|orientation,windowx1,windowy1,windowx2,windowy2);

}

#ifdef RRRA
void myospalsb(long x, long y, short tilenum, signed char shade, char orientation, char p)
{
    char fp;
    short a;

    if(orientation&4)
        a = 1024;
    else a = 0;

    fp = sector[ps[screenpeek].cursectnum].floorpal;

    rotatesprite(x<<16,y<<16,32768L,a,tilenum,shade,p,2|orientation,windowx1,windowy1,windowx2,windowy2);

}
#endif

void rdmyospal(long x, long y, short tilenum, signed char shade, char orientation, char p)
{
    char fp;
    short a;

    if(orientation&4)
        a = 1024;
    else a = 0;

    fp = sector[ps[screenpeek].cursectnum].floorpal;

    rotatesprite(x<<16,y<<16,36700L,a,tilenum,shade,p,2|orientation,windowx1,windowy1,windowx2,windowy2);

}
void rd2myospal(long x, long y, short tilenum, signed char shade, char orientation, char p)
{
    char fp;
    short a;

    if(orientation&4)
        a = 1024;
    else a = 0;

    fp = sector[ps[screenpeek].cursectnum].floorpal;

    rotatesprite(x<<16,y<<16,44040L,a,tilenum,shade,p,2|orientation,windowx1,windowy1,windowx2,windowy2);

}

void rd3myospal(long x, long y, short tilenum, signed char shade, char orientation, char p)
{
    char fp;
    short a;

    if(orientation&4)
        a = 1024;
    else a = 0;

    fp = sector[ps[screenpeek].cursectnum].floorpal;

    rotatesprite(x<<16,y<<16,47040L,a,tilenum,shade,p,2|orientation,windowx1,windowy1,windowx2,windowy2);

}

void invennum(long x,long y,char num1,char ha,char sbits)
{
    char dabuf[80] = {0};
    sprintf(dabuf,"%ld",num1);
    if(num1 > 99)
    {
        rotatesprite((x-4)<<16,(y-1)<<16,32768L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,sbits,0,0,xdim-1,ydim-1);
        rotatesprite((x)<<16,y<<16,32768L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,sbits,0,0,xdim-1,ydim-1);
        rotatesprite((x+4)<<16,y<<16,32768L,0,THREEBYFIVE+dabuf[2]-'0',ha,0,sbits,0,0,xdim-1,ydim-1);
    }
    else if(num1 > 9)
    {
        rotatesprite((x)<<16,y<<16,32768L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,sbits,0,0,xdim-1,ydim-1);
        rotatesprite((x+4)<<16,y<<16,32768L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,sbits,0,0,xdim-1,ydim-1);
    }
    else
        rotatesprite((x+4)<<16,y<<16,32768L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,sbits,0,0,xdim-1,ydim-1);
}


void weaponnum(short ind,long x,long y,long num1, long num2,char ha)
{
    char dabuf[80] = {0};

    rotatesprite((x-7)<<16,y<<16,65536L,0,THREEBYFIVE+ind+1,ha-10,7,10+128,0,0,xdim-1,ydim-1);
    rotatesprite((x-3)<<16,y<<16,65536L,0,THREEBYFIVE+10,ha,0,10+128,0,0,xdim-1,ydim-1);
    rotatesprite((x+9)<<16,y<<16,65536L,0,THREEBYFIVE+11,ha,0,10+128,0,0,xdim-1,ydim-1);

    if(num1 > 99) num1 = 99;
    if(num2 > 99) num2 = 99;

    sprintf(dabuf,"%ld",num1);
    if(num1 > 9)
    {
        rotatesprite((x)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        rotatesprite((x+4)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
    }
    else rotatesprite((x+4)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);

    sprintf(dabuf,"%ld",num2);
    if(num2 > 9)
    {
        rotatesprite((x+13)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        rotatesprite((x+17)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
    }
    else rotatesprite((x+13)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
}

void weaponnum999(char ind,long x,long y,long num1, long num2,char ha)
{
    char dabuf[80] = {0};

    rotatesprite((x-7)<<16,y<<16,65536L,0,THREEBYFIVE+ind+1,ha-10,7,10+128,0,0,xdim-1,ydim-1);
    rotatesprite((x-4)<<16,y<<16,65536L,0,THREEBYFIVE+10,ha,0,10+128,0,0,xdim-1,ydim-1);
    rotatesprite((x+13)<<16,y<<16,65536L,0,THREEBYFIVE+11,ha,0,10+128,0,0,xdim-1,ydim-1);

    sprintf(dabuf,"%ld",num1);
    if(num1 > 99)
    {
        rotatesprite((x)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        rotatesprite((x+4)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        rotatesprite((x+8)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[2]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
    }
    else if(num1 > 9)
    {
        rotatesprite((x+4)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        rotatesprite((x+8)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
    }
    else rotatesprite((x+8)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);

    sprintf(dabuf,"%ld",num2);
    if(num2 > 99)
    {
        rotatesprite((x+17)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        rotatesprite((x+21)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        rotatesprite((x+25)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[2]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
    }
    else if(num2 > 9)
    {
        rotatesprite((x+17)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        rotatesprite((x+21)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
    }
    else rotatesprite((x+25)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
}


    //REPLACE FULLY
void weapon_amounts(struct player_struct *p,long x,long y,long u)
{
     int cw;

     cw = p->curr_weapon;

     if (u&4)
     {
         if (u != 0xffffffff) patchstatusbar(96,178,96+12,178+6);
         weaponnum999(PISTOL_WEAPON,x,y,
                     p->ammo_amount[PISTOL_WEAPON],max_ammo_amount[PISTOL_WEAPON],
                     12-20*(cw == PISTOL_WEAPON) );
     }
     if (u&8)
     {
         if (u != 0xffffffff) patchstatusbar(96,184,96+12,184+6);
         weaponnum999(SHOTGUN_WEAPON,x,y+6,
                     p->ammo_amount[SHOTGUN_WEAPON],max_ammo_amount[SHOTGUN_WEAPON],
                     (!p->gotweapon[SHOTGUN_WEAPON]*9)+12-18*
                     (cw == SHOTGUN_WEAPON) );
     }
     if (u&16)
     {
         if (u != 0xffffffff) patchstatusbar(96,190,96+12,190+6);
         weaponnum999(CHAINGUN_WEAPON,x,y+12,
                      p->ammo_amount[CHAINGUN_WEAPON],max_ammo_amount[CHAINGUN_WEAPON],
                      (!p->gotweapon[CHAINGUN_WEAPON]*9)+12-18*
                      (cw == CHAINGUN_WEAPON) );
     }
     if (u&32)
     {
         if (u != 0xffffffff) patchstatusbar(135,178,135+8,178+6);
         weaponnum(RPG_WEAPON,x+39,y,
                  p->ammo_amount[RPG_WEAPON],max_ammo_amount[RPG_WEAPON],
                  (!p->gotweapon[RPG_WEAPON]*9)+12-19*
                  (cw == RPG_WEAPON) );
     }
     if (u&64)
     {
         if (u != 0xffffffff) patchstatusbar(135,184,135+8,184+6);
         weaponnum(HANDBOMB_WEAPON,x+39,y+6,
                     p->ammo_amount[HANDBOMB_WEAPON],max_ammo_amount[HANDBOMB_WEAPON],
                     (((!p->ammo_amount[HANDBOMB_WEAPON])|(!p->gotweapon[HANDBOMB_WEAPON]))*9)+12-19*
                     ((cw == HANDBOMB_WEAPON) || (cw == HANDREMOTE_WEAPON)));
     }
     if (u&128)
     {
         if (u != 0xffffffff) patchstatusbar(135,190,135+8,190+6);
         if(!p->ammo_amount[SHRINKER_WEAPON] || cw == GROW_WEAPON)
             weaponnum(SHRINKER_WEAPON,x+39,y+12,
                 p->ammo_amount[GROW_WEAPON],max_ammo_amount[GROW_WEAPON],
                 (!p->gotweapon[GROW_WEAPON]*9)+12-18*
                 (cw == GROW_WEAPON) );
         else
             weaponnum(SHRINKER_WEAPON,x+39,y+12,
                 p->ammo_amount[SHRINKER_WEAPON],max_ammo_amount[SHRINKER_WEAPON],
                 (!p->gotweapon[SHRINKER_WEAPON]*9)+12-18*
                 (cw == SHRINKER_WEAPON) );
     }
     if (u&256)
     {
         if (u != 0xffffffff) patchstatusbar(166,178,166+8,178+6);
         weaponnum(DEVISTATOR_WEAPON,x+70,y,
                     p->ammo_amount[DEVISTATOR_WEAPON],max_ammo_amount[DEVISTATOR_WEAPON],
                     (!p->gotweapon[DEVISTATOR_WEAPON]*9)+12-18*
                     (cw == DEVISTATOR_WEAPON) );
     }
     if (u&512)
     {
         if (u != 0xffffffff) patchstatusbar(166,184,166+8,184+6);
         weaponnum(TRIPBOMB_WEAPON,x+70,y+6,
                     p->ammo_amount[TRIPBOMB_WEAPON],max_ammo_amount[TRIPBOMB_WEAPON],
                     (!p->gotweapon[TRIPBOMB_WEAPON]*9)+12-18*
                     (cw == TRIPBOMB_WEAPON) );
     }

     if (u&65536L)
     {
         if (u != 0xffffffff) patchstatusbar(166,190,166+8,190+6);
         weaponnum(-1,x+70,y+12,
                     p->ammo_amount[FREEZE_WEAPON],max_ammo_amount[FREEZE_WEAPON],
                     (!p->gotweapon[FREEZE_WEAPON]*9)+12-18*
                     (cw == FREEZE_WEAPON) );
     }
}

void digitalnumber(long x,long y,long n,char s,char cs)
{
    short i, j, k, p, c;
    char b[10];

    ltoa(n,b,10);
    i = strlen(b);
    j = 0;

    for(k=0;k<i;k++)
    {
        p = DIGITALNUM+*(b+k)-'0';
        j += (tilesizx[p]>>1)+1;
    }
    c = x-(j>>1);

    j = 0;
    for(k=0;k<i;k++)
    {
        p = DIGITALNUM+*(b+k)-'0';
        rotatesprite((c+j)<<16,y<<16,32768L,0,p,s,0,cs,0,0,xdim-1,ydim-1);
        j += (tilesizx[p]>>1)+1;
    }
}

/*

void scratchmarks(long x,long y,long n,char s,char p)
{
    long i, ni;

    ni = n/5;
    for(i=ni;i >= 0;i--)
    {
        overwritesprite(x-2,y,SCRATCH+4,s,0,0);
        x += tilesizx[SCRATCH+4]-1;
    }

    ni = n%5;
    if(ni) overwritesprite(x,y,SCRATCH+ni-1,s,p,0);
}
  */
void displayinventory(struct player_struct *p)
{
    short n, j, xoff, y;

    j = xoff = 0;

    n = (p->jetpack_amount > 0)<<3; if(n&8) j++;
    n |= ( p->scuba_amount > 0 )<<5; if(n&32) j++;
    n |= (p->steroids_amount > 0)<<1; if(n&2) j++;
    n |= ( p->holoduke_amount > 0)<<2; if(n&4) j++;
    n |= (p->firstaid_amount > 0); if(n&1) j++;
    n |= (p->heat_amount > 0)<<4; if(n&16) j++;
    n |= (p->boot_amount > 0)<<6; if(n&64) j++;

    xoff = 160-(j*11);

    j = 0;

    if(ud.screen_size > 4)
    {
        y = 160;
        if(ud.multimode > 1)
            y -= 4;
        if(ud.multimode > 4)
            y -= 4;
    }
    else y = 180;

    if(ud.screen_size == 4)
        xoff += 56;

    while( j <= 9 )
    {
        if( n&(1<<j) )
        {
            switch( n&(1<<j) )
            {
                case   1:
                rotatesprite(xoff<<16,y<<16,32768L,0,FIRSTAID_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
                case   2:
                rotatesprite((xoff+1)<<16,y<<16,32768L,0,STEROIDS_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
                case   4:
                rotatesprite((xoff+2)<<16,y<<16,32768L,0,HOLODUKE_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
                case   8:
                rotatesprite(xoff<<16,y<<16,32768L,0,JETPACK_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
                case  16:
                rotatesprite(xoff<<16,y<<16,32768L,0,HEAT_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
                case  32:
                rotatesprite(xoff<<16,y<<16,32768L,0,AIRTANK_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
                case 64:
                rotatesprite(xoff<<16,(y-1)<<16,32768L,0,BOOT_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
            }

            xoff += 22;

            if(p->inven_icon == j+1)
                rotatesprite((xoff-2)<<16,(y+19)<<16,32768L,1024,ARROW,-32,0,2+16,windowx1,windowy1,windowx2,windowy2);
        }

        j++;
    }
}



void displayfragbar(void)
{
    short i, j;

    j = 0;

    for(i=connecthead;i>=0;i=connectpoint2[i])
        if(i > j) j = i;

    rotatesprite(0,0,32800L,0,FRAGBAR,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
    if(j >= 4) rotatesprite(319,(8)<<16,32800L,0,FRAGBAR,0,0,10+16+64+128,0,0,xdim-1,ydim-1);
    if(j >= 8) rotatesprite(319,(16)<<16,32800L,0,FRAGBAR,0,0,10+16+64+128,0,0,xdim-1,ydim-1);
    if(j >= 12) rotatesprite(319,(24)<<16,32800L,0,FRAGBAR,0,0,10+16+64+128,0,0,xdim-1,ydim-1);

    for(i=connecthead;i>=0;i=connectpoint2[i])
    {
        minitext(21+(73*(i&3)),2+((i&28)<<1),&ud.user_name[i][0],9+(i&7),2+8+16+128);
        sprintf(tempbuf,"%d",ps[i].frag-ps[i].fraggedself);
        minitext(17+50+(73*(i&3)),2+((i&28)<<1),tempbuf,9+(i&7),2+8+16+128);
    }
}

void weaponbar(short snum)
{
    struct player_struct *p;
    short i, icon, amount;

    p = &ps[snum];
    rotatesprite(0,158<<16,32800L,0,WEAPONBAR,0,0,10+16+64+128,0,0,xdim-1,ydim-1);
    for(i=0;i<10;i++)
    {
#ifdef RRRA
        if (i == 4 && p->curr_weapon == RA16_WEAPON)
        {
            icon = AMMO_ICON + 10;
            rotatesprite((18+i*32)<<16,160<<16,32800L,0,icon,0,0,10+16+64+128,0,0,xdim-1,ydim-1);
            amount = p->ammo_amount[RA16_WEAPON];
            invennum(38+i*32,162,(char)amount,0,10+128);
        }
        else
        {
#endif
        icon = AMMO_ICON + i;
        if(p->gotweapon[i+1])
            rotatesprite((18+i*32)<<16,160<<16,32800L,0,icon,0,0,10+16+64+128,0,0,xdim-1,ydim-1);
        amount = p->ammo_amount[i + 1];
        invennum(38+i*32,162,(char)amount,0,10+128);
#ifdef RRRA
        }
#endif
    }
}

void coolgaugetext(short snum)
{
    struct player_struct *p;
    long i, j, o, ss, u;
    char c, permbit;

    p = &ps[snum];

    if (p->invdisptime > 0) displayinventory(p);


    if(ps[snum].gm&MODE_MENU)
        if( (current_menu >= 400  && current_menu <= 405) )
            return;

    ss = ud.screen_size; if (ss < 4) return;

    if ( ud.multimode > 1 && ud.coop != 1 )
    {
        if (pus)
            { displayfragbar(); }
        else
        {
            for(i=connecthead;i>=0;i=connectpoint2[i])
                if (ps[i].frag != sbar.frag[i]) { displayfragbar(); break; }
        }
        for(i=connecthead;i>=0;i=connectpoint2[i])
            if (i != myconnectindex)
                sbar.frag[i] = ps[i].frag;
    }

    if (ss == 4)   //DRAW MINI STATUS BAR:
    {
        rotatesprite(2<<16,(200-28)<<16,32768L,0,HEALTHBOX,0,21,10+16,0,0,xdim-1,ydim-1);
        if (p->inven_icon)
            rotatesprite(77<<16,(200-30)<<16,32768L,0,INVENTORYBOX,0,21,10+16,0,0,xdim-1,ydim-1);

        if(sprite[p->i].pal == 1 && p->last_extra < 2)
            digitalnumber(20,200-17,1,-16,10+16);
        else digitalnumber(20,200-17,p->last_extra,-16,10+16);

        rotatesprite(41<<16,(200-28)<<16,32768L,0,AMMOBOX,0,21,10+16,0,0,xdim-1,ydim-1);

        if (p->curr_weapon == HANDREMOTE_WEAPON) i = HANDBOMB_WEAPON; else i = p->curr_weapon;
        digitalnumber(59,200-17,p->ammo_amount[i],-16,10+16);

        o = 158; permbit = 0;
        if (p->inven_icon)
        {
            switch(p->inven_icon)
            {
                case 1: i = FIRSTAID_ICON; minitext(262-o+10,190,"%",0,permbit+10+16); break;
                case 2: i = STEROIDS_ICON; minitext(262-o+10,190,"%",0,permbit+10+16); break;
                case 3: i = HOLODUKE_ICON; break;
                case 4: i = JETPACK_ICON; break;
                case 5: i = HEAT_ICON; break;
                case 6: i = AIRTANK_ICON; break;
                case 7: i = BOOT_ICON; break;
                default: i = -1;
            }
            if (i >= 0) rotatesprite((231-o+10)<<16,(200-21)<<16,32768L,0,i,0,0,10+16+permbit,0,0,xdim-1,ydim-1);
            
            j = 0x80000000;
            switch(p->inven_icon)
            {
                case 1: i = p->firstaid_amount; break;
                case 2: i = ((p->steroids_amount+3)>>2); break;
                case 3: i = p->holoduke_amount/400; j = p->holoduke_on; break;
                case 4: i = p->jetpack_amount/100; j = p->jetpack_on; break;
                case 5: i = p->heat_amount/12; j = p->heat_on; break;
                case 6: i = ((p->scuba_amount+63)>>6); break;
                case 7: i = ((p->boot_amount/10)>>2); break;
            }
            invennum(254-o+8,200-6,(char)i,0,10+permbit);
        }
        return;
    }

        //DRAW/UPDATE FULL STATUS BAR:

    if (pus) { pus = 0; u = 0xffffffff; } else u = 0;

    if (sbar.frag[myconnectindex] != p->frag) { sbar.frag[myconnectindex] = p->frag; u |= 32768; }
    if (sbar.got_access != p->got_access) { sbar.got_access = p->got_access; u |= 16384; }
    if (sbar.last_extra != p->last_extra) { sbar.last_extra = p->last_extra; u |= 1; }
    if (sbar.shield_amount != p->shield_amount) { sbar.shield_amount = p->shield_amount; u |= 2; }
    if (sbar.curr_weapon != p->curr_weapon) { sbar.curr_weapon = p->curr_weapon; u |= (4+8+16+32+64+128+256+512+1024+65536L); }
    for(i=1;i < 10;i++)
    {
        if (sbar.ammo_amount[i] != p->ammo_amount[i]) {
        sbar.ammo_amount[i] = p->ammo_amount[i]; if(i < 9) u |= ((2<<i)+1024); else u |= 65536L+1024; }
        if (sbar.gotweapon[i] != p->gotweapon[i]) { sbar.gotweapon[i] =
        p->gotweapon[i]; if(i < 9 ) u |= ((2<<i)+1024); else u |= 65536L+1024; }
    }
#ifdef RRRA
    if (p->OnMotorcycle || p->OnBoat || p->curr_weapon == RA16_WEAPON)
        u |= 1024;
#endif
    if (sbar.inven_icon != p->inven_icon) { sbar.inven_icon = p->inven_icon; u |= (2048+4096+8192); }
    if (sbar.holoduke_on != p->holoduke_on) { sbar.holoduke_on = p->holoduke_on; u |= (4096+8192); }
    if (sbar.jetpack_on != p->jetpack_on) { sbar.jetpack_on = p->jetpack_on; u |= (4096+8192); }
    if (sbar.heat_on != p->heat_on) { sbar.heat_on = p->heat_on; u |= (4096+8192); }
    if (sbar.firstaid_amount != p->firstaid_amount) { sbar.firstaid_amount = p->firstaid_amount; u |= 8192; }
    if (sbar.steroids_amount != p->steroids_amount) { sbar.steroids_amount = p->steroids_amount; u |= 8192; }
    if (sbar.holoduke_amount != p->holoduke_amount) { sbar.holoduke_amount = p->holoduke_amount; u |= 8192; }
    if (sbar.jetpack_amount != p->jetpack_amount) { sbar.jetpack_amount = p->jetpack_amount; u |= 8192; }
    if (sbar.heat_amount != p->heat_amount) { sbar.heat_amount = p->heat_amount; u |= 8192; }
    if (sbar.scuba_amount != p->scuba_amount) { sbar.scuba_amount = p->scuba_amount; u |= 8192; }
    if (sbar.boot_amount != p->boot_amount) { sbar.boot_amount = p->boot_amount; u |= 8192; }
    if (u == 0) return;

    //0 - update health
    //1 - update armor
    //2 - update PISTOL_WEAPON ammo
    //3 - update SHOTGUN_WEAPON ammo
    //4 - update CHAINGUN_WEAPON ammo
    //5 - update RPG_WEAPON ammo
    //6 - update HANDBOMB_WEAPON ammo
    //7 - update SHRINKER_WEAPON ammo
    //8 - update DEVISTATOR_WEAPON ammo
    //9 - update TRIPBOMB_WEAPON ammo
    //10 - update ammo display
    //11 - update inventory icon
    //12 - update inventory on/off
    //13 - update inventory %
    //14 - update keys
    //15 - update kills
    //16 - update FREEZE_WEAPON ammo

    if (u == 0xffffffff)
    {
        patchstatusbar(0,0,320,200);
        if (ud.multimode > 1 && ud.coop != 1)
            rotatesprite(277<<16,(200-27)<<16,65536L,0,KILLSICON,0,0,10+16+128,0,0,xdim-1,ydim-1);
        if (ud.screen_size > 8)
            weaponbar(snum);
    }
    if (ud.multimode > 1 && ud.coop != 1)
    {
        if (u&32768)
        {
            if (u != 0xffffffff) patchstatusbar(276,183,299,193);
            digitalnumber(287,200-17,max(p->frag-p->fraggedself,0),-16,10+16+128);
        }
    }
    else
    {
        if (u != 0xffffffff) patchstatusbar(136,182,164,194);
        if (p->keys[3]) rotatesprite(140<<16,182<<16,32768L,0,ACCESS_ICON,0,23,10+16+128,0,0,xdim-1,ydim-1);
        if (p->keys[2]) rotatesprite(153<<16,182<<16,32768L,0,ACCESS_ICON,0,21,10+16+128,0,0,xdim-1,ydim-1);
        if (p->keys[1]) rotatesprite(146<<16,189<<16,32768L,0,ACCESS_ICON,0,0,10+16+128,0,0,xdim-1,ydim-1);
    }

    if (u&1)
    {
        if (u != 0xffffffff) patchstatusbar(52,183,77,193);
        if(sprite[p->i].pal == 1 && p->last_extra < 2)
            digitalnumber(64,200-17,1,-16,10+16+128);
        else digitalnumber(64,200-17,p->last_extra,-16,10+16+128);
    }

    if (u&1024)
    {
        if (u != 0xffffffff) patchstatusbar(95,183,120,193);
#ifdef RRRA
        if (p->curr_weapon != KNEE_WEAPON && p->curr_weapon != RA15_WEAPON)
#else
        if (p->curr_weapon != KNEE_WEAPON)
#endif
        {
            if (p->curr_weapon == HANDREMOTE_WEAPON) i = HANDBOMB_WEAPON; else i = p->curr_weapon;
            digitalnumber(107,200-17,p->ammo_amount[i],-16,10+16+128);
            if (ud.screen_size > 8)
            {
                if (p->curr_weapon == RPG_WEAPON || p->curr_weapon == HANDBOMB_WEAPON)
                {
                    patchstatusbar2(126,158,142,174);
                    invennum(134,162,p->ammo_amount[HANDBOMB_WEAPON],0,10+128);
                    patchstatusbar2(158,158,174,174);
                    invennum(166,162,p->ammo_amount[RPG_WEAPON],0,10+128);
                }
#ifdef RRRA
                if (p->curr_weapon == RA16_WEAPON)
                {
                    patchstatusbar2(158,158,174,174);
                    invennum(166,162,p->ammo_amount[RA16_WEAPON],0,10+128);
                }
#endif
                else
                {
                    patchstatusbar2(p->curr_weapon*32-2,158,p->curr_weapon*32+14,174);
                    invennum(p->curr_weapon*32+6,162,p->ammo_amount[p->curr_weapon],0,10+128);
                }
                for (i=1;i<=10;i++)
                {
#ifdef RRRA
                    if (i == 4 && p->curr_weapon == RA16_WEAPON)
                    {
                        if (!p->ammo_amount[RA16_WEAPON-1])
                            if (p->gotweapon[RA16_WEAPON-1])
                        {
                            patchstatusbar2(i*32-2,158,i*32+14,174);
                            invennum(i*32+6,162,p->ammo_amount[RA16_WEAPON-1],0,10+128);
                        }
                    }
                    else
#endif
                    if (!p->ammo_amount[i])
                        if (p->gotweapon[i])
                    {
                        patchstatusbar2(i*32-2,158,i*32+14,174);
                        invennum(i*32+6,162,p->ammo_amount[i],0,10+128);
                    }
                }
            }
        }
    }

    if (u&(2048+4096+8192))
    {
        if (u != 0xffffffff)
        {
            if (u&(2048+4096)) { patchstatusbar(177,176,222,197); }
                              else { patchstatusbar(201,190,216,200); }
        }
        if (p->inven_icon)
        {
            o = 0; permbit = 128;

            if (u&(2048+4096))
            {
                switch(p->inven_icon)
                {
                    case 1: i = FIRSTAID_ICON; minitext(214-o+2,190,"%",0,10+16+permbit); break;
                    case 2: i = STEROIDS_ICON; minitext(214-o+2,190,"%",0,10+16+permbit); break;
                    case 3: i = HOLODUKE_ICON; break;
                    case 4: i = JETPACK_ICON; break;
                    case 5: i = HEAT_ICON; break;
                    case 6: i = AIRTANK_ICON; break;
                    case 7: i = BOOT_ICON; break;
                }
                if (i == AIRTANK_ICON)
                    rotatesprite((183-o)<<16,(200-24)<<16,32768L,0,AIRTANK_ICON,0,0,10+16+permbit,0,0,xdim-1,ydim-1);
                else if (i == FIRSTAID_ICON || i == BOOT_ICON || i == STEROIDS_ICON)
                    rotatesprite((183-o)<<16,(200-22)<<16,32768L,0,i,0,0,10+16+permbit,0,0,xdim-1,ydim-1);
                else
                    rotatesprite((183-o)<<16,(200-21)<<16,32768L,0,i,0,0,10+16+permbit,0,0,xdim-1,ydim-1);
                if (p->inven_icon == 6 || p->inven_icon == 7) minitext(201-o,180,"AUTO",2,10+16+permbit);
            }
            if (u&(2048+4096))
            {
                switch(p->inven_icon)
                {
                    case 3: j = p->holoduke_on; break;
                    case 4: j = p->jetpack_on; break;
                    case 5: j = p->heat_on; break;
                    default: j = 0x80000000;
                }
            }
            if (u&8192)
            {
                switch(p->inven_icon)
                {
                    case 1: i = p->firstaid_amount; break;
                    case 2: i = ((p->steroids_amount+3)>>2); break;
                    case 3: i = p->holoduke_amount/400; break;
                    case 4: i = p->jetpack_amount/100; break;
                    case 5: i = p->heat_amount/12; break;
                    case 6: i = ((p->scuba_amount+63)>>6); break;
                    case 7: i = ((p->boot_amount/10)>>1); break;
                }
                invennum(206-o,200-6,(char)i,0,10+permbit);
            }
        }
    }
}

void gutmeter(short player)
{
    long o, ss;
    struct player_struct *p;

    p = &ps[player];
    ss = ud.screen_size;

    if (ss <= 4)
        return;

    patchstatusbar(240,168,310,199);
    p->drunkang = ((p->drink_amt * 8) + 1647) & 0x7ff;
    if (p->drink_amt >= 100)
    {
        p->drink_amt = 100;
        p->drunkang = 400;
    }
    rotatesprite(257<<16,181<<16,32768L,p->drunkang,GUTMETER,0,0,10+128,0,0,xdim-1,ydim-1);
    rotatesprite(293<<16,181<<16,32768L,p->eatang,GUTMETER,0,0,10+128,0,0,xdim-1,ydim-1);
    o = 9;
    if (p->drink_amt >= 0 && p->drink_amt <= 30)
    {
        rotatesprite(239<<16,(181+o)<<16,32768L,0,GUTMETER_LIGHT1,0,0,10+128+16,0,0,xdim-1,ydim-1);
    }
    else if (p->drink_amt >= 31 && p->drink_amt <= 65)
    {
        rotatesprite(248<<16,(181+o)<<16,32768L,0,GUTMETER_LIGHT2,0,0,10+128+16,0,0,xdim-1,ydim-1);
    }
    else if (p->drink_amt >= 66 && p->drink_amt <= 87)
    {
        rotatesprite(256<<16,(181+o)<<16,32768L,0,GUTMETER_LIGHT3,0,0,10+128+16,0,0,xdim-1,ydim-1);
    }
    else
    {
        rotatesprite(265<<16,(181+o)<<16,32768L,0,GUTMETER_LIGHT4,0,0,10+128+16,0,0,xdim-1,ydim-1);
    }

    if (p->eat >= 0 && p->eat <= 30)
    {
        rotatesprite(276<<16,(181+o)<<16,32768L,0,GUTMETER_LIGHT1,0,0,10+128+16,0,0,xdim-1,ydim-1);
    }
    else if (p->eat >= 31 && p->eat <= 65)
    {
        rotatesprite(285<<16,(181+o)<<16,32768L,0,GUTMETER_LIGHT2,0,0,10+128+16,0,0,xdim-1,ydim-1);
    }
    else if (p->eat >= 66 && p->eat <= 87)
    {
        rotatesprite(294<<16,(181+o)<<16,32768L,0,GUTMETER_LIGHT3,0,0,10+128+16,0,0,xdim-1,ydim-1);
    }
    else
    {
        rotatesprite(302<<16,(181+o)<<16,32768L,0,GUTMETER_LIGHT4,0,0,10+128+16,0,0,xdim-1,ydim-1);
    }
}
  

#define AVERAGEFRAMES 16
static long frameval[AVERAGEFRAMES], framecnt = 0;

void tics(void)
{
    long i;
    char b[10];

    i = totalclock;
    if (i != frameval[framecnt])
    {
        sprintf(b,"%ld",(TICRATE*AVERAGEFRAMES)/(i-frameval[framecnt]));
        printext256(windowx1,windowy1,31,-21,b,1);
        frameval[framecnt] = i;
    }

    framecnt = ((framecnt+1)&(AVERAGEFRAMES-1));
}

void coords(short snum)
{
    short y = 0;

    if(ud.multimode > 1 && ud.multimode < 5)
        y = 8;
    else if(ud.multimode > 4)
        y = 16;

    sprintf(tempbuf,"X= %ld",ps[snum].posx);
    printext256(274L,y,31,-1,tempbuf,1);
    sprintf(tempbuf,"Y= %ld",ps[snum].posy);
    printext256(274L,y+7L,31,-1,tempbuf,1);
    sprintf(tempbuf,"Z= %ld",ps[snum].posz);
    printext256(274L,y+14L,31,-1,tempbuf,1);
    sprintf(tempbuf,"A= %ld",ps[snum].ang);
    printext256(274L,y+21L,31,-1,tempbuf,1);
    sprintf(tempbuf,"ZV= %ld",ps[snum].poszv);
    printext256(274L,y+28L,31,-1,tempbuf,1);
    sprintf(tempbuf,"OG= %ld",ps[snum].on_ground);
    printext256(274L,y+35L,31,-1,tempbuf,1);
    sprintf(tempbuf,"AM= %ld",ps[snum].ammo_amount[GROW_WEAPON]);
    printext256(274L,y+43L,31,-1,tempbuf,1);
}

void operatefta(void)
{
    short i, j, k;

    if (user_quote_time > 0)
    {
        if(ud.screen_size > 0)
            gametext( (320>>1),200-41,user_quote,0);
        else gametext( (320>>1),200-8,user_quote,0);
        if(user_quote_time == 1)
        {
            pub = NUMPAGES;
            pus = NUMPAGES;
        }
    }

    j = ps[screenpeek].fta;

    if(j > 1)
    {
        if( ud.coop != 1 && ud.screen_size > 0 && ud.multimode > 1)
        {
            j = 0;
            k = 8;

            for(i=connecthead;i>=0;i=connectpoint2[i])
                if(i > j) j = i;

            if(j >= 4 && j <= 8)
                k += 8;
            else if(j > 8 && j <= 12)
                k += 16;
            else if(j > 12)
                k += 24;
        }
        else k = 0;

        gametext(320>>1,k,fta_quotes[ps[screenpeek].ftq],0);
    }
}

void FTA(short q,struct player_struct *p)
{
    if( ud.fta_on == 1)
    {
        if( p->fta > 0 && q != 115 && q != 116 )
            if( p->ftq == 115 || p->ftq == 116 ) return;
        
        p->fta = 100;

        if( p->ftq != q )
        {
            pub = NUMPAGES;
            pus = NUMPAGES;
            p->ftq = q;
        }
    }
}

void showtwoscreens(void)
{
    short i;
#ifdef RRRA
    return;
#endif
    if (!KB_KeyWaiting())
    {
        getpackets();
        if (ready2send != 0)
            return;
        playanm("in_03.anm", 5, 3);
        totalclock = 0;
        while (!KB_KeyWaiting());
        KB_FlushKeyboardQueue();
    }
    if (!KB_KeyWaiting())
    {
        getpackets();
        if (ready2send != 0)
            return;
        playanm("in_04.anm", 5, 3);
        totalclock = 0;
        while (!KB_KeyWaiting());
        KB_FlushKeyboardQueue();
    }
    return;
    for(i=0;i<64;i+=7) palto(0,0,0,i);
    KB_FlushKeyboardQueue();
    clearview(0L);
    rotatesprite(0,0,65536L,0,TENSCREEN,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
    nextpage(); for(i=63;i>0;i-=7) palto(0,0,0,i);
    totalclock = 0;
    while( !KB_KeyWaiting() && totalclock < 2400);
}

void binscreen(void)
{
    long fil;
    fil = kopen4load("redneck.bin",loadfromgrouponly);
    if(fil == -1) return;
    kread(fil,(char *)0xb8000,4000);
    kclose(fil);
}


void gameexit(char *t)
{
    short i;

    if(*t != 0) ps[myconnectindex].palette = (char *) &palette[0];

    if(numplayers > 1)
    {
        allowtimetocorrecterrorswhenquitting();
        uninitmultiplayers();
    }

    if(ud.recstat == 1) closedemowrite();
    else if(ud.recstat == 2) { fclose(frecfilep); }

    if(playerswhenstarted > 1 && ud.coop != 1 && *t == ' ')
    {
        dobonus(1);
        setgamemode(ScreenMode,ScreenWidth,ScreenHeight);
    }

    if( *t != 0 && *(t+1) != 'V' && *(t+1) != 'Y' && playonten == 0 )
        showtwoscreens();

    Shutdown();
#ifdef RRRA
    if (numplayers < 2)
#endif
    shutdowncdrom();

    if(*t != 0)
    {
        setvmode(0x3);
        if(playonten == 0)
        {
            printf("Y'all come back now, ya' hear...\n");
#ifdef RRRA
            if (*t != 0)
                printf("%s%s","\n",t);
#endif
        } 
    }

    uninitgroupfile();

    unlink("redneck.tmp");

    exit(0);
}




short inputloc = 0;
short strget(short x,short y,char *t,short dalen,short c)
{
    short ch,sc;

    while(KB_KeyWaiting())
    {
        sc = 0;
        ch = KB_Getch();

        if (ch == 0)
        {

            sc = KB_Getch();
            if( sc == 104) return(1);

            continue;
        }
        else
        {
            if(ch == 8)
            {
                if( inputloc > 0 )
                {
                    inputloc--;
                    *(t+inputloc) = 0;
                }
            }
            else
            {
                if(ch == asc_Enter || sc == 104)
                {
                    KB_ClearKeyDown(sc_Enter);
                    KB_ClearKeyDown(sc_kpad_Enter);
                    return (1);
                }
                else if(ch == asc_Escape)
                {
                    KB_ClearKeyDown(sc_Escape);
                    return (-1);
                }
                else if ( ch >= 32 && inputloc < dalen && ch < 127)
                {
                    ch = toupper(ch);
                    *(t+inputloc) = ch;
                    *(t+inputloc+1) = 0;
                    inputloc++;
                }
            }
        }
    }

    if( c == 999 ) return(0);
    if( c == 998 )
    {
        char b[41],ii;
        for(ii=0;ii<inputloc;ii++)
            b[ii] = '*';
        b[ii] = 0;
        x = gametext(x,y,b,c);
    }
    else x = gametext(x,y,t,c);
    c = 4-(sintable[(totalclock<<4)&2047]>>11);

    return (0);
}
