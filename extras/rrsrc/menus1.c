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

#include "lava.h"
#include "warpfx.h"

extern short inputloc;
extern int recfilep;
extern char vgacompatible;
short probey=0,lastprobey=0,last_menu,globalskillsound=-1;
short sh,onbar,buttonstat,deletespot;
short last_zero,last_fifty,last_threehundred = 0;
char fileselect = 1, menunamecnt, menuname[256][17], curpath[80], menupath[80];

extern ControlInfo minfo;
extern int32 volnum, levnum, plrskl, numplr;

/* Error codes */
#define eTenBnNotInWindows 3801
#define eTenBnBadGameIni 3802
#define eTenBnBadTenIni 3803
#define eTenBnBrowseCancel 3804
#define eTenBnBadTenInst 3805

int  tenBnStart(void);
void tenBnSetBrowseRtn(char *(*rtn)(char *str, int len));
void tenBnSetExitRtn(void (*rtn)(void));
void tenBnSetEndRtn(void (*rtn)(void));

void dummyfunc(void)
{
}

void TENtext(void)
{
    long dacount,dalastcount;

    puts("\nRedneck Rampage has not been licensed exclusively to TEN (Total");
    puts("Entertainment Network) for wide-area networked (WAN) multiplayer");
    puts("games.\n");

    puts("The multiplayer code within Redneck Rampage has been highly");
    puts("customized to run best on TEN, where you'll experience fast and");
    puts("stable performance, plus other special benefits.\n");

    puts("We do not authorize or recommend the use of Redneck Rampage with");
    puts("gaming services other than TEN.\n");

    puts("Redneck Rampage is protected by United States copyright law and");
    puts("international treaty.\n");

    puts("For the best online multiplayer gaming experience, please call TEN");
    puts("at 800-8040-TEN, or visit TEN's Web Site at www.ten.net.\n");

    puts("Press any key to continue.\n");

    _bios_timeofday(0,&dacount);

    while( _bios_keybrd(1) == 0 )
    {
        _bios_timeofday(0,&dalastcount);
        if( (dacount+240) < dalastcount ) break;
    }
}

void cmenu(short cm)
{
    current_menu = cm;

    if( (cm >= 1000 && cm <= 1009) )
        return;

    if( cm == 0 )
        probey = last_zero;
    else if(cm == 50)
        probey = last_fifty;
    else if(cm >= 300 && cm < 400)
        probey = last_threehundred;
    else if(cm == 110)
        probey = 1;
    else probey = 0;
    lastprobey = -1;
}


void savetemp(char *fn,long daptr,long dasiz)
{
    FILE* fp;

    fp = fopen(fn,"wb");

    if (!fp)
        return;

    fwrite((char *)daptr,dasiz,1,fp);

    fclose(fp);
}

void getangplayers(short snum)
{
    short i,a;

    for(i=connecthead;i>=0;i=connectpoint2[i])
    {
        if(i != snum)
        {
            a = ps[snum].ang+getangle(ps[i].posx-ps[snum].posx,ps[i].posy-ps[snum].posy);
            a = a-1024;
            rotatesprite(
                (320<<15) + (((sintable[(a+512)&2047])>>7)<<15),
                (320<<15) - (((sintable[a&2047])>>8)<<15),
                klabs(sintable[((a>>1)+768)&2047]<<2),0,APLAYER,0,ps[i].palookup,0,0,0,xdim-1,ydim-1);
        }
    }
}

loadpheader(char spot,int32 *vn,int32 *ln,int32 *psk,int32 *nump)
{

     long i;
         char *fn = "game0.sav";
         long fil;
     long bv;

         fn[4] = spot+'0';

     if ((fil = kopen4load(fn,0)) == -1) return(-1);

     walock[MAXTILES-3] = 255;

     kdfread(&bv,4,1,fil);
     if(bv != BYTEVERSION)
     {
        FTA(114,&ps[myconnectindex]);
        kclose(fil);
        return 1;
     }

     kdfread(nump,sizeof(int32),1,fil);

     kdfread(tempbuf,19,1,fil);
         kdfread(vn,sizeof(int32),1,fil);
         kdfread(ln,sizeof(int32),1,fil);
     kdfread(psk,sizeof(int32),1,fil);

     if (waloff[MAXTILES-3] == 0) allocache(&waloff[MAXTILES-3],160*100,&walock[MAXTILES-3]);
     tilesizx[MAXTILES-3] = 100; tilesizy[MAXTILES-3] = 160;
     kdfread((char *)waloff[MAXTILES-3],160,100,fil);

         kclose(fil);

         return(0);
}


loadplayer(signed char spot)
{
     short k,music_changed;
     char *fn = "game0.sav";
     char *mpfn = "gameA_00.sav";
     char *fnptr, scriptptrs[MAXSCRIPTSIZE];
     long fil, bv, i, j, x;
     int32 nump;

     if(spot < 0)
     {
        multiflag = 1;
        multiwhat = 0;
        multipos = -spot-1;
        return -1;
     }

     if( multiflag == 2 && multiwho != myconnectindex )
     {
         fnptr = mpfn;
         mpfn[4] = spot + 'A';

         if(ud.multimode > 9)
         {
             mpfn[6] = (multiwho/10) + '0';
             mpfn[7] = (multiwho%10) + '0';
         }
         else mpfn[7] = multiwho + '0';
     }
     else
     {
        fnptr = fn;
        fn[4] = spot + '0';
     }

     if ((fil = kopen4load(fnptr,0)) == -1) return(-1);

     ready2send = 0;

     kdfread(&bv,4,1,fil);
     if(bv != BYTEVERSION)
     {
        FTA(114,&ps[myconnectindex]);
        kclose(fil);
        ototalclock = totalclock;
        ready2send = 1;
        return 1;
     }

     kdfread(&nump,sizeof(nump),1,fil);
     if(nump != numplayers)
     {
        kclose(fil);
        ototalclock = totalclock;
        ready2send = 1;
        FTA(124,&ps[myconnectindex]);
        return 1;
     }

     if(numplayers > 1)
     {
         pub = NUMPAGES;
         pus = NUMPAGES;
         vscrn();
         drawbackground();
         menutext(160,100,0,0,"LOADING...");
         nextpage();
    }

     waitforeverybody();

         FX_StopAllSounds();
     clearsoundlocks();
         MUSIC_StopSong();

     if(numplayers > 1)
         kdfread(&buf,19,1,fil);
     else
         kdfread(&ud.savegame[spot][0],19,1,fil);

     music_changed = (music_select != (ud.volume_number*11) + ud.level_number);

         kdfread(&ud.volume_number,sizeof(ud.volume_number),1,fil);
         kdfread(&ud.level_number,sizeof(ud.level_number),1,fil);
         kdfread(&ud.player_skill,sizeof(ud.player_skill),1,fil);

         ud.m_level_number = ud.level_number;
         ud.m_volume_number = ud.volume_number;
         ud.m_player_skill = ud.player_skill;

                 //Fake read because lseek won't work with compression
     walock[MAXTILES-3] = 1;
     if (waloff[MAXTILES-3] == 0) allocache(&waloff[MAXTILES-3],160*100,&walock[MAXTILES-3]);
     tilesizx[MAXTILES-3] = 100; tilesizy[MAXTILES-3] = 160;
     kdfread((char *)waloff[MAXTILES-3],160,100,fil);

     if(music_changed == 0)
        music_select = (ud.volume_number*11) + ud.level_number;
     playmusic(&music_fn[0][music_select][0]);

         kdfread(&numwalls,2,1,fil);
     kdfread(&wall[0],sizeof(walltype),MAXWALLS,fil);
         kdfread(&numsectors,2,1,fil);
     kdfread(&sector[0],sizeof(sectortype),MAXSECTORS,fil);
         kdfread(&sprite[0],sizeof(spritetype),MAXSPRITES,fil);
         kdfread(&headspritesect[0],2,MAXSECTORS+1,fil);
         kdfread(&prevspritesect[0],2,MAXSPRITES,fil);
         kdfread(&nextspritesect[0],2,MAXSPRITES,fil);
         kdfread(&headspritestat[0],2,MAXSTATUS+1,fil);
         kdfread(&prevspritestat[0],2,MAXSPRITES,fil);
         kdfread(&nextspritestat[0],2,MAXSPRITES,fil);
         kdfread(&numcyclers,sizeof(numcyclers),1,fil);
         kdfread(&cyclers[0][0],12,MAXCYCLERS,fil);
     kdfread(ps,sizeof(ps),1,fil);
     kdfread(po,sizeof(po),1,fil);
         kdfread(&numanimwalls,sizeof(numanimwalls),1,fil);
         kdfread(&animwall,sizeof(animwall),1,fil);
         kdfread(&msx[0],sizeof(long),sizeof(msx)/sizeof(long),fil);
         kdfread(&msy[0],sizeof(long),sizeof(msy)/sizeof(long),fil);
     kdfread((short *)&spriteqloc,sizeof(short),1,fil);
     //kdfread((short *)&spriteqamount,sizeof(short),1,fil);
     kdfread((short *)&spriteq[0],sizeof(short),spriteqamount,fil);
         kdfread(&mirrorcnt,sizeof(short),1,fil);
         kdfread(&mirrorwall[0],sizeof(short),64,fil);
     kdfread(&mirrorsector[0],sizeof(short),64,fil);
     kdfread(&show2dsector[0],sizeof(char),MAXSECTORS>>3,fil);
     kdfread(&actortype[0],sizeof(char),MAXTILES,fil);
     kdfread(&boardfilename[0],sizeof(boardfilename),1,fil);

     kdfread(&scriptptrs[0],1,MAXSCRIPTSIZE,fil);
     kdfread(&script[0],4,MAXSCRIPTSIZE,fil);
     for(i=0;i<MAXSCRIPTSIZE;i++)
        if( scriptptrs[i] )
     {
         j = (long)script[i]+(long)&script[0];
         script[i] = j;
     }

     kdfread(&actorscrptr[0],4,MAXTILES,fil);
     for(i=0;i<MAXTILES;i++)
         if(actorscrptr[i])
     {
        j = (long)actorscrptr[i]+(long)&script[0];
        actorscrptr[i] = (long *)j;
     }

     kdfread(&scriptptrs[0],1,MAXSPRITES,fil);
     kdfread(&hittype[0],sizeof(struct weaponhit),MAXSPRITES,fil);

     for(i=0;i<MAXSPRITES;i++)
     {
        j = (long)(&script[0]);
        if( scriptptrs[i]&1 ) T2 += j;
        if( scriptptrs[i]&2 ) T5 += j;
        if( scriptptrs[i]&4 ) T6 += j;
     }

         kdfread(&lockclock,sizeof(lockclock),1,fil);
     kdfread(&pskybits,sizeof(pskybits),1,fil);
     kdfread(&pskyoff[0],sizeof(pskyoff[0]),MAXPSKYTILES,fil);

         kdfread(&animatecnt,sizeof(animatecnt),1,fil);
         kdfread(&animatesect[0],2,MAXANIMATES,fil);
         kdfread(&animateptr[0],4,MAXANIMATES,fil);
     for(i = animatecnt-1;i>=0;i--) animateptr[i] = (long *)((long)animateptr[i]+(long)(&sector[0]));
         kdfread(&animategoal[0],4,MAXANIMATES,fil);
         kdfread(&animatevel[0],4,MAXANIMATES,fil);

         kdfread(&earthquaketime,sizeof(earthquaketime),1,fil);
     kdfread(&ud.from_bonus,sizeof(ud.from_bonus),1,fil);
     kdfread(&ud.secretlevel,sizeof(ud.secretlevel),1,fil);
     kdfread(&ud.respawn_monsters,sizeof(ud.respawn_monsters),1,fil);
     ud.m_respawn_monsters = ud.respawn_monsters;
     kdfread(&ud.respawn_items,sizeof(ud.respawn_items),1,fil);
     ud.m_respawn_items = ud.respawn_items;
     kdfread(&ud.respawn_inventory,sizeof(ud.respawn_inventory),1,fil);
     ud.m_respawn_inventory = ud.respawn_inventory;

     kdfread(&ud.god,sizeof(ud.god),1,fil);
     kdfread(&ud.auto_run,sizeof(ud.auto_run),1,fil);
     kdfread(&ud.crosshair,sizeof(ud.crosshair),1,fil);
     kdfread(&ud.monsters_off,sizeof(ud.monsters_off),1,fil);
     ud.m_monsters_off = ud.monsters_off;
     kdfread(&ud.last_level,sizeof(ud.last_level),1,fil);
     kdfread(&ud.eog,sizeof(ud.eog),1,fil);

     kdfread(&ud.coop,sizeof(ud.coop),1,fil);
     ud.m_coop = ud.coop;
     kdfread(&ud.marker,sizeof(ud.marker),1,fil);
     ud.m_marker = ud.marker;
     kdfread(&ud.ffire,sizeof(ud.ffire),1,fil);
     ud.m_ffire = ud.ffire;

     kdfread(&camsprite,sizeof(camsprite),1,fil);
     kdfread(&connecthead,sizeof(connecthead),1,fil);
     kdfread(connectpoint2,sizeof(connectpoint2),1,fil);
     kdfread(&numplayersprites,sizeof(numplayersprites),1,fil);
     kdfread((short *)&frags[0][0],sizeof(frags),1,fil);
     kdfread(&screenpeek,sizeof(screenpeek),1,fil);

     kdfread(&randomseed,sizeof(randomseed),1,fil);
     kdfread(&global_random,sizeof(global_random),1,fil);
     kdfread(&parallaxyscale,sizeof(parallaxyscale),1,fil);

     kdfread(&jaildoorsecthtag[0],sizeof(jaildoorsecthtag[0]),32,fil);
     kdfread(&jaildoorsect[0],sizeof(jaildoorsect[0]),32,fil);
     kdfread(&jaildooropen[0],sizeof(jaildooropen[0]),32,fil);
     kdfread(&jaildoordir[0],sizeof(jaildoordir[0]),32,fil);
     kdfread(&jaildoordrag[0],sizeof(jaildoordrag[0]),32,fil);
     kdfread(&jaildoordist[0],sizeof(jaildoordist[0]),32,fil);
     kdfread(&jaildoorspeed[0],sizeof(jaildoorspeed[0]),32,fil);
     kdfread(&jaildoorsound[0],sizeof(jaildoorsound[0]),32,fil);
     kdfread(&jaildoorcnt,sizeof(jaildoorcnt),1,fil);
     kdfread(&shadedsector[0],sizeof(shadedsector[0]),MAXSECTORS,fil);
     kdfread(&minecartsect[0],sizeof(minecartsect[0]),16,fil);
     kdfread(&minecartchildsect[0],sizeof(minecartchildsect[0]),16,fil);
     kdfread(&minecartopen[0],sizeof(minecartopen[0]),16,fil);
     kdfread(&minecartdir[0],sizeof(minecartdir[0]),16,fil);
     kdfread(&minecartdrag[0],sizeof(minecartdrag[0]),16,fil);
     kdfread(&minecartdist[0],sizeof(minecartdist[0]),16,fil);
     kdfread(&minecartspeed[0],sizeof(minecartspeed[0]),16,fil);
     kdfread(&minecartsound[0],sizeof(minecartsound[0]),16,fil);
     kdfread(&minecartcnt,sizeof(minecartcnt),1,fil);
     kdfread(&ambientfx,sizeof(ambientfx),1,fil);
     kdfread(&ambienthitag[0],sizeof(ambienthitag[0]),64,fil);
     kdfread(&ambientlotag[0],sizeof(ambientlotag[0]),64,fil);
     kdfread(&ambientsprite[0],sizeof(ambientsprite[0]),64,fil);
     kdfread(&ufospawn,sizeof(ufospawn),1,fil);
     kdfread(&ufocnt,sizeof(ufocnt),1,fil);
     kdfread(&hulkspawn,sizeof(hulkspawn),1,fil);
     kdfread(&lastlevel,sizeof(lastlevel),1,fil);
     kdfread(&lastlevel,sizeof(lastlevel),1,fil);
     kdfread(&torchsector[0],sizeof(torchsector[0]),64,fil);
     kdfread(&torchsectorshade[0],sizeof(torchsectorshade[0]),64,fil);
     kdfread(&torchtype[0],sizeof(torchtype[0]),64,fil);
     kdfread(&torchcnt,sizeof(torchcnt),1,fil);
     kdfread(&geosector[0],sizeof(geosector[0]),64,fil);
     kdfread(&geosectorwarp[0],sizeof(geosectorwarp[0]),64,fil);
     kdfread(&geox[0],sizeof(geox[0]),64,fil);
     kdfread(&geoy[0],sizeof(geoy[0]),64,fil);
     kdfread(&geoz[0],sizeof(geoz[0]),64,fil);
     kdfread(&geosectorwarp2[0],sizeof(geosectorwarp2[0]),64,fil);
     kdfread(&geox2[0],sizeof(geox2[0]),64,fil);
     kdfread(&geoy2[0],sizeof(geoy2[0]),64,fil);
     kdfread(&geoz2[0],sizeof(geoz2[0]),64,fil);
     kdfread(&geocnt,sizeof(geocnt),1,fil);
#ifdef RRRA
     kdfread(&WindTime,4,1,fil);
     kdfread(&WindDir,4,1,fil);
     kdfread(&word_119BD8,2,1,fil);
     kdfread(&word_119BDA,2,1,fil);
     if (ps[myconnectindex].fogtype > 1)
         sub_86730(ps[myconnectindex].fogtype);
     else if (ps[myconnectindex].fogtype == 0)
         sub_86730(0);
#else
     tilesizx[0] = tilesizy[0] = 0;
#endif

     kclose(fil);

     restorepalette = 1;
     if(ps[myconnectindex].over_shoulder_on != 0)
     {
         cameradist = 0;
         cameraclock = 0;
         ps[myconnectindex].over_shoulder_on = 1;
     }

     clearbufbyte(gotpic,sizeof(gotpic),0L);
         cacheit();
     docacheit();

     ps[myconnectindex].gm = MODE_GAME;
         ud.recstat = 0;

     setpal(&ps[myconnectindex]);

     FX_SetReverb(0);

     if(ud.lockout == 0)
     {
         for(x=0;x<numanimwalls;x++)
             if( wall[animwall[x].wallnum].extra >= 0 )
                 wall[animwall[x].wallnum].picnum = wall[animwall[x].wallnum].extra;
     }
     else
     {
         for(x=0;x<numanimwalls;x++)
             switch(wall[animwall[x].wallnum].picnum)
         {
             default:
                 break;
         }
     }

     numinterpolations = 0;
     startofdynamicinterpolations = 0;

     k = headspritestat[3];
     while(k >= 0)
     {
        switch(sprite[k].lotag)
        {
            case 31:
                setinterpolation(&sector[sprite[k].sectnum].floorz);
                break;
            case 32:
                setinterpolation(&sector[sprite[k].sectnum].ceilingz);
                break;
            case 25:
                setinterpolation(&sector[sprite[k].sectnum].floorz);
                setinterpolation(&sector[sprite[k].sectnum].ceilingz);
                break;
            case 17:
                setinterpolation(&sector[sprite[k].sectnum].floorz);
                setinterpolation(&sector[sprite[k].sectnum].ceilingz);
                break;
            case 0:
            case 5:
            case 6:
            case 11:
            case 14:
            case 15:
            case 16:
            case 26:
            case 30:
                setsectinterpolate(k);
                break;
        }

        k = nextspritestat[k];
     }

     for(i = animatecnt-1;i>=0;i--)
         setinterpolation(animateptr[i]);

     show_shareware = 0;

     if (ps[myconnectindex].one_parallax_sectnum >= 0)
         setupbackdrop(sector[ps[myconnectindex].one_parallax_sectnum].ceilingpicnum);

     clearfifo();
     flushpackets();
     resettimevars();

     return(0);
}

saveplayer(signed char spot)
{
     long i, j;
         char *fn = "game0.sav";
     char *mpfn = "gameA_00.sav";
     char *fnptr,scriptptrs[MAXSCRIPTSIZE];
         FILE *fil;
     long bv = BYTEVERSION;

     if(spot < 0)
     {
        multiflag = 1;
        multiwhat = 1;
        multipos = -spot-1;
        return -1;
     }

     waitforeverybody();

     if( multiflag == 2 && multiwho != myconnectindex )
     {
         fnptr = mpfn;
         mpfn[4] = spot + 'A';

         if(ud.multimode > 9)
         {
             mpfn[6] = (multiwho/10) + '0';
             mpfn[7] = multiwho + '0';
         }
         else mpfn[7] = multiwho + '0';
     }
     else
     {
        fnptr = fn;
        fn[4] = spot + '0';
     }

     if ((fil = fopen(fnptr,"wb")) == 0) return(-1);

     ready2send = 0;

     dfwrite(&bv,4,1,fil);
     dfwrite(&ud.multimode,sizeof(ud.multimode),1,fil);

         dfwrite(&ud.savegame[spot][0],19,1,fil);
         dfwrite(&ud.volume_number,sizeof(ud.volume_number),1,fil);
     dfwrite(&ud.level_number,sizeof(ud.level_number),1,fil);
         dfwrite(&ud.player_skill,sizeof(ud.player_skill),1,fil);
     dfwrite((char *)waloff[MAXTILES-1],160,100,fil);

         dfwrite(&numwalls,2,1,fil);
     dfwrite(&wall[0],sizeof(walltype),MAXWALLS,fil);
         dfwrite(&numsectors,2,1,fil);
     dfwrite(&sector[0],sizeof(sectortype),MAXSECTORS,fil);
         dfwrite(&sprite[0],sizeof(spritetype),MAXSPRITES,fil);
         dfwrite(&headspritesect[0],2,MAXSECTORS+1,fil);
         dfwrite(&prevspritesect[0],2,MAXSPRITES,fil);
         dfwrite(&nextspritesect[0],2,MAXSPRITES,fil);
         dfwrite(&headspritestat[0],2,MAXSTATUS+1,fil);
         dfwrite(&prevspritestat[0],2,MAXSPRITES,fil);
         dfwrite(&nextspritestat[0],2,MAXSPRITES,fil);
         dfwrite(&numcyclers,sizeof(numcyclers),1,fil);
         dfwrite(&cyclers[0][0],12,MAXCYCLERS,fil);
     dfwrite(ps,sizeof(ps),1,fil);
     dfwrite(po,sizeof(po),1,fil);
         dfwrite(&numanimwalls,sizeof(numanimwalls),1,fil);
         dfwrite(&animwall,sizeof(animwall),1,fil);
         dfwrite(&msx[0],sizeof(long),sizeof(msx)/sizeof(long),fil);
         dfwrite(&msy[0],sizeof(long),sizeof(msy)/sizeof(long),fil);
     dfwrite(&spriteqloc,sizeof(short),1,fil);
    // dfwrite(&spriteqamount,sizeof(short),1,fil);
     dfwrite(&spriteq[0],sizeof(short),spriteqamount,fil);
         dfwrite(&mirrorcnt,sizeof(short),1,fil);
         dfwrite(&mirrorwall[0],sizeof(short),64,fil);
         dfwrite(&mirrorsector[0],sizeof(short),64,fil);
     dfwrite(&show2dsector[0],sizeof(char),MAXSECTORS>>3,fil);
     dfwrite(&actortype[0],sizeof(char),MAXTILES,fil);
     dfwrite(&boardfilename[0],sizeof(boardfilename),1,fil);

     for(i=0;i<MAXSCRIPTSIZE;i++)
     {
          if( (long)script[i] >= (long)(&script[0]) && (long)script[i] < (long)(&script[MAXSCRIPTSIZE]) )
          {
                scriptptrs[i] = 1;
                j = (long)script[i] - (long)&script[0];
                script[i] = j;
          }
          else scriptptrs[i] = 0;
     }

     dfwrite(&scriptptrs[0],1,MAXSCRIPTSIZE,fil);
     dfwrite(&script[0],4,MAXSCRIPTSIZE,fil);

     for(i=0;i<MAXSCRIPTSIZE;i++)
        if( scriptptrs[i] )
     {
        j = script[i]+(long)&script[0];
        script[i] = j;
     }

     for(i=0;i<MAXTILES;i++)
         if(actorscrptr[i])
     {
        j = (long)actorscrptr[i]-(long)&script[0];
        actorscrptr[i] = (long *)j;
     }
     dfwrite(&actorscrptr[0],4,MAXTILES,fil);
     for(i=0;i<MAXTILES;i++)
         if(actorscrptr[i])
     {
         j = (long)actorscrptr[i]+(long)&script[0];
         actorscrptr[i] = (long *)j;
     }

     for(i=0;i<MAXSPRITES;i++)
     {
        scriptptrs[i] = 0;

        if(actorscrptr[PN] == 0) continue;

        j = (long)&script[0];

        if(T2 >= j && T2 < (long)(&script[MAXSCRIPTSIZE]) )
        {
            scriptptrs[i] |= 1;
            T2 -= j;
        }
        if(T5 >= j && T5 < (long)(&script[MAXSCRIPTSIZE]) )
        {
            scriptptrs[i] |= 2;
            T5 -= j;
        }
        if(T6 >= j && T6 < (long)(&script[MAXSCRIPTSIZE]) )
        {
            scriptptrs[i] |= 4;
            T6 -= j;
        }
    }

    dfwrite(&scriptptrs[0],1,MAXSPRITES,fil);
    dfwrite(&hittype[0],sizeof(struct weaponhit),MAXSPRITES,fil);

    for(i=0;i<MAXSPRITES;i++)
    {
        if(actorscrptr[PN] == 0) continue;
        j = (long)&script[0];

        if(scriptptrs[i]&1)
            T2 += j;
        if(scriptptrs[i]&2)
            T5 += j;
        if(scriptptrs[i]&4)
            T6 += j;
    }

         dfwrite(&lockclock,sizeof(lockclock),1,fil);
     dfwrite(&pskybits,sizeof(pskybits),1,fil);
     dfwrite(&pskyoff[0],sizeof(pskyoff[0]),MAXPSKYTILES,fil);
         dfwrite(&animatecnt,sizeof(animatecnt),1,fil);
         dfwrite(&animatesect[0],2,MAXANIMATES,fil);
         for(i = animatecnt-1;i>=0;i--) animateptr[i] = (long *)((long)animateptr[i]-(long)(&sector[0]));
         dfwrite(&animateptr[0],4,MAXANIMATES,fil);
         for(i = animatecnt-1;i>=0;i--) animateptr[i] = (long *)((long)animateptr[i]+(long)(&sector[0]));
         dfwrite(&animategoal[0],4,MAXANIMATES,fil);
         dfwrite(&animatevel[0],4,MAXANIMATES,fil);

         dfwrite(&earthquaketime,sizeof(earthquaketime),1,fil);
         dfwrite(&ud.from_bonus,sizeof(ud.from_bonus),1,fil);
     dfwrite(&ud.secretlevel,sizeof(ud.secretlevel),1,fil);
     dfwrite(&ud.respawn_monsters,sizeof(ud.respawn_monsters),1,fil);
     dfwrite(&ud.respawn_items,sizeof(ud.respawn_items),1,fil);
     dfwrite(&ud.respawn_inventory,sizeof(ud.respawn_inventory),1,fil);
     dfwrite(&ud.god,sizeof(ud.god),1,fil);
     dfwrite(&ud.auto_run,sizeof(ud.auto_run),1,fil);
     dfwrite(&ud.crosshair,sizeof(ud.crosshair),1,fil);
     dfwrite(&ud.monsters_off,sizeof(ud.monsters_off),1,fil);
     dfwrite(&ud.last_level,sizeof(ud.last_level),1,fil);
     dfwrite(&ud.eog,sizeof(ud.eog),1,fil);
     dfwrite(&ud.coop,sizeof(ud.coop),1,fil);
     dfwrite(&ud.marker,sizeof(ud.marker),1,fil);
     dfwrite(&ud.ffire,sizeof(ud.ffire),1,fil);
     dfwrite(&camsprite,sizeof(camsprite),1,fil);
     dfwrite(&connecthead,sizeof(connecthead),1,fil);
     dfwrite(connectpoint2,sizeof(connectpoint2),1,fil);
     dfwrite(&numplayersprites,sizeof(numplayersprites),1,fil);
     dfwrite((short *)&frags[0][0],sizeof(frags),1,fil);
     dfwrite(&screenpeek,sizeof(screenpeek),1,fil);

     dfwrite(&randomseed,sizeof(randomseed),1,fil);
     dfwrite(&global_random,sizeof(global_random),1,fil);
     dfwrite(&parallaxyscale,sizeof(parallaxyscale),1,fil);

     dfwrite(&jaildoorsecthtag[0],sizeof(jaildoorsecthtag[0]),32,fil);
     dfwrite(&jaildoorsect[0],sizeof(jaildoorsect[0]),32,fil);
     dfwrite(&jaildooropen[0],sizeof(jaildooropen[0]),32,fil);
     dfwrite(&jaildoordir[0],sizeof(jaildoordir[0]),32,fil);
     dfwrite(&jaildoordrag[0],sizeof(jaildoordrag[0]),32,fil);
     dfwrite(&jaildoordist[0],sizeof(jaildoordist[0]),32,fil);
     dfwrite(&jaildoorspeed[0],sizeof(jaildoorspeed[0]),32,fil);
     dfwrite(&jaildoorsound[0],sizeof(jaildoorsound[0]),32,fil);
     dfwrite(&jaildoorcnt,sizeof(jaildoorcnt),1,fil);
     dfwrite(&shadedsector[0],sizeof(shadedsector[0]),MAXSECTORS,fil);
     dfwrite(&minecartsect[0],sizeof(minecartsect[0]),16,fil);
     dfwrite(&minecartchildsect[0],sizeof(minecartchildsect[0]),16,fil);
     dfwrite(&minecartopen[0],sizeof(minecartopen[0]),16,fil);
     dfwrite(&minecartdir[0],sizeof(minecartdir[0]),16,fil);
     dfwrite(&minecartdrag[0],sizeof(minecartdrag[0]),16,fil);
     dfwrite(&minecartdist[0],sizeof(minecartdist[0]),16,fil);
     dfwrite(&minecartspeed[0],sizeof(minecartspeed[0]),16,fil);
     dfwrite(&minecartsound[0],sizeof(minecartsound[0]),16,fil);
     dfwrite(&minecartcnt,sizeof(minecartcnt),1,fil);
     dfwrite(&ambientfx,sizeof(ambientfx),1,fil);
     dfwrite(&ambienthitag[0],sizeof(ambienthitag[0]),64,fil);
     dfwrite(&ambientlotag[0],sizeof(ambientlotag[0]),64,fil);
     dfwrite(&ambientsprite[0],sizeof(ambientsprite[0]),64,fil);
     dfwrite(&ufospawn,sizeof(ufospawn),1,fil);
     dfwrite(&ufocnt,sizeof(ufocnt),1,fil);
     dfwrite(&hulkspawn,sizeof(hulkspawn),1,fil);
     dfwrite(&lastlevel,sizeof(lastlevel),1,fil);
     dfwrite(&lastlevel,sizeof(lastlevel),1,fil);
     dfwrite(&torchsector[0],sizeof(torchsector[0]),64,fil);
     dfwrite(&torchsectorshade[0],sizeof(torchsectorshade[0]),64,fil);
     dfwrite(&torchtype[0],sizeof(torchtype[0]),64,fil);
     dfwrite(&torchcnt,sizeof(torchcnt),1,fil);
     dfwrite(&geosector[0],sizeof(geosector[0]),64,fil);
     dfwrite(&geosectorwarp[0],sizeof(geosectorwarp[0]),64,fil);
     dfwrite(&geox[0],sizeof(geox[0]),64,fil);
     dfwrite(&geoy[0],sizeof(geoy[0]),64,fil);
     dfwrite(&geoz[0],sizeof(geoz[0]),64,fil);
     dfwrite(&geosectorwarp2[0],sizeof(geosectorwarp2[0]),64,fil);
     dfwrite(&geox2[0],sizeof(geox2[0]),64,fil);
     dfwrite(&geoy2[0],sizeof(geoy2[0]),64,fil);
     dfwrite(&geoz2[0],sizeof(geoz2[0]),64,fil);
     dfwrite(&geocnt,sizeof(geocnt),1,fil);
#ifdef RRRA
     dfwrite(&WindTime,4,1,fil);
     dfwrite(&WindDir,4,1,fil);
     dfwrite(&word_119BD8,2,1,fil);
     dfwrite(&word_119BDA,2,1,fil);
#endif

         fclose(fil);

     if(ud.multimode < 2)
     {
         strcpy(&fta_quotes[122],"GAME SAVED");
         FTA(122,&ps[myconnectindex]);
     }

     ready2send = 1;

     ototalclock = totalclock;

     return(0);
}

#define LMB (buttonstat&1)
#define RMB (buttonstat&2)

ControlInfo minfo;

long mi;

int probe(int x,int y,int i,int n, int t)
{
    short centre, s;

    s = 1+(CONTROL_GetMouseSensitivity()>>4);

    if( ControllerType == 1 && CONTROL_MousePresent )
    {
        CONTROL_GetInput( &minfo );
        mi += minfo.dz;
    }

    else minfo.dz = minfo.dyaw = 0;

    if( x == (320>>1) )
        centre = 320>>2;
    else centre = 0;

    if(!buttonstat)
    {
        if( KB_KeyPressed( sc_UpArrow ) || KB_KeyPressed( sc_PgUp ) || KB_KeyPressed( sc_kpad_8 ) ||
            mi < -8192 )
        {
            mi = 0;
            KB_ClearKeyDown( sc_UpArrow );
            KB_ClearKeyDown( sc_kpad_8 );
            KB_ClearKeyDown( sc_PgUp );
            sound(335);

            probey--;
            if(probey < 0) probey = n-1;
            minfo.dz = 0;
        }
        if( KB_KeyPressed( sc_DownArrow ) || KB_KeyPressed( sc_PgDn ) || KB_KeyPressed( sc_kpad_2 )
            || mi > 8192 )
        {
            mi = 0;
            KB_ClearKeyDown( sc_DownArrow );
            KB_ClearKeyDown( sc_kpad_2 );
            KB_ClearKeyDown( sc_PgDn );
            sound(335);
            probey++;
            minfo.dz = 0;
        }
        if (t == 3)
        {
            if (KB_KeyPressed(sc_LeftArrow) || KB_KeyPressed(sc_kpad_4))
            {
                KB_ClearKeyDown(sc_LeftArrow);
                KB_ClearKeyDown(sc_kpad_4);
                sound(335);

                probey--;
                if(probey < 0) probey = n-1;
                minfo.dz = 0;
            }
            else if (KB_KeyPressed(sc_RightArrow) || KB_KeyPressed(sc_kpad_6))
            {
                KB_ClearKeyDown(sc_RightArrow);
                KB_ClearKeyDown(sc_kpad_6);
                sound(335);

                probey++;
                minfo.dz = 0;
            }
            else if (KB_KeyPressed(sc_1))
            {
                KB_ClearKeyDown(sc_1);
                sound(335);

                probey = 0;
            }
            else if (KB_KeyPressed(sc_2))
            {
                KB_ClearKeyDown(sc_2);
                sound(335);

                probey = 1;
            }
            else if (KB_KeyPressed(sc_3))
            {
                KB_ClearKeyDown(sc_3);
                sound(335);

                probey = 2;
            }
            else if (KB_KeyPressed(sc_4))
            {
                KB_ClearKeyDown(sc_4);
                sound(335);

                probey = 3;
            }
            else if (KB_KeyPressed(sc_5))
            {
                KB_ClearKeyDown(sc_5);
                sound(335);

                probey = 4;
            }
            else if (KB_KeyPressed(sc_6))
            {
                KB_ClearKeyDown(sc_6);
                sound(335);

                probey = 5;
            }
            else if (KB_KeyPressed(sc_7))
            {
                KB_ClearKeyDown(sc_7);
                sound(335);

                probey = 6;
            }
            else if (KB_KeyPressed(sc_8))
            {
                KB_ClearKeyDown(sc_8);
                sound(335);

                probey = 7;
            }
        }
    }

    if(probey >= n)
        probey = 0;

    if(centre)
    {
//        rotatesprite(((320>>1)+(centre)+54)<<16,(y+(probey*i)-4)<<16,65536L,0,SPINNINGNUKEICON+6-((6+(totalclock>>3))%7),sh,0,10,0,0,xdim-1,ydim-1);
//        rotatesprite(((320>>1)-(centre)-54)<<16,(y+(probey*i)-4)<<16,65536L,0,SPINNINGNUKEICON+((totalclock>>3)%7),sh,0,10,0,0,xdim-1,ydim-1);

        rotatesprite(((320>>1)+(centre>>1)+70)<<16,(y+(probey*i)-4)<<16,13107L,0,SPINNINGNUKEICON+15-((15+(totalclock>>3))%16),sh,0,10,0,0,xdim-1,ydim-1);
        rotatesprite(((320>>1)-(centre>>1)-70)<<16,(y+(probey*i)-4)<<16,13107L,0,SPINNINGNUKEICON+((totalclock>>3)%16),sh,0,10,0,0,xdim-1,ydim-1);
    }
    else
    {
        switch (t)
        {
        case 1:
        case 2:
            rotatesprite((x-tilesizx[BIGFNTCURSOR]-4)<<16,(y+(probey*i)-4)<<16,13107L,0,SPINNINGNUKEICON+(((totalclock>>3))%16),sh,0,10,0,0,xdim-1,ydim-1);
            break;
        case 3:
            break;
        default:
            rotatesprite((x-tilesizx[BIGFNTCURSOR]-4)<<16,(y+(probey*i)-4)<<16,6553L,0,SPINNINGNUKEICON+(((totalclock>>3))%16),sh,0,10,0,0,xdim-1,ydim-1);
            break;
        }
    }

    if( KB_KeyPressed( sc_kpad_Enter ) || KB_KeyPressed( sc_Enter ) || (LMB && !onbar) )
    {
        if(current_menu != 110)
            sound(341);
        KB_ClearKeyDown( sc_Enter );
        KB_ClearKeyDown( sc_kpad_Enter );
        return(probey);
    }
    else if( KB_KeyPressed( sc_Escape ) || (RMB) )
    {
        onbar = 0;
        KB_ClearKeyDown( sc_Escape );
        sound(243);
        return(-1);
    }
    else
    {
        if(onbar == 0) return(-probey-2);
        if ( KB_KeyPressed( sc_LeftArrow ) || KB_KeyPressed( sc_kpad_4 ) || ((buttonstat&1) && minfo.dyaw < -128 ) )
            return(probey);
        else if ( KB_KeyPressed( sc_RightArrow ) || KB_KeyPressed( sc_kpad_6 ) || ((buttonstat&1) && minfo.dyaw > 128 ) )
            return(probey);
        else return(-probey-2);
    }
}

int menutext(int x,int y,short s,short p,char *t)
{
    short i, ac, centre;

    y -= 12;

    i = centre = 0;

    if( x == (320>>1) )
    {
        while( *(t+i) )
        {
            if(*(t+i) == ' ')
            {
                centre += 5;
                i++;
                continue;
            }
            ac = 0;
            if(*(t+i) >= '0' && *(t+i) <= '9')
                ac = *(t+i) - '0' + BIGALPHANUM-10;
            else if(*(t+i) >= 'a' && *(t+i) <= 'z')
                ac = toupper(*(t+i)) - 'A' + BIGALPHANUM;
            else if(*(t+i) >= 'A' && *(t+i) <= 'Z')
                ac = *(t+i) - 'A' + BIGALPHANUM;
            else switch(*(t+i))
            {
                case '.':
                    ac = BIGPERIOD;
                    break;
                case '\'':
                    ac = BIGAPPOS;
                    break;
                case ',':
                    ac = BIGCOMMA;
                    break;
                case '!':
                    ac = BIGX;
                    break;
                case '?':
                    ac = BIGQ;
                    break;
                case ';':
                    ac = BIGSEMI;
                    break;
                case ':':
                    ac = BIGSEMI;
                    break;
                default:
                    centre += 5;
                    i++;
                    continue;
            }

            centre += (tilesizx[ac]-1)/2;
            i++;
        }
    }

    if(centre)
        x = (320-centre-10)>>1;

    while(*t)
    {
        if(*t == ' ') {x+=5;t++;continue;}
        ac = 0;
        if(*t >= '0' && *t <= '9')
            ac = *t - '0' + BIGALPHANUM-10;
        else if(*t >= 'a' && *t <= 'z')
            ac = toupper(*t) - 'A' + BIGALPHANUM;
        else if(*t >= 'A' && *t <= 'Z')
            ac = *t - 'A' + BIGALPHANUM;
        else switch(*t)
        {
            case '.':
                ac = BIGPERIOD;
                break;
            case ',':
                ac = BIGCOMMA;
                break;
            case '!':
                ac = BIGX;
                break;
            case '\'':
                ac = BIGAPPOS;
                break;
            case '?':
                ac = BIGQ;
                break;
            case ';':
                ac = BIGSEMI;
                break;
            case ':':
                ac = BIGCOLIN;
                break;
            default:
                x += 5;
                t++;
                continue;
        }

        rotatesprite(x<<16,y<<16,26214L,0,ac,s,p,10+16,0,0,xdim-1,ydim-1);

        x += tilesizx[ac]/2;
        t++;
    }
    return (x);
}

int endlvlmenutext(int x,int y,short s,short p,char *t)
{
    short i, ac, centre;

    y -= 12;

    i = centre = 0;

    if( x == (320>>1) )
    {
        while( *(t+i) )
        {
            if(*(t+i) == ' ')
            {
                centre += 5;
                i++;
                continue;
            }
            ac = 0;
            if(*(t+i) >= '0' && *(t+i) <= '9')
                ac = *(t+i) - '0' + BIGALPHANUM-10;
            else if(*(t+i) >= 'a' && *(t+i) <= 'z')
                ac = toupper(*(t+i)) - 'A' + BIGALPHANUM;
            else if(*(t+i) >= 'A' && *(t+i) <= 'Z')
                ac = *(t+i) - 'A' + BIGALPHANUM;
            else switch(*(t+i))
            {
                case '.':
                    ac = BIGPERIOD;
                    break;
                case '\'':
                    ac = BIGAPPOS;
                    break;
                case ',':
                    ac = BIGCOMMA;
                    break;
                case '!':
                    ac = BIGX;
                    break;
                case '?':
                    ac = BIGQ;
                    break;
                case ';':
                    ac = BIGSEMI;
                    break;
                case ':':
                    ac = BIGSEMI;
                    break;
                default:
                    centre += 5;
                    i++;
                    continue;
            }

            centre += (tilesizx[ac]-1)/2;
            i++;
        }
    }

    if(centre)
        x = (320-centre-10)>>1;

    while(*t)
    {
        if(*t == ' ') {x+=5;t++;continue;}
        ac = 0;
        if(*t >= '0' && *t <= '9')
            ac = *t - '0' + BIGALPHANUM-10;
        else if(*t >= 'a' && *t <= 'z')
            ac = toupper(*t) - 'A' + BIGALPHANUM;
        else if(*t >= 'A' && *t <= 'Z')
            ac = *t - 'A' + BIGALPHANUM;
        else switch(*t)
        {
            case '.':
                ac = BIGPERIOD;
                break;
            case ',':
                ac = BIGCOMMA;
                break;
            case '!':
                ac = BIGX;
                break;
            case '\'':
                ac = BIGAPPOS;
                break;
            case '?':
                ac = BIGQ;
                break;
            case ';':
                ac = BIGSEMI;
                break;
            case ':':
                ac = BIGCOLIN;
                break;
            default:
                x += 5;
                t++;
                continue;
        }

        rotatesprite(x<<16,y<<16,26214L,0,ac,s,p,10+16,0,0,xdim-1,ydim-1);

        x += tilesizx[ac]/2;
        t++;
    }
    return (x);
}

int menutextc(int x,int y,short s,short p,char *t)
{
    short i, ac, centre;

    s += 8;
    y -= 12;

    i = centre = 0;

//    if( x == (320>>1) )
    {
        while( *(t+i) )
        {
            if(*(t+i) == ' ')
            {
                centre += 5;
                i++;
                continue;
            }
            ac = 0;
            if(*(t+i) >= '0' && *(t+i) <= '9')
                ac = *(t+i) - '0' + BIGALPHANUM+26+26;
            if(*(t+i) >= 'a' && *(t+i) <= 'z')
                ac = *(t+i) - 'a' + BIGALPHANUM+26;
            if(*(t+i) >= 'A' && *(t+i) <= 'Z')
                ac = *(t+i) - 'A' + BIGALPHANUM;

            else switch(*t)
            {
                case '.':
                    ac = BIGPERIOD;
                    break;
                case ',':
                    ac = BIGCOMMA;
                    break;
                case '!':
                    ac = BIGX;
                    break;
                case '?':
                    ac = BIGQ;
                    break;
                case ';':
                    ac = BIGSEMI;
                    break;
                case ':':
                    ac = BIGCOLIN;
                    break;
            }

            centre += tilesizx[ac]-1;
            i++;
        }
    }

    x -= centre>>1;

    while(*t)
    {
        if(*t == ' ') {x+=5;t++;continue;}
        ac = 0;
        if(*t >= '0' && *t <= '9')
            ac = *t - '0' + BIGALPHANUM+26+26;
        if(*t >= 'a' && *t <= 'z')
            ac = *t - 'a' + BIGALPHANUM+26;
        if(*t >= 'A' && *t <= 'Z')
            ac = *t - 'A' + BIGALPHANUM;
        switch(*t)
        {
            case '.':
                ac = BIGPERIOD;
                break;
            case ',':
                ac = BIGCOMMA;
                break;
            case '!':
                ac = BIGX;
                break;
            case '?':
                ac = BIGQ;
                break;
            case ';':
                ac = BIGSEMI;
                break;
            case ':':
                ac = BIGCOLIN;
                break;
        }

        rotatesprite(x<<16,y<<16,65536L,0,ac,s,p,10+16,0,0,xdim-1,ydim-1);

        x += tilesizx[ac];
        t++;
    }
    return (x);
}


void bar(int x,int y,short *p,short dainc,char damodify,short s, short pa)
{
    short xloc;
    char rev;

    if(dainc < 0) { dainc = -dainc; rev = 1; }
    else rev = 0;
    y-=2;

    if(damodify)
    {
        if(rev == 0)
        {
            if( KB_KeyPressed( sc_LeftArrow ) || KB_KeyPressed( sc_kpad_4 ) || ((buttonstat&1) && minfo.dyaw < -256 ) ) // && onbar) )
            {
                KB_ClearKeyDown( sc_LeftArrow );
                KB_ClearKeyDown( sc_kpad_4 );

                *p -= dainc;
                if(*p < 0)
                    *p = 0;
                sound(335);
            }
            if( KB_KeyPressed( sc_RightArrow ) || KB_KeyPressed( sc_kpad_6 ) || ((buttonstat&1) && minfo.dyaw > 256 ) )//&& onbar) )
            {
                KB_ClearKeyDown( sc_RightArrow );
                KB_ClearKeyDown( sc_kpad_6 );

                *p += dainc;
                if(*p > 63)
                    *p = 63;
                sound(335);
            }
        }
        else
        {
            if( KB_KeyPressed( sc_RightArrow ) || KB_KeyPressed( sc_kpad_6 ) || ((buttonstat&1) && minfo.dyaw > 256 ))//&& onbar ))
            {
                KB_ClearKeyDown( sc_RightArrow );
                KB_ClearKeyDown( sc_kpad_6 );

                *p -= dainc;
                if(*p < 0)
                    *p = 0;
                sound(335);
            }
            if( KB_KeyPressed( sc_LeftArrow ) || KB_KeyPressed( sc_kpad_4 ) || ((buttonstat&1) && minfo.dyaw < -256 ))// && onbar) )
            {
                KB_ClearKeyDown( sc_LeftArrow );
                KB_ClearKeyDown( sc_kpad_4 );

                *p += dainc;
                if(*p > 64)
                    *p = 64;
                sound(335);
            }
        }
    }

    xloc = *p;

    rotatesprite( (x+32)<<16,(y-3)<<16,65536L,0,SLIDEBAR,s,pa,10,0,0,xdim-1,ydim-1);
    if(rev == 0)
        rotatesprite( (x+xloc+1)<<16,(y+1)<<16,32768L,0,BIGALPHANUM-9,s,pa,10,0,0,xdim-1,ydim-1);
    else
        rotatesprite( (x+(65-xloc) )<<16,(y+1)<<16,32768L,0,BIGALPHANUM-9,s,pa,10,0,0,xdim-1,ydim-1);
}

#define SHX(X) 0
// ((x==X)*(-sh))
#define PHX(X) 0
// ((x==X)?1:2)
#define MWIN(X) rotatesprite( 320<<15,200<<15,X,0,MENUSCREEN,-16,0,10+64,0,0,xdim-1,ydim-1)
#define MWINXY(X,OX,OY) rotatesprite( ( 320+(OX) )<<15, ( 200+(OY) )<<15,X,0,MENUSCREEN,-16,0,10+64,0,0,xdim-1,ydim-1)


int32 volnum,levnum,plrskl,numplr;
short lastsavedpos = -1;

void dispnames(void)
{
    short x, c = 160;

    c += 64;
    for(x = 0;x <= 108;x += 12)
    rotatesprite((c+91-64)<<16,(x+56)<<16,65536L,0,TEXTBOX,24,0,10,0,0,xdim-1,ydim-1);

    rotatesprite(22<<16,97<<16,65536L,0,WINDOWBORDER2,24,0,10,0,0,xdim-1,ydim-1);
    rotatesprite(180<<16,97<<16,65536L,1024,WINDOWBORDER2,24,0,10,0,0,xdim-1,ydim-1);
    rotatesprite(99<<16,50<<16,65536L,512,WINDOWBORDER1,24,0,10,0,0,xdim-1,ydim-1);
    rotatesprite(103<<16,144<<16,65536L,1024+512,WINDOWBORDER1,24,0,10,0,0,xdim-1,ydim-1);

    minitext(c,48,ud.savegame[0],2,10+16);
    minitext(c,48+12,ud.savegame[1],2,10+16);
    minitext(c,48+12+12,ud.savegame[2],2,10+16);
    minitext(c,48+12+12+12,ud.savegame[3],2,10+16);
    minitext(c,48+12+12+12+12,ud.savegame[4],2,10+16);
    minitext(c,48+12+12+12+12+12,ud.savegame[5],2,10+16);
    minitext(c,48+12+12+12+12+12+12,ud.savegame[6],2,10+16);
    minitext(c,48+12+12+12+12+12+12+12,ud.savegame[7],2,10+16);
    minitext(c,48+12+12+12+12+12+12+12+12,ud.savegame[8],2,10+16);
    minitext(c,48+12+12+12+12+12+12+12+12+12,ud.savegame[9],2,10+16);

}

getfilenames(char kind[6])
{
        short type;
        struct find_t fileinfo;

        if (strcmp(kind,"SUBD") == 0)
        {
                strcpy(kind,"*.*");
                if (_dos_findfirst(kind,_A_SUBDIR,&fileinfo) != 0)
                        return(-1);
                type = 1;
        }
        else
        {
                if (_dos_findfirst(kind,_A_NORMAL,&fileinfo) != 0)
                        return(-1);
                type = 0;
        }
        do
        {
                if ((type == 0) || ((fileinfo.attrib&16) > 0))
                        if ((fileinfo.name[0] != '.') || (fileinfo.name[1] != 0))
                        {
                                strcpy(menuname[menunamecnt],fileinfo.name);
                                menuname[menunamecnt][16] = type;
                                menunamecnt++;
                        }
        }
        while (_dos_findnext(&fileinfo) == 0);

        return(0);
}

void sortfilenames()
{
        char sortbuffer[17];
        long i, j, k;

        for(i=1;i<menunamecnt;i++)
                for(j=0;j<i;j++)
                {
                         k = 0;
                         while ((menuname[i][k] == menuname[j][k]) && (menuname[i][k] != 0) && (menuname[j][k] != 0))
                                 k++;
                        if (menuname[i][k] < menuname[j][k])
                        {
                                memcpy(&sortbuffer[0],&menuname[i][0],sizeof(menuname[0]));
                                memcpy(&menuname[i][0],&menuname[j][0],sizeof(menuname[0]));
                                memcpy(&menuname[j][0],&sortbuffer[0],sizeof(menuname[0]));
                        }
                }
}
