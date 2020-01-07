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

#include "lava.h"

#ifdef RRRA
#define VERSION "REL 1.0"
#define HEAD   "REDNECK RAMPAGE RIDES AGAIN(tm) "VERSION" - KENTUCKY BOURBON EDITION"
#define HEAD2  "REDNECK RAMPAGE RIDES AGAIN(tm) "VERSION" - KENTUCKY BOURBON EDITION"
#else
#define VERSION "REL 1.01"
#define HEAD   "REDNECK RAMPAGE(tm) "VERSION" - MOONSHINE"
#define HEAD2  "REDNECK RAMPAGE(tm) "VERSION" - MOONSHINE"
#endif

extern char firstdemofile[];
extern int32 numlumps;
extern char debug_on;
extern short inputloc;
extern int recfilep,totalreccnt;
extern char todd[];
extern char trees[];
extern char sixteen[];
extern FILE *frecfilep;

void main(int argc,char **argv)
{
    long i, j, k, l;
    int var_10;
#ifdef RRRA
    int playmve = 1;
#endif

    //copyprotect();

    todd[0] = 'T';
    sixteen[0] = 'D';
    trees[0] = 'I';

#ifdef RRRA
    CONFIG_GetSetupFilename();
    CONFIG_ReadSetup();
    {
        short i;
        char *c;
        i = 1;
        if (argc > 1)
        {
            while (i < argc)
            {
                c = argv[i];
                if (*c == '?')
                {
                    comlinehelp(argv);
                    exit(0);
                }

                if(*c == '/')
                {
                    c++;
                    switch(*c)
                    {
                        case 'n':
                        case 'N':
                            c++;
                            if(*c == 'i' || *c == 'I')
                            {
                                playmve = 0;
                            }
                            break;
                    }
                }
                i++;
            }
        }
    }

    if (CheckParm("MAP") || CheckParm("NET"))
        playmve = 0;
    if (playmve) {
        //sub_DACF0(257);
        //sub_A89C0("REDINT.MVE",argv[0],1);
    }
#else

    initcdrom();
#endif

    setvmode(0x03);

#ifdef RRRA
    printstr(0,0,"                                                                                ",31);
    printstr(40-(strlen(HEAD2)>>1)-3,0,HEAD2,31);

    ud.multimode = 1;
    printstr(0,1,"                    COPYRIGHT XATRIX ENTERTAINMENT 1998                         ",31);

    printstr(0,2,"                REDNECK RAMPAGE RIDES AGAIN VERSION "VERSION"                     ",31);
#else
    printstr(0,0,"                                                                                ",79);
    printstr(40-(strlen(HEAD2)>>1)-3,0,HEAD2,79);

    ud.multimode = 1;
    printstr(0,1,"                    COPYRIGHT XATRIX ENTERTAINMENT 1997                         ",79);

    printstr(0,2,"                      REDNECK RAMPAGE VERSION "VERSION"                          ",79);
#endif

    l = Z_AvailHeap();
    if(l < (3192000-350000))
    {
        puts("\n\nYou don't have enough free memory to run Redneck Rampage.");
#ifndef RRRA
        puts("The DOS \"mem\" command should report 6,800K (or 6.8 megs)");
        puts("'of total memory free'.\n");
        printf("Redneck Rampage requires %ld more bytes to run.\n",3192000-350000-l);
#endif
        exit(0);
    }

    printf("\n\n\n");

    {
        char c, *wd;
        wd = getenv("windir");
        if (wd)
        {
            puts("\n\nRedneck Rampage is not designed to run in a DOS box.");
            puts("\nDouble-click the \"Rampage\" shortcut instead to restart");
            puts("\nyour computer in MS-DOS mode and launch the game.");
            puts("\n");
            printf("Mash yer keyboard to quit\n");
            printf("or press C to continue\n");
            c = getch();
            if(c != 'C' && c != 'c')
                return;
        }
    }

    initgroupfile("redneck.grp");
    checkcommandline(argc, argv);
    RegisterShutdownFunction( Shutdown );

    Startup();

    if( eightytwofifty && numplayers > 1 && (MusicDevice != NumSoundCards) )
    {
        puts("\n=========================================================================");
        puts("WARNING: 8250 UART detected.");
        puts("Music is being disabled and lower quality sound is being set.  We apologize");
        puts("for this, but it is necessary to maintain high frame rates while trying to");
        puts("play the game on an 8250.  We suggest upgrading to a 16550 or better UART");
        puts("for maximum performance.  Press any key to continue.");
        puts("=========================================================================\n");

        while( !KB_KeyWaiting() );
    }

    if(numplayers > 1)
    {
        ud.multimode = numplayers;
        sendlogon();
    }
    else if(boardfilename[0] != 0)
    {
        ud.m_level_number = 7;
        ud.m_volume_number = 0;
        ud.warp_on = 1;
    }

    getnames();

    if(ud.multimode > 1)
    {
        playerswhenstarted = ud.multimode;

        if(ud.warp_on == 0)
        {
            ud.m_monsters_off = 1;
            ud.m_player_skill = 0;
        }
    }

    ud.last_level = -1;

   RTS_Init(ud.rtsname);
   if(numlumps) printf("Using .RTS file:%s\n",ud.rtsname);

   if (CONTROL_JoystickEnabled)
       CONTROL_CenterJoystick
          (
          CenterCenter,
          UpperLeft,
          LowerRight,
          CenterThrottle,
          CenterRudder
          );


    if (ud.warp_on > 1)
    {
        if (numplayers > 1)
        {
            printf("Attemptin' to load multiplayer slot #%ld.\n", ud.warp_on - 2);
            var_10 = loadplayer(-ud.warp_on + 1);
        }
        else
        {
            printf("Attemptin' to load single player slot #%ld.\n", ud.warp_on - 2);
            var_10 = loadplayer(ud.warp_on - 2);
        }
        if (var_10 == -1)
        {
            printf("Error loadin' slot # %ld.\n", ud.warp_on - 2);
            ud.warp_on = 0;
        }
    }

        puts("Loadin' palette/lookups.");

    if( setgamemode(ScreenMode,ScreenWidth,ScreenHeight) < 0 )
    {
        printf("\nVESA driver for ( %ld * %ld ) not found/supported!\n",xdim,ydim);
        vidoption = ScreenMode = 2;
        setgamemode(ScreenMode,ScreenWidth,ScreenHeight);
    }

    genspriteremaps();

#ifdef RRRA
    if (numplayers < 2)
        initcdrom();
#endif

#ifdef VOLUMEONE
        if(numplayers > 4 || ud.multimode > 4)
            gameexit(" The full version of Duke Nukem 3D supports 5 or more players.");
#endif

    setbrightness(ud.brightness>>2,&ps[myconnectindex].palette[0]);

    ESCESCAPE;

//    getpackets();

    MAIN_LOOP_RESTART:

    FX_StopAllSounds();
    clearsoundlocks();

#ifndef RRRA
    if(ud.warp_on == 0)
        Logo();
    else
#endif
        if(ud.warp_on == 1)
    {
        newgame(ud.m_volume_number,ud.m_level_number,ud.m_player_skill);
        enterlevel(MODE_GAME);
    }
    else vscrn();

    if( ud.warp_on == 0 && playback() )
    {
        goto MAIN_LOOP_RESTART;
    }

    ud.warp_on = 0;

    if (waitabort)
    {
        goto MAIN_LOOP_RESTART;
    }

    while ( !(ps[myconnectindex].gm&MODE_END) ) //The whole loop!!!!!!!!!!!!!!!!!!
    {
        if( ud.recstat == 2 || ud.multimode > 1 || ( ud.show_help == 0 && (ps[myconnectindex].gm&MODE_MENU) != MODE_MENU ) )
            if( ps[myconnectindex].gm&MODE_GAME )
                if( moveloop() ) continue;

        if( ps[myconnectindex].gm&MODE_EOL || ps[myconnectindex].gm&MODE_RESTART )
        {
            if( ps[myconnectindex].gm&MODE_EOL )
            {
                closedemowrite();

                ready2send = 0;

                i = ud.screen_size;
                ud.screen_size = 0;
                vscrn();
                ud.screen_size = i;
#ifdef RRRA
                if (playerswhenstarted > 1 && numplayers > 1)
                    dobonus(0);
                else
                    dobonus2(0);
#else
                dobonus(0);
#endif

                if(ud.eog)
                {
                    ud.eog = 0;
                    if(ud.multimode < 2)
                    {
                        ps[myconnectindex].gm = MODE_MENU;
                        cmenu(0);
                        probey = 0;
                        goto MAIN_LOOP_RESTART;
                    }
                    else
                    {
                        ud.m_level_number = 0;
                        ud.level_number = 0;
                    }
                }
            }

            if(numplayers > 1) ps[myconnectindex].gm = MODE_GAME;
            enterlevel(ps[myconnectindex].gm);
            continue;
        }

        cheats();
        nonsharedkeys();

        if( (ud.show_help == 0 && ud.multimode < 2 && !(ps[myconnectindex].gm&MODE_MENU) ) || ud.multimode > 1 || ud.recstat == 2)
            i = min(max((totalclock-ototalclock)*(65536L/TICSPERFRAME),0),65536);
        else
            i = 65536;

        if(ud.multimode < 2)
            if(torchcnt)
            dotorch();

        displayrooms(screenpeek,i);
        displayrest(i);

//        if( KB_KeyPressed(sc_F) )
//        {
//            KB_ClearKeyDown(sc_F);
//            addplayer();
//        }

        if(ps[myconnectindex].gm&MODE_DEMO)
            goto MAIN_LOOP_RESTART;

        if(debug_on) caches();

        checksync();
        nextpage();
    }

    gameexit(" ");
}

char opendemoread(char which_demo) // 0 = mine
{
    char *d = "demo_.dmo";
    char ver;
    short i;

    if(which_demo == 10)
        d[4] = 'x';
    else
        d[4] = '0' + which_demo;

     if(which_demo == 1 && firstdemofile[0] != 0)
     {
       if ((recfilep = kopen4load(firstdemofile,loadfromgrouponly)) == -1) return(0);
     }
     else
       if ((recfilep = kopen4load(d,loadfromgrouponly)) == -1) return(0);

     kread(recfilep,&ud.reccnt,sizeof(long));
     kread(recfilep,&ver,sizeof(char));
     if( (ver != BYTEVERSION) || (ud.reccnt < 1024) )
     {
        kclose(recfilep);
        return 0;
     }
         kread(recfilep,(char *)&ud.volume_number,sizeof(char));
         kread(recfilep,(char *)&ud.level_number,sizeof(char));
         kread(recfilep,(char *)&ud.player_skill,sizeof(char));
     kread(recfilep,(char *)&ud.m_coop,sizeof(char));
     kread(recfilep,(char *)&ud.m_ffire,sizeof(char));
     kread(recfilep,(short *)&ud.multimode,sizeof(short));
     kread(recfilep,(short *)&ud.m_monsters_off,sizeof(short));
     kread(recfilep,(int32 *)&ud.m_respawn_monsters,sizeof(int32));
     kread(recfilep,(int32 *)&ud.m_respawn_items,sizeof(int32));
     kread(recfilep,(int32 *)&ud.m_respawn_inventory,sizeof(int32));
     kread(recfilep,(int32 *)&ud.playerai,sizeof(int32));
     kread(recfilep,(char *)&ud.user_name[0][0],sizeof(ud.user_name),1);

     for(i=0;i<ud.multimode;i++)
        kread(recfilep,(char *)&ps[i].aim_mode,sizeof(char),1);
     ud.god = ud.cashman = ud.eog = ud.showallmap = 0;
     ud.clipping = ud.scrollmode = ud.overhead_on = 0;
     ud.showweapons =  ud.pause_on = ud.auto_run = 0;

         newgame(ud.volume_number,ud.level_number,ud.player_skill);
         return(1);
}


void opendemowrite(void)
{
    char *d = "demo1.dmo";
    long dummylong = 0;
    char ver;
    short i;

    if(ud.recstat == 2) kclose(recfilep);

    ver = BYTEVERSION;

    if ((frecfilep = fopen(d,"wb")) == NULL) return;
    fwrite(&dummylong,4,1,frecfilep);
    fwrite(&ver,sizeof(char),1,frecfilep);
    fwrite((char *)&ud.volume_number,sizeof(char),1,frecfilep);
    fwrite((char *)&ud.level_number,sizeof(char),1,frecfilep);
    fwrite((char *)&ud.player_skill,sizeof(char),1,frecfilep);
    fwrite((char *)&ud.m_coop,sizeof(char),1,frecfilep);
    fwrite((char *)&ud.m_ffire,sizeof(char),1,frecfilep);
    fwrite((short *)&ud.multimode,sizeof(short),1,frecfilep);
    fwrite((short *)&ud.m_monsters_off,sizeof(short),1,frecfilep);
    fwrite((int32 *)&ud.m_respawn_monsters,sizeof(int32),1,frecfilep);
    fwrite((int32 *)&ud.m_respawn_items,sizeof(int32),1,frecfilep);
    fwrite((int32 *)&ud.m_respawn_inventory,sizeof(int32),1,frecfilep);
    fwrite((int32 *)&ud.playerai,sizeof(int32),1,frecfilep);
    fwrite((char *)&ud.user_name[0][0],sizeof(ud.user_name),1,frecfilep);

    for(i=0;i<ud.multimode;i++)
        fwrite((char *)&ps[i].aim_mode,sizeof(char),1,frecfilep);

    totalreccnt = 0;
    ud.reccnt = 0;
}

void record(void)
{
    short i;

    for(i=connecthead;i>=0;i=connectpoint2[i])
         {
         copybufbyte(&sync[i],&recsync[ud.reccnt],sizeof(input));
                 ud.reccnt++;
                 totalreccnt++;
                 if (ud.reccnt >= RECSYNCBUFSIZ)
                 {
              dfwrite(recsync,sizeof(input)*ud.multimode,ud.reccnt/ud.multimode,frecfilep);
                          ud.reccnt = 0;
                 }
         }
}

void closedemowrite(void)
{
         if (ud.recstat == 1)
         {
        if (ud.reccnt > 0)
        {
            dfwrite(recsync,sizeof(input)*ud.multimode,ud.reccnt/ud.multimode,frecfilep);
        }

        fseek(frecfilep,SEEK_SET,0L);
        fwrite(&totalreccnt,sizeof(long),1,frecfilep);
        ud.recstat = ud.m_recstat = 0;
        fclose(frecfilep);
    }
}

char which_demo = 1;

char in_menu = 0;

// extern long syncs[];
long playback(void)
{
    long i,j,k,l,t;
    short p;
    char foundemo;

    if( ready2send ) return 0;

    foundemo = 0;

    RECHECK:

    in_menu = ps[myconnectindex].gm&MODE_MENU;

    pub = NUMPAGES;
    pus = NUMPAGES;

    flushperms();

    if(numplayers < 2) foundemo = opendemoread(which_demo);

    if(foundemo == 0)
    {
        if(which_demo > 1)
        {
            which_demo = 1;
            goto RECHECK;
        }
        for(t=0;t<63;t+=7) palto(0,0,0,t);
        drawbackground();
        menus();
        ps[myconnectindex].palette = palette;
        nextpage();
        for(t=63;t>0;t-=7) palto(0,0,0,t);
        ud.reccnt = 0;
    }
    else
    {
        ud.recstat = 2;
        which_demo++;
        if(which_demo == 10) which_demo = 1;
        enterlevel(MODE_DEMO);
    }

    if(foundemo == 0 || in_menu || KB_KeyWaiting() || ud.multimode > 1)
    {
        FX_StopAllSounds();
        clearsoundlocks();
        ps[myconnectindex].gm |= MODE_MENU;
    }

    ready2send = 0;
    i = 0;

    KB_FlushKeyboardQueue();

    k = 0;

    while (ud.reccnt > 0 || foundemo == 0)
    {
        if(foundemo) while ( totalclock >= (lockclock+TICSPERFRAME) )
        {
            if ((i == 0) || (i >= RECSYNCBUFSIZ))
            {
                i = 0;
                l = min(ud.reccnt,RECSYNCBUFSIZ);
                kdfread(recsync,sizeof(input)*ud.multimode,l/ud.multimode,recfilep);
            }

            for(j=connecthead;j>=0;j=connectpoint2[j])
            {
               copybufbyte(&recsync[i],&inputfifo[movefifoend[j]&(MOVEFIFOSIZ-1)][j],sizeof(input));
               movefifoend[j]++;
               i++;
               ud.reccnt--;
            }
            domovethings();
        }

        if(foundemo == 0)
            drawbackground();
        else
        {
            nonsharedkeys();

            j = min(max((totalclock-lockclock)*(65536/TICSPERFRAME),0),65536);
            displayrooms(screenpeek,j);
            displayrest(j);

            if(ud.multimode > 1 && ps[myconnectindex].gm )
                getpackets();
        }

        if( (ps[myconnectindex].gm&MODE_MENU) && (ps[myconnectindex].gm&MODE_EOL) )
            goto RECHECK;

        if (KB_KeyPressed(sc_Escape))
        {
            KB_ClearKeyDown(sc_Escape);
            FX_StopAllSounds();
            clearsoundlocks();
            ps[myconnectindex].gm |= MODE_MENU;
            cmenu(0);
            intomenusounds();
        }

        if(ps[myconnectindex].gm&MODE_TYPE)
        {
            typemode();
            if((ps[myconnectindex].gm&MODE_TYPE) != MODE_TYPE)
                ps[myconnectindex].gm = MODE_MENU;
        }
        else
        {
            menus();
            if( ud.multimode > 1 )
            {
                ControlInfo noshareinfo;
                CONTROL_GetInput( &noshareinfo );
                if( BUTTON(gamefunc_SendMessage) )
                {
                    KB_FlushKeyboardQueue();
                    CONTROL_ClearButton( gamefunc_SendMessage );
                    ps[myconnectindex].gm = MODE_TYPE;
                    typebuf[0] = 0;
                    inputloc = 0;
                }
            }
        }

        operatefta();

        if(ud.last_camsprite != ud.camerasprite)
        {
            ud.last_camsprite = ud.camerasprite;
            ud.camera_time = totalclock+(TICRATE*2);
        }

        getpackets();
        nextpage();

        if( ps[myconnectindex].gm==MODE_END || ps[myconnectindex].gm==MODE_GAME )
        {
            kclose(recfilep);
            return 0;
        }
    }
    kclose(recfilep);
    if(ps[myconnectindex].gm&MODE_MENU) goto RECHECK;
    return 1;
}

char moveloop()
{
    long i;

    if (numplayers > 1)
        while (fakemovefifoplc < movefifoend[myconnectindex]) fakedomovethings();

    getpackets();

    if (numplayers < 2) bufferjitter = 0;
    while (movefifoend[myconnectindex]-movefifoplc > bufferjitter)
    {
        for(i=connecthead;i>=0;i=connectpoint2[i])
            if (movefifoplc == movefifoend[i]) break;
        if (i >= 0) break;
        if( domovethings() ) return 1;
    }
    return 0;
}

void fakedomovethingscorrect(void)
{
     long i;
     struct player_struct *p;

     if (numplayers < 2) return;

     i = ((movefifoplc-1)&(MOVEFIFOSIZ-1));
     p = &ps[myconnectindex];

     if (p->posx == myxbak[i] && p->posy == myybak[i] && p->posz == myzbak[i]
          && p->horiz == myhorizbak[i] && p->ang == myangbak[i]) return;

     myx = p->posx; omyx = p->oposx; myxvel = p->posxv;
     myy = p->posy; omyy = p->oposy; myyvel = p->posyv;
     myz = p->posz; omyz = p->oposz; myzvel = p->poszv;
     myang = p->ang; omyang = p->oang;
     mycursectnum = p->cursectnum;
     myhoriz = p->horiz; omyhoriz = p->ohoriz;
     myhorizoff = p->horizoff; omyhorizoff = p->ohorizoff;
     myjumpingcounter = p->jumping_counter;
     myjumpingtoggle = p->jumping_toggle;
     myonground = p->on_ground;
     myhardlanding = p->hard_landing;
     myreturntocenter = p->return_to_center;

     fakemovefifoplc = movefifoplc;
     while (fakemovefifoplc < movefifoend[myconnectindex])
          fakedomovethings();

}

void fakedomovethings(void)
{
        input *syn;
        struct player_struct *p;
        long i, j, k, doubvel, fz, cz, hz, lz, x, y;
        unsigned long sb_snum;
        short psect, psectlotag, tempsect, backcstat;
        char shrunk, spritebridge;

        syn = (input *)&inputfifo[fakemovefifoplc&(MOVEFIFOSIZ-1)][myconnectindex];

        p = &ps[myconnectindex];

        backcstat = sprite[p->i].cstat;
        sprite[p->i].cstat &= ~257;

        sb_snum = syn->bits;

        psect = mycursectnum;
        psectlotag = sector[psect].lotag;
        spritebridge = 0;

        shrunk = (sprite[p->i].yrepeat < 8);

        if( ud.clipping == 0 && ( sector[psect].floorpicnum == MIRROR || psect < 0 || psect >= MAXSECTORS) )
        {
            myx = omyx;
            myy = omyy;
        }
        else
        {
            omyx = myx;
            omyy = myy;
        }

        omyhoriz = myhoriz;
        omyhorizoff = myhorizoff;
        omyz = myz;
        omyang = myang;

        getzrange(myx,myy,myz,psect,&cz,&hz,&fz,&lz,163L,CLIPMASK0);

        j = getflorzofslope(psect,myx,myy);

        if( (lz&49152) == 16384 && psectlotag == 1 && klabs(myz-j) > PHEIGHT+(16<<8) )
            psectlotag = 0;

        if( p->aim_mode == 0 && myonground && psectlotag != 2 && (sector[psect].floorstat&2) )
        {
                x = myx+(sintable[(myang+512)&2047]>>5);
                y = myy+(sintable[myang&2047]>>5);
                tempsect = psect;
                updatesector(x,y,&tempsect);
                if (tempsect >= 0)
                {
                     k = getflorzofslope(psect,x,y);
                     if (psect == tempsect)
                          myhorizoff += mulscale16(j-k,160);
                     else if (klabs(getflorzofslope(tempsect,x,y)-k) <= (4<<8))
                          myhorizoff += mulscale16(j-k,160);
                }
        }
        if (myhorizoff > 0) myhorizoff -= ((myhorizoff>>3)+1);
        else if (myhorizoff < 0) myhorizoff += (((-myhorizoff)>>3)+1);

        if(hz >= 0 && (hz&49152) == 49152)
        {
                hz &= (MAXSPRITES-1);
                if (sprite[hz].statnum == 1 && sprite[hz].extra >= 0)
                {
                    hz = 0;
                    cz = getceilzofslope(psect,myx,myy);
                }
        }

        if(lz >= 0 && (lz&49152) == 49152)
        {
                 j = lz&(MAXSPRITES-1);
                 if ((sprite[j].cstat&33) == 33)
                 {
                        psectlotag = 0;
                        spritebridge = 1;
                 }
                 if(badguy(&sprite[j]) && sprite[j].xrepeat > 24 && klabs(sprite[p->i].z-sprite[j].z) < (84<<8) )
                 {
                    j = getangle( sprite[j].x-myx,sprite[j].y-myy);
                    myxvel -= sintable[(j+512)&2047]<<4;
                    myyvel -= sintable[j&2047]<<4;
                }
        }

        if( sprite[p->i].extra <= 0 )
        {
                 if( psectlotag == 2 )
                 {
                            if(p->on_warping_sector == 0)
                            {
                                     if( klabs(myz-fz) > (PHEIGHT>>1))
                                             myz += 348;
                            }
                            clipmove(&myx,&myy,&myz,&mycursectnum,0,0,164L,(4L<<8),(4L<<8),CLIPMASK0);
                 }

                 updatesector(myx,myy,&mycursectnum);
                 pushmove(&myx,&myy,&myz,&mycursectnum,128L,(4L<<8),(20L<<8),CLIPMASK0);

                myhoriz = 100;
                myhorizoff = 0;

                 goto ENDFAKEPROCESSINPUT;
        }

        doubvel = TICSPERFRAME;

        if(p->on_crane >= 0) goto FAKEHORIZONLY;

        if(p->one_eighty_count < 0) myang += 128;

        i = 40;

        if( psectlotag == 2)
        {
                 myjumpingcounter = 0;

#ifdef RRRA
                 if ( (sb_snum&1) && !(p->OnMotorcycle || p->OnBoat) )
#else
                 if ( sb_snum&1 )
#endif
                 {
                            if(myzvel > 0) myzvel = 0;
                            myzvel -= 348;
                            if(myzvel < -(256*6)) myzvel = -(256*6);
                 }
#ifdef RRRA
                 else if ( (sb_snum&(1<<1)) && !(p->OnMotorcycle || p->OnBoat) )
#else
                 else if (sb_snum&(1<<1))
#endif
                 {
                            if(myzvel < 0) myzvel = 0;
                            myzvel += 348;
                            if(myzvel > (256*6)) myzvel = (256*6);
                 }
                 else
                 {
                    if(myzvel < 0)
                    {
                        myzvel += 256;
                        if(myzvel > 0)
                            myzvel = 0;
                    }
                    if(myzvel > 0)
                    {
                        myzvel -= 256;
                        if(myzvel < 0)
                            myzvel = 0;
                    }
                }

                if(myzvel > 2048) myzvel >>= 1;

                 myz += myzvel;

                 if(myz > (fz-(15<<8)) )
                            myz += ((fz-(15<<8))-myz)>>1;

                 if(myz < (cz+(4<<8)) )
                 {
                            myz = cz+(4<<8);
                            myzvel = 0;
                 }
        }

        else if(p->jetpack_on)
        {
                 myonground = 0;
                 myjumpingcounter = 0;
                 myhardlanding = 0;

                 if(p->jetpack_on < 11)
                            myz -= (p->jetpack_on<<7); //Goin up

                 if(shrunk) j = 512;
                 else j = 2048;
                 
#ifdef RRRA
                 if ((sb_snum&1) && !(p->OnMotorcycle || p->OnBoat))
#else
                 if (sb_snum&1)                            //A
#endif
                            myz -= j;
#ifdef RRRA
                 if ((sb_snum&(1<<1)) && !(p->OnMotorcycle || p->OnBoat))
#else
                 if (sb_snum&(1<<1))                       //Z
#endif
                            myz += j;

                 if(shrunk == 0 && ( psectlotag == 0 || psectlotag == 2 ) ) k = 32;
                 else k = 16;

                 if(myz > (fz-(k<<8)) )
                            myz += ((fz-(k<<8))-myz)>>1;
                 if(myz < (cz+(18<<8)) )
                            myz = cz+(18<<8);
        }
        else if( psectlotag != 2 )
        {
            if (psectlotag == 1 && p->spritebridge == 0)
            {
                 if(shrunk == 0) i = 34;
                 else i = 12;
            }
                 if(myz < (fz-(i<<8)) && (floorspace(psect)|ceilingspace(psect)) == 0 ) //falling
                 {
#ifdef RRRA
                            if( (sb_snum&3) == 0 && !(p->OnMotorcycle || p->OnBoat) && myonground && (sector[psect].floorstat&2) && myz >= (fz-(i<<8)-(16<<8) ) )
#else
                            if( (sb_snum&3) == 0 && myonground && (sector[psect].floorstat&2) && myz >= (fz-(i<<8)-(16<<8) ) )
#endif
                                     myz = fz-(i<<8);
                            else
                            {
                                     myonground = 0;

                                     myzvel += (gc+80);

                                     if(myzvel >= (4096+2048)) myzvel = (4096+2048);
                            }
                 }

                 else
                 {
                            if(psectlotag != 1 && psectlotag != 2 && myonground == 0 && myzvel > (6144>>1))
                                 myhardlanding = myzvel>>10;
                            myonground = 1;

                            if(i==40)
                            {
                                     //Smooth on the ground

                                     k = ((fz-(i<<8))-myz)>>1;
                                     if( klabs(k) < 256 ) k = 0;
                                     myz += k; // ((fz-(i<<8))-myz)>>1;
                                     myzvel -= 768; // 412;
                                     if(myzvel < 0) myzvel = 0;
                            }
                            else if(myjumpingcounter == 0)
                            {
                                myz += ((fz-(i<<7))-myz)>>1; //Smooth on the water
                                if(p->on_warping_sector == 0 && myz > fz-(16<<8))
                                {
                                    myz = fz-(16<<8);
                                    myzvel >>= 1;
                                }
                            }

#ifdef RRRA
                            if( (sb_snum&2) && !(p->OnMotorcycle || p->OnBoat) )
#else
                            if( sb_snum&2 )
#endif
                                     myz += (2048+768);

#ifdef RRRA
                            if( (sb_snum&1) == 0 && !(p->OnMotorcycle || p->OnBoat) && myjumpingtoggle == 1)
#else
                            if( (sb_snum&1) == 0 && myjumpingtoggle == 1)
#endif
                                     myjumpingtoggle = 0;

#ifdef RRRA
                            else if( (sb_snum&1) && !(p->OnMotorcycle || p->OnBoat) && myjumpingtoggle == 0 )
#else
                            else if( (sb_snum&1) && myjumpingtoggle == 0 )
#endif
                            {
                                     if( myjumpingcounter == 0 )
                                             if( (fz-cz) > (56<<8) )
                                             {
                                                myjumpingcounter = 1;
                                                myjumpingtoggle = 1;
                                             }
                            }
                 }

                 if(myjumpingcounter)
                 {
#ifdef RRRA
                            if( (sb_snum&1) == 0 && !(p->OnMotorcycle || p->OnBoat) && myjumpingtoggle == 1)
#else
                            if( (sb_snum&1) == 0 && myjumpingtoggle == 1)
#endif
                                     myjumpingtoggle = 0;

                            if( myjumpingcounter < (1024+256) )
                            {
                                     if(psectlotag == 1 && myjumpingcounter > 768)
                                     {
                                             myjumpingcounter = 0;
                                             myzvel = -512;
                                     }
                                     else
                                     {
                                             myzvel -= (sintable[(2048-128+myjumpingcounter)&2047])/12;
                                             myjumpingcounter += 180;

                                             myonground = 0;
                                     }
                            }
                            else
                            {
                                     myjumpingcounter = 0;
                                     myzvel = 0;
                            }
                 }

                 myz += myzvel;

                 if(myz < (cz+(4<<8)) )
                 {
                            myjumpingcounter = 0;
                            if(myzvel < 0) myxvel = myyvel = 0;
                            myzvel = 128;
                            myz = cz+(4<<8);
                 }

        }

        if ( p->fist_incs ||
                     p->transporter_hold > 2 ||
                     myhardlanding ||
                     p->access_incs > 0 ||
                     p->knee_incs > 0 ||
                     (p->curr_weapon == TRIPBOMB_WEAPON &&
                      p->kickback_pic > 1 &&
                      p->kickback_pic < 4 ) )
        {
                 doubvel = 0;
                 myxvel = 0;
                 myyvel = 0;
        }
        else if ( syn->avel )          //p->ang += syncangvel * constant
        {                         //ENGINE calculates angvel for you
            long tempang;

            tempang = syn->avel<<1;

            if(psectlotag == 2)
                myang += (tempang-(tempang>>3))*sgn(doubvel);
            else myang += (tempang)*sgn(doubvel);
            myang &= 2047;
        }

        if ( myxvel || myyvel || syn->fvel || syn->svel )
        {
                 if(p->steroids_amount > 0 && p->steroids_amount < 400)
                     doubvel <<= 1;

                 myxvel += ((syn->fvel*doubvel)<<6);
                 myyvel += ((syn->svel*doubvel)<<6);

#ifdef RRRA
                 if( ( p->curr_weapon == KNEE_WEAPON && p->kickback_pic > 10 && myonground ) || ( myonground && (sb_snum&2) && !(p->OnMotorcycle || p->OnBoat)) )
#else
                 if( ( p->curr_weapon == KNEE_WEAPON && p->kickback_pic > 10 && myonground ) || ( myonground && (sb_snum&2) ) )
#endif
                 {
                            myxvel = mulscale16(myxvel,rdnkfriction-0x2000);
                            myyvel = mulscale16(myyvel,rdnkfriction-0x2000);
                 }
                 else
                 {
                    if(psectlotag == 2)
                    {
                        myxvel = mulscale16(myxvel,rdnkfriction-0x1400);
                        myyvel = mulscale16(myyvel,rdnkfriction-0x1400);
                    }
                    else
                    {
                        myxvel = mulscale16(myxvel,rdnkfriction);
                        myyvel = mulscale16(myyvel,rdnkfriction);
                    }
                 }

                 if( abs(myxvel) < 2048 && abs(myyvel) < 2048 )
                     myxvel = myyvel = 0;

                 if( shrunk )
                 {
                     myxvel =
                         mulscale16(myxvel,(rdnkfriction)-(rdnkfriction>>1)+(rdnkfriction>>2));
                     myyvel =
                         mulscale16(myyvel,(rdnkfriction)-(rdnkfriction>>1)+(rdnkfriction>>2));
                 }
        }

FAKEHORIZONLY:
        if(psectlotag == 1 || spritebridge == 1) i = (4L<<8); else i = (20L<<8);

        clipmove(&myx,&myy,&myz,&mycursectnum,myxvel,myyvel,164L,4L<<8,i,CLIPMASK0);
        pushmove(&myx,&myy,&myz,&mycursectnum,164L,4L<<8,4L<<8,CLIPMASK0);

        if( p->jetpack_on == 0 && psectlotag != 1 && psectlotag != 2 && shrunk)
            myz += 30<<8;

        if ((sb_snum&(1<<18)) || myhardlanding)
            myreturntocenter = 9;

        if (sb_snum&(1<<13))
        {
                myreturntocenter = 9;
                if (sb_snum&(1<<5)) myhoriz += 6;
                myhoriz += 6;
        }
        else if (sb_snum&(1<<14))
        {
                myreturntocenter = 9;
                if (sb_snum&(1<<5)) myhoriz -= 6;
                myhoriz -= 6;
        }
#ifdef RRRA
        else if ((sb_snum&(1<<3)) && !(p->OnMotorcycle || p->OnBoat))
#else
        else if (sb_snum&(1<<3))
#endif
        {
                if (sb_snum&(1<<5)) myhoriz += 6;
                myhoriz += 6;
        }
#ifdef RRRA
        else if ((sb_snum&(1<<4)) && !(p->OnMotorcycle || p->OnBoat))
#else
        else if (sb_snum&(1<<4))
#endif
        {
                if (sb_snum&(1<<5)) myhoriz -= 6;
                myhoriz -= 6;
        }

        if (myreturntocenter > 0)
            if ((sb_snum&(1<<13)) == 0 && (sb_snum&(1<<14)) == 0)
        {
             myreturntocenter--;
             myhoriz += 33-(myhoriz/3);
        }

        if(p->aim_mode)
            myhoriz += syn->horz>>1;
        else
        {
            if( myhoriz > 95 && myhoriz < 105) myhoriz = 100;
            if( myhorizoff > -5 && myhorizoff < 5) myhorizoff = 0;
        }

        if (myhardlanding > 0)
        {
            myhardlanding--;
            myhoriz -= (myhardlanding<<4);
        }

        if (myhoriz > 299) myhoriz = 299;
        else if (myhoriz < -99) myhoriz = -99;

        if(p->knee_incs > 0)
        {
            myhoriz -= 48;
            myreturntocenter = 9;
        }


ENDFAKEPROCESSINPUT:

        myxbak[fakemovefifoplc&(MOVEFIFOSIZ-1)] = myx;
        myybak[fakemovefifoplc&(MOVEFIFOSIZ-1)] = myy;
        myzbak[fakemovefifoplc&(MOVEFIFOSIZ-1)] = myz;
        myangbak[fakemovefifoplc&(MOVEFIFOSIZ-1)] = myang;
        myhorizbak[fakemovefifoplc&(MOVEFIFOSIZ-1)] = myhoriz;
        fakemovefifoplc++;

        sprite[p->i].cstat = backcstat;
}
