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
#include "mouse.h"
#include "animlib.h"


extern short inputloc;
extern int recfilep;
extern char vgacompatible;
extern short probey, lastprobey, last_menu, globalskillsound;
extern short sh, onbar, buttonstat, deletespot;
extern short last_zero, last_fifty, last_threehundred;
extern char fileselect, menunamecnt, menuname[256][17], curpath[80], menupath[80];

#define LMB (buttonstat&1)
#define RMB (buttonstat&2)

extern ControlInfo minfo;

#define SHX(X) 0
// ((x==X)*(-sh))
#define PHX(X) 0
// ((x==X)?1:2)
#define MWIN(X) rotatesprite( 320<<15,200<<15,X,0,MENUSCREEN,-16,0,10+64,0,0,xdim-1,ydim-1)
#define MWINXY(X,OX,OY) rotatesprite( ( 320+(OX) )<<15, ( 200+(OY) )<<15,X,0,MENUSCREEN,-16,0,10+64,0,0,xdim-1,ydim-1)

extern int32 volnum,levnum,plrskl,numplr;
extern short lastsavedpos;
long quittimer = 0;

#ifdef RRRA
short word_1D7EF8;
#endif

void menus(void)
{
    short c,x;
    volatile long l;
    int tenerr, unk, tx, ty;

    unk = 0;

    if(ControllerType == 1 && CONTROL_MousePresent)
    {
        if(buttonstat != 0 && !onbar)
        {
            x = MOUSE_GetButtons()<<3;
            if( x ) buttonstat = x<<3;
            else buttonstat = 0;
        }
        else
            buttonstat = MOUSE_GetButtons();
    }
    else buttonstat = 0;

    if( (ps[myconnectindex].gm&MODE_MENU) == 0 )
    {
        walock[MAXTILES-3] = 1;
        return;
    }

    ps[myconnectindex].gm &= (0xff-MODE_TYPE);
    ps[myconnectindex].fta = 0;

    x = 0;

    if (numplayers > 1)
    {
        getpackets();
        if (ps[myconnectindex].gm == MODE_GAME)
            return;
    }

    sh = 4-(sintable[(totalclock<<4)&2047]>>11);
    
    if(current_menu < 1000 || current_menu > 2999)
        if(!(current_menu >= 300 && current_menu <= 369))
            vscrn();

    switch(current_menu)
    {
        case 25000:
            gametext(160,100,"SELECT A SAVE SPOT BEFORE",0);
            gametext(160,110,"YOU QUICK RESTORE.",0);

            x = probe(186,124,0,0,0);
            if(x >= -1)
            {
                if(ud.multimode < 2 && ud.recstat != 2)
                {
                    ready2send = 1;
                    totalclock = ototalclock;
                }
                ps[myconnectindex].gm &= ~MODE_MENU;
            }
            break;

        case 20000:
            x = probe(326,190,0,0,0);
            gametext(160,50-8,"THIS HYAR PREVIEW VERSION OF",0);
            gametext(160,59-8,"REDNECK RAMPAGE IS NEARLY 'BOUT",0);
            gametext(160,68-8,"ONE SMALL SWIG OF THE WHOLE DAMN",0);
            gametext(160,77-8,"ENCHILADAR.  SO QUIT YER' WHININ' AND",0);
            gametext(160,86-8,"COMMENCE TA KILLIN'!  'CAUSE THIS",0);
            gametext(160,95-8,"SHORE OUGHTA BE ENUF TO WHET YER'",0);
            gametext(160,104-8,"APPETITE!  ",0);

            gametext(160,113+8,"IF YOU WANT THE WHOLE DAMN THING,",0);
            gametext(160,122+8,"CALL THEM OL' BOYS OVER TH'AR AT",0);
            gametext(160,131+8,"YER' INTERPLAY DIE-RECT AT",0);
            gametext(160,140+8,"1-800-INTERPLAY...",0);
            gametext(160,149+16,"HIT YER' ANY KEY RE-COMMENCE TO DEMO'N!",0);

            if( x >= -1 ) cmenu(100);
            break;
        case 20001:
            x = probe(161,144,0,0,0);
            gametext(160,56,"PLAY REDNECK RAMPAGE OVER THE INTERNET!",0);
            gametext(160,66,"IF YA'S TIRED OF PLAYING WITH YERSELF,",0);
            gametext(160,76,"GET YER INTERNET VERSION OF REDNECK",0);
            gametext(160,86,"RAMPAGE AT WWW.ENGAGEGAMES.COM TO BEAT",0);
            gametext(160,96,"THE HELL OUT OF REDNECKS WORLDWIDE!",0);
            gametext(160,116,"WHACK YER ESC KEY...",0);
            if(x >= -1) cmenu(0);
            break;

        case 20002:
            x = probe(161,144,0,0,0);
            gametext(160,78,"YOUR .INI FILE FOR REDNECK IS BAD",0);
            gametext(160,118,"PRESS ANY KEY...",0);
            if(x >= -1) cmenu(0);
            break;
        case 20003:
            x = probe(161,144,0,0,0);
            gametext(160,78,"YOUR .INI FILE FOR ENGAGE IS BAD",0);
            gametext(160,118,"PRESS ANY KEY...",0);
            if(x >= -1) cmenu(0);
            break;
        case 20004:
            x = probe(161,144,0,0,0);
            gametext(160,78,"BROWSE CANCEL",0);
            gametext(160,118,"PRESS ANY KEY...",0);
            if(x >= -1) cmenu(0);
            break;
        case 20005:
            x = probe(161, 144, 0, 0, 0);
            gametext(160,78,"GET THE LATEST ENGAGE SOFTWARE AT",0);
            gametext(160,86,"HTTP://WWW.INTERPLAY.COM",0);
            gametext(160,118,"PRESS ANY KEY...",0);
            if(x >= -1) cmenu(0);
            break;

        case 15001:
        case 15000:

            gametext(160,60,"LOAD last game:",0);

            sprintf(tempbuf,"\"%s\"",ud.savegame[lastsavedpos]);
            gametext(160,70,tempbuf,0);

            gametext(160,80,"(Y/N)",0);

            if( KB_KeyPressed(sc_Escape) || KB_KeyPressed(sc_N) || RMB)
            {
                if(sprite[ps[myconnectindex].i].extra <= 0)
                {
                    enterlevel(MODE_GAME);
                    return;
                }

                KB_ClearKeyDown(sc_N);
                KB_ClearKeyDown(sc_Escape);

                ps[myconnectindex].gm &= ~MODE_MENU;
                if(ud.multimode < 2 && ud.recstat != 2)
                {
                    ready2send = 1;
                    totalclock = ototalclock;
                }
            }

            if(  KB_KeyPressed(sc_Enter) || KB_KeyPressed(sc_kpad_Enter) || KB_KeyPressed(sc_Y) || LMB )
            {
                KB_FlushKeyboardQueue();
                FX_StopAllSounds();
                clearsoundlocks();

                if(ud.multimode > 1)
                {
                    loadplayer(-1-lastsavedpos);
                }
                else
                {
                    loadplayer(lastsavedpos);
                }
            }

            probe(186,124+9,0,0,0);

            break;

        case 10000:
        case 10001:

            c = 60;
            rotatesprite(160<<16,19<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(160,24,0,0,"ADULT MODE");

            x = probe(60,50+16,16,2,0);
            if(x == -1) { cmenu(200); break; }

            menutext(c,50+16,SHX(-2),PHX(-2),"ADULT MODE");
            menutext(c,50+16+16,SHX(-3),PHX(-3),"ENTER PASSWORD");

            if(ud.lockout) menutext(c+160+40,50+16,0,0,"OFF");
            else menutext(c+160+40,50+16,0,0,"ON");

            if(current_menu == 10001)
            {
                gametext(160,50+16+16+16+16-12,"ENTER PASSWORD",0);
                x = strget((320>>1),50+16+16+16+16,buf,19, 998);

                if( x )
                {
                    if(ud.pwlockout[0] == 0 || ud.lockout == 0 )
                        strcpy(&ud.pwlockout[0],buf);
                    else if( strcmp(buf,&ud.pwlockout[0]) == 0 )
                    {
                        ud.lockout = 0;
                        buf[0] = 0;

                        for(x=0;x<numanimwalls;x++)
                            if( wall[animwall[x].wallnum].picnum != W_SCREENBREAK &&
                                wall[animwall[x].wallnum].picnum != W_SCREENBREAK+1 &&
                                wall[animwall[x].wallnum].picnum != W_SCREENBREAK+2 )
                                    if( wall[animwall[x].wallnum].extra >= 0 )
                                        wall[animwall[x].wallnum].picnum = wall[animwall[x].wallnum].extra;

                    }
                    current_menu = 10000;
                    KB_ClearKeyDown(sc_Enter);
                    KB_ClearKeyDown(sc_kpad_Enter);
                    KB_FlushKeyboardQueue();
                }
            }
            else
            {
                if(x == 0)
                {
                    if( ud.lockout == 1 )
                    {
                        if(ud.pwlockout[0] == 0)
                        {
                            ud.lockout = 0;
                            for(x=0;x<numanimwalls;x++)
                            if( wall[animwall[x].wallnum].picnum != W_SCREENBREAK &&
                                wall[animwall[x].wallnum].picnum != W_SCREENBREAK+1 &&
                                wall[animwall[x].wallnum].picnum != W_SCREENBREAK+2 )
                                    if( wall[animwall[x].wallnum].extra >= 0 )
                                        wall[animwall[x].wallnum].picnum = wall[animwall[x].wallnum].extra;
                        }
                        else
                        {
                            buf[0] = 0;
                            current_menu = 10001;
                            inputloc = 0;
                            KB_FlushKeyboardQueue();
                        }
                    }
                    else
                    {
                        ud.lockout = 1;

                        for(x=0;x<numanimwalls;x++)
                            switch(wall[animwall[x].wallnum].picnum)
                            {
                            }
                    }
                }

                else if(x == 1)
                {
                    current_menu = 10001;
                    inputloc = 0;
                    KB_FlushKeyboardQueue();
                }
            }

            break;

        case 1000:
        case 1001:
        case 1002:
        case 1003:
        case 1004:
        case 1005:
        case 1006:
        case 1007:
        case 1008:
        case 1009:
            unk = 1;

            rotatesprite(160<<16,200<<15,65536L,0,MENUSCREEN,16,0,10+64,0,0,xdim-1,ydim-1);
            rotatesprite(160<<16,19<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(160,24,0,0,"LOAD GAME");
            rotatesprite(101<<16,97<<16,65536,512,MAXTILES-3,-32,0,4+10+64,0,0,xdim-1,ydim-1);

            dispnames();

            sprintf(tempbuf,"PLAYERS: %-2d                      ",numplr);
            gametext(160,158,tempbuf,0);

            sprintf(tempbuf,"EPISODE: %-2d / LEVEL: %-2d / SKILL: %-2d",1+volnum,1+levnum,plrskl);
            gametext(160,170,tempbuf,0);

            gametext(160,60,"LOAD game:",0);
            sprintf(tempbuf,"\"%s\"",ud.savegame[current_menu-1000]);
            gametext(160,70,tempbuf,0);
            gametext(160,80,"(Y/N)",0);

            if( KB_KeyPressed(sc_Enter) || KB_KeyPressed(sc_kpad_Enter) || KB_KeyPressed(sc_Y) || LMB )
            {
                KB_FlushKeyboardQueue();
                if(ud.multimode < 2 && ud.recstat != 2)
                {
                    ready2send = 1;
                    totalclock = ototalclock;
                }

                ps[myconnectindex].gm = MODE_GAME;

                if(ud.multimode > 1)
                    loadplayer(-1-(current_menu-1000));
                else
                    loadplayer(current_menu-1000);

                lastsavedpos = current_menu-1000;

                break;
            }
            if( KB_KeyPressed(sc_N) || KB_KeyPressed(sc_Escape) || RMB)
            {
                KB_ClearKeyDown(sc_N);
                KB_ClearKeyDown(sc_Escape);
                sound(341);
                if(ps[myconnectindex].gm&MODE_DEMO) cmenu(300);
                else
                {
                    ps[myconnectindex].gm &= ~MODE_MENU;
                    if(ud.multimode < 2 && ud.recstat != 2)
                    {
                        ready2send = 1;
                        totalclock = ototalclock;
                    }
                }
            }

            probe(186,124+9,0,0,1);

            break;

        case 1500:

            if( KB_KeyPressed(sc_Enter) || KB_KeyPressed(sc_kpad_Enter) || KB_KeyPressed(sc_Y) || LMB )
            {
                KB_FlushKeyboardQueue();
                cmenu(100);
            }
            if( KB_KeyPressed(sc_N) || KB_KeyPressed(sc_Escape) || RMB)
            {
                KB_ClearKeyDown(sc_N);
                KB_ClearKeyDown(sc_Escape);
                if(ud.multimode < 2 && ud.recstat != 2)
                {
                    ready2send = 1;
                    totalclock = ototalclock;
                }
                ps[myconnectindex].gm &= ~MODE_MENU;
                sound(341);
                break;
            }
#ifdef RRRA
            probe(186,134,0,0,0);
#else
            probe(186,124,0,0,0);
#endif
            gametext(160,90,"ABORT this game?",0);
            gametext(160,110,"(Y/N)",0);

            break;

        case 2000:
        case 2001:
        case 2002:
        case 2003:
        case 2004:
        case 2005:
        case 2006:
        case 2007:
        case 2008:
        case 2009:
            unk = 2;

            rotatesprite(160<<16,200<<15,65536L,0,MENUSCREEN,16,0,10+64,0,0,xdim-1,ydim-1);
            rotatesprite(160<<16,19<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(160,24,0,0,"SAVE GAME");

            rotatesprite(101<<16,97<<16,65536L,512,MAXTILES-3,-32,0,4+10+64,0,0,xdim-1,ydim-1);
            sprintf(tempbuf,"PLAYERS: %-2d                      ",ud.multimode);
            gametext(160,158,tempbuf,0);

            sprintf(tempbuf,"EPISODE: %-2d / LEVEL: %-2d / SKILL: %-2d",1+ud.volume_number,1+ud.level_number,ud.player_skill);
            gametext(160,170,tempbuf,0);

            dispnames();

            gametext(160,60,"OVERWRITE previous SAVED game?",0);
            gametext(160,70,"(Y/N)",0);

            if( KB_KeyPressed(sc_Enter) || KB_KeyPressed(sc_kpad_Enter) || KB_KeyPressed(sc_Y) || LMB )
            {
#ifdef RRRA
                KB_ClearKeyDown(sc_Enter);
                KB_ClearKeyDown(sc_Escape);
                KB_ClearKeyDown(sc_Y);
#else
                KB_FlushKeyboardQueue();
#endif
                inputloc = strlen(&ud.savegame[current_menu-2000][0]);

                cmenu(current_menu-2000+360);

                KB_FlushKeyboardQueue();
                break;
            }
            if( KB_KeyPressed(sc_N) || KB_KeyPressed(sc_Escape) || RMB)
            {
                KB_ClearKeyDown(sc_N);
                KB_ClearKeyDown(sc_Escape);
                cmenu(351);
                sound(341);
            }

            probe(186,124,0,0,2);

            break;

        case 960:
        case 961:
        case 962:
        case 963:
        case 964:
        case 965:
        case 966:
        case 967:
        case 968:
        case 969:
        case 970:
        case 971:
        case 972:
        case 973:
        case 974:
        case 975:
        case 976:
        case 977:
        case 978:
        case 979:
        case 980:
        case 981:
        case 982:
#ifdef RRRA
        case 983:
        case 984:
        case 985:
        case 986:
        case 987:
        case 988:
        case 989:
#endif
            c = 320>>1;
            rotatesprite(c<<16,19<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(c,24,0,0,"CREDITS");

            if(KB_KeyPressed(sc_Escape)) { cmenu(0); break; }

            if( KB_KeyPressed( sc_LeftArrow ) ||
                KB_KeyPressed( sc_kpad_4 ) ||
                KB_KeyPressed( sc_UpArrow ) ||
                KB_KeyPressed( sc_PgUp ) ||
                KB_KeyPressed( sc_kpad_8 ) )
            {
                KB_ClearKeyDown(sc_LeftArrow);
                KB_ClearKeyDown(sc_kpad_4);
                KB_ClearKeyDown(sc_UpArrow);
                KB_ClearKeyDown(sc_PgUp);
                KB_ClearKeyDown(sc_kpad_8);

                sound(335);
                current_menu--;
#ifdef RRRA
                if(current_menu < 960) current_menu = 989;
#else
                if(current_menu < 960) current_menu = 982;
#endif
            }
            else if(
                KB_KeyPressed( sc_PgDn ) ||
                KB_KeyPressed( sc_Enter ) ||
                KB_KeyPressed( sc_kpad_Enter ) ||
                KB_KeyPressed( sc_RightArrow ) ||
                KB_KeyPressed( sc_DownArrow ) ||
                KB_KeyPressed( sc_kpad_2 ) ||
                KB_KeyPressed( sc_kpad_9 ) ||
                KB_KeyPressed( sc_kpad_6 ) )
            {
                KB_ClearKeyDown(sc_PgDn);
                KB_ClearKeyDown(sc_Enter);
                KB_ClearKeyDown(sc_RightArrow);
                KB_ClearKeyDown(sc_kpad_Enter);
                KB_ClearKeyDown(sc_kpad_6);
                KB_ClearKeyDown(sc_kpad_9);
                KB_ClearKeyDown(sc_kpad_2);
                KB_ClearKeyDown(sc_DownArrow);
                sound(335);
                current_menu++;
#ifdef RRRA
                if(current_menu > 989) current_menu = 960;
#else
                if(current_menu > 982) current_menu = 960;
#endif
            }

            switch(current_menu)
            {
#ifdef RRRA
                case 960:
                    gametext(c,80,"ORIGINAL CONCEPT, DESIGN AND DIRECTION",0);
                    gametext(c,100,"DREW MARKHAM",0);
                    break;
                case 961:
                    gametext(c,80,"ART DIRECTION AND ADDITIONAL DESIGN",0);
                    gametext(c,100,"CORKY LEHMKUHL",0);
                    break;
                case 962:
                    gametext(c,80,"PRODUCED BY",0);
                    gametext(c,100,"GREG GOODRICH",0);
                    break;
                case 963:
                    gametext(c,80,"GAME PROGRAMMING",0);
                    gametext(c,100,"JOSEPH AURILI",0);
                    break;
                case 964:
                    gametext(c,80,"ORIGINAL GAME PROGRAMMING",0);
                    gametext(c,100,"RAFAEL PAIZ",0);
                    break;
                case 965:
                    gametext(c,80,"LEVEL DESIGN",0);
                    gametext(c,100,"RHETT BALDWIN & AARON BARBER",0);
                    break;
                case 966:
                    gametext(c,80,"ORIGINAL ART DIRECTION AND SUPPORT",0);
                    gametext(c,100,"MAXX KAUFMAN & CLAIRE PRADERIE-MARKHAM",0);
                    break;
                case 967:
                    gametext(c,80,"COMPUTER GRAPHICS SUPERVISOR &",0);
                    gametext(c,90,"CHARACTER ANIMATION DIRECTION",0);
                    gametext(c,110,"BARRY DEMPSEY",0);
                    break;
                case 968:
                    gametext(c,80,"SENIOR ANIMATOR & MODELER",0);
                    gametext(c,100,"JASON HOOVER",0);
                    break;
                case 969:
                    gametext(c,80,"CHARACTER ANIMATION &",0);
                    gametext(c,90,"MOTION CAPTURE SPECIALIST",0);
                    gametext(c,110,"AMIT DORON",0);
                    break;
                case 970:
                    gametext(c,80,"SOUND DESIGN &",0);
                    gametext(c,90,"MUSIC PRODUCTION COORDINATION",0);
                    gametext(c,110,"GARY BRADFIELD",0);
                    break;
                case 971:
                    gametext(c,80,"INTRODUCTION ANIMATION",0);
                    gametext(c,100,"DOMINIQUE DROZDZ",0);
                    break;
                case 972:
                    gametext(c,80,"ARTIST",0);
                    gametext(c,100,"MATTHIAS BEEGUER",0);
                    break;
                case 973:
                    gametext(c,80,"ADDITIONAL ART",0);
                    gametext(c,100,"VIKTOR ANTONOV",0);
                    break;
                case 974:
                    gametext(c,80,"PRODUCTION COORDINATOR",0);
                    gametext(c,100,"VICTORIA SYLVESTER",0);
                    break;
                case 975:
                    gametext(c,40,"CHARACTER VOICES",0);
                    gametext(c,60,"LEONARD",0);
                    gametext(c,70,"BURTON GILLIAM",0);
                    gametext(c,90,"DAISY MAE",0);
                    gametext(c,100,"TARA CHARENDOFF",0);
                    gametext(c,120,"BUBBA, BILLY RAY, SKINNY OL' COOT,",0);
                    gametext(c,130,"FRANK THE BIKER, THE TURD MINION",0);
                    gametext(c,140,"& ALL OTHER VARIOUS RAMBLINGS...",0);
                    gametext(c,150,"DREW MARKHAM",0);
                    break;
                case 976:
                    gametext(c,70,"SPECIAL APPEARENCE BY",0);
                    gametext(c,90,"SHERIFF LESTER T. HOBBES",0);
                    gametext(c,100,"MOJO NIXON",0);
                    gametext(c,120,"ALIEN VIXEN",0);
                    gametext(c,130,"PEGGY JO JACOBS",0);
                    break;
                case 977:
                    gametext(c,70,"REDNECK RAMPAGE TITLE TRACK & CYBERSEX",0);
                    gametext(c,80,"WRITTEN & PERFORMED BY",0);
                    gametext(c,90,"MOJO NIXON",0);
                    gametext(c,110,"(c) MUFFIN'STUFFIN' MUSIC (BMI)",0);
                    gametext(c,120,"ADMINISTERED BY BUG.",0);
                    break;
                case 978:
                    gametext(c,60,"MUSIC",0);
                    gametext(c,80,"DISGRACELAND",0);
                    gametext(c,90,"TINY D & THE SOFA KINGS",0);
                    gametext(c,110,"BANJO AND GUITAR PICKIN",0);
                    gametext(c,120,"JOHN SCHLOCKER",0);
                    gametext(c,130,"HOWARD YEARWOOD",0);
                    break;
                case 979:
                    gametext(c,80,"RECORDING ENGINEER",0);
                    gametext(c,90,"DAVE AHLERT",0);
                    gametext(c,110,"RECORDING ASSISTANCE",0);
                    gametext(c,120,"JEFF GILBERT",0);
                    break;
                case 980:
                    gametext(c,80,"MOTION CAPTURE ACTOR",0);
                    gametext(c,90,"J.P. MANOUX",0);
                    gametext(c,110,"MOTION CAPTURE ACTRESS",0);
                    gametext(c,120,"SHAWN WOLFE",0);
                    break;
                case 981:
                    gametext(c,50,"THIS GAME COULD NOT HAVE BEEN MADE WITHOUT",0);
                    gametext(c,60,"ALEX MAYBERRY",0);
                    gametext(c,70,"MAL BLACKWELL",0);
                    gametext(c,90,"NUTS AND BOLTS",0);
                    gametext(c,100,"STEVE GOLDBERG",0);
                    gametext(c,120,"BEAN COUNTING",0);
                    gametext(c,130,"MAX YOSHIKAWA",0);
                    gametext(c,150,"ADMINISTRATIVE ASSISTANCE",0);
                    gametext(c,160,"MINERVA MAYBERRY",0);
                    break;
                case 982:
                    gametext(c,60,"FOR INTERPLAY",0);
                    gametext(c,80,"PRODUCER",0);
                    gametext(c,90,"BILL DUGAN",0);
                    gametext(c,110,"LINE PRODUCER",0);
                    gametext(c,120,"CHRIS BENSON",0);
                    gametext(c,140,"LEAD TESTER",0);
                    gametext(c,150,"DARRELL JONES",0);
                    break;
                case 983:
                    gametext(c,70,"TESTERS",0);
                    gametext(c,90,"TIM ANDERSON",0);
                    gametext(c,100,"PRIMO PULANCO",0);
                    gametext(c,110,"MARK MCCARTY",0);
                    gametext(c,120,"BRIAN AXLINE",0);
                    break;
                case 984:
                    gametext(c,80,"PRODUCTION BABY",0);
                    gametext(c,100,"PAULINE MARIE MARKHAM",0);
                    break;
                case 985:
                    gametext(c,80,"ORIGINAL PRODUCTION BABY",0);
                    gametext(c,100,"ALYSON KAUFMAN",0);
                    break;
                case 986:
                    gametext(c,80,"3D BUILD ENGINE LICENSED FROM",0);
                    gametext(c,90,"3D REALMS ENTERTAINMENT",0);
                    gametext(c,110,"BUILD ENGINE AND RELATED TOOLS",0);
                    gametext(c,120,"CREATED BY KEN SILVERMAN",0);
                    break;
                case 987:
                    gametext(c,80,"SPECIAL THANKS",0);
                    gametext(c,100,"SCOTT MILLER",0);
                    gametext(c,110,"GEORGE BROUSSARD",0);
                    break;
                case 988:
                    gametext(c,80,"EXTRA SPECIAL THANKS",0);
                    gametext(c,100,"BRIAN FARGO",0);
                    break;
                case 989:
                    gametext(c,70,"REDNECK RAMPAGE RIDES AGAIN",0);
                    gametext(c,80,"(c) 1998 XATRIX ENTERTAINMENT, INC.",0);
                    gametext(c,100,"REDNECK RAMPAGE RIDES AGAIN",0);
                    gametext(c,110,"IS A TRADEMARK OF",0);
                    gametext(c,120,"INTERPLAY PRODUCTIONS",0);
                    break;
#else
                case 960:
                    gametext(c,80,"ORIGINAL CONCEPT, DESIGN AND DIRECTION",0);
                    gametext(c,100,"DREW MARKHAM",0);
                    break;
                case 961:
                    gametext(c,80,"PRODUCED BY",0);
                    gametext(c,100,"GREG GOODRICH",0);
                    break;
                case 962:
                    gametext(c,80,"GAME PROGRAMMING",0);
                    gametext(c,100,"RAFAEL PAIZ",0);
                    break;
                case 963:
                    gametext(c,80,"ART DIRECTORS",0);
                    gametext(c,100,"CLAIRE PRADERIE     MAXX KAUFMAN ",0);
                    break;
                case 964:
                    gametext(c,80,"LEAD LEVEL DESIGNER",0);
                    gametext(c,90,"ALEX MAYBERRY",0);
                    gametext(c,110,"LEVEL DESIGN",0);
                    gametext(c,120,"MAL BLACKWELL",0);
                    gametext(c,130,"SVERRE KVERNMO",0);
                    break;
                case 965:
                    gametext(c,80,"SENIOR ANIMATOR AND ARTIST",0);
                    gametext(c,100,"JASON HOOVER",0);
                    break;
                case 966:
                    gametext(c,80,"TECHNICAL DIRECTOR",0);
                    gametext(c,100,"BARRY DEMPSEY",0);
                    break;
                case 967:
                    gametext(c,60,"MOTION CAPTURE SPECIALIST AND",0);
                    gametext(c,70,"CHARACTER ANIMATION",0);
                    gametext(c,80,"AMIT DORON",0);
                    gametext(c,100,"A.I. PROGRAMMING",0);
                    gametext(c,110,"ARTHUR DONAVAN",0);
                    gametext(c,130,"ADDITIONAL ANIMATION",0);
                    gametext(c,140,"GEORGE KARL",0);
                    break;
                case 968:
                    gametext(c,50,"CHARACTER DESIGN",0);
                    gametext(c,60,"CORKY LEHMKUHL",0);
                    gametext(c,80,"MAP PAINTERS",0);
                    gametext(c,90,"VIKTOR ANTONOV",0);
                    gametext(c,100,"MATTHIAS BEEGUER",0);
                    gametext(c,110,"STEPHAN BURLE",0);
                    gametext(c,130,"SCULPTORS",0);
                    gametext(c,140,"GEORGE ENGEL",0);
                    gametext(c,150,"JAKE GARBER",0);
                    gametext(c,160,"JEFF HIMMEL",0);
                    break;
                case 969:
                    gametext(c,40,"CHARACTER VOICES",0);
                    gametext(c,60,"LEONARD",0);
                    gametext(c,70,"BURTON GILLIAM",0);
                    gametext(c,90,"BUBBA, BILLY RAY, SKINNY OL' COOT",0);
                    gametext(c,100,"AND THE TURD MINION",0);
                    gametext(c,110,"DREW MARKHAM",0);
                    gametext(c,130,"SHERIFF LESTER T. HOBBES",0);
                    gametext(c,140,"MOJO NIXON",0);
                    gametext(c,160,"ALIEN VIXEN",0);
                    gametext(c,170,"PEGGY JO JACOBS",0);
                    break;
                case 970:
                    gametext(c,50,"SOUND DESIGN",0);
                    gametext(c,60,"GARY BRADFIELD",0);
                    gametext(c,80,"MUSIC",0);
                    gametext(c,90,"MOJO NIXON",0);
                    gametext(c,100,"THE BEAT FARMERS",0);
                    gametext(c,110,"THE REVEREND HORTON HEAT",0);
                    gametext(c,120,"CEMENT POND",0);
                    gametext(c,140,"ADDITIONAL SOUND EFFECTS",0);
                    gametext(c,150,"JIM SPURGIN",0);
                    break;
                case 971:
                    gametext(c,80,"MOTION CAPTURE ACTOR",0);
                    gametext(c,90,"J.P. MANOUX",0);
                    gametext(c,110,"MOTION CAPTURE VIXEN",0);
                    gametext(c,120,"SHAWN WOLFE",0);
                    break;
                case 972:
                    gametext(c,50,"PRODUCTION ASSISTANCE",0);
                    gametext(c,60,"MINERVA MAYBERRY",0);
                    gametext(c,80,"NUTS AND BOLTS",0);
                    gametext(c,90,"STEVE GOLDBERG",0);
                    gametext(c,100,"MARCUS HUTCHINSON",0);
                    gametext(c,120,"BEAN COUNTING",0);
                    gametext(c,130,"MAX YOSHIKAWA",0);
                    gametext(c,150,"ADMINISTRATIVE ASSISTANCE",0);
                    gametext(c,160,"SERAFIN LEWIS",0);
                    break;
                case 973:
                    gametext(c,70,"LOCATION MANAGER, LOUISIANA",0);
                    gametext(c,80,"RICK SKINNER",0);
                    gametext(c,100,"LOCATION SCOUT, LOUISIANA",0);
                    gametext(c,110,"BRIAN BENOS",0);
                    gametext(c,130,"PHOTOGRAPHER",0);
                    gametext(c,140,"CARLOS SERRAO",0);
                    break;
                case 974:
                    gametext(c,50,"ADDITIONAL 3D MODELING BY",0);
                    gametext(c,60,"3 NAME 3D",0);
                    gametext(c,70,"VIEWPOINT DATALABS INTERNATIONAL",0);
                    gametext(c,90,"AUDIO RECORDED AT",0);
                    gametext(c,100,"PACIFIC OCEAN POST, SANTA MONICA, C.A.",0);
                    gametext(c,120,"CEMENT POND TRACKS RECORDED AT",0);
                    gametext(c,130,"DREAMSTATE RECORDING, BURBANK, C.A.",0);
                    gametext(c,150,"RECORDING ENGINEER",0);
                    gametext(c,160,"DAVE AHLERT",0);
                    break;
                case 975:
                    gametext(c,80,"3D BUILD ENGINE LICENSED FROM",0);
                    gametext(c,90,"3D REALMS ENTERTAINMENT",0);
                    gametext(c,110,"BUILD ENGINE AND RELATED TOOLS",0);
                    gametext(c,120,"CREATED BY KEN SILVERMAN",0);
                    break;
                case 976:
                    gametext(c,60,"FOR INTERPLAY",0);
                    gametext(c,80,"LEAD TESTER",0);
                    gametext(c,90,"DARRELL JONES",0);
                    gametext(c,110,"TESTERS",0);
                    gametext(c,120,"TIM ANDERSON",0);
                    gametext(c,130,"ERICK LUJAN",0);
                    gametext(c,140,"TIEN TRAN",0);
                    break;
                case 977:
                    gametext(c,60,"IS TECHS",0);
                    gametext(c,70,"BILL DELK",0);
                    gametext(c,80,"AARON MEYERS",0);
                    gametext(c,100,"COMPATIBILITY TECHS",0);
                    gametext(c,110,"MARC DURAN",0);
                    gametext(c,120,"DAN FORSYTH",0);
                    gametext(c,130,"DEREK GIBBS",0);
                    gametext(c,140,"AARON OLAIZ",0);
                    gametext(c,150,"JACK PARKER",0);
                    break;
                case 978:
                    gametext(c,70,"DIRECTOR OF COMPATIBILITY",0);
                    gametext(c,80,"PHUONG NGUYEN",0);
                    gametext(c,100,"ASSISTANT QA DIRECTOR",0);
                    gametext(c,110,"COLIN TOTMAN",0);
                    gametext(c,130,"QA DIRECTOR",0);
                    gametext(c,140,"CHAD ALLISON",0);
                    break;
                case 979:
                    gametext(c,50,"INTERPLAY PRODUCER",0);
                    gametext(c,60,"BILL DUGAN",0);
                    gametext(c,80,"INTERPLAY LINE PRODUCER",0);
                    gametext(c,90,"CHRIS BENSON",0);
                    gametext(c,110,"PRODUCT MANAGER",0);
                    gametext(c,120,"JIM VEEVAERT",0);
                    gametext(c,140,"PUBLIC RELATIONS",0);
                    gametext(c,150,"ERIKA PRICE",0);
                    break;
                case 980:
                    gametext(c,60,"SPECIAL THANKS",0);
                    gametext(c,80,"JIM GAUER",0);
                    gametext(c,90,"PAUL VAIS",0);
                    gametext(c,100,"SCOTT MILLER",0);
                    gametext(c,110,"TODD REPLOGLE",0);
                    gametext(c,120,"CHUCK BUECHE",0);
                    gametext(c,130,"CARTER LIPSCOMB",0);
                    gametext(c,140,"JOHN CONLEY",0);
                    gametext(c,150,"DON MAGGI",0);
                    break;
                case 981:
                    gametext(c,80,"EXTRA SPECIAL THANKS",0);
                    gametext(c,100,"BRIAN FARGO",0);
                    break;
                case 982:
                    gametext(c,80,"REDNECK RAMPAGE",0);
                    gametext(c,90,"(c) 1997 XATRIX ENTERTAINMENT, INC.",0);
                    gametext(c,110,"REDNECK RAMPAGE IS A TRADEMARK OF",0);
                    gametext(c,120,"INTERPLAY PRODUCTIONS",0);
                    break;
#endif

            }
            break;

        case 0:
            c = (320>>1);
#ifdef RRRA
            rotatesprite((c-5)<<16,57<<16,16592L,0,THREEDEE,0,0,10,0,0,xdim-1,ydim-1);
            x = probe(c,63,16,6,0);
#else
            rotatesprite((c+5)<<16,24<<16,23592L,0,INGAMEDUKETHREEDEE,0,0,10,0,0,xdim-1,ydim-1);
            x = probe(c,63,16,7,0);
#endif
            if(x >= 0)
            {
                if( ud.multimode > 1 && x == 0 && ud.recstat != 2)
                {
                    last_zero = 0;
                    cmenu( 600 );
                }
                else
                {
                    last_zero = x;
                    switch(x)
                    {
                        case 0:
                            if(movesperpacket == 4 && myconnectindex != connecthead)
                                break;

                            cmenu(100);
                            break;
#ifdef RRRA
                        case 1: cmenu(200);break;
                        case 2:
                            if(movesperpacket == 4 && myconnectindex != connecthead)
                                break;
                            cmenu(300);
                            break;
                        case 3: KB_FlushKeyboardQueue();cmenu(400);break;
                        case 4: cmenu(960);break;
                        case 5: cmenu(500);break;
#else
                        case 1:
                            if(movesperpacket == 4)
                                break;

                            cmenu(20001);
                            break;
                        case 2: cmenu(200);break;
                        case 3:
                            if(movesperpacket == 4 && connecthead != myconnectindex)
                                break;
                            cmenu(300);
                            break;
                        case 4: KB_FlushKeyboardQueue();cmenu(400);break;
                        case 5: cmenu(960);break;
                        case 6: cmenu(500);break;
#endif
                    }
                }
            }

            if(KB_KeyPressed(sc_Q)) cmenu(500);

            if(x == -1)
            {
                ps[myconnectindex].gm &= ~MODE_MENU;
                if(ud.multimode < 2 && ud.recstat != 2)
                {
                    ready2send = 1;
                    totalclock = ototalclock;
                }
            }

            if(movesperpacket == 4)
            {
                if( myconnectindex == connecthead )
                    menutext(c,63,SHX(-2),PHX(-2),"NEW GAME");
                else
                    menutext(c,63,SHX(-2),1,"NEW GAME");
            }
            else
                menutext(c,63,SHX(-2),PHX(-2),"NEW GAME");

#ifdef RRRA
            menutext(c,63+16,SHX(-4),PHX(-4),"OPTIONS");
            menutext(c,63+16+16,SHX(-5),PHX(-5),"LOAD GAME");
            menutext(c,63+16+16+16,SHX(-6),PHX(-6),"HELP");
            menutext(c,63+16+16+16+16,SHX(-7),PHX(-7),"CREDITS");
            menutext(c,63+16+16+16+16+16,SHX(-8),PHX(-8),"QUIT");
#else
            menutext(c,63+16,SHX(-3),PHX(-3),"PLAY ON ENGAGE");
            menutext(c,63+16+16,SHX(-4),PHX(-4),"OPTIONS");
            menutext(c,63+16+16+16,SHX(-5),PHX(-5),"LOAD GAME");
            menutext(c,63+16+16+16+16,SHX(-6),PHX(-6),"HELP");
            menutext(c,63+16+16+16+16+16,SHX(-7),PHX(-7),"CREDITS");
            menutext(c,63+16+16+16+16+16+16,SHX(-8),PHX(-8),"QUIT");
#endif

            break;

        case 50:
            c = (320>>1);
#ifdef RRRA
            rotatesprite((c-5)<<16,57<<16,16592L,0,THREEDEE,0,0,10,0,0,xdim-1,ydim-1);
#else
            rotatesprite(c<<16,24<<16,23592L,0,INGAMEDUKETHREEDEE,0,0,10,0,0,xdim-1,ydim-1);
#endif
            x = probe(c,61,16,7,0);
            switch(x)
            {
                case 0:
                    if(movesperpacket == 4 && myconnectindex != connecthead)
                        break;
                    if(ud.multimode < 2 || ud.recstat == 2)
                        cmenu(1500);
                    else
                    {
                        cmenu(600);
                        last_fifty = 0;
                    }
                    break;
                case 1:
                    if(ud.recstat != 2)
                    {
                        last_fifty = 1;
                        cmenu(350);
                        setview(0,0,xdim-1,ydim-1);
                    }
                    break;
                case 2:
                    last_fifty = 2;
                    cmenu(300);
                    break;
                case 3:
                    last_fifty = 3;
                    cmenu(200);
                    break;
                case 4:
                    last_fifty = 4;
                    KB_FlushKeyboardQueue();
                    cmenu(400);
                    break;
                case 5:
                    if(ud.multimode < 2)
                    {
                        last_fifty = 5;
                        cmenu(501);
                    }
                    break;
                case 6:
                    last_fifty = 6;
                    cmenu(500);
                    break;
                case -1:
                    ps[myconnectindex].gm &= ~MODE_MENU;
                    if(ud.multimode < 2 && ud.recstat != 2)
                    {
                        ready2send = 1;
                        totalclock = ototalclock;
                    }
                    break;
            }

            if( KB_KeyPressed(sc_Q) )
                cmenu(500);

            if(movesperpacket == 4 && connecthead != myconnectindex)
            {
                menutext(c,61                  ,SHX(-2),1,"NEW GAME");
                menutext(c,61+16               ,SHX(-3),1,"SAVE GAME");
                menutext(c,61+16+16            ,SHX(-4),1,"LOAD GAME");
            }
            else
            {
                menutext(c,61                  ,SHX(-2),PHX(-2),"NEW GAME");
                menutext(c,61+16               ,SHX(-3),PHX(-3),"SAVE GAME");
                menutext(c,61+16+16            ,SHX(-4),PHX(-4),"LOAD GAME");
            }

            menutext(c,61+16+16+16         ,SHX(-5),PHX(-5),"OPTIONS");
            menutext(c,61+16+16+16+16      ,SHX(-6),PHX(-6)," HELP");
            if(ud.multimode > 1)
                menutext(c,61+16+16+16+16+16   ,SHX(-7),1,"QUIT TO TITLE");
            else menutext(c,61+16+16+16+16+16   ,SHX(-7),PHX(-7),"QUIT TO TITLE");
            menutext(c,61+16+16+16+16+16+16,SHX(-8),PHX(-8),"QUIT GAME");
            break;

        case 100:
            rotatesprite(160<<16,19<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(160,24,0,0,"SELECT YER EPISODE");
#ifdef RRRA
            x = probe(160,80,20,2,0);
#else
            if(boardfilename[0])
                x = probe(160,80,20,3,0);
            else x = probe(160,80,20,2,0);
#endif
            if(x >= 0)
            {
                if(x == 4 && boardfilename[0])
                {
                    ud.m_volume_number = 0;
                    ud.m_level_number = 7;
                }
                else
                {
                    ud.m_volume_number = x;
                    ud.m_level_number = 0;
                }
                cmenu(110);
            }
            else if(x == -1)
            {
                if(ps[myconnectindex].gm&MODE_GAME) cmenu(50);
                else cmenu(0);
            }

            menutext(160,80,SHX(-2),PHX(-2),volume_names[0]);

            c = 80;
            menutext(160,80+20,SHX(-3),PHX(-3),volume_names[1]);
#ifndef RRRA
            if(boardfilename[0])
            {
                menutext(160,80+20+20,SHX(-4),PHX(-4),boardfilename);
            }
#endif
            break;

        case 110:
            c = (320>>1);
            rotatesprite(c<<16,19<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(c,24,0,0,"SELECT SKILL");
            x = probe(c,70,19,5,0);
            if(x >= 0)
            {
                switch(x)
                {
                    case 0: globalskillsound = 427;break;
                    case 1: globalskillsound = 428;break;
                    case 2: globalskillsound = 196;break;
                    case 3: globalskillsound = 195;break;
                    case 4: globalskillsound = 197;break;
                }

                sound(globalskillsound);

#ifdef RRRA
                ud.m_player_skill = x;
#else
                ud.m_player_skill = x+1;
#endif
                if(x == 3) ud.m_respawn_monsters = 1;
                else ud.m_respawn_monsters = 0;

                ud.m_monsters_off = ud.monsters_off = 0;

                ud.m_respawn_items = 0;
                ud.m_respawn_inventory = 0;

                ud.multimode = 1;

                newgame(ud.m_volume_number,ud.m_level_number,ud.m_player_skill);
                enterlevel(MODE_GAME);
            }
            else if(x == -1)
            {
                cmenu(100);
                KB_FlushKeyboardQueue();
            }

#ifdef RRRA
            if (x < 0)
            {
#endif
            menutext(c,70,SHX(-2),PHX(-2),skill_names[0]);
            menutext(c,70+19,SHX(-3),PHX(-3),skill_names[1]);
            menutext(c,70+19+19,SHX(-4),PHX(-4),skill_names[2]);
            menutext(c,70+19+19+19,SHX(-5),PHX(-5),skill_names[3]);
            menutext(c,70+19+19+19+19,SHX(-6),PHX(-6),skill_names[4]);
#ifdef RRRA
            }
#endif
            break;

        case 200:

            rotatesprite(320<<15,10<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(320>>1,15,0,0,"OPTIONS");

            c = (320>>1)-120;

            onbar = (probey == 2 || probey == 3 || probey == 4);
            x = probe(c+6,49,15,9,0);

            if(x == -1)
                { if(ps[myconnectindex].gm&MODE_GAME) cmenu(50);else cmenu(0); }

            if(onbar == 0) switch(x)
            {
                case 0:
                    ud.shadows = 1-ud.shadows;
                    break;
                case 1:
                    ud.screen_tilting = 1-ud.screen_tilting;
                    break;
                case 5:
                    if ( ControllerType == controltype_keyboardandmouse )
                        ud.mouseflip = 1-ud.mouseflip;
                    break;
                case 6:
                    cmenu(700);
                    break;
                case 7:
                    cmenu(10000);
                    break;
                case 8:
                    if( (ps[myconnectindex].gm&MODE_GAME) )
                    {
                        closedemowrite();
                        break;
                    }
                    ud.m_recstat = !ud.m_recstat;
                    break;
            }

            if(ud.shadows) menutext(c+160+40,31+15,0,0,"ON");
            else menutext(c+160+40,31+15,0,0,"OFF");

            switch(ud.screen_tilting)
            {
                case 0: menutext(c+160+40,31+15+15,0,0,"OFF");break;
                case 1: menutext(c+160+40,31+15+15,0,0,"ON");break;
                case 2: menutext(c+160+40,31+15+15,0,0,"FULL");break;
            }

            menutext(c,31+15,SHX(-3),PHX(-3),"SHADDERS");
            menutext(c,31+15+15,SHX(-4),PHX(-4),"SCREEN TILTIN'");
            menutext(c,31+15+15+15,SHX(-5),PHX(-5),"SCREEN SIZE");

                bar(c+167+40,31+15+15+15,(short *)&ud.screen_size,-4,x==2,SHX(-5),PHX(-5));

            menutext(c,31+15+15+15+15,SHX(-6),PHX(-6),"BRIGHTNESS");
                bar(c+167+40,31+15+15+15+15,(short *)&ud.brightness,8,x==3,SHX(-6),PHX(-6));
                if(x==3) setbrightness(ud.brightness>>2,&ps[myconnectindex].palette[0]);

            if ( ControllerType == controltype_keyboardandmouse )
            {
                short sense;
                sense = CONTROL_GetMouseSensitivity()>>10;

                menutext(c,31+15+15+15+15+15,SHX(-7),PHX(-7),"RAT SENSITIVITY");
                bar(c+167+40,31+15+15+15+15+15,&sense,4,x==4,SHX(-7),PHX(-7));
                CONTROL_SetMouseSensitivity( sense<<10 );
                menutext(c,31+15+15+15+15+15+15,SHX(-7),PHX(-7),"RAT AIMIN' FLIP");

                if(ud.mouseflip) menutext(c+160+40,31+15+15+15+15+15+15,SHX(-7),PHX(-7),"ON");
                else menutext(c+160+40,31+15+15+15+15+15+15,SHX(-7),PHX(-7),"OFF");

            }
            else
            {
                short sense;
                sense = CONTROL_GetMouseSensitivity()>>10;

                menutext(c,31+15+15+15+15+15,SHX(-7),1,"RAT SENSITIVITY");
                bar(c+167+40,31+15+15+15+15+15,&sense,4,x==99,SHX(-7),1);
                menutext(c,31+15+15+15+15+15+15,SHX(-7),1,"RAT AIMIN' FLIP");

                if(ud.mouseflip) menutext(c+160+40,31+15+15+15+15+15+15,SHX(-7),1,"ON");
                else menutext(c+160+40,31+15+15+15+15+15+15,SHX(-7),1,"OFF");
            }

            menutext(c,31+15+15+15+15+15+15+15,SHX(-8),PHX(-8),"SOUNDS");
            menutext(c,31+15+15+15+15+15+15+15+15,SHX(-9),PHX(-9),"PARENTAL LOCK");
            if( (ps[myconnectindex].gm&MODE_GAME) && ud.m_recstat != 1 )
            {
                menutext(c,31+15+15+15+15+15+15+15+15+15,SHX(-10),1,"RECORD");
                menutext(c+160+40,31+15+15+15+15+15+15+15+15+15,SHX(-10),1,"OFF");
            }
            else
            {
                menutext(c,31+15+15+15+15+15+15+15+15+15,SHX(-10),PHX(-10),"RECORD");

                if(ud.m_recstat == 1)
                    menutext(c+160+40,31+15+15+15+15+15+15+15+15+15,SHX(-10),PHX(-10),"ON");
                else menutext(c+160+40,31+15+15+15+15+15+15+15+15+15,SHX(-10),PHX(-10),"OFF");
            }

            break;

        case 800:
#ifdef RRRA
            if (numplayers > 1)
            {
                cmenu(700);
                break;
            }
#endif
            if (!cddrives)
            {
                cmenu(700);
                break;
            }
            c = (320>>1)-120;
            probey = whichtrack-cdlotrack-1;
            x = probe(c,50,16,8,3);
            rotatesprite(320<<15,19<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(320>>1,24,0,0,"8 TRACK PLAYER");
            rotatesprite(320<<15,200<<15,32768L,0,CDPLAYER,16,0,10,0,0,xdim-1,ydim-1);
#ifdef RRRA
            if (word_1D7EF8 == 0)
            {
                word_1D7EF8 = 1;
                x = 9;
            }
#endif
            switch (x)
            {
                case -1:
                    cmenu(700);
                    break;
                case -2:
                    x = 0;
                    tx = 22;
                    ty = 15;
                    break;
                case -3:
                    x = 1;
                    tx = 64;
                    ty = 15;
                    break;
                case -4:
                    x = 2;
                    tx = 104;
                    ty = 15;
                    break;
                case -5:
                    x = 3;
                    tx = 142;
                    ty = 15;
                    break;
                case -6:
                    x = 4;
                    tx = 22;
                    ty = 25;
                    break;
                case -7:
                    x = 5;
                    tx = 64;
                    ty = 25;
                    break;
                case -8:
                    x = 6;
                    tx = 104;
                    ty = 25;
                    break;
                case -9:
                    x = 7;
                    tx = 142;
                    ty = 25;
                    break;
                default:
                    x = 9;
                    tx = 22;
                    ty = 50;
                    if (cdon)
                    {
                        cdon = 0;
                        rbstop();
                    }
                    else
                    {
                        cdon = 1;
                        rbPlayTrack(whichtrack);
                    }
                    break;
            }
            if (x != -1)
            {
                if (x < 8)
                {
                    rotatesprite(320<<15,200<<15,32768L,0,CDPLAYER,16,0,10,0,0,xdim-1,ydim-1);
                    rotatesprite((tx+166)<<15,(ty+65)<<16,32768L,0,CDPLAYER+1,16,0,10,0,0,xdim-1,ydim-1);
                }
                if (cdon)
                {
                    rotatesprite(100<<16,113<<16,32768L,0,CDPLAYER+3,16,0,10,0,0,xdim-1,ydim-1);
                }
                else
                {
                    rotatesprite(100<<16,113<<16,32768L,0,CDPLAYER+2,16,0,10,0,0,xdim-1,ydim-1);
                }
            }
            if (x > -1)
            {
                if (whichtrack != oldtrack)
                {
                    oldtrack = whichtrack;
                    whichtrack = cdlotrack+x+1;
                    rbstop();
                    rbPlayTrack(whichtrack);
                }
                else
                    whichtrack = cdlotrack + x + 1;
            }
            break;

        case 700:
            c = (320>>1)-120;
            rotatesprite(320<<15,19<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(320>>1,24,0,0,"SOUNDS");
            onbar = ( probey == 2 );

            x = probe(c,50,16,6,0);
            switch(x)
            {
                case -1:
                    if(ps[myconnectindex].gm&MODE_GAME)
                    {
                        ps[myconnectindex].gm &= ~MODE_MENU;
                        if(ud.multimode < 2  && ud.recstat != 2)
                        {
                            ready2send = 1;
                            totalclock = ototalclock;
                        }
                    }

                    else cmenu(200);
                    break;
                case 0:
                    if (FXDevice != NumSoundCards)
                    {
                        SoundToggle = 1-SoundToggle;
                        if( SoundToggle == 0 )
                        {
                            FX_StopAllSounds();
                            clearsoundlocks();
                        }
                        onbar = 0;
                    }
                    break;
                case 1:
                    cmenu(800);
                    onbar = 0;

                    break;
                case 3:
                    if(SoundToggle && (FXDevice != NumSoundCards)) VoiceToggle = 1-VoiceToggle;
                    onbar = 0;
                    break;
                case 4:
                    if(SoundToggle && (FXDevice != NumSoundCards)) AmbienceToggle = 1-AmbienceToggle;
                    onbar = 0;
                    break;
                case 5:
                    if(SoundToggle && (FXDevice != NumSoundCards))
                    {
                        ReverseStereo = 1-ReverseStereo;
                        FX_SetReverseStereo(ReverseStereo);
                    }
                    onbar = 0;
                    break;
                default:
                    onbar = 1;
                    break;
            }

            if(SoundToggle && FXDevice != NumSoundCards) menutext(c+160+40,50,0,0,"ON");
            else menutext(c+160+40,50,0,0,"OFF");

            menutext(c,50,SHX(-2),0,"SOUND");
            menutext(c,50+16+16,SHX(-4),(FXDevice==NumSoundCards)||SoundToggle==0,"SOUND VOLUME");
            {
                l = FXVolume;
                FXVolume >>= 2;
                bar(c+167+40,50+16+16,(short *)&FXVolume,4,(FXDevice!=NumSoundCards)&&x==2,SHX(-4),SoundToggle==0||(FXDevice==NumSoundCards));
                if(l != FXVolume)
                    FXVolume <<= 2;
                if(l != FXVolume)
                    FX_SetVolume( (short) FXVolume );
            }
            if (cddrives == 0)
                menutext(c,50+16,SHX(-3),1,"8 TRACK PLAYER");
            else
                menutext(c,50+16,SHX(-3),0,"8 TRACK PLAYER");
            menutext(c,50+16+16+16,SHX(-6),SoundToggle==0,"LEONARD TALK");
            menutext(c,50+16+16+16+16,SHX(-7),SoundToggle==0,"AMBIENCE");

            menutext(c,50+16+16+16+16+16,SHX(-8),SoundToggle==0,"FLIP STEREO");

            if(VoiceToggle) menutext(c+160+40,50+16+16+16,0,0,"ON");
            else menutext(c+160+40,50+16+16+16,0,0,"OFF");

            if(AmbienceToggle) menutext(c+160+40,50+16+16+16+16,0,0,"ON");
            else menutext(c+160+40,50+16+16+16+16,0,0,"OFF");

            if(ReverseStereo) menutext(c+160+40,50+16+16+16+16+16,0,0,"ON");
            else menutext(c+160+40,50+16+16+16+16+16,0,0,"OFF");


            break;

        case 350:
            cmenu(351);
            screencapt = 1;
            displayrooms(myconnectindex,65536);
            savetemp("redneck.tmp",waloff[MAXTILES-1],160*100);
            screencapt = 0;
            break;

        case 360:
        case 361:
        case 362:
        case 363:
        case 364:
        case 365:
        case 366:
        case 367:
        case 368:
        case 369:
        case 351:
        case 300:

            c = 320>>1;
            rotatesprite(c<<16,200<<15,65536L,0,MENUSCREEN,16,0,10+64,0,0,xdim-1,ydim-1);
            rotatesprite(c<<16,19<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
#ifdef RRRA
            if (ud.m_player_skill == 4)
#else
            if (ud.m_player_skill == 5)
#endif
            {
                ps[myconnectindex].gm &= ~MODE_MENU;
                FTA(53,&ps[0]);
                break;
            }

            if(current_menu == 300) menutext(c,24,0,0,"LOAD GAME");
            else menutext(c,24,0,0,"SAVE GAME");

            if(current_menu >= 360 && current_menu <= 369 )
            {
                sprintf(tempbuf,"PLAYERS: %-2d                      ",ud.multimode);
                gametext(160,158,tempbuf,0);
                sprintf(tempbuf,"EPISODE: %-2d / LEVEL: %-2d / SKILL: %-2d",1+ud.volume_number,1+ud.level_number,ud.player_skill);
                gametext(160,170,tempbuf,0);

                x = strget((320>>1),184,&ud.savegame[current_menu-360][0],19, 999 );

                if(x == -1)
                {
                    readsavenames();
                    ps[myconnectindex].gm = MODE_GAME;
                    if(ud.multimode < 2  && ud.recstat != 2)
                    {
                        ready2send = 1;
                        totalclock = ototalclock;
                    }
                    goto DISPLAYNAMES;
                }

                if( x == 1 )
                {
                    if( ud.savegame[current_menu-360][0] == 0 )
                    {
                        KB_FlushKeyboardQueue();
                        cmenu(351);
                    }
                    else
                    {
                        if(ud.multimode > 1)
                            saveplayer(-1-(current_menu-360));
                        else saveplayer(current_menu-360);
                        lastsavedpos = current_menu-360;
                        ps[myconnectindex].gm = MODE_GAME;

                        if(ud.multimode < 2  && ud.recstat != 2)
                        {
                            ready2send = 1;
                            totalclock = ototalclock;
                        }
                        KB_ClearKeyDown(sc_Escape);
                        sound(341);
                    }
                }

                rotatesprite(101<<16,97<<16,65536,512,MAXTILES-1,-32,0,2+4+8+64,0,0,xdim-1,ydim-1);
                dispnames();
                rotatesprite((c+67+strlen(&ud.savegame[current_menu-360][0])*4)<<16,(50+12*probey)<<16,3276L,0,SPINNINGNUKEICON+(((totalclock)>>3)%16),0,0,10,0,0,xdim-1,ydim-1);
                //break;
            }

           last_threehundred = probey;

            x = probe(c+68,54,12,10,0);

          if(current_menu == 300)
          {
              if( ud.savegame[probey][0] )
              {
                  if( lastprobey != probey )
                  {
                     loadpheader(probey,&volnum,&levnum,&plrskl,&numplr);
                     lastprobey = probey;
                  }

                  rotatesprite(101<<16,97<<16,65536L,512,MAXTILES-3,-32,0,4+10+64,0,0,xdim-1,ydim-1);
                  sprintf(tempbuf,"PLAYERS: %-2d                      ",numplr);
                  gametext(160,158,tempbuf,0);
                  sprintf(tempbuf,"EPISODE: %-2d / LEVEL: %-2d / SKILL: %-2d",1+volnum,1+levnum,plrskl);
                  gametext(160,170,tempbuf,0);
              }
              else menutext(69,70,0,0,"EMPTY");
          }
          else
          {
              if( ud.savegame[probey][0] )
              {
                  if(lastprobey != probey)
                      loadpheader(probey,&volnum,&levnum,&plrskl,&numplr);
                  lastprobey = probey;
                  rotatesprite(101<<16,97<<16,65536L,512,MAXTILES-1,-32,0,4+10+64,0,0,xdim-1,ydim-1);
              }
              else menutext(69,70,0,0,"EMPTY");
              sprintf(tempbuf,"PLAYERS: %-2d                      ",ud.multimode);
              gametext(160,158,tempbuf,0);
              sprintf(tempbuf,"EPISODE: %-2d / LEVEL: %-2d / SKILL: %-2d",1+ud.volume_number,1+ud.level_number,ud.player_skill);
              gametext(160,170,tempbuf,0);
          }

            switch( x )
            {
                case -1:
                    if(current_menu == 300)
                    {
                        if( (ps[myconnectindex].gm&MODE_GAME) != MODE_GAME)
                        {
                            cmenu(0);
                            break;
                        }
                        else
                            ps[myconnectindex].gm &= ~MODE_MENU;
                    }
                    else
                        ps[myconnectindex].gm = MODE_GAME;

                    if(ud.multimode < 2 && ud.recstat != 2)
                    {
                        ready2send = 1;
                        totalclock = ototalclock;
                    }

                    break;
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                    if( current_menu == 300)
                    {
                        if( ud.savegame[x][0] )
                            current_menu = (1000+x);
                    }
                    else
                    {
                        if( ud.savegame[x][0] != 0)
                            current_menu = 2000+x;
                        else
                        {
                            KB_FlushKeyboardQueue();
                            current_menu = (360+x);
                            ud.savegame[x][0] = 0;
                            inputloc = 0;
                        }
                    }
                    break;
            }

            DISPLAYNAMES:
            dispnames();
            break;

        case 370:
            c = 320>>1;
            rotatesprite(c<<16,200<<15,65536L,0,MENUSCREEN,16,0,10+64,0,0,xdim-1,ydim-1);
            rotatesprite(c<<16,19<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(c,24,0,0,"DELETE GAME!");
            if (KB_KeyPressed(sc_Y) || LMB)
            {
                char *fn = "game0.sav";
                fn[4] = deletespot + '0';
                KB_FlushKeyboardQueue();
                current_menu = last_menu;
                probey = lastprobey;
                ud.savegame[deletespot][0] = 0;
                unlink(fn);
            }
            if (KB_KeyPressed(sc_kpad_Enter) || KB_KeyPressed(sc_Enter) || KB_KeyPressed(sc_N) || KB_KeyPressed(sc_Escape) || RMB)
            {
                KB_FlushKeyboardQueue();
                current_menu = last_menu;
                probey = lastprobey;
                sound(341);
            }
            dispnames();
            rotatesprite(101<<16,97<<16,65536L,512,MAXTILES-3,-32,0,4+10+64,0,0,xdim-1,ydim-1);
            probe(186,133,0,0,0);
            gametext(160,60,"Delete game:",0);
            sprintf(tempbuf,"\"%s\"",ud.savegame[deletespot]);
            gametext(160,70,tempbuf,0);
            gametext(160,80,"(Y/N)",0);
            break;

        case 400:
        case 401:
#ifdef RRRA
        case 402:
#endif

            c = 320>>1;

            if( KB_KeyPressed( sc_LeftArrow ) ||
                KB_KeyPressed( sc_kpad_4 ) ||
                KB_KeyPressed( sc_UpArrow ) ||
                KB_KeyPressed( sc_PgUp ) ||
                KB_KeyPressed( sc_kpad_8 ) )
            {
                KB_ClearKeyDown(sc_LeftArrow);
                KB_ClearKeyDown(sc_kpad_4);
                KB_ClearKeyDown(sc_UpArrow);
                KB_ClearKeyDown(sc_PgUp);
                KB_ClearKeyDown(sc_kpad_8);

                sound(335);
                current_menu--;
#ifdef RRRA
                if(current_menu < 400) current_menu = 402;
#else
                if(current_menu < 400) current_menu = 401;
#endif
            }
            else if(
                KB_KeyPressed( sc_PgDn ) ||
                KB_KeyPressed( sc_Enter ) ||
                KB_KeyPressed( sc_kpad_Enter ) ||
                KB_KeyPressed( sc_RightArrow ) ||
                KB_KeyPressed( sc_DownArrow ) ||
                KB_KeyPressed( sc_kpad_2 ) ||
                KB_KeyPressed( sc_kpad_9 ) ||
                KB_KeyPressed( sc_kpad_6 ) )
            {
                KB_ClearKeyDown(sc_PgDn);
                KB_ClearKeyDown(sc_Enter);
                KB_ClearKeyDown(sc_RightArrow);
                KB_ClearKeyDown(sc_kpad_Enter);
                KB_ClearKeyDown(sc_kpad_6);
                KB_ClearKeyDown(sc_kpad_9);
                KB_ClearKeyDown(sc_kpad_2);
                KB_ClearKeyDown(sc_DownArrow);
                sound(335);
                current_menu++;
#ifdef RRRA
                if(current_menu > 402) current_menu = 400;
#else
                if(current_menu > 401) current_menu = 400;
#endif
            }

            if( KB_KeyPressed(sc_Escape) )
            {
                if(ps[myconnectindex].gm&MODE_GAME)
                    cmenu(50);
                else cmenu(0);
                return;
            }

            flushperms();
            switch(current_menu)
            {
                case 400:
                    rotatesprite(0,0,65536L,0,TEXTSTORY,0,0,10+16+64, 0,0,xdim-1,ydim-1);
                    break;
                case 401:
                    rotatesprite(0,0,65536L,0,F1HELP,0,0,10+16+64, 0,0,xdim-1,ydim-1);
                    break;
#ifdef RRRA
                case 402:
                    rotatesprite(0,0,65536L,0,RRTILE1636,0,0,10+16+64, 0,0,xdim-1,ydim-1);
                    break;
#endif
            }

            break;

        case 500:
            c = 320>>1;

            gametext(c,90,"Quit? You ain't done yet",0);
            gametext(c,100,"(Y/N)",0);

            if( KB_KeyPressed(sc_Enter) || KB_KeyPressed(sc_kpad_Enter) || KB_KeyPressed(sc_Y) || LMB )
            {
                KB_FlushKeyboardQueue();

                if( gamequit == 0 && ( numplayers > 1 ) )
                {
                    if(ps[myconnectindex].gm&MODE_GAME)
                    {
                        gamequit = 1;
                        quittimer = totalclock+120;
                    }
                    else
                    {
                        sendlogoff();
                        gameexit(" ");
                    }
                }
                else if( numplayers < 2 )
                    gameexit(" ");

                if( ( totalclock > quittimer ) && ( gamequit == 1) )
                    gameexit("Timed out.");
            }

            x = probe(186,124,0,0,0);
            if(x == -1 || KB_KeyPressed(sc_N) || RMB)
            {
                KB_ClearKeyDown(sc_N);
                quittimer = 0;
                if( ps[myconnectindex].gm&MODE_DEMO )
                    ps[myconnectindex].gm = MODE_DEMO;
                else
                {
                    ps[myconnectindex].gm &= ~MODE_MENU;
                    if(ud.multimode < 2  && ud.recstat != 2)
                    {
                        ready2send = 1;
                        totalclock = ototalclock;
                    }
                }
            }

            break;
        case 501:
            c = 320>>1;
            gametext(c,90,"Quit to Title?",0);
            gametext(c,100,"(Y/N)",0);

            if( KB_KeyPressed(sc_Enter) || KB_KeyPressed(sc_kpad_Enter) || KB_KeyPressed(sc_Y) || LMB )
            {
                KB_FlushKeyboardQueue();
                ps[myconnectindex].gm = MODE_DEMO;
                if(ud.recstat == 1)
                    closedemowrite();
                cmenu(0);
            }

            x = probe(186,124,0,0,0);

            if(x == -1 || KB_KeyPressed(sc_N) || RMB)
            {
                ps[myconnectindex].gm &= ~MODE_MENU;
                if(ud.multimode < 2  && ud.recstat != 2)
                {
                    ready2send = 1;
                    totalclock = ototalclock;
                }
            }

            break;

        case 601:
            displayfragbar();
            rotatesprite(160<<16,29<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(320>>1,34,0,0,&ud.user_name[myconnectindex][0]);

            sprintf(tempbuf,"Waiting for master");
            gametext(160,50,tempbuf,0);
            gametext(160,59,"to select level",0);

            if( KB_KeyPressed(sc_Escape) )
            {
                KB_ClearKeyDown(sc_Escape);
                sound(341);
                cmenu(0);
            }
            break;

        case 602:
            if(menunamecnt == 0)
            {
        //        getfilenames("SUBD");
                getfilenames("*.MAP");
                sortfilenames();
                if (menunamecnt == 0)
                    cmenu(600);
            }
        case 603:
            c = (320>>1) - 120;
            displayfragbar();
            rotatesprite(320>>1<<16,19<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(320>>1,24,0,0,"USER MAPS");
            for(x=0;x<menunamecnt;x++)
            {
                if(x == fileselect)
                    minitext(15 + (x/15)*54,32 + (x%15)*8,menuname[x],0,26);
                else minitext(15 + (x/15)*54,32 + (x%15)*8,menuname[x],16,26);
            }

            fileselect = probey;
            if( KB_KeyPressed( sc_LeftArrow ) || KB_KeyPressed( sc_kpad_4 ) || ((buttonstat&1) && minfo.dyaw < -256 ) )
            {
                KB_ClearKeyDown( sc_LeftArrow );
                KB_ClearKeyDown( sc_kpad_4 );
                probey -= 15;
                if(probey < 0) probey += 15;
                else sound(335);
            }
            if( KB_KeyPressed( sc_RightArrow ) || KB_KeyPressed( sc_kpad_6 ) || ((buttonstat&1) && minfo.dyaw > 256 ) )
            {
                KB_ClearKeyDown( sc_RightArrow );
                KB_ClearKeyDown( sc_kpad_6 );
                probey += 15;
                if(probey >= menunamecnt)
                    probey -= 15;
                else sound(335);
            }

            onbar = 0;
            x = probe(0,0,0,menunamecnt,0);

            if(x == -1) cmenu(600);
            else if(x >= 0)
            {
                tempbuf[0] = 8;
                tempbuf[1] = ud.m_level_number = 6;
                tempbuf[2] = ud.m_volume_number = 0;
                tempbuf[3] = ud.m_player_skill+1;

                if(ud.player_skill == 3)
                    ud.m_respawn_monsters = 1;
                else ud.m_respawn_monsters = 0;

                if(ud.m_coop == 0) ud.m_respawn_items = 1;
                else ud.m_respawn_items = 0;

                ud.m_respawn_inventory = 1;

                tempbuf[4] = ud.m_monsters_off;
                tempbuf[5] = ud.m_respawn_monsters;
                tempbuf[6] = ud.m_respawn_items;
                tempbuf[7] = ud.m_respawn_inventory;
                tempbuf[8] = ud.m_coop;
                tempbuf[9] = ud.m_marker;

                x = strlen(menuname[probey]);

                copybufbyte(menuname[probey],tempbuf+10,x);
                copybufbyte(menuname[probey],boardfilename,x+1);

                for(c=connecthead;c>=0;c=connectpoint2[c])
                    if(c != myconnectindex)
                        sendpacket(c,tempbuf,x+10);

                newgame(ud.m_volume_number,ud.m_level_number,ud.m_player_skill+1);
                enterlevel(MODE_GAME);
            }
            break;

        case 600:
            c = (320>>1) - 120;
            if((ps[myconnectindex].gm&MODE_GAME) != MODE_GAME)
                displayfragbar();
            rotatesprite(160<<16,26<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(160,31,0,0,&ud.user_name[myconnectindex][0]);

            x = probe(c,57-8,16,8,0);

            switch(x)
            {
                case -1:
                    ud.m_recstat = 0;
                    if(ps[myconnectindex].gm&MODE_GAME) cmenu(50);
                    else cmenu(0);
                    break;
                case 0:
                    ud.m_coop++;
                    if(ud.m_coop == 3) ud.m_coop = 0;
                    break;
                case 1:
                    ud.m_volume_number++;
                    if(ud.m_volume_number > 2) ud.m_volume_number = 0;
                    break;
                case 2:
                    ud.m_level_number++;
                    if(ud.m_level_number > 6) ud.m_level_number = 0;
                    break;
                case 3:
                    if(ud.m_monsters_off == 0)
                    {
                        ud.m_player_skill++;
                        if(ud.m_player_skill > 4)
                        {
                            ud.m_player_skill = 0;
                            ud.m_monsters_off = 1;
                        }
                    }
                    else ud.m_monsters_off = 0;

                    break;

                case 4:
                    if(ud.m_coop == 0)
                        ud.m_marker = !ud.m_marker;
                    break;

                case 5:
                    if(ud.m_coop == 1)
                        ud.m_ffire = !ud.m_ffire;
                    break;

                case 6:
                    if(boardfilename[0] == 0) break;

                    tempbuf[0] = 5;
                    tempbuf[1] = ud.m_level_number = 7;
                    tempbuf[2] = ud.m_volume_number = 0;
                    tempbuf[3] = ud.m_player_skill+1;

                    ud.level_number = ud.m_level_number;
                    ud.volume_number = ud.m_volume_number;

                    if( ud.m_player_skill == 3 ) ud.m_respawn_monsters = 1;
                    else ud.m_respawn_monsters = 0;

                    if(ud.m_coop == 0) ud.m_respawn_items = 1;
                    else ud.m_respawn_items = 0;

                    ud.m_respawn_inventory = 1;

                    tempbuf[4] = ud.m_monsters_off;
                    tempbuf[5] = ud.m_respawn_monsters;
                    tempbuf[6] = ud.m_respawn_items;
                    tempbuf[7] = ud.m_respawn_inventory;
                    tempbuf[8] = ud.m_coop;
                    tempbuf[9] = ud.m_marker;
                    tempbuf[10] = ud.m_ffire;

                    for(c=connecthead;c>=0;c=connectpoint2[c])
                    {
                        resetweapons(c);
                        resetinventory(c);

                        if(c != myconnectindex)
                            sendpacket(c,tempbuf,11);
                    }

                    newgame(ud.m_volume_number,ud.m_level_number,ud.m_player_skill+1);
                    enterlevel(MODE_GAME);

                    return;
                case 7:

                    tempbuf[0] = 5;
                    tempbuf[1] = ud.m_level_number;
                    tempbuf[2] = ud.m_volume_number;
                    tempbuf[3] = ud.m_player_skill+1;

                    if( ud.m_player_skill == 3 ) ud.m_respawn_monsters = 1;
                    else ud.m_respawn_monsters = 0;

                    if(ud.m_coop == 0) ud.m_respawn_items = 1;
                    else ud.m_respawn_items = 0;

                    ud.m_respawn_inventory = 1;

                    tempbuf[4] = ud.m_monsters_off;
                    tempbuf[5] = ud.m_respawn_monsters;
                    tempbuf[6] = ud.m_respawn_items;
                    tempbuf[7] = ud.m_respawn_inventory;
                    tempbuf[8] = ud.m_coop;
                    tempbuf[9] = ud.m_marker;
                    tempbuf[10] = ud.m_ffire;

                    for(c=connecthead;c>=0;c=connectpoint2[c])
                    {
                        resetweapons(c);
                        resetinventory(c);

                        if(c != myconnectindex)
                            sendpacket(c,tempbuf,11);
                    }

                    newgame(ud.m_volume_number,ud.m_level_number,ud.m_player_skill+1);
                    enterlevel(MODE_GAME);

                    return;

            }

            c += 40;

            if(ud.m_coop==1) gametext2(c+70,57-7-9,"COOPERATIVE PLAY",0);
            else if(ud.m_coop==2) gametext2(c+70,57-7-9,"REDNECK MATCH NO SPAWN",0);
            else gametext2(c+70,57-7-9,"REDNECK MATCH SPAWN",0);

            gametext2(c+70,57+16-7-9,volume_names[ud.m_volume_number],0);

            gametext2(c+70,57+16+16-7-9,&level_names[7*ud.m_volume_number+ud.m_level_number][0],0);

            if(ud.m_monsters_off == 0)
                gametext2(c+70,57+16+16+16-7-9,skill_names[ud.m_player_skill],0);
            else gametext2(c+70,57+16+16+16-7-9,"NONE",0);

            if(ud.m_coop == 0)
            {
                if(ud.m_marker)
                    gametext2(c+70,57+16+16+16+16-7-9,"ON",0);
                else gametext2(c+70,57+16+16+16+16-7-9,"OFF",0);
            }

            if(ud.m_coop == 1)
            {
                if(ud.m_ffire)
                    gametext2(c+70,57+16+16+16+16+16-7-9,"ON",0);
                else gametext2(c+70,57+16+16+16+16+16-7-9,"OFF",0);
            }

            c -= 44;

            gametext2(c,57-7-9,"GAME TYPE",0);
            sprintf(tempbuf,"EPISODE %ld",ud.m_volume_number+1);
            gametext2(c,57+16-7-9,tempbuf,0);
            sprintf(tempbuf,"LEVEL %ld",ud.m_level_number+1);
            gametext2(c,57+16+16-7-9,tempbuf,0);
            gametext2(c,57+16+16+16-7-9,"MONSTERS",0);

            if(ud.m_coop == 0)
                gametext2(c,57+16+16+16+16-7-9,"MARKERS",0);
            else
                gametext2(c,57+16+16+16+16-7-9,"MARKERS",1);

            if(ud.m_coop == 1)
                gametext2(c,57+16+16+16+16+16-7-9,"SHOOT CUZ'",0);
            else gametext2(c,57+16+16+16+16+16-7-9,"SHOOT CUZ'",1);

            
            if( boardfilename[0] != 0 )
                gametext2(c,57+16+16+16+16+16+16-7-9,boardfilename,0);
            else
                gametext2(c,57+16+16+16+16+16+16-7-9,"USER MAP",0);

            gametext2(c,57+16+16+16+16+16+16+16-7-9,"START GAME",0);

            break;
    }

    if( (ps[myconnectindex].gm&MODE_MENU) != MODE_MENU)
    {
        vscrn();
        cameraclock = totalclock;
        cameradist = 65536L;
    }
}
