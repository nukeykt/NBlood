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
#include "warpfx.h"

extern char everyothertime;
short which_palookup = 9;

short lastlevel = 0;
short turdlevel = 0;

short ambientlotag[64];
short ambientsprite[64];
short ambienthitag[64];
char shadedsector[MAXSECTORS];
short ambientfx;

extern int crashcnt;

void xyzmirror(short i,short wn)
{
    if (waloff[wn] == 0) loadtile(wn);
	setviewtotile(wn,tilesizy[wn],tilesizx[wn]);

	drawrooms(SX,SY,SZ,SA,100+sprite[i].shade,SECT);
	display_mirror = 1; animatesprites(SX,SY,SA,65536L); display_mirror = 0;
	drawmasks();

	setviewback();
	squarerotatetile(wn);
}

void vscrn(void)
{
     long i, j, ss, x1, x2, y1, y2, unk;

	 if(ud.screen_size < 0) ud.screen_size = 0;
	 else if(ud.screen_size > 63) ud.screen_size = 64;

     if(ud.screen_size == 0) flushperms();

     if(ud.screen_size == 12)
     {
	    ss = max(ud.screen_size-10,0);
        unk = 0;
     }
     else
	    ss = max(ud.screen_size-8,0);


     if (ud.screen_size == 12)
         x1 = scale(unk,xdim,160);
     else
	     x1 = scale(ss,xdim,160);
	 x2 = xdim-x1;

     if (ud.screen_size == 12)
         y1 = ss = 0;
     else
         y1 = ss;

	 y2 = 200;

     if ( ud.screen_size > 0 && ud.coop != 1 && ud.multimode > 1)
	 {
         j = 0;
         for(i=connecthead;i>=0;i=connectpoint2[i])
             if(i > j) j = i;

         if (j >= 1) y1 += 8;
         if (j >= 4) y1 += 8;
         if (j >= 8) y1 += 8;
         if (j >= 12) y1 += 8;
	 }

	 if (ud.screen_size == 8) y2 -= (ss+34);
     else if (ud.screen_size > 8) y2-= (ss+42);

	 y1 = scale(y1,ydim,200);
	 y2 = scale(y2,ydim,200);

	 setview(x1,y1,x2-1,y2-1);

     pub = NUMPAGES;
     pus = NUMPAGES;
}

void pickrandomspot(short snum)
{
    struct player_struct *p;
    short i;

    p = &ps[snum];

    if( ud.multimode > 1 && ud.coop != 1)
        i = TRAND%numplayersprites;
    else i = snum;

    p->bobposx = p->oposx = p->posx = po[i].ox;
    p->bobposy = p->oposy = p->posy = po[i].oy;
    p->oposz = p->posz = po[i].oz;
    p->ang = po[i].oa;
    p->cursectnum = po[i].os;
}

tloadtile(short tilenume)
{
    gotpic[tilenume>>3] |= (1<<(tilenume&7));
}


void docacheit(void)
{
    long i,j;

    j = 0;

    for(i=0;i<MAXTILES;i++)
        if( (gotpic[i>>3]&(1<<(i&7))) && waloff[i] == 0)
    {
        loadtile((short)i);
        j++;
        if((j&7) == 0) getpackets();
    }

    clearbufbyte(gotpic,sizeof(gotpic),0L);

}

extern long wupass;
void resetplayerstats(short snum)
{
    struct player_struct *p;
    short i;

    p = &ps[snum];

    ud.show_help        = 0;
    ud.showallmap       = 0;
    p->dead_flag        = 0;
    p->wackedbyactor    = -1;
    p->falling_counter  = 0;
    p->quick_kick       = 0;
    p->subweapon        = 0;
    p->last_full_weapon = 0;
    p->ftq              = 0;
    p->fta              = 0;
    p->tipincs          = 0;
    p->buttonpalette    = 0;
    p->actorsqu         =-1;
    p->invdisptime      = 0;
    p->refresh_inventory= 0;
    p->last_pissed_time = 0;
    p->holster_weapon   = 0;
    p->pycount          = 0;
    p->pyoff            = 0;
    p->opyoff           = 0;
    p->loogcnt          = 0;
    p->angvel           = 0;
    p->weapon_sway      = 0;
//    p->select_dir       = 0;
    p->extra_extra8     = 0;
    p->show_empty_weapon= 0;
    p->dummyplayersprite=-1;
    p->crack_time       = 0;
    p->hbomb_hold_delay = 0;
    p->transporter_hold = 0;
    p->wantweaponfire  = -1;
    p->hurt_delay       = 0;
#ifdef RRRA
    p->hurt_delay2      = 0;
#endif
    p->footprintcount   = 0;
    p->footprintpal     = 0;
    p->footprintshade   = 0;
    p->jumping_toggle   = 0;
    p->ohoriz = p->horiz= 140;
    p->horizoff         = 0;
    p->bobcounter       = 0;
    p->on_ground        = 0;
    p->player_par       = 0;
    p->return_to_center = 9;
    p->airleft          = 15*26;
    p->rapid_fire_hold  = 0;
    p->toggle_key_flag  = 0;
    p->access_spritenum = -1;
    if(ud.multimode > 1 && ud.coop != 1 )
        p->got_access = 7;
    else p->got_access      = 0;
    p->random_club_frame= 0;
    pus = 1;
    p->on_warping_sector = 0;
    p->spritebridge      = 0;
    p->palette = (char *) &palette[0];

    if(p->steroids_amount < 400 )
    {
        p->steroids_amount = 0;
        p->inven_icon = 0;
    }
    p->heat_on =            0;
    p->jetpack_on =         0;
    p->holoduke_on =       -1;

    p->look_ang          = 512 - ((ud.level_number&1)<<10);

    p->rotscrnang        = 0;
    p->newowner          =-1;
    p->jumping_counter   = 0;
    p->hard_landing      = 0;
    p->posxv             = 0;
    p->posyv             = 0;
    p->poszv             = 0;
    fricxv            = 0;
    fricyv            = 0;
    p->somethingonplayer =-1;
    p->one_eighty_count  = 0;
    p->cheat_phase       = 0;

    p->on_crane          = -1;

    if(p->curr_weapon == PISTOL_WEAPON)
        p->kickback_pic  = 22;
    else p->kickback_pic = 0;

    p->weapon_pos        = 6;
    p->walking_snd_toggle= 0;
    p->weapon_ang        = 0;

    p->knuckle_incs      = 1;
    p->fist_incs = 0;
    p->knee_incs         = 0;
    p->jetpack_on        = 0;
    setpal(p);
    p->at280 = 0;
#ifndef RRRA
    p->fogtype = 0;
#endif
    p->at286 = 0;
    p->at28a = 0;
    p->at28e = 0;
    p->at290 = 0;
    if (ud.multimode > 1 && ud.coop != 1)
    {
        p->keys[0] = 1;
        p->keys[1] = 1;
        p->keys[2] = 1;
        p->keys[3] = 1;
        p->keys[4] = 1;
    }
    else
    {
        p->keys[0] = 0;
        p->keys[1] = 0;
        p->keys[2] = 0;
        p->keys[3] = 0;
        p->keys[4] = 0;
    }
    wupass = 0;
    p->at582 = 0;
    p->drunkang = 1647;
    p->eatang = 1647;
    p->drink_amt = 0;
    p->eat = 0;
    p->at58e = 4096;
    p->at592 = 4096;
    p->at596 = 1;
    p->at597 = 1;
    p->at598 = 0;
    p->at599 = 0;
    p->at59a = 0;
    p->at57c = 0;
    p->at57e = 0;
    p->at59b = 0;
    p->at59d = 0;
#ifdef RRRA
    p->raat605 = 0;
    if (p->OnMotorcycle)
    {
        p->OnMotorcycle = 0;
        p->gotweapon[RA13_WEAPON] = 0;
        p->curr_weapon = RA15_WEAPON;
    }
    p->raat60b = 0;
    p->raat5b5 = 0;
    p->MotoOnGround = 1;
    p->raat5b9 = 0;
    p->MotoSpeed = 0;
    p->TiltStatus = 0;
    p->raat5c1 = 0;
    p->VBumpTarget = 0;
    p->VBumpNow  =0;
    p->raat5c7 = 0;
    p->TurbCount = 0;
    p->raat5cd = 0;
    p->raat5cf = 0;
    if (p->OnBoat)
    {
        p->OnBoat = 0;
        p->gotweapon[RA14_WEAPON] = 0;
        p->curr_weapon = RA15_WEAPON;
    }
    p->NotOnWater = 0;
    p->raat5d9 = 0;
    p->SeaSick = 0;
    p->raat5e1 = 0;
    p->nocheat = 0;
    p->DrugMode = 0;
    p->raat5f1 = 0;
    p->raat5f3 = 0;
    p->raat5f5 = 0;
    p->raat5f7 = 0;
#endif
    resetlanepics();
#ifdef RRRA
    if (numplayers < 2)
#endif
    cdromtime = 0;
#ifdef RRRA
    if (numplayers < 2)
    {
        ufospawn = 3;
        if (ufospawn > 32)
            ufospawn = 32;
        ufocnt = 0;
        hulkspawn = ud.m_player_skill + 1;
    }
    else
    {
        ufospawn = 0;
        ufocnt = 0;
        hulkspawn = 0;
    }
#else
    if (numplayers < 2)
    {
        ufospawn = ud.m_player_skill*4+1;
        if (ufospawn > 32)
            ufospawn = 32;
        ufocnt = 0;
        hulkspawn = ud.m_player_skill + 1;
    }
    else
    {
        ufospawn = 32;
        ufocnt = 0;
        hulkspawn = 2;
    }
#endif
}



void resetweapons(short snum)
{
    short  weapon;
    struct player_struct *p;

    p = &ps[snum];

    for ( weapon = PISTOL_WEAPON; weapon < MAX_WEAPONS; weapon++ )
        p->gotweapon[weapon] = 0;
    for ( weapon = PISTOL_WEAPON; weapon < MAX_WEAPONS; weapon++ )
        p->ammo_amount[weapon] = 0;

#ifdef RRRA
    p->raat605 = 0;
#endif
    p->weapon_pos = 6;
    p->kickback_pic = 5;
    p->curr_weapon = PISTOL_WEAPON;
    p->gotweapon[PISTOL_WEAPON] = 1;
    p->gotweapon[KNEE_WEAPON] = 1;
    p->ammo_amount[PISTOL_WEAPON] = 48;
#ifdef RRRA
    p->ammo_amount[KNEE_WEAPON] = 1;
    p->gotweapon[RA15_WEAPON] = 1;
    p->ammo_amount[RA15_WEAPON] = 1;
#endif
    p->gotweapon[HANDREMOTE_WEAPON] = 1;
    p->last_weapon = -1;

    p->show_empty_weapon= 0;
    p->last_pissed_time = 0;
    p->holster_weapon = 0;
#ifdef RRRA
    p->OnMotorcycle = 0;
    p->raat5b9 = 0;
    p->OnBoat = 0;
    p->raat60b = 0;
#endif
}

void resetinventory(short snum)
{
    struct player_struct *p;
    short i;

    p = &ps[snum];

    p->inven_icon       = 0;
    p->boot_amount = 0;
    p->scuba_on =           0;p->scuba_amount =         0;
    p->heat_amount        = 0;p->heat_on = 0;
    p->jetpack_on =         0;p->jetpack_amount =       0;
    p->shield_amount =      max_armour_amount;
    p->holoduke_on = -1;
    p->holoduke_amount =    0;
    p->firstaid_amount = 0;
    p->steroids_amount = 0;
    p->inven_icon = 0;

    if (ud.multimode > 1 && ud.coop != 1)
    {
        p->keys[0] = 1;
        p->keys[1] = 1;
        p->keys[2] = 1;
        p->keys[3] = 1;
        p->keys[4] = 1;
    }
    else
    {
        p->keys[0] = 0;
        p->keys[1] = 0;
        p->keys[2] = 0;
        p->keys[3] = 0;
        p->keys[4] = 0;
    }

    p->at582 = 0;
    p->drunkang = 1647;
    p->eatang = 1647;
    p->drink_amt = 0;
    p->eat = 0;
    p->at58e = 0;
    p->at592 = 0;
    p->at596 = 1;
    p->at597 = 1;
    p->at598 = 0;
    p->at599 = 0;
    p->at59a = 0;
    p->at57c = 0;
    p->at57e = 0;
    p->at59b = 0;
    p->at59d = 0;
    resetlanepics();

    if (numplayers < 2)
    {
        ufospawn = ud.m_player_skill*4+1;
        if (ufospawn > 32)
            ufospawn = 32;
        ufocnt = 0;
        hulkspawn = ud.m_player_skill + 1;
    }
    else
    {
        ufospawn = 32;
        ufocnt = 0;
        hulkspawn = 2;
    }
}


void resetprestat(short snum,char g)
{
    struct player_struct *p;
    short i;

    p = &ps[snum];

    spriteqloc = 0;
    for(i=0;i<spriteqamount;i++) spriteq[i] = -1;

    p->hbomb_on          = 0;
    p->cheat_phase       = 0;
    p->pals_time         = 0;
    p->toggle_key_flag   = 0;
    p->secret_rooms      = 0;
    p->max_secret_rooms  = 0;
    p->actors_killed     = 0;
    p->max_actors_killed = 0;
    p->lastrandomspot = 0;
    p->weapon_pos = 6;
    p->kickback_pic = 5;
    p->last_weapon = -1;
    p->weapreccnt = 0;
    p->show_empty_weapon= 0;
    p->holster_weapon = 0;
    p->last_pissed_time = 0;

    p->one_parallax_sectnum = -1;
    p->visibility = ud.const_visibility;

    screenpeek              = myconnectindex;
    numanimwalls            = 0;
    numcyclers              = 0;
    animatecnt              = 0;
    parallaxtype            = 0;
    randomseed              = 17L;
    ud.pause_on             = 0;
    ud.camerasprite         =-1;
    ud.eog                  = 0;
    tempwallptr             = 0;
    camsprite               =-1;
    earthquaketime          = 0;

#ifdef RRRA
    WindTime = 0;
    WindDir = 0;
    word_119BD8 = 0;
    word_119BE2 = 0;
    BellTime = 0;
    word_119BE0 = 0;
#endif

    numinterpolations = 0;
    startofdynamicinterpolations = 0;

    if( ( (g&MODE_EOL) != MODE_EOL && numplayers < 2) || (ud.coop != 1 && numplayers > 1) )
    {
        resetweapons(snum);
        resetinventory(snum);
    }
    else if(p->curr_weapon == HANDREMOTE_WEAPON)
    {
        p->ammo_amount[HANDBOMB_WEAPON]++;
        p->curr_weapon = HANDBOMB_WEAPON;
    }

    p->timebeforeexit   = 0;
    p->customexitsound  = 0;

    p->at280 = 0;
#ifndef RRRA
    p->fogtype = 0;
#endif
    p->at286 = 131072;
    p->at28a = 131072;
    p->at28e = 0;
    p->at290 = 0;

    if (ud.multimode > 1 && ud.coop != 1)
    {
        p->keys[0] = 1;
        p->keys[1] = 1;
        p->keys[2] = 1;
        p->keys[3] = 1;
        p->keys[4] = 1;
    }
    else
    {
        p->keys[0] = 0;
        p->keys[1] = 0;
        p->keys[2] = 0;
        p->keys[3] = 0;
        p->keys[4] = 0;
    }

    p->at582 = 0;
    p->drunkang = 1647;
    p->eatang = 1647;
    p->drink_amt = 0;
    p->eat = 0;
    p->at58e = 0;
    p->at592 = 0;
    p->at596 = 1;
    p->at597 = 1;
    p->at598 = 0;
    p->at599 = 0;
    p->at59a = 0;
    p->at57c = 0;
    p->at57e = 0;
    p->at59b = 0;
    p->at59d = 0;
    resetlanepics();

    if (numplayers < 2)
    {
        ufospawn = ud.m_player_skill*4+1;
        if (ufospawn > 32)
            ufospawn = 32;
        ufocnt = 0;
        hulkspawn = ud.m_player_skill + 1;
    }
    else
    {
        ufospawn = 32;
        ufocnt = 0;
        hulkspawn = 2;
    }

}

void setupbackdrop(short backpicnum)
{
    short i,sky;

    sky = 0;
    for(i=0;i<8;i++) pskyoff[i]=0;

    switch(backpicnum)
    {
        case MOONSKY1:
        case BIGORBIT1:
        case LA:
            sky = backpicnum;
            break;
    }

    parallaxyscale = 32768;

    switch(sky)
    {
        case MOONSKY1 :
            pskyoff[6]=1; pskyoff[1]=2; pskyoff[4]=2; pskyoff[2]=3;
            break;
        case BIGORBIT1: // orbit
            pskyoff[5]=1; pskyoff[6]=2; pskyoff[7]=3; pskyoff[2]=4;
            break;
        case LA:
            parallaxyscale = 16384+1024;
            pskyoff[0]=1; pskyoff[1]=2; pskyoff[2]=1; pskyoff[3]=3;
            pskyoff[4]=4; pskyoff[5]=0; pskyoff[6]=2; pskyoff[7]=3;
            break;
#ifdef RRRA
        default:
            if (tilesizx[backpicnum] == 512)
            {
                pskyoff[0] = 0;
                pskyoff[1] = 0;
                pskybits = 1;
                return;
            }
            if (tilesizx[backpicnum] == 1024)
            {
                pskyoff[0] = 0;
                pskybits = 0;
                return;
            }
            break;
#endif
   }

   pskybits=3;
}

void cachespritenum(short i)
{
    char maxc;
    short j;

    if(ud.monsters_off && badguy(&sprite[i])) return;

    maxc = 1;

    switch(PN)
    {
        case HYDRENT:
            tloadtile(BROKEFIREHYDRENT);
            for(j = TOILETWATER; j < (TOILETWATER+4); j++)
                if(waloff[j] == 0) tloadtile(j);
            break;
        case RRTILE2121:
        case RRTILE2122:
            if (waloff[j] == 0) tloadtile(j);
            break;
        case TOILET:
            tloadtile(TOILETBROKE);
            for(j = TOILETWATER; j < (TOILETWATER+4); j++)
                if(waloff[j] == 0) tloadtile(j);
            break;
        case STALL:
            tloadtile(STALLBROKE);
            for(j = TOILETWATER; j < (TOILETWATER+4); j++)
                if(waloff[j] == 0) tloadtile(j);
            break;
        case FORCERIPPLE:
            maxc = 9;
            break;
        case RUBBERCAN:
            maxc = 2;
            break;
        case TOILETWATER:
            maxc = 4;
            break;
        case BUBBASTAND:
            for(j = BUBBASCRATCH; j <= (BUBBASCRATCH+47); j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            maxc = 0;
            break;
#ifdef RRRA
        case SBSWIPE:
            for(j = SBSWIPE; j <= (SBSWIPE+29); j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            maxc = 0;
            break;

#endif
        case COOT:
            for(j = COOT; j <= (COOT+217); j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            for(j=COOTJIBA;j<COOTJIBC+4;j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            maxc = 0;
            break;
        case LTH:
            maxc = 105;
            for(j = LTH; j < (LTH+maxc); j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            maxc = 0;
            break;
        case BILLYRAY:
            maxc = 144;
            for(j = BILLYWALK; j < (BILLYWALK+maxc); j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            for(j=BILLYJIBA;j<=BILLYJIBB+4;j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            maxc = 0;
            break;
        case COW:
            maxc = 56;
            for(j = PN; j < (PN+maxc); j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            maxc = 0;
            break;
        case DOGRUN:
            for(j = DOGATTACK; j <= DOGATTACK+35; j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            for(j = DOGRUN; j <= DOGRUN+80; j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            maxc = 0;
            break;
#ifdef RRRA
        case RABBIT:
            for(j = RABBIT; j <= RABBIT+54; j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            for(j = RABBIT+56; j <= RABBIT+56+49; j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            for(j = RABBIT+56; j <= RABBIT+56+49; j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            maxc = 0;
            break;
        case BIKERB:
        case BIKERBV2:
            for(j = BIKERB; j <= BIKERB+104; j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            maxc = 0;
            break;
        case BIKER:
            for(j = BIKER; j <= BIKER+116; j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            for(j = BIKER+150; j <= BIKER+150+104; j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            maxc = 0;
            break;
        case CHEER:
            for(j = CHEER; j <= CHEER+44; j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            for(j = CHEER+47; j <= CHEER+47+211; j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            for(j = CHEER+262; j <= CHEER+262+72; j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            maxc = 0;
            break;
        case CHEERB:
            for(j = CHEERB; j <= CHEERB+83; j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            for(j = CHEERB+157; j <= CHEERB+157+83; j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            maxc = 0;
            break;
        case MAMA:
            for(j = MAMA; j <= MAMA+78; j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            for(j = MAMA+80; j <= MAMA+80+7; j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            for(j = MAMA+90; j <= MAMA+90+94; j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            maxc = 0;
            break;
        case CHEERBOAT:
            if (waloff[CHEERBOAT] == 0) tloadtile(CHEERBOAT);
            maxc = 0;
            break;
        case HULKBOAT:
            if (waloff[HULKBOAT] == 0) tloadtile(HULKBOAT);
            maxc = 0;
            break;
        case MINIONBOAT:
            if (waloff[MINIONBOAT] == 0) tloadtile(MINIONBOAT);
            maxc = 0;
            break;
        case BILLYPLAY:
            for(j = BILLYPLAY; j <= BILLYPLAY+2; j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            maxc = 0;
            break;
        case COOTPLAY:
            for(j = COOTPLAY; j <= COOTPLAY+4; j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            maxc = 0;
            break;
#endif
        case PIG:
        case PIGSTAYPUT:
            maxc = 68;
            break;
        case TORNADO:
            maxc = 7;
            break;
        case HEN:
        case HENSTAND:
            maxc = 34;
            break;
        case APLAYER:
            maxc = 0;
            if(ud.multimode > 1)
            {
                maxc = 5;
                for(j = APLAYER;j < APLAYER+220; j++)
                    if(waloff[j] == 0)
                        tloadtile(j);
                for(j = DUKEGUN;j < DUKELEG+4; j++)
                    if(waloff[j] == 0)
                        tloadtile(j);
            }
            break;
        case ATOMICHEALTH:
            maxc = 14;
            break;
        case DRONE:
            maxc = 6;
            break;
        case EXPLODINGBARREL:
        case SEENINE:
        case OOZFILTER:
            maxc = 3;
            break;
        case NUKEBARREL:
        case CAMERA1:
            maxc = 5;
            break;
        case VIXEN:
            maxc = 214;
            for(j = PN; j < PN+maxc; j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            maxc = 0;
            break;
#ifndef RRRA
        case SBMOVE:
            maxc = 54;
            for(j = PN; j < PN+maxc; j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            maxc = 100;
            for(j = SBMOVE; j < SBMOVE+maxc; j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            maxc = 0;
            break;
#endif
        case HULK:
            maxc = 40;
            for(j = PN-41; j < PN+maxc-41; j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            for(j = HULKJIBA; j <= HULKJIBC+4; j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            maxc = 0;
            break;
        case MINION:
            maxc = 141;
            for(j = PN; j < PN+maxc; j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            for(j = MINJIBA; j <= MINJIBC+4; j++)
                if(waloff[j] == 0)
                    tloadtile(j);
            maxc = 0;
            break;


    }

    for(j = PN; j < (PN+maxc); j++)
        if(waloff[j] == 0)
            tloadtile(j);
}

void cachegoodsprites(void)
{
    short i;

    if(ud.screen_size >= 8)
    {
        if(waloff[BOTTOMSTATUSBAR] == 0)
            tloadtile(BOTTOMSTATUSBAR);
        if( ud.multimode > 1)
        {
            if(waloff[FRAGBAR] == 0)
                tloadtile(FRAGBAR);
            for(i=MINIFONT;i<MINIFONT+63;i++)
                if(waloff[i] == 0)
                    tloadtile(i);
        }
    }

    //tloadtile(VIEWSCREEN);

    for(i=STARTALPHANUM;i<ENDALPHANUM+1;i++)
        if (waloff[i] == 0)
            tloadtile(i);

    for(i=FOOTPRINTS;i<FOOTPRINTS+3;i++)
        if (waloff[i] == 0)
            tloadtile(i);

    for( i = BIGALPHANUM; i < BIGALPHANUM+82; i++)
        if(waloff[i] == 0)
            tloadtile(i);

    for( i = BURNING; i < BURNING+14; i++)
        if(waloff[i] == 0)
            tloadtile(i);

    for( i = FIRSTGUN; i < FIRSTGUN+10 ; i++ )
        if(waloff[i] == 0)
            tloadtile(i);

    for( i = EXPLOSION2; i < EXPLOSION2+21 ; i++ )
        if(waloff[i] == 0)
            tloadtile(i);

    tloadtile(BULLETHOLE);

    for( i = SHOTGUN; i < SHOTGUN+8 ; i++ )
        if(waloff[i] == 0)
            tloadtile(i);

    tloadtile(FOOTPRINTS);

    for( i = JIBS1; i < (JIBS5+5); i++)
        if(waloff[i] == 0)
            tloadtile(i);

    for( i = SCRAP1; i < (SCRAP1+19); i++)
        if(waloff[i] == 0)
            tloadtile(i);

    for( i = SMALLSMOKE; i < (SMALLSMOKE+4); i++)
        if(waloff[i] == 0)
            tloadtile(i);

#ifdef RRRA
    if (ud.volume_number == 0 && ud.level_number == 4)
    {
        if(waloff[RRTILE2577] == 0)
            tloadtile(RRTILE2577);
    }
#else
    if (ud.volume_number == 1 && ud.level_number == 2)
    {
        if(waloff[RRTILE3190] == 0)
            tloadtile(RRTILE3190);
        if(waloff[RRTILE3191] == 0)
            tloadtile(RRTILE3191);
        if(waloff[RRTILE3192] == 0)
            tloadtile(RRTILE3192);
        if(waloff[RRTILE3144] == 0)
            tloadtile(RRTILE3144);
        if(waloff[RRTILE3139] == 0)
            tloadtile(RRTILE3139);
        if(waloff[RRTILE3132] == 0)
            tloadtile(RRTILE3132);
        if(waloff[RRTILE3120] == 0)
            tloadtile(RRTILE3120);
        if(waloff[RRTILE3121] == 0)
            tloadtile(RRTILE3121);
        if(waloff[RRTILE3122] == 0)
            tloadtile(RRTILE3122);
        if(waloff[RRTILE3123] == 0)
            tloadtile(RRTILE3123);
        if(waloff[RRTILE3124] == 0)
            tloadtile(RRTILE3124);
    }
#endif
    if (lastlevel)
    {
        i = UFO1;
        if(waloff[i] == 0)
            tloadtile(i);
        i = UFO2;
        if (waloff[i] == 0)
            tloadtile(i);
        i = UFO3;
        if (waloff[i] == 0)
            tloadtile(i);
        i = UFO4;
        if (waloff[i] == 0)
            tloadtile(i);
        i = UFO5;
        if (waloff[i] == 0)
            tloadtile(i);
    }
}

void prelevel(char g)
{
#if 1
    struct player_struct *p;
    short i;
    short nexti;
    short j;
    short startwall;
    short endwall;
    short lotaglist;
    short k;
    short lotags[65];
    int speed;
    int dist;
    short sound;
    sound = 0;

    p = &ps[screenpeek];
#else
    short i, nexti, j, startwall, endwall, lotaglist, k;
    short lotags[65];
    long p2;
    struct player_struct *p;
    short p3;
    long p1;
    p3 = 0;

    p = &ps[screenpeek];
#endif

#ifdef RRRA
    sub_86730(0);
    p->fogtype = 0;
    p->raat5dd = 0;
    p->raat5fd = 0;
    p->raat601 = 0;
    p->SlotWin = 0;
    p->raat607 = 0;
    p->raat609 = 0;
    word_119BDA = 15;
    word_119BDC = 0;
    word_119BE2 = 0;
    if (ud.level_number == 3 && ud.volume_number == 0)
        word_119BDA = 5;
    else if (ud.level_number == 2 && ud.volume_number == 1)
        word_119BDA = 10;
    else if (ud.level_number == 6 && ud.volume_number == 1)
        word_119BDA = 15;
    else if (ud.level_number == 4 && ud.volume_number == 1)
        ps[myconnectindex].steroids_amount = 0;
#endif

    clearbufbyte(show2dsector,sizeof(show2dsector),0L);
    clearbufbyte(show2dwall,sizeof(show2dwall),0L);
    clearbufbyte(show2dsprite,sizeof(show2dsprite),0L);

    for (i = 0; i < MAXSECTORS; i++)
        shadedsector[i] = 0;

    for (i = 0; i < 64; i++)
    {
        geosectorwarp[i] = -1;
        geosectorwarp2[i] = -1;
    }
    
    for (i = 0; i < 64; i++)
    {
        ambienthitag[i] = -1;
        ambientlotag[i] = -1;
        ambientsprite[i] = -1;
    }

    resetprestat(0,g);
    lightnincnt = 0;
    torchcnt = 0;
    geocnt = 0;
    jaildoorcnt = 0;
    minecartcnt = 0;
    ambientfx = 0;
    crashcnt = 0;
    thunderon = 0;
    chickenplant = 0;
#ifdef RRRA
    WindTime = 0;
    WindDir = 0;
    word_119BD8 = 0;
    word_119BE2 = 0;
    word_119BDA = 15;
    BellTime = 0;
    word_119BE0 = 0;

    for (j = 0; j < MAXSPRITES; j++)
    {
        if (sprite[j].pal == 100)
        {
            if (numplayers > 1)
                deletesprite(j);
            else
                sprite[j].pal = 0;
        }
        else if (sprite[j].pal == 101)
        {
            sprite[j].extra = 0;
            sprite[j].hitag = 1;
            sprite[j].pal = 0;
            changespritestat(j,118);
        }
    }
#endif

    for(i=0;i<numsectors;i++)
    {
        if (sector[i].ceilingpicnum == RRTILE2577)
            thunderon = 1;
        sector[i].extra = 256;

        switch(sector[i].lotag)
        {
            case 41:
                k = headspritesect[i];
                while (k != -1)
                {
                    nexti = nextspritesect[k];
                    if (sprite[k].picnum == RRTILE11)
                    {
                        dist = sprite[k].lotag<<4;
                        speed = sprite[k].hitag;
                        deletesprite(k);
                    }
                    if (sprite[k].picnum == RRTILE38)
                    {
                        sound = sprite[k].lotag;
                        deletesprite(k);
                    }
                    k = nexti;
                }
                for(j=0;j<numsectors;j++)
                {
                    if (sector[i].hitag == sector[j].hitag && j != i)
                    {
                        if (jaildoorcnt > 32)
                            gameexit("\nToo many jaildoor sectors");
                        jaildoordist[jaildoorcnt] = dist;
                        jaildoorspeed[jaildoorcnt] = speed;
                        jaildoorsecthtag[jaildoorcnt] = sector[i].hitag;
                        jaildoorsect[jaildoorcnt] = j;
                        jaildoordrag[jaildoorcnt] = 0;
                        jaildooropen[jaildoorcnt] = 0;
                        jaildoordir[jaildoorcnt] = sector[j].lotag;
                        jaildoorsound[jaildoorcnt] = sound;
                        jaildoorcnt++;
                    }
                }
                break;
            case 42:
            {
                short ii;
                k = headspritesect[i];
                while (k != -1)
                {
                    nexti = nextspritesect[k];
                    if (sprite[k].picnum == RRTILE64)
                    {
                        dist = sprite[k].lotag<<4;
                        speed = sprite[k].hitag;
                        for (ii = 0; ii < MAXSPRITES; ii++)
                        {
                            if (sprite[ii].picnum == RRTILE66)
                                if (sprite[ii].lotag == sprite[k].sectnum)
                                {
                                    minecartchildsect[minecartcnt] = sprite[ii].sectnum;
                                    deletesprite(ii);
                                }
                        }
                        deletesprite(k);
                    }
                    if (sprite[k].picnum == RRTILE65)
                    {
                        sound = sprite[k].lotag;
                        deletesprite(k);
                    }
                    k = nexti;
                }
                if (minecartcnt > 16)
                    gameexit("\nToo many minecart sectors");
                minecartdist[minecartcnt] = dist;
                minecartspeed[minecartcnt] = speed;
                minecartsect[minecartcnt] = i;
                minecartdir[minecartcnt] = sector[i].hitag;
                minecartdrag[minecartcnt] = dist;
                minecartopen[minecartcnt] = 1;
                minecartsound[minecartcnt] = sound;
                minecartcnt++;
                break;
            }
            case 20:
            case 22:
                if( sector[i].floorz > sector[i].ceilingz)
                    sector[i].lotag |= 32768;
                continue;
        }

        if(sector[i].ceilingstat&1)
        {
            if(waloff[sector[i].ceilingpicnum] == 0)
            {
                if(sector[i].ceilingpicnum == LA)
                    for(j=0;j<5;j++)
                        if(waloff[sector[i].ceilingpicnum+j] == 0)
                            tloadtile(sector[i].ceilingpicnum+j);
            }
            setupbackdrop(sector[i].ceilingpicnum);

            if(ps[0].one_parallax_sectnum == -1)
                ps[0].one_parallax_sectnum = i;
        }

        if(sector[i].lotag == 32767) //Found a secret room
        {
            ps[0].max_secret_rooms++;
            continue;
        }

        if(sector[i].lotag == -1)
        {
            ps[0].exitx = wall[sector[i].wallptr].x;
            ps[0].exity = wall[sector[i].wallptr].y;
            continue;
        }
    }

    i = headspritestat[0];
    while(i >= 0)
    {
        nexti = nextspritestat[i];

        if(sprite[i].lotag == -1 && (sprite[i].cstat&16) )
        {
            ps[0].exitx = SX;
            ps[0].exity = SY;
        }
        else switch(PN)
        {
            case NUKEBUTTON:
                chickenplant = 1;
                break;

            case GPSPEED:
                sector[SECT].extra = SLT;
                deletesprite(i);
                break;

            case CYCLER:
                if(numcyclers >= MAXCYCLERS)
                    gameexit("\nToo many cycling sectors.");
                cyclers[numcyclers][0] = SECT;
                cyclers[numcyclers][1] = SLT;
                cyclers[numcyclers][2] = SS;
                cyclers[numcyclers][3] = sector[SECT].floorshade;
                cyclers[numcyclers][4] = SHT;
                cyclers[numcyclers][5] = (SA == 1536);
                numcyclers++;
                deletesprite(i);
                break;

            case RRTILE18:
                if (torchcnt > 64)
                    gameexit("\nToo many torch effects");
                torchsector[torchcnt] = SECT;
                torchsectorshade[torchcnt] = sector[SECT].floorshade;
                torchtype[torchcnt] = SLT;
                torchcnt++;
                deletesprite(i);
                break;

            case RRTILE35:
                if (lightnincnt > 64)
                    gameexit("\nToo many lightnin effects");
                lightninsector[lightnincnt] = SECT;
                lightninsectorshade[lightnincnt] = sector[SECT].floorshade;
                lightnincnt++;
                deletesprite(i);
                break;

            case RRTILE68:
                shadedsector[SECT] = 1;
                deletesprite(i);
                break;

            case RRTILE67:
                sprite[i].cstat |= 32768;
                break;

            case SOUNDFX:
                if (ambientfx >= 64)
                    gameexit("\nToo many ambient effects");
                else
                {
                    ambienthitag[ambientfx] = SHT;
                    ambientlotag[ambientfx] = SLT;
                    ambientsprite[ambientfx] = i;
                    sprite[i].ang = ambientfx;
                    ambientfx++;
                    sprite[i].lotag = 0;
                    sprite[i].hitag = 0;
                }
                break;
        }
        i = nexti;
    }

    for(i=0;i<MAXSPRITES;i++)
    {
        if (sprite[i].picnum == RRTILE19)
        {
            if (geocnt > 64)
                gameexit("\nToo many geometry effects");
            if (sprite[i].hitag == 0)
            {
                geosector[geocnt] = sprite[i].sectnum;
                for(j=0;j<MAXSPRITES;j++)
                {
                    if (sprite[i].lotag == sprite[j].lotag && j != i && sprite[j].picnum == RRTILE19)
                    {
                        if (sprite[j].hitag == 1)
                        {
                            geosectorwarp[geocnt] = sprite[j].sectnum;
                            geox[geocnt] = sprite[i].x-sprite[j].x;
                            geoy[geocnt] = sprite[i].y-sprite[j].y;
                            geoz[geocnt] = sprite[i].z-sprite[j].z;
                        }
                        if (sprite[j].hitag == 2)
                        {
                            geosectorwarp2[geocnt] = sprite[j].sectnum;
                            geox2[geocnt] = sprite[i].x-sprite[j].x;
                            geoy2[geocnt] = sprite[i].y-sprite[j].y;
                            geoz2[geocnt] = sprite[i].z-sprite[j].z;
                        }
                    }
                }
                geocnt++;
            }
        }
    }

    for(i=0;i < MAXSPRITES;i++)
    {
        if(sprite[i].statnum < MAXSTATUS)
        {
            if(PN == SECTOREFFECTOR && SLT == 14)
                continue;
            spawn(-1,i);
        }
    }

    for(i=0;i < MAXSPRITES;i++)
    {
        if(sprite[i].statnum < MAXSTATUS)
        {
            if( PN == SECTOREFFECTOR && SLT == 14 )
                spawn(-1,i);
        }
        if (sprite[i].picnum == RRTILE19)
            deletesprite(i);
        if (sprite[i].picnum == RRTILE34)
        {
            sector[sprite[i].sectnum].filler = sprite[i].lotag;
            deletesprite(i);
        }
    }

    lotaglist = 0;

    i = headspritestat[0];
    while(i >= 0)
    {
        switch(PN)
        {
            case DIPSWITCH:
            case DIPSWITCH2:
            case ACCESSSWITCH:
            case PULLSWITCH:
            case HANDSWITCH:
            case SLOTDOOR:
            case LIGHTSWITCH:
            case SPACELIGHTSWITCH:
            case SPACEDOORSWITCH:
            case FRANKENSTINESWITCH:
            case LIGHTSWITCH2:
            case POWERSWITCH1:
            case LOCKSWITCH1:
            case POWERSWITCH2:
#ifdef RRRA
            case RRTILE8464:
#endif
                break;
            case DIPSWITCH+1:
            case DIPSWITCH2+1:
            case PULLSWITCH+1:
            case HANDSWITCH+1:
            case SLOTDOOR+1:
            case LIGHTSWITCH+1:
            case SPACELIGHTSWITCH+1:
            case SPACEDOORSWITCH+1:
            case FRANKENSTINESWITCH+1:
            case LIGHTSWITCH2+1:
            case POWERSWITCH1+1:
            case LOCKSWITCH1+1:
            case POWERSWITCH2+1:
            case NUKEBUTTON:
            case NUKEBUTTON+1:
#ifdef RRRA
            case RRTILE8464+1:
#endif
                for(j=0;j<lotaglist;j++)
                    if( SLT == lotags[j] )
                        break;

                if( j == lotaglist )
                {
                    lotags[lotaglist] = SLT;
                    lotaglist++;
                    if(lotaglist > 64)
                        gameexit("\nToo many switches (64 max).");

                    j = headspritestat[3];
                    while(j >= 0)
                    {
                        if(sprite[j].lotag == 12 && sprite[j].hitag == SLT)
                            hittype[j].temp_data[0] = 1;
                        j = nextspritestat[j];
                    }
                }
                break;
        }
        i = nextspritestat[i];
    }

    mirrorcnt = 0;

    for( i = 0; i < numwalls; i++ )
    {
        walltype *wal;
        wal = &wall[i];

        if(wal->overpicnum == MIRROR && (wal->cstat&32) != 0)
        {
            j = wal->nextsector;

            if(mirrorcnt > 63)
                gameexit("\nToo many mirrors (64 max.)");
            if ( (j >= 0) && sector[j].ceilingpicnum != MIRROR )
            {
                sector[j].ceilingpicnum = MIRROR;
                sector[j].floorpicnum = MIRROR;
                mirrorwall[mirrorcnt] = i;
                mirrorsector[mirrorcnt] = j;
                mirrorcnt++;
                continue;
            }
        }

        if(numanimwalls >= MAXANIMWALLS)
            gameexit("\nToo many 'anim' walls (max 512.)");

        animwall[numanimwalls].tag = 0;
        animwall[numanimwalls].wallnum = 0;

        switch(wal->overpicnum)
        {
            case FANSPRITE:
                wall->cstat |= 65;
                animwall[numanimwalls].wallnum = i;
                numanimwalls++;
                break;
            case BIGFORCE:
                animwall[numanimwalls].wallnum = i;
                numanimwalls++;
                continue;
        }

        wal->extra = -1;

        switch(wal->picnum)
        {
            case WATERTILE2:
                for(j=0;j<3;j++)
                    if(waloff[wal->picnum+j] == 0)
                        tloadtile(wal->picnum+j);
                break;

            case RRTILE1814:
            case RRTILE1817:
                if(waloff[wal->picnum] == 0)
                    tloadtile(wal->picnum);
                break;
            case RRTILE1939:
            case RRTILE1986:
            case RRTILE1987:
            case RRTILE1988:
            case RRTILE2004:
            case RRTILE2005:
            case RRTILE2123:
            case RRTILE2124:
            case RRTILE2125:
            case RRTILE2126:
            case RRTILE2636:
            case RRTILE2637:
            case RRTILE2878:
            case RRTILE2879:
            case RRTILE2898:
            case RRTILE2899:
                if(waloff[wal->picnum] == 0)
                    tloadtile(wal->picnum);
                break;
            case TECHLIGHT2:
            case TECHLIGHT4:
                if(waloff[wal->picnum] == 0)
                    tloadtile(wal->picnum);
                break;
            case SCREENBREAK6:
            case SCREENBREAK7:
            case SCREENBREAK8:
                if(waloff[SCREENBREAK6] == 0)
                    for(j=SCREENBREAK6;j<=SCREENBREAK8;j++)
                        tloadtile(j);
                animwall[numanimwalls].wallnum = i;
                animwall[numanimwalls].tag = -1;
                numanimwalls++;
                break;
        }
    }

    //Invalidate textures in sector behind mirror
    for(i=0;i<mirrorcnt;i++)
    {
        startwall = sector[mirrorsector[i]].wallptr;
        endwall = startwall + sector[mirrorsector[i]].wallnum;
        for(j=startwall;j<endwall;j++)
        {
            wall[j].picnum = MIRROR;
            wall[j].overpicnum = MIRROR;
        }
    }
    if (!thunderon)
    {
        char brightness = ud.brightness>>2;
        setbrightness(brightness,palette);
        visibility = p->visibility;
    }
    tilesizx[0] = tilesizy[0] = 0;
}

void newgame(char vn,char ln,char sk)
{
    struct player_struct *p = &ps[0];
    short i;

    ready2send = 0;
    waitforeverybody();

    if( ud.m_recstat != 2 && ud.last_level >= 0 && ud.multimode > 1 && ud.coop != 1)
#ifdef RRRA
    {
        if (playerswhenstarted > 1 || numplayers > 1)
            dobonus(1);
        else
            dobonus2(1);
    }
#else
        dobonus(1);
    if (turdlevel && !lastlevel)
        dobonus(0);
#endif

    show_shareware = 26*34;

    ud.level_number =   ln;
    ud.volume_number =  vn;
    ud.player_skill =   sk;
    ud.secretlevel =    0;
    ud.from_bonus = 0;

    ud.last_level = -1;
    lastsavedpos = -1;
    p->zoom            = 768;
    p->gm              = 0;

    if(ud.m_coop != 1)
    {
        p->curr_weapon = PISTOL_WEAPON;
        p->gotweapon[PISTOL_WEAPON] = 1;
        p->gotweapon[KNEE_WEAPON] = 1;
        p->ammo_amount[PISTOL_WEAPON] = 48;
#ifdef RRRA
        p->gotweapon[RA15_WEAPON] = 1;
        p->ammo_amount[RA15_WEAPON] = 1;
        p->ammo_amount[KNEE_WEAPON] = 1;
#endif
        p->last_weapon = -1;
    }

    display_mirror =        0;

    if(ud.multimode > 1 )
    {
        if(numplayers < 2)
        {
            connecthead = 0;
            for(i=0;i<MAXPLAYERS;i++) connectpoint2[i] = i+1;
            connectpoint2[ud.multimode-1] = -1;
        }
    }
    else
    {
        connecthead = 0;
        connectpoint2[0] = -1;
    }
}


void resetpspritevars(char g)
{
    short i, j, nexti,circ;
    long firstx,firsty;
    spritetype *s;
    char aimmode[MAXPLAYERS];
    STATUSBARTYPE tsbar[MAXPLAYERS];

    EGS(ps[0].cursectnum,ps[0].posx,ps[0].posy,ps[0].posz,
        APLAYER,0,0,0,ps[0].ang,0,0,0,10);

    if(ud.recstat != 2) for(i=0;i<MAXPLAYERS;i++)
    {
        aimmode[i] = ps[i].aim_mode;
        if(ud.multimode > 1 && ud.coop == 1 && ud.last_level >= 0)
        {
            for(j=0;j<MAX_WEAPONS;j++)
            {
                tsbar[i].ammo_amount[j] = ps[i].ammo_amount[j];
                tsbar[i].gotweapon[j] = ps[i].gotweapon[j];
            }

            tsbar[i].shield_amount = ps[i].shield_amount;
            tsbar[i].curr_weapon = ps[i].curr_weapon;
            tsbar[i].inven_icon = ps[i].inven_icon;

            tsbar[i].firstaid_amount = ps[i].firstaid_amount;
            tsbar[i].steroids_amount = ps[i].steroids_amount;
            tsbar[i].holoduke_amount = ps[i].holoduke_amount;
            tsbar[i].jetpack_amount = ps[i].jetpack_amount;
            tsbar[i].heat_amount = ps[i].heat_amount;
            tsbar[i].scuba_amount = ps[i].scuba_amount;
            tsbar[i].boot_amount = ps[i].boot_amount;
        }
    }

    resetplayerstats(0);

    for(i=1;i<MAXPLAYERS;i++)
       memcpy(&ps[i],&ps[0],sizeof(ps[0]));

    if(ud.recstat != 2) for(i=0;i<MAXPLAYERS;i++)
    {
        ps[i].aim_mode = aimmode[i];
        if(ud.multimode > 1 && ud.coop == 1 && ud.last_level >= 0)
        {
            for(j=0;j<MAX_WEAPONS;j++)
            {
                ps[i].ammo_amount[j] = tsbar[i].ammo_amount[j];
                ps[i].gotweapon[j] = tsbar[i].gotweapon[j];
            }
            ps[i].shield_amount = tsbar[i].shield_amount;
            ps[i].curr_weapon = tsbar[i].curr_weapon;
            ps[i].inven_icon = tsbar[i].inven_icon;

            ps[i].firstaid_amount = tsbar[i].firstaid_amount;
            ps[i].steroids_amount= tsbar[i].steroids_amount;
            ps[i].holoduke_amount = tsbar[i].holoduke_amount;
            ps[i].jetpack_amount = tsbar[i].jetpack_amount;
            ps[i].heat_amount = tsbar[i].heat_amount;
            ps[i].scuba_amount= tsbar[i].scuba_amount;
            ps[i].boot_amount = tsbar[i].boot_amount;
        }
    }

    numplayersprites = 0;
    circ = 2048/ud.multimode;

    which_palookup = 9;
    j = connecthead;
    i = headspritestat[10];
    while(i >= 0)
    {
        nexti = nextspritestat[i];
        s = &sprite[i];

        if( numplayersprites == MAXPLAYERS)
            gameexit("\nToo many player sprites (max 8.)");

        if(numplayersprites == 0)
        {
            firstx = ps[0].posx;
            firsty = ps[0].posy;
        }

        po[numplayersprites].ox = s->x;
        po[numplayersprites].oy = s->y;
        po[numplayersprites].oz = s->z;
        po[numplayersprites].oa = s->ang;
        po[numplayersprites].os = s->sectnum;

        numplayersprites++;
        if(j >= 0)
        {
            s->owner = i;
            s->shade = 0;
            s->xrepeat = 24;
            s->yrepeat = 17;
            s->cstat = 1+256;
            s->xoffset = 0;
            s->clipdist = 64;

            if( (g&MODE_EOL) != MODE_EOL || ps[j].last_extra == 0)
            {
                ps[j].last_extra = max_player_health;
                s->extra = max_player_health;
            }
            else s->extra = ps[j].last_extra;

            s->yvel = j;

            if (ud.last_level == -1)
            {
                if(s->pal == 0)
                {
                    s->pal = ps[j].palookup = which_palookup;
                    ud.user_pals[j] = which_palookup;
                    which_palookup++;
                    if( which_palookup == 17 ) which_palookup = 9;
                }
                else ud.user_pals[j] = ps[j].palookup = s->pal;
            }
            else
                s->pal = ps[j].palookup = ud.user_pals[j];

            ps[j].i = i;
            ps[j].frag_ps = j;
            hittype[i].owner = i;

            hittype[i].bposx = ps[j].bobposx = ps[j].oposx = ps[j].posx =        s->x;
            hittype[i].bposy = ps[j].bobposy = ps[j].oposy = ps[j].posy =        s->y;
            hittype[i].bposz = ps[j].oposz = ps[j].posz =        s->z;
            ps[j].oang  = ps[j].ang  =        s->ang;

            updatesector(s->x,s->y,&ps[j].cursectnum);

            j = connectpoint2[j];

        }
        else deletesprite(i);
        i = nexti;
    }
}

void clearfrags(void)
{
    short i;

    for(i = 0;i<MAXPLAYERS;i++)
        ps[i].frag = ps[i].fraggedself = 0;
     clearbufbyte(&frags[0][0],(MAXPLAYERS*MAXPLAYERS)<<1,0L);
}

void resettimevars(void)
{
    vel = svel = angvel = horiz = 0;

    totalclock = 0L;
    ototalclock = 0L;
    lockclock = 0L;
    ready2send = 1;
}


void genspriteremaps(void)
{
    long j,fp;
    signed char look_pos;
    char *lookfn = "lookup.dat";
    char numl;
    char table[768];
    short unk;

    fp = kopen4load(lookfn,loadfromgrouponly);
    if(fp != -1)
        kread(fp,(char *)&numl,1);
    else
        gameexit("\nERROR: File 'LOOKUP.DAT' not found.");

    for(j=0;j < numl;j++)
    {
        kread(fp,(signed char *)&look_pos,1);
        kread(fp,tempbuf,256);
        makepalookup((long)look_pos,tempbuf,0,0,0,1);
#ifdef RRRA
        if (look_pos == 8)
            makepalookup(54,tempbuf,32,32,32,1);
#endif
    }

    kread(fp,&waterpal[0],768);
    kread(fp,&slimepal[0],768);
    kread(fp,&titlepal[0],768);
    kread(fp,&drealms[0],768);
    kread(fp,&endingpal[0],768);

    palette[765] = palette[766] = palette[767] = 0;
    slimepal[765] = slimepal[766] = slimepal[767] = 0;
    waterpal[765] = waterpal[766] = waterpal[767] = 0;

    kclose(fp);

    for (j = 0; j < 768; j++)
        table[j] = j;
    for (j = 0; j < 32; j++)
        table[j] = j + 32;

    makepalookup(7,table,0,0,0,1);

    for (j = 0; j < 768; j++)
        table[j] = j;
    makepalookup(30,table,0,0,0,1);
    makepalookup(31,table,0,0,0,1);
    makepalookup(32,table,0,0,0,1);
    makepalookup(33,table,0,0,0,1);
#ifdef RRRA
    makepalookup(105,table,0,0,0,1);
#endif
    unk = 63;
    for (j = 64; j < 80; j++)
    {
        unk--;
        table[j] = unk;
        table[j + 16] = j - 24;
    }
    table[80] = 80;
    table[81] = 81;
    for (j = 0; j < 32; j++)
    {
        table[j] = j + 32;
    }
    makepalookup(34,table,0,0,0,1);

    for (j = 0; j < 768; j++)
        table[j] = j;
    for (j = 0; j < 16; j++)
        table[j] = j + 129;
    for (j = 16; j < 32; j++)
        table[j] = j + 192;
    makepalookup(35,table,0,0,0,1);
}

char waitabort;
void waitforeverybody()
{
    long i;

    if (numplayers < 2) return;
    waitabort = 0;

    packbuf[0] = 250;
    for(i=connecthead;i>=0;i=connectpoint2[i])
        if (i != myconnectindex)
            sendpacket(i,packbuf,1);

    playerreadyflag[myconnectindex]++;
    do
    {
        if (KB_KeyPressed(sc_Escape)) { waitabort = 1; return; }
        getpackets();
        for(i=connecthead;i>=0;i=connectpoint2[i])
            if (playerreadyflag[i] < playerreadyflag[myconnectindex]) break;
    } while (i >= 0);
}

char checksum(long sum)
{

    long i, j, delaytotalclock;

    delaytotalclock = totalclock;

    tempbuf[0] = 9;
    tempbuf[1] = sum&0x000000ff;
    tempbuf[2] = (sum&0x0000ff00)>>8;
    tempbuf[3] = (sum&0x00ff0000)>>16;
    tempbuf[4] = (sum&0x7f000000)>>24;

    if(connectpoint2[myconnectindex] < 0)
        i = connecthead;
    else
        i = connectpoint2[myconnectindex];

    sendpacket(i,tempbuf,5);

    checksume = -1;

    while(checksume == -1)
        if(totalclock >= delaytotalclock)
        {
            getpackets();
            delaytotalclock = totalclock + 8;
        }

    return (sum != checksume);
}

char getsound(unsigned short num)
{
    short fp;
    long   l;

    if(num >= NUM_SOUNDS || SoundToggle == 0) return 0;
    if (FXDevice == NumSoundCards) return 0;

    fp = kopen4load(sounds[num],loadfromgrouponly);
    if(fp == -1) return 0;

    l = kfilelength( fp );
    soundsiz[num] = l;

    if( (ud.level_number == 0 && ud.volume_number == 0 && (num == 189 || num == 232 || num == 99 || num == 233 || num == 17 ) ) ||
        ( l < 12800 ) )
    {
        Sound[num].lock = 2;
        allocache((long *)&Sound[num].ptr,l,&Sound[num].lock);
        if(Sound[num].ptr != NULL)
            kread( fp, Sound[num].ptr , l);
    }
    kclose( fp );
    return 1;
}

void precachenecessarysounds(void)
{
    short i;

    if (FXDevice == NumSoundCards) return;

    for(i=0;i<NUM_SOUNDS;i++)
    {
        SoundOwner[i][0].i = -1;
        SoundOwner[i][1].i = -1;
        SoundOwner[i][2].i = -1;
        SoundOwner[i][3].i = -1;
        if(Sound[i].ptr == 0)
            getsound(i);
    }
}


void cacheit(void)
{
    short i,j;

    precachenecessarysounds();

    cachegoodsprites();

    for(i=0;i<numwalls;i++)
        if( waloff[wall[i].picnum] == 0 )
    {
        if(waloff[wall[i].picnum] == 0)
            tloadtile(wall[i].picnum);
        if(wall[i].overpicnum >= 0 && waloff[wall[i].overpicnum] == 0 )
            tloadtile(wall[i].overpicnum);
    }

    for(i=0;i<numsectors;i++)
    {
        if( waloff[sector[i].floorpicnum] == 0 )
            tloadtile( sector[i].floorpicnum );
        if( waloff[sector[i].ceilingpicnum] == 0 )
        {
            tloadtile( sector[i].ceilingpicnum );
            if( waloff[sector[i].ceilingpicnum] == LA)
            {
                tloadtile(LA+1);
                tloadtile(LA+2);
            }
        }

        j = headspritesect[i];
        while(j >= 0)
        {
            if(sprite[j].xrepeat != 0 && sprite[j].yrepeat != 0 && (sprite[j].cstat&32768) == 0)
                if(waloff[sprite[j].picnum] == 0)
                    cachespritenum(j);
            j = nextspritesect[j];
        }
    }

}

void dofrontscreens(void)
{
    long tincs,i,j;

    if(ud.recstat != 2)
    {
        ps[myconnectindex].palette = palette;
        for(j=0;j<63;j+=7) palto(0,0,0,j);
        i = ud.screen_size;
        ud.screen_size = 0;
        vscrn();
        clearview(0L);

        rotatesprite(320<<15,200<<15,65536L,0,LOADSCREEN,2,0,2+8+64,0,0,xdim-1,ydim-1);

        if( boardfilename[0] != 0 && ud.level_number == 7 && ud.volume_number == 0 )
        {
#ifdef RRRA
            menutext(160,140,0,0,"ENTERIN' USER MAP");
            menutext(160,140+20,0,0,boardfilename);
#else
            menutext(160,90,0,0,"ENTERIN' USER MAP");
            menutext(160,90+20,0,0,boardfilename);
#endif
        }
        else if(lastlevel)
        {
#ifdef RRRA
            menutext(160,140,0,0,"ENTERIN'");
            menutext(160,140+16+8,0,0,"CLOSE ENCOUNTERS");
#else
            menutext(160,90,0,0,"ENTERIN'");
            menutext(160,90+16+8,0,0,"CLOSE ENCOUNTERS");
#endif
        }
        else
        {
#ifdef RRRA
            menutext(160,140,0,0,"ENTERIN'");
            menutext(160,140+16+8,0,0,level_names[(ud.volume_number*7) + ud.level_number]);
#else
            menutext(160,90,0,0,"ENTERIN'");
            menutext(160,90+16+8,0,0,level_names[(ud.volume_number*7) + ud.level_number]);
#endif
        }

        nextpage();

        for(j=63;j>0;j-=7) palto(0,0,0,j);

        KB_FlushKeyboardQueue();
        ud.screen_size = i;
    }
    else
    {
        clearview(0L);
        ps[myconnectindex].palette = palette;
        palto(0,0,0,0);
        rotatesprite(320<<15,200<<15,65536L,0,LOADSCREEN,2,0,2+8+64,0,0,xdim-1,ydim-1);
#ifdef RRRA
        menutext(160,155,0,0,"LOADIN'");
#else
        menutext(160,105,0,0,"LOADIN'");
#endif
        nextpage();
    }
}

void clearfifo(void)
{
    short i;

    syncvaltail = 0L;
    syncvaltottail = 0L;
    syncstat = 0;
    bufferjitter = 1;
    mymaxlag = otherminlag = 0;

    movefifoplc = movefifosendplc = fakemovefifoplc = 0;
    avgfvel = avgsvel = avgavel = avghorz = avgbits = 0;
    otherminlag = mymaxlag = 0;

    clearbufbyte(myminlag,MAXPLAYERS<<2,0L);
    clearbufbyte(&loc,sizeof(input),0L);
    clearbufbyte(&sync[0],sizeof(sync),0L);
    clearbufbyte(inputfifo,sizeof(input)*MOVEFIFOSIZ*MAXPLAYERS,0L);

    clearbuf(movefifoend,MAXPLAYERS,0L);
    clearbuf(syncvalhead,MAXPLAYERS,0L);

    for(i=connecthead;i>=0;i=connectpoint2[i])
        myminlag[i] = 0;
}

void enterlevel(char g)
{
    short i,j;
    long l;
    char levname[256];

    if( (g&MODE_DEMO) != MODE_DEMO ) ud.recstat = ud.m_recstat;
    ud.respawn_monsters = ud.m_respawn_monsters;
    ud.respawn_items    = ud.m_respawn_items;
    ud.respawn_inventory    = ud.m_respawn_inventory;
    ud.monsters_off = ud.m_monsters_off;
    ud.coop = ud.m_coop;
    ud.marker = ud.m_marker;
    ud.ffire = ud.m_ffire;

    if( (g&MODE_DEMO) == 0 && ud.recstat == 2)
        ud.recstat = 0;

    i = ud.screen_size;
    ud.screen_size = 0;
    dofrontscreens();
    vscrn();
    ud.screen_size = i;

    if (lastlevel)
    {
        if ( loadboard( "endgame.map",&ps[0].posx, &ps[0].posy, &ps[0].posz, &ps[0].ang,&ps[0].cursectnum ) == -1 )
        {
            sprintf(tempbuf,"Map %s not found!",boardfilename);
            gameexit(tempbuf);
        }
    }
    else
    {
        if( boardfilename[0] != 0 && ud.m_level_number == 7 && ud.m_volume_number == 0 )
        {
            if ( loadboard( boardfilename,&ps[0].posx, &ps[0].posy, &ps[0].posz, &ps[0].ang,&ps[0].cursectnum ) == -1 )
            {
                sprintf(tempbuf,"Map %s not found!",boardfilename);
                gameexit(tempbuf);
            }
        }
        else if ( loadboard( level_file_names[ (ud.volume_number*7)+ud.level_number],&ps[0].posx, &ps[0].posy, &ps[0].posz, &ps[0].ang,&ps[0].cursectnum ) == -1)
        {
            sprintf(tempbuf,"Map %s not found!",level_file_names[(ud.volume_number*8)+ud.level_number]);
            gameexit(tempbuf);
        }
    }

#ifndef RRRA
    if (ud.volume_number == 1 && ud.level_number == 1)
    {
        short ii;
        for (ii = PISTOL_WEAPON; ii < MAX_WEAPONS; ii++)
            ps[0].gotweapon[ii] = 0;
        for (ii = PISTOL_WEAPON; ii < MAX_WEAPONS; ii++)
            ps[0].ammo_amount[ii] = 0;
    }
#endif

    clearbufbyte(gotpic,sizeof(gotpic),0L);

    prelevel(g);

#ifdef RRRA
    if (ud.level_number == 2 && ud.volume_number == 0)
    {
        short ii;
        for (ii = PISTOL_WEAPON; ii < MAX_WEAPONS; ii++)
            ps[0].gotweapon[ii] = 0;
        for (ii = PISTOL_WEAPON; ii < MAX_WEAPONS; ii++)
            ps[0].ammo_amount[ii] = 0;
        ps[0].gotweapon[RA15_WEAPON] = 1;
        ps[0].ammo_amount[RA15_WEAPON] = 1;
        ps[0].curr_weapon = RA15_WEAPON;
    }
#endif

    allignwarpelevators();
    resetpspritevars(g);

    cachedebug = 0;
    automapping = 0;

    cacheit();
    docacheit();

    if (globalskillsound >= 0)
    {
        while (Sound[globalskillsound].lock >= 200);
    }
    globalskillsound = -1;

    FX_StopAllSounds();
    clearsoundlocks();
    FX_SetReverb(0);

    if( (g&MODE_GAME) || (g&MODE_EOL) )
        ps[myconnectindex].gm = MODE_GAME;
    else if(g&MODE_RESTART)
    {
        if(ud.recstat == 2)
            ps[myconnectindex].gm = MODE_DEMO;
        else ps[myconnectindex].gm = MODE_GAME;
    }

    if( (ud.recstat == 1) && (g&MODE_RESTART) != MODE_RESTART )
        opendemowrite();

    for(i=connecthead;i>=0;i=connectpoint2[i])
        switch(sector[sprite[ps[i].i].sectnum].floorpicnum)
        {
            case HURTRAIL:
            case FLOORSLIME:
            case FLOORPLASMA:
                resetweapons(i);
                resetinventory(i);
                ps[i].gotweapon[PISTOL_WEAPON] = 0;
                ps[i].ammo_amount[PISTOL_WEAPON] = 0;
                ps[i].curr_weapon = KNEE_WEAPON;
                ps[i].kickback_pic = 0;
                break;
        }

      //PREMAP.C - replace near the my's at the end of the file
      myx = omyx = ps[myconnectindex].posx;
	  myy = omyy = ps[myconnectindex].posy;
	  myz = omyz = ps[myconnectindex].posz;
	  myxvel = myyvel = myzvel = 0;
	  myang = omyang = ps[myconnectindex].ang;
	  myhoriz = omyhoriz = ps[myconnectindex].horiz;
	  myhorizoff = omyhorizoff = ps[myconnectindex].horizoff;
	  mycursectnum = ps[myconnectindex].cursectnum;
	  myjumpingcounter = ps[myconnectindex].jumping_counter;
	  myjumpingtoggle = ps[myconnectindex].jumping_toggle;
      myonground = ps[myconnectindex].on_ground;
      myhardlanding = ps[myconnectindex].hard_landing;
      myreturntocenter = ps[myconnectindex].return_to_center;

     ps[myconnectindex].palette = palette;
     palto(0,0,0,0);

     setpal(&ps[myconnectindex]);
     flushperms();

     everyothertime = 0;
     global_random = 0;

     ud.last_level = ud.level_number+1;

     clearfifo();

     for(i=numinterpolations-1;i>=0;i--) bakipos[i] = *curipos[i];

     restorepalette = 1;

     flushpackets();
     waitforeverybody();

     palto(0,0,0,0);
     vscrn();
     clearview(0L);
     drawbackground();
     displayrooms(screenpeek,65536);
     displayrest(screenpeek);
     nextpage();

     clearbufbyte(playerquitflag,MAXPLAYERS,0x01010101);
     if (waitabort == 1)
         gameexit(" ");
     ps[myconnectindex].over_shoulder_on = 0;

     clearfrags();

     resettimevars();  // Here we go
}

/*
Duke Nukem V

Layout:

      Settings:
        Suburbs
          Duke inflitrating neighborhoods inf. by aliens
        Death Valley:
          Sorta like a western.  Bull-skulls halb buried in the sand
          Military compound:  Aliens take over nuke-missle silo, duke
            must destroy.
          Abondend Aircraft field
        Vegas:
          Blast anything bright!  Alien lights camoflauged.
          Alien Drug factory. The Blue Liquid
        Mountainal Cave:
          Interior cave battles.
        Jungle:
          Trees, canopee, animals, a mysterious hole in the earth
        Penetencury:
          Good use of spotlights:
      Inventory:
        Wood,
        Metal,
        Torch,
        Rope,
        Plastique,
        Cloth,
        Wiring,
        Glue,
        Cigars,
        Food,
        Duck Tape,
        Nails,
        Piping,
        Petrol,
        Uranium,
        Gold,
        Prism,
        Power Cell,

        Hand spikes (Limited usage, they become dull)
        Oxygent     (Oxygen mixed with stimulant)


      Player Skills:
        R-Left,R-Right,Foward,Back
        Strafe, Jump, Double Flip Jump for distance
        Help, Escape
        Fire/Use
        Use Menu

Programming:
     Images: Polys
     Actors:
       Multi-Object sections for change (head,arms,legs,torsoe,all change)
       Facial expressions.  Pal lookup per poly?

     struct imagetype
        {
            int *itable; // AngX,AngY,AngZ,Xoff,Yoff,Zoff;
            int *idata;
            struct imagetype *prev, *next;
        }

*/
