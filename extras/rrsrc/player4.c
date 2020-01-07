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

// Savage Baggage Masters

#include "duke3d.h"

extern long lastvisinc;


#ifdef DESYNCCORRECT
/////Desync fix////
extern short fakejval;
///////////////////
#endif

void processinput(short snum)
{
    long j, i, k, doubvel, fz, cz, hz, lz, truefdist, x, y, var60;
    char shrunk;
    unsigned long sb_snum;
    short psect, psectlotag,*kb, tempsect, pi;
    struct player_struct *p;
    spritetype *s;
    short unk1, unk2;

    p = &ps[snum];
    pi = p->i;
    s = &sprite[pi];

    kb = &p->kickback_pic;

#ifdef DESYNCCORRECT
    /////Desync fix////
#ifdef RRRA
    fakejval = 1;
#else
    fakejval = snum;
#endif
    ///////////////////
#endif

    if(p->cheat_phase <= 0) sb_snum = sync[snum].bits;
    else sb_snum = 0;

    psect = p->cursectnum;
#ifdef RRRA
    if (p->OnMotorcycle && s->extra > 0)
    {
        int var64, var68, var6c, var70, var74, var78, var7c, var80;
        short var84;
        if (p->MotoSpeed < 0)
            p->MotoSpeed = 0;
        if (sb_snum & 2)
        {
            var64 = 1;
            sb_snum &= ~2;
        }
        else
            var64 = 0;

        if (sb_snum & 1)
        {
            var68 = 1;
            sb_snum &= ~1;
            if (p->on_ground)
            {
                if (p->MotoSpeed == 0 && var64)
                {
                    if (Sound[187].num == 0)
                        spritesound(187,pi);
                }
                else if (p->MotoSpeed == 0 && Sound[214].num == 0)
                {
                    if (Sound[187].num > 0)
                        stopsound(Sound[187].num);
                    spritesound(214,pi);
                }
                else if (p->MotoSpeed >= 50 && Sound[188].num == 0)
                {
                    spritesound(188,pi);
                }
                else if (Sound[188].num == 0 && Sound[214].num == 0)
                {
                    spritesound(188,pi);
                }
            }
        }
        else
        {
            var68 = 0;
            if (Sound[214].num > 0)
            {
                stopsound(Sound[214].num);
                if (Sound[189].num == 0)
                    spritesound(189,pi);
            }
            if (Sound[188].num > 0)
            {
                stopsound(Sound[188].num);
                if (Sound[189].num == 0)
                    spritesound(189,pi);
            }
            if (Sound[189].num == 0 && Sound[187].num == 0)
                spritesound(187,pi);
        }
        if (sb_snum & 8)
        {
            var6c = 1;
            sb_snum &= ~8;
        }
        else
            var6c = 0;
        if (sb_snum & 16)
        {
            var70 = 1;
            var74 = 1;
            sb_snum &= ~16;
        }
        else
        {
            var70 = 0;
            var74 = 0;
        }
        if (sb_snum & 64)
        {
            var78 = 1;
            var7c = 1;
            sb_snum &= ~64;
        }
        else
        {
            var78 = 0;
            var7c = 0;
        }
        var80 = 0;
        if (p->drink_amt > 88 && p->raat5c1 == 0)
        {
            var84 = TRAND & 63;
            if (var84 == 1)
                p->raat5c1 = -10;
            else if (var84 == 2)
                p->raat5c1 = 10;
        }
        else if (p->drink_amt > 99 && p->raat5c1 == 0)
        {
            var84 = TRAND & 31;
            if (var84 == 1)
                p->raat5c1 = -20;
            else if (var84 == 2)
                p->raat5c1 = 20;
        }
        if (p->on_ground == 1)
        {
            if (var64 && p->MotoSpeed > 0)
            {
                if (p->raat5cf)
                    p->MotoSpeed -= 2;
                else
                    p->MotoSpeed -= 4;
                if (p->MotoSpeed < 0)
                    p->MotoSpeed = 0;
                p->VBumpTarget = -30;
                p->raat5b5 = 1;
            }
            else if (var68 && !var64)
            {
                if (p->MotoSpeed < 40)
                {
                    p->VBumpTarget = 70;
                    p->raat5c7 = 1;
                }
                p->MotoSpeed += 2;
                if (p->MotoSpeed > 120)
                    p->MotoSpeed = 120;
                if (!p->NotOnWater)
                    if (p->MotoSpeed > 80)
                        p->MotoSpeed = 80;
            }
            else if (p->MotoSpeed > 0)
                p->MotoSpeed--;
            if (p->raat5b5 && (!var64 || p->MotoSpeed == 0))
            {
                p->VBumpTarget = 0;
                p->raat5b5 = 0;
            }
            if (var6c && p->MotoSpeed <= 0 && !var64)
            {
                int var88;
                p->MotoSpeed = -15;
                var88 = var7c;
                var7c = var74;
                var74 = var88;
                var80 = 1;
            }
        }
        if (p->MotoSpeed != 0 && p->on_ground == 1)
        {
            if (!p->VBumpNow)
                if ((TRAND & 3) == 2)
                    p->VBumpTarget = (p->MotoSpeed>>4)*((TRAND&7)-4);
            if (var74 || p->raat5c1 < 0)
            {
                if (p->raat5c1 < 0)
                    p->raat5c1++;
            }
            else if (var7c || p->raat5c1 > 0)
            {
                if (p->raat5c1 > 0)
                    p->raat5c1--;
            }
        }
        if (p->TurbCount)
        {
            if (p->TurbCount <= 1)
            {
                p->horiz = 100;
                p->TurbCount = 0;
                p->VBumpTarget = 0;
                p->VBumpNow = 0;
            }
            else
            {
                p->horiz = 100+((TRAND&15)-7);
                p->TurbCount--;
                p->raat5c1 = (TRAND&3)-2;
            }
        }
        else if (p->VBumpTarget > p->VBumpNow)
        {
            if (p->raat5c7)
                p->VBumpNow += 6;
            else
                p->VBumpNow++;
            if (p->VBumpTarget < p->VBumpNow)
                p->VBumpNow = p->VBumpTarget;
            p->horiz = 100+p->VBumpNow/3;
        }
        else if (p->VBumpTarget < p->VBumpNow)
        {
            if (p->raat5c7)
                p->VBumpNow -= 6;
            else
                p->VBumpNow--;
            if (p->VBumpTarget > p->VBumpNow)
                p->VBumpNow = p->VBumpTarget;
            p->horiz = 100+p->VBumpNow/3;
        }
        else
        {
            p->VBumpTarget = 0;
            p->raat5c7 = 0;
        }
        if (p->MotoSpeed >= 20 && p->on_ground == 1 && (var74 || var7c))
        {
            short var8c, var90, var94, var98;
            var8c = p->MotoSpeed;
            var90 = p->ang;
            if (var74)
                var94 = -10;
            else
                var94 = 10;
            if (var94 < 0)
                var98 = 350;
            else
                var98 = -350;
            if (p->raat5cd || p->raat5cf || !p->NotOnWater)
            {
                if (p->raat5cf)
                    var8c <<= 3;
                else
                    var8c <<= 2;
                if (p->raat5b5)
                {
                    p->posxv += (var8c>>5)*(sintable[(var94*-51+var90+512)&2047]<<4);
                    p->posyv += (var8c>>5)*(sintable[(var94*-51+var90)&2047]<<4);
                    p->ang = (var90-(var98>>2))&2047;
                }
                else
                {
                    p->posxv += (var8c>>7)*(sintable[(var94*-51+var90+512)&2047]<<4);
                    p->posyv += (var8c>>7)*(sintable[(var94*-51+var90)&2047]<<4);
                    p->ang = (var90-(var98>>6))&2047;
                }
                p->raat5cd = 0;
                p->raat5cf = 0;
            }
            else
            {
                if (p->raat5b5)
                {
                    p->posxv += (var8c>>5)*(sintable[(var94*-51+var90+512)&2047]<<4);
                    p->posyv += (var8c>>5)*(sintable[(var94*-51+var90)&2047]<<4);
                    p->ang = (var90-(var98>>4))&2047;
                    if (Sound[220].num == 0)
                        spritesound(220,p->i);
                }
                else
                {
                    p->posxv += (var8c>>7)*(sintable[(var94*-51+var90+512)&2047]<<4);
                    p->posyv += (var8c>>7)*(sintable[(var94*-51+var90)&2047]<<4);
                    p->ang = (var90-(var98>>7))&2047;
                }
            }
        }
        else if (p->MotoSpeed >= 20 && p->on_ground == 1 && (p->raat5cd || p->raat5cf))
        {
            short var9c, vara0, vara4, vara8;
            var9c = p->MotoSpeed;
            vara0 = p->ang;
            var84 = TRAND&1;
            if (var84 == 0)
                vara4 = -10;
            else if (var84 == 1)
                vara4 = 10;
            if (p->raat5cf)
                var9c *= 10;
            else
                var9c *= 5;
            p->posxv += (var9c>>7)*(sintable[(vara4*-51+vara0+512)&2047]<<4);
            p->posyv += (var9c>>7)*(sintable[(vara4*-51+vara0)&2047]<<4);
        }
        p->raat5cd = 0;
        p->raat5cf = 0;
    }
    else if (p->OnBoat && s->extra > 0)
    {
        int vara8, varac, varb0, varb4, varb8, varbc, varc0, varc4, varc8;
        short varcc;
        if (p->NotOnWater)
        {
            if (p->MotoSpeed > 0)
            {
                if (Sound[88].num == 0)
                    spritesound(88,pi);
            }
            else
            {
                if (Sound[87].num == 0)
                    spritesound(87,pi);
            }
        }
        if (p->MotoSpeed < 0)
            p->MotoSpeed = 0;
        if ((sb_snum&2) && (sb_snum&1))
        {
            vara8 = 1;
            varac = 0;
            sb_snum &= ~1;
            varb0 = 0;
            sb_snum &= ~2;
        }
        else
            vara8 = 0;
        if (sb_snum & 1)
        {
            varac = 1;
            sb_snum &= ~1;
            if (p->MotoSpeed == 0 && Sound[89].num == 0)
            {
                if (Sound[87].num > 0)
                    stopsound(Sound[87].num);
                spritesound(89,pi);
            }
            else if (p->MotoSpeed >= 50 && Sound[88].num == 0)
                spritesound(88,pi);
            else if (Sound[88].num == 0 && Sound[89].num == 0)
                spritesound(88,pi);
        }
        else
        {
            varac = 0;
            if (Sound[89].num > 0)
            {
                stopsound(Sound[89].num);
                if (Sound[90].num == 0)
                    spritesound(90,pi);
            }
            if (Sound[88].num > 0)
            {
                stopsound(Sound[88].num);
                if (Sound[90].num == 0)
                    spritesound(90,pi);
            }
            if (Sound[90].num == 0 && Sound[87].num == 0)
                spritesound(87,pi);
        }
        if (sb_snum & 2)
        {
            varb0 = 1;
            sb_snum &= ~2;
        }
        else
            varb0 = 0;
        if (sb_snum & 8)
        {
            varb4 = 1;
            sb_snum &= ~8;
        }
        else varb4 = 0;
        if (sb_snum & 16)
        {
            varb8 = 1;
            varbc = 1;
            sb_snum &= ~16;
            if (Sound[91].num == 0 && p->MotoSpeed > 30 && !p->NotOnWater)
                spritesound(91,pi);
        }
        else
        {
            varb8 = 0;
            varbc = 0;
        }
        if (sb_snum & 64)
        {
            varc0 = 1;
            varc4 = 1;
            sb_snum &= ~64;
            if (Sound[91].num == 0 && p->MotoSpeed > 30 && !p->NotOnWater)
                spritesound(91,pi);
        }
        else
        {
            varc0 = 0;
            varc4 = 0;
        }
        varc8 = 0;
        if (!p->NotOnWater)
        {
            if (p->drink_amt > 88 && p->raat5c1 == 0)
            {
                varcc = TRAND & 63;
                if (varcc == 1)
                    p->raat5c1 = -10;
                else if (varcc == 2)
                    p->raat5c1 = 10;
            }
            else if (p->drink_amt > 99 && p->raat5c1 == 0)
            {
                varcc = TRAND & 31;
                if (varcc == 1)
                    p->raat5c1 = -20;
                else if (varcc == 2)
                    p->raat5c1 = 20;
            }
        }
        if (p->on_ground == 1)
        {
            if (vara8)
            {
                if (p->MotoSpeed <= 25)
                {
                    p->MotoSpeed++;
                    if (Sound[182].num == 0)
                        spritesound(182, pi);
                }
                else
                {
                    p->MotoSpeed -= 2;
                    if (p->MotoSpeed < 0)
                        p->MotoSpeed = 0;
                    p->VBumpTarget = 30;
                    p->raat5b5 = 1;
                }
            }
            else if (varb0 && p->MotoSpeed > 0)
            {
                p->MotoSpeed -= 2;
                if (p->MotoSpeed < 0)
                    p->MotoSpeed = 0;
                p->VBumpTarget = 30;
                p->raat5b5 = 1;
            }
            else if (varac)
            {
                if (p->MotoSpeed < 40)
                    if (!p->NotOnWater)
                    {
                        p->VBumpTarget = -30;
                        p->raat5c7 = 1;
                    }
                p->MotoSpeed++;
                if (p->MotoSpeed > 120)
                    p->MotoSpeed = 120;
            }
            else if (p->MotoSpeed > 0)
                p->MotoSpeed--;
            if (p->raat5b5 && (!varb0 || p->MotoSpeed == 0))
            {
                p->VBumpTarget = 0;
                p->raat5b5 = 0;
            }
            if (varb4 && p->MotoSpeed == 0 && !varb0)
            {
                int vard0;
                if (!p->NotOnWater)
                    p->MotoSpeed = -25;
                else
                    p->MotoSpeed = -20;
                vard0 = varc4;
                varc4 = varbc;
                varbc = vard0;
                varc8 = 1;
            }
        }
        if (p->MotoSpeed != 0 && p->on_ground == 1)
        {
            if (!p->VBumpNow)
                if ((TRAND & 15) == 14)
                    p->VBumpTarget = (p->MotoSpeed>>4)*((TRAND&3)-2);
            if (varbc || p->raat5c1 < 0)
            {
                if (p->raat5c1 < 0)
                    p->raat5c1++;
            }
            else if (varc4 || p->raat5c1 > 0)
            {
                if (p->raat5c1 > 0)
                    p->raat5c1--;
            }
        }
        if (p->TurbCount)
        {
            if (p->TurbCount <= 1)
            {
                p->horiz = 100;
                p->TurbCount = 0;
                p->VBumpTarget = 0;
                p->VBumpNow = 0;
            }
            else
            {
                p->horiz = 100+((TRAND&15)-7);
                p->TurbCount--;
                p->raat5c1 = (TRAND&3)-2;
            }
        }
        else if (p->VBumpTarget > p->VBumpNow)
        {
            if (p->raat5c7)
                p->VBumpNow += 6;
            else
                p->VBumpNow++;
            if (p->VBumpTarget < p->VBumpNow)
                p->VBumpNow = p->VBumpTarget;
            p->horiz = 100+p->VBumpNow/3;
        }
        else if (p->VBumpTarget < p->VBumpNow)
        {
            if (p->raat5c7)
                p->VBumpNow -= 6;
            else
                p->VBumpNow--;
            if (p->VBumpTarget > p->VBumpNow)
                p->VBumpNow = p->VBumpTarget;
            p->horiz = 100+p->VBumpNow/3;
        }
        else
        {
            p->VBumpTarget = 0;
            p->raat5c7 = 0;
        }
        if (p->MotoSpeed > 0 && p->on_ground == 1 && (varbc || varc4))
        {
            short vard4, vard8, vardc, vare0;
            vard4 = p->MotoSpeed;
            vard8 = p->ang;
            if (varbc)
                vardc = -10;
            else
                vardc = 10;
            if (vardc < 0)
                vare0 = 350;
            else
                vare0 = -350;
            vard4 <<= 2;
            if (p->raat5b5)
            {
                p->posxv += (vard4>>6)*(sintable[(vardc*-51+vard8+512)&2047]<<4);
                p->posyv += (vard4>>6)*(sintable[(vardc*-51+vard8)&2047]<<4);
                p->ang = (vard8-(vare0>>5))&2047;
            }
            else
            {
                p->posxv += (vard4>>7)*(sintable[(vardc*-51+vard8+512)&2047]<<4);
                p->posyv += (vard4>>7)*(sintable[(vardc*-51+vard8)&2047]<<4);
                p->ang = (vard8-(vare0>>6))&2047;
            }
        }
        if (p->NotOnWater)
            if (p->MotoSpeed > 50)
                p->MotoSpeed -= (p->MotoSpeed>>1);
    }
#endif
    if(psect == -1)
    {
        if(s->extra > 0 && ud.clipping == 0)
        {
            quickkill(p);
            spritesound(SQUISHED,pi);
        }
        psect = 0;
    }

    psectlotag = sector[psect].lotag;

    if (psectlotag == 867)
    {
        short sj, nextsj;
        sj = headspritesect[psect];
        while (sj >= 0)
        {
            nextsj = nextspritesect[sj];
            if (sprite[sj].picnum == RRTILE380)
                if (sprite[sj].z - (8<<8) < p->posz)
                psectlotag = 2;
            sj = nextsj;
        }
    }
    else if (psectlotag == 7777)
        if (ud.volume_number == 1 && ud.level_number == 6)
            lastlevel = 1;

    if (psectlotag == 848 && sector[psect].floorpicnum == WATERTILE2)
        psectlotag = 1;

    if (psectlotag == 857)
        s->clipdist = 1;
    else
        s->clipdist = 64;
 
    p->spritebridge = 0;

    shrunk = (s->yrepeat < 8);
    if (s->clipdist == 64)
    {
        getzrange(p->posx,p->posy,p->posz,psect,&cz,&hz,&fz,&lz,163L,CLIPMASK0);
        j = getflorzofslope(psect,p->posx,p->posy);
    }
    else
    {
        getzrange(p->posx,p->posy,p->posz,psect,&cz,&hz,&fz,&lz,4L,CLIPMASK0);
        j = getflorzofslope(psect,p->posx,p->posy);
    }

    p->truefz = j;
    p->truecz = getceilzofslope(psect,p->posx,p->posy);

    truefdist = klabs(p->posz-j);
    if( (lz&49152) == 16384 && psectlotag == 1 && truefdist > PHEIGHT+(16<<8) )
        psectlotag = 0;

    hittype[pi].floorz = fz;
    hittype[pi].ceilingz = cz;

    p->ohoriz = p->horiz;
    p->ohorizoff = p->horizoff;

    if( p->aim_mode == 0 && p->on_ground && psectlotag != 2 && (sector[psect].floorstat&2) )
    {
          x = p->posx+(sintable[(p->ang+512)&2047]>>5);
          y = p->posy+(sintable[p->ang&2047]>>5);
          tempsect = psect;
          updatesector(x,y,&tempsect);
          if (tempsect >= 0)
          {
              k = getflorzofslope(psect,x,y);
              if (psect == tempsect)
                  p->horizoff += mulscale16(j-k,160);
              else if (klabs(getflorzofslope(tempsect,x,y)-k) <= (4<<8))
                  p->horizoff += mulscale16(j-k,160);
          }
     }
     if (p->horizoff > 0) p->horizoff -= ((p->horizoff>>3)+1);
     else if (p->horizoff < 0) p->horizoff += (((-p->horizoff)>>3)+1);

    if( hz >= 0 && (hz&49152) == 49152)
    {
        hz &= (MAXSPRITES-1);

        if(sprite[hz].statnum == 1 && sprite[hz].extra >= 0)
        {
            hz = 0;
            cz = p->truecz;
        }
        if (sprite[hz].picnum == RRTILE3587)
        {
            if (!p->at280)
            {
                p->at280 = 10;
#ifdef RRRA
                if ((sb_snum & 1) && !p->OnMotorcycle)
#else
                if (sb_snum & 1)
#endif
                {
                    hz = 0;
                    cz = p->truecz;
                }
            }
            else
                p->at280--;
        }
    }

    if(lz >= 0 && (lz&49152) == 49152)
    {
        j = lz&(MAXSPRITES-1);

#ifdef RRRA
        var60 = j&(MAXSPRITES-1);
#endif

        if( (sprite[j].cstat&33) == 33 )
        {
            psectlotag = 0;
            p->footprintcount = 0;
            p->spritebridge = 1;
        }
#ifdef RRRA
        if (p->OnMotorcycle)
            if (badguy(&sprite[var60]))
            {
                hittype[var60].picnum = MOTOHIT;
                hittype[var60].extra = 2+(p->MotoSpeed>>1);
                p->MotoSpeed -= p->MotoSpeed>>4;
            }
        if (p->OnBoat)
        {
            if (badguy(&sprite[var60]))
            {
                hittype[var60].picnum = MOTOHIT;
                hittype[var60].extra = 2+(p->MotoSpeed>>1);
                p->MotoSpeed -= p->MotoSpeed>>4;
            }
        }
#endif
        else if(badguy(&sprite[j]) && sprite[j].xrepeat > 24 && klabs(s->z-sprite[j].z) < (84<<8) )
        {
            j = getangle(sprite[j].x-p->posx,sprite[j].y-p->posy);
            p->posxv -= sintable[(j+512)&2047]<<4;
            p->posyv -= sintable[j&2047]<<4;
        }
        if (sprite[j].picnum == RRTILE3587)
        {
            if (!p->at280)
            {
                p->at280 = 10;
#ifdef RRRA
                if ((sb_snum & 2) && !p->OnMotorcycle)
#else
                if (sb_snum & 2)
#endif
                {
                    cz = sprite[j].z;
                    hz = 0;
                    fz = sprite[j].z + (4<<8);
                }
            }
            else
                p->at280--;
        }
        else if (sprite[j].picnum == TOILET || sprite[j].picnum == RRTILE2121)
        {
#ifdef RRRA
            if ((sb_snum & 2) && !p->OnMotorcycle)
#else
            if (sb_snum & 2)
#endif
            if (Sound[436].num == 0)
            {
                spritesound(436,p->i);
                p->last_pissed_time = 4000;
                p->eat = 0;
            }
        }
    }


    if ( s->extra > 0 ) incur_damage( p );
    else
    {
        s->extra = 0;
        p->shield_amount = 0;
    }

    p->last_extra = s->extra;

    if(p->loogcnt > 0) p->loogcnt--;
    else p->loogcnt = 0;

    if(p->fist_incs)
    {
        p->fist_incs++;
        if(p->fist_incs == 28)
        {
            if(ud.recstat == 1) closedemowrite();
            sound(PIPEBOMB_EXPLODE);
            p->pals[0] = 64;
            p->pals[1] = 64;
            p->pals[2] = 64;
            p->pals_time = 48;
        }
        if(p->fist_incs > 42)
        {
            if(p->buttonpalette && ud.from_bonus == 0)
            {
                ud.from_bonus = ud.level_number+1;
                if(ud.secretlevel > 0 && ud.secretlevel < 9) ud.level_number = ud.secretlevel-1;
                ud.m_level_number = ud.level_number;
            }
            else
            {
                if(ud.from_bonus)
                {
                    ud.level_number = ud.from_bonus;
                    ud.m_level_number = ud.level_number;
                    ud.from_bonus = 0;
                }
                else
                {
                    if(ud.level_number == ud.secretlevel && ud.from_bonus > 0 )
                        ud.level_number = ud.from_bonus;
                    else ud.level_number++;

                    if(ud.level_number > 6) ud.level_number = 0;
                    ud.m_level_number = ud.level_number;

                }
            }
            for(i=connecthead;i>=0;i=connectpoint2[i])
                ps[i].gm = MODE_EOL;
            p->fist_incs = 0;
        }
    }

    if(p->timebeforeexit > 1 && p->last_extra > 0)
    {
        p->timebeforeexit--;
        if(p->timebeforeexit == 26*5)
        {
            FX_StopAllSounds();
            clearsoundlocks();
            if(p->customexitsound >= 0)
            {
                sound(p->customexitsound);
                FTA(102,p);
            }
        }
        else if(p->timebeforeexit == 1)
        {
            for(i=connecthead;i>=0;i=connectpoint2[i])
                ps[i].gm = MODE_EOL;

#ifndef RRRA
            if (ud.level_number == 6 && ud.volume_number == 0)
                turdlevel = 1;
#endif
            ud.level_number++;
            ud.m_level_number = ud.level_number;
            return;
        }
    }
/*
    if(p->select_dir)
    {
        if(psectlotag != 15 || (sb_snum&(1<<31)) )
            p->select_dir = 0;
        else
        {
            if(sync[snum].fvel > 127)
            {
                p->select_dir = 0;
                activatewarpelevators(pi,-1);
            }
            else if(sync[snum].fvel <= -127)
            {
                p->select_dir = 0;
                activatewarpelevators(pi,1);
            }
            return;
        }
    }
  */
    if(p->pals_time > 0)
        p->pals_time--;

    if(p->fta > 0)
    {
        p->fta--;
        if(p->fta == 0)
        {
            pub = NUMPAGES;
            pus = NUMPAGES;
            p->ftq = 0;
        }
    }

    if( s->extra <= 0 )
    {
        if(p->dead_flag == 0)
        {
            if(s->pal != 1)
            {
                p->pals[0] = 63;
                p->pals[1] = 0;
                p->pals[2] = 0;
                p->pals_time = 63;
                p->posz -= (16<<8);
                s->z -= (16<<8);
            }

            if(ud.recstat == 1 && ud.multimode < 2)
                closedemowrite();

            if(s->pal != 1)
                p->dead_flag = (512-((TRAND&1)<<10)+(TRAND&255)-512)&2047;

            p->jetpack_on = 0;
            p->holoduke_on = -1;

            //stopsound(DUKE_JETPACK_IDLE);
            if(p->scream_voice > FX_Ok)
            {
                FX_StopSound(p->scream_voice);
                testcallback(DUKE_SCREAM);
                p->scream_voice = FX_Ok;
            }

            if( s->pal != 1 && (s->cstat&32768) == 0) s->cstat = 0;

            if( ud.multimode > 1 && ( s->pal != 1 || (s->cstat&32768) ) )
            {
                if(p->frag_ps != snum)
                {
                    ps[p->frag_ps].frag++;
                    frags[p->frag_ps][snum]++;

                    if( ud.user_name[p->frag_ps][0] != 0)
                    {
                        if(snum == screenpeek)
                        {
                            sprintf(&fta_quotes[115][0],"KILLED BY %s",&ud.user_name[p->frag_ps][0]);
                            FTA(115,p);
                        }
                        else
                        {
                            sprintf(&fta_quotes[116][0],"KILLED %s",&ud.user_name[snum][0]);
                            FTA(116,&ps[p->frag_ps]);
                        }
                    }
                    else
                    {
                        if(snum == screenpeek)
                        {
                            sprintf(&fta_quotes[115][0],"KILLED BY PLAYER %ld",1+p->frag_ps);
                            FTA(115,p);
                        }
                        else
                        {
                            sprintf(&fta_quotes[116][0],"KILLED PLAYER %ld",1+snum);
                            FTA(116,&ps[p->frag_ps]);
                        }
                    }
                }
                else p->fraggedself++;

                if(myconnectindex == connecthead)
                {
                    sprintf(tempbuf,"frag %d killed %d\n",p->frag_ps+1,snum+1);
                    sendscore(tempbuf);
//                    printf(tempbuf);
                }

                p->frag_ps = snum;
                pus = NUMPAGES;
            }
        }

        if( psectlotag == 2 )
        {
            if(p->on_warping_sector == 0)
            {
                if( klabs(p->posz-fz) > (PHEIGHT>>1))
                    p->posz += 348;
            }
            else
            {
                s->z -= 512;
                s->zvel = -348;
            }

            clipmove(&p->posx,&p->posy,
                &p->posz,&p->cursectnum,
                0,0,164L,(4L<<8),(4L<<8),CLIPMASK0);
//            p->bobcounter += 32;
        }

        p->oposx = p->posx;
        p->oposy = p->posy;
        p->oposz = p->posz;
        p->oang = p->ang;
        p->opyoff = p->pyoff;

        p->horiz = 100;
        p->horizoff = 0;

        updatesector(p->posx,p->posy,&p->cursectnum);

        pushmove(&p->posx,&p->posy,&p->posz,&p->cursectnum,128L,(4L<<8),(20L<<8),CLIPMASK0);

        if( fz > cz+(16<<8) && s->pal != 1)
            p->rotscrnang = (p->dead_flag + ( (fz+p->posz)>>7))&2047;

        p->on_warping_sector = 0;

        return;
    }

    if(p->transporter_hold > 0)
    {
        p->transporter_hold--;
        if(p->transporter_hold == 0 && p->on_warping_sector)
            p->transporter_hold = 2;
    }
    if(p->transporter_hold < 0)
        p->transporter_hold++;

    if(p->newowner >= 0)
    {
        i = p->newowner;
        p->posx = SX;
        p->posy = SY;
        p->posz = SZ;
        p->ang =  SA;
        p->posxv = p->posyv = s->xvel = 0;
        p->look_ang = 0;
        p->rotscrnang = 0;

        doincrements(p);

        if(p->curr_weapon == HANDREMOTE_WEAPON) goto SHOOTINCODE;

        return;
    }

    doubvel = TICSPERFRAME;

    if (p->rotscrnang > 0) p->rotscrnang -= ((p->rotscrnang>>1)+1);
    else if (p->rotscrnang < 0) p->rotscrnang += (((-p->rotscrnang)>>1)+1);

    p->look_ang -= (p->look_ang>>2);

#ifdef RRRA
    if( sb_snum&(1<<6) && !p->OnMotorcycle)
#else
    if( sb_snum&(1<<6) )
#endif
    {
        p->look_ang -= 152;
        p->rotscrnang += 24;
    }

#ifdef RRRA
    if( sb_snum&(1<<7) && !p->OnMotorcycle )
#else
    if( sb_snum&(1<<7) )
#endif
    {
        p->look_ang += 152;
        p->rotscrnang -= 24;
    }

#ifdef RRRA
    if (p->SeaSick)
    {
        if (p->SeaSick < 250)
        {
            if (p->SeaSick >= 180)
                p->rotscrnang += 24;
            else if (p->SeaSick >= 130)
                p->rotscrnang -= 24;
            else if (p->SeaSick >= 70)
                p->rotscrnang += 24;
            else if (p->SeaSick >= 20)
                p->rotscrnang += 24;
        }
        if (p->SeaSick < 250)
            p->look_ang += (TRAND&255)-128;
    }
#endif

    if(p->on_crane >= 0)
        goto HORIZONLY;

    j = ksgn(sync[snum].avel);
    /*
    if( j && ud.screen_tilting == 2)
    {
        k = 4;
        if(sb_snum&(1<<5)) k <<= 2;
        p->rotscrnang -= k*j;
        p->look_ang += k*j;
    }
    */

    if( s->xvel < 32 || p->on_ground == 0 || p->bobcounter == 1024 )
    {
        if( (p->weapon_sway&2047) > (1024+96) )
            p->weapon_sway -= 96;
        else if( (p->weapon_sway&2047) < (1024-96) )
            p->weapon_sway += 96;
        else p->weapon_sway = 1024;
    }
    else p->weapon_sway = p->bobcounter;

    s->xvel =
        ksqrt( (p->posx-p->bobposx)*(p->posx-p->bobposx)+(p->posy-p->bobposy)*(p->posy-p->bobposy));
    if(p->on_ground) p->bobcounter += sprite[p->i].xvel>>1;

    if( ud.clipping == 0 && ( sector[p->cursectnum].floorpicnum == MIRROR || p->cursectnum < 0 || p->cursectnum >= MAXSECTORS) )
    {
        p->posx = p->oposx;
        p->posy = p->oposy;
    }
    else
    {
        p->oposx = p->posx;
        p->oposy = p->posy;
    }

    p->bobposx = p->posx;
    p->bobposy = p->posy;

    p->oposz = p->posz;
    p->opyoff = p->pyoff;
    p->oang = p->ang;

    if(p->one_eighty_count < 0)
    {
        p->one_eighty_count += 128;
        p->ang += 128;
    }

    // Shrinking code

    i = 40;

    if (psectlotag == 17)
    {
        int tmp;
        tmp = getanimationgoal(&sector[p->cursectnum].floorz);
        if (tmp >= 0)
        {
            if (Sound[432].num == 0)
                spritesound(432,pi);
        }
        else
            stopsound(432);
    }
#ifdef RRRA
    else if (psectlotag == 18)
    {
        int tmp;
        tmp = getanimationgoal(&sector[p->cursectnum].floorz);
        if (tmp >= 0)
        {
            if (Sound[432].num == 0)
                spritesound(432,pi);
        }
        else
            stopsound(432);
    }
    if (p->raat5dd)
    {
        p->pycount += 32;
        p->pycount &= 2047;
        if (p->SeaSick)
            p->pyoff = sintable[p->pycount]>>2;
        else
            p->pyoff = sintable[p->pycount]>>7;
    }
#endif

    if( psectlotag == 2)
    {
        p->jumping_counter = 0;

        p->pycount += 32;
        p->pycount &= 2047;
        p->pyoff = sintable[p->pycount]>>7;

        if( Sound[DUKE_UNDERWATER].num == 0 )
            spritesound(DUKE_UNDERWATER,pi);

#ifdef RRRA
        if ( (sb_snum&1) && !p->OnMotorcycle )
#else
        if ( sb_snum&1 )
#endif
        {
            if(p->poszv > 0) p->poszv = 0;
            p->poszv -= 348;
            if(p->poszv < -(256*6)) p->poszv = -(256*6);
        }
#ifdef RRRA
        else if ((sb_snum&(1<<1)) && !p->OnMotorcycle)
#else
        else if (sb_snum&(1<<1))
#endif
        {
            if(p->poszv < 0) p->poszv = 0;
            p->poszv += 348;
            if(p->poszv > (256*6)) p->poszv = (256*6);
        }
#ifdef RRRA
        else if (p->OnMotorcycle)
        {
            if(p->poszv < 0) p->poszv = 0;
            p->poszv += 348;
            if(p->poszv > (256*6)) p->poszv = (256*6);
        }
#endif
        else
        {
            if(p->poszv < 0)
            {
                p->poszv += 256;
                if(p->poszv > 0)
                    p->poszv = 0;
            }
            if(p->poszv > 0)
            {
                p->poszv -= 256;
                if(p->poszv < 0)
                    p->poszv = 0;
            }
        }

        if(p->poszv > 2048)
            p->poszv >>= 1;

        p->posz += p->poszv;

        if(p->posz > (fz-(15<<8)) )
            p->posz += ((fz-(15<<8))-p->posz)>>1;

        if(p->posz < (cz+(4<<8)) )
        {
            p->posz = cz+(4<<8);
            p->poszv = 0;
        }

        if( p->scuba_on && (TRAND&255) < 8 )
        {
            j = spawn(pi,WATERBUBBLE);
            sprite[j].x +=
                sintable[(p->ang+512+64-(global_random&128)+128)&2047]>>6;
            sprite[j].y +=
                sintable[(p->ang+64-(global_random&128)+128)&2047]>>6;
            sprite[j].xrepeat = 3;
            sprite[j].yrepeat = 2;
            sprite[j].z = p->posz+(8<<8);
            sprite[j].cstat = 514;
        }
    }
    else if( psectlotag != 2 )
    {
        if(p->airleft != 15*26)
            p->airleft = 15*26; //Aprox twenty seconds.

        if(p->scuba_on == 1)
            p->scuba_on = 0;

        if( psectlotag == 1 && p->spritebridge == 0)
        {
            if(shrunk == 0)
            {
                i = 34;
                p->pycount += 32;
                p->pycount &= 2047;
                p->pyoff = sintable[p->pycount]>>6;
            }
            else i = 12;

            if(shrunk == 0 && truefdist <= PHEIGHT)
            {
                if(p->on_ground == 1)
                {
                    if( p->dummyplayersprite == -1 )
                        p->dummyplayersprite =
                            spawn(pi,PLAYERONWATER);

                    p->footprintcount = 6;
#ifdef RRRA
                    if(sector[p->cursectnum].floorpicnum == FLOORSLIME)
                    {
                        p->footprintpal = 8;
                        p->footprintshade = 0;
                    }
                    else if(sector[p->cursectnum].floorpicnum == RRTILE7756 || sector[p->cursectnum].floorpicnum == RRTILE7888)
                    {
                        p->footprintpal = 0;
                        p->footprintshade = 40;
                    }
                    else
                    {
                        p->footprintpal = 0;
                        p->footprintshade = 0;
                    }
#else
                    if(sector[p->cursectnum].floorpicnum == FLOORSLIME)
                        p->footprintpal = 8;
                    else p->footprintpal = 0;
                    p->footprintshade = 0;
#endif
                }
            }
        }
#ifdef RRRA
        else if (!p->OnMotorcycle)
#else
        else
#endif
        {
            if(p->footprintcount > 0 && p->on_ground)
                if( (sector[p->cursectnum].floorstat&2) != 2 )
            {
                for(j=headspritesect[psect];j>=0;j=nextspritesect[j])
                    if( sprite[j].picnum == FOOTPRINTS || sprite[j].picnum == FOOTPRINTS2 || sprite[j].picnum == FOOTPRINTS3 || sprite[j].picnum == FOOTPRINTS4 )
                        if (klabs(sprite[j].x-p->posx) < 384)
                            if (klabs(sprite[j].y-p->posy) < 384)
                                break;
                if(j < 0)
                {
                    p->footprintcount--;
                    if( sector[p->cursectnum].lotag == 0 && sector[p->cursectnum].hitag == 0 )
                    {
                        switch(TRAND&3)
                        {
                            case 0:  j = spawn(pi,FOOTPRINTS); break;
                            case 1:  j = spawn(pi,FOOTPRINTS2); break;
                            case 2:  j = spawn(pi,FOOTPRINTS3); break;
                            default: j = spawn(pi,FOOTPRINTS4); break;
                        }
                        sprite[j].pal = p->footprintpal;
                        sprite[j].shade = p->footprintshade;
                    }
                }
            }
        }

        if(p->posz < (fz-(i<<8)) ) //falling
        {
            if( (sb_snum&3) == 0 && p->on_ground && (sector[psect].floorstat&2) && p->posz >= (fz-(i<<8)-(16<<8) ) )
                p->posz = fz-(i<<8);
            else
            {
                p->on_ground = 0;
#ifdef RRRA
                if ((p->OnMotorcycle || p->OnBoat) && fz-(i<<8)*2 > p->posz)
                {
                    if (p->MotoOnGround)
                    {
                        p->VBumpTarget = 80;
                        p->raat5c7 = 1;
                        p->poszv -= gc*(p->MotoSpeed>>4);
                        p->MotoOnGround = 0;
                        if (Sound[188].num)
                            stopsound(Sound[188].num);
                        spritesound(189,pi);
                    }
                    else
                    {
                        p->poszv += gc-80+(120-p->MotoSpeed);
                        if (Sound[189].num == 0)
                            if (Sound[190].num == 0)
                                spritesound(190,pi);
                    }
                }
                else
                    p->poszv += (gc+80); // (TICSPERFRAME<<6);
#else
                p->poszv += (gc+80); // (TICSPERFRAME<<6);
#endif
                if(p->poszv >= (4096+2048)) p->poszv = (4096+2048);
                if(p->poszv > 2400 && p->falling_counter < 255)
                {
                    p->falling_counter++;
                    if( p->falling_counter == 38 )
                        p->scream_voice = spritesound(DUKE_SCREAM,pi);
                }

                if( (p->posz+p->poszv) >= (fz-(i<<8)) ) // hit the ground
                    if(sector[p->cursectnum].lotag != 1)
                    {
#ifdef RRRA
                        p->MotoOnGround = 1;
#endif
                        if( p->falling_counter > 62 ) quickkill(p);

#ifdef RRRA
                        else if (p->falling_counter > 2 && sector[p->cursectnum].lotag == 802)
                            quickkill(p);
#endif
                        else if( p->falling_counter > 9 )
                        {
                            j = p->falling_counter;
                            s->extra -= j-(TRAND&3);
                            if(s->extra <= 0)
                            {
                                spritesound(SQUISHED,pi);
                                p->pals[0] = 63;
                                p->pals[1] = 0;
                                p->pals[2] = 0;
                                p->pals_time = 63;
                            }
                            else
                            {
                                spritesound(DUKE_LAND,pi);
                                spritesound(DUKE_LAND_HURT,pi);
                            }

                            p->pals[0] = 16;
                            p->pals[1] = 0;
                            p->pals[2] = 0;
                            p->pals_time = 32;
                        }
#ifdef RRRA
                        else if (p->poszv > 2048)
                        {
                            if (p->OnMotorcycle)
                            {
                                if (Sound[190].num > 0)
                                    stopsound(Sound[190].num);
                                spritesound(191, pi);
                                p->TurbCount = 12;
                            }
                            else spritesound(DUKE_LAND,pi);
                        }
                        else if (p->poszv > 1024 && p->OnMotorcycle)
                        {
                            spritesound(DUKE_LAND,pi);
                            p->TurbCount = 12;
                        }
#else
                        else if(p->poszv > 2048) spritesound(DUKE_LAND,pi);
#endif
                    }
            }
        }

        else
        {
            p->falling_counter = 0;
            if(p->scream_voice > FX_Ok)
            {
                FX_StopSound(p->scream_voice);
                p->scream_voice = FX_Ok;
            }

            if(psectlotag != 1 && psectlotag != 2 && p->on_ground == 0 && p->poszv > (6144>>1))
                p->hard_landing = p->poszv>>10;

            p->on_ground = 1;

            if( i==40 )
            {
                //Smooth on the ground

                k = ((fz-(i<<8))-p->posz)>>1;
                if( klabs(k) < 256 ) k = 0;
                p->posz += k;
                p->poszv -= 768;
                if(p->poszv < 0) p->poszv = 0;
            }
            else if(p->jumping_counter == 0)
            {
                p->posz += ((fz-(i<<7))-p->posz)>>1; //Smooth on the water
                if(p->on_warping_sector == 0 && p->posz > fz-(16<<8))
                {
                    p->posz = fz-(16<<8);
                    p->poszv >>= 1;
                }
            }

            p->on_warping_sector = 0;

#ifdef RRRA
            if( (sb_snum&2) && !p->OnMotorcycle )
#else
            if( (sb_snum&2) )
#endif
            {
                p->posz += (2048+768);
                p->crack_time = 777;
            }

#ifdef RRRA
            if( (sb_snum&1) == 0 && !p->OnMotorcycle && p->jumping_toggle == 1)
#else
            if( (sb_snum&1) == 0 && p->jumping_toggle == 1)
#endif
                p->jumping_toggle = 0;

#ifdef RRRA
            else if( (sb_snum&1) && !p->OnMotorcycle && p->jumping_toggle == 0 )
#else
            else if( (sb_snum&1) && p->jumping_toggle == 0 )
#endif
            {
                if( p->jumping_counter == 0 )
                    if( (fz-cz) > (56<<8) )
                    {
                        p->jumping_counter = 1;
                        p->jumping_toggle = 1;
                    }
            }
        }

        if(p->jumping_counter)
        {
#ifdef RRRA
            if( (sb_snum&1) == 0 && !p->OnMotorcycle && p->jumping_toggle == 1)
#else
            if( (sb_snum&1) == 0 && p->jumping_toggle == 1)
#endif
                p->jumping_toggle = 0;

            if( p->jumping_counter < 768 )
            {
                if(psectlotag == 1 && p->jumping_counter > 768)
                {
                    p->jumping_counter = 0;
                    p->poszv = -512;
                }
                else
                {
                    p->poszv -= (sintable[(2048-128+p->jumping_counter)&2047])/12;
                    p->jumping_counter += 180;
                    p->on_ground = 0;
                }
            }
            else
            {
                p->jumping_counter = 0;
                p->poszv = 0;
            }
        }

        p->posz += p->poszv;

        if(p->posz < (cz+(4<<8)))
        {
            p->jumping_counter = 0;
            if(p->poszv < 0)
                p->posxv = p->posyv = 0;
            p->poszv = 128;
            p->posz = cz+(4<<8);
        }
    }

    //Do the quick lefts and rights

    if ( p->fist_incs ||
         p->transporter_hold > 2 ||
         p->hard_landing ||
         p->access_incs > 0 ||
         p->knee_incs > 0 )
    {
        doubvel = 0;
        p->posxv = 0;
        p->posyv = 0;
    }
    else if ( sync[snum].avel )          //p->ang += syncangvel * constant
    {                         //ENGINE calculates angvel for you
        long tempang;

        tempang = sync[snum].avel<<1;

        if( psectlotag == 2 ) p->angvel =(tempang-(tempang>>3))*sgn(doubvel);
        else p->angvel = tempang*sgn(doubvel);

        p->ang += p->angvel;
        p->ang &= 2047;
        p->crack_time = 777;
    }

    if(p->spritebridge == 0)
    {
        j = sector[s->sectnum].floorpicnum;
        k = 0;

        if(p->on_ground && truefdist <= PHEIGHT+(16<<8))
        {
            switch(j)
            {
                case HURTRAIL:
                    if( rnd(32) )
                    {
                        if(p->boot_amount > 0)
                            k = 1;
                        else
                        {
                            if(Sound[DUKE_LONGTERM_PAIN].num < 1)
                                spritesound(DUKE_LONGTERM_PAIN,pi);
                            p->pals[0] = 64; p->pals[1] = 64; p->pals[2] = 64;
                            p->pals_time = 32;
                            s->extra -= 1+(TRAND&3);
                            if(Sound[SHORT_CIRCUIT].num < 1)
                                spritesound(SHORT_CIRCUIT,pi);
                        }
                    }
                    break;
                case FLOORSLIME:
                    if( rnd(16) )
                    {
                        if(p->boot_amount > 0)
                            k = 1;
                        else
                        {
                            if(Sound[DUKE_LONGTERM_PAIN].num < 1)
                                spritesound(DUKE_LONGTERM_PAIN,pi);
                            p->pals[0] = 0; p->pals[1] = 8; p->pals[2] = 0;
                            p->pals_time = 32;
                            s->extra -= 1+(TRAND&3);
                        }
                    }
                    break;
                case FLOORPLASMA:
                    if( rnd(32) )
                    {
                        if( p->boot_amount > 0 )
                            k = 1;
                        else
                        {
                            if(Sound[DUKE_LONGTERM_PAIN].num < 1)
                                spritesound(DUKE_LONGTERM_PAIN,pi);
                            p->pals[0] = 8; p->pals[1] = 0; p->pals[2] = 0;
                            p->pals_time = 32;
                            s->extra -= 1+(TRAND&3);
                        }
                    }
                    break;
#ifdef RRRA
                case RRTILE7768:
                case RRTILE7820:
                    if ((TRAND & 3) == 1)
                        if (p->on_ground)
                        {
                            if (p->OnMotorcycle)
                                s->extra -= 2;
                            else
                                s->extra -= 4;
                            spritesound(DUKE_LONGTERM_PAIN,pi);
                        }
                    break;
#endif
            }
        }

        if( k )
        {
            FTA(75,p);
            p->boot_amount -= 2;
            if(p->boot_amount <= 0)
                checkavailinven(p);
        }
    }

    if ( p->posxv || p->posyv || sync[snum].fvel || sync[snum].svel )
    {
        p->crack_time = 777;

        k = sintable[p->bobcounter&2047]>>12;

#ifdef RRRA
        if (p->spritebridge == 0 && p->on_ground)
        {
            if (psectlotag == 1)
                p->NotOnWater = 0;
            else if (p->OnBoat)
            {
                if (psectlotag == 1234)
                    p->NotOnWater = 0;
                else
                    p->NotOnWater = 1;
            }
            else
                p->NotOnWater = 1;
        }
#endif

        if(truefdist < PHEIGHT+(8<<8) )
            if( k == 1 || k == 3 )
        {
            if(p->spritebridge == 0 && p->walking_snd_toggle == 0 && p->on_ground)
            {
                switch( psectlotag )
                {
                    case 0:

                        if(lz >= 0 && (lz&(MAXSPRITES-1))==49152 )
                            j = sprite[lz&(MAXSPRITES-1)].picnum;
                        else j = sector[psect].floorpicnum;
                        break;
                    case 1:
                        if((TRAND&1) == 0)
#ifdef RRRA
                            if (!p->OnBoat && !p->OnMotorcycle && sector[p->cursectnum].hitag != 321)
#endif
                            spritesound(DUKE_ONWATER,pi);
                        p->walking_snd_toggle = 1;
                        break;
                }
            }
        }
        else if(p->walking_snd_toggle > 0)
            p->walking_snd_toggle --;

        if(p->jetpack_on == 0 && p->steroids_amount > 0 && p->steroids_amount < 400)
            doubvel <<= 1;

        p->posxv += ((sync[snum].fvel*doubvel)<<6);
        p->posyv += ((sync[snum].svel*doubvel)<<6);

#ifndef RRRA
        if( ( p->curr_weapon == KNEE_WEAPON && *kb > 10 && p->on_ground ) || ( p->on_ground && (sb_snum&2) ) )
        {
            p->posxv = mulscale(p->posxv,rdnkfriction-0x2000,16);
            p->posyv = mulscale(p->posyv,rdnkfriction-0x2000,16);
        }
        else
        {
#endif
            if(psectlotag == 2)
            {
                p->posxv = mulscale(p->posxv,rdnkfriction-0x1400,16);
                p->posyv = mulscale(p->posyv,rdnkfriction-0x1400,16);
            }
            else
            {
                p->posxv = mulscale(p->posxv,rdnkfriction,16);
                p->posyv = mulscale(p->posyv,rdnkfriction,16);
            }
#ifndef RRRA
        }
#endif

#ifdef RRRA
        if (sector[psect].floorpicnum == RRTILE7888)
        {
            if (p->OnMotorcycle)
                if (p->on_ground)
                    p->raat5cf = 1;
        }
        else if (sector[psect].floorpicnum == RRTILE7889)
        {
            if (p->OnMotorcycle)
            {
                if (p->on_ground)
                    p->raat5cd = 1;
            }
            else if (p->boot_amount > 0)
                p->boot_amount--;
            else
            {
                p->posxv = mulscale(p->posxv,rdnkfriction,16);
                p->posyv = mulscale(p->posyv,rdnkfriction,16);
            }
        }
        else
#endif

        if (sector[psect].floorpicnum == RRTILE3073 || sector[psect].floorpicnum == RRTILE2702)
        {
#ifdef RRRA
            if (p->OnMotorcycle)
            {
                if (p->on_ground)
                {
                    p->posxv = mulscale(p->posxv,rdnkfriction-0x1800,16);
                    p->posyv = mulscale(p->posyv,rdnkfriction-0x1800,16);
                }
            }
            else
#endif
            if (p->boot_amount > 0)
                p->boot_amount--;
            else
            {
                p->posxv = mulscale(p->posxv,rdnkfriction-0x1800,16);
                p->posyv = mulscale(p->posyv,rdnkfriction-0x1800,16);
            }
        }

        if( abs(p->posxv) < 2048 && abs(p->posyv) < 2048 )
            p->posxv = p->posyv = 0;

        if( shrunk )
        {
            p->posxv =
                mulscale16(p->posxv,rdnkfriction-(rdnkfriction>>1)+(rdnkfriction>>2));
            p->posyv =
                mulscale16(p->posyv,rdnkfriction-(rdnkfriction>>1)+(rdnkfriction>>2));
        }
    }

    HORIZONLY:

        if(psectlotag == 1 || p->spritebridge == 1) i = (4L<<8);
        else i = (20L<<8);

        if(sector[p->cursectnum].lotag == 2) k = 0;
        else k = 1;

        if(ud.clipping)
        {
            j = 0;
            p->posx += p->posxv>>14;
            p->posy += p->posyv>>14;
            updatesector(p->posx,p->posy,&p->cursectnum);
            changespritesect(pi,p->cursectnum);
        }
        else
            j = clipmove(&p->posx,&p->posy,
                &p->posz,&p->cursectnum,
                p->posxv,p->posyv,164L,(4L<<8),i,CLIPMASK0);

        if(p->jetpack_on == 0 && psectlotag != 2 && psectlotag != 1 && shrunk)
            p->posz += 32<<8;

        if(j)
            checkplayerhurt(p,j);
#ifdef RRRA
        else if (p->hurt_delay2 > 0)
            p->hurt_delay2--;
#endif

        var60 = j&(MAXWALLS-1);
        var60 = wall[j&(MAXWALLS-1)].lotag;

        if ((j & 49152) == 32768)
        {
#ifdef RRRA
            if (p->OnMotorcycle)
            {
                short var104, var108, var10c;
                var104 = 0;
                j &= (MAXWALLS-1);
                var108 = getangle(wall[wall[j].point2].x-wall[j].x,wall[wall[j].point2].y-wall[j].y);
                var10c = abs(p->ang-var108);
                switch (TRAND&1)
                {
                    case 0:
                        p->ang += p->MotoSpeed>>1;
                        break;
                    case 1:
                        p->ang -= p->MotoSpeed>>1;
                        break;
                }
                if (var10c >= 441 && var10c <= 581)
                {
                    var104 = (p->MotoSpeed*p->MotoSpeed)>>8;
                    p->MotoSpeed = 0;
                    if (Sound[238].num == 0)
                        spritesound(238,p->i);
                }
                else if (var10c >= 311 && var10c <= 711)
                {
                    var104 = (p->MotoSpeed*p->MotoSpeed)>>11;
                    p->MotoSpeed -= (p->MotoSpeed>>1)+(p->MotoSpeed>>2);
                    if (Sound[238].num == 0)
                        spritesound(238,p->i);
                }
                else if (var10c >= 111 && var10c <= 911)
                {
                    var104 = (p->MotoSpeed*p->MotoSpeed)>>14;
                    p->MotoSpeed -= (p->MotoSpeed>>1);
                    if (Sound[239].num == 0)
                        spritesound(239,p->i);
                }
                else
                {
                    var104 = (p->MotoSpeed*p->MotoSpeed)>>15;
                    p->MotoSpeed -= (p->MotoSpeed>>3);
                    if (Sound[240].num == 0)
                        spritesound(240,p->i);
                }
                s->extra -= var104;
                if (s->extra <= 0)
                {
                    spritesound(SQUISHED,pi);
                    p->pals[0] = 63;
                    p->pals[1] = 0;
                    p->pals[2] = 0;
                    p->pals_time = 63;
                }
                else if (var104)
                    spritesound(DUKE_LAND_HURT,pi);
            }
            else if (p->OnBoat)
            {
                short var114, var118, var11c;
                j &= (MAXWALLS-1);
                var114 = getangle(wall[wall[j].point2].x-wall[j].x,wall[wall[j].point2].y-wall[j].y);
                var118 = abs(p->ang-var114);
                switch (TRAND&1)
                {
                    case 0:
                        p->ang += p->MotoSpeed>>2;
                        break;
                    case 1:
                        p->ang -= p->MotoSpeed>>2;
                        break;
                }
                if (var118 >= 441 && var118 <= 581)
                {
                    p->MotoSpeed = ((p->MotoSpeed>>1)+(p->MotoSpeed>>2))>>2;
                    if (psectlotag == 1)
                        if (Sound[178].num == 0)
                            spritesound(178,p->i);
                }
                else if (var118 >= 311 && var118 <= 711)
                {
                    p->MotoSpeed -= ((p->MotoSpeed>>1)+(p->MotoSpeed>>2))>>3;
                    if (psectlotag == 1)
                        if (Sound[179].num == 0)
                            spritesound(179,p->i);
                }
                else if (var118 >= 111 && var118 <= 911)
                {
                    p->MotoSpeed -= (p->MotoSpeed>>4);
                    if (psectlotag == 1)
                        if (Sound[180].num == 0)
                            spritesound(180,p->i);
                }
                else
                {
                    p->MotoSpeed -= (p->MotoSpeed>>6);
                    if (psectlotag == 1)
                        if (Sound[181].num == 0)
                            spritesound(181,p->i);
                }
            }
            else
            {
#endif
            if (wall[j&(MAXWALLS-1)].lotag >= 40 && wall[j&(MAXWALLS-1)].lotag <= 44)
            {
                if (wall[j&(MAXWALLS-1)].lotag < 44)
                {
                    dofurniture(j&(MAXWALLS-1),p->cursectnum,snum);
                    pushmove(&p->posx,&p->posy,&p->posz,&p->cursectnum,172L,(4L<<8),(4L<<8),CLIPMASK0);
                }
                else
                    pushmove(&p->posx,&p->posy,&p->posz,&p->cursectnum,172L,(4L<<8),(4L<<8),CLIPMASK0);
            }
#ifdef RRRA
            }
#endif
        }

        if ((j & 49152) == 49152)
        {
            var60 = j&(MAXSPRITES-1);
#ifdef RRRA
            if (p->OnMotorcycle)
            {
                if (badguy(&sprite[var60]) || sprite[var60].picnum == APLAYER)
                {
                    if (sprite[var60].picnum != APLAYER)
                    {
                        if (numplayers == 1)
                        {
                            movesprite(var60,sintable[(p->TiltStatus*20+p->ang+512)&2047]>>8,
                                sintable[(p->TiltStatus*20+p->ang)&2047]>>8,sprite[var60].zvel,CLIPMASK0);
                        }
                    }
                    else
                        hittype[var60].owner = p->i;
                    hittype[var60].picnum = MOTOHIT;
                    hittype[var60].extra = p->MotoSpeed>>1;
                    p->MotoSpeed -= p->MotoSpeed>>2;
                    p->TurbCount = 6;
                }
                else if ((sprite[var60].picnum == RRTILE2431 || sprite[var60].picnum == RRTILE2443 || sprite[var60].picnum == RRTILE2451 || sprite[var60].picnum == RRTILE2455)
                    && sprite[var60].picnum != ACTIVATORLOCKED && p->MotoSpeed > 45)
                {
                    spritesound(SQUISHED,var60);
                    if (sprite[var60].picnum == RRTILE2431 || sprite[var60].picnum == RRTILE2451)
                    {
                        if (sprite[var60].lotag != 0)
                        {
                            for(j = 0; j < MAXSPRITES; j++)
                            {
                                if ((sprite[j].picnum == RRTILE2431 || sprite[j].picnum == RRTILE2451) && sprite[j].pal == 4)
                                {
                                    if (sprite[var60].lotag == sprite[j].lotag)
                                    {
                                        sprite[j].xrepeat = 0;
                                        sprite[j].yrepeat = 0;
                                    }
                                }
                            }
                        }
                        guts(&sprite[var60],RRTILE2460,12,myconnectindex);
                        guts(&sprite[var60],RRTILE2465,3,myconnectindex);
                    }
                    else
                        guts(&sprite[var60],RRTILE2465,3,myconnectindex);
                    guts(&sprite[var60],RRTILE2465,3,myconnectindex);
                    sprite[var60].xrepeat = 0;
                    sprite[var60].yrepeat = 0;
                }
            }
            else if (p->OnBoat)
            {
                if (badguy(&sprite[var60]) || sprite[var60].picnum == APLAYER)
                {
                    if (sprite[var60].picnum != APLAYER)
                    {
                        if (numplayers == 1)
                        {
                            movesprite(var60,sintable[(p->TiltStatus*20+p->ang+512)&2047]>>9,
                                sintable[(p->TiltStatus*20+p->ang)&2047]>>9,sprite[var60].zvel,CLIPMASK0);
                        }
                    }
                    else
                        hittype[var60].owner = p->i;
                    hittype[var60].picnum = MOTOHIT;
                    hittype[var60].extra = p->MotoSpeed>>2;
                    p->MotoSpeed -= p->MotoSpeed>>2;
                    p->TurbCount = 6;
                }
            }
            else
#endif
            if (badguy(&sprite[var60]))
            {
                if (sprite[var60].statnum != 1)
                {
                    hittype[var60].timetosleep = 0;
                    if (sprite[var60].picnum == BILLYRAY)
                        spritesound(404,var60);
                    else
                        check_fta_sounds(var60);
                    changespritestat(var60,1);
                }
            }
#ifndef RRRA
            else
#endif
            if(sprite[var60].picnum == RRTILE3410)
            {
                quickkill(p);
                spritesound(446,pi);
            }
#ifdef RRRA
            else if (sprite[var60].picnum == RRTILE2443 && sprite[var60].pal == 19)
            {
                sprite[var60].pal = 0;
                p->DrugMode = 5;
                sprite[ps[snum].i].extra = max_player_health;
            }
#endif
        }


        if(p->jetpack_on == 0)
        {
            if( s->xvel > 16 )
            {
#ifdef RRRA
                if( psectlotag != 1 && psectlotag != 2 && p->on_ground && !p->raat5dd )
#else
                if( psectlotag != 1 && psectlotag != 2 && p->on_ground )
#endif
                {
                    p->pycount += 52;
                    p->pycount &= 2047;
                    p->pyoff =
                        klabs(s->xvel*sintable[p->pycount])/1596;
                }
            }
#ifdef RRRA
            else if( psectlotag != 2 && psectlotag != 1 && !p->raat5dd)
#else
            else if( psectlotag != 2 && psectlotag != 1 )
#endif
                p->pyoff = 0;
        }

        // RBG***
        setsprite(pi,p->posx,p->posy,p->posz+PHEIGHT);

#ifdef RRRA
        if (psectlotag == 800 && !p->raat60b)
#else
        if (psectlotag == 800)
#endif
        {
#ifdef RRRA
            p->raat60b = 1;
#endif
            quickkill(p);
            return;
        }

        if( psectlotag < 3 )
        {
            psect = s->sectnum;
            if( ud.clipping == 0 && sector[psect].lotag == 31)
            {
                if( sprite[sector[psect].hitag].xvel && hittype[sector[psect].hitag].temp_data[0] == 0)
                {
                    quickkill(p);
                    return;
                }
            }
        }

        if(truefdist < PHEIGHT && p->on_ground && psectlotag != 1 && shrunk == 0 && sector[p->cursectnum].lotag == 1)
            if( Sound[DUKE_ONWATER].num == 0 )
#ifdef RRRA
                if (!p->OnBoat && !p->OnMotorcycle && sector[p->cursectnum].hitag != 321)
#endif
                spritesound(DUKE_ONWATER,pi);

        if (p->cursectnum != s->sectnum)
            changespritesect(pi,p->cursectnum);

        if(ud.clipping == 0)
        {
            if (s->clipdist == 64)
                j = ( pushmove(&p->posx,&p->posy,&p->posz,&p->cursectnum,128L,(4L<<8),(4L<<8),CLIPMASK0) < 0 && furthestangle(pi,8) < 512 );
            else
                j = ( pushmove(&p->posx,&p->posy,&p->posz,&p->cursectnum,16L,(4L<<8),(4L<<8),CLIPMASK0) < 0 && furthestangle(pi,8) < 512 );
        }
        else j = 0;

        if(ud.clipping == 0)
        {
            if( klabs(hittype[pi].floorz-hittype[pi].ceilingz) < (48<<8) || j )
            {
                if ( !(sector[s->sectnum].lotag&0x8000) && ( isanunderoperator(sector[s->sectnum].lotag) ||
                    isanearoperator(sector[s->sectnum].lotag) ) )
                        activatebysector(s->sectnum,snum);
                if(j)
                {
                    quickkill(p);
                    return;
                }
            }
            else if( klabs(fz-cz) < (32<<8) && isanunderoperator(sector[psect].lotag) )
                activatebysector(psect,snum);
        }

        if (ud.clipping == 0 && sector[p->cursectnum].ceilingz > (sector[p->cursectnum].floorz-(12<<8)))
        {
            quickkill(p);
            return;
        }

        if( sb_snum&(1<<18) || p->hard_landing)
            p->return_to_center = 9;

        if( sb_snum&(1<<13) )
        {
            p->return_to_center = 9;
            if( sb_snum&(1<<5) ) p->horiz += 12;
            p->horiz += 12;
        }

        else if( sb_snum&(1<<14) )
        {
            p->return_to_center = 9;
            if( sb_snum&(1<<5) ) p->horiz -= 12;
            p->horiz -= 12;
        }

#ifdef RRRA
        else if( (sb_snum&(1<<3)) && !p->OnMotorcycle )
#else
        else if( sb_snum&(1<<3) )
#endif
        {
            if( sb_snum&(1<<5) ) p->horiz += 6;
            p->horiz += 6;
        }
        
#ifdef RRRA
        else if( (sb_snum&(1<<4)) && !p->OnMotorcycle )
#else
        else if( sb_snum&(1<<4) )
#endif
        {
            if( sb_snum&(1<<5) ) p->horiz -= 6;
            p->horiz -= 6;
        }
        if (p->at59b && p->kickback_pic == 0)
        {
            short d = p->at59b >> 1;
            if (!d)
                d = 1;
            p->at59b -= d;
            p->horiz -= d;
        }
        else if(p->return_to_center > 0)
            if( (sb_snum&(1<<13)) == 0 && (sb_snum&(1<<14)) == 0 )
        {
            p->return_to_center--;
            p->horiz += 33-(p->horiz/3);
        }

        if(p->hard_landing > 0)
        {
            p->hard_landing--;
            p->horiz -= (p->hard_landing<<4);
        }

        if(p->aim_mode)
            p->horiz += sync[snum].horz>>1;
        else if (!p->at59b)
        {
             if( p->horiz > 95 && p->horiz < 105) p->horiz = 100;
             if( p->horizoff > -5 && p->horizoff < 5) p->horizoff = 0;
        }

        if(p->horiz > 299) p->horiz = 299;
        else if(p->horiz < -99) p->horiz = -99;

    //Shooting code/changes

    if( p->show_empty_weapon > 0)
        p->show_empty_weapon--;

    if (p->show_empty_weapon == 1)
    {
        addweapon(p,p->last_full_weapon);
        return;
    }

    if(p->knee_incs > 0)
    {
        p->knee_incs++;
        p->horiz -= 48;
        p->return_to_center = 9;
        if(p->knee_incs > 15)
        {
            p->knee_incs = 0;
            p->holster_weapon = 0;
            if(p->weapon_pos < 0)
                p->weapon_pos = -p->weapon_pos;
            if(p->actorsqu >= 0 && dist(&sprite[pi],&sprite[p->actorsqu]) < 1400 )
            {
                guts(&sprite[p->actorsqu],JIBS6,7,myconnectindex);
                spawn(p->actorsqu,BLOODPOOL);
                spritesound(SQUISHED,p->actorsqu);
                switch(sprite[p->actorsqu].picnum)
                {
                    case FEM10:
                    case NAKED1:
                    case STATUE:
                        if(sprite[p->actorsqu].yvel)
                            operaterespawns(sprite[p->actorsqu].yvel);
                        break;
                }

                if(sprite[p->actorsqu].picnum == APLAYER)
                {
                    quickkill(&ps[sprite[p->actorsqu].yvel]);
                    ps[sprite[p->actorsqu].yvel].frag_ps = snum;
                }
                else if(badguy(&sprite[p->actorsqu]))
                {
                    deletesprite(p->actorsqu);
                    p->actors_killed++;
                }
                else deletesprite(p->actorsqu);
            }
            p->actorsqu = -1;
        }
        else if(p->actorsqu >= 0)
            p->ang += getincangle(p->ang,getangle(sprite[p->actorsqu].x-p->posx,sprite[p->actorsqu].y-p->posy))>>2;
    }

    if( doincrements(p) ) return;
    
    if(p->weapon_pos != 0)
    {
        if(p->weapon_pos == -9)
        {
            if(p->last_weapon >= 0)
            {
                p->weapon_pos = 10;
//                if(p->curr_weapon == KNEE_WEAPON) *kb = 1;
                p->last_weapon = -1;
            }
            else if(p->holster_weapon == 0)
                p->weapon_pos = 10;
        }
        else p->weapon_pos--;
    }

    // HACKS

    SHOOTINCODE:
    
    if (p->at57e > 0)
    {
        if (ud.god)
        {
            p->at57c = 45;
            p->at57e = 0;
        }
        else if (p->at57c <= 0 && (*kb) < 5)
        {
            sound(14);
            quickkill(p);
        }
    }

#ifdef RRRA
    
    if( p->curr_weapon == KNEE_WEAPON || p->curr_weapon == RA15_WEAPON)
        p->random_club_frame += 64;
#endif

    if( p->curr_weapon == SHRINKER_WEAPON || p->curr_weapon == GROW_WEAPON)
        p->random_club_frame += 64; // Glowing
    
    if( p->curr_weapon == TRIPBOMB_WEAPON || p->curr_weapon == BOWLING_WEAPON)
        p->random_club_frame += 64;

    if(p->rapid_fire_hold == 1)
    {
        if( sb_snum&(1<<2) ) return;
        p->rapid_fire_hold = 0;
    }

    if(shrunk || p->tipincs || p->access_incs)
        sb_snum &= ~(1<<2);
    else if ( shrunk == 0 && (sb_snum&(1<<2)) && (*kb) == 0 && p->fist_incs == 0 &&
         p->last_weapon == -1 && ( p->weapon_pos == 0 || p->holster_weapon == 1 ) )
    {

        p->crack_time = 777;

        if(p->holster_weapon == 1)
        {
            if( p->last_pissed_time <= (26*218) && p->weapon_pos == -9)
            {
                p->holster_weapon = 0;
                p->weapon_pos = 10;
                FTA(74,p);
            }
        }
        else switch(p->curr_weapon)
        {
            case HANDBOMB_WEAPON:
                p->hbomb_hold_delay = 0;
                if( p->ammo_amount[HANDBOMB_WEAPON] > 0 )
                    (*kb)=1;
                break;
            case HANDREMOTE_WEAPON:
                p->hbomb_hold_delay = 0;
                (*kb) = 1;
                break;

            case PISTOL_WEAPON:
                if( p->ammo_amount[PISTOL_WEAPON] > 0 )
                {
                    p->ammo_amount[PISTOL_WEAPON]--;
                    (*kb) = 1;
                }
                break;


            case CHAINGUN_WEAPON:
                if( p->ammo_amount[CHAINGUN_WEAPON] > 0 ) // && p->random_club_frame == 0)
                    (*kb)=1;
                break;

            case SHOTGUN_WEAPON:
                if( p->ammo_amount[SHOTGUN_WEAPON] > 0 && p->random_club_frame == 0 )
                    (*kb)=1;
                break;

            case TRIPBOMB_WEAPON:
            case BOWLING_WEAPON:
                if (p->curr_weapon == BOWLING_WEAPON)
                {
                    if (p->ammo_amount[BOWLING_WEAPON] > 0)
                        (*kb) = 1;
                }
                else if (p->ammo_amount[TRIPBOMB_WEAPON] > 0)
                    (*kb) = 1;
                break;

            case SHRINKER_WEAPON:
            case GROW_WEAPON:
                if( p->curr_weapon == GROW_WEAPON )
                {
                    if( p->ammo_amount[GROW_WEAPON] > 0 )
                    {
                        (*kb) = 1;
                        spritesound(431,pi);
                    }
                }
                else if( p->ammo_amount[SHRINKER_WEAPON] > 0)
                {
                    (*kb) = 1;
                    spritesound(SHRINKER_FIRE,pi);
                }
                break;

            case FREEZE_WEAPON:
                if( p->ammo_amount[FREEZE_WEAPON] > 0 )
                    (*kb) = 1;
                break;
            case DEVISTATOR_WEAPON:
                if( p->ammo_amount[DEVISTATOR_WEAPON] > 0 )
                {
                    (*kb) = 1;
                    p->hbomb_hold_delay = !p->hbomb_hold_delay;
                }
                break;

#ifdef RRRA
            case RA13_WEAPON:
                if( p->ammo_amount[RA13_WEAPON] > 0 )
                {
                    (*kb) = 1;
                    p->hbomb_hold_delay = !p->hbomb_hold_delay;
                }
                break;

            case RA14_WEAPON:
                if( p->ammo_amount[RA14_WEAPON] > 0 )
                    (*kb) = 1;
                break;
#endif

            case RPG_WEAPON:
                if ( p->ammo_amount[RPG_WEAPON] > 0)
                    (*kb) = 1;
                break;

#ifdef RRRA
            case RA16_WEAPON:
                if ( p->ammo_amount[RA16_WEAPON] > 0)
                    (*kb) = 1;
                break;

            case KNEE_WEAPON:
            case RA15_WEAPON:
                if (p->curr_weapon == RA15_WEAPON)
                {
                    if (p->ammo_amount[RA15_WEAPON] > 0)
                        if (p->quick_kick == 0)
                            (*kb) = 1;
                }
                else if (p->ammo_amount[KNEE_WEAPON] > 0)
                    if (p->quick_kick == 0)
                        (*kb) = 1;
                break;
#else

            case KNEE_WEAPON:
                if(p->quick_kick == 0) (*kb) = 1;
                break;
#endif
        }
    }
    else if((*kb))
    {
        switch( p->curr_weapon )
        {
            case HANDBOMB_WEAPON:

                if ((*kb) == 1)
                    sound(401);
                if( (*kb) == 6 && (sb_snum&(1<<2)) )
                    p->rapid_fire_hold = 1;
                (*kb)++;
                if( (*kb) > 19 )
                {
                    (*kb) = 0;
                    p->curr_weapon = HANDREMOTE_WEAPON;
                    p->last_weapon = -1;
                    p->weapon_pos = 10;
                    p->at57c = 45;
                    p->at57e = 1;
                    sound(402);
                }

                break;


            case HANDREMOTE_WEAPON:

                (*kb)++;

                if (p->at57c < 0)
                {
                    p->hbomb_on = 0;
                }

                if ((*kb) == 39)
                {
                    p->hbomb_on = 0;
                    p->at290 = 8192;
                    madenoise(snum);
                }
                if ((*kb) == 12)
                {
                    p->ammo_amount[HANDBOMB_WEAPON]--;
                    if (p->ammo_amount[RPG_WEAPON])
                        p->ammo_amount[RPG_WEAPON]--;
#ifdef RRRA
                    if(p->on_ground && (sb_snum&2) && !p->OnMotorcycle)
#else
                    if(p->on_ground && (sb_snum&2) )
#endif
                    {
                        k = 15;
                        i = ((p->horiz+p->horizoff-100)*20);
                    }
                    else
                    {
                        k = 140;
                        i = -512-((p->horiz+p->horizoff-100)*20);
                    }

                    j = EGS(p->cursectnum,
                        p->posx+(sintable[(p->ang+512)&2047]>>6),
                        p->posy+(sintable[p->ang&2047]>>6),
                        p->posz,HEAVYHBOMB,-16,9,9,
                        p->ang,(k+(p->hbomb_hold_delay<<5))*2,i,pi,1);

                    if(k == 15)
                    {
                        sprite[j].yvel = 3;
                        sprite[j].z += (8<<8);
                    }

                    k = hits(pi);
                    if( k < 512 )
                    {
                        sprite[j].ang += 1024;
                        sprite[j].zvel /= 3;
                        sprite[j].xvel /= 3;
                    }

                    p->hbomb_on = 1;
                }
                else if ((*kb) < 12 && (sb_snum&4))
                    p->hbomb_hold_delay++;

                if((*kb) == 40)
                {
                    (*kb) = 0;
                    p->curr_weapon = HANDBOMB_WEAPON;
                    p->last_weapon = -1;
                    p->at57e = 0;
                    p->at57c = 45;
                    if(p->ammo_amount[HANDBOMB_WEAPON] > 0)
                    {
                        addweapon(p,HANDBOMB_WEAPON);
                        p->weapon_pos = -9;
                    }
                    else checkavailweapon(p);
                }
                break;

            case PISTOL_WEAPON:
                if( (*kb)==1)
                {
                    shoot(pi,SHOTSPARK1);
                    spritesound(PISTOL_FIRE,pi);
                    p->at290 = 8192;
                    madenoise(snum);

                    lastvisinc = totalclock+32;
                    p->visibility = 0;
                    if (psectlotag != 857)
                    {
                        p->posxv -= sintable[(p->ang+512)&2047]<<4;
                        p->posyv -= sintable[p->ang&2047]<<4;
                    }
                }
                else if((*kb) == 2)
                    if (p->ammo_amount[PISTOL_WEAPON] <= 0)
                {
                    (*kb)=0;
                    checkavailweapon(p);
                }

                (*kb)++;

                if((*kb) >= 22)
                {
                    if( p->ammo_amount[PISTOL_WEAPON] <= 0 )
                    {
                        (*kb)=0;
                        checkavailweapon(p);
                        break;
                    }
                    else if ((p->ammo_amount[PISTOL_WEAPON]%6) == 0)
                    {
                        switch((*kb))
                        {
                            case 24:
                                spritesound(EJECT_CLIP,pi);
                                break;
                            case 30:
                                spritesound(INSERT_CLIP,pi);
                                break;
                        }
                    }
                    else
                        (*kb) = 38;
                }

                if((*kb) == 38)
                {
                    (*kb) = 0;
                    checkavailweapon(p);
                }

                break;

            case SHOTGUN_WEAPON:

                (*kb)++;

                if ((*kb) == 6)
                    if (p->at599 == 0)
                        if (p->ammo_amount[SHOTGUN_WEAPON] > 1)
                            if (sb_snum & 4)
                    p->at59a = 1;

                if(*kb == 4)
                {
                    shoot(pi,SHOTGUN);
                    shoot(pi,SHOTGUN);
                    shoot(pi,SHOTGUN);
                    shoot(pi,SHOTGUN);
                    shoot(pi,SHOTGUN);
                    shoot(pi,SHOTGUN);
                    shoot(pi,SHOTGUN);
                    shoot(pi,SHOTGUN);
                    shoot(pi,SHOTGUN);
                    shoot(pi,SHOTGUN);

                    p->ammo_amount[SHOTGUN_WEAPON]--;

                    spritesound(SHOTGUN_FIRE,pi);

                    p->at290 = 8192;
                    madenoise(snum);

                    lastvisinc = totalclock+32;
                    p->visibility = 0;
                }
                
                if(*kb == 7)
                {
                    if (p->at59a)
                    {
                        shoot(pi,SHOTGUN);
                        shoot(pi,SHOTGUN);
                        shoot(pi,SHOTGUN);
                        shoot(pi,SHOTGUN);
                        shoot(pi,SHOTGUN);
                        shoot(pi,SHOTGUN);
                        shoot(pi,SHOTGUN);
                        shoot(pi,SHOTGUN);
                        shoot(pi,SHOTGUN);
                        shoot(pi,SHOTGUN);

                        p->ammo_amount[SHOTGUN_WEAPON]--;

                        spritesound(SHOTGUN_FIRE,pi);

                        if (psectlotag != 857)
                        {
                            p->posxv -= sintable[(p->ang+512)&2047]<<5;
                            p->posyv -= sintable[p->ang&2047]<<5;
                        }
                    }
                    else if (psectlotag != 857)
                    {
                        p->posxv -= sintable[(p->ang+512)&2047]<<4;
                        p->posyv -= sintable[p->ang&2047]<<4;
                    }
                }

                if (p->at599)
                {
                    switch (*kb)
                    {
                        case 16:
                            checkavailweapon(p);
                            break;
                        case 17:
                            spritesound(SHOTGUN_COCK,pi);
                            break;
                        case 28:
                            *kb = 0;
                            p->at599 = 0;
                            p->at59a = 0;
                            return;
                    }
                }
                else if (p->at59a)
                {
                    switch (*kb)
                    {
                        case 26:
                            checkavailweapon(p);
                            break;
                        case 27:
                            spritesound(SHOTGUN_COCK,pi);
                            break;
                        case 38:
                            *kb = 0;
                            p->at599 = 0;
                            p->at59a = 0;
                            return;
                    }
                }
                else
                {
                    switch (*kb)
                    {
                        case 16:
                            checkavailweapon(p);
                            (*kb)=0;
                            p->at599 = 1;
                            p->at59a = 0;
                            return;
                    }
                }
                break;

            case CHAINGUN_WEAPON:

                (*kb)++;
                p->horiz++;
                p->at59b++;

                if( (*kb) <= 12 )
                {
                    if( ((*kb)%3) == 0 )
                    {
                        p->ammo_amount[CHAINGUN_WEAPON]--;

                        if( ((*kb)%3) == 0 )
                        {
                            j = spawn(pi,SHELL);

                            sprite[j].ang += 1024;
                            sprite[j].ang &= 2047;
                            sprite[j].xvel += 32;
                            sprite[j].z += (3<<8);
                            ssp(j,CLIPMASK0);
                        }

                        spritesound(CHAINGUN_FIRE,pi);
                        shoot(pi,CHAINGUN);
                        p->at290 = 8192;
                        madenoise(snum);
                        lastvisinc = totalclock+32;
                        p->visibility = 0;

                        if (psectlotag != 857)
                        {
                            p->posxv -= sintable[(p->ang+512)&2047]<<4;
                            p->posyv -= sintable[p->ang&2047]<<4;
                        }
                        checkavailweapon(p);

                        if( ( sb_snum&(1<<2) ) == 0 )
                        {
                            *kb = 0;
                            break;
                        }
                    }
                }
                else if((*kb) > 10)
                {
                    if( sb_snum&(1<<2) ) *kb = 1;
                    else *kb = 0;
                }

                break;

            case SHRINKER_WEAPON:
            case GROW_WEAPON:

                if(p->curr_weapon == GROW_WEAPON)
                {
                    if((*kb) > 3)
                    {
                        *kb = 0;
                        if( screenpeek == snum ) pus = 1;
                        shoot(pi,GROWSPARK);
                        p->at290 = 1024;
                        madenoise(snum);
                        checkavailweapon(p);
                    }
                    else (*kb)++;
                }
                else
                {
                    if ((*kb) == 1)
                    {
                        p->ammo_amount[SHRINKER_WEAPON]--;
                        shoot(pi,SHRINKSPARK);
                        checkavailweapon(p);
                    }
                    (*kb)++;
                    if ((*kb) > 20)
                        *kb = 0;
                }
                break;

            case DEVISTATOR_WEAPON:
                (*kb)++;
                if ((*kb) == 2 || (*kb) == 4)
                {
                    p->visibility = 0;
                    lastvisinc = totalclock+32;
                    spritesound(CHAINGUN_FIRE,pi);
                    shoot(pi,SHOTSPARK1);
                    p->at290 = 16384;
                    madenoise(snum);
                    p->ammo_amount[DEVISTATOR_WEAPON]--;
                    checkavailweapon(p);
                }
                if ((*kb) == 2)
                {
                    p->ang += 16;
                }
                else if ((*kb) == 4)
                {
                    p->ang -= 16;
                }
                if ((*kb) > 4)
                    (*kb) = 1;
                if (!(sb_snum & 4))
                    (*kb) = 0;
                break;
#ifdef RRRA
            case RA13_WEAPON:
                (*kb)++;
                if ((*kb) == 2 || (*kb) == 4)
                {
                    p->visibility = 0;
                    lastvisinc = totalclock+32;
                    spritesound(CHAINGUN_FIRE,pi);
                    shoot(pi,CHAINGUN);
                    p->at290 = 16384;
                    madenoise(snum);
                    p->ammo_amount[RA13_WEAPON]--;
                    if (p->ammo_amount[RA13_WEAPON] <= 0)
                        (*kb) = 0;
                    else
                        checkavailweapon(p);
                }
                if ((*kb) == 2)
                {
                    p->ang += 4;
                }
                else if ((*kb) == 4)
                {
                    p->ang -= 4;
                }
                if ((*kb) > 4)
                    (*kb) = 1;
                if (!(sb_snum & 4))
                    (*kb) = 0;
                break;
            case RA14_WEAPON:
                if ((*kb) == 3)
                {
                    p->MotoSpeed -= 20;
                    p->ammo_amount[RA14_WEAPON]--;
                    shoot(pi,RRTILE1790);
                }
                (*kb)++;
                if ((*kb) > 20)
                {
                    (*kb) = 0;
                    checkavailweapon(p);
                }
                if (p->ammo_amount[RA14_WEAPON] <= 0)
                    (*kb) = 0;
                else
                    checkavailweapon(p);
                break;


#endif
            case FREEZE_WEAPON:
                (*kb)++;
                if ((*kb) >= 7 && (*kb) <= 11)
                    shoot(pi,FIRELASER);

                if( (*kb) == 5 )
                {
                    spritesound(CAT_FIRE,pi);
                    p->at290 = 2048;
                    madenoise(snum);
                }
                else if ((*kb) == 9)
                {
                    p->ammo_amount[FREEZE_WEAPON]--;
                    p->visibility = 0;
                    lastvisinc = totalclock + 32;
                    checkavailweapon(p);
                }
                else if ((*kb) == 12)
                {
                    p->posxv -= sintable[(p->ang+512)&2047]<<4;
                    p->posyv -= sintable[p->ang&2047]<<4;
                    p->horiz += 20;
                    p->at59b += 20;
                }
                if ((*kb) > 20)
                    (*kb) = 0;
                break;

            case TRIPBOMB_WEAPON:
            case BOWLING_WEAPON:
                if (p->curr_weapon == TRIPBOMB_WEAPON)
                {
                    if ((*kb) == 3)
                    {
                        if (screenpeek == snum) pus = 1;
                        p->ammo_amount[TRIPBOMB_WEAPON]--;
                        p->gotweapon[TRIPBOMB_WEAPON] = 0;
#ifdef RRRA
                        if(p->on_ground && (sb_snum&2) && !p->OnMotorcycle)
#else
                        if(p->on_ground && (sb_snum&2) )
#endif
                        {
                            k = 15;
                            i = ((p->horiz+p->horizoff-100)*20);
                        }
                        else
                        {
                            k = 32;
                            i = -512-((p->horiz+p->horizoff-100)*20);
                        }

                        j = EGS(p->cursectnum,
                            p->posx+(sintable[(p->ang+512)&2047]>>6),
                            p->posy+(sintable[p->ang&2047]>>6),
                            p->posz,TRIPBOMBSPRITE,-16,9,9,
                            p->ang,k*2,i,pi,1);
                    }
                    (*kb)++;
                    if ((*kb) > 20)
                    {
                        (*kb) = 0;
                        checkavailweapon(p);
                    }
                }
                else
                {
                    if ((*kb) == 30)
                    {
                        p->ammo_amount[BOWLING_WEAPON]--;
                        spritesound(354,pi);
                        shoot(pi,BOWLINGBALL);
                        p->at290 = 1024;
                        madenoise(snum);
                    }
                    if ((*kb) < 30)
                    {
                        p->posxv += sintable[(p->ang+512)&2047]<<4;
                        p->posyv += sintable[p->ang&2047]<<4;
                    }
                    (*kb)++;
                    if ((*kb) > 40)
                    {
                        (*kb) = 0;
                        p->gotweapon[BOWLING_WEAPON] = 0;
                        checkavailweapon(p);
                    }
                }
                break;
            case KNEE_WEAPON:
                (*kb)++;
                if((*kb)==3)
                    spritesound(426,pi);
                if( (*kb) == 12)
                {
                    shoot(pi,KNEE);
                    p->at290 = 1024;
                    madenoise(snum);
                }
                else if( (*kb) == 16)
                    (*kb) = 0;

                if(p->wantweaponfire >= 0)
                    checkavailweapon(p);
                break;
#ifdef RRRA
            case RA15_WEAPON:
                (*kb)++;
                if((*kb)==3)
                    spritesound(252,pi);
                if( (*kb) == 8)
                {
                    shoot(pi,SLINGBLADE);
                    p->at290 = 1024;
                    madenoise(snum);
                }
                else if( (*kb) == 16)
                    (*kb) = 0;

                if(p->wantweaponfire >= 0)
                    checkavailweapon(p);
                break;
#endif

            case RPG_WEAPON:
                (*kb)++;
                if( (*kb) == 4 )
                {
                    p->ammo_amount[RPG_WEAPON]--;
                    if (p->ammo_amount[HANDBOMB_WEAPON])
                        p->ammo_amount[HANDBOMB_WEAPON]--;
                    lastvisinc = totalclock+32;
                    p->visibility = 0;
                    shoot(pi,RPG);
                    p->at290 = 32768;
                    madenoise(snum);
                    checkavailweapon(p);
                }
                else if ((*kb) == 16)
                    spritesound(450,pi);
                else if ((*kb) == 34)
                    (*kb) = 0;
                break;
#ifdef RRRA
            case RA16_WEAPON:
                (*kb)++;
                if( (*kb) == 4 )
                {
                    p->ammo_amount[RA16_WEAPON]--;
                    lastvisinc = totalclock+32;
                    p->visibility = 0;
                    shoot(pi,RPG2);
                    p->at290 = 32768;
                    madenoise(snum);
                    checkavailweapon(p);
                }
                else if ((*kb) == 16)
                    spritesound(450,pi);
                else if ((*kb) == 34)
                    (*kb) = 0;
                break;
#endif
        }
    }
}

int haskey(short sect, short snum)
{
    short wk;
    struct player_struct *p;
    p = &ps[snum];
    if (!sector[sect].filler)
        return 1;
    if (sector[sect].filler > 6)
        return 1;
    wk = sector[sect].filler;
    if (wk > 3)
        wk -= 3;

    if (p->keys[wk] == 1)
    {
        sector[sect].filler = 0;
        return 1;
    }

    return 0;
}

