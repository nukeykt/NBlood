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

extern char vgacompatible;

void palto(char r,char g,char b,long e)
{
    int i;
    char temparray[768];

    for(i=0;i<768;i+=3)
    {
        temparray[i  ] =
            ps[myconnectindex].palette[i+0]+((((long)r-(long)ps[myconnectindex].palette[i+0])*(long)(e&127))>>6);
        temparray[i+1] =
            ps[myconnectindex].palette[i+1]+((((long)g-(long)ps[myconnectindex].palette[i+1])*(long)(e&127))>>6);
        temparray[i+2] =
            ps[myconnectindex].palette[i+2]+((((long)b-(long)ps[myconnectindex].palette[i+2])*(long)(e&127))>>6);
    }

    if( (e&128) == 0 )
        if ((vidoption != 1) || (vgacompatible == 1)) limitrate();

    setbrightness(ud.brightness>>2,temparray);
}


void drawoverheadmap(long cposx, long cposy, long czoom, short cang)
{
        long i, j, k, l, x1, y1, x2, y2, x3, y3, x4, y4, ox, oy, xoff, yoff;
        long dax, day, cosang, sinang, xspan, yspan, sprx, spry;
        long xrepeat, yrepeat, z1, z2, startwall, endwall, tilenum, daang;
        long xvect, yvect, xvect2, yvect2;
        short p;
        char col;
        walltype *wal, *wal2;
        spritetype *spr;

        xvect = sintable[(-cang)&2047] * czoom;
        yvect = sintable[(1536-cang)&2047] * czoom;
        xvect2 = mulscale16(xvect,yxaspect);
        yvect2 = mulscale16(yvect,yxaspect);

                //Draw red lines
        for(i=0;i<numsectors;i++)
        {
                if (!(show2dsector[i>>3]&(1<<(i&7)))) continue;

                startwall = sector[i].wallptr;
                endwall = sector[i].wallptr + sector[i].wallnum;

                z1 = sector[i].ceilingz; z2 = sector[i].floorz;

                for(j=startwall,wal=&wall[startwall];j<endwall;j++,wal++)
                {
                        k = wal->nextwall; if (k < 0) continue;

                        //if ((show2dwall[j>>3]&(1<<(j&7))) == 0) continue;
                        //if ((k > j) && ((show2dwall[k>>3]&(1<<(k&7))) > 0)) continue;

                        if (sector[wal->nextsector].ceilingz == z1)
                                if (sector[wal->nextsector].floorz == z2)
                                        if (((wal->cstat|wall[wal->nextwall].cstat)&(16+32)) == 0) continue;

                        col = 139; //red
                        if ((wal->cstat|wall[wal->nextwall].cstat)&1) col = 234; //magenta

                        if (!(show2dsector[wal->nextsector>>3]&(1<<(wal->nextsector&7))))
                                col = 24;
            else continue;

                        ox = wal->x-cposx; oy = wal->y-cposy;
                        x1 = dmulscale16(ox,xvect,-oy,yvect)+(xdim<<11);
                        y1 = dmulscale16(oy,xvect2,ox,yvect2)+(ydim<<11);

                        wal2 = &wall[wal->point2];
                        ox = wal2->x-cposx; oy = wal2->y-cposy;
                        x2 = dmulscale16(ox,xvect,-oy,yvect)+(xdim<<11);
                        y2 = dmulscale16(oy,xvect2,ox,yvect2)+(ydim<<11);

                        drawline256(x1,y1,x2,y2,col);
                }
        }

                //Draw sprites
        k = ps[screenpeek].i;
        for(i=0;i<numsectors;i++)
        {
                if (!(show2dsector[i>>3]&(1<<(i&7)))) continue;
                for(j=headspritesect[i];j>=0;j=nextspritesect[j])
                        //if ((show2dsprite[j>>3]&(1<<(j&7))) > 0)
                        {
                spr = &sprite[j];

                if (j == k || (spr->cstat&0x8000) || spr->cstat == 257 || spr->xrepeat == 0) continue;

                                col = 71; //cyan
                                if (spr->cstat&1) col = 234; //magenta

                                sprx = spr->x;
                                spry = spr->y;

                if( (spr->cstat&257) != 0) switch (spr->cstat&48)
                                {
                    case 0: break;
                    case 16: break;
                    case 32:

                                                tilenum = spr->picnum;
                                                xoff = (long)((signed char)((picanm[tilenum]>>8)&255))+((long)spr->xoffset);
                                                yoff = (long)((signed char)((picanm[tilenum]>>16)&255))+((long)spr->yoffset);
                                                if ((spr->cstat&4) > 0) xoff = -xoff;
                                                if ((spr->cstat&8) > 0) yoff = -yoff;

                                                k = spr->ang;
                                                cosang = sintable[(k+512)&2047]; sinang = sintable[k];
                                                xspan = tilesizx[tilenum]; xrepeat = spr->xrepeat;
                                                yspan = tilesizy[tilenum]; yrepeat = spr->yrepeat;

                                                dax = ((xspan>>1)+xoff)*xrepeat; day = ((yspan>>1)+yoff)*yrepeat;
                                                x1 = sprx + dmulscale16(sinang,dax,cosang,day);
                                                y1 = spry + dmulscale16(sinang,day,-cosang,dax);
                                                l = xspan*xrepeat;
                                                x2 = x1 - mulscale16(sinang,l);
                                                y2 = y1 + mulscale16(cosang,l);
                                                l = yspan*yrepeat;
                                                k = -mulscale16(cosang,l); x3 = x2+k; x4 = x1+k;
                                                k = -mulscale16(sinang,l); y3 = y2+k; y4 = y1+k;

                                                ox = x1-cposx; oy = y1-cposy;
                                                x1 = dmulscale16(ox,xvect,-oy,yvect);
                                                y1 = dmulscale16(oy,xvect2,ox,yvect2);

                                                ox = x2-cposx; oy = y2-cposy;
                                                x2 = dmulscale16(ox,xvect,-oy,yvect);
                                                y2 = dmulscale16(oy,xvect2,ox,yvect2);

                                                ox = x3-cposx; oy = y3-cposy;
                                                x3 = dmulscale16(ox,xvect,-oy,yvect);
                                                y3 = dmulscale16(oy,xvect2,ox,yvect2);

                                                ox = x4-cposx; oy = y4-cposy;
                                                x4 = dmulscale16(ox,xvect,-oy,yvect);
                                                y4 = dmulscale16(oy,xvect2,ox,yvect2);

                                                drawline256(x1+(xdim<<11),y1+(ydim<<11),
                                                                                x2+(xdim<<11),y2+(ydim<<11),col);

                                                drawline256(x2+(xdim<<11),y2+(ydim<<11),
                                                                                x3+(xdim<<11),y3+(ydim<<11),col);

                                                drawline256(x3+(xdim<<11),y3+(ydim<<11),
                                                                                x4+(xdim<<11),y4+(ydim<<11),col);

                                                drawline256(x4+(xdim<<11),y4+(ydim<<11),
                                                                                x1+(xdim<<11),y1+(ydim<<11),col);

                                                break;
                                }
                        }
        }

                //Draw white lines
        for(i=0;i<numsectors;i++)
        {
                if (!(show2dsector[i>>3]&(1<<(i&7)))) continue;

                startwall = sector[i].wallptr;
                endwall = sector[i].wallptr + sector[i].wallnum;

                k = -1;
                for(j=startwall,wal=&wall[startwall];j<endwall;j++,wal++)
                {
                        if (wal->nextwall >= 0) continue;

                        //if ((show2dwall[j>>3]&(1<<(j&7))) == 0) continue;

                        if (tilesizx[wal->picnum] == 0) continue;
                        if (tilesizy[wal->picnum] == 0) continue;

                        if (j == k)
                                { x1 = x2; y1 = y2; }
                        else
                        {
                                ox = wal->x-cposx; oy = wal->y-cposy;
                                x1 = dmulscale16(ox,xvect,-oy,yvect)+(xdim<<11);
                                y1 = dmulscale16(oy,xvect2,ox,yvect2)+(ydim<<11);
                        }

                        k = wal->point2; wal2 = &wall[k];
                        ox = wal2->x-cposx; oy = wal2->y-cposy;
                        x2 = dmulscale16(ox,xvect,-oy,yvect)+(xdim<<11);
                        y2 = dmulscale16(oy,xvect2,ox,yvect2)+(ydim<<11);

                        drawline256(x1,y1,x2,y2,24);
                }
        }

         for(p=connecthead;p >= 0;p=connectpoint2[p])
         {
          if(ud.scrollmode && p == screenpeek) continue;

          ox = sprite[ps[p].i].x-cposx; oy = sprite[ps[p].i].y-cposy;
                  daang = (sprite[ps[p].i].ang-cang)&2047;
                  if (p == screenpeek) { ox = 0; oy = 0; daang = 0; }
                  x1 = mulscale(ox,xvect,16) - mulscale(oy,yvect,16);
                  y1 = mulscale(oy,xvect2,16) + mulscale(ox,yvect2,16);

          if(p == screenpeek || ud.coop == 1 )
          {
#ifdef RRRA
              if (ps[p].OnMotorcycle)
                  i = RRTILE7169;
              else if (ps[p].OnBoat)
                  i = RRTILE7191;
              else
#endif
                if(sprite[ps[p].i].xvel > 16 && ps[p].on_ground)
                    i = APLAYERTOP+((totalclock>>4)&3);
                else
                    i = APLAYERTOP;

                j = klabs(ps[p].truefz-ps[p].posz)>>8;
                j = mulscale(czoom*(sprite[ps[p].i].yrepeat+j),yxaspect,16);

                if(j < 22000) j = 22000;
                else if(j > (65536<<1)) j = (65536<<1);

#ifdef RRRA
                if (ps[p].OnMotorcycle)
                    rotatesprite((x1<<4)+(xdim<<15),(y1<<4)+(ydim<<15),j>>1,
                        daang,i,sprite[ps[p].i].shade,sprite[ps[p].i].pal,
                        (sprite[ps[p].i].cstat&2)>>1,windowx1,windowy1,windowx2,windowy2);
                else if(ps[p].OnBoat)
                    rotatesprite((x1<<4)+(xdim<<15),(y1<<4)+(ydim<<15),j>>1,
                        daang,i,sprite[ps[p].i].shade,sprite[ps[p].i].pal,
                        (sprite[ps[p].i].cstat&2)>>1,windowx1,windowy1,windowx2,windowy2);
                else
#endif
                rotatesprite((x1<<4)+(xdim<<15),(y1<<4)+(ydim<<15),j,
                    daang,i,sprite[ps[p].i].shade,sprite[ps[p].i].pal,
                    (sprite[ps[p].i].cstat&2)>>1,windowx1,windowy1,windowx2,windowy2);
          }
         }
}



void endanimsounds(long fr)
{
    switch(ud.volume_number)
    {
        case 0:break;
        case 1:
            switch(fr)
            {
                case 1:
                    sound(390);
                    break;
                case 26:
                    sound(390);
                    break;
                case 36:
                    sound(390);
                    break;
                case 54:
                    sound(390);
                    break;
                case 62:
                    sound(390);
                    break;
                case 75:
                    sound(390);
                    break;
                case 81:
                    sound(390);
                    break;
                case 115:
                    sound(390);
                    break;
                case 124:
                    sound(390);
                    break;
            }
            break;
        case 2:
            switch(fr)
            {
                case 1:
                    sound(390);
                    break;
                case 98:
                    sound(390);
                    break;
                case 82+20:
                    sound(390);
                    sound(390);
                    break;
                case 104+20:
                    sound(390);
                    break;
                case 114+20:
                    sound(390);
                    break;
                case 158:
                    sound(390);
                    break;
            }
            break;
    }
}

void logoanimsounds(long fr, short s)
{
#ifdef RRRA
    switch (s)
    {
        case 1:
            if (fr == 1)
                sound(256);
            break;
        case 2:
            if (fr == 1)
                sound(257);
            break;
        case 3:
            if (fr == 1)
                sound(258);
            break;
        case 4:
            if (fr == 1)
                sound(259);
            break;
        case 5:
            break;
        case 6:
            if (fr == 1)
                sound(479);
            break;
    }
#else
    switch(s)
    {
        case -1:
            if (fr == 1)
                sound(29);
            break;
        case 0:
            if (fr == 1)
                sound(478);
            break;
        case 1:
            if (fr == 1)
                sound(479);
            break;
        case 4:
            if (fr == 1)
                sound(35);
            break;
        case 5:
            if (fr == 1)
                sound(82);
            break;
    }
#endif
}


#ifdef RRRA
short playanm(char *fn,char t, short s)
#else
void playanm(char *fn,char t, short s)
#endif
{
        char *animbuf, *palptr;
    long i, j, k, length, numframes;
    int32 handle;
#ifdef RRRA
    char *windir = getenv("windir");
    if (windir)
    {
        cddrives = 0;
        cdon = 0;
        sound(390);
        return -1;
    }
        handle = kopen4load(fn,0);
        if(handle == -1) return -1;
        length = kfilelength(handle);
#else
        handle = kopen4load(fn,0);
        if(handle == -1) return;
        length = kfilelength(handle);
#endif

    walock[MAXTILES-3-t] = 219+t;

    if(anim == 0)
        allocache((long *)&anim,length+sizeof(anim_t),&walock[MAXTILES-3-t]);

    animbuf = (char *)(FP_OFF(anim)+sizeof(anim_t));

    tilesizx[MAXTILES-3-t] = 200;
    tilesizy[MAXTILES-3-t] = 320;

        kread(handle,animbuf,length);
        kclose(handle);

        setview(0, 0, xdim, ydim);
        clearview(0);
        nextpage();

        ANIM_LoadAnim (animbuf);
        numframes = ANIM_NumFrames();

        palptr = ANIM_GetPalette();
        for(i=0;i<256;i++)
        {
                j = (i<<2); k = j-i;
                tempbuf[j+0] = (palptr[k+2]>>2);
                tempbuf[j+1] = (palptr[k+1]>>2);
                tempbuf[j+2] = (palptr[k+0]>>2);
                tempbuf[j+3] = 0;
        }

        VBE_setPalette(0L,256L,tempbuf);

    ototalclock = totalclock + 10;

    KB_FlushKeyboardQueue();

        for(i=1;i<numframes;i++)
        {
       while(totalclock < ototalclock)
       {
          if( KB_KeyWaiting() )
          {
              FX_StopAllSounds();
              clearsoundlocks();
#ifdef RRRA
              ANIM_FreeAnim();
              walock[MAXTILES-3-t] = 1;
              return 10;
#else
              goto ENDOFANIMLOOP;
#endif
          }
          getpackets();
       }

       if(t == 6) ototalclock += 400;
#ifdef RRRA
       else if(t == 5) ototalclock += 8;
#else
       else if(t == 5) ototalclock += 9;
#endif
       else if(ud.volume_number == 2) ototalclock += 10;
       else if(ud.volume_number == 1) ototalclock += 18;
       else                           ototalclock += 10;

       waloff[MAXTILES-3-t] = FP_OFF(ANIM_DrawFrame(i));
       rotatesprite(0<<16,0<<16,65536L,512,MAXTILES-3-t,0,0,2+4+8+16+64, 0,0,xdim-1,ydim-1);
       nextpage();

       if(t == 5) logoanimsounds(i,s);
       else if(t < 4) endanimsounds(i);
        }

    ENDOFANIMLOOP:

    ANIM_FreeAnim ();
    walock[MAXTILES-3-t] = 1;
#ifdef RRRA
    return 0;
#endif
}

#ifdef RRRA

void PlayMapAnim(void)
{
    char *fn;
    char t;
        char *animbuf, *palptr;
    long i, j, k, length=0, numframes=0;
    int32 handle=-1;
    char *windir;

    fn = NULL;
    t = 5;
    if (ud.volume_number == 0)
    {
        switch (ud.level_number)
        {
            case 1:
                fn = "lvl1.anm";
                break;
            case 2:
                fn = "lvl2.anm";
                break;
            case 3:
                fn = "lvl3.anm";
                break;
            case 4:
                fn = "lvl4.anm";
                break;
            case 5:
                fn = "lvl5.anm";
                break;
            case 6:
                fn = "lvl6.anm";
                break;
            default:
                fn = "lvl7.anm";
                break;
        }
    }
    else
    {
        switch (ud.level_number)
        {
            case 1:
                fn = "lvl8.anm";
                break;
            case 2:
                fn = "lvl9.anm";
                break;
            case 3:
                fn = "lvl10.anm";
                break;
            case 4:
                fn = "lvl11.anm";
                break;
            case 5:
                fn = "lvl12.anm";
                break;
            case 6:
                fn = "lvl13.anm";
                break;
            default:
                fn = NULL;
                break;
        }
    }
    windir = getenv("windir");
    if (windir)
    {
        cddrives = 0;
        cdon = 0;
        sound(390);
        return;
    }
        handle = kopen4load(fn,0);
        if(handle == -1) return;
        length = kfilelength(handle);

    walock[MAXTILES-3-t] = 219+t;

    if(anim == 0)
        allocache((long *)&anim,length+sizeof(anim_t),&walock[MAXTILES-3-t]);

    animbuf = (char *)(FP_OFF(anim)+sizeof(anim_t));

    tilesizx[MAXTILES-3-t] = 200;
    tilesizy[MAXTILES-3-t] = 320;

        kread(handle,animbuf,length);
        kclose(handle);

        ANIM_LoadAnim (animbuf);
        numframes = ANIM_NumFrames();

        palptr = ANIM_GetPalette();
        for(i=0;i<256;i++)
        {
                j = (i<<2); k = j-i;
                tempbuf[j+0] = (palptr[k+2]>>2);
                tempbuf[j+1] = (palptr[k+1]>>2);
                tempbuf[j+2] = (palptr[k+0]>>2);
                tempbuf[j+3] = 0;
        }

        VBE_setPalette(0L,256L,tempbuf);

    ototalclock = totalclock + 10;

    KB_FlushKeyboardQueue();

        for(i=1;i<numframes;i++)
        {
       while(totalclock < ototalclock)
       {
          if( KB_KeyWaiting() )
          {
              FX_StopAllSounds();
              clearsoundlocks();
              goto ENDOFANIMLOOP;
          }
          getpackets();
       }

       ototalclock += 20;

       waloff[MAXTILES-3-t] = FP_OFF(ANIM_DrawFrame(i));
       rotatesprite(0<<16,0<<16,65536L,512,MAXTILES-3-t,0,0,2+4+8+16+64, 0,0,xdim-1,ydim-1);
       nextpage();
        }

    ENDOFANIMLOOP:

    ANIM_FreeAnim ();
    walock[MAXTILES-3-t] = 1;
}

void ShowMapFrame(void)
{
    short t = -1, i;
    ps[myconnectindex].palette = palette;
    if (ud.volume_number == 0)
    {
        switch (ud.level_number)
        {
            case 1:
                t = 0;
                break;
            case 2:
                t = 1;
                break;
            case 3:
                t = 2;
                break;
            case 4:
                t = 3;
                break;
            case 5:
                t = 4;
                break;
            case 6:
                t = 5;
                break;
            default:
                t = 6;
                break;
        }
    }
    else
    {
        switch (ud.level_number)
        {
            case 1:
                t = 7;
                break;
            case 2:
                t = 8;
                break;
            case 3:
                t = 9;
                break;
            case 4:
                t = 10;
                break;
            case 5:
                t = 11;
                break;
            case 6:
                t = 12;
                break;
            default:
                t = -1;
                break;
        }
    }
    rotatesprite(0,0,65536,0,8624+t,0,0,10+16+64+128,0,0,xdim-1,ydim-1);
    for (i = 0; i < 64; i++)
        palto(0,0,0,63-i);
    ps[myconnectindex].palette = palette;
}

#endif
