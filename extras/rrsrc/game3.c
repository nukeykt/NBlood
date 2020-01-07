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

#include "warpfx.h"

extern int sendmessagecommand;
extern char recbuf[];
extern long lastvisinc;

int crashcnt;

void typemode(void)
{
     short ch, hitstate, i, j;

     if( ps[myconnectindex].gm&MODE_SENDTOWHOM )
     {
          if(sendmessagecommand != -1 || ud.multimode < 3)
          {
                tempbuf[0] = 4;
                tempbuf[1] = 0;
                recbuf[0]  = 0;

                if(ud.multimode < 3)
                     sendmessagecommand = 2;

                strcat(recbuf,ud.user_name[myconnectindex]);
                strcat(recbuf,": ");
                strcat(recbuf,typebuf);
                j = strlen(recbuf);
                recbuf[j] = 0;
                strcat(tempbuf+1,recbuf);

                if(sendmessagecommand >= ud.multimode)
                {
                     for(ch=connecthead;ch >= 0;ch=connectpoint2[ch])
                          if (ch != myconnectindex)
                                sendpacket(ch,tempbuf,j+1);
                     user_quote = recbuf;
                     user_quote_time = 160;
                }
                else if(sendmessagecommand >= 0)
                     sendpacket(sendmessagecommand,tempbuf,j+1);

                sendmessagecommand = -1;
                ps[myconnectindex].gm &= ~(MODE_TYPE|MODE_SENDTOWHOM);
          }
          else if(sendmessagecommand == -1)
          {
                sprintf(buf,"\"%s\"",typebuf);
                gametext(320>>1,36,buf,0);
                gametext(320>>1,50,"SEND MESSAGE TO...",0);
                for(i=0;i<ud.multimode;i++)
                {
                     if (i == myconnectindex)
                     {
                         sprintf(buf,"A.      ALL");
                     }
                     else
                     {
                         sprintf(buf,"%ld.      %s",i+1,ud.user_name[i]);
                     }
                     gametext((320>>1)-40,60+i*8,buf,0);
                }

                sprintf(buf,"PRESS 1-%ld FOR INDIVIDUAL PLAYER.",ud.multimode);
                gametext(320>>1,60+ud.multimode*8,buf,0);
                gametext(320>>1,68+ud.multimode*8,"'A' OR 'ENTER' FOR ALL PLAYERS",0);
                gametext(320>>1,76+ud.multimode*8,"ESC ABORTS",0);

                if( KB_KeyWaiting() )
                {
                     i = KB_GetCh();

                     if(i == 'A' || i == 'a' || i == 13)
                          sendmessagecommand = ud.multimode;
                     else if(i >= '1' || i <= (ud.multimode + '1') )
                          sendmessagecommand = i - '1';
                     else
                     {
                          ps[myconnectindex].gm &= ~(MODE_TYPE|MODE_SENDTOWHOM);
                          sendmessagecommand = -1;
                          typebuf[0] = 0;
                     }

                     KB_ClearKeyDown(sc_1);
                     KB_ClearKeyDown(sc_2);
                     KB_ClearKeyDown(sc_3);
                     KB_ClearKeyDown(sc_4);
                     KB_ClearKeyDown(sc_5);
                     KB_ClearKeyDown(sc_6);
                     KB_ClearKeyDown(sc_7);
                     KB_ClearKeyDown(sc_8);
                     KB_ClearKeyDown(sc_A);
                     KB_ClearKeyDown(sc_Escape);
                     KB_ClearKeyDown(sc_Enter);
                }
          }
     }
     else
     {
          if(ud.screen_size > 0)
              hitstate = strget(320>>1,200-59,typebuf,30,1);
          else
              hitstate = strget(320>>1,200-29,typebuf,30,1);

          if(hitstate == 1)
          {
                KB_ClearKeyDown(sc_Enter);
                ps[myconnectindex].gm |= MODE_SENDTOWHOM;
          }
          else if(hitstate == -1)
                ps[myconnectindex].gm &= ~(MODE_TYPE|MODE_SENDTOWHOM);
          else pub = NUMPAGES;
     }
}


void displayrest(long smoothratio)
{
    long a, i, j;

    struct player_struct *pp;
    walltype *wal;
    long cposx,cposy,cang;

    pp = &ps[screenpeek];

    if( pp->pals_time > 0 && pp->loogcnt == 0)
    {
        palto( pp->pals[0],
               pp->pals[1],
               pp->pals[2],
               pp->pals_time|128);

        restorepalette = 1;
    }
    else if( restorepalette )
    {
        setbrightness(ud.brightness>>2,&pp->palette[0]);
        restorepalette = 0;
    }
    else if (pp->loogcnt)
    {
    }

    if(ud.show_help)
    {
        switch(ud.show_help)
        {
            case 1:
                rotatesprite(0,0,65536L,0,TEXTSTORY,0,0,10+16+64, 0,0,xdim-1,ydim-1);
                break;
            case 2:
                rotatesprite(0,0,65536L,0,F1HELP,0,0,10+16+64, 0,0,xdim-1,ydim-1);
                break;
#ifdef RRRA
            case 3:
                rotatesprite(0,0,65536L,0,RRTILE1636,0,0,10+16+64, 0,0,xdim-1,ydim-1);
                break;
#endif
        }

        if ( KB_KeyPressed(sc_Escape ) )
        {
            KB_ClearKeyDown(sc_Escape);
            ud.show_help = 0;
            if(ud.multimode < 2 && ud.recstat != 2)
            {
                ready2send = 1;
                totalclock = ototalclock;
            }
            vscrn();
        }
        return;
    }

    i = pp->cursectnum;

    show2dsector[i>>3] |= (1<<(i&7));
    wal = &wall[sector[i].wallptr];
    for(j=sector[i].wallnum;j>0;j--,wal++)
    {
        i = wal->nextsector;
        if (i < 0) continue;
        if (wal->cstat&0x0071) continue;
        if (wall[wal->nextwall].cstat&0x0071) continue;
        if (sector[i].lotag == 32767) continue;
        if (sector[i].ceilingz >= sector[i].floorz) continue;
        show2dsector[i>>3] |= (1<<(i&7));
    }

    if(ud.camerasprite == -1)
    {
        if( ud.overhead_on != 2 )
        {
            if(pp->newowner < 0)
            {
                displayweapon(screenpeek);
                if(pp->over_shoulder_on == 0 )
                    displaymasks(screenpeek);
            }
        }

        if( ud.overhead_on > 0 )
        {
                smoothratio = min(max(smoothratio,0),65536);
                dointerpolations(smoothratio);
                if( ud.scrollmode == 0 )
                {
                     if(pp->newowner == -1)
                     {
                         if (screenpeek == myconnectindex && numplayers > 1)
                         {
                             cposx = omyx+mulscale16((long)(myx-omyx),smoothratio);
                             cposy = omyy+mulscale16((long)(myy-omyy),smoothratio);
                             cang = omyang+mulscale16((long)(((myang+1024-omyang)&2047)-1024),smoothratio);
                         }
                         else
                         {
                              cposx = pp->oposx+mulscale16((long)(pp->posx-pp->oposx),smoothratio);
                              cposy = pp->oposy+mulscale16((long)(pp->posy-pp->oposy),smoothratio);
                              cang = pp->oang+mulscale16((long)(((pp->ang+1024-pp->oang)&2047)-1024),smoothratio);
                         }
                    }
                    else
                    {
                        cposx = pp->oposx;
                        cposy = pp->oposy;
                        cang = pp->oang;
                    }
                }
                else
                {

                     ud.fola += ud.folavel>>3;
                     ud.folx += (ud.folfvel*sintable[(512+2048-ud.fola)&2047])>>14;
                     ud.foly += (ud.folfvel*sintable[(512+1024-512-ud.fola)&2047])>>14;

                     cposx = ud.folx;
                     cposy = ud.foly;
                     cang = ud.fola;
                }

                if(ud.overhead_on == 2)
                {
                    clearview(0L);
                    drawmapview(cposx,cposy,pp->zoom,cang);
                }
                drawoverheadmap( cposx,cposy,pp->zoom,cang);

                restoreinterpolations();

                if(ud.overhead_on == 2)
                {
                    a = 0;
                    if (lastlevel)
                        minitext(1,a+6,"CLOSE ENCOUNTERS",0,2+8+16);
                    else
                    {
                        minitext(1,a+6,volume_names[ud.volume_number],0,2+8+16);
                        minitext(1,a+12,level_names[ud.volume_number*7 + ud.level_number],0,2+8+16);
                    }
                }
        }
    }

    coolgaugetext(screenpeek);
    gutmeter(screenpeek);
    operatefta();

    if( KB_KeyPressed(sc_Escape) && ud.overhead_on == 0
        && ud.show_help == 0
        && ps[myconnectindex].newowner == -1)
    {
        if( (ps[myconnectindex].gm&MODE_MENU) == MODE_MENU && current_menu < 51)
        {
            KB_ClearKeyDown(sc_Escape);
            ps[myconnectindex].gm &= ~MODE_MENU;
            if(ud.multimode < 2 && ud.recstat != 2)
            {
                ready2send = 1;
                totalclock = ototalclock;
                cameraclock = totalclock;
                cameradist = 65536L;
            }
            walock[MAXTILES-1] = 199;
            vscrn();
        }
        else if( (ps[myconnectindex].gm&MODE_MENU) != MODE_MENU &&
            ps[myconnectindex].newowner == -1 &&
            (ps[myconnectindex].gm&MODE_TYPE) != MODE_TYPE)
        {
            KB_ClearKeyDown(sc_Escape);
            FX_StopAllSounds();
            clearsoundlocks();

            intomenusounds();

            ps[myconnectindex].gm |= MODE_MENU;

            if(ud.multimode < 2 && ud.recstat != 2) ready2send = 0;

            if(ps[myconnectindex].gm&MODE_GAME) cmenu(50);
            else cmenu(0);
            screenpeek = myconnectindex;
        }
    }

    if(ps[myconnectindex].newowner == -1 && ud.overhead_on == 0 && ud.crosshair && ud.camerasprite == -1)
        rotatesprite((160L-(ps[myconnectindex].look_ang>>1))<<16,100L<<16,32768L,0,CROSSHAIR,0,0,2+1,windowx1,windowy1,windowx2,windowy2);

    if(ps[myconnectindex].gm&MODE_TYPE)
        typemode();
    else
        menus();
    
    if( ud.pause_on==1 && (ps[myconnectindex].gm&MODE_MENU) == 0 )
        menutext(160,100,0,0,"GAME PAUSED");

    if(ud.coords)
        coords(screenpeek);
    if(ud.tickrate)
        tics();
}


void updatesectorz(long x, long y, long z, short *sectnum)
{
    walltype *wal;
    long i, j, cz, fz;

    getzsofslope(*sectnum,x,y,&cz,&fz);
    if ((z >= cz) && (z <= fz))
        if (inside(x,y,*sectnum) != 0) return;

    if ((*sectnum >= 0) && (*sectnum < numsectors))
    {
        wal = &wall[sector[*sectnum].wallptr];
        j = sector[*sectnum].wallnum;
        do
        {
            i = wal->nextsector;
            if (i >= 0)
            {
                getzsofslope(i,x,y,&cz,&fz);
                if ((z >= cz) && (z <= fz))
                    if (inside(x,y,(short)i) == 1)
                        { *sectnum = i; return; }
            }
            wal++; j--;
        } while (j != 0);
    }

    for(i=numsectors-1;i>=0;i--)
    {
        getzsofslope(i,x,y,&cz,&fz);
        if ((z >= cz) && (z <= fz))
            if (inside(x,y,(short)i) == 1)
                { *sectnum = i; return; }
    }

    *sectnum = -1;
}

void view(struct player_struct *pp, long *vx, long *vy,long *vz,short *vsectnum, short ang, short horiz)
{
     spritetype *sp;
     long i, nx, ny, nz, hx, hy, hz, hitx, hity, hitz;
     short bakcstat, hitsect, hitwall, hitsprite, daang;

     nx = (sintable[(ang+1536)&2047]>>4);
     ny = (sintable[(ang+1024)&2047]>>4);
     nz = (horiz-100)*128;

     sp = &sprite[pp->i];

     bakcstat = sp->cstat;
     sp->cstat &= (short)~0x101;

     updatesectorz(*vx,*vy,*vz,vsectnum);
     hitscan(*vx,*vy,*vz,*vsectnum,nx,ny,nz,&hitsect,&hitwall,&hitsprite,&hitx,&hity,&hitz,CLIPMASK1);

     if(*vsectnum < 0)
     {
        sp->cstat = bakcstat;
        return;
     }

     hx = hitx-(*vx); hy = hity-(*vy);
     if (klabs(nx)+klabs(ny) > klabs(hx)+klabs(hy))
     {
         *vsectnum = hitsect;
         if (hitwall >= 0)
         {
             daang = getangle(wall[wall[hitwall].point2].x-wall[hitwall].x,
                                    wall[wall[hitwall].point2].y-wall[hitwall].y);

             i = nx*sintable[daang]+ny*sintable[(daang+1536)&2047];
             if (klabs(nx) > klabs(ny)) hx -= mulscale28(nx,i);
                                          else hy -= mulscale28(ny,i);
         }
         else if (hitsprite < 0)
         {
             if (klabs(nx) > klabs(ny)) hx -= (nx>>5);
                                          else hy -= (ny>>5);
         }
         if (klabs(nx) > klabs(ny)) i = divscale16(hx,nx);
                                      else i = divscale16(hy,ny);
         if (i < cameradist) cameradist = i;
     }
     *vx = (*vx)+mulscale16(nx,cameradist);
     *vy = (*vy)+mulscale16(ny,cameradist);
     *vz = (*vz)+mulscale16(nz,cameradist);

     cameradist = min(cameradist+((totalclock-cameraclock)<<10),65536);
     cameraclock = totalclock;

     updatesectorz(*vx,*vy,*vz,vsectnum);

     sp->cstat = bakcstat;
}
     
    //REPLACE FULLY
void drawbackground(void)
{
     short dapicnum;
     long x,y,x1,y1,x2,y2,topy;

     flushperms();

     switch(ud.m_volume_number)
     {
#ifdef RRRA
          default:dapicnum = RRTILE7629;break;
          case 1:dapicnum = RRTILE7629;break;
          case 2:dapicnum = RRTILE7629;break;
#else
          default:dapicnum = BIGHOLE;break;
          case 1:dapicnum = BIGHOLE;break;
          case 2:dapicnum = BIGHOLE;break;
#endif
     }

     y1 = 0; y2 = ydim;
     if( ready2send || ud.recstat == 2 )
     {
        if(ud.coop != 1)
        {
            if (ud.multimode > 1) y1 += scale(ydim,8,200);
            if (ud.multimode > 4) y1 += scale(ydim,8,200);
        }
        if (ud.screen_size >= 8) y2 = scale(ydim,200-34,200);
     }

     for(y=y1;y<y2;y+=128)
          for(x=0;x<xdim;x+=128)
                rotatesprite(x<<16,y<<16,32768L,0,dapicnum,18,0,8+16+64+128,0,y1,xdim-1,y2-1);

     if(ud.screen_size > 12)
     {
          y = 0;
          if(ud.coop != 1)
          {
             if (ud.multimode > 1) y += 8;
             if (ud.multimode > 4) y += 8;
          }

          x1 = max(windowx1-4,0);
          y1 = max(windowy1-4,y);
          x2 = min(windowx2+4,xdim-1);
          y2 = min(windowy2+4,scale(ydim,200-34,200)-1);

          for(y=y1+4;y<y2-4;y+=64)
          {
                rotatesprite(x1<<16,y<<16,65536L,0,VIEWBORDER,0,0,8+16+64+128,x1,y1,x2,y2);
                rotatesprite((x2+1)<<16,(y+64)<<16,65536L,1024,VIEWBORDER,0,0,8+16+64+128,x1,y1,x2,y2);
          }

          for(x=x1+4;x<x2-4;x+=64)
          {
                rotatesprite((x+64)<<16,y1<<16,65536L,512,VIEWBORDER,0,0,8+16+64+128,x1,y1,x2,y2);
                rotatesprite(x<<16,(y2+1)<<16,65536L,1536,VIEWBORDER,0,0,8+16+64+128,x1,y1,x2,y2);
          }

          rotatesprite(x1<<16,y1<<16,65536L,0,VIEWBORDER+1,0,0,8+16+64+128,x1,y1,x2,y2);
          rotatesprite((x2+1)<<16,y1<<16,65536L,512,VIEWBORDER+1,0,0,8+16+64+128,x1,y1,x2,y2);
          rotatesprite((x2+1)<<16,(y2+1)<<16,65536L,1024,VIEWBORDER+1,0,0,8+16+64+128,x1,y1,x2,y2);
          rotatesprite(x1<<16,(y2+1)<<16,65536L,1536,VIEWBORDER+1,0,0,8+16+64+128,x1,y1,x2,y2);
     }
}

#ifdef RRRA
// Floor Over Floor

// If standing in sector with SE42
// then draw viewing to SE41 and raise all =hi SE43 cielings.

// If standing in sector with SE43
// then draw viewing to SE40 and lower all =hi SE42 floors.

// If standing in sector with SE44
// then draw viewing to SE40.

// If standing in sector with SE45
// then draw viewing to SE41.

#define FOFTILE FOF
#define FOFTILEX 32
#define FOFTILEY 32
long tempsectorz[MAXSECTORS];
long tempsectorpicnum[MAXSECTORS];
//short tempcursectnum;

SE_150_Draw(int spnum,long x,long y,long z,short a,short h,long smoothratio)
{
 int i=FOFTILE,j,k=0;
 int floor1=spnum,floor2=0,ok=0,fofmode;
 long offx,offy;

 if(sprite[spnum].ang!=512) return;

 tilesizx[FOFTILE] = 0;
 tilesizy[FOFTILE] = 0;
 if (!(gotpic[i>>3]&(1<<(i&7)))) return;
 gotpic[i>>3] &= ~(1<<(i&7));

 floor1=spnum;

 if(sprite[spnum].lotag==152) fofmode=150;
 if(sprite[spnum].lotag==153) fofmode=151;
 if(sprite[spnum].lotag==154) fofmode=150;
 if(sprite[spnum].lotag==155) fofmode=151;

// fofmode=sprite[spnum].lotag-2;

// sectnum=sprite[j].sectnum;
// sectnum=cursectnum;
 ok++;

/*  recursive?
 for(j=0;j<MAXSPRITES;j++)
 {
  if(
     sprite[j].sectnum==sectnum &&
     sprite[j].picnum==1 &&
     sprite[j].lotag==110
    ) { DrawFloorOverFloor(j); break;}
 }
*/

// if(ok==0) { Message("no fof",RED); return; }

 for(j=0;j<MAXSPRITES;j++)
 {
  if(
     sprite[j].picnum==1 &&
     sprite[j].lotag==fofmode &&
     sprite[j].hitag==sprite[floor1].hitag
    ) { floor1=j; fofmode=sprite[j].lotag; ok++; break;}
 }
// if(ok==1) { Message("no floor1",RED); return; }

 if(fofmode==150) k=151; else k=150;

 for(j=0;j<MAXSPRITES;j++)
 {
  if(
     sprite[j].picnum==1 &&
     sprite[j].lotag==k &&
     sprite[j].hitag==sprite[floor1].hitag
    ) {floor2=j; ok++; break;}
 }

// if(ok==2) { Message("no floor2",RED); return; }

 for(j=0;j<MAXSPRITES;j++)  // raise ceiling or floor
 {
  if(sprite[j].picnum==1 &&
     sprite[j].lotag==k+2 &&
     sprite[j].hitag==sprite[floor1].hitag
    )
    {
     if(k==150)
     {tempsectorz[sprite[j].sectnum]=sector[sprite[j].sectnum].floorz;
      sector[sprite[j].sectnum].floorz+=(((z-sector[sprite[j].sectnum].floorz)/32768)+1)*32768;
      tempsectorpicnum[sprite[j].sectnum]=sector[sprite[j].sectnum].floorpicnum;
      sector[sprite[j].sectnum].floorpicnum=13;
     }
     else if(k==151)
     {tempsectorz[sprite[j].sectnum]=sector[sprite[j].sectnum].ceilingz;
      sector[sprite[j].sectnum].ceilingz+=(((z-sector[sprite[j].sectnum].ceilingz)/32768)-1)*32768;
      tempsectorpicnum[sprite[j].sectnum]=sector[sprite[j].sectnum].ceilingpicnum;
      sector[sprite[j].sectnum].ceilingpicnum=13;
     }
    }
 }

 i=floor1;
 offx=x-sprite[i].x;
 offy=y-sprite[i].y;
 i=floor2;
 drawrooms(offx+sprite[i].x,offy+sprite[i].y,z,a,h,sprite[i].sectnum);
 animatesprites(offx+sprite[i].x,offy+sprite[i].y,a,smoothratio);
 drawmasks();

 for(j=0;j<MAXSPRITES;j++)  // restore ceiling or floor
 {
  if(sprite[j].picnum==1 &&
     sprite[j].lotag==k+2 &&
     sprite[j].hitag==sprite[floor1].hitag
    )
    {
     if(k==150)
     {sector[sprite[j].sectnum].floorz=tempsectorz[sprite[j].sectnum];
      sector[sprite[j].sectnum].floorpicnum=tempsectorpicnum[sprite[j].sectnum];
     }
     else if(k==151)
     {sector[sprite[j].sectnum].ceilingz=tempsectorz[sprite[j].sectnum];
      sector[sprite[j].sectnum].ceilingpicnum=tempsectorpicnum[sprite[j].sectnum];
     }
    }// end if
 }// end for

} // end SE40




void SE_150(long x,long y,long z,long a,long h, long smoothratio)
{
    int i;

    for (i = headspritestat[15]; i >= 0; i = nextspritestat[i])
    {
        switch(sprite[i].lotag)
        {
//            case 40:
//            case 41:
//                SE40_Draw(i,x,y,a,smoothratio);
//                break;
            case 152:
            case 153:
            case 154:
            case 155:
                if(ps[screenpeek].cursectnum == sprite[i].sectnum)
                    SE_150_Draw(i,x,y,z,a,h,smoothratio);
                break;
        }
    }
}
#endif

static long oyrepeat=-1;
long tempoyrepeat;

void displayrooms(short snum,long smoothratio)
{
    long cposx,cposy,cposz,dst,j,fz,cz,hz,lz;
    short sect, cang, k, choriz,tsect;
    struct player_struct *p;
    long tposx,tposy,tposz,dx,dy,thoriz,i;
    short tang;

    p = &ps[snum];

    if(screencapt == 0 && (p->gm&MODE_MENU) && ( (current_menu/100) == 3 ) )
        return;

    if(pub > 0)
    {
        if(ud.screen_size > 8) drawbackground();
        pub = 0;
    }

    if( ud.overhead_on == 2 || ud.show_help || p->cursectnum == -1)
        return;

    smoothratio = min(max(smoothratio,0),65536);

#ifdef RRRA
    if (p->fogtype)
        p->visibility = ud.const_visibility;
#endif

    visibility = p->visibility;

    if(ud.pause_on || ps[snum].on_crane > -1) smoothratio = 65536;

    sect = p->cursectnum;
    if(sect < 0 || sect >= MAXSECTORS) return;

    dointerpolations(smoothratio);

    animatecamsprite();

    if(ud.camerasprite >= 0)
    {
        spritetype *s;

        s = &sprite[ud.camerasprite];

        if(s->yvel < 0) s->yvel = -100;
        else if(s->yvel > 199) s->yvel = 300;

        cang = hittype[ud.camerasprite].tempang+mulscale16((long)(((s->ang+1024-hittype[ud.camerasprite].tempang)&2047)-1024),smoothratio);

        drawrooms(s->x,s->y,s->z-(4<<8),cang,s->yvel,s->sectnum);
        animatesprites(s->x,s->y,cang,smoothratio);
        drawmasks();
    }
    else
    {
        i = divscale22(1,64);
#ifdef RRRA
        if (i != oyrepeat && !p->DrugMode)
#else
        if (i != oyrepeat)
#endif
        {
            oyrepeat = i;
            setaspect(oyrepeat,yxaspect);
        }

        if(screencapt)
        {
            walock[MAXTILES-1] = 199;
            if (waloff[MAXTILES-1] == 0)
                allocache((long *)&waloff[MAXTILES-1],100*160,&walock[MAXTILES-1]);
            setviewtotile(MAXTILES-1,100L,160L);
        }
        else if( ( ud.screen_tilting && p->rotscrnang ) || ud.detail==0 || p->drink_amt > 89)
        {
                if (ud.screen_tilting) tang = p->rotscrnang; else tang = 0;

#ifdef RRRA
                walock[MAXTILES-2] = 199;
                if (p->drink_amt > 89)
                {
                    if (waloff[MAXTILES-2] == 0)
                        allocache(&waloff[MAXTILES-2],320L*320L,&walock[MAXTILES-2]);
                    if ((tang&1023) == 0)
                        setviewtotile(MAXTILES-2,200L>>(1-ud.detail),320L>>(1-ud.detail));
                    else
                        setviewtotile(MAXTILES-2,320L>>(1-ud.detail),320L>>(1-ud.detail));
                }
                else
                {
                    if (waloff[MAXTILES-2] == 0)
                        allocache(&waloff[MAXTILES-2],640L*640L,&walock[MAXTILES-2]);
                    if ((tang&1023) == 0)
                        setviewtotile(MAXTILES-2,480L>>(1-ud.detail),640L>>(1-ud.detail));
                    else
                        setviewtotile(MAXTILES-2,640L>>(1-ud.detail),640L>>(1-ud.detail));
                }
#else
                walock[MAXTILES-2] = 255;
                if (waloff[MAXTILES-2] == 0)
                    allocache(&waloff[MAXTILES-2],320L*320L,&walock[MAXTILES-2]);
                if ((tang&1023) == 0)
                    setviewtotile(MAXTILES-2,200L>>(1-ud.detail),320L>>(1-ud.detail));
                else
                    setviewtotile(MAXTILES-2,320L>>(1-ud.detail),320L>>(1-ud.detail));
#endif
                if ((tang&1023) == 512)
                {     //Block off unscreen section of 90ø tilted screen
                    j = ((320-60)>>(1-ud.detail));
                    for(i=(60>>(1-ud.detail))-1;i>=0;i--)
                    {
                        startumost[i] = 1; startumost[i+j] = 1;
                        startdmost[i] = 0; startdmost[i+j] = 0;
                    }
                }

                i = (tang&511); if (i > 256) i = 512-i;
                i = sintable[i+512]*8 + sintable[i]*5L;
                setaspect(i>>1,yxaspect);
          }

#ifdef RRRA
        if (!(p->gm & MODE_MENU))
        {
            if (p->DrugMode > 0 && !(p->gm&MODE_TYPE) && !ud.pause_on)
            {
                int var_8c;
                if (p->raat5f1 == 0)
                {
                    p->raat5f3++;
                    var_8c = oyrepeat + p->raat5f3 * 5000;
                    if (oyrepeat * 3 < var_8c)
                    {
                        setaspect(oyrepeat * 3, yxaspect);
                        p->raat5f7 = oyrepeat * 3;
                        p->raat5f1 = 2;
                    }
                    else
                    {
                        setaspect(var_8c, yxaspect);
                        p->raat5f7 = var_8c;
                    }
                    setpal(p);
                }
                else if (p->raat5f1 == 3)
                {
                    p->raat5f3--;
                    var_8c = oyrepeat + p->raat5f3 * 5000;
                    if (var_8c < oyrepeat)
                    {
                        setaspect(oyrepeat, yxaspect);
                        p->DrugMode = 0;
                        p->raat5f1 = 0;
                        p->raat5f5 = 0;
                        p->raat5f3 = 0;
                    }
                    else
                    {
                        setaspect(var_8c, yxaspect);
                        p->raat5f7 = var_8c;
                    }
                    setpal(p);
                }
                else if (p->raat5f1 == 2)
                {
                    if (p->raat5f5 > 30)
                    {
                        p->raat5f1 = 1;
                    }
                    else
                    {
                        p->raat5f5++;
                        setaspect(p->raat5f5 * 500 + oyrepeat * 3, yxaspect);
                        p->raat5f7 = oyrepeat * 3 + p->raat5f5 * 500;
                        setpal(p);
                    }
                }
                else
                {
                    if (p->raat5f5 < 1)
                    {
                        p->raat5f1 = 2;
                        p->DrugMode--;
                        if (p->DrugMode == 1)
                            p->raat5f1 = 3;
                    }
                    else
                    {
                        p->raat5f5--;
                        setaspect(p->raat5f5 * 500 + oyrepeat * 3, yxaspect);
                        p->raat5f7 = oyrepeat * 3 + p->raat5f5 * 500;
                        setpal(p);
                    }
                }
            }
        }
        else if (p->DrugMode > 0)
        {
            setaspect(p->raat5f7, yxaspect);
            setpal(p);
        }
#endif
        
          if ( (snum == myconnectindex) && (numplayers > 1) )
		  {
				cposx = omyx+mulscale16((long)(myx-omyx),smoothratio);
				cposy = omyy+mulscale16((long)(myy-omyy),smoothratio);
				cposz = omyz+mulscale16((long)(myz-omyz),smoothratio);
				cang = omyang+mulscale16((long)(((myang+1024-omyang)&2047)-1024),smoothratio);
				choriz = omyhoriz+omyhorizoff+mulscale16((long)(myhoriz+myhorizoff-omyhoriz-omyhorizoff),smoothratio);
				sect = mycursectnum;
		  }
		  else
		  {
				cposx = p->oposx+mulscale16((long)(p->posx-p->oposx),smoothratio);
				cposy = p->oposy+mulscale16((long)(p->posy-p->oposy),smoothratio);
				cposz = p->oposz+mulscale16((long)(p->posz-p->oposz),smoothratio);
				cang = p->oang+mulscale16((long)(((p->ang+1024-p->oang)&2047)-1024),smoothratio);
				choriz = p->ohoriz+p->ohorizoff+mulscale16((long)(p->horiz+p->horizoff-p->ohoriz-p->ohorizoff),smoothratio);
		  }
		  cang += p->look_ang;

		  if (p->newowner >= 0)
		  {
				cang = p->ang+p->look_ang;
				choriz = p->horiz+p->horizoff;
				cposx = p->posx;
				cposy = p->posy;
				cposz = p->posz;
				sect = sprite[p->newowner].sectnum;
				smoothratio = 65536L;
		  }

		  else if( p->over_shoulder_on == 0 )
				cposz += p->opyoff+mulscale16((long)(p->pyoff-p->opyoff),smoothratio);
		  else view(p,&cposx,&cposy,&cposz,&sect,cang,choriz);

        cz = hittype[p->i].ceilingz;
        fz = hittype[p->i].floorz;

        if(earthquaketime > 0 && p->on_ground == 1)
        {
            cposz += 256-(((earthquaketime)&1)<<9);
            cang += (2-((earthquaketime)&2))<<2;
        }

        if(sprite[p->i].pal == 1) cposz -= (18<<8);

        if(p->newowner >= 0)
            choriz = 100+sprite[p->newowner].shade;
        else if(p->spritebridge == 0)
        {
            if( cposz < ( p->truecz + (4<<8) ) ) cposz = cz + (4<<8);
            else if( cposz > ( p->truefz - (4<<8) ) ) cposz = fz - (4<<8);
        }

        if (sect >= 0)
        {
            getzsofslope(sect,cposx,cposy,&cz,&fz);
            if (cposz < cz+(4<<8)) cposz = cz+(4<<8);
            if (cposz > fz-(4<<8)) cposz = fz-(4<<8);
        }

        if(choriz > 299) choriz = 299;
        else if(choriz < -99) choriz = -99;

#ifdef RRRA
        SE_150(cposx,cposy,cposz,cang,choriz,smoothratio);
#endif

        if ((gotpic[MIRROR>>3]&(1<<(MIRROR&7))) > 0)
        {
            dst = 0x7fffffff; i = 0;
            for(k=0;k<mirrorcnt;k++)
            {
                j = klabs(wall[mirrorwall[k]].x-cposx);
                j += klabs(wall[mirrorwall[k]].y-cposy);
                if (j < dst) dst = j, i = k;
            }

            if( wall[mirrorwall[i]].overpicnum == MIRROR )
            {
                preparemirror(cposx,cposy,cposz,cang,choriz,mirrorwall[i],mirrorsector[i],&tposx,&tposy,&tang);

                j = visibility;
                visibility = (j>>1) + (j>>2);

                drawrooms(tposx,tposy,cposz,tang,choriz,mirrorsector[i]+MAXSECTORS);

                display_mirror = 1;
                animatesprites(tposx,tposy,tang,smoothratio);
                display_mirror = 0;

                drawmasks();
                completemirror();   //Reverse screen x-wise in this function
                visibility = j;
            }
            gotpic[MIRROR>>3] &= ~(1<<(MIRROR&7));
        }

        if (sector[sect].lotag == 848)
        {
            short gs, tgsect, nextspr, geosect, geoid;
            int spr;
            drawrooms(cposx, cposy, cposz, cang, choriz, sect);
            animatesprites(cposx,cposy,cang,smoothratio);
            drawmasks();
            for (gs = 0; gs < geocnt; gs++)
            {
                tgsect = geosector[gs];
                spr = headspritesect[tgsect];
                while (spr != -1)
                {
                    nextspr = nextspritesect[spr];
                    changespritesect((short)spr, geosectorwarp[gs]);
                    setsprite((short)spr,sprite[spr].x -= geox[gs],sprite[spr].y -= geoy[gs],sprite[spr].z);
                    spr = nextspr;
                }
                if (geosector[gs] == sect)
                {
                    geosect = geosectorwarp[gs];
                    geoid = gs;
                }
            }
            cposx -= geox[geoid];
            cposy -= geoy[geoid];
            drawrooms(cposx, cposy, cposz, cang, choriz, sect);
            cposx += geox[geoid];
            cposy += geoy[geoid];
            for (gs = 0; gs < geocnt; gs++)
            {
                tgsect = geosectorwarp[gs];
                spr = headspritesect[tgsect];
                while (spr != -1)
                {
                    nextspr = nextspritesect[spr];
                    changespritesect((short)spr, geosector[gs]);
                    setsprite((short)spr,sprite[spr].x += geox[gs],sprite[spr].y += geoy[gs],sprite[spr].z);
                    spr = nextspr;
                }
            }
            animatesprites(cposx,cposy,cang,smoothratio);
            drawmasks();
            for (gs = 0; gs < geocnt; gs++)
            {
                tgsect = geosector[gs];
                spr = headspritesect[tgsect];
                while (spr != -1)
                {
                    nextspr = nextspritesect[spr];
                    changespritesect((short)spr, geosectorwarp2[gs]);
                    setsprite((short)spr,sprite[spr].x -= geox2[gs],sprite[spr].y -= geoy2[gs],sprite[spr].z);
                    spr = nextspr;
                }
                if (geosector[gs] == sect)
                {
                    geosect = geosectorwarp2[gs];
                    geoid = gs;
                }
            }
            cposx -= geox2[geoid];
            cposy -= geoy2[geoid];
            drawrooms(cposx, cposy, cposz, cang, choriz, sect);
            cposx += geox2[geoid];
            cposy += geoy2[geoid];
            for (gs = 0; gs < geocnt; gs++)
            {
                tgsect = geosectorwarp2[gs];
                spr = headspritesect[tgsect];
                while (spr != -1)
                {
                    nextspr = nextspritesect[spr];
                    changespritesect((short)spr, geosector[gs]);
                    setsprite((short)spr,sprite[spr].x += geox2[gs],sprite[spr].y += geoy2[gs],sprite[spr].z);
                    spr = nextspr;
                }
            }
            animatesprites(cposx,cposy,cang,smoothratio);
            drawmasks();
        }
        else
        {
            drawrooms(cposx, cposy, cposz, cang, choriz, sect);
            animatesprites(cposx, cposy, cang, smoothratio);
            drawmasks();
        }

        if(screencapt == 1)
        {
            setviewback();
            walock[MAXTILES-1] = 190;
            screencapt = 0;
        }
        else if( ( ud.screen_tilting && p->rotscrnang) || ud.detail==0 || p->drink_amt > 89)
        {
            if (ud.screen_tilting) tang = p->rotscrnang; else tang = 0;
            setviewback();
            picanm[MAXTILES-2] &= 0xff0000ff;
            i = (tang&511); if (i > 256) i = 512-i;
            i = sintable[i+512]*8 + sintable[i]*5L;
#ifdef RRRA
            if (p->drink_amt > 89)
            {
                if ((1-ud.detail) == 0) i >>= 1;
            }
            else
            {
                if ((1-ud.detail) == 0) i >>= 2;
            }
#else
            if ((1-ud.detail) == 0) i >>= 1;
#endif
            rotatesprite(160<<16,100<<16,i,tang+512,MAXTILES-2,0,0,4+2+64,windowx1,windowy1,windowx2,windowy2);
            walock[MAXTILES-2] = 199;
        }
    }

    restoreinterpolations();

#ifdef RRRA
    if (p->fogtype) return;
#endif

    if (totalclock < lastvisinc)
    {
        if (klabs(p->visibility-ud.const_visibility) > 8)
            p->visibility += (ud.const_visibility-p->visibility)>>2;
    }
    else p->visibility = ud.const_visibility;
}





short LocateTheLocator(short n,short sn)
{
    short i;

    i = headspritestat[7];
    while(i >= 0)
    {
        if( (sn == -1 || sn == SECT) && n == SLT )
            return i;
        i = nextspritestat[i];
    }
    return -1;
}

short EGS(short whatsect,long s_x,long s_y,long s_z,short s_pn,signed char s_s,signed char s_xr,signed char s_yr,short s_a,short s_ve,long s_zv,short s_ow,signed char s_ss)
{
    short i;
    spritetype *s;

    if (s_ow < 0)
        return 0;

    i = insertsprite(whatsect,s_ss);

    if( i < 0 )
        gameexit(" Too many sprites spawned.");

    hittype[i].bposx = s_x;
    hittype[i].bposy = s_y;
    hittype[i].bposz = s_z;

    s = &sprite[i];

    s->x = s_x;
    s->y = s_y;
    s->z = s_z;
    s->cstat = 0;
    s->picnum = s_pn;
    s->shade = s_s;
    s->xrepeat = s_xr;
    s->yrepeat = s_yr;
    s->pal = 0;

    s->ang = s_a;
    s->xvel = s_ve;
    s->zvel = s_zv;
    s->owner = s_ow;
    s->xoffset = 0;
    s->yoffset = 0;
    s->yvel = 0;
    s->clipdist = 0;
    s->pal = 0;
    s->lotag = 0;

    hittype[i].picnum = sprite[s_ow].picnum;

    hittype[i].lastvx = 0;
    hittype[i].lastvy = 0;

    hittype[i].timetosleep = 0;
    hittype[i].actorstayput = -1;
    hittype[i].extra = -1;
    hittype[i].owner = s_ow;
    hittype[i].cgg = 0;
    hittype[i].movflag = 0;
    hittype[i].tempang = 0;
    hittype[i].dispicnum = 0;
    hittype[i].floorz = hittype[s_ow].floorz;
    hittype[i].ceilingz = hittype[s_ow].ceilingz;

    T1=T3=T4=T6=0;
    if( actorscrptr[s_pn] )
    {
        s->extra = *actorscrptr[s_pn];
        T5 = *(actorscrptr[s_pn]+1);
        T2 = *(actorscrptr[s_pn]+2);
        s->hitag = *(actorscrptr[s_pn]+3);
    }
    else
    {
        T2=T5=0;
        s->extra = 0;
        s->hitag = 0;
    }

    if (show2dsector[SECT>>3]&(1<<(SECT&7))) show2dsprite[i>>3] |= (1<<(i&7));
    else show2dsprite[i>>3] &= ~(1<<(i&7));
/*
    if(s->sectnum < 0)
    {
        s->xrepeat = s->yrepeat = 0;
        changespritestat(i,5);
    }
*/
    return(i);
}

char wallswitchcheck(short i)
{
    switch(PN)
    {
        case HANDPRINTSWITCH:
        case HANDPRINTSWITCH+1:
        case ALIENSWITCH:
        case ALIENSWITCH+1:
        case MULTISWITCH:
        case MULTISWITCH+1:
        case MULTISWITCH+2:
        case MULTISWITCH+3:
        case ACCESSSWITCH:
        case ACCESSSWITCH2:
        case PULLSWITCH:
        case PULLSWITCH+1:
        case HANDSWITCH:
        case HANDSWITCH+1:
        case SLOTDOOR:
        case SLOTDOOR+1:
        case LIGHTSWITCH:
        case LIGHTSWITCH+1:
        case SPACELIGHTSWITCH:
        case SPACELIGHTSWITCH+1:
        case SPACEDOORSWITCH:
        case SPACEDOORSWITCH+1:
        case FRANKENSTINESWITCH:
        case FRANKENSTINESWITCH+1:
        case LIGHTSWITCH2:
        case LIGHTSWITCH2+1:
        case POWERSWITCH1:
        case POWERSWITCH1+1:
        case LOCKSWITCH1:
        case LOCKSWITCH1+1:
        case POWERSWITCH2:
        case POWERSWITCH2+1:
        case DIPSWITCH:
        case DIPSWITCH+1:
        case DIPSWITCH2:
        case DIPSWITCH2+1:
        case TECHSWITCH:
        case TECHSWITCH+1:
        case DIPSWITCH3:
        case DIPSWITCH3+1:
        case NUKEBUTTON:
        case NUKEBUTTON+1:
#ifdef RRRA
        case MULTISWITCH2:
        case MULTISWITCH2+1:
        case MULTISWITCH2+2:
        case MULTISWITCH2+3:
        case RRTILE8464:
        case RRTILE8464+1:
#endif
            return 1;
    }
    return 0;
}