/***************************************************************************
 *   TEKMAP.C  - 2 dimensional map modes                                   *
 *                                                                         *
 *                                                                         *
 ***************************************************************************/

#include "build.h"
#include "pragmas.h"
#include "names.h"

#include "tekwar.h"

void
drawoverheadmap(int cposx, int cposy, int czoom, short cang)
{
     int i, j, k, l, x1, y1, x2, y2, x3, y3, x4, y4, ox, oy, xoff, yoff;
     int dax, day, cosang, sinang, xspan, yspan, sprx, spry;
     int xrepeat, yrepeat, z1, z2, startwall, endwall, tilenum, daang;
     int xvect, yvect, xvect2, yvect2;
     unsigned char col;
     walltype *wal, *wal2;
     spritetype *spr;

     xvect = sintable[(2048-cang)&2047] * czoom;
     yvect = sintable[(1536-cang)&2047] * czoom;
     xvect2 = mulscale(xvect,yxaspect,16);
     yvect2 = mulscale(yvect,yxaspect,16);

          //Draw red lines
     for(i=0;i<numsectors;i++)
     {
          startwall = sector[i].wallptr;
          endwall = sector[i].wallptr + sector[i].wallnum - 1;

          z1 = sector[i].ceilingz; z2 = sector[i].floorz;

          for(j=startwall,wal=&wall[startwall];j<=endwall;j++,wal++)
          {
               k = wal->nextwall; if (k < 0) continue;

               if ((show2dwall[j>>3]&(1<<(j&7))) == 0) continue;
               if ((k > j) && ((show2dwall[k>>3]&(1<<(k&7))) > 0)) continue;

               if (sector[wal->nextsector].ceilingz == z1)
                    if (sector[wal->nextsector].floorz == z2)
                         if (((wal->cstat|wall[wal->nextwall].cstat)&(16+32)) == 0) continue;

               col = 232;

               if (dimensionmode[screenpeek] == 2)
               {
                    if (sector[i].floorz != sector[i].ceilingz)
                         if (sector[wal->nextsector].floorz != sector[wal->nextsector].ceilingz)
                              if (((wal->cstat|wall[wal->nextwall].cstat)&(16+32)) == 0)
                                   if (sector[i].floorz == sector[wal->nextsector].floorz) continue;
                    if (sector[i].floorpicnum != sector[wal->nextsector].floorpicnum) continue;
                    if (sector[i].floorshade != sector[wal->nextsector].floorshade) continue;
                    col = 232;
               }

               ox = wal->x-cposx; oy = wal->y-cposy;
               x1 = mulscale(ox,xvect,16) - mulscale(oy,yvect,16);
               y1 = mulscale(oy,xvect2,16) + mulscale(ox,yvect2,16);

               wal2 = &wall[wal->point2];
               ox = wal2->x-cposx; oy = wal2->y-cposy;
               x2 = mulscale(ox,xvect,16) - mulscale(oy,yvect,16);
               y2 = mulscale(oy,xvect2,16) + mulscale(ox,yvect2,16);

               drawline256(x1+(xdim<<11),y1+(ydim<<11),x2+(xdim<<11),y2+(ydim<<11),col);
          }
     }

          //Draw sprites
     k = playersprite[screenpeek];
     show2dsprite[k>>3] |= (1<<(k&7));
     for(i=0;i<numsectors;i++)
          for(j=headspritesect[i];j>=0;j=nextspritesect[j])
               if ((show2dsprite[j>>3]&(1<<(j&7))) > 0)
               {
                    spr = &sprite[j];
                    col = 56;
                    if ((spr->cstat&1) > 0) col = 248;
                    if (j == k) col = 31;

                    sprx = spr->x;
                    spry = spr->y;

                    k = spr->statnum;
                    if ((k >= 1) && (k <= 8) && (k != 2))  //Interpolate moving sprite
                    {
                         sprx = osprite[j].x+mulscale(sprx-osprite[j].x,smoothratio,16);
                         spry = osprite[j].y+mulscale(spry-osprite[j].y,smoothratio,16);
                    }

                    switch (spr->cstat&48)
                    {
                         case 0:
                              ox = sprx-cposx; oy = spry-cposy;
                              x1 = mulscale(ox,xvect,16) - mulscale(oy,yvect,16);
                              y1 = mulscale(oy,xvect2,16) + mulscale(ox,yvect2,16);

                              if (dimensionmode[screenpeek] == 1)
                              {
                                   ox = (sintable[(spr->ang+512)&2047]>>7);
                                   oy = (sintable[(spr->ang)&2047]>>7);
                                   x2 = mulscale(ox,xvect,16) - mulscale(oy,yvect,16);
                                   y2 = mulscale(oy,xvect,16) + mulscale(ox,yvect,16);

                                   if (j == playersprite[screenpeek])
                                   {
                                        x2 = 0L;
                                        y2 = -(czoom<<5);
                                   }

                                   x3 = mulscale(x2,yxaspect,16);
                                   y3 = mulscale(y2,yxaspect,16);

                                   drawline256(x1-x2+(xdim<<11),y1-y3+(ydim<<11),
                                                       x1+x2+(xdim<<11),y1+y3+(ydim<<11),col);
                                   drawline256(x1-y2+(xdim<<11),y1+x3+(ydim<<11),
                                                       x1+x2+(xdim<<11),y1+y3+(ydim<<11),col);
                                   drawline256(x1+y2+(xdim<<11),y1-x3+(ydim<<11),
                                                       x1+x2+(xdim<<11),y1+y3+(ydim<<11),col);
                              }
                              else
                              {
                                   if (((gotsector[i>>3]&(1<<(i&7))) > 0) && (czoom > 192))
                                   {
                                        daang = (spr->ang-cang)&2047;
                                        if (j == playersprite[screenpeek])
                                             { x1 = 0; y1 = (yxaspect<<2); daang = 0; }
                                        rotatesprite((x1<<4)+(xdim<<15),(y1<<4)+(ydim<<15),mulscale(czoom*spr->yrepeat,yxaspect,16),daang,spr->picnum,spr->shade,spr->pal,(spr->cstat&2)>>1, windowx1, windowy1, windowx2, windowy2);
                                   }
                              }
                              break;
                         case 16:
                              x1 = sprx; y1 = spry;
                              tilenum = spr->picnum;
                              xoff = (int)((signed char)((picanm[tilenum]>>8)&255))+((int)spr->xoffset);
                              if ((spr->cstat&4) > 0) xoff = -xoff;
                              k = spr->ang; l = spr->xrepeat;
                              dax = sintable[k&2047]*l; day = sintable[(k+1536)&2047]*l;
                              l = tilesizx[tilenum]; k = (l>>1)+xoff;
                              x1 -= mulscale(dax,k,16); x2 = x1+mulscale(dax,l,16);
                              y1 -= mulscale(day,k,16); y2 = y1+mulscale(day,l,16);

                              ox = x1-cposx; oy = y1-cposy;
                              x1 = mulscale(ox,xvect,16) - mulscale(oy,yvect,16);
                              y1 = mulscale(oy,xvect2,16) + mulscale(ox,yvect2,16);

                              ox = x2-cposx; oy = y2-cposy;
                              x2 = mulscale(ox,xvect,16) - mulscale(oy,yvect,16);
                              y2 = mulscale(oy,xvect2,16) + mulscale(ox,yvect2,16);

                              drawline256(x1+(xdim<<11),y1+(ydim<<11),
                                                  x2+(xdim<<11),y2+(ydim<<11),col);

                              break;
                         case 32:
                              if (dimensionmode[screenpeek] == 1)
                              {
                                   tilenum = spr->picnum;
                                   xoff = (int)((signed char)((picanm[tilenum]>>8)&255))+((int)spr->xoffset);
                                   yoff = (int)((signed char)((picanm[tilenum]>>16)&255))+((int)spr->yoffset);
                                   if ((spr->cstat&4) > 0) xoff = -xoff;
                                   if ((spr->cstat&8) > 0) yoff = -yoff;

                                   k = spr->ang;
                                   cosang = sintable[(k+512)&2047]; sinang = sintable[k];
                                   xspan = tilesizx[tilenum]; xrepeat = spr->xrepeat;
                                   yspan = tilesizy[tilenum]; yrepeat = spr->yrepeat;

                                   dax = ((xspan>>1)+xoff)*xrepeat; day = ((yspan>>1)+yoff)*yrepeat;
                                   x1 = sprx + mulscale(sinang,dax,16) + mulscale(cosang,day,16);
                                   y1 = spry + mulscale(sinang,day,16) - mulscale(cosang,dax,16);
                                   l = xspan*xrepeat;
                                   x2 = x1 - mulscale(sinang,l,16);
                                   y2 = y1 + mulscale(cosang,l,16);
                                   l = yspan*yrepeat;
                                   k = -mulscale(cosang,l,16); x3 = x2+k; x4 = x1+k;
                                   k = -mulscale(sinang,l,16); y3 = y2+k; y4 = y1+k;

                                   ox = x1-cposx; oy = y1-cposy;
                                   x1 = mulscale(ox,xvect,16) - mulscale(oy,yvect,16);
                                   y1 = mulscale(oy,xvect2,16) + mulscale(ox,yvect2,16);

                                   ox = x2-cposx; oy = y2-cposy;
                                   x2 = mulscale(ox,xvect,16) - mulscale(oy,yvect,16);
                                   y2 = mulscale(oy,xvect2,16) + mulscale(ox,yvect2,16);

                                   ox = x3-cposx; oy = y3-cposy;
                                   x3 = mulscale(ox,xvect,16) - mulscale(oy,yvect,16);
                                   y3 = mulscale(oy,xvect2,16) + mulscale(ox,yvect2,16);

                                   ox = x4-cposx; oy = y4-cposy;
                                   x4 = mulscale(ox,xvect,16) - mulscale(oy,yvect,16);
                                   y4 = mulscale(oy,xvect2,16) + mulscale(ox,yvect2,16);

                                   drawline256(x1+(xdim<<11),y1+(ydim<<11),
                                                       x2+(xdim<<11),y2+(ydim<<11),col);

                                   drawline256(x2+(xdim<<11),y2+(ydim<<11),
                                                       x3+(xdim<<11),y3+(ydim<<11),col);

                                   drawline256(x3+(xdim<<11),y3+(ydim<<11),
                                                       x4+(xdim<<11),y4+(ydim<<11),col);

                                   drawline256(x4+(xdim<<11),y4+(ydim<<11),
                                                       x1+(xdim<<11),y1+(ydim<<11),col);

                              }
                              break;
                    }
               }

          //Draw white lines
     for(i=0;i<numsectors;i++)
     {
          startwall = sector[i].wallptr;
          endwall = sector[i].wallptr + sector[i].wallnum - 1;

          for(j=startwall,wal=&wall[startwall];j<=endwall;j++,wal++)
          {
               if (wal->nextwall >= 0) continue;

               if ((show2dwall[j>>3]&(1<<(j&7))) == 0) continue;

               if (tilesizx[wal->picnum] == 0) continue;
               if (tilesizy[wal->picnum] == 0) continue;

               ox = wal->x-cposx; oy = wal->y-cposy;
               x1 = mulscale(ox,xvect,16) - mulscale(oy,yvect,16);
               y1 = mulscale(oy,xvect2,16) + mulscale(ox,yvect2,16);

               wal2 = &wall[wal->point2];
               ox = wal2->x-cposx; oy = wal2->y-cposy;
               x2 = mulscale(ox,xvect,16) - mulscale(oy,yvect,16);
               y2 = mulscale(oy,xvect2,16) + mulscale(ox,yvect2,16);

               drawline256(x1+(xdim<<11),y1+(ydim<<11),x2+(xdim<<11),y2+(ydim<<11),239);
          }
     }
}

