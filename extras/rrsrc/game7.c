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

extern char confilename[];
extern int32 CommandSoundToggleOff;
extern int32 CommandMusicToggleOff;
extern char firstdemofile[];

#ifdef RRRA
#define VERSION "REL 1.0"
#define HEAD   "REDNECK RAMPAGE RIDES AGAIN(tm) "VERSION" - KENTUCKY BOURBON EDITION"
#define HEAD2  "REDNECK RAMPAGE RIDES AGAIN(tm) "VERSION" - KENTUCKY BOURBON EDITION"
#else
#define VERSION "REL 1.01"
#define HEAD   "REDNECK RAMPAGE(tm) "VERSION" - MOONSHINE"
#define HEAD2  "REDNECK RAMPAGE(tm) "VERSION" - MOONSHINE"
#endif

void comlinehelp(char **argv)
{
    printf("Command line help.  %s [/flags...]\n",argv[0]);
    puts(" ?, /?         This help message");
    puts(" /l##          Level (1-11)");
    puts(" /v#           Volume (1-4)");
    puts(" /v#           Volume (1-3)");
    puts(" /s#           Skill (1-4)");
    puts(" /r            Record demo");
    puts(" /dFILE        Start to play demo FILE");
    puts(" /m            No monsters");
    puts(" /ns           No sound");
    puts(" /nm           No music");
#ifdef RRRA
    puts(" /ni           No intro movie");
#endif
    puts(" /t#           Respawn, 1 = Monsters, 2 = Items, 3 = Inventory, x = All");
    puts(" /c#           MP mode, 1 = Redneck Match(spawn), 2 = Coop, 3 = Redneckmatch(no spawn)");
    puts(" /q#           Fake multiplayer (2-8 players)");
    puts(" /a            Use player AI (fake multiplayer only)");
    puts(" /i#           Network mode (1/0) (multiplayer only)");
    puts(" /f#           Send fewer packets (1, 2, 4) (multiplayer only)");
    puts(" /gFILE        Use a groupfile FILE");
    puts(" /xFILE        Compile FILE (default GAME.CON)");
    puts(" /u#########   User's favorite weapon order (default: 3425689071)");
    puts(" /#            Load and run a game (slot 0-9)");
    puts(" -8250         8250 with modem play, force music OFF");
    puts(" -map FILE     Use a map FILE");
    puts(" -name NAME    Foward NAME");
  printf(" -net          Net mode game");
}

void checkcommandline(int argc,char **argv)
{
    short i, j;
    char *c;

    i = 1;

    ud.fta_on = 1;
    ud.god = 0;
    ud.m_respawn_items = 0;
    ud.m_respawn_monsters = 0;
    ud.m_respawn_inventory = 0;
    ud.warp_on = 0;
    ud.cashman = 0;
    ud.m_player_skill = ud.player_skill = 1;

    if(argc > 1)
    {
        while(i < argc)
        {
            c = argv[i];
            if(*c == '-')
            {
                if( *(c+1) == '8' ) eightytwofifty = 1;
                i++;
                continue;
            }

            if(*c == '?')
            {
                comlinehelp(argv);
                exit(0);
            }

            if(*c == '/')
            {
                c++;
                switch(*c)
                {
                    default:
  //                      printf("Unknown command line parameter '%s'\n",argv[i]);
                    case '?':
                        comlinehelp(argv);
                        exit(0);
                    case 'x':
                    case 'X':
                        c++;
                        if(*c)
                        {
                            strcpy(confilename,c);
                            if(SafeFileExists(c) == 0)
                            {
                                printf("Could not find con file '%s'.\n",confilename );
                                exit(0);
                            }
                            else printf("Using con file: '%s'\n",confilename);
                        }
                        break;
                    case 'g':
                    case 'G':
                        c++;
                        if(*c)
                        {
                            j = initgroupfile(c);
                            if( j == -1 )
                                printf("Could not find group file %s.\n",c);
                            else
                            {
                                groupfile = j;
                                printf("Using group file %s.\n",c);
                            }
                        }

                        break;
                    case 'a':
                    case 'A':
                        ud.playerai = 1;
                        break;
                    case 'n':
                    case 'N':
                        c++;
                        if(*c == 's' || *c == 'S')
                        {
                            CommandSoundToggleOff = 2;
                            puts("Sound off.");
                        }
                        else if(*c == 'm' || *c == 'M')
                        {
                            CommandMusicToggleOff = 1;
                            puts("Music off.");
                        }
#ifdef RRRA
                        else if(*c == 'i' || *c == 'I')
                        {
                            puts("Intro movie off.");
                        }
#endif
                        else
                        {
                            comlinehelp(argv);
                            exit(0);
                        }
                        break;
                    case 'i':
                    case 'I':
                        c++;
                        if(*c == '0') networkmode = 0;
                        if(*c == '1') networkmode = 1;
                        printf("Network Mode %d\n",networkmode);
                        break;
                    case 'c':
                    case 'C':
                        c++;
                        if(*c == '1' || *c == '2' || *c == '3' )
                            ud.m_coop = *c - '0' - 1;
                        else ud.m_coop = 0;

                        switch(ud.m_coop)
                        {
                            case 0:
                                puts("Redneckmatch (spawn).");
                                break;
                            case 1:
                                puts("Cooperative play.");
                                break;
                            case 2:
                                puts("Redneckmatch (no spawn).");
                                break;
                        }

                        break;
                    case 'f':
                    case 'F':
                        c++;
                        if(*c == '1')
                            movesperpacket = 1;
                        if(*c == '2')
                            movesperpacket = 2;
                        if(*c == '4')
                        {
                            movesperpacket = 4;
#ifndef RRRA
                            setpackettimeout(0x3fffffff,0x3fffffff);
#endif
                        }
                        break;
                    case 't':
                    case 'T':
                        c++;
                        if(*c == '1') ud.m_respawn_monsters = 1;
                        else if(*c == '2') ud.m_respawn_items = 1;
                        else if(*c == '3') ud.m_respawn_inventory = 1;
                        else
                        {
                            ud.m_respawn_monsters = 1;
                            ud.m_respawn_items = 1;
                            ud.m_respawn_inventory = 1;
                        }
                        puts("Respawn on.");
                        break;
                    case 'm':
                    case 'M':
                        if( *(c+1) != 'a' && *(c+1) != 'A' )
                        {
                            ud.m_monsters_off = 1;
                            ud.m_player_skill = ud.player_skill = 0;
                            puts("Monsters off.");
                        }
                        break;
                    case 'q':
                    case 'Q':
                        puts("Fake multi Mode.");
                        if( *(++c) == 0) ud.multimode = 1;
                        else ud.multimode = atol(c)%17;
                        ud.m_coop = ud.coop = 0;
                        ud.m_marker = ud.marker = 1;
                        ud.m_respawn_monsters = ud.respawn_monsters = 1;
                        ud.m_respawn_items = ud.respawn_items = 1;
                        ud.m_respawn_inventory = ud.respawn_inventory = 1;

                        break;
                    case 'r':
                    case 'R':
                        ud.m_recstat = 1;
                        puts("Demo record mode on.");
                        break;
                    case 'd':
                    case 'D':
                        c++;
                        printf("Start playing demo %s.\n",c);
                        strcpy(firstdemofile,c);
                        break;
                    case 'l':
                    case 'L':
                        ud.warp_on = 1;
                        c++;
                        ud.m_level_number = ud.level_number = (atol(c)-1)%7;
                        break;
                    case 'j':
                    case 'J':
                        printf("Redneck Ramage (FULL VERSION) v%s\n",VERSION);
                        exit(0);

                    case 'v':
                    case 'V':
                        c++;
                        ud.warp_on = 1;
                        ud.m_volume_number = ud.volume_number = atol(c)-1;
                        break;
                    case 's':
                    case 'S':
                        c++;
                        ud.m_player_skill = ud.player_skill = (atol(c)%5);
                        if(ud.m_player_skill == 4)
                            ud.m_respawn_monsters = ud.respawn_monsters = 1;
                        break;
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        ud.warp_on = 2 + (*c) - '0';
                        break;
                    case 'u':
                    case 'U':
                        c++;
                        j = 0;
                        if(*c)
                        {
                            puts("Using favorite weapon order(s).");
                            while(*c)
                            {
                                ud.wchoice[0][j] = *c-'0';
                                c++;
                                j++;
                            }
                            while(j < 10)
                            {
                                if(j == 9)
                                    ud.wchoice[0][9] = 1;
                                else
                                    ud.wchoice[0][j] = 2;

                                j++;
                            }
                        }
                        else
                        {
                            puts("Using default weapon orders.");
                            ud.wchoice[0][0] = 3;
                            ud.wchoice[0][1] = 4;
                            ud.wchoice[0][2] = 5;
                            ud.wchoice[0][3] = 7;
                            ud.wchoice[0][4] = 8;
                            ud.wchoice[0][5] = 6;
                            ud.wchoice[0][6] = 0;
                            ud.wchoice[0][7] = 2;
                            ud.wchoice[0][8] = 9;
                            ud.wchoice[0][9] = 1;
                        }

                        break;
                }
            }
            i++;
        }
    }
}



void printstr(short x, short y, char string[81], char attribute)
{
        char character;
        short i, pos;

        pos = (y*80+x)<<1;
        i = 0;
        while (string[i] != 0)
        {
                character = string[i];
                printchrasm(0xb8000+(long)pos,1L,((long)attribute<<8)+(long)character);
                i++;
                pos+=2;
        }
}

/*
void cacheicon(void)
{
    if(cachecount > 0)
    {
        if( (ps[myconnectindex].gm&MODE_MENU) == 0 )
            rotatesprite((320-7)<<16,(200-23)<<16,32768L,0,SPINNINGNUKEICON,0,0,2,windowx1,windowy1,windowx2,windowy2);
        cachecount = 0;
    }
}
       */

void Logo(void)
{
    short i,j,soundanm;

    soundanm = 0;

    ready2send = 0;

    KB_FlushKeyboardQueue();

    setview(0,0,xdim-1,ydim-1);
    clearview(0L);
    palto(0,0,0,63);

    flushperms();
    nextpage();

#ifdef RRRA
    if (numplayers == 1)
    {
        sub_86730(0);
        ps[screenpeek].fogtype = 0;
        if (!KB_KeyWaiting())
        {
            short temp;
            temp = 0;
            getpackets();
            if (ready2send)
                return;
            temp = playanm("1st.anm",5,1);
            if (temp != 10)
            {
                temp = playanm("2nd.anm",5,2);
                if (temp != 10)
                {
                    temp = playanm("3nd.anm",5,3);
                    if (temp != 10)
                    {
                        playanm("4nd.anm",5,4);
                    }
                }
            }
            palto(0,0,0,63);
            KB_FlushKeyboardQueue();
        }

        FX_StopAllSounds();
        clearsoundlocks();
        flushperms();
        clearview(0L);
        nextpage();

        if(!KB_KeyWaiting())
        {
            getpackets();
            if(ready2send)
                return;
            playanm("xatlogo.anm",5,6);
            palto(0,0,0,63);
            KB_FlushKeyboardQueue();
        }

        FX_StopAllSounds();
        clearsoundlocks();
        flushperms();
        clearview(0L);
        nextpage();

        if(!KB_KeyWaiting())
        {
            getpackets();
            if(ready2send)
                return;
            playanm("inter.anm",5,5);
            palto(0,0,0,63);
            KB_FlushKeyboardQueue();
        }
    }
#else
    if(!KB_KeyWaiting())
    {
        getpackets();
        if(ready2send)
            return;
        playanm("rr_intro.anm",5,-1);
        palto(0,0,0,63);
        KB_FlushKeyboardQueue();
    }

    FX_StopAllSounds();
    clearsoundlocks();
    flushperms();
    clearview(0L);
    nextpage();

    if(!KB_KeyWaiting())
    {
        getpackets();
        if(ready2send)
            return;
        playanm("redneck.anm",5,0);
        palto(0,0,0,63);
        KB_FlushKeyboardQueue();
    }

    FX_StopAllSounds();
    clearsoundlocks();
    flushperms();
    clearview(0L);
    nextpage();

    if(!KB_KeyWaiting())
    {
        getpackets();
        if(ready2send)
            return;
        playanm("xatlogo.anm",5,1);
        palto(0,0,0,63);
        KB_FlushKeyboardQueue();
    }
#endif

    FX_StopAllSounds();
    clearsoundlocks();
    flushperms();
    clearview(0L);
    nextpage();
    ps[myconnectindex].palette = (char *) &palette[0];

    if(numplayers > 1)
    {
        palto(0,0,0,0);
        menutext(160,100,0,0,"WAITIN' FER PLAYERS...");
    }

    clearview(0L);
}

void loadtmb(void)
{
    char tmb[8000];
    long fil, l;

    fil = kopen4load("d3dtimbr.tmb",loadfromgrouponly);
    if(fil == -1) return;
    l = kfilelength(fil);
    kread(fil,(char *)tmb,l);
    MUSIC_RegisterTimbreBank(tmb);
    kclose(fil);
}

/*
===================
=
= ShutDown
=
===================
*/

void Shutdown( void )
{
    SoundShutdown();
    MusicShutdown();
    uninittimer();
#ifdef RRRA
    if (numplayers == 1 && playerswhenstarted == 1)
#endif
    uninitengine();
    CONTROL_Shutdown();
    CONFIG_WriteSetup();
#ifndef RRRA
    KB_Shutdown();
#endif
}

#ifdef RRRA
char todd[] = "Redneck Ramage Rides Again(tm) Copyright 1997 Xatrix Entertainment";
#else
char todd[] = "Redneck Ramage(tm) Copyright 1996 Xatrix Entertainment";
#endif
char trees[] = "I want to make a game with trees";
char sixteen[] = "16 Possible Rednecks";

/*
===================
=
= Startup
=
===================
*/

void compilecons(void)
{
   mymembuf = (char *)&hittype[0];
   labelcode = (long *)&sector[0];
   label = (char *)&sprite[0];

//   printf("%ld %ld %ld\n",sizeof(hittype),sizeof(sector),sizeof(sprite));
//   exit(0);

   loadefs(confilename,mymembuf);
   if( loadfromgrouponly )
   {
       printf("  * Writing defaults to current directory.\n");
       loadefs(confilename,mymembuf);
   }
}


void Startup(void)
{
   int i;

   KB_Startup();

   compilecons();

#ifndef RRRA
   CONFIG_GetSetupFilename();
   CONFIG_ReadSetup();
#endif

   if(CommandSoundToggleOff) SoundToggle = 0;
   if(CommandMusicToggleOff) MusicToggle = 0;

   CONTROL_Startup( ControllerType, &GetTime, TICRATE );

   initengine(ScreenMode,ScreenWidth,ScreenHeight);
   inittimer();

   puts("* Hold Esc to Abort. *");
   puts("Loading art header.");
   loadpics("tiles000.art");

   readsavenames();

   tilesizx[MIRROR] = tilesizy[MIRROR] = 0;

   for(i=0;i<MAXPLAYERS;i++) playerreadyflag[i] = 0;
   initmultiplayers(0,0,0);

   if(numplayers > 1)
    puts("Multiplayer initialized.");

   ps[myconnectindex].palette = (char *) &palette[0];
   SetupGameButtons();

   if(networkmode == 255)
   {
       if(numplayers > 1 && numplayers <= 4)
           networkmode = 1;
       else
           networkmode=0;
   }
   puts("Checking music inits.");
   MusicStartup();
   puts("Checking sound inits.");
   SoundStartup();
   loadtmb();
}


void sendscore(char *s)
{
    if(numplayers > 1)
#ifdef RRRA
      return;
#else
      genericmultifunction(-1,s,strlen(s)+1,5);
#endif
}


void getnames(void)
{
    short i,l;

    for(l=0;myname[l];l++)
    {
        ud.user_name[myconnectindex][l] = toupper(myname[l]);
        buf[l+2] = toupper(myname[l]);
    }

    if(numplayers > 1)
    {
        buf[0] = 6;
        buf[1] = BYTEVERSION;

        buf[l+2] = 0;
        l += 3;

        puts("Searching for other players.");
        waitforeverybody();

        for(i=connecthead;i>=0;i=connectpoint2[i])
            if( i != myconnectindex )
                sendpacket(i,&buf[0],l);

        getpackets();

        l = 1;
        buf[0] = 9;

        for(i=0;i<10;i++)
        {
            ud.wchoice[myconnectindex][i] = ud.wchoice[0][i];
            buf[l] = (char) ud.wchoice[0][i];
            l++;
        }

        for(i=connecthead;i>=0;i=connectpoint2[i])
            if(i != myconnectindex)
                sendpacket(i,&buf[0],l);

        getpackets();

        buf[0] = 10;
        buf[1] = ps[0].aim_mode;
        ps[myconnectindex].aim_mode = ps[0].aim_mode;

        for(i=connecthead;i>=0;i=connectpoint2[i])
            if(i != myconnectindex)
                sendpacket(i,buf,2);

        getpackets();
    }
}
