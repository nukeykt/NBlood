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


char everyothertime;
short vixenlevel = 0;
char domovethings(void)
{
    short i, j;
    char ch;

    for(i=connecthead;i>=0;i=connectpoint2[i])
        if( sync[i].bits&(1<<17) )
    {
        multiflag = 2;
        multiwhat = (sync[i].bits>>18)&1;
        multipos = (unsigned) (sync[i].bits>>19)&15;
        multiwho = i;

        if( multiwhat )
        {
            saveplayer( multipos );
            multiflag = 0;

            if(multiwho != myconnectindex)
            {
                strcpy(&fta_quotes[122],&ud.user_name[multiwho][0]);
                strcat(&fta_quotes[122]," SAVED A MULTIPLAYER GAME");
                FTA(122,&ps[myconnectindex]);
            }
            else
            {
                strcpy(&fta_quotes[122],"MULTIPLAYER GAME SAVED");
                FTA(122,&ps[myconnectindex]);
            }
            break;
        }
        else
        {
//            waitforeverybody();

            loadplayer( multipos );

            multiflag = 0;

            return 1;
        }
    }

    ud.camerasprite = -1;
    lockclock += TICSPERFRAME;

    if(earthquaketime > 0) earthquaketime--;
    if(user_quote_time > 0) user_quote_time--;
    if(rtsplaying > 0) rtsplaying--;

    if( show_shareware > 0 )
    {
        show_shareware--;
        if(show_shareware == 0)
        {
            pus = NUMPAGES;
            pub = NUMPAGES;
        }
    }

    everyothertime++;

    for(i=connecthead;i>=0;i=connectpoint2[i])
        copybufbyte(&inputfifo[movefifoplc&(MOVEFIFOSIZ-1)][i],&sync[i],sizeof(input));
    movefifoplc++;

    updateinterpolations();

    j = -1;
    for(i=connecthead;i>=0;i=connectpoint2[i])
     {
          if ((sync[i].bits&(1<<26)) == 0) { j = i; continue; }

          if (i == myconnectindex) gameexit(" ");
          if (screenpeek == i)
          {
                screenpeek = connectpoint2[i];
                if (screenpeek < 0) screenpeek = connecthead;
          }

          if (i == connecthead) connecthead = connectpoint2[connecthead];
          else connectpoint2[j] = connectpoint2[i];

          numplayers--;
          ud.multimode--;

          closedemowrite();

          pub = NUMPAGES;
          pus = NUMPAGES;
          vscrn();

          sprintf(buf,"%s is history!",ud.user_name[i]);

          quickkill(&ps[i]);
          deletesprite(ps[i].i);

          user_quote = buf;
          user_quote_time = 160;

          if(j < 0 && networkmode == 0 )
              gameexit( " \nThe 'MASTER/First player' just bit the bullet.  All\nplayers are returned from the game. This only happens in 5-8\nplayer mode as a different network scheme is used.");
      }

      if ((numplayers >= 2) && ((movefifoplc&7) == 7))
      {
            ch = (char)(randomseed&255);
            for(i=connecthead;i>=0;i=connectpoint2[i])
                 ch += ((ps[i].posx+ps[i].posy+ps[i].posz+ps[i].ang+ps[i].horiz)&255);
            syncval[myconnectindex][syncvalhead[myconnectindex]&(MOVEFIFOSIZ-1)] = ch;
            syncvalhead[myconnectindex]++;
      }

    if(ud.recstat == 1) record();

    if( ud.pause_on == 0 )
    {
        global_random = TRAND;
        movedummyplayers();//ST 13
    }

    for(i=connecthead;i>=0;i=connectpoint2[i])
    {
        cheatkeys(i);

        if( ud.pause_on == 0 )
        {
            processinput(i);
            checksectors(i);
        }
    }

    if( ud.pause_on == 0 )
    {
        movefta();//ST 2
        moveweapons();          //ST 5 (must be last)
        movetransports();       //ST 9

        moveplayers();          //ST 10
        movefallers();          //ST 12
        moveexplosions();       //ST 4

        moveactors();           //ST 1
        moveeffectors();        //ST 3

        movestandables();       //ST 6
        doanimations();
        movefx();               //ST 11

        if(numplayers < 2 && thunderon)
            thunder();
    }

    fakedomovethingscorrect();

    if( (everyothertime&1) == 0)
    {
        animatewalls();
        movecyclers();
        pan3dsound();
    }


    return 0;
}


void doorders(void)
{
    short i;

    setview(0,0,xdim-1,ydim-1);

    for(i=0;i<63;i+=7) palto(0,0,0,i);
    ps[myconnectindex].palette = palette;
    totalclock = 0;
    KB_FlushKeyboardQueue();
    rotatesprite(0,0,65536L,0,ORDERING,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
    nextpage(); for(i=63;i>0;i-=7) palto(0,0,0,i);
    totalclock = 0;while( !KB_KeyWaiting() ) getpackets();

    for(i=0;i<63;i+=7) palto(0,0,0,i);
    totalclock = 0;
    KB_FlushKeyboardQueue();
    rotatesprite(0,0,65536L,0,ORDERING+1,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
    nextpage(); for(i=63;i>0;i-=7) palto(0,0,0,i);
    totalclock = 0;while( !KB_KeyWaiting() ) getpackets();

    for(i=0;i<63;i+=7) palto(0,0,0,i);
    totalclock = 0;
    KB_FlushKeyboardQueue();
    rotatesprite(0,0,65536L,0,ORDERING+2,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
    nextpage(); for(i=63;i>0;i-=7) palto(0,0,0,i);
    totalclock = 0;while( !KB_KeyWaiting() ) getpackets();

    for(i=0;i<63;i+=7) palto(0,0,0,i);
    totalclock = 0;
    KB_FlushKeyboardQueue();
    rotatesprite(0,0,65536L,0,ORDERING+3,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
    nextpage(); for(i=63;i>0;i-=7) palto(0,0,0,i);
    totalclock = 0;while( !KB_KeyWaiting() ) getpackets();
}

#ifdef RRRA
void dobonus2(char bonusonly)
{
    short t, r, tinc,gfx_offset,bg_tile;
    long i, y,xfragtotal,yfragtotal,var24;
    short bonuscnt;

    var24 = 0;
    bonuscnt = 0;

    for(t=0;t<64;t++) palto(0,0,0,t);
    setview(0,0,xdim-1,ydim-1);
    clearview(0L);
    nextpage();
    flushperms();

    FX_StopAllSounds();
    clearsoundlocks();
    FX_SetReverb(0L);

    if (boardfilename[0] == 0 && numplayers < 2)
    {
        if ((ud.eog == 0 || ud.volume_number != 1) && ud.volume_number <= 1)
        {
            var24 = 1;
            MUSIC_StopSong();
            KB_FlushKeyboardQueue();
            ShowMapFrame();
        }
    }

    if(bonusonly) goto FRAGBONUS;

    FRAGBONUS:

    ps[myconnectindex].palette = palette;
    KB_FlushKeyboardQueue();
    totalclock = 0; tinc = 0;
    bonuscnt = 0;

    MUSIC_StopSong();

    if(playerswhenstarted > 1 && ud.coop != 1 )
    {
        if(!(MusicToggle == 0 || MusicDevice == NumSoundCards))
            sound(249);

        rotatesprite(0,0,65536L,0,MENUSCREEN,16,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
        rotatesprite(160<<16,57<<16,16592L,0,THREEDEE,0,0,2+8,0,0,xdim-1,ydim-1);
        gametext(160,58,"MULTIPLAYER TOTALS",0);
        gametext(160,58+10,level_names[(ud.volume_number*7)+ud.last_level-1],0);

        gametext(160,175,"PRESS ANY KEY TO CONTINUE",0);


        t = 0;
        minitext(23,80,"   NAME                                           KILLS",8,2+8+16+128);
        for(i=0;i<playerswhenstarted;i++)
        {
            sprintf(tempbuf,"%-4ld",i+1);
            minitext(92+(i*23),80,tempbuf,0,2+8+16+128);
        }

        for(i=0;i<playerswhenstarted;i++)
        {
            xfragtotal = 0;
            sprintf(tempbuf,"%ld",i+1);

            minitext(30,90+t,tempbuf,0,2+8+16+128);
            minitext(38,90+t,ud.user_name[i],ps[i].palookup,2+8+16+128);

            for(y=0;y<playerswhenstarted;y++)
            {
                if(i == y)
                {
                    sprintf(tempbuf,"%-4ld",ps[y].fraggedself);
                    minitext(92+(y*23),90+t,tempbuf,0,2+8+16+128);
                    xfragtotal -= ps[y].fraggedself;
                }
                else
                {
                    sprintf(tempbuf,"%-4ld",frags[i][y]);
                    minitext(92+(y*23),90+t,tempbuf,0,2+8+16+128);
                    xfragtotal += frags[i][y];
                }

                if(myconnectindex == connecthead)
                {
                    sprintf(tempbuf,"stats %ld killed %ld %ld\n",i+1,y+1,frags[i][y]);
                    sendscore(tempbuf);
                }
            }

            sprintf(tempbuf,"%-4ld",xfragtotal);
            minitext(101+(8*23),90+t,tempbuf,0,2+8+16+128);

            t += 7;
        }

        for(y=0;y<playerswhenstarted;y++)
        {
            yfragtotal = 0;
            for(i=0;i<playerswhenstarted;i++)
            {
                if(i == y)
                    yfragtotal += ps[i].fraggedself;
                yfragtotal += frags[i][y];
            }
            sprintf(tempbuf,"%-4ld",yfragtotal);
            minitext(92+(y*23),96+(8*7),tempbuf,0,2+8+16+128);
        }

        minitext(45,96+(8*7),"DEATHS",0,2+8+16+128);
        nextpage();

        for(t=0;t<64;t++)
            palto(0,0,0,63-t);

        KB_FlushKeyboardQueue();
        while(KB_KeyWaiting()==0) getpackets();

        if( KB_KeyPressed( sc_F12 ) )
        {
            KB_ClearKeyDown( sc_F12 );
            screencapture("rdnk0000.pcx",0);
        }

        if(bonusonly || ud.multimode > 1) return;

        for(t=0;t<64;t++) palto(0,0,0,t);
    }

    if(bonusonly || ud.multimode > 1) return;

    if(!(MusicToggle == 0 || MusicDevice == NumSoundCards))
        sound(249);

    
    gfx_offset = (ud.volume_number&1)*5;
    bg_tile = RRTILE403;
    if (ud.volume_number == 0)
        bg_tile = ud.level_number+RRTILE403-1;
    else
        bg_tile = ud.level_number+RRTILE409-1;

    if (lastlevel || vixenlevel)
        bg_tile = RRTILE409+7;

    if (boardfilename[0])
    {
        if (!var24)
        {
            rotatesprite(0,0,65536L,0,403,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
            endlvlmenutext(80,16,0,0,boardfilename);
        }
    }
    else if (!var24)
    {
        rotatesprite(0,0,65536L,0,bg_tile,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
        if ((lastlevel && ud.volume_number == 2) || vixenlevel)
            endlvlmenutext(80,16,0,0,"CLOSE ENCOUNTERS");
        else if (turdlevel)
            endlvlmenutext(80,16,0,0,"SMELTING PLANT");
        else
            endlvlmenutext(80,16,0,0,level_names[(ud.volume_number*7)+ud.last_level-1]);
    }
    else
    {
        endlvlmenutext(80,16,0,0,level_names[(ud.volume_number*7)+ud.last_level-1]);
    }

    endlvlmenutext(15,192,0,0,"PRESS ANY KEY TO CONTINUE");
    KB_FlushKeyboardQueue();
    if (!var24)
    {
        nextpage();
        for(t=0;t<64;t++) palto(0,0,0,63-t);
    }
    bonuscnt = 0;
    totalclock = 0; tinc = 0;

    while( 1 )
    {
        if(ps[myconnectindex].gm&MODE_EOL)
        {
            if (boardfilename[0])
            {
                if (!var24)
                    rotatesprite(0,0,65536L,0,403,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
            }
            else if (!var24)
                rotatesprite(0,0,65536L,0,bg_tile,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);

            if( totalclock > (1000000000L) && totalclock < (1000000320L) )
            {
                switch( ((unsigned long)totalclock>>4)%15 )
                {
                    case 0:
                        if(bonuscnt == 6)
                        {
                            bonuscnt++;
                            sound(425);
                            switch(rand()&3)
                            {
                                case 0:
                                    sound(195);
                                    break;
                                case 1:
                                    sound(196);
                                    break;
                                case 2:
                                    sound(197);
                                    break;
                                case 3:
                                    sound(199);
                                    break;
                            }
                        }
                    case 1:
                    case 4:
                    case 5:
                    case 2:
                    case 3:
                       break;
                }
            }
            else if( totalclock > (10240+120L) ) break;
            else
            {
                switch( (totalclock>>5)&3 )
                {
                    case 1:
                    case 3:
                        break;
                    case 2:
                        break;
                }
            }

            if (boardfilename[0])
            {
                if (!var24)
                {
                    rotatesprite(0,0,65536L,0,403,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
                    endlvlmenutext(80,16,0,0,boardfilename);
                }
            }
            else if (!var24)
            {
                rotatesprite(0,0,65536L,0,bg_tile,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
                if ((lastlevel && ud.volume_number == 2) || vixenlevel)
                    endlvlmenutext(80,16,0,0,"CLOSE ENCOUNTERS");
                else if (turdlevel)
                    endlvlmenutext(80,16,0,0,"SMELTING PLANT");
                else
                    endlvlmenutext(80,16,0,0,level_names[(ud.volume_number*7)+ud.last_level-1]);
            }
            else
            {
                endlvlmenutext(80,16,0,0,level_names[(ud.volume_number*7)+ud.last_level-1]);
            }

            endlvlmenutext(15,192,0,0,"PRESS ANY KEY TO CONTINUE");

            if( totalclock > (60*3) )
            {
                endlvlmenutext(30,48,0,0,"Yer Time:");
                endlvlmenutext(30,64,0,0,"Par time:");
                endlvlmenutext(30,80,0,0,"Xatrix Time:");
                if(bonuscnt == 0)
                    bonuscnt++;

                if( totalclock > (60*4) )
                {
                    if(bonuscnt == 1)
                    {
                        bonuscnt++;
                        sound(404);
                    }
                    sprintf(tempbuf,"%02ld : %02ld",
                        (ps[myconnectindex].player_par/(26*60))%60,
                        (ps[myconnectindex].player_par/26)%60);
                    endlvlmenutext(191,48,0,0,tempbuf);

                    if(!boardfilename[0])
                    {
                        sprintf(tempbuf,"%02ld : %02ld",
                            (partime[ud.volume_number*7+ud.last_level-1]/(26*60))%60,
                            (partime[ud.volume_number*7+ud.last_level-1]/26)%60);
                        endlvlmenutext(191,64,0,0,tempbuf);

                        sprintf(tempbuf,"%02ld : %02ld",
                            (designertime[ud.volume_number*7+ud.last_level-1]/(26*60))%60,
                            (designertime[ud.volume_number*7+ud.last_level-1]/26)%60);
                        endlvlmenutext(191,80,0,0,tempbuf);
                    }

                }
            }
            if( totalclock > (60*6) )
            {
                endlvlmenutext(30,112,0,0,"Varmints Killed:");
                endlvlmenutext(30,128,0,0,"Varmints Left:");

                if(bonuscnt == 2)
                    bonuscnt++;

                if( totalclock > (60*7) )
                {
                    if(bonuscnt == 3)
                    {
                        bonuscnt++;
                        sound(422);
                    }
                    sprintf(tempbuf,"%-3ld",ps[myconnectindex].actors_killed);
                    endlvlmenutext(231,112,0,0,tempbuf);
                    if(ud.player_skill > 3 )
                    {
                        sprintf(tempbuf,"N/A");
                        endlvlmenutext(231,128,0,0,tempbuf);
                    }
                    else
                    {
                        if( (ps[myconnectindex].max_actors_killed-ps[myconnectindex].actors_killed) < 0 )
                            sprintf(tempbuf,"%-3ld",0);
                        else sprintf(tempbuf,"%-3ld",ps[myconnectindex].max_actors_killed-ps[myconnectindex].actors_killed);
                        endlvlmenutext(231,128,0,0,tempbuf);
                    }
                }
            }
            if( totalclock > (60*9) )
            {
                endlvlmenutext(30,144,0,0,"Secrets Found:");
                endlvlmenutext(30,160,0,0,"Secrets Missed:");
                if(bonuscnt == 4) bonuscnt++;

                if( totalclock > (60*10) )
                {
                    if(bonuscnt == 5)
                    {
                        bonuscnt++;
                        sound(404);
                    }
                    sprintf(tempbuf,"%-3ld",ps[myconnectindex].secret_rooms);
                    endlvlmenutext(231,144,0,0,tempbuf);
                    if( ps[myconnectindex].secret_rooms > 0 )
                        sprintf(tempbuf,"%-3ld%",(100*ps[myconnectindex].secret_rooms/ps[myconnectindex].max_secret_rooms));
                    sprintf(tempbuf,"%-3ld",ps[myconnectindex].max_secret_rooms-ps[myconnectindex].secret_rooms);
                    endlvlmenutext(231,160,0,0,tempbuf);
                }
            }

            if(totalclock > 10240 && totalclock < 10240+10240)
                totalclock = 1024;

            if( KB_KeyWaiting() && totalclock > (60*2) )
            {
                if (var24)
                {
                    if (bonuscnt == 7)
                    {
                        bonuscnt++;
                        MUSIC_StopSong();
                        KB_FlushKeyboardQueue();
                        PlayMapAnim();
                    }
                    else if (bonuscnt == 8)
                    {
                        KB_FlushKeyboardQueue();
                        totalclock = 10361;
                        break;
                    }

                }
                if( KB_KeyPressed( sc_F12 ) )
                {
                    KB_ClearKeyDown( sc_F12 );
                    screencapture("rdnk0000.pcx",0);
                }

                if (var24)
                {
                    if( totalclock < (60*13) )
                    {
                        KB_FlushKeyboardQueue();
                        totalclock = (60*13);
                    }
                    else if( totalclock < (1000000000L))
                       totalclock = (1000000000L);
                }
                else
                {
                    if( totalclock < (60*13) )
                    {
                        KB_FlushKeyboardQueue();
                        totalclock = (60*13);
                    }
                    else if( totalclock < (1000000000L))
                       totalclock = (1000000000L);
                }
            }
        }
        else break;
        if (!var24 || bonuscnt)
            nextpage();
    }
    if (ud.eog)
    {
        for (t = 0; t < 64; t++) palto(0, 0, 0, t);
        clearview(0);
        nextpage();
        spritesound(35,ps[0].i);
        palto(0, 0, 0, 0);
        ps[myconnectindex].palette = palette;
        while (1)
        {
            int var40;
            switch ((totalclock >> 4) & 1)
            {
            case 0:
                rotatesprite(0,0,65536,0,0,RRTILE8677,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
                nextpage();
                palto(0, 0, 0, 0);
                ps[myconnectindex].palette = palette;
                getpackets();
                break;
            case 1:
                rotatesprite(0,0,65536,0,0,RRTILE8677+1,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
                nextpage();
                palto(0, 0, 0, 0);
                ps[myconnectindex].palette = palette;
                getpackets();
                break;
            }
            if (Sound[35].num == 0) break;
        }
    }
    if (word_119BE4)
    {
        word_119BE4 = 0;
        ud.m_volume_number = ud.volume_number = 1;
        ud.m_level_number = ud.level_number = 0;
        ud.eog = 0;
    }
    if (turdlevel)
        turdlevel = 0;
    if (vixenlevel)
        vixenlevel = 0;
}
#else
void dobonus(char bonusonly)
{
    short t, r, tinc,gfx_offset;
    long i, y,xfragtotal,yfragtotal;
    short bonuscnt;
    short bg_tile;

    bonuscnt = 0;

    for(t=0;t<64;t++) palto(0,0,0,t);
    setview(0,0,xdim-1,ydim-1);
    clearview(0L);
    nextpage();
    flushperms();

    FX_StopAllSounds();
    clearsoundlocks();
    FX_SetReverb(0L);

    if(bonusonly) goto FRAGBONUS;

    if(numplayers < 2 && ud.eog && ud.from_bonus == 0)
        switch(ud.volume_number)
    {
        case 0:
            MUSIC_StopSong();
            clearview(0L);
            nextpage();
            if(ud.lockout == 0)
            {
                playanm("turdmov.anm",5,5);
                KB_FlushKeyboardQueue();
                clearview(0L);
                nextpage();
            }
            ud.level_number = 0;
            ud.volume_number = 1;
            ud.eog = 0;

            for(t=0;t<64;t++) palto(0,0,0,t);

            KB_FlushKeyboardQueue();
            ps[myconnectindex].palette = palette;

            rotatesprite(0,0,65536L,0,1685,0,0,2+8+16+64,0,0,xdim-1,ydim-1);
            nextpage(); for(t=63;t>0;t--) palto(0,0,0,t);
            while( !KB_KeyWaiting() ) getpackets();
            for(t=0;t<64;t++) palto(0,0,0,t);
            MUSIC_StopSong();
            FX_StopAllSounds();
            clearsoundlocks();
            break;
        case 1:
            MUSIC_StopSong();
            clearview(0L);
            nextpage();

            if(ud.lockout == 0)
            {
                playanm("rr_outro.anm",5,4);
                KB_FlushKeyboardQueue();
                clearview(0L);
                nextpage();
            }
            lastlevel = 0;
            vixenlevel = 1;
            ud.level_number = 0;
            ud.volume_number = 0;

            for(t=0;t<64;t++) palto(0,0,0,t);
            setview(0,0,xdim-1,ydim-1);
            KB_FlushKeyboardQueue();
            ps[myconnectindex].palette = palette;
            rotatesprite(0,0,65536L,0,1685,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
            nextpage(); for(t=63;t>0;t--) palto(0,0,0,t);
            while( !KB_KeyWaiting() ) getpackets();
            for(t=0;t<64;t++) palto(0,0,0,t);

            break;

        case 2:
            KB_FlushKeyboardQueue();
            while( !KB_KeyWaiting() ) getpackets();

            FX_StopAllSounds();
            clearsoundlocks();
            KB_FlushKeyboardQueue();

            clearview(0L);
            nextpage();
            playanm("LNRDTEAM.ANM",4,3);
            KB_FlushKeyboardQueue();

            while( !KB_KeyWaiting() ) getpackets();

            FX_StopAllSounds();
            clearsoundlocks();

            KB_FlushKeyboardQueue();

            break;
    }

    FRAGBONUS:

    ps[myconnectindex].palette = palette;
    KB_FlushKeyboardQueue();
    totalclock = 0; tinc = 0;
    bonuscnt = 0;

    MUSIC_StopSong();
    FX_StopAllSounds();
    clearsoundlocks();

    if(playerswhenstarted > 1 && ud.coop != 1 )
    {
        if(!(MusicToggle == 0 || MusicDevice == NumSoundCards))
            sound(249);

        rotatesprite(0,0,65536L,0,MENUSCREEN,16,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
        rotatesprite(160<<16,24<<16,23592L,0,INGAMEDUKETHREEDEE,0,0,2+8,0,0,xdim-1,ydim-1);
        gametext(160,58,"MULTIPLAYER TOTALS",0);
        gametext(160,58+10,level_names[(ud.volume_number*7)+ud.last_level-1],0);

        gametext(160,175,"PRESS ANY KEY TO CONTINUE",0);


        t = 0;
        minitext(23,80,"   NAME                                           KILLS",8,2+8+16+128);
        for(i=0;i<playerswhenstarted;i++)
        {
            sprintf(tempbuf,"%-4ld",i+1);
            minitext(92+(i*23),80,tempbuf,0,2+8+16+128);
        }

        for(i=0;i<playerswhenstarted;i++)
        {
            xfragtotal = 0;
            sprintf(tempbuf,"%ld",i+1);

            minitext(30,90+t,tempbuf,0,2+8+16+128);
            minitext(38,90+t,ud.user_name[i],ps[i].palookup,2+8+16+128);

            for(y=0;y<playerswhenstarted;y++)
            {
                if(i == y)
                {
                    sprintf(tempbuf,"%-4ld",ps[y].fraggedself);
                    minitext(92+(y*23),90+t,tempbuf,0,2+8+16+128);
                    xfragtotal -= ps[y].fraggedself;
                }
                else
                {
                    sprintf(tempbuf,"%-4ld",frags[i][y]);
                    minitext(92+(y*23),90+t,tempbuf,0,2+8+16+128);
                    xfragtotal += frags[i][y];
                }

                if(myconnectindex == connecthead)
                {
                    sprintf(tempbuf,"stats %ld killed %ld %ld\n",i+1,y+1,frags[i][y]);
                    sendscore(tempbuf);
                }
            }

            sprintf(tempbuf,"%-4ld",xfragtotal);
            minitext(101+(8*23),90+t,tempbuf,0,2+8+16+128);

            t += 7;
        }

        for(y=0;y<playerswhenstarted;y++)
        {
            yfragtotal = 0;
            for(i=0;i<playerswhenstarted;i++)
            {
                if(i == y)
                    yfragtotal += ps[i].fraggedself;
                yfragtotal += frags[i][y];
            }
            sprintf(tempbuf,"%-4ld",yfragtotal);
            minitext(92+(y*23),96+(8*7),tempbuf,0,2+8+16+128);
        }

        minitext(45,96+(8*7),"DEATHS",0,2+8+16+128);
        nextpage();

        for(t=0;t<64;t++)
            palto(0,0,0,63-t);

        KB_FlushKeyboardQueue();
        while(KB_KeyWaiting()==0) getpackets();

        if( KB_KeyPressed( sc_F12 ) )
        {
            KB_ClearKeyDown( sc_F12 );
            screencapture("rdnk0000.pcx",0);
        }

        if(bonusonly || ud.multimode > 1) return;

        for(t=0;t<64;t++) palto(0,0,0,t);
    }

    if(bonusonly || ud.multimode > 1) return;

    if(!(MusicToggle == 0 || MusicDevice == NumSoundCards))
        sound(249);

    
    gfx_offset = (ud.volume_number&1)*5;
    bg_tile = RRTILE403;
    if (ud.volume_number == 0)
        bg_tile = ud.level_number+RRTILE403-1;
    else
        bg_tile = ud.level_number+RRTILE409-1;

    if (lastlevel || vixenlevel)
        bg_tile = RRTILE409+7;

    if (boardfilename[0])
    {
        rotatesprite(0,0,65536L,0,403,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
        endlvlmenutext(80,16,0,0,boardfilename);
    }
    else
    {
        rotatesprite(0,0,65536L,0,bg_tile,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
        if ((lastlevel && ud.volume_number == 2) || vixenlevel)
            endlvlmenutext(80,16,0,0,"CLOSE ENCOUNTERS");
        else if (turdlevel)
            endlvlmenutext(80,16,0,0,"SMELTING PLANT");
        else
            endlvlmenutext(80,16,0,0,level_names[(ud.volume_number*7)+ud.last_level-1]);
    }

    endlvlmenutext(15,192,0,0,"PRESS ANY KEY TO CONTINUE");
    nextpage();
    KB_FlushKeyboardQueue();
    for(t=0;t<64;t++) palto(0,0,0,63-t);
    bonuscnt = 0;
    totalclock = 0; tinc = 0;

    while( 1 )
    {
        if(ps[myconnectindex].gm&MODE_EOL)
        {
            if (boardfilename[0])
                rotatesprite(0,0,65536L,0,403,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
            else
                rotatesprite(0,0,65536L,0,bg_tile,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);

            if( totalclock > (1000000000L) && totalclock < (1000000320L) )
            {
                switch( ((unsigned long)totalclock>>4)%15 )
                {
                    case 0:
                        if(bonuscnt == 6)
                        {
                            bonuscnt++;
                            sound(425);
                            switch(rand()&3)
                            {
                                case 0:
                                    sound(195);
                                    break;
                                case 1:
                                    sound(196);
                                    break;
                                case 2:
                                    sound(197);
                                    break;
                                case 3:
                                    sound(199);
                                    break;
                            }
                        }
                    case 1:
                    case 4:
                    case 5:
                    case 2:
                    case 3:
                       break;
                }
            }
            else if( totalclock > (10240+120L) ) break;
            else
            {
                switch( (totalclock>>5)&3 )
                {
                    case 1:
                    case 3:
                        break;
                    case 2:
                        break;
                }
            }

            if (boardfilename[0])
            {
                rotatesprite(0,0,65536L,0,403,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
                endlvlmenutext(80,16,0,0,boardfilename);
            }
            else
            {
                rotatesprite(0,0,65536L,0,bg_tile,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
                if ((lastlevel && ud.volume_number == 2) || vixenlevel)
                    endlvlmenutext(80,16,0,0,"CLOSE ENCOUNTERS");
                else if (turdlevel)
                    endlvlmenutext(80,16,0,0,"SMELTING PLANT");
                else
                    endlvlmenutext(80,16,0,0,level_names[(ud.volume_number*7)+ud.last_level-1]);
            }

            endlvlmenutext(15,192,0,0,"PRESS ANY KEY TO CONTINUE");

            if( totalclock > (60*3) )
            {
                endlvlmenutext(30,48,0,0,"Yer Time:");
                endlvlmenutext(30,64,0,0,"Par time:");
                endlvlmenutext(30,80,0,0,"Xatrix Time:");
                if(bonuscnt == 0)
                    bonuscnt++;

                if( totalclock > (60*4) )
                {
                    if(bonuscnt == 1)
                    {
                        bonuscnt++;
                        sound(404);
                    }
                    sprintf(tempbuf,"%02ld : %02ld",
                        (ps[myconnectindex].player_par/(26*60))%60,
                        (ps[myconnectindex].player_par/26)%60);
                    endlvlmenutext(191,48,0,0,tempbuf);

                    if(!boardfilename[0])
                    {
                        sprintf(tempbuf,"%02ld : %02ld",
                            (partime[ud.volume_number*7+ud.last_level-1]/(26*60))%60,
                            (partime[ud.volume_number*7+ud.last_level-1]/26)%60);
                        endlvlmenutext(191,64,0,0,tempbuf);

                        sprintf(tempbuf,"%02ld : %02ld",
                            (designertime[ud.volume_number*7+ud.last_level-1]/(26*60))%60,
                            (designertime[ud.volume_number*7+ud.last_level-1]/26)%60);
                        endlvlmenutext(191,80,0,0,tempbuf);
                    }

                }
            }
            if( totalclock > (60*6) )
            {
                endlvlmenutext(30,112,0,0,"Varmints Killed:");
                endlvlmenutext(30,128,0,0,"Varmints Left:");

                if(bonuscnt == 2)
                    bonuscnt++;

                if( totalclock > (60*7) )
                {
                    if(bonuscnt == 3)
                    {
                        bonuscnt++;
                        sound(422);
                    }
                    sprintf(tempbuf,"%-3ld",ps[myconnectindex].actors_killed);
                    endlvlmenutext(231,112,0,0,tempbuf);
                    if(ud.player_skill > 3 )
                    {
                        sprintf(tempbuf,"N/A");
                        endlvlmenutext(231,128,0,0,tempbuf);
                    }
                    else
                    {
                        if( (ps[myconnectindex].max_actors_killed-ps[myconnectindex].actors_killed) < 0 )
                            sprintf(tempbuf,"%-3ld",0);
                        else sprintf(tempbuf,"%-3ld",ps[myconnectindex].max_actors_killed-ps[myconnectindex].actors_killed);
                        endlvlmenutext(231,128,0,0,tempbuf);
                    }
                }
            }
            if( totalclock > (60*9) )
            {
                endlvlmenutext(30,144,0,0,"Secrets Found:");
                endlvlmenutext(30,160,0,0,"Secrets Missed:");
                if(bonuscnt == 4) bonuscnt++;

                if( totalclock > (60*10) )
                {
                    if(bonuscnt == 5)
                    {
                        bonuscnt++;
                        sound(404);
                    }
                    sprintf(tempbuf,"%-3ld",ps[myconnectindex].secret_rooms);
                    endlvlmenutext(231,144,0,0,tempbuf);
                    if( ps[myconnectindex].secret_rooms > 0 )
                        sprintf(tempbuf,"%-3ld%",(100*ps[myconnectindex].secret_rooms/ps[myconnectindex].max_secret_rooms));
                    sprintf(tempbuf,"%-3ld",ps[myconnectindex].max_secret_rooms-ps[myconnectindex].secret_rooms);
                    endlvlmenutext(231,160,0,0,tempbuf);
                }
            }

            if(totalclock > 10240 && totalclock < 10240+10240)
                totalclock = 1024;

            if( KB_KeyWaiting() && totalclock > (60*2) )
            {
                if( KB_KeyPressed( sc_F12 ) )
                {
                    KB_ClearKeyDown( sc_F12 );
                    screencapture("rdnk0000.pcx",0);
                }

                if( totalclock < (60*13) )
                {
                    KB_FlushKeyboardQueue();
                    totalclock = (60*13);
                }
                else if( totalclock < (1000000000L))
                   totalclock = (1000000000L);
            }
        }
        else break;
        nextpage();
    }
    if (turdlevel)
        turdlevel = 0;
    if (vixenlevel)
        vixenlevel = 0;
}
#endif

void vglass(long x,long y,short a,short wn,short n)
{
    long z, zincs;
    short sect;

    sect = wall[wn].nextsector;
    if(sect == -1) return;
    zincs = ( sector[sect].floorz-sector[sect].ceilingz ) / n;

    for(z = sector[sect].ceilingz;z < sector[sect].floorz; z += zincs )
        EGS(sect,x,y,z-(TRAND&8191),GLASSPIECES+(z&(TRAND%3)),-32,36,36,a+128-(TRAND&255),16+(TRAND&31),0,-1,5);
}

void lotsofglass(short i,short wallnum,short n)
{
     long j, xv, yv, z, x1, y1;
     short sect, a;

     sect = -1;

     if(wallnum < 0)
     {
        for(j=n-1; j >= 0 ;j--)
        {
            a = SA-256+(TRAND&511)+1024;
            EGS(SECT,SX,SY,SZ,GLASSPIECES+(j%3),-32,36,36,a,32+(TRAND&63),1024-(TRAND&1023),i,5);
        }
        return;
     }

     j = n+1;

     x1 = wall[wallnum].x;
     y1 = wall[wallnum].y;

     xv = wall[wall[wallnum].point2].x-x1;
     yv = wall[wall[wallnum].point2].y-y1;

     x1 -= ksgn(yv);
     y1 += ksgn(xv);

     xv /= j;
     yv /= j;

     for(j=n;j>0;j--)
	 {
		  x1 += xv;
		  y1 += yv;

          updatesector(x1,y1,&sect);
          if(sect >= 0)
          {
              z = sector[sect].floorz-(TRAND&(klabs(sector[sect].ceilingz-sector[sect].floorz)));
              if( z < -(32<<8) || z > (32<<8) )
                  z = SZ-(32<<8)+(TRAND&((64<<8)-1));
              a = SA-1024;
              EGS(SECT,x1,y1,z,GLASSPIECES+(j%3),-32,36,36,a,32+(TRAND&63),-(TRAND&1023),i,5);
          }
	 }
}

void lotsofpopcorn(short i,short wallnum,short n)
{
     long j, xv, yv, z, x1, y1;
     short sect, a;

     sect = -1;

     if(wallnum < 0)
     {
        for(j=n-1; j >= 0 ;j--)
        {
            a = SA-256+(TRAND&511)+1024;
            EGS(SECT,SX,SY,SZ,POPCORN,-32,36,36,a,32+(TRAND&63),1024-(TRAND&1023),i,5);
        }
        return;
     }

     j = n+1;

     x1 = wall[wallnum].x;
     y1 = wall[wallnum].y;

     xv = wall[wall[wallnum].point2].x-x1;
     yv = wall[wall[wallnum].point2].y-y1;

     x1 -= ksgn(yv);
     y1 += ksgn(xv);

     xv /= j;
     yv /= j;

     for(j=n;j>0;j--)
     {
          x1 += xv;
          y1 += yv;

          updatesector(x1,y1,&sect);
          if(sect >= 0)
          {
              z = sector[sect].floorz-(TRAND&(klabs(sector[sect].ceilingz-sector[sect].floorz)));
              if( z < -(32<<8) || z > (32<<8) )
                  z = SZ-(32<<8)+(TRAND&((64<<8)-1));
              a = SA-1024;
              EGS(SECT,x1,y1,z,POPCORN,-32,36,36,a,32+(TRAND&63),-(TRAND&1023),i,5);
          }
     }
}

void spriteglass(short i,short n)
{
    long j, k, a, z;

    for(j=n;j>0;j--)
    {
        a = TRAND&2047;
        z = SZ-((TRAND&16)<<8);
        k = EGS(SECT,SX,SY,z,GLASSPIECES+(j%3),TRAND&15,36,36,a,32+(TRAND&63),-512-(TRAND&2047),i,5);
        sprite[k].pal = sprite[i].pal;
    }
}

void ceilingglass(short i,short sectnum,short n)
{
     long j, xv, yv, z, x1, y1;
     short a,s, startwall,endwall;

     startwall = sector[sectnum].wallptr;
     endwall = startwall+sector[sectnum].wallnum;

     for(s=startwall;s<(endwall-1);s++)
     {
         x1 = wall[s].x;
         y1 = wall[s].y;

         xv = (wall[s+1].x-x1)/(n+1);
         yv = (wall[s+1].y-y1)/(n+1);

         for(j=n;j>0;j--)
         {
              x1 += xv;
              y1 += yv;
              a = TRAND&2047;
              z = sector[sectnum].ceilingz+((TRAND&15)<<8);
              EGS(sectnum,x1,y1,z,GLASSPIECES+(j%3),-32,36,36,a,(TRAND&31),0,i,5);
          }
     }
}



void lotsofcolourglass(short i,short wallnum,short n)
{
     long j, xv, yv, z, x1, y1;
     short sect = -1, a, k;

     if(wallnum < 0)
     {
        for(j=n-1; j >= 0 ;j--)
        {
            a = TRAND&2047;
            k = EGS(SECT,SX,SY,SZ-(TRAND&(63<<8)),GLASSPIECES+(j%3),-32,36,36,a,32+(TRAND&63),1024-(TRAND&2047),i,5);
            sprite[k].pal = TRAND&15;
        }
        return;
     }

     j = n+1;
     x1 = wall[wallnum].x;
     y1 = wall[wallnum].y;

     xv = (wall[wall[wallnum].point2].x-wall[wallnum].x)/j;
     yv = (wall[wall[wallnum].point2].y-wall[wallnum].y)/j;

     for(j=n;j>0;j--)
         {
                  x1 += xv;
                  y1 += yv;

          updatesector(x1,y1,&sect);
          z = sector[sect].floorz-(TRAND&(klabs(sector[sect].ceilingz-sector[sect].floorz)));
          if( z < -(32<<8) || z > (32<<8) )
              z = SZ-(32<<8)+(TRAND&((64<<8)-1));
          a = SA-1024;
          k = EGS(SECT,x1,y1,z,GLASSPIECES+(j%3),-32,36,36,a,32+(TRAND&63),-(TRAND&2047),i,5);
          sprite[k].pal = TRAND&7;
         }
}

void SetupGameButtons( void )
{
   CONTROL_DefineFlag(gamefunc_Move_Forward,false);
   CONTROL_DefineFlag(gamefunc_Move_Backward,false);
   CONTROL_DefineFlag(gamefunc_Turn_Left,false);
   CONTROL_DefineFlag(gamefunc_Turn_Right,false);
   CONTROL_DefineFlag(gamefunc_Strafe,false);
   CONTROL_DefineFlag(gamefunc_Fire,false);
   CONTROL_DefineFlag(gamefunc_Open,false);
   CONTROL_DefineFlag(gamefunc_Run,false);
   CONTROL_DefineFlag(gamefunc_AutoRun,false);
   CONTROL_DefineFlag(gamefunc_Jump,false);
   CONTROL_DefineFlag(gamefunc_Crouch,false);
   CONTROL_DefineFlag(gamefunc_Look_Up,false);
   CONTROL_DefineFlag(gamefunc_Look_Down,false);
   CONTROL_DefineFlag(gamefunc_Look_Left,false);
   CONTROL_DefineFlag(gamefunc_Look_Right,false);
   CONTROL_DefineFlag(gamefunc_Strafe_Left,false);
   CONTROL_DefineFlag(gamefunc_Strafe_Right,false);
   CONTROL_DefineFlag(gamefunc_Aim_Up,false);
   CONTROL_DefineFlag(gamefunc_Aim_Down,false);
   CONTROL_DefineFlag(gamefunc_Weapon_1,false);
   CONTROL_DefineFlag(gamefunc_Weapon_2,false);
   CONTROL_DefineFlag(gamefunc_Weapon_3,false);
   CONTROL_DefineFlag(gamefunc_Weapon_4,false);
   CONTROL_DefineFlag(gamefunc_Weapon_5,false);
   CONTROL_DefineFlag(gamefunc_Weapon_6,false);
   CONTROL_DefineFlag(gamefunc_Weapon_7,false);
   CONTROL_DefineFlag(gamefunc_Weapon_8,false);
   CONTROL_DefineFlag(gamefunc_Weapon_9,false);
   CONTROL_DefineFlag(gamefunc_Weapon_10,false);
   CONTROL_DefineFlag(gamefunc_Inventory,false);
   CONTROL_DefineFlag(gamefunc_Inventory_Left,false);
   CONTROL_DefineFlag(gamefunc_Inventory_Right,false);
   CONTROL_DefineFlag(gamefunc_Holo_Duke,false);
   CONTROL_DefineFlag(gamefunc_Jetpack,false);
   CONTROL_DefineFlag(gamefunc_NightVision,false);
   CONTROL_DefineFlag(gamefunc_MedKit,false);
   CONTROL_DefineFlag(gamefunc_TurnAround,false);
   CONTROL_DefineFlag(gamefunc_SendMessage,false);
   CONTROL_DefineFlag(gamefunc_Map,false);
   CONTROL_DefineFlag(gamefunc_Shrink_Screen,false);
   CONTROL_DefineFlag(gamefunc_Enlarge_Screen,false);
   CONTROL_DefineFlag(gamefunc_Center_View,false);
   CONTROL_DefineFlag(gamefunc_Holster_Weapon,false);
   CONTROL_DefineFlag(gamefunc_Show_Opponents_Weapon,false);
   CONTROL_DefineFlag(gamefunc_Map_Follow_Mode,false);
   CONTROL_DefineFlag(gamefunc_See_Coop_View,false);
   CONTROL_DefineFlag(gamefunc_Mouse_Aiming,false);
   CONTROL_DefineFlag(gamefunc_Toggle_Crosshair,false);
   CONTROL_DefineFlag(gamefunc_Steroids,false);
   CONTROL_DefineFlag(gamefunc_Quick_Kick,false);
   CONTROL_DefineFlag(gamefunc_Next_Weapon,false);
   CONTROL_DefineFlag(gamefunc_Previous_Weapon,false);
}

#ifdef RRRA

void dobonus(char bonusonly)
{
    short t, r, tinc,gfx_offset,bg_tile;
    long i, y,xfragtotal,yfragtotal;
    short bonuscnt;

    bonuscnt = 0;

    for(t=0;t<64;t++) palto(0,0,0,t);
    setview(0,0,xdim-1,ydim-1);
    clearview(0L);
    nextpage();
    flushperms();

    FX_StopAllSounds();
    clearsoundlocks();
    FX_SetReverb(0L);

    if(bonusonly) goto FRAGBONUS;

    FRAGBONUS:

    ps[myconnectindex].palette = palette;
    KB_FlushKeyboardQueue();
    totalclock = 0; tinc = 0;
    bonuscnt = 0;

    MUSIC_StopSong();
    FX_StopAllSounds();
    clearsoundlocks();

    if(playerswhenstarted > 1 && ud.coop != 1 )
    {
        if(!(MusicToggle == 0 || MusicDevice == NumSoundCards))
            sound(249);

        rotatesprite(0,0,65536L,0,MENUSCREEN,16,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
        rotatesprite(160<<16,24<<16,23592L,0,INGAMEDUKETHREEDEE,0,0,2+8,0,0,xdim-1,ydim-1);
        gametext(160,58,"MULTIPLAYER TOTALS",0);
        gametext(160,58+10,level_names[(ud.volume_number*7)+ud.last_level-1],0);

        gametext(160,175,"PRESS ANY KEY TO CONTINUE",0);


        t = 0;
        minitext(23,80,"   NAME                                           KILLS",8,2+8+16+128);
        for(i=0;i<playerswhenstarted;i++)
        {
            sprintf(tempbuf,"%-4ld",i+1);
            minitext(92+(i*23),80,tempbuf,0,2+8+16+128);
        }

        for(i=0;i<playerswhenstarted;i++)
        {
            xfragtotal = 0;
            sprintf(tempbuf,"%ld",i+1);

            minitext(30,90+t,tempbuf,0,2+8+16+128);
            minitext(38,90+t,ud.user_name[i],ps[i].palookup,2+8+16+128);

            for(y=0;y<playerswhenstarted;y++)
            {
                if(i == y)
                {
                    sprintf(tempbuf,"%-4ld",ps[y].fraggedself);
                    minitext(92+(y*23),90+t,tempbuf,0,2+8+16+128);
                    xfragtotal -= ps[y].fraggedself;
                }
                else
                {
                    sprintf(tempbuf,"%-4ld",frags[i][y]);
                    minitext(92+(y*23),90+t,tempbuf,0,2+8+16+128);
                    xfragtotal += frags[i][y];
                }

                if(myconnectindex == connecthead)
                {
                    sprintf(tempbuf,"stats %ld killed %ld %ld\n",i+1,y+1,frags[i][y]);
                    sendscore(tempbuf);
                }
            }

            sprintf(tempbuf,"%-4ld",xfragtotal);
            minitext(101+(8*23),90+t,tempbuf,0,2+8+16+128);

            t += 7;
        }

        for(y=0;y<playerswhenstarted;y++)
        {
            yfragtotal = 0;
            for(i=0;i<playerswhenstarted;i++)
            {
                if(i == y)
                    yfragtotal += ps[i].fraggedself;
                yfragtotal += frags[i][y];
            }
            sprintf(tempbuf,"%-4ld",yfragtotal);
            minitext(92+(y*23),96+(8*7),tempbuf,0,2+8+16+128);
        }

        minitext(45,96+(8*7),"DEATHS",0,2+8+16+128);
        nextpage();

        for(t=0;t<64;t++)
            palto(0,0,0,63-t);

        KB_FlushKeyboardQueue();
        while(KB_KeyWaiting()==0) getpackets();

        if( KB_KeyPressed( sc_F12 ) )
        {
            KB_ClearKeyDown( sc_F12 );
            screencapture("rdnk0000.pcx",0);
        }

        if(bonusonly || ud.multimode > 1) return;

        for(t=0;t<64;t++) palto(0,0,0,t);
    }

    if(bonusonly || ud.multimode > 1) return;

    if(!(MusicToggle == 0 || MusicDevice == NumSoundCards))
        sound(249);

    
    gfx_offset = (ud.volume_number&1)*5;
    bg_tile = RRTILE403;
    if (ud.volume_number == 0)
        bg_tile = ud.level_number+RRTILE403-1;
    else
        bg_tile = ud.level_number+RRTILE409-1;

    if (lastlevel || vixenlevel)
        bg_tile = RRTILE409+7;

    if (boardfilename[0])
    {
        rotatesprite(0,0,65536L,0,403,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
        endlvlmenutext(80,16,0,0,boardfilename);
    }
    else
    {
        rotatesprite(0,0,65536L,0,bg_tile,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
        if ((lastlevel && ud.volume_number == 2) || vixenlevel)
            endlvlmenutext(80,16,0,0,"CLOSE ENCOUNTERS");
        else if (turdlevel)
            endlvlmenutext(80,16,0,0,"SMELTING PLANT");
        else
            endlvlmenutext(80,16,0,0,level_names[(ud.volume_number*7)+ud.last_level-1]);
    }

    endlvlmenutext(15,192,0,0,"PRESS ANY KEY TO CONTINUE");
    nextpage();
    KB_FlushKeyboardQueue();
    for(t=0;t<64;t++) palto(0,0,0,63-t);
    bonuscnt = 0;
    totalclock = 0; tinc = 0;

    while( 1 )
    {
        if(ps[myconnectindex].gm&MODE_EOL)
        {
            if (boardfilename[0])
                rotatesprite(0,0,65536L,0,403,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
            else
                rotatesprite(0,0,65536L,0,bg_tile,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);

            if( totalclock > (1000000000L) && totalclock < (1000000320L) )
            {
                switch( ((unsigned long)totalclock>>4)%15 )
                {
                    case 0:
                        if(bonuscnt == 6)
                        {
                            bonuscnt++;
                            sound(425);
                            switch(rand()&3)
                            {
                                case 0:
                                    sound(195);
                                    break;
                                case 1:
                                    sound(196);
                                    break;
                                case 2:
                                    sound(197);
                                    break;
                                case 3:
                                    sound(199);
                                    break;
                            }
                        }
                    case 1:
                    case 4:
                    case 5:
                    case 2:
                    case 3:
                       break;
                }
            }
            else if( totalclock > (10240+120L) ) break;
            else
            {
                switch( (totalclock>>5)&3 )
                {
                    case 1:
                    case 3:
                        break;
                    case 2:
                        break;
                }
            }

            if (boardfilename[0])
            {
                rotatesprite(0,0,65536L,0,403,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
                endlvlmenutext(80,16,0,0,boardfilename);
            }
            else
            {
                rotatesprite(0,0,65536L,0,bg_tile,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
                if ((lastlevel && ud.volume_number == 2) || vixenlevel)
                    endlvlmenutext(80,16,0,0,"CLOSE ENCOUNTERS");
                else if (turdlevel)
                    endlvlmenutext(80,16,0,0,"SMELTING PLANT");
                else
                    endlvlmenutext(80,16,0,0,level_names[(ud.volume_number*7)+ud.last_level-1]);
            }

            endlvlmenutext(15,192,0,0,"PRESS ANY KEY TO CONTINUE");

            if( totalclock > (60*3) )
            {
                endlvlmenutext(30,48,0,0,"Yer Time:");
                endlvlmenutext(30,64,0,0,"Par time:");
                endlvlmenutext(30,80,0,0,"Xatrix Time:");
                if(bonuscnt == 0)
                    bonuscnt++;

                if( totalclock > (60*4) )
                {
                    if(bonuscnt == 1)
                    {
                        bonuscnt++;
                        sound(404);
                    }
                    sprintf(tempbuf,"%02ld : %02ld",
                        (ps[myconnectindex].player_par/(26*60))%60,
                        (ps[myconnectindex].player_par/26)%60);
                    endlvlmenutext(191,48,0,0,tempbuf);

                    if(!boardfilename[0])
                    {
                        sprintf(tempbuf,"%02ld : %02ld",
                            (partime[ud.volume_number*7+ud.last_level-1]/(26*60))%60,
                            (partime[ud.volume_number*7+ud.last_level-1]/26)%60);
                        endlvlmenutext(191,64,0,0,tempbuf);

                        sprintf(tempbuf,"%02ld : %02ld",
                            (designertime[ud.volume_number*7+ud.last_level-1]/(26*60))%60,
                            (designertime[ud.volume_number*7+ud.last_level-1]/26)%60);
                        endlvlmenutext(191,80,0,0,tempbuf);
                    }

                }
            }
            if( totalclock > (60*6) )
            {
                endlvlmenutext(30,112,0,0,"Varmints Killed:");
                endlvlmenutext(30,128,0,0,"Varmints Left:");

                if(bonuscnt == 2)
                    bonuscnt++;

                if( totalclock > (60*7) )
                {
                    if(bonuscnt == 3)
                    {
                        bonuscnt++;
                        sound(422);
                    }
                    sprintf(tempbuf,"%-3ld",ps[myconnectindex].actors_killed);
                    endlvlmenutext(231,112,0,0,tempbuf);
                    if(ud.player_skill > 3 )
                    {
                        sprintf(tempbuf,"N/A");
                        endlvlmenutext(231,128,0,0,tempbuf);
                    }
                    else
                    {
                        if( (ps[myconnectindex].max_actors_killed-ps[myconnectindex].actors_killed) < 0 )
                            sprintf(tempbuf,"%-3ld",0);
                        else sprintf(tempbuf,"%-3ld",ps[myconnectindex].max_actors_killed-ps[myconnectindex].actors_killed);
                        endlvlmenutext(231,128,0,0,tempbuf);
                    }
                }
            }
            if( totalclock > (60*9) )
            {
                endlvlmenutext(30,144,0,0,"Secrets Found:");
                endlvlmenutext(30,160,0,0,"Secrets Missed:");
                if(bonuscnt == 4) bonuscnt++;

                if( totalclock > (60*10) )
                {
                    if(bonuscnt == 5)
                    {
                        bonuscnt++;
                        sound(404);
                    }
                    sprintf(tempbuf,"%-3ld",ps[myconnectindex].secret_rooms);
                    endlvlmenutext(231,144,0,0,tempbuf);
                    if( ps[myconnectindex].secret_rooms > 0 )
                        sprintf(tempbuf,"%-3ld%",(100*ps[myconnectindex].secret_rooms/ps[myconnectindex].max_secret_rooms));
                    sprintf(tempbuf,"%-3ld",ps[myconnectindex].max_secret_rooms-ps[myconnectindex].secret_rooms);
                    endlvlmenutext(231,160,0,0,tempbuf);
                }
            }

            if(totalclock > 10240 && totalclock < 10240+10240)
                totalclock = 1024;

            if( KB_KeyWaiting() && totalclock > (60*2) )
            {
                if( KB_KeyPressed( sc_F12 ) )
                {
                    KB_ClearKeyDown( sc_F12 );
                    screencapture("rdnk0000.pcx",0);
                }

                if( totalclock < (60*13) )
                {
                    KB_FlushKeyboardQueue();
                    totalclock = (60*13);
                }
                else if( totalclock < (1000000000L))
                   totalclock = (1000000000L);
            }
        }
        else break;
        nextpage();
    }
    if (turdlevel)
        turdlevel = 0;
    if (vixenlevel)
        vixenlevel = 0;
}
#endif

/*
===================
=
= GetTime
=
===================
*/

long GetTime(void)
   {
   return totalclock;
   }


/*
===================
=
= CenterCenter
=
===================
*/

void CenterCenter(void)
   {
   printf("Center the joystick and press a button\n");
   }

/*
===================
=
= UpperLeft
=
===================
*/

void UpperLeft(void)
   {
   printf("Move joystick to upper-left corner and press a button\n");
   }

/*
===================
=
= LowerRight
=
===================
*/

void LowerRight(void)
   {
   printf("Move joystick to lower-right corner and press a button\n");
   }

/*
===================
=
= CenterThrottle
=
===================
*/

void CenterThrottle(void)
   {
   printf("Center the throttle control and press a button\n");
   }

/*
===================
=
= CenterRudder
=
===================
*/

void CenterRudder(void)
{
   printf("Center the rudder control and press a button\n");
}