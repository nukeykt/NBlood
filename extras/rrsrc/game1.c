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

#define TIMERUPDATESIZ 32

long cameradist = 0, cameraclock = 0;
char eightytwofifty = 0;
char playerswhenstarted;

int32 CommandSoundToggleOff = 0;
int32 CommandMusicToggleOff = 0;

char confilename[128] = {"GAME.CON"},boardfilename[128] = {0};
char waterpal[768], slimepal[768], titlepal[768], drealms[768], endingpal[768];
char firstdemofile[80] = { '\0' };

int recfilep;
char debug_on = 0,actor_tog = 0,*rtsptr;

extern char syncstate;
extern int32 numlumps;

FILE *frecfilep = (FILE *)NULL;
void pitch_test( void );

char restorepalette, screencapt;
int sendmessagecommand = -1;

task *TimerPtr = NULL;

extern long lastvisinc;
int totalreccnt;

void timerhandler()
{
    totalclock++;
}

void inittimer()
{
    TimerPtr = TS_ScheduleTask( timerhandler,TICRATE, 1, NULL );
    TS_Dispatch();
}

void uninittimer(void)
{
   if (TimerPtr)
      TS_Terminate( TimerPtr );
   TimerPtr = NULL;
   TS_Shutdown();
}

int gametext(int x,int y,char *t,char s)
{
    short ac,newx;
    char centre, *oldt;

    centre = ( x == (320>>1) );
    newx = 0;
    oldt = t;

    if(centre)
    {
        while(*t)
        {
            if(*t == 32) {newx+=5;t++;continue;}
            else ac = *t - '!' + STARTALPHANUM;

            if( ac < STARTALPHANUM || ac > ENDALPHANUM ) break;

            if(*t >= '0' && *t <= '9')
                newx += 8;
            else newx += tilesizx[ac] / 2;
            t++;
        }

        t = oldt;
        x = (320>>1)-(newx>>1);
    }

    while(*t)
    {
        if(*t == 32) {x+=5;t++;continue;}
        else ac = *t - '!' + STARTALPHANUM;

        if( ac < STARTALPHANUM || ac > ENDALPHANUM )
            break;

        rotatesprite(x<<16,y<<16,32768L,0,ac,s,0,2+8+16,0,0,xdim-1,ydim-1);

        if(*t >= '0' && *t <= '9')
            x += 8;
        else x += tilesizx[ac] / 2;

        t++;
    }

    return (x);
}

int gametext2(int x,int y,char *t,char s)
{
    short ac,newx;
    char centre, *oldt;

    centre = ( x == (320>>1) );
    newx = 0;
    oldt = t;

    if(centre)
    {
        while(*t)
        {
            if(*t == 32) {newx+=5;t++;continue;}
            else ac = *t - '!' + STARTALPHANUM;

            if( ac < STARTALPHANUM || ac > ENDALPHANUM ) break;

            if(*t >= '0' && *t <= '9')
                newx += 8;
            else newx += tilesizx[ac] / 2;
            t++;
        }

        t = oldt;
        x = (320>>1)-(newx>>1);
    }

    while(*t)
    {
        if(*t == 32) {x+=5;t++;continue;}
        else if(*t == '\'') ac = '`' - '!' + STARTALPHANUM;
        else ac = *t - '!' + STARTALPHANUM;

        if( ac < STARTALPHANUM || ac > ENDALPHANUM )
            break;

        rotatesprite(x<<16,y<<16,32768L,0,ac,s,s,2+8+16,0,0,xdim-1,ydim-1);

        if(*t >= '0' && *t <= '9')
            x += 8;
        else x += tilesizx[ac] / 2;

        t++;
    }

    return (x);
}

int gametextpal(int x,int y,char *t,char s,char p)
{
    short ac,newx;
    char centre, *oldt;

    centre = ( x == (320>>1) );
    newx = 0;
    oldt = t;

    if(centre)
    {
        while(*t)
        {
            if(*t == 32) {newx+=5;t++;continue;}
            else ac = *t - '!' + STARTALPHANUM;

            if( ac < STARTALPHANUM || ac > ENDALPHANUM ) break;

            if(*t >= '0' && *t <= '9')
                newx += 8;
            else newx += tilesizx[ac];
            t++;
        }

        t = oldt;
        x = (320>>1)-(newx>>1);
    }

    while(*t)
    {
        if(*t == 32) {x+=5;t++;continue;}
        else ac = *t - '!' + STARTALPHANUM;

        if( ac < STARTALPHANUM || ac > ENDALPHANUM )
            break;

        rotatesprite(x<<16,y<<16,65536L,0,ac,s,p,2+8+16,0,0,xdim-1,ydim-1);
        if(*t >= '0' && *t <= '9')
            x += 8;
        else x += tilesizx[ac];

        t++;
    }

    return (x);
}

int gametextpart(int x,int y,char *t,char s,short p)
{
    short ac,newx, cnt;
    char centre, *oldt;

    centre = ( x == (320>>1) );
    newx = 0;
    oldt = t;
    cnt = 0;

    if(centre)
    {
        while(*t)
        {
            if(cnt == p) break;

            if(*t == 32) {newx+=5;t++;continue;}
            else ac = *t - '!' + STARTALPHANUM;

            if( ac < STARTALPHANUM || ac > ENDALPHANUM ) break;

            newx += tilesizx[ac];
            t++;
            cnt++;

        }

        t = oldt;
        x = (320>>1)-(newx>>1);
    }

    cnt = 0;
    while(*t)
    {
        if(*t == 32) {x+=5;t++;continue;}
        else ac = *t - '!' + STARTALPHANUM;

        if( ac < STARTALPHANUM || ac > ENDALPHANUM ) break;

        if(cnt == p)
        {
            rotatesprite(x<<16,y<<16,65536L,0,ac,s,1,2+8+16,0,0,xdim-1,ydim-1);
            break;
        }
        else
            rotatesprite(x<<16,y<<16,65536L,0,ac,s,0,2+8+16,0,0,xdim-1,ydim-1);

        x += tilesizx[ac];

        t++;
        cnt++;
    }

    return (x);
}

int minitext(int x,int y,char *t,char p,char sb)
{
    short ac;

    while(*t)
    {
        *t = toupper(*t);
        if(*t == 32) {x+=5;t++;continue;}
        else ac = *t - '!' + MINIFONT;

        rotatesprite(x<<16,y<<16,32768L,0,ac,0,p,sb,0,0,xdim-1,ydim-1);
        x += 4; // tilesizx[ac]+1;

        t++;
    }
    return (x);
}

int minitextshade(int x,int y,char *t,char s,char p,char sb)
{
    short ac;

    while(*t)
    {
        *t = toupper(*t);
        if(*t == 32) {x+=5;t++;continue;}
        else ac = *t - '!' + MINIFONT;

        rotatesprite(x<<16,y<<16,32768L,0,ac,s,p,sb,0,0,xdim-1,ydim-1);
        x += 4; // tilesizx[ac]+1;

        t++;
    }
    return (x);
}

void gamenumber(long x,long y,long n,char s)
{
    char b[10];
    ltoa(n,b,10);
    gametext(x,y,b,s);
}


char recbuf[80];
void allowtimetocorrecterrorswhenquitting(void)
{
     long i, j, oldtotalclock;

     ready2send = 0;

     for(j=0;j<8;j++)
     {
          oldtotalclock = totalclock;

          while (totalclock < oldtotalclock+TICSPERFRAME)
              getpackets();

          if(KB_KeyPressed(sc_Escape)) return;

          packbuf[0] = 127;
          for(i=connecthead;i>=0;i=connectpoint2[i])
                if (i != myconnectindex)
                     sendpacket(i,packbuf,1);
     }
}


void getpackets(void)
{
    long i, j, k, l;
    FILE *fp;
    short other, packbufleng;
    input *osyn, *nsyn;

    if (numplayers < 2) return;
    while ((packbufleng = getpacket(&other,packbuf)) > 0)
    {
        switch(packbuf[0])
        {
            case 0:  //[0] (receive master sync buffer)
                j = 1;

                if ((movefifoend[other]&(TIMERUPDATESIZ-1)) == 0)
                    for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
                    {
                        if (playerquitflag[i] == 0) continue;
                        if (i == myconnectindex)
                            otherminlag = (long)((signed char)packbuf[j]);
                        j++;
                    }

                osyn = (input *)&inputfifo[(movefifoend[connecthead]-1)&(MOVEFIFOSIZ-1)][0];
                nsyn = (input *)&inputfifo[(movefifoend[connecthead])&(MOVEFIFOSIZ-1)][0];

                k = j;
                for(i=connecthead;i>=0;i=connectpoint2[i])
                    j += playerquitflag[i];
                for(i=connecthead;i>=0;i=connectpoint2[i])
                {
                    if (playerquitflag[i] == 0) continue;

                    l = packbuf[k++];
                    if (i == myconnectindex)
                        { j += ((l&1)<<1)+(l&2)+((l&4)>>2)+((l&8)>>3)+((l&16)>>4)+((l&32)>>5)+((l&64)>>6)+((l&128)>>7); continue; }

                    copybufbyte(&osyn[i],&nsyn[i],sizeof(input));
                    if (l&1)   nsyn[i].fvel = packbuf[j]+((short)packbuf[j+1]<<8), j += 2;
                    if (l&2)   nsyn[i].svel = packbuf[j]+((short)packbuf[j+1]<<8), j += 2;
                    if (l&4)   nsyn[i].avel = (signed char)packbuf[j++];
                    if (l&8)   nsyn[i].bits = ((nsyn[i].bits&0xffffff00)|((long)packbuf[j++]));
                    if (l&16)  nsyn[i].bits = ((nsyn[i].bits&0xffff00ff)|((long)packbuf[j++])<<8);
                    if (l&32)  nsyn[i].bits = ((nsyn[i].bits&0xff00ffff)|((long)packbuf[j++])<<16);
                    if (l&64)  nsyn[i].bits = ((nsyn[i].bits&0x00ffffff)|((long)packbuf[j++])<<24);
                    if (l&128) nsyn[i].horz = (signed char)packbuf[j++];

                    if (nsyn[i].bits&(1<<26)) playerquitflag[i] = 0;
                    movefifoend[i]++;
                }

                while (j != packbufleng)
                {
                    for(i=connecthead;i>=0;i=connectpoint2[i])
                        if(i != myconnectindex)
                    {
                        syncval[i][syncvalhead[i]&(MOVEFIFOSIZ-1)] = packbuf[j];
                        syncvalhead[i]++;
                    }
                    j++;
                }

                for(i=connecthead;i>=0;i=connectpoint2[i])
                    if (i != myconnectindex)
                        for(j=1;j<movesperpacket;j++)
                        {
                            copybufbyte(&nsyn[i],&inputfifo[movefifoend[i]&(MOVEFIFOSIZ-1)][i],sizeof(input));
                            movefifoend[i]++;
                        }

                 movefifosendplc += movesperpacket;

                break;
            case 1:  //[1] (receive slave sync buffer)
                j = 2; k = packbuf[1];

                osyn = (input *)&inputfifo[(movefifoend[other]-1)&(MOVEFIFOSIZ-1)][0];
                nsyn = (input *)&inputfifo[(movefifoend[other])&(MOVEFIFOSIZ-1)][0];

                copybufbyte(&osyn[other],&nsyn[other],sizeof(input));
                if (k&1)   nsyn[other].fvel = packbuf[j]+((short)packbuf[j+1]<<8), j += 2;
                if (k&2)   nsyn[other].svel = packbuf[j]+((short)packbuf[j+1]<<8), j += 2;
                if (k&4)   nsyn[other].avel = (signed char)packbuf[j++];
                if (k&8)   nsyn[other].bits = ((nsyn[other].bits&0xffffff00)|((long)packbuf[j++]));
                if (k&16)  nsyn[other].bits = ((nsyn[other].bits&0xffff00ff)|((long)packbuf[j++])<<8);
                if (k&32)  nsyn[other].bits = ((nsyn[other].bits&0xff00ffff)|((long)packbuf[j++])<<16);
                if (k&64)  nsyn[other].bits = ((nsyn[other].bits&0x00ffffff)|((long)packbuf[j++])<<24);
                if (k&128) nsyn[other].horz = (signed char)packbuf[j++];
                movefifoend[other]++;

                while (j != packbufleng)
                {
                    syncval[other][syncvalhead[other]&(MOVEFIFOSIZ-1)] = packbuf[j++];
                    syncvalhead[other]++;
                }

                for(i=1;i<movesperpacket;i++)
                {
                    copybufbyte(&nsyn[other],&inputfifo[movefifoend[other]&(MOVEFIFOSIZ-1)][other],sizeof(input));
                    movefifoend[other]++;
                }

                break;

            case 4:
                strcpy(recbuf,packbuf+1);

                recbuf[packbufleng-1] = 0;
                user_quote = recbuf;
                user_quote_time = 160;

                sound(EXITMENUSOUND);

                pus = NUMPAGES;
                pub = NUMPAGES;

                break;

            case 5:
                ud.m_level_number = ud.level_number = packbuf[1];
                ud.m_volume_number = ud.volume_number = packbuf[2];
                ud.m_player_skill = ud.player_skill = packbuf[3];
                ud.m_monsters_off = ud.monsters_off = packbuf[4];
                ud.m_respawn_monsters = ud.respawn_monsters = packbuf[5];
                ud.m_respawn_items = ud.respawn_items = packbuf[6];
                ud.m_respawn_inventory = ud.respawn_inventory = packbuf[7];
                ud.m_coop = packbuf[8];
                ud.m_marker = ud.marker = packbuf[9];
                ud.m_ffire = ud.ffire = packbuf[10];

                for(i=connecthead;i>=0;i=connectpoint2[i])
                {
                    resetweapons(i);
                    resetinventory(i);
                }

                newgame(ud.volume_number,ud.level_number,ud.player_skill);
                ud.coop = ud.m_coop;

                enterlevel(MODE_GAME);

                break;
            case 6:
                if (packbuf[1] != BYTEVERSION)
                    gameexit("\nYou cannot play Redneck with different versions.");
                for (i=2;packbuf[i];i++)
                    ud.user_name[other][i-2] = packbuf[i];
                ud.user_name[other][i-2] = 0;
                break;
            case 9:
                for (i=1;i<packbufleng;i++)
                    ud.wchoice[other][i-1] = packbuf[i];
                break;
            case 7:
                if (ud.lockout == 1)
                    break;

                if ((RTS_NumSounds() > 0 && SoundToggle == 0) || ud.lockout == 1 || FXDevice == NumSoundCards)
                    break;
                rtsptr = (char *)RTS_GetSound(packbuf[1]-1);
                if (*rtsptr == 'C')
                    FX_PlayVOC3D(rtsptr,0,0,0,255,-packbuf[1]);
                else
                    FX_PlayWAV3D(rtsptr,0,0,0,255,-packbuf[1]);
                rtsplaying = 7;
                break;
            case 8:
                ud.m_level_number = ud.level_number = packbuf[1];
                ud.m_volume_number = ud.volume_number = packbuf[2];
                ud.m_player_skill = ud.player_skill = packbuf[3];
                ud.m_monsters_off = ud.monsters_off = packbuf[4];
                ud.m_respawn_monsters = ud.respawn_monsters = packbuf[5];
                ud.m_respawn_items = ud.respawn_items = packbuf[6];
                ud.m_respawn_inventory = ud.respawn_inventory = packbuf[7];
                ud.m_coop = ud.coop = packbuf[8];
                ud.m_marker = ud.marker = packbuf[9];
                ud.m_ffire = ud.ffire = packbuf[10];

                copybufbyte(packbuf+10,boardfilename,packbufleng-11);
                boardfilename[packbufleng-11] = 0;

                for(i=connecthead;i>=0;i=connectpoint2[i])
                {
                    resetweapons(i);
                    resetinventory(i);
                }

                newgame(ud.volume_number,ud.level_number,ud.player_skill);
                enterlevel(MODE_GAME);
                break;

            case 16:
                movefifoend[other] = movefifoplc = movefifosendplc = fakemovefifoplc = 0;
                syncvalhead[other] = syncvaltottail = 0L;
            case 17:
                j = 1;

                if ((movefifoend[other]&(TIMERUPDATESIZ-1)) == 0)
                    if (other == connecthead)
                        for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
                        {
                            if (i == myconnectindex)
                                otherminlag = (long)((signed char)packbuf[j]);
                            j++;
                        }

                osyn = (input *)&inputfifo[(movefifoend[other]-1)&(MOVEFIFOSIZ-1)][0];
                nsyn = (input *)&inputfifo[(movefifoend[other])&(MOVEFIFOSIZ-1)][0];

                copybufbyte(&osyn[other],&nsyn[other],sizeof(input));
                k = packbuf[j++];
                if (k&1)   nsyn[other].fvel = packbuf[j]+((short)packbuf[j+1]<<8), j += 2;
                if (k&2)   nsyn[other].svel = packbuf[j]+((short)packbuf[j+1]<<8), j += 2;
                if (k&4)   nsyn[other].avel = (signed char)packbuf[j++];
                if (k&8)   nsyn[other].bits = ((nsyn[other].bits&0xffffff00)|((long)packbuf[j++]));
                if (k&16)  nsyn[other].bits = ((nsyn[other].bits&0xffff00ff)|((long)packbuf[j++])<<8);
                if (k&32)  nsyn[other].bits = ((nsyn[other].bits&0xff00ffff)|((long)packbuf[j++])<<16);
                if (k&64)  nsyn[other].bits = ((nsyn[other].bits&0x00ffffff)|((long)packbuf[j++])<<24);
                if (k&128) nsyn[other].horz = (signed char)packbuf[j++];
                movefifoend[other]++;

                for(i=1;i<movesperpacket;i++)
                {
                    copybufbyte(&nsyn[other],&inputfifo[movefifoend[other]&(MOVEFIFOSIZ-1)][other],sizeof(input));
                    movefifoend[other]++;
                }

                if (j > packbufleng)
                    printf("DAGNABIT GAME PACKET!!! (%ld too many bytes)\n",j-packbufleng);

                while (j != packbufleng)
                {
                    syncval[other][syncvalhead[other]&(MOVEFIFOSIZ-1)] = packbuf[j++];
                    syncvalhead[other]++;
                }

                break;
            case 127:
                break;

            case 250:
                playerreadyflag[other]++;
                break;
            case 255:
                gameexit(" ");
                break;
        }
    }
}

void faketimerhandler()
{
    long i, j, k, l;
//    short who;
    input *osyn, *nsyn;

    if ((totalclock < ototalclock+TICSPERFRAME) || (ready2send == 0)) return;
    ototalclock += TICSPERFRAME;

    getpackets(); if (getoutputcirclesize() >= 16) return;

    for(i=connecthead;i>=0;i=connectpoint2[i])
        if (i != myconnectindex)
            if (movefifoend[i] < movefifoend[myconnectindex]-200) return;

#ifdef RRRA
    if (ps[myconnectindex].OnMotorcycle)
        getinputmotorcycle(myconnectindex);
    else if (ps[myconnectindex].OnBoat)
        getinputboat(myconnectindex);
    else
#endif
     getinput(myconnectindex);

     avgfvel += loc.fvel;
     avgsvel += loc.svel;
     avgavel += loc.avel;
     avghorz += loc.horz;
     avgbits |= loc.bits;
     if (movefifoend[myconnectindex]&(movesperpacket-1))
     {
          copybufbyte(&inputfifo[(movefifoend[myconnectindex]-1)&(MOVEFIFOSIZ-1)][myconnectindex],
                          &inputfifo[movefifoend[myconnectindex]&(MOVEFIFOSIZ-1)][myconnectindex],sizeof(input));
          movefifoend[myconnectindex]++;
          return;
     }
     nsyn = &inputfifo[movefifoend[myconnectindex]&(MOVEFIFOSIZ-1)][myconnectindex];
     nsyn[0].fvel = avgfvel/movesperpacket;
     nsyn[0].svel = avgsvel/movesperpacket;
     nsyn[0].avel = avgavel/movesperpacket;
     nsyn[0].horz = avghorz/movesperpacket;
     nsyn[0].bits = avgbits;
     avgfvel = avgsvel = avgavel = avghorz = avgbits = 0;
     movefifoend[myconnectindex]++;

     if (numplayers < 2)
     {
          if (ud.multimode > 1) for(i=connecthead;i>=0;i=connectpoint2[i])
              if(i != myconnectindex)
              {
                  //clearbufbyte(&inputfifo[movefifoend[i]&(MOVEFIFOSIZ-1)][i],sizeof(input),0L);
                  if(ud.playerai)
                      computergetinput(i,&inputfifo[movefifoend[i]&(MOVEFIFOSIZ-1)][i]);
                  movefifoend[i]++;
              }
          return;
     }

    for(i=connecthead;i>=0;i=connectpoint2[i])
        if (i != myconnectindex)
        {
            k = (movefifoend[myconnectindex]-1)-movefifoend[i];
            myminlag[i] = min(myminlag[i],k);
            mymaxlag = max(mymaxlag,k);
        }

    if (((movefifoend[myconnectindex]-1)&(TIMERUPDATESIZ-1)) == 0)
    {
        i = mymaxlag-bufferjitter; mymaxlag = 0;
        if (i > 0) bufferjitter += ((3+i)>>2);
        else if (i < 0) bufferjitter -= ((1-i)>>2);
    }

    if (networkmode == 1)
    {
        packbuf[0] = 17;
        if ((movefifoend[myconnectindex]-1) == 0) packbuf[0] = 16;
        j = 1;

            //Fix timers and buffer/jitter value
        if (((movefifoend[myconnectindex]-1)&(TIMERUPDATESIZ-1)) == 0)
        {
            if (myconnectindex != connecthead)
            {
                i = myminlag[connecthead]-otherminlag;
                if (klabs(i) > 8) i >>= 1;
                else if (klabs(i) > 2) i = ksgn(i);
                else i = 0;

                totalclock -= TICSPERFRAME*i;
                myminlag[connecthead] -= i; otherminlag += i;
            }

            if (myconnectindex == connecthead)
                for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
                    packbuf[j++] = min(max(myminlag[i],-128),127);

            for(i=connecthead;i>=0;i=connectpoint2[i])
                myminlag[i] = 0x7fffffff;
        }

        osyn = (input *)&inputfifo[(movefifoend[myconnectindex]-2)&(MOVEFIFOSIZ-1)][myconnectindex];
        nsyn = (input *)&inputfifo[(movefifoend[myconnectindex]-1)&(MOVEFIFOSIZ-1)][myconnectindex];

        k = j;
        packbuf[j++] = 0;

        if (nsyn[0].fvel != osyn[0].fvel)
        {
            packbuf[j++] = (char)nsyn[0].fvel;
            packbuf[j++] = (char)(nsyn[0].fvel>>8);
            packbuf[k] |= 1;
        }
        if (nsyn[0].svel != osyn[0].svel)
        {
            packbuf[j++] = (char)nsyn[0].svel;
            packbuf[j++] = (char)(nsyn[0].svel>>8);
            packbuf[k] |= 2;
        }
        if (nsyn[0].avel != osyn[0].avel)
        {
            packbuf[j++] = (signed char)nsyn[0].avel;
            packbuf[k] |= 4;
        }
        if ((nsyn[0].bits^osyn[0].bits)&0x000000ff) packbuf[j++] = (nsyn[0].bits&255), packbuf[k] |= 8;
        if ((nsyn[0].bits^osyn[0].bits)&0x0000ff00) packbuf[j++] = ((nsyn[0].bits>>8)&255), packbuf[k] |= 16;
        if ((nsyn[0].bits^osyn[0].bits)&0x00ff0000) packbuf[j++] = ((nsyn[0].bits>>16)&255), packbuf[k] |= 32;
        if ((nsyn[0].bits^osyn[0].bits)&0xff000000) packbuf[j++] = ((nsyn[0].bits>>24)&255), packbuf[k] |= 64;
        if (nsyn[0].horz != osyn[0].horz)
        {
            packbuf[j++] = (char)nsyn[0].horz;
            packbuf[k] |= 128;
        }

        while (syncvalhead[myconnectindex] != syncvaltail)
        {
            packbuf[j++] = syncval[myconnectindex][syncvaltail&(MOVEFIFOSIZ-1)];
            syncvaltail++;
        }

        for(i=connecthead;i>=0;i=connectpoint2[i])
            if (i != myconnectindex)
                sendpacket(i,packbuf,j);

        return;
    }
    if (myconnectindex != connecthead)   //Slave
    {
            //Fix timers and buffer/jitter value
        if (((movefifoend[myconnectindex]-1)&(TIMERUPDATESIZ-1)) == 0)
        {
            i = myminlag[connecthead]-otherminlag;
            if (klabs(i) > 8) i >>= 1;
            else if (klabs(i) > 2) i = ksgn(i);
            else i = 0;

            totalclock -= TICSPERFRAME*i;
            myminlag[connecthead] -= i; otherminlag += i;

            for(i=connecthead;i>=0;i=connectpoint2[i])
                myminlag[i] = 0x7fffffff;
        }

        packbuf[0] = 1; packbuf[1] = 0; j = 2;

        osyn = (input *)&inputfifo[(movefifoend[myconnectindex]-2)&(MOVEFIFOSIZ-1)][myconnectindex];
        nsyn = (input *)&inputfifo[(movefifoend[myconnectindex]-1)&(MOVEFIFOSIZ-1)][myconnectindex];

        if (nsyn[0].fvel != osyn[0].fvel)
        {
            packbuf[j++] = (char)nsyn[0].fvel;
            packbuf[j++] = (char)(nsyn[0].fvel>>8);
            packbuf[1] |= 1;
        }
        if (nsyn[0].svel != osyn[0].svel)
        {
            packbuf[j++] = (char)nsyn[0].svel;
            packbuf[j++] = (char)(nsyn[0].svel>>8);
            packbuf[1] |= 2;
        }
        if (nsyn[0].avel != osyn[0].avel)
        {
            packbuf[j++] = (signed char)nsyn[0].avel;
            packbuf[1] |= 4;
        }
        if ((nsyn[0].bits^osyn[0].bits)&0x000000ff) packbuf[j++] = (nsyn[0].bits&255), packbuf[1] |= 8;
        if ((nsyn[0].bits^osyn[0].bits)&0x0000ff00) packbuf[j++] = ((nsyn[0].bits>>8)&255), packbuf[1] |= 16;
        if ((nsyn[0].bits^osyn[0].bits)&0x00ff0000) packbuf[j++] = ((nsyn[0].bits>>16)&255), packbuf[1] |= 32;
        if ((nsyn[0].bits^osyn[0].bits)&0xff000000) packbuf[j++] = ((nsyn[0].bits>>24)&255), packbuf[1] |= 64;
        if (nsyn[0].horz != osyn[0].horz)
        {
            packbuf[j++] = (char)nsyn[0].horz;
            packbuf[1] |= 128;
        }

        while (syncvalhead[myconnectindex] != syncvaltail)
        {
            packbuf[j++] = syncval[myconnectindex][syncvaltail&(MOVEFIFOSIZ-1)];
            syncvaltail++;
        }

        sendpacket(connecthead,packbuf,j);
        return;
    }

        //This allows allow packet-resends
    for(i=connecthead;i>=0;i=connectpoint2[i])
        if (movefifoend[i] <= movefifosendplc)
        {
            packbuf[0] = 127;
            for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
               sendpacket(i,packbuf,1);
            return;
        }

    while (1)  //Master
    {
        for(i=connecthead;i>=0;i=connectpoint2[i])
            if (playerquitflag[i] && (movefifoend[i] <= movefifosendplc)) return;

        osyn = (input *)&inputfifo[(movefifosendplc-1)&(MOVEFIFOSIZ-1)][0];
        nsyn = (input *)&inputfifo[(movefifosendplc  )&(MOVEFIFOSIZ-1)][0];

            //MASTER -> SLAVE packet
        packbuf[0] = 0; j = 1;

            //Fix timers and buffer/jitter value
        if ((movefifosendplc&(TIMERUPDATESIZ-1)) == 0)
        {
            for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
               if (playerquitflag[i])
                packbuf[j++] = min(max(myminlag[i],-128),127);

            for(i=connecthead;i>=0;i=connectpoint2[i])
                myminlag[i] = 0x7fffffff;
        }

        k = j;
        for(i=connecthead;i>=0;i=connectpoint2[i])
           j += playerquitflag[i];
        for(i=connecthead;i>=0;i=connectpoint2[i])
        {
            if (playerquitflag[i] == 0) continue;

            packbuf[k] = 0;
            if (nsyn[i].fvel != osyn[i].fvel)
            {
                packbuf[j++] = (char)nsyn[i].fvel;
                packbuf[j++] = (char)(nsyn[i].fvel>>8);
                packbuf[k] |= 1;
            }
            if (nsyn[i].svel != osyn[i].svel)
            {
                packbuf[j++] = (char)nsyn[i].svel;
                packbuf[j++] = (char)(nsyn[i].svel>>8);
                packbuf[k] |= 2;
            }
            if (nsyn[i].avel != osyn[i].avel)
            {
                packbuf[j++] = (signed char)nsyn[i].avel;
                packbuf[k] |= 4;
            }
            if ((nsyn[i].bits^osyn[i].bits)&0x000000ff) packbuf[j++] = (nsyn[i].bits&255), packbuf[k] |= 8;
            if ((nsyn[i].bits^osyn[i].bits)&0x0000ff00) packbuf[j++] = ((nsyn[i].bits>>8)&255), packbuf[k] |= 16;
            if ((nsyn[i].bits^osyn[i].bits)&0x00ff0000) packbuf[j++] = ((nsyn[i].bits>>16)&255), packbuf[k] |= 32;
            if ((nsyn[i].bits^osyn[i].bits)&0xff000000) packbuf[j++] = ((nsyn[i].bits>>24)&255), packbuf[k] |= 64;
            if (nsyn[i].horz != osyn[i].horz)
            {
                packbuf[j++] = (char)nsyn[i].horz;
                packbuf[k] |= 128;
            }
            k++;
        }

        while (syncvalhead[myconnectindex] != syncvaltail)
        {
            packbuf[j++] = syncval[myconnectindex][syncvaltail&(MOVEFIFOSIZ-1)];
            syncvaltail++;
        }

        for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
            if (playerquitflag[i])
            {
                 sendpacket(i,packbuf,j);
                 if (nsyn[i].bits&(1<<26))
                    playerquitflag[i] = 0;
            }

        movefifosendplc += movesperpacket;
    }
}

extern long cacnum;
typedef struct { long *hand, leng; char *lock; } cactype;
extern cactype cac[];

void caches(void)
{
     short i,k;

     k = 0;
     for(i=0;i<cacnum;i++)
          if ((*cac[i].lock) >= 200)
          {
                sprintf(tempbuf,"Locked- %ld: Leng:%ld, Lock:%ld",i,cac[i].leng,*cac[i].lock);
                printext256(0L,k,31,-1,tempbuf,1); k += 6;
          }

     k += 6;

     for(i=1;i<11;i++)
          if (lumplockbyte[i] >= 200)
          {
                sprintf(tempbuf,"RTS Locked %ld:",i);
                printext256(0L,k,31,-1,tempbuf,1); k += 6;
          }


}

#ifdef RRRA
extern short bike_turn;
#endif

void checksync(void)
{
      long i, k;
#ifdef RRRA
      struct player_struct *p;
#endif

      for(i=connecthead;i>=0;i=connectpoint2[i])
            if (syncvalhead[i] == syncvaltottail) break;
      if (i < 0)
      {
             syncstat = 0;
             do
             {
                     for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
                            if (syncval[i][syncvaltottail&(MOVEFIFOSIZ-1)] !=
                                syncval[connecthead][syncvaltottail&(MOVEFIFOSIZ-1)])
                                 syncstat = 1;
                     syncvaltottail++;
                     for(i=connecthead;i>=0;i=connectpoint2[i])
                            if (syncvalhead[i] == syncvaltottail) break;
             } while (i < 0);
      }
      if (connectpoint2[connecthead] < 0) syncstat = 0;

      if (syncstat)
      {
          printext256(4L,130L,31,0,"Out Of Sync - Please restart yer game",0);
          printext256(4L,138L,31,0,"RUN RDHELP.EXE for information.",0);
      }
      if (syncstate)
      {
          printext256(4L,160L,31,0,"Missed yer Network packet!",0);
          printext256(4L,138L,31,0,"RUN RDHELP.EXE for information.",0);
      }
#ifdef RRRA
      p = &ps[screenpeek];
      if (p->raat5d9)
      {
          char tbuf[40];
          sprintf(tbuf, "drink_amt = %d", p->drink_amt);
          printext256(4,16,31,0,tbuf,0);
          sprintf(tbuf, "TiltStatus = %d", p->TiltStatus);
          printext256(4,32,31,0,tbuf,0);
          sprintf(tbuf, "MotoSpeed = %d", p->MotoSpeed);
          printext256(4,48,31,0,tbuf,0);
          sprintf(tbuf, "VBumpTarget = %d", p->VBumpTarget);
          printext256(4,64,31,0,tbuf,0);
          sprintf(tbuf, "VBumpNow = %d", p->VBumpNow);
          printext256(4,80,31,0,tbuf,0);
          sprintf(tbuf, "horiz = %d", p->horiz);
          printext256(4,96,31,0,tbuf,0);
          sprintf(tbuf, "TurbCount = %d", p->TurbCount);
          printext256(4,112,31,0,tbuf,0);
          sprintf(tbuf, "MotoOnGround = %d", p->MotoOnGround);
          printext256(4,128,31,0,tbuf,0);
          sprintf(tbuf, "ud.player_skill = %d", ud.player_skill);
          printext256(4,144,31,0,tbuf,0);
          sprintf(tbuf, "DrugMode = %d", p->DrugMode);
          printext256(4,160,31,0,tbuf,0);
          sprintf(tbuf, "WindDir = %d", WindDir);
          printext256(4,176,31,0,tbuf,0);
          sprintf(tbuf, "WindTime = %d", WindTime);
          printext256(4,192,31,0,tbuf,0);
          sprintf(tbuf, "OnMotorcycle = %d", p->OnMotorcycle);
          printext256(4,208,31,0,tbuf,0);
          sprintf(tbuf, "bike_turn = %d", bike_turn);
          printext256(4,224,31,0,tbuf,0);
          sprintf(tbuf, "myconnectindex = %d", myconnectindex);
          printext256(4,240,31,0,tbuf,0);
          sprintf(tbuf, "OnBoat = %d", p->OnBoat);
          printext256(4,256,31,0,tbuf,0);
          sprintf(tbuf, "NotOnWater = %d", p->NotOnWater);
          printext256(4,272,31,0,tbuf,0);
          sprintf(tbuf, "SeaSick = %d", p->SeaSick);
          printext256(4,288,31,0,tbuf,0);
          sprintf(tbuf, "rotscrnang = %d", p->rotscrnang);
          printext256(4,304,31,0,tbuf,0);
          sprintf(tbuf, "ud.level_number = %d", ud.level_number);
          printext256(4,320,31,0,tbuf,0);
          sprintf(tbuf, "ud.volume_number = %d", ud.volume_number);
          printext256(4,336,31,0,tbuf,0);
          sprintf(tbuf, "on_ground = %d", p->on_ground);
          printext256(4,352,31,0,tbuf,0);
          sprintf(tbuf, "fogtype = %d", p->fogtype);
          printext256(4,368,31,0,tbuf,0);
          sprintf(tbuf, "SlotWin = %d", p->SlotWin);
          printext256(4,384,31,0,tbuf,0);
          sprintf(tbuf, "spritebridge = %d", p->spritebridge);
          printext256(4,400,31,0,tbuf,0);
          sprintf(tbuf, "BellTime = %d", BellTime);
          printext256(4,416,31,0,tbuf,0);
      }
#endif
}


void check_fta_sounds(short i)
{
    if(sprite[i].extra > 0) switch(PN)
    {
        case COOT: // LIZTROOP
#ifdef RRRA
            if ((TRAND&3)==2)
#endif
            spritesound(PRED_RECOG, i);
            break;
        case LTH: // LIZMAN
            break;
        case BILLYCOCK:
        case BILLYRAY:
        case BRAYSNIPER: // PIGCOP
            spritesound(PIG_RECOG, i);
            break;
        case DOGRUN:
        case HULK:
        case HEN:
        case DRONE:
        case PIG:
        case RECON:
        case MINION:
        case COW:
        case VIXEN:
#ifdef RRRA
        case RABBIT:
#endif
            break;
    }
}

short inventory(spritetype *s)
{
    switch(s->picnum)
    {
        case FIRSTAID:
        case STEROIDS:
        case HEATSENSOR:
        case BOOTS:
        case JETPACK:
        case HOLODUKE:
        case AIRTANK:
            return 1;
    }
    return 0;
}
