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

/*
void displayloogie(short snum)
{
    long i, a, x, y, z;

    if(ps[snum].loogcnt == 0) return;

    y = (ps[snum].loogcnt<<2);
    for(i=0;i<ps[snum].numloogs;i++)
    {
        a = klabs(sintable[((ps[snum].loogcnt+i)<<5)&2047])>>5;
        z = 4096+((ps[snum].loogcnt+i)<<9);
        x = (-sync[snum].avel)+(sintable[((ps[snum].loogcnt+i)<<6)&2047]>>10);

        rotatesprite(
            (ps[snum].loogiex[i]+x)<<16,(200+ps[snum].loogiey[i]-y)<<16,z-(i<<8),256-a,
            LOOGIE,0,0,2,0,0,xdim-1,ydim-1);
    }
}

char animatefist(short gs,short snum)
{
    short looking_arc,fisti,fistpal;
    long fistzoom, fistz;

    fisti = ps[snum].fist_incs;
    if(fisti > 32) fisti = 32;
    if(fisti <= 0) return 0;

    looking_arc = klabs(ps[snum].look_ang)/9;

    fistzoom = 65536L - (sintable[(512+(fisti<<6))&2047]<<2);
    if(fistzoom > 90612L)
        fistzoom = 90612L;
    if(fistzoom < 40920)
        fistzoom = 40290;
    fistz = 194 + (sintable[((6+fisti)<<7)&2047]>>9);

    if(sprite[ps[snum].i].pal == 1)
        fistpal = 1;
    else
        fistpal = sector[ps[snum].cursectnum].floorpal;

    rotatesprite(
        (-fisti+222+(sync[snum].avel>>4))<<16,
        (looking_arc+fistz)<<16,
        fistzoom,0,FIST,gs,fistpal,2,0,0,xdim-1,ydim-1);

    return 1;
}

char animateknee(short gs,short snum)
{
    short knee_y[] = {0,-8,-16,-32,-64,-84,-108,-108,-108,-72,-32,-8};
    short looking_arc, pal;

    if(ps[snum].knee_incs > 11 || ps[snum].knee_incs == 0 || sprite[ps[snum].i].extra <= 0) return 0;

    looking_arc = knee_y[ps[snum].knee_incs] + klabs(ps[snum].look_ang)/9;

    looking_arc -= (ps[snum].hard_landing<<3);

    if(sprite[ps[snum].i].pal == 1)
        pal = 1;
    else
    {
        pal = sector[ps[snum].cursectnum].floorpal;
        if(pal == 0)
            pal = ps[snum].palookup;
    }

    myospal(105+(sync[snum].avel>>4)-(ps[snum].look_ang>>1)+(knee_y[ps[snum].knee_incs]>>2),looking_arc+280-((ps[snum].horiz-ps[snum].horizoff)>>4),KNEE,gs,4,pal);

    return 1;
}

char animateknuckles(short gs,short snum)
{
    short knuckle_frames[] = {0,1,2,2,3,3,3,2,2,1,0};
    short looking_arc, pal;

    if(ps[snum].knuckle_incs == 0 || sprite[ps[snum].i].extra <= 0) return 0;

    looking_arc = klabs(ps[snum].look_ang)/9;

    looking_arc -= (ps[snum].hard_landing<<3);

    if(sprite[ps[snum].i].pal == 1)
        pal = 1;
    else
        pal = sector[ps[snum].cursectnum].floorpal;

    myospal(160+(sync[snum].avel>>4)-(ps[snum].look_ang>>1),looking_arc+180-((ps[snum].horiz-ps[snum].horizoff)>>4),CRACKKNUCKLES+knuckle_frames[ps[snum].knuckle_incs>>1],gs,4,pal);

    return 1;
}
*/


long lastvisinc;

void displaymasks(short snum)
{
    short i, p;

    if(sprite[ps[snum].i].pal == 1)
        p = 1;
    else
        p = sector[ps[snum].cursectnum].floorpal;

     if(ps[snum].scuba_on)
	 {
        if(ud.screen_size > 4)
        {
            rotatesprite((320-(tilesizx[SCUBAMASK]>>1)-15)<<16,(200-(tilesizy[SCUBAMASK]>>1)+(sintable[totalclock&2047]>>10))<<16,49152,0,SCUBAMASK,0,p,2+16,windowx1,windowy1,windowx2,windowy2);
            rotatesprite((320-tilesizx[SCUBAMASK+4])<<16,(200-tilesizy[SCUBAMASK+4])<<16,65536,0,SCUBAMASK+4,0,p,2+16,windowx1,windowy1,windowx2,windowy2);
            rotatesprite(tilesizx[SCUBAMASK+4]<<16,(200-tilesizy[SCUBAMASK+4])<<16,65536,1024,SCUBAMASK+4,0,p,2+4+16,windowx1,windowy1,windowx2,windowy2);
            rotatesprite(35<<16,(-1)<<16,65536,0,SCUBAMASK+3,0,p,2+16,windowx1,windowy1,windowx2,windowy2);
            rotatesprite(285<<16,200<<16,65536,1024,SCUBAMASK+3,0,p,2+16,windowx1,windowy1,windowx2,windowy2);
        }
        else
        {
            rotatesprite((320-(tilesizx[SCUBAMASK]>>1)-15)<<16,(200-(tilesizy[SCUBAMASK]>>1)+(sintable[totalclock&2047]>>10))<<16,49152,0,SCUBAMASK,0,p,2+16,windowx1,windowy1,windowx2,windowy2);
            rotatesprite((320-tilesizx[SCUBAMASK+4])<<16,(200-tilesizy[SCUBAMASK+4])<<16,65536,0,SCUBAMASK+4,0,p,2+16,windowx1,windowy1,windowx2,windowy2);
            rotatesprite(tilesizx[SCUBAMASK+4]<<16,(200-tilesizy[SCUBAMASK+4])<<16,65536,1024,SCUBAMASK+4,0,p,2+4+16,windowx1,windowy1,windowx2,windowy2);
            rotatesprite(35<<16,(-1)<<16,65536,0,SCUBAMASK+3,0,p,2+16,windowx1,windowy1,windowx2,windowy2);
            rotatesprite(285<<16,200<<16,65536,1024,SCUBAMASK+3,0,p,2+16,windowx1,windowy1,windowx2,windowy2);
        }
	 }
}
#if 0
char animatetip(short gs,short snum)
{
    short p,looking_arc;
    short tip_y[] = {0,-8,-16,-32,-64,-84,-108,-108,-108,-108,-108,-108,-108,-108,-108,-108,-96,-72,-64,-32,-16};

    if(ps[snum].tipincs == 0) return 0;

    looking_arc = klabs(ps[snum].look_ang)/9;
    looking_arc -= (ps[snum].hard_landing<<3);

    if(sprite[ps[snum].i].pal == 1)
        p = 1;
    else
        p = sector[ps[snum].cursectnum].floorpal;

/*    if(ps[snum].access_spritenum >= 0)
        p = sprite[ps[snum].access_spritenum].pal;
    else
        p = wall[ps[snum].access_wallnum].pal;
  */
    myospal(170+(sync[snum].avel>>4)-(ps[snum].look_ang>>1),
        (tip_y[ps[snum].tipincs]>>1)+looking_arc+240-((ps[snum].horiz-ps[snum].horizoff)>>4),TIP+((26-ps[snum].tipincs)>>4),gs,0,p);

    return 1;
}

char animateaccess(short gs,short snum)
{
    short access_y[] = {0,-8,-16,-32,-64,-84,-108,-108,-108,-108,-108,-108,-108,-108,-108,-108,-96,-72,-64,-32,-16};
    short looking_arc;
    char p;

    if(ps[snum].access_incs == 0 || sprite[ps[snum].i].extra <= 0) return 0;

    looking_arc = access_y[ps[snum].access_incs] + klabs(ps[snum].look_ang)/9;
    looking_arc -= (ps[snum].hard_landing<<3);

    if(ps[snum].access_spritenum >= 0)
        p = sprite[ps[snum].access_spritenum].pal;
    else p = 0;
//    else
//        p = wall[ps[snum].access_wallnum].pal;

    if((ps[snum].access_incs-3) > 0 && (ps[snum].access_incs-3)>>3)
        myospal(170+(sync[snum].avel>>4)-(ps[snum].look_ang>>1)+(access_y[ps[snum].access_incs]>>2),looking_arc+266-((ps[snum].horiz-ps[snum].horizoff)>>4),HANDHOLDINGLASER+(ps[snum].access_incs>>3),gs,0,p);
    else
        myospal(170+(sync[snum].avel>>4)-(ps[snum].look_ang>>1)+(access_y[ps[snum].access_incs]>>2),looking_arc+266-((ps[snum].horiz-ps[snum].horizoff)>>4),HANDHOLDINGACCESS,gs,4,p);

    return 1;
}
#endif

short fistsign;

void displayweapon(short snum)
{
    long gun_pos, looking_arc, cw;
    long weapon_xoffset, i, j, x1, y1, x2;
    char o,pal;
    signed char gs;
    struct player_struct *p;
    short *kb;

    p = &ps[snum];
    kb = &p->kickback_pic;

    o = 0;

    looking_arc = klabs(p->look_ang)/9;

    if (shadedsector[p->cursectnum] == 1)
        gs = 16;
    else
        gs = sprite[p->i].shade;
    if(gs > 24) gs = 24;

    if(p->newowner >= 0 || ud.camerasprite >= 0 || (sprite[p->i].pal != 1 && sprite[p->i].extra <= 0))
        return;

    gun_pos = 80-(p->weapon_pos*p->weapon_pos);

    weapon_xoffset =  (160)-90;
    weapon_xoffset -= (sintable[((p->weapon_sway>>1)+512)&2047]/(1024+512));
    weapon_xoffset -= 58 + p->weapon_ang;
    if( sprite[p->i].xrepeat < 8 )
        gun_pos -= klabs(sintable[(p->weapon_sway<<2)&2047]>>9);
    else gun_pos -= klabs(sintable[(p->weapon_sway>>1)&2047]>>10);

    gun_pos -= (p->hard_landing<<3);

    if(p->last_weapon >= 0)
        cw = p->last_weapon;
    else cw = p->curr_weapon;

    j = 14-p->quick_kick;
    if(j != 14)
    {
        if(sprite[p->i].pal == 1)
            pal = 1;
        else
            pal = p->palookup;
    }

#ifdef RRRA
    if (p->OnMotorcycle)
    {
        int temp1, temp_kb;
        if (numplayers == 1)
        {
            if (*kb)
            {
                gs = 0;
                if (*kb == 1)
                {
                    if ((TRAND&1) == 1)
                        temp_kb = MOTOHIT+1;
                    else
                        temp_kb = MOTOHIT+2;
                }
                else if (*kb == 4)
                {
                    if ((TRAND&1) == 1)
                        temp_kb = MOTOHIT+3;
                    else
                        temp_kb = MOTOHIT+4;
                }
                else
                    temp_kb = MOTOHIT;

            }
            else
                temp_kb = MOTOHIT;
        }
        else
        {
            if (*kb)
            {
                gs = 0;
                if (*kb == 1)
                    temp_kb = MOTOHIT+1;
                else if (*kb == 2)
                    temp_kb = MOTOHIT+2;
                else if (*kb == 3)
                    temp_kb = MOTOHIT+3;
                else if (*kb == 4)
                    temp_kb = MOTOHIT+4;
                else
                    temp_kb = MOTOHIT;

            }
            else
                temp_kb = MOTOHIT;
        }
        if (sprite[p->i].pal == 1)
            pal = 1;
        else
            pal = sector[p->cursectnum].floorpal;

        if (p->TiltStatus >= 0)
            ShowMotorcycle(160-(p->look_ang>>1), 174, temp_kb, gs, 0, pal, p->TiltStatus*5);
        else if (p->TiltStatus < 0)
            ShowMotorcycle(160-(p->look_ang>>1), 174, temp_kb, gs, 0, pal, p->TiltStatus*5+2047);
        return;
    }
    if (p->OnBoat)
    {
        int temp2, temp_kb, temp3;
        temp2 = 0;
        if (p->TiltStatus > 0)
        {
            if (*kb == 0)
                temp_kb = BOATHIT+1;
            else if (*kb <= 3)
            {
                temp_kb = BOATHIT+5;
                temp2 = 1;
            }
            else if (*kb <= 6)
            {
                temp_kb = BOATHIT+6;
                temp2 = 1;
            }
            else
                temp_kb = BOATHIT+1;
        }
        else if (p->TiltStatus < 0)
        {
            if (*kb == 0)
                temp_kb = BOATHIT+2;
            else if (*kb <= 3)
            {
                temp_kb = BOATHIT+7;
                temp2 = 1;
            }
            else if (*kb <= 6)
            {
                temp_kb = BOATHIT+8;
                temp2 = 1;
            }
            else
                temp_kb = BOATHIT+2;
        }
        else
        {
            if (*kb == 0)
                temp_kb = BOATHIT;
            else if (*kb <= 3)
            {
                temp_kb = BOATHIT+3;
                temp2 = 1;
            }
            else if (*kb <= 6)
            {
                temp_kb = BOATHIT+4;
                temp2 = 1;
            }
            else
                temp_kb = BOATHIT;
        }

        if (sprite[p->i].pal == 1)
            pal = 1;
        else
            pal = sector[p->cursectnum].floorpal;

        if (p->NotOnWater)
            temp3 = 170;
        else
            temp3 = 170 + (*kb>>2);

        if (temp2)
            gs = -96;

        if (p->TiltStatus >= 0)
            ShowBoat(160-(p->look_ang>>1), temp3, temp_kb, gs, 0, pal, p->TiltStatus);
        else if (p->TiltStatus < 0)
            ShowBoat(160-(p->look_ang>>1), temp3, temp_kb, gs, 0, pal, p->TiltStatus+2047);
        return;
    }
#endif

    if( sprite[p->i].xrepeat < 8 )
    {
        if(p->jetpack_on == 0 )
        {
            i = sprite[p->i].xvel;
            looking_arc += 32-(i>>1);
            fistsign += i>>1;
        }
        cw = weapon_xoffset;
        weapon_xoffset += sintable[(fistsign)&2047]>>10;
        myos(weapon_xoffset+250-(p->look_ang>>1),
             looking_arc+258-(klabs(sintable[(fistsign)&2047]>>8)),
             FIST,gs,o);
        weapon_xoffset = cw;
        weapon_xoffset -= sintable[(fistsign)&2047]>>10;
        myos(weapon_xoffset+40-(p->look_ang>>1),
             looking_arc+200+(klabs(sintable[(fistsign)&2047]>>8)),
             FIST,gs,o|4);
    }
    else switch(cw)
    {
        case KNEE_WEAPON:
#ifdef RRRA
        case RA15_WEAPON:
#endif
            if(sprite[p->i].pal == 1)
                pal = 1;
            else
                pal = sector[p->cursectnum].floorpal;
#ifdef RRRA
            if (cw == KNEE_WEAPON)
#endif
            {
                short kb_frames[] = { 0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7 };
                short kb_ox[] = { 310,342,364,418,350,316,282,288,0,0 };
                short kb_oy[] = { 300,362,320,268,248,248,277,420,0,0 };
                short x;
                short y;
                x = weapon_xoffset + ((kb_ox[kb_frames[*kb]]>>1) - 12);
                y = 200 - (244-kb_oy[kb_frames[*kb]]);
                myospal(x-(p->look_ang>>1),looking_arc+y-gun_pos,
                        KNEE+kb_frames[*kb],gs,0,pal);
            }
#ifdef RRRA
            else
            {
                short kb_frames[] = { 0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7 };
                short kb_ox[] = { 580,676,310,491,356,210,310,614 };
                short kb_oy[] = { 369,363,300,323,371,400,300,440 };
                short x, y;
                x = weapon_xoffset + ((kb_ox[kb_frames[*kb]]>>1) - 12);
                y = 210 - (244-kb_oy[kb_frames[*kb]]);
                myospalsb(x-(p->look_ang>>1)+20,looking_arc+y-gun_pos-80,
                        SLINGBLADE+kb_frames[*kb],gs,0,pal);
            }
#endif
            break;

        case TRIPBOMB_WEAPON:
        case BOWLING_WEAPON:
            if(sprite[p->i].pal == 1)
                pal = 1;
            else
                pal = sector[p->cursectnum].floorpal;

            weapon_xoffset += 8;
            gun_pos -= 10;

            if (cw == BOWLING_WEAPON)
            {
                if (p->ammo_amount[BOWLING_WEAPON])
                {
                    myospal(weapon_xoffset+162-(p->look_ang>>1),
                            looking_arc+214-gun_pos+(*kb<<3),BOWLINGBALLH,gs,o,pal);
                }
                else
                {
                    rdmyospal(weapon_xoffset+162-(p->look_ang>>1),
                              looking_arc+214-gun_pos,HANDTHROW+5,gs,o,pal);
                }
            }
            else
            {
                if (p->ammo_amount[TRIPBOMB_WEAPON])
                {
                    rdmyospal(weapon_xoffset+180-(p->look_ang>>1),
                              looking_arc+214-gun_pos+(*kb<<3),POWDERH,gs,o,pal);
                    rdmyospal(weapon_xoffset+90-(p->look_ang>>1),
                              looking_arc+214-gun_pos+(*kb<<3),POWDERH,gs,o|4,pal);
                }
                else
                {
                    rdmyospal(weapon_xoffset+162-(p->look_ang>>1),
                              looking_arc+214-gun_pos,HANDTHROW+5,gs,o,pal);
                }
            }
            break;

        case RPG_WEAPON:
#ifdef RRRA
        case RA16_WEAPON:
#endif
            if(sprite[p->i].pal == 1)
                pal = 1;
            else pal = sector[p->cursectnum].floorpal;
            
#ifdef RRRA
            if (cw == RPG_WEAPON)
            {
#endif
            if(*kb)
            {
                char kb_frames[] = { 0,1,1,2,2,3,2,3,2,3,2,2,2,2,2,2,2,2,2,4,4,4,4,5,5,5,5,6,6,6,6,6,6,7,7,7,7,7,7 };
                if (kb_frames[*kb] == 2)
                {
                    rdmyospal((weapon_xoffset+200)-(p->look_ang>>1),
                              looking_arc+250-gun_pos,RPGGUN+kb_frames[*kb],gs,o,pal);
                }
                else if (kb_frames[*kb] == 3)
                {
                    rdmyospal((weapon_xoffset+200)-(p->look_ang>>1),
                              looking_arc+250-gun_pos,RPGGUN+kb_frames[*kb],gs,o,pal);
                }
                else if (kb_frames[*kb] == 1)
                {
                    rdmyospal((weapon_xoffset+200)-(p->look_ang>>1),
                              looking_arc+250-gun_pos,RPGGUN+kb_frames[*kb],0,o,pal);
                }
                else
                {
                    rdmyospal((weapon_xoffset+210)-(p->look_ang>>1),
                              looking_arc+255-gun_pos,RPGGUN+kb_frames[*kb],gs,o,pal);
                }
            }
            else
            {
                rdmyospal((weapon_xoffset+210)-(p->look_ang>>1),
                          looking_arc+255-gun_pos,RPGGUN,gs,o,pal);
            }
#ifdef RRRA
            }
            else
            {
                if(*kb)
                {
                    char kb_frames[] = { 0,1,1,2,2,3,2,3,2,3,2,2,2,2,2,2,2,2,2,4,4,4,4,5,5,5,5,6,6,6,6,6,6,7,7,7,7,7,7 };
                    if (kb_frames[*kb] == 2)
                    {
                        rdmyospal((weapon_xoffset+200)-(p->look_ang>>1),
                                  looking_arc+250-gun_pos,RPGGUN2+kb_frames[*kb],gs,o,pal);
                    }
                    else if (kb_frames[*kb] == 3)
                    {
                        rdmyospal((weapon_xoffset+200)-(p->look_ang>>1),
                                  looking_arc+250-gun_pos,RPGGUN2+kb_frames[*kb],gs,o,pal);
                    }
                    else if (kb_frames[*kb] == 1)
                    {
                        rdmyospal((weapon_xoffset+200)-(p->look_ang>>1),
                                  looking_arc+250-gun_pos,RPGGUN2+kb_frames[*kb],0,o,pal);
                    }
                    else
                    {
                        rdmyospal((weapon_xoffset+210)-(p->look_ang>>1),
                                  looking_arc+255-gun_pos,RPGGUN2+kb_frames[*kb],gs,o,pal);
                    }
                }
                else
                {
                    if (ud.multimode < 2)
                    {
                        if (p->raat605)
                        {
                            rdmyospal((weapon_xoffset+210)-(p->look_ang>>1),
                                      looking_arc+222-gun_pos,RPGGUN2+7,gs,o,pal);
                        }
                        else if ((TRAND&15) == 5)
                        {
                            spritesound(327,p->i);
                            rdmyospal((weapon_xoffset+210)-(p->look_ang>>1),
                                      looking_arc+222-gun_pos,RPGGUN2+7,gs,o,pal);
                            p->raat605 = 6;
                        }
                        else
                        {
                            rdmyospal((weapon_xoffset+210)-(p->look_ang>>1),
                                      looking_arc+225-gun_pos,RPGGUN2,gs,o,pal);
                        }
                    }
                    else
                    {
                        rdmyospal((weapon_xoffset+210)-(p->look_ang>>1),
                                  looking_arc+225-gun_pos,RPGGUN2,gs,o,pal);
                    }
                }
            }
#endif

            break;

        case SHOTGUN_WEAPON:
            if(sprite[p->i].pal == 1)
                pal = 1;
            else
                pal = sector[p->cursectnum].floorpal;

            weapon_xoffset -= 8;

            if(sprite[p->i].pal == 1)
                pal = 1;
            else
                pal = sector[p->cursectnum].floorpal;

            {
                short x;
                short y;
                short kb_frames3[] = { 0,0,1,1,2,2,5,5,6,6,7,7,8,8,0,0,0,0,0,0,0 };
                short kb_frames2[] = { 0,0,3,3,4,4,5,5,6,6,7,7,8,8,0,0,20,20,21,21,21,21,20,20,20,20,0,0 };
                short kb_frames[] = { 0,0,1,1,2,2,3,3,4,4,5,5,5,5,6,6,6,6,7,7,7,7,8,8,0,0,20,20,21,21,21,21,20,20,20,20,0,0 };
                short kb_ox[] = { 300,300,300,300,300,330,320,310,305,306,302 };
                short kb_oy[] = { 315,300,302,305,302,302,303,306,302,404,384 };
                short tm;
                tm = 180;
                if (p->at59a)
                {
                    if ((*kb) < 26)
                    {
                        if (kb_frames[*kb] == 3 || kb_frames[*kb] == 4)
                            gs = 0;
                        x = weapon_xoffset+((kb_ox[kb_frames[*kb]]>>1)-12);
                        y = tm-(244-kb_oy[kb_frames[*kb]]);
                        myospal(x+64-(p->look_ang>>1),
                                y+looking_arc-gun_pos,SHOTGUN+kb_frames[*kb],gs,0,pal);
                    }
                    else
                    {
                        if (kb_frames[*kb] > 0)
                        {
                            x = weapon_xoffset+((kb_ox[kb_frames[(*kb)-11]]>>1)-12);
                            y = tm-(244-kb_oy[kb_frames[(*kb)-11]]);
                        }
                        else
                        {
                            x = weapon_xoffset+((kb_ox[kb_frames[*kb]]>>1)-12);
                            y = tm-(244-kb_oy[kb_frames[*kb]]);
                        }
                        switch (*kb)
                        {
                            case 23:
                                y += 60;
                                break;
                            case 24:
                                y += 30;
                                break;
                        }
                        myospal(x+64-(p->look_ang>>1),y+looking_arc-gun_pos,SHOTGUN+kb_frames[*kb],gs,0,pal);
                        if(kb_frames[*kb] == 21)
                            myospal(x+96-(p->look_ang>>1),y+looking_arc-gun_pos,SHOTGUNSHELLS,gs,0,pal);
                    }
                }
                else
                {
                    if ((*kb) < 16)
                    {
                        if (p->at599)
                        {
                            if (kb_frames2[*kb] == 3 || kb_frames2[*kb] == 4)
                                gs = 0;
                            x = weapon_xoffset+((kb_ox[kb_frames2[*kb]]>>1)-12);
                            y = tm-(244-kb_oy[kb_frames2[*kb]]);
                            myospal(x+64-(p->look_ang>>1),
                                    y+looking_arc-gun_pos,SHOTGUN+kb_frames2[*kb],gs,0,pal);
                        }
                        else
                        {
                            if (kb_frames3[*kb] == 1 || kb_frames3[*kb] == 2)
                                gs = 0;
                            x = weapon_xoffset+((kb_ox[kb_frames3[*kb]]>>1)-12);
                            y = tm-(244-kb_oy[kb_frames3[*kb]]);
                            myospal(x+64-(p->look_ang>>1),
                                    y+looking_arc-gun_pos,SHOTGUN+kb_frames3[*kb],gs,0,pal);
                        }
                    }
                    else if (p->at599)
                    {
                        if (kb_frames2[*kb] > 0)
                        {
                            x = weapon_xoffset+((kb_ox[kb_frames2[(*kb)-11]]>>1)-12);
                            y = tm-(244-kb_oy[kb_frames2[(*kb)-11]]);
                        }
                        else
                        {
                            x = weapon_xoffset+((kb_ox[kb_frames2[*kb]]>>1)-12);
                            y = tm-(244-kb_oy[kb_frames2[*kb]]);
                        }
                        switch (*kb)
                        {
                            case 23:
                                y += 60;
                                break;
                            case 24:
                                y += 30;
                                break;
                        }
                        myospal(x+64-(p->look_ang>>1),y+looking_arc-gun_pos,SHOTGUN+kb_frames2[*kb],gs,0,pal);
                        if(kb_frames2[*kb] == 21)
                            myospal(x+96-(p->look_ang>>1),y+looking_arc-gun_pos,SHOTGUNSHELLS,gs,0,pal);
                    }
                }
            }
            break;



        case CHAINGUN_WEAPON:
            if(sprite[p->i].pal == 1)
                pal = 1;
            else
                pal = sector[p->cursectnum].floorpal;

            if(*kb > 0)
                gun_pos -= sintable[(*kb)<<7]>>12;

            if(*kb > 0 && sprite[p->i].pal != 1) weapon_xoffset += 1-(rand()&3);

            switch(*kb)
            {
                case 0:
                    myospal(weapon_xoffset+178-(p->look_ang>>1)+30,looking_arc+233-gun_pos+5,
                        CHAINGUN,gs,o,pal);
                    break;
                default:
                    gs = 0;
                    if(*kb < 8)
                    {
                        i = rand()&7;
                        myospal(weapon_xoffset+178-(p->look_ang>>1)+30,looking_arc+233-gun_pos+5,
                            CHAINGUN+1,gs,o,pal);
                    }
                    else myospal(weapon_xoffset+178-(p->look_ang>>1)+30,looking_arc+233-gun_pos+5,
                        CHAINGUN+2,gs,o,pal);
                    break;
            }
            break;
         case PISTOL_WEAPON:
             if(sprite[p->i].pal == 1)
                pal = 1;
            else
                pal = sector[p->cursectnum].floorpal;

            if( (*kb) < 22)
            {
                short kb_frames[] = { 0,0,1,1,2,2,3,3,4,4,6,6,6,6,5,5,4,4,3,3,0,0 };
                short kb_ox[] = { 194,190,185,208,215,215,216,216,201,170 };
                short kb_oy[] = { 256,249,248,238,228,218,208,256,245,258 };
                short x;
                short y;

                x = weapon_xoffset+(kb_ox[kb_frames[*kb]]-12);
                y = 244-(244-kb_oy[kb_frames[*kb]]);

                if (kb_frames[*kb])
                    gs = 0;

                rdmyospal(x-(p->look_ang>>1),
                          y+looking_arc-gun_pos,FIRSTGUN+kb_frames[*kb],gs,0,pal);
            }
            else
            {
                short kb_frames[] = { 0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0 };
                short kb_ox[] = { 244,244,244 };
                short kb_oy[] = { 256,249,248 };
                short x;
                short dx;
                short y;
                short dy;

                x = weapon_xoffset+(kb_ox[kb_frames[(*kb)-22]]-12);
                y = 244-(244-kb_oy[kb_frames[(*kb)-22]]);
                switch (*kb)
                {
                    case 28:
                        dy = 10;
                        dx = 5;
                        break;
                    case 29:
                        dy = 20;
                        dx = 10;
                        break;
                    case 30:
                        dy = 30;
                        dx = 15;
                        break;
                    case 31:
                        dy = 40;
                        dx = 20;
                        break;
                    case 32:
                        dy = 50;
                        dx = 25;
                        break;
                    case 33:
                        dy = 40;
                        dx = 20;
                        break;
                    case 34:
                        dy = 30;
                        dx = 15;
                        break;
                    case 35:
                        dy = 20;
                        dx = 10;
                        break;
                    case 36:
                        dy = 10;
                        dx = 5;
                        break;
                    default:
                        dy = 0;
                        dx = 0;
                        break;
                }
                rdmyospal(x-(p->look_ang>>1)-dx,
                          y+looking_arc-gun_pos+dy,FIRSTGUNRELOAD+kb_frames[(*kb)-22],gs,0,pal);
            }

            break;
        case HANDBOMB_WEAPON:
        {
            if(sprite[p->i].pal == 1)
                pal = 1;
            else
                pal = sector[p->cursectnum].floorpal;

            gun_pos -= 9*(*kb);

            rdmyospal(weapon_xoffset+190-(p->look_ang>>1),looking_arc+260-gun_pos,HANDTHROW,gs,o,pal);
        }
        break;

        case HANDREMOTE_WEAPON:
            {
                int dx;
                int x;
                int dy;
                dx = 25;
                x = 100;
                dy = 20;
                if((*kb) < 20)
                {
                    signed char remote_frames[] = {1,1,1,1,1,2,2,2,2,3,3,3,4,4,4,5,5,5,5,5,6,6,6};
                    if(sprite[p->i].pal == 1)
                        pal = 1;
                    else
                        pal = sector[p->cursectnum].floorpal;

                    if (*kb)
                    {
                        if ((*kb) < 5)
                        {
                            rdmyospal(weapon_xoffset+x+190-(p->look_ang>>1)-dx,
                                      looking_arc+258-gun_pos-64+p->at57e-dy,RRTILE1752,0,o,pal);
                        }
                        rdmyospal(weapon_xoffset+x+190-(p->look_ang>>1),
                                  looking_arc+258-gun_pos-dy,HANDTHROW+remote_frames[*kb],gs,o,pal);
                    }
                    else
                    {
                        if ((*kb) < 5)
                        {
                            rdmyospal(weapon_xoffset+x+190-(p->look_ang>>1)-dx,
                                      looking_arc+258-gun_pos-64+p->at57e-dy,RRTILE1752,0,o,pal);
                        }
                        rdmyospal(weapon_xoffset+x+190-(p->look_ang>>1),
                                  looking_arc+258-gun_pos-dy,HANDTHROW+1,gs,o,pal);
                    }
                }
            }
            break;
        case DEVISTATOR_WEAPON:
            if(sprite[p->i].pal == 1)
                pal = 1;
            else
                pal = sector[p->cursectnum].floorpal;

            if (*kb)
            {
                gs = 0;
                rd3myospal(150+(weapon_xoffset>>1)-(p->look_ang>>1),266+(looking_arc>>1)-gun_pos,DEVISTATOR,gs,o,pal);
            }
            else
                rd3myospal(150+(weapon_xoffset>>1)-(p->look_ang>>1),266+(looking_arc>>1)-gun_pos,DEVISTATOR+1,gs,o,pal);
            break;

#ifdef RRRA
        case RA13_WEAPON:
        case RA14_WEAPON:
            break;
#endif

        case FREEZE_WEAPON:
            if(sprite[p->i].pal == 1)
                pal = 1;
            else
                pal = sector[p->cursectnum].floorpal;

            if((*kb))
            {
                char cat_frames[] = { 0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
                rdmyospal(weapon_xoffset+260-(p->look_ang>>1),looking_arc+215-gun_pos,FREEZE+cat_frames[*kb],-32,o,pal);
            }
            else rdmyospal(weapon_xoffset+260-(p->look_ang>>1),looking_arc+215-gun_pos,FREEZE,gs,o,pal);

            break;

        case SHRINKER_WEAPON:
        case GROW_WEAPON:
            weapon_xoffset += 28;
            looking_arc += 18;
            if(sprite[p->i].pal == 1)
                pal = 1;
            else
                pal = sector[p->cursectnum].floorpal;
            if((*kb) == 0)
            {
                rd2myospal(weapon_xoffset+188-(p->look_ang>>1),
                           looking_arc+240-gun_pos,SHRINKER,gs,o,pal);
            }
            else
            {
                if(sprite[p->i].pal != 1)
                {
                    weapon_xoffset += rand()&3;
                    gun_pos += (rand()&3);
                }

                if(cw == GROW_WEAPON)
                {
                    rd2myospal(weapon_xoffset+184-(p->look_ang>>1),
                               looking_arc+240-gun_pos,GROWSPARK+((*kb)&2),gs,o,0);
                }
                else
                {
                    signed char kb_frames[] = { 1,1,1,1,1,2,2,2,2,2,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0 };
                    short frm = kb_frames[*kb];
                    rd2myospal(weapon_xoffset+184-(p->look_ang>>1),
                               looking_arc+240-gun_pos,SHRINKER+frm,gs,o,0);
                }
            }
            break;
    }

    //displayloogie(snum);

}
