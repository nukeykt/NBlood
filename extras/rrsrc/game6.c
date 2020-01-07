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

extern char debug_on,actor_tog,*rtsptr;
extern short inputloc;

#ifdef RRRA
#define NUMCHEATCODES 40
char cheatquotes[NUMCHEATCODES][14] = {
    {"hounddog"},
    {"all"},
    {"meadow###"},
    {"yerat"},
    {"view"},
    {"time"},
    {"unlock"},
    {"cluck"},
    {"items"},
    {"rate"},
    {"skill#"},
    {"teachers"},
    {"moonshine"},
    {"critters"},
    {"<RESERVED>"},
    {"<RESERVED>"},
    {"rafael"},
    {"showmap"},
    {"elvis"},
    {"<RESERVED>"},
    {"clip"},
    {"guns"},
    {"inventory"},
    {"keys"},
    {"debug"},
    {"joseph"},
    {"mrbill"},
    {"tony"},
    {"gary"},
    {"rhett"},
    {"aaron"},
    {"nocheat"},
    {"woleslagle"},
    {"mikael"},
    {"greg"},
    {"noah"},
    {"arijit"},
    {"donut"},
    {"kfc"},
    {"van"}
};


char cheatbuf[20],cheatbuflen;
#else
#define NUMCHEATCODES 25
char cheatquotes[NUMCHEATCODES][14] = {
    {"hounddog"},
    {"all"},
    {"meadow###"},
    {"yerat"},
    {"view"},
    {"time"},
    {"unlock"},
    {"cluck"},
    {"items"},
    {"rate"},
    {"skill#"},
    {"teachers"},
    {"moonshine"},
    {"critters"},
    {"<RESERVED>"},
    {"<RESERVED>"},
    {"rafael"},
    {"showmap"},
    {"elvis"},
    {"<RESERVED>"},
    {"clip"},
    {"guns"},
    {"inventory"},
    {"keys"},
    {"debug"}
};


char cheatbuf[10],cheatbuflen;
#endif
void cheats(void)
{
    short ch, i, j, k, keystate, weapon, key;

#ifdef RRRA
    if (ud.m_player_skill > 3 || ps[myconnectindex].nocheat)
#else
    if (ud.m_player_skill > 3)
#endif
        return;

    if( (ps[myconnectindex].gm&MODE_TYPE) || (ps[myconnectindex].gm&MODE_MENU))
        return;

    if ( ps[myconnectindex].cheat_phase == 1)
    {
       while (KB_KeyWaiting())
       {
          ch = KB_Getch();
          ch = tolower(ch);

          if( !( (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') ) )
          {
             ps[myconnectindex].cheat_phase = 0;
//             FTA(46,&ps[myconnectindex]);
             return;
          }

          cheatbuf[cheatbuflen++] = ch;
          cheatbuf[cheatbuflen] = 0;

#ifdef RRRA
          if(cheatbuflen > 13)
#else
          if(cheatbuflen > 11)
#endif
          {
              ps[myconnectindex].cheat_phase = 0;
              return;
          }

          for(k = 0;k < NUMCHEATCODES;k++)
          {
              for(j = 0;j<cheatbuflen;j++)
              {
                  if( cheatbuf[j] == cheatquotes[k][j] || (cheatquotes[k][j] == '#' && ch >= '0' && ch <= '9') )
                  {
                      if( cheatquotes[k][j+1] == 0 ) goto FOUNDCHEAT;
                      if(j == cheatbuflen-1) return;
                  }
                  else break;
              }
          }

          ps[myconnectindex].cheat_phase = 0;
          return;

          FOUNDCHEAT:
          {
                switch(k)
                {
                    case 21:
                        j = 0;

                        for ( weapon = PISTOL_WEAPON;weapon < MAX_WEAPONS-j;weapon++ )
                        {
                            addammo( weapon, &ps[myconnectindex], max_ammo_amount[weapon] );
                            ps[myconnectindex].gotweapon[weapon]  = 1;
                        }
#ifdef RRRA
                        ps[myconnectindex].ammo_amount[RA15_WEAPON] = 1;
#endif

                        KB_FlushKeyboardQueue();
                        ps[myconnectindex].cheat_phase = 0;
                        FTA(119,&ps[myconnectindex]);
                        return;
                    case 22:
                        KB_FlushKeyboardQueue();
                        ps[myconnectindex].cheat_phase = 0;
                        ps[myconnectindex].steroids_amount =         400;
                        ps[myconnectindex].boot_amount          =    2000;
                        ps[myconnectindex].shield_amount =           100;
                        ps[myconnectindex].scuba_amount =            6400;
                        ps[myconnectindex].holoduke_amount =         2400;
                        ps[myconnectindex].jetpack_amount =          600;
                        ps[myconnectindex].firstaid_amount =         max_player_health;
                        FTA(120,&ps[myconnectindex]);
                        ps[myconnectindex].cheat_phase = 0;
                        return;
                    case 23:
                        {
                            int ikey;
                            ps[myconnectindex].got_access =              7;
                            KB_FlushKeyboardQueue();
                            ps[myconnectindex].cheat_phase = 0;
                            for (ikey = 0; ikey < 5; ikey++)
                                ps[myconnectindex].keys[ikey] = 1;
                            FTA(121,&ps[myconnectindex]);
                        }
                        return;
                    case 24:
                        debug_on = 1-debug_on;
                        KB_FlushKeyboardQueue();
                        ps[myconnectindex].cheat_phase = 0;
                        break;
                    case 20:
                        ud.clipping = 1-ud.clipping;
                        KB_FlushKeyboardQueue();
                        ps[myconnectindex].cheat_phase = 0;
                        FTA(112+ud.clipping,&ps[myconnectindex]);
                        return;

                    case 15:
                        FTA(52,&ps[myconnectindex]);
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;

                    case 14:
                        FTA(51,&ps[myconnectindex]);
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;

                    case 19:
                        FTA(79,&ps[myconnectindex]);
                        ps[myconnectindex].cheat_phase = 0;
                        KB_ClearKeyDown(sc_N);
                        return;
                    case 0:
                    case 18:

                        ud.god = 1-ud.god;

                        if(ud.god)
                        {
#ifdef RRRA
                            sound(218);
#endif
                            pus = 1;
                            pub = 1;
                            sprite[ps[myconnectindex].i].cstat = 257;

                            hittype[ps[myconnectindex].i].temp_data[0] = 0;
                            hittype[ps[myconnectindex].i].temp_data[1] = 0;
                            hittype[ps[myconnectindex].i].temp_data[2] = 0;
                            hittype[ps[myconnectindex].i].temp_data[3] = 0;
                            hittype[ps[myconnectindex].i].temp_data[4] = 0;
                            hittype[ps[myconnectindex].i].temp_data[5] = 0;

                            sprite[ps[myconnectindex].i].hitag = 0;
                            sprite[ps[myconnectindex].i].lotag = 0;
                            sprite[ps[myconnectindex].i].pal =
                                ps[myconnectindex].palookup;

                            FTA(17,&ps[myconnectindex]);
                        }
                        else
                        {
                            ud.god = 0;
                            sprite[ps[myconnectindex].i].extra = max_player_health;
                            hittype[ps[myconnectindex].i].extra = -1;
                            ps[myconnectindex].last_extra = max_player_health;
                            FTA(18,&ps[myconnectindex]);
                        }

                        sprite[ps[myconnectindex].i].extra = max_player_health;
                        hittype[ps[myconnectindex].i].extra = 0;
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();

                        return;

                    case 1:
                        j = 0;
                        for ( weapon = PISTOL_WEAPON;weapon < MAX_WEAPONS-j;weapon++ )
                           ps[myconnectindex].gotweapon[weapon]  = 1;

                        for ( weapon = PISTOL_WEAPON;
                              weapon < (MAX_WEAPONS-j);
                              weapon++ )
                            addammo( weapon, &ps[myconnectindex], max_ammo_amount[weapon] );
#ifdef RRRA
                        ps[myconnectindex].ammo_amount[RA15_WEAPON] = 1;
#endif

                        ps[myconnectindex].steroids_amount =         400;
                        ps[myconnectindex].boot_amount          =    2000;
                        ps[myconnectindex].shield_amount =           100;
                        ps[myconnectindex].scuba_amount =            6400;
                        ps[myconnectindex].holoduke_amount =         2400;
                        ps[myconnectindex].jetpack_amount =          600;
                        ps[myconnectindex].firstaid_amount =         max_player_health;

                        ps[myconnectindex].got_access =              7;
                        for (key = 0; key < 5; key++)
                            ps[myconnectindex].keys[key] = 1;
                        FTA(5,&ps[myconnectindex]);
                        ps[myconnectindex].cheat_phase = 0;


//                        FTA(21,&ps[myconnectindex]);
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        ps[myconnectindex].inven_icon = 1;
                        return;

                    case 2:
                    case 10:

                        lastlevel = 0;
                        if(k == 2)
                        {
                            short volnume,levnume;
                            volnume = cheatbuf[6] - '0';
                            levnume = (cheatbuf[7] - '0')*10+(cheatbuf[8]-'0');

                            volnume--;
                            levnume--;
                            if(volnume > 4)
                            {
                                ps[myconnectindex].cheat_phase = 0;
                                KB_FlushKeyboardQueue();
                                return;
                            }
                            else

                            if(volnume == 0)
                            {
                                if(levnume > 7)
                                {
                                    ps[myconnectindex].cheat_phase = 0;
                                    KB_FlushKeyboardQueue();
                                    return;
                                }
                            }
                            else
                            {
                                if(levnume >= 8)
                                {
                                    ps[myconnectindex].cheat_phase = 0;
                                    KB_FlushKeyboardQueue();
                                    return;
                                }
                            }

                            ud.m_volume_number = ud.volume_number = volnume;
                            ud.m_level_number = ud.level_number = levnume;

                        }
                        else ud.m_player_skill = ud.player_skill =
                            cheatbuf[5] - '1';

                        if(numplayers > 1 && myconnectindex == connecthead)
                        {
                            tempbuf[0] = 5;
                            tempbuf[1] = ud.m_level_number;
                            tempbuf[2] = ud.m_volume_number;
                            tempbuf[3] = ud.m_player_skill;
                            tempbuf[4] = ud.m_monsters_off;
                            tempbuf[5] = ud.m_respawn_monsters;
                            tempbuf[6] = ud.m_respawn_items;
                            tempbuf[7] = ud.m_respawn_inventory;
                            tempbuf[8] = ud.m_coop;
                            tempbuf[9] = ud.m_marker;
                            tempbuf[10] = ud.m_ffire;

                            for(i=connecthead;i>=0;i=connectpoint2[i])
                                sendpacket(i,tempbuf,11);
                        }
                        else
                        {
                            ps[myconnectindex].gm |= MODE_RESTART;
                            FX_StopAllSounds();
                            clearsoundlocks();
                            FX_SetReverb(0);
                        }

                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;

                    case 3:
                        ps[myconnectindex].cheat_phase = 0;
                        ud.coords = 1-ud.coords;
                        KB_FlushKeyboardQueue();
                        return;

                    case 4:
#ifdef RRRA
                        if (ps[myconnectindex].OnMotorcycle == 0 && ps[myconnectindex].OnBoat == 0)
                        {
                            if( ps[myconnectindex].over_shoulder_on )
                                ps[myconnectindex].over_shoulder_on = 0;
                            else
                            {
                                ps[myconnectindex].over_shoulder_on = 1;
                                cameradist = 0;
                                cameraclock = totalclock;
                            }
                            FTA(22,&ps[myconnectindex]);
                        }
#else
                        if( ps[myconnectindex].over_shoulder_on )
                            ps[myconnectindex].over_shoulder_on = 0;
                        else
                        {
                            ps[myconnectindex].over_shoulder_on = 1;
                            cameradist = 0;
                            cameraclock = totalclock;
                        }
                        FTA(22,&ps[myconnectindex]);
#endif
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;
                    case 5:

                        FTA(21,&ps[myconnectindex]);
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;
                    case 6:
                        for(i=numsectors-1;i>=0;i--) //Unlock
                        {
                            j = sector[i].lotag;
                            if(j == -1 || j == 32767) continue;
                            if( (j & 0x7fff) > 2 )
                            {
                                if( j&(0xffff-16384) )
                                    sector[i].lotag &= (0xffff-16384);
                                operatesectors(i,ps[myconnectindex].i);
                            }
                        }
                        operateforcefields(ps[myconnectindex].i,-1);

                        FTA(100,&ps[myconnectindex]);
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;

                    case 7:
                        ud.cashman = 1-ud.cashman;
                        KB_ClearKeyDown(sc_N);
                        ps[myconnectindex].cheat_phase = 0;
                        return;
                    case 8:

                        ps[myconnectindex].steroids_amount =         400;
                        ps[myconnectindex].boot_amount          =    2000;
                        ps[myconnectindex].shield_amount =           100;
                        ps[myconnectindex].scuba_amount =            6400;
                        ps[myconnectindex].holoduke_amount =         2400;
                        ps[myconnectindex].jetpack_amount =          600;

                        ps[myconnectindex].firstaid_amount =         max_player_health;
                        ps[myconnectindex].got_access =              7;
                        for (key = 0; key < 5; key++)
                            ps[myconnectindex].keys[key] = 1;
                        FTA(5,&ps[myconnectindex]);
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;
                    case 17: // SHOW ALL OF THE MAP TOGGLE;
                        ud.showallmap = 1-ud.showallmap;
                        if(ud.showallmap)
                        {
                            for(i=0;i<(MAXSECTORS>>3);i++)
                                show2dsector[i] = 255;
                            for(i=0;i<(MAXWALLS>>3);i++)
                                show2dwall[i] = 255;
                            FTA(111,&ps[myconnectindex]);
                        }
                        else
                        {
                            for(i=0;i<(MAXSECTORS>>3);i++)
                                show2dsector[i] = 0;
                            for(i=0;i<(MAXWALLS>>3);i++)
                                show2dwall[i] = 0;
                            FTA(1,&ps[myconnectindex]);
                        }
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;

                    case 16:
                        FTA(99,&ps[myconnectindex]);
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;
                    case 9:
                        ud.tickrate = !ud.tickrate;
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;
                    case 11:
                        FTA(105,&ps[myconnectindex]);
                        KB_ClearKeyDown(sc_H);
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;
                    case 12:
                        ps[myconnectindex].steroids_amount = 399;
                        ps[myconnectindex].cheat_phase = 0;
                        FTA(37,&ps[myconnectindex]);
                        KB_FlushKeyboardQueue();
                        return;
                    case 13:
                        if(actor_tog == 3) actor_tog = 0;
                        actor_tog++;
                        ps[screenpeek].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;
#ifdef RRRA
                    case 25:
                        OnMotorcycle(&ps[myconnectindex],0);
                        ps[myconnectindex].ammo_amount[RA13_WEAPON] = max_ammo_amount[RA13_WEAPON];
                        FTA(126,&ps[myconnectindex]);
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;
                    case 26:
                        quickkill(&ps[myconnectindex]);
                        FTA(127,&ps[myconnectindex]);
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;
                    case 28:
                        if (numplayers < 2 && ps[myconnectindex].gm == MODE_GAME && cdon)
                        {
                            rbstop();
                            rbPlayTrack(10);
                        }
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;
                    case 35:
                        if (ps[myconnectindex].raat5d9)
                            ps[myconnectindex].raat5d9 = 0;
                        else
                            ps[myconnectindex].raat5d9 = 1;
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;
                    case 29:
                        ud.god = 0;
                        for (weapon = KNEE_WEAPON; weapon < MAX_WEAPONS; weapon++)
                        {
                            ps[myconnectindex].gotweapon[weapon] = 0;
                        }
                        ps[myconnectindex].curr_weapon = KNEE_WEAPON;
                        ps[myconnectindex].nocheat = 1;
                        sprite[ps[myconnectindex].i].extra = 1;
                        FTA(128,&ps[myconnectindex]);
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;
                    case 30:
                        if (ps[myconnectindex].DrugMode)
                            ps[myconnectindex].DrugMode = 0;
                        else
                            ps[myconnectindex].DrugMode = 5;
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;
                    case 31:
                        ps[myconnectindex].nocheat = 1;
                        FTA(130,&ps[myconnectindex]);
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;
                    case 27:
                    case 38:
                    case 39:
                        if (k == 27)
                        {
                            ps[myconnectindex].raat607 = 2;
                        }
                        else if (k == 39)
                        {
                            ps[myconnectindex].raat607 = 3;
                        }
                        else
                        {
                            for (i = 0; i < 7; i++)
                            {
                                short spr;

                                spr = spawn(ps[screenpeek].i,HEN);
                                sprite[spr].pal = 1;
                                sprite[spr].xrepeat = sprite[spr].xrepeat<<2;
                                sprite[spr].yrepeat = sprite[spr].yrepeat<<2;
                            }
                            FTA(139,&ps[myconnectindex]);
                        }
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;
                    case 32:
                        if (ps[myconnectindex].drink_amt)
                        {
                            ps[myconnectindex].drink_amt = 0;
                            FTA(132,&ps[myconnectindex]);
                        }
                        else
                        {
                            ps[myconnectindex].drink_amt = 90;
                            FTA(131,&ps[myconnectindex]);
                        }
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;
                    case 33:
                        addammo(PISTOL_WEAPON,&ps[myconnectindex],max_ammo_amount[weapon]);
                        for (weapon = PISTOL_WEAPON; weapon < MAX_WEAPONS; weapon++)
                        {
                            ps[myconnectindex].gotweapon[weapon] = 1;
                        }

                        for (weapon = PISTOL_WEAPON; weapon < MAX_WEAPONS-j; weapon++)
                        {
                            ps[myconnectindex].ammo_amount[weapon] = 66;
                        }

                        ps[myconnectindex].ammo_amount[RA15_WEAPON] = 1;

                        ps[myconnectindex].steroids_amount = 400;
                        ps[myconnectindex].boot_amount = 2000;
                        ps[myconnectindex].shield_amount = 100;
                        ps[myconnectindex].scuba_amount = 6400;
                        ps[myconnectindex].holoduke_amount = 2400;
                        ps[myconnectindex].jetpack_amount = 600;
                        ps[myconnectindex].firstaid_amount = max_player_health;
                        ps[myconnectindex].got_access = 7;
                        for (key = 0; key < 5; key++)
                        {
                            ps[myconnectindex].keys[key] = 1;
                        }

                        FTA(5,&ps[myconnectindex]);
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;
                    case 34:
                        if (ps[myconnectindex].raat5dd)
                        {
                            ps[myconnectindex].raat5dd = 0;
                            FTA(129,&ps[myconnectindex]);
                        }
                        else
                        {
                            ps[myconnectindex].raat5dd = 1;
                            FTA(137, &ps[myconnectindex]);
                        }
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;
                    case 36:
                    case 37:
                        OnBoat(&ps[myconnectindex],0);
                        ps[myconnectindex].ammo_amount[RA14_WEAPON] = max_ammo_amount[RA14_WEAPON];
                        FTA(136,&ps[myconnectindex]);
                        ps[myconnectindex].cheat_phase = 0;
                        return;
#endif
                }
             }
          }
       }

    else
    {
        if( KB_KeyPressed(sc_R) )
        {
            if( ps[myconnectindex].cheat_phase >= 0 && numplayers < 2 && ud.recstat == 0)
                ps[myconnectindex].cheat_phase = -1;
        }

        if( KB_KeyPressed(sc_D) )
        {
            if( ps[myconnectindex].cheat_phase == -1 )
            {
                if(ud.player_skill == 4)
                {
                    FTA(22,&ps[myconnectindex]);
                    ps[myconnectindex].cheat_phase = 0;
                }
                else
                {
                    ps[myconnectindex].cheat_phase = 1;
//                    FTA(25,&ps[myconnectindex]);
                    cheatbuflen = 0;
                }
                KB_FlushKeyboardQueue();
            }
            else if(ps[myconnectindex].cheat_phase != 0)
            {
                ps[myconnectindex].cheat_phase = 0;
                KB_ClearKeyDown(sc_R);
                KB_ClearKeyDown(sc_D);
            }
        }
    }
}


long nonsharedtimer;
void nonsharedkeys(void)
{
    short i,ch, weapon;
    long j;
        
    if(ud.recstat == 2)
    {
        ControlInfo noshareinfo;
        CONTROL_GetInput( &noshareinfo );
    }

    if( KB_KeyPressed( sc_F12 ) )
    {
        KB_ClearKeyDown( sc_F12 );
        screencapture("RDNK0000.pcx",0);
        FTA(103,&ps[myconnectindex]);
    }

    if( !ALT_IS_PRESSED && ud.overhead_on == 0)
        {
            if( BUTTON( gamefunc_Enlarge_Screen ) )
            {
                CONTROL_ClearButton( gamefunc_Enlarge_Screen );
                if(ud.screen_size > 0)
                    sound(341);
                ud.screen_size -= 4;
                vscrn();
            }
            if( BUTTON( gamefunc_Shrink_Screen ) )
            {
                CONTROL_ClearButton( gamefunc_Shrink_Screen );
                if(ud.screen_size < 64) sound(341);
                ud.screen_size += 4;
                vscrn();
            }
        }

    if( ps[myconnectindex].cheat_phase == 1 || ps[myconnectindex].gm&(MODE_MENU|MODE_TYPE)) return;

    if( BUTTON(gamefunc_See_Coop_View) && ( ud.coop == 1 || ud.recstat == 2) )
    {
        CONTROL_ClearButton( gamefunc_See_Coop_View );
        screenpeek = connectpoint2[screenpeek];
        if(screenpeek == -1) screenpeek = connecthead;
        restorepalette = 1;
    }

    if( ud.multimode > 1 && BUTTON(gamefunc_Show_Opponents_Weapon) )
    {
        CONTROL_ClearButton(gamefunc_Show_Opponents_Weapon);
        ud.showweapons = 1-ud.showweapons;
        FTA(82-ud.showweapons,&ps[screenpeek]);
    }

    if( BUTTON(gamefunc_Toggle_Crosshair) )
    {
        CONTROL_ClearButton(gamefunc_Toggle_Crosshair);
        ud.crosshair = 1-ud.crosshair;
        FTA(21-ud.crosshair,&ps[screenpeek]);
    }

    if(ud.overhead_on && BUTTON(gamefunc_Map_Follow_Mode) )
    {
        CONTROL_ClearButton(gamefunc_Map_Follow_Mode);
        ud.scrollmode = 1-ud.scrollmode;
        if(ud.scrollmode)
        {
            ud.folx = ps[screenpeek].oposx;
            ud.foly = ps[screenpeek].oposy;
            ud.fola = ps[screenpeek].oang;
        }
        FTA(83+ud.scrollmode,&ps[myconnectindex]);
    }

    if( SHIFTS_IS_PRESSED || ALT_IS_PRESSED )
    {
        i = 0;
        if( KB_KeyPressed( sc_F1) ) { KB_ClearKeyDown(sc_F1);i = 1; }
        if( KB_KeyPressed( sc_F2) ) { KB_ClearKeyDown(sc_F2);i = 2; }
        if( KB_KeyPressed( sc_F3) ) { KB_ClearKeyDown(sc_F3);i = 3; }
        if( KB_KeyPressed( sc_F4) ) { KB_ClearKeyDown(sc_F4);i = 4; }
        if( KB_KeyPressed( sc_F5) ) { KB_ClearKeyDown(sc_F5);i = 5; }
        if( KB_KeyPressed( sc_F6) ) { KB_ClearKeyDown(sc_F6);i = 6; }
        if( KB_KeyPressed( sc_F7) ) { KB_ClearKeyDown(sc_F7);i = 7; }
        if( KB_KeyPressed( sc_F8) ) { KB_ClearKeyDown(sc_F8);i = 8; }
        if( KB_KeyPressed( sc_F9) ) { KB_ClearKeyDown(sc_F9);i = 9; }
        if( KB_KeyPressed( sc_F10) ) {KB_ClearKeyDown(sc_F10);i = 10; }

        if(i)
        {
            if(SHIFTS_IS_PRESSED)
            {
                if(i == 5 && ps[myconnectindex].fta > 0 && ps[myconnectindex].ftq == 26)
                {
                    music_select++;
                    if(music_select == 44) music_select = 0;
                    strcpy(&tempbuf[0],"PLAYING ");
                    strcat(&tempbuf[0],&music_fn[0][music_select][0]);
                    playmusic(&music_fn[0][music_select][0]);
                    strcpy(&fta_quotes[26][0],&tempbuf[0]);
                    FTA(26,&ps[myconnectindex]);
                    return;
                }
                
                user_quote = ud.ridecule[i-1];
                user_quote_time = 180;

                ch = 0;

                tempbuf[ch] = 4;
                tempbuf[ch+1] = 0;
                strcat(tempbuf+1,ud.ridecule[i-1]);

                i = 1+strlen(ud.ridecule[i-1]);

                if(ud.multimode > 1)
                    for(ch=connecthead;ch>=0;ch=connectpoint2[ch])
                        if (ch != myconnectindex)
                            sendpacket(ch,tempbuf,i);

                pus = NUMPAGES;
                pub = NUMPAGES;

                return;

            }

            if(ud.lockout == 0)
                if(ALT_IS_PRESSED && ( RTS_NumSounds() > 0 ) && rtsplaying == 0 && VoiceToggle )
            {
                rtsptr = (char *)RTS_GetSound (i-1);
                if(*rtsptr == 'C')
                    FX_PlayVOC3D( rtsptr,0,0,0,255,-i);
                else FX_PlayWAV3D( rtsptr,0,0,0,255,-i);

                rtsplaying = 7;

                if(ud.multimode > 1)
                {
                    tempbuf[0] = 7;
                    tempbuf[1] = i;

                    for(ch=connecthead;ch>=0;ch=connectpoint2[ch])
                        if(ch != myconnectindex)
                            sendpacket(ch,tempbuf,2);
                }

                pus = NUMPAGES;
                pub = NUMPAGES;

                return;
            }
        }
    }

    if(!ALT_IS_PRESSED && !SHIFTS_IS_PRESSED)
    {

        if( ud.multimode > 1 && BUTTON(gamefunc_SendMessage) )
        {
            KB_FlushKeyboardQueue();
            CONTROL_ClearButton( gamefunc_SendMessage );
            ps[myconnectindex].gm |= MODE_TYPE;
            typebuf[0] = 0;
            inputloc = 0;
        }

        if( KB_KeyPressed(sc_F1) || ( ud.show_help && ( KB_KeyPressed(sc_Space) || KB_KeyPressed(sc_Enter) || KB_KeyPressed(sc_kpad_Enter) ) ) )
        {
            KB_ClearKeyDown(sc_F1);
            KB_ClearKeyDown(sc_Space);
            KB_ClearKeyDown(sc_kpad_Enter);
            KB_ClearKeyDown(sc_Enter);
            ud.show_help ++;

#ifdef RRRA
            if( ud.show_help > 3 )
#else
            if( ud.show_help > 2 )
#endif
            {
                ud.show_help = 0;
                if(ud.multimode < 2 && ud.recstat != 2) ready2send = 1;
                vscrn();
            }
            else
            {
                setview(0,0,xdim-1,ydim-1);
                if(ud.multimode < 2 && ud.recstat != 2)
                {
                    ready2send = 0;
                    totalclock = ototalclock;
                }
            }
        }

//        if(ud.multimode < 2)
        {
            if(ud.recstat != 2 && KB_KeyPressed( sc_F2 ) )
            {
                KB_ClearKeyDown( sc_F2 );

                if(movesperpacket == 4 && connecthead != myconnectindex)
                    return;

                FAKE_F2:
                if(sprite[ps[myconnectindex].i].extra <= 0)
                {
                    FTA(118,&ps[myconnectindex]);
                    return;
                }
                cmenu(350);
                screencapt = 1;
                displayrooms(myconnectindex,65536);
                savetemp("redneck.tmp",waloff[MAXTILES-1],160*100);
                screencapt = 0;
                FX_StopAllSounds();
                clearsoundlocks();

//                setview(0,0,xdim-1,ydim-1);
                ps[myconnectindex].gm |= MODE_MENU;

                if(ud.multimode < 2)
                {
                    ready2send = 0;
                    totalclock = ototalclock;
                    screenpeek = myconnectindex;
                }
            }

            if(KB_KeyPressed( sc_F3 ))
            {
                KB_ClearKeyDown( sc_F3 );

                if(movesperpacket == 4 && connecthead != myconnectindex)
                    return;

                cmenu(300);
                FX_StopAllSounds();
                clearsoundlocks();

//                setview(0,0,xdim-1,ydim-1);
                ps[myconnectindex].gm |= MODE_MENU;
                if(ud.multimode < 2 && ud.recstat != 2)
                {
                    ready2send = 0;
                    totalclock = ototalclock;
                }
                screenpeek = myconnectindex;
            }
        }

        if(KB_KeyPressed( sc_F4 ) && FXDevice != NumSoundCards )
        {
            KB_ClearKeyDown( sc_F4 );
            FX_StopAllSounds();
            clearsoundlocks();

            ps[myconnectindex].gm |= MODE_MENU;
            if(ud.multimode < 2 && ud.recstat != 2)
            {
                ready2send = 0;
                totalclock = ototalclock;
            }
            cmenu(700);

        }

        if( KB_KeyPressed( sc_F6 ) && (ps[myconnectindex].gm&MODE_GAME))
        {
            KB_ClearKeyDown( sc_F6 );

            if(movesperpacket == 4 && connecthead != myconnectindex)
                return;

            if(lastsavedpos == -1) goto FAKE_F2;

            KB_FlushKeyboardQueue();

            if(sprite[ps[myconnectindex].i].extra <= 0)
            {
                FTA(118,&ps[myconnectindex]);
                return;
            }
            screencapt = 1;
            displayrooms(myconnectindex,65536);
            savetemp("redneck.tmp",waloff[MAXTILES-1],160*100);
            screencapt = 0;
            if( lastsavedpos >= 0 )
            {
                inputloc = strlen(&ud.savegame[lastsavedpos][0]);
                current_menu = 360+lastsavedpos;
                probey = lastsavedpos;
            }
            FX_StopAllSounds();
            clearsoundlocks();

            setview(0,0,xdim-1,ydim-1);
            ps[myconnectindex].gm |= MODE_MENU;
            if(ud.multimode < 2 && ud.recstat != 2)
            {
                ready2send = 0;
                totalclock = ototalclock;
            }
        }

        if(KB_KeyPressed( sc_F7 ) )
        {
            KB_ClearKeyDown(sc_F7);
#ifdef RRRA
            if (ps[myconnectindex].OnMotorcycle == 0 && ps[myconnectindex].OnBoat == 0)
            {
                if( ps[myconnectindex].over_shoulder_on )
                    ps[myconnectindex].over_shoulder_on = 0;
                else
                {
                    ps[myconnectindex].over_shoulder_on = 1;
                    cameradist = 0;
                    cameraclock = totalclock;
                }
                FTA(109+ps[myconnectindex].over_shoulder_on,&ps[myconnectindex]);
            }
#else
            if( ps[myconnectindex].over_shoulder_on )
                ps[myconnectindex].over_shoulder_on = 0;
            else
            {
                ps[myconnectindex].over_shoulder_on = 1;
                cameradist = 0;
                cameraclock = totalclock;
            }
            FTA(109+ps[myconnectindex].over_shoulder_on,&ps[myconnectindex]);
#endif
        }

        if( KB_KeyPressed( sc_F5 ) && MusicDevice != NumSoundCards )
        {
            KB_ClearKeyDown( sc_F5 );
            strcpy(&tempbuf[0],&music_fn[0][music_select][0]);
            strcat(&tempbuf[0],".  USE SHIFT-F5 TO CHANGE.");
            strcpy(&fta_quotes[26][0],&tempbuf[0]);
            FTA(26,&ps[myconnectindex]);

        }

        if(KB_KeyPressed( sc_F8 ))
        {
            KB_ClearKeyDown( sc_F8 );
            ud.fta_on = !ud.fta_on;
            if(ud.fta_on) FTA(23,&ps[myconnectindex]);
            else
            {
                ud.fta_on = 1;
                FTA(24,&ps[myconnectindex]);
                ud.fta_on = 0;
            }
        }

        if(KB_KeyPressed( sc_F9 ) && (ps[myconnectindex].gm&MODE_GAME) )
        {
            KB_ClearKeyDown( sc_F9 );

            if(movesperpacket == 4 && myconnectindex != connecthead)
                return;

            if( lastsavedpos >= 0 ) cmenu(15001);
            else cmenu(25000);
            FX_StopAllSounds();
            clearsoundlocks();
            ps[myconnectindex].gm |= MODE_MENU;
            if(ud.multimode < 2 && ud.recstat != 2)
            {
                ready2send = 0;
                totalclock = ototalclock;
            }
        }

        if(KB_KeyPressed( sc_F10 ))
        {
            KB_ClearKeyDown( sc_F10 );
            cmenu(500);
            FX_StopAllSounds();
            clearsoundlocks();
            ps[myconnectindex].gm |= MODE_MENU;
            if(ud.multimode < 2 && ud.recstat != 2)
            {
                ready2send = 0;
                totalclock = ototalclock;
            }
        }

        
        if( ud.overhead_on != 0)
        {

            j = totalclock-nonsharedtimer; nonsharedtimer += j;
            if ( BUTTON( gamefunc_Enlarge_Screen ) )
                ps[myconnectindex].zoom += mulscale6(j,max(ps[myconnectindex].zoom,256));
            if ( BUTTON( gamefunc_Shrink_Screen ) )
                ps[myconnectindex].zoom -= mulscale6(j,max(ps[myconnectindex].zoom,256));

            if( (ps[myconnectindex].zoom > 2048) )
                ps[myconnectindex].zoom = 2048;
            if( (ps[myconnectindex].zoom < 48) )
                ps[myconnectindex].zoom = 48;

        }
    }

    if( KB_KeyPressed(sc_Escape) && ud.overhead_on && ps[myconnectindex].newowner == -1 )
    {
        KB_ClearKeyDown( sc_Escape );
        ud.last_overhead = ud.overhead_on;
        ud.overhead_on = 0;
        ud.scrollmode = 0;
        vscrn();
    }

    if( BUTTON(gamefunc_AutoRun) )
    {
#ifdef RRRA
        if (ps[myconnectindex].OnMotorcycle == 0 && ps[myconnectindex].OnBoat == 0)
        {
            CONTROL_ClearButton(gamefunc_AutoRun);
            ud.auto_run = 1-ud.auto_run;
            FTA(85+ud.auto_run,&ps[myconnectindex]);
        }
#else
        CONTROL_ClearButton(gamefunc_AutoRun);
        ud.auto_run = 1-ud.auto_run;
        FTA(85+ud.auto_run,&ps[myconnectindex]);
#endif
    }

    if( BUTTON(gamefunc_Map) )
    {
        CONTROL_ClearButton( gamefunc_Map );
        if( ud.last_overhead != ud.overhead_on && ud.last_overhead)
        {
            ud.overhead_on = ud.last_overhead;
            ud.last_overhead = 0;
        }
        else
        {
            ud.overhead_on++;
            if(ud.overhead_on == 3 ) ud.overhead_on = 0;
            ud.last_overhead = ud.overhead_on;
        }
        restorepalette = 1;
        vscrn();
    }

    if(KB_KeyPressed( sc_F11 ))
    {
        KB_ClearKeyDown( sc_F11 );
        if(SHIFTS_IS_PRESSED) ud.brightness-=4;
        else ud.brightness+=4;

        if (ud.brightness > (7<<2) )
            ud.brightness = 0;
        else if(ud.brightness < 0)
            ud.brightness = (7<<2);

        setbrightness(ud.brightness>>2,&ps[myconnectindex].palette[0]);
        if(ud.brightness < 20) FTA( 29 + (ud.brightness>>2) ,&ps[myconnectindex]);
        else if(ud.brightness < 40) FTA( 96 + (ud.brightness>>2) - 5,&ps[myconnectindex]);
    }
}
