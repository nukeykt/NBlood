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

void checkhitsprite(short i,short sn)
{
    short j, k, l, nextj, p, unk;
    spritetype *s;

    i &= (MAXSPRITES-1);

    switch(PN)
    {
#ifdef RRRA
        case RRTILE8464:
            break;
        case RRTILE8487:
        case RRTILE8489:
            spritesound(471,i);
            PN++;
            break;
        case RRTILE7638:
        case RRTILE7644:
        case RRTILE7646:
        case RRTILE7650:
        case RRTILE7653:
        case RRTILE7655:
        case RRTILE7691:
        case RRTILE7876:
        case RRTILE7881:
        case RRTILE7883:
            PN++;
            spritesound(VENT_BUST, i);
            break;
        case RRTILE7879:
            PN++;
            spritesound(495, i);
            hitradius(i,10,0,0,1,1);
            break;
        case RRTILE7648:
        case RRTILE7694:
        case RRTILE7700:
        case RRTILE7702:
        case RRTILE7711:
            PN++;
            spritesound(47,i);
            break;
        case RRTILE7636:
            PN += 3;
            spritesound(VENT_BUST, i);
            break;
        case RRTILE7875:
            PN += 3;
            spritesound(VENT_BUST, i);
            break;
        case RRTILE7640:
            PN += 2;
            spritesound(VENT_BUST, i);
            break;
        case RRTILE7595:
        case RRTILE7704:
            PN = RRTILE7705;
            spritesound(495, i);
            break;
        case RRTILE8579:
            PN = RRTILE5014;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE7441:
            PN = RRTILE5016;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE7534:
            PN = RRTILE5029;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE7545:
            PN = RRTILE5030;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE7547:
            PN = RRTILE5031;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE7574:
            PN = RRTILE5032;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE7575:
            PN = RRTILE5033;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE7578:
            PN = RRTILE5034;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE7478:
            PN = RRTILE5035;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8525:
            PN = RRTILE5036;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8537:
            PN = RRTILE5062;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8215:
            PN = RRTILE5064;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8216:
            PN = RRTILE5065;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8217:
            PN = RRTILE5066;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8218:
            PN = RRTILE5067;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8220:
            PN = RRTILE5068;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8221:
            PN = RRTILE5069;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8312:
            PN = RRTILE5071;
            spritesound(472, i);
            break;
        case RRTILE8395:
            PN = RRTILE5072;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8423:
            PN = RRTILE5073;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE3462:
            PN = RRTILE5074;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case UWHIP:
            PN = RRTILE5075;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8608:
            PN = RRTILE5083;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8609:
            PN = RRTILE5084;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8567:
        case RRTILE8568:
        case RRTILE8569:
        case RRTILE8570:
        case RRTILE8571:
            PN = RRTILE5082;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8640:
            PN = RRTILE5085;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8611:
            PN = RRTILE5086;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case TECHLIGHTBUST2:
            PN = TECHLIGHTBUST4;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8497:
            PN = RRTILE5076;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8162:
        case RRTILE8163:
        case RRTILE8164:
        case RRTILE8165:
        case RRTILE8166:
        case RRTILE8167:
        case RRTILE8168:
            changespritestat(i, 5);
            PN = RRTILE5063;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8589:
        case RRTILE8590:
        case RRTILE8591:
        case RRTILE8592:
        case RRTILE8593:
        case RRTILE8594:
        case RRTILE8595:
            changespritestat(i, 5);
            PN = RRTILE8588;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE3497:
            PN = RRTILE5076;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE3498:
            PN = RRTILE5077;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE3499:
            PN = RRTILE5078;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8503:
            PN = RRTILE5079;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE7901:
            PN = RRTILE5080;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE7696:
            PN = RRTILE7697;
            spritesound(DUKE_SHUCKS, i);
            break;
        case RRTILE7806:
            PN = RRTILE5043;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE7885:
        case RRTILE7890:
            PN = RRTILE5045;
            spritesound(495, i);
            hitradius(i, 10, 0, 0, 1, 1);
            break;
        case RRTILE7886:
            PN = RRTILE5046;
            spritesound(495, i);
            hitradius(i, 10, 0, 0, 1, 1);
            break;
        case RRTILE7887:
            PN = RRTILE5044;
            spritesound(GLASS_HEAVYBREAK, i);
            hitradius(i, 10, 0, 0, 1, 1);
            break;
        case RRTILE7900:
            PN = RRTILE5047;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE7906:
            PN = RRTILE5048;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE7912:
        case RRTILE7913:
            PN = RRTILE5049;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8047:
            PN = RRTILE5050;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8596:
            PN = RRTILE8598;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8059:
            PN = RRTILE5051;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8060:
            PN = RRTILE5052;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8222:
            PN = RRTILE5053;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8223:
            PN = RRTILE5054;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8224:
            PN = RRTILE5055;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8370:
            PN = RRTILE5056;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8371:
            PN = RRTILE5057;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8372:
            PN = RRTILE5058;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8373:
            PN = RRTILE5059;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8396:
            PN = RRTILE5038;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8397:
            PN = RRTILE5039;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8398:
            PN = RRTILE5040;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8399:
            PN = RRTILE5041;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8385:
            PN = RRTILE8386;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8387:
            PN = RRTILE8388;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8389:
            PN = RRTILE8390;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8391:
            PN = RRTILE8392;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE7553:
            PN = RRTILE5035;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8475:
            PN = RRTILE5075;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8498:
            PN = RRTILE5077;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8499:
            PN = RRTILE5078;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE2445:
            PN = RRTILE2450;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE2123:
            PN = RRTILE2124;
            spritesound(GLASS_BREAKING,i);
            lotsofglass(i,-1,10);
            break;
        case RRTILE3773:
            PN = RRTILE8651;
            spritesound(GLASS_BREAKING,i);
            lotsofglass(i,-1,10);
            break;
        case RRTILE7533:
            PN = RRTILE5035;
            spritesound(495, i);
            hitradius(i, 10, 0, 0, 1, 1);
            break;
        case RRTILE8394:
            PN = RRTILE5072;
            spritesound(495, i);
            break;
        case RRTILE8461:
        case RRTILE8462:
            PN = RRTILE5074;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8679:
            PN = RRTILE8680;
            spritesound(DUKE_SHUCKS, i);
            hitradius(i, 10, 0, 0, 1, 1);
            if (SLT != 0)
            {
                short j;
                for (j = 0; j < MAXSPRITES; j++)
                {
                    if (sprite[j].picnum == RRTILE8679 && sprite[j].pal == 4)
                    {
                        if (sprite[j].lotag == SLT)
                            sprite[j].picnum = RRTILE8680;
                    }
                }
            }
            break;
        case RRTILE3584:
            PN = RRTILE8681;
            spritesound(495, i);
            hitradius(i, 250, 0, 0, 1, 1);
            break;
        case RRTILE8682:
            PN = RRTILE8683;
            spritesound(GLASS_HEAVYBREAK, i);
            break;
        case RRTILE8099:
            if (SLT == 5)
            {
                short j;
                SLT = 0;
                PN = RRTILE5087;
                spritesound(340,i);
                for (j = 0; j < MAXSPRITES; j++)
                {
                    if (sprite[j].picnum == RRTILE8094)
                        sprite[j].picnum = RRTILE5088;
                }
            }
            break;
        case RRTILE2431:
            if (PL != 4)
            {
                PN = RRTILE2451;
                if (SLT != 0)
                {
                    short j;
                    for (j = 0; j < MAXSPRITES; j++)
                    {
                        if (sprite[j].picnum == RRTILE2431 && sprite[j].pal == 4)
                        {
                            if (SLT == sprite[j].lotag)
                                sprite[j].picnum = RRTILE2451;
                        }
                    }
                }
            }
            break;
        case RRTILE2443:
            if (PL != 19)
                PN = RRTILE2455;
            break;
        case RRTILE2455:
            spritesound(SQUISHED, i);
            guts(&sprite[i],RRTILE2465,3,myconnectindex);
            deletesprite(i);
            break;
        case RRTILE2451:
            if (PL != 4)
            {
                spritesound(SQUISHED, i);
                if (SLT != 0)
                {
                    short j;
                    for (j = 0; j < MAXSPRITES; j++)
                    {
                        if (sprite[j].picnum == RRTILE2451 && sprite[j].pal == 4)
                        {
                            if (SLT == sprite[j].lotag)
                            {
                                guts(&sprite[i], RRTILE2460, 12, myconnectindex);
                                guts(&sprite[i], RRTILE2465, 3, myconnectindex);
                                sprite[j].xrepeat = 0;
                                sprite[j].yrepeat = 0;
                                sprite[i].xrepeat = 0;
                                sprite[i].yrepeat = 0;
                            }
                        }
                    }
                }
                else
                {
                    guts(&sprite[i], RRTILE2460, 12, myconnectindex);
                    guts(&sprite[i], RRTILE2465, 3, myconnectindex);
                    sprite[i].xrepeat = 0;
                    sprite[i].yrepeat = 0;
                }
            }
            break;
        case RRTILE2437:
            spritesound(439, i);
            break;
#endif
        case RRTILE3114:
            PN = RRTILE3117;
            break;
        case RRTILE2876:
            PN = RRTILE2990;
            break;
        case RRTILE3152:
            PN = RRTILE3218;
            break;
        case RRTILE3153:
            PN = RRTILE3219;
            break;
        case RRTILE2030:
            PN = RRTILE2034;
            spritesound(GLASS_BREAKING,i);
            lotsofglass(i,-1,10);
            break;
        case RRTILE2893:
        case RRTILE2915:
        case RRTILE3115:
        case RRTILE3171:
            switch (PN)
            {
                case RRTILE2915:
                    PN = RRTILE2977;
                    break;
                case RRTILE2893:
                    PN = RRTILE2978;
                    break;
                case RRTILE3115:
                    PN = RRTILE3116;
                    break;
                case RRTILE3171:
                    PN = RRTILE3216;
                    break;
            }
            spritesound(GLASS_BREAKING,i);
            lotsofglass(i,-1,10);
            break;
        case RRTILE2156:
        case RRTILE2158:
        case RRTILE2160:
        case RRTILE2175:
            PN++;
            spritesound(GLASS_BREAKING,i);
            lotsofglass(i,-1,10);
            break;
        case RRTILE2137:
        case RRTILE2151:
        case RRTILE2152:
            spritesound(GLASS_BREAKING,i);
            lotsofglass(i,-1,10);
            PN++;
            for (k = 0; k < 6; k++)
                EGS(SECT,SX,SY,SZ-(8<<8),SCRAP6+(TRAND&15),-8,48,48,TRAND&2047,(TRAND&63)+64,-(TRAND&4095)-(sprite[i].zvel>>2),i,5);
            break;
        case BOWLINGBALL:
            sprite[sn].xvel = (sprite[i].xvel>>1)+(sprite[i].xvel>>2);
            sprite[sn].ang -= (TRAND&16);
            spritesound(355,i);
            break;

        case STRIPEBALL:
        case QUEBALL:
        case RRTILE3440:
        case RRTILE3440+1:
        case HENSTAND:
        case HENSTAND+1:
            if (sprite[sn].picnum == QUEBALL || sprite[sn].picnum == STRIPEBALL)
            {
                sprite[sn].xvel = (sprite[i].xvel>>1)+(sprite[i].xvel>>2);
                sprite[sn].ang -= (SA<<1)+1024;
                SA = getangle(SX-sprite[sn].x,SY-sprite[sn].y)-512;
                if(Sound[POOLBALLHIT].num < 2)
                    spritesound(POOLBALLHIT,i);
            }
            else if (sprite[sn].picnum == RRTILE3440 || sprite[sn].picnum == RRTILE3440+1)
            {
                sprite[sn].xvel = (sprite[i].xvel>>1)+(sprite[i].xvel>>2);
                sprite[sn].ang -= ((SA<<1)+TRAND)&64;
                SA = (SA+TRAND)&16;
                spritesound(355,i);
            }
            else if (sprite[sn].picnum == HENSTAND || sprite[sn].picnum == HENSTAND+1)
            {
                sprite[sn].xvel = (sprite[i].xvel>>1)+(sprite[i].xvel>>2);
                sprite[sn].ang -= ((SA<<1)+TRAND)&16;
                SA = (SA+TRAND)&16;
                spritesound(355,i);
            }
            else
            {
                if( TRAND&3 )
                {
                    sprite[i].xvel = 164;
                    sprite[i].ang = sprite[sn].ang;
                }
            }
            break;

        case TREE1:
        case TREE2:
        case TIRE:
        case BOX:
            switch(sprite[sn].picnum)
            {
                case RADIUSEXPLOSION:
                case RPG:
                case FIRELASER:
                case HYDRENT:
                case HEAVYHBOMB:
                case TRIPBOMBSPRITE:
                case COOLEXPLOSION1:
                case OWHIP:
                case UWHIP:
#ifdef RRRA
                case RPG2:
#endif
                    if(T1 == 0)
                    {
                        CS &= ~257;
                        T1 = 1;
                        spawn(i,BURNING);
                    }
                    break;
            }
            break;

        case CACTUS:
//        case CACTUSBROKE:
            switch(sprite[sn].picnum)
            {
                case RADIUSEXPLOSION:
                case RPG:
                case FIRELASER:
                case HYDRENT:
                case HEAVYHBOMB:
                case TRIPBOMBSPRITE:
                case COOLEXPLOSION1:
                case OWHIP:
                case UWHIP:
#ifdef RRRA
                case RPG2:
#endif
                    for(k=0;k<64;k++)
                    {
                        j = EGS(SECT,SX,SY,SZ-(TRAND%(48<<8)),SCRAP6+(TRAND&3),-8,48,48,TRAND&2047,(TRAND&63)+64,-(TRAND&4095)-(sprite[i].zvel>>2),i,5);
                        sprite[j].pal = 8;
                    }

                    if(PN == CACTUS)
                        PN = CACTUSBROKE;
                    CS &= ~257;
             //       else deletesprite(i);
                    break;
            }
            break;


        case FANSPRITE:
            PN = FANSPRITEBROKE;
            CS &= (65535-257);
            spritesound(GLASS_HEAVYBREAK,i);
            s = &sprite[i];
            for(j=0;j<16;j++) RANDOMSCRAP;

            break;
        case WATERFOUNTAIN:
        case WATERFOUNTAIN+1:
        case WATERFOUNTAIN+2:
        case WATERFOUNTAIN+3:
            spawn(i,TOILETWATER);
            break;
        case SATELITE:
        case FUELPOD:
        case SOLARPANNEL:
        case ANTENNA:
            if(sprite[sn].extra != *actorscrptr[SHOTSPARK1] )
            {
                for(j=0;j<15;j++)
                    EGS(SECT,SX,SY,sector[SECT].floorz-(12<<8)-(j<<9),SCRAP1+(TRAND&15),-8,64,64,
                        TRAND&2047,(TRAND&127)+64,-(TRAND&511)-256,i,5);
                spawn(i,EXPLOSION2);
                deletesprite(i);
            }
            break;
        case BOTTLE1:
        case BOTTLE2:
        case BOTTLE3:
        case BOTTLE4:
        case BOTTLE5:
        case BOTTLE6:
        case BOTTLE8:
        case BOTTLE10:
        case BOTTLE11:
        case BOTTLE12:
        case BOTTLE13:
        case BOTTLE14:
        case BOTTLE15:
        case BOTTLE16:
        case BOTTLE17:
        case BOTTLE18:
        case BOTTLE19:
        case DOMELITE:
        case SUSHIPLATE1:
        case SUSHIPLATE2:
        case SUSHIPLATE3:
        case SUSHIPLATE4:
        case SUSHIPLATE5:
        case WAITTOBESEATED:
        case VASE:
        case STATUEFLASH:
        case STATUE:
#ifdef RRRA
        case RRTILE1824:
#endif
            if(PN == BOTTLE10)
                lotsofmoney(&sprite[i],4+(TRAND&3));
            else if(PN == STATUE || PN == STATUEFLASH)
            {
                lotsofcolourglass(i,-1,40);
                spritesound(GLASS_HEAVYBREAK,i);
            }
            else if(PN == VASE)
                lotsofglass(i,-1,40);

            spritesound(GLASS_BREAKING,i);
            SA = TRAND&2047;
            lotsofglass(i,-1,8);
            deletesprite(i);
            break;
        case BOTTLE7:
#ifdef RRRA
        case RRTILE2654:
        case RRTILE2656:
        case RRTILE3172:
#endif
            spritesound(GLASS_BREAKING,i);
            lotsofglass(i,-1,10);
            deletesprite(i);
            break;
        case FORCESPHERE:
            sprite[i].xrepeat = 0;
            hittype[OW].temp_data[0] = 32;
            hittype[OW].temp_data[1] = !hittype[OW].temp_data[1];
            hittype[OW].temp_data[2] ++;
            spawn(i,EXPLOSION2);
            break;
        case TOILET:
            PN = TOILETBROKE;
            CS |= (TRAND&1)<<2;
            CS &= ~257;
            spawn(i,TOILETWATER);
            spritesound(GLASS_BREAKING,i);
            break;

        case STALL:
            PN = STALLBROKE;
            CS |= (TRAND&1)<<2;
            CS &= ~257;
            spawn(i,TOILETWATER);
            spritesound(GLASS_HEAVYBREAK,i);
            break;

        case HYDRENT:
            PN = BROKEFIREHYDRENT;
            spawn(i,TOILETWATER);

//            for(k=0;k<5;k++)
  //          {
    //            j = EGS(SECT,SX,SY,SZ-(TRAND%(48<<8)),SCRAP3+(TRAND&3),-8,48,48,TRAND&2047,(TRAND&63)+64,-(TRAND&4095)-(sprite[i].zvel>>2),i,5);
      //          sprite[j].pal = 2;
        //    }
            spritesound(GLASS_HEAVYBREAK,i);
            break;

        case GRATE1:
            PN = BGRATE1;
            CS &= (65535-256-1);
            spritesound(VENT_BUST,i);
            break;

        case CIRCLEPANNEL:
            PN = CIRCLEPANNELBROKE;
            CS &= (65535-256-1);
            spritesound(VENT_BUST,i);
            break;

        case PIPE1:
        case PIPE2:
        case PIPE3:
        case PIPE4:
        case PIPE5:
        case PIPE6:
            switch(PN)
            {
                case PIPE1:PN=PIPE1B;break;
                case PIPE2:PN=PIPE2B;break;
                case PIPE3:PN=PIPE3B;break;
                case PIPE4:PN=PIPE4B;break;
                case PIPE5:PN=PIPE5B;break;
                case PIPE6:PN=PIPE6B;break;
            }

            j = spawn(i,STEAM);
            sprite[j].z = sector[SECT].floorz-(32<<8);
            break;

        case CHAIR1:
        case CHAIR2:
            PN = BROKENCHAIR;
            CS = 0;
            break;
        case CHAIR3:
        case MOVIECAMERA:
        case SCALE:
        case VACUUM:
        case CAMERALIGHT:
        case IVUNIT:
        case POT1:
        case POT2:
        case POT3:
            spritesound(GLASS_HEAVYBREAK,i);
            s = &sprite[i];
            for(j=0;j<16;j++) RANDOMSCRAP;
            deletesprite(i);
            break;
        case PLAYERONWATER:
            i = OW;
        default:
            if( (sprite[i].cstat&16) && SHT == 0 && SLT == 0 && sprite[i].statnum == 0)
                break;

            if( ( sprite[sn].picnum == SHRINKSPARK || sprite[sn].picnum == FREEZEBLAST || sprite[sn].owner != i ) && sprite[i].statnum != 4)
            {
                if( badguy(&sprite[i]) == 1)
                {
                    if(sprite[sn].picnum == RPG) sprite[sn].extra <<= 1;
#ifdef RRRA
                    else if(sprite[sn].picnum == RPG2) sprite[sn].extra <<= 1;
#endif

                    if( (PN != DRONE) )
                        if(sprite[sn].picnum != FREEZEBLAST )
                            if( actortype[PN] == 0 )
                    {
                        j = spawn(sn,JIBS6);
                        if(sprite[sn].pal == 6)
                            sprite[j].pal = 6;
                        sprite[j].z += (4<<8);
                        sprite[j].xvel = 16;
                        sprite[j].xrepeat = sprite[j].yrepeat = 24;
                        sprite[j].ang += 32-(TRAND&63);
                    }

                    j = sprite[sn].owner;

                    if( j >= 0 && sprite[j].picnum == APLAYER && PN != DRONE )
                        if( ps[sprite[j].yvel].curr_weapon == SHOTGUN_WEAPON )
                    {
                        shoot(i,BLOODSPLAT3);
                        shoot(i,BLOODSPLAT1);
                        shoot(i,BLOODSPLAT2);
                        shoot(i,BLOODSPLAT4);
                    }

                    if (actortype[PN] == 0) {
                    }

                    if(sprite[i].statnum == 2)
                    {
                        changespritestat(i,1);
                        hittype[i].timetosleep = SLEEPTIME;
                    }
                }

                if( sprite[i].statnum != 2 )
                {
                    if( sprite[sn].picnum == FREEZEBLAST && ( (PN == APLAYER && sprite[i].pal == 1 ) || ( freezerhurtowner == 0 && sprite[sn].owner == i ) ) )
                        return;

                    hittype[i].picnum = sprite[sn].picnum;
                    hittype[i].extra += sprite[sn].extra;
                    if (PN != COW)
                        hittype[i].ang = sprite[sn].ang;
                    hittype[i].owner = sprite[sn].owner;
                }

                if(sprite[i].statnum == 10)
                {
                    p = sprite[i].yvel;
                    if(ps[p].newowner >= 0)
                    {
                        ps[p].newowner = -1;
                        ps[p].posx = ps[p].oposx;
                        ps[p].posy = ps[p].oposy;
                        ps[p].posz = ps[p].oposz;
                        ps[p].ang = ps[p].oang;

                        updatesector(ps[p].posx,ps[p].posy,&ps[p].cursectnum);
                        setpal(&ps[p]);

                        j = headspritestat[1];
                        while(j >= 0)
                        {
                            if(sprite[j].picnum==CAMERA1) sprite[j].yvel = 0;
                            j = nextspritestat[j];
                        }
                    }

                    if( sprite[hittype[i].owner].picnum != APLAYER)
                        if(ud.player_skill >= 3)
                            sprite[sn].extra += (sprite[sn].extra>>1);
                }

            }
            break;
    }
}


void allignwarpelevators(void)
{
    short i, j;

    i = headspritestat[3];
    while(i >= 0)
    {
        if( SLT == 17 && SS > 16)
        {
            j = headspritestat[3];
            while(j >= 0)
            {
                if( (sprite[j].lotag) == 17 && i != j &&
                    (SHT) == (sprite[j].hitag) )
                {
                    sector[sprite[j].sectnum].floorz =
                        sector[SECT].floorz;
                    sector[sprite[j].sectnum].ceilingz =
                        sector[SECT].ceilingz;
                }

                j = nextspritestat[j];
            }
        }
        i = nextspritestat[i];
    }
}


#ifdef RRRA
int dword_119C08 = 0;
#endif


#ifdef DESYNCCORRECT
/////Desync fix////
extern short fakejval;
///////////////////
#endif
void cheatkeys(short snum)
{
    short i, k;
    char dainv;
    unsigned long sb_snum, j;
    struct player_struct *p;
    short unk;

    unk = 0;
    sb_snum = sync[snum].bits;
    p = &ps[snum];

    if(p->cheat_phase == 1) return;

    i = p->aim_mode;
    p->aim_mode = (sb_snum>>23)&1;
    if(p->aim_mode < i)
        p->return_to_center = 9;

    if (KB_KeyPressed(sc_Tilde) && p->last_pissed_time == 0)
#ifdef RRRA
        if (sprite[p->i].extra > 0)
#endif
    {
        p->last_pissed_time = 4000;
        if (!ud.lockout)
            spritesound(437,p->i);
        if (sprite[p->i].extra <= max_player_health - max_player_health / 10)
        {
            sprite[p->i].extra += 2;
            p->last_extra = sprite[p->i].extra;
        }
        else if (sprite[p->i].extra < max_player_health)
            sprite[p->i].extra = max_player_health;
    }

    if( !(sb_snum&((15<<8)|(1<<12)|(1<<15)|(1<<16)|(1<<22)|(1<<19)|(1<<20)|(1<<21)|(1<<24)|(1<<25)|(1<<27)|(1<<28)|(1<<29)|(1<<30)|(1<<31))) )
        p->interface_toggle_flag = 0;
    else if(p->interface_toggle_flag == 0)
    {
        p->interface_toggle_flag = 1;

        if( sb_snum&(1<<21) )
        {
            KB_ClearKeyDown( sc_Pause );
            ud.pause_on = !ud.pause_on;
            if( ud.pause_on == 1 && sb_snum&(1<<5) ) ud.pause_on = 2;
            if(ud.pause_on)
            {
                MUSIC_Pause();
                FX_StopAllSounds();
                clearsoundlocks();
            }
            else
            {
                if(MusicToggle) MUSIC_Continue();
                pub = NUMPAGES;
                pus = NUMPAGES;
            }
        }

        if(ud.pause_on) return;
        
        if(sprite[p->i].extra <= 0) return;

        if( sb_snum&(1<<30) && p->newowner == -1 )
        {
            switch(p->inven_icon)
            {
                case 4: sb_snum |= (1<<25);break;
                case 3: sb_snum |= (1<<24);break;
                case 5: sb_snum |= (1<<15);break;
                case 1: sb_snum |= (1<<16);break;
                case 2: sb_snum |= (1<<12);break;
                case 8: unk = 1; break;
            }
        }

        if( (sb_snum&(1<<12)) )
        {
            if(p->steroids_amount == 400 )
            {
                p->steroids_amount--;
                spritesound(DUKE_TAKEPILLS,p->i);
                p->inven_icon = 2;
                FTA(12,p);
            }
            return;
        }

        if(p->newowner == -1)
            if( sb_snum&(1<<20) || sb_snum&(1<<27) || p->refresh_inventory)
        {
            p->invdisptime = 26*2;

            if( sb_snum&(1<<27) ) k = 1;
            else k = 0;

            if(p->refresh_inventory) p->refresh_inventory = 0;
            dainv = p->inven_icon;

            i = 0;
            CHECKINV1:

            if(i < 9)
            {
                i++;

                switch(dainv)
                {
                    case 4:
                        if(p->jetpack_amount > 0 && i > 1)
                            break;
                        if(k) dainv = 5;
                        else dainv = 3;
                        goto CHECKINV1;
                    case 6:
                        if(p->scuba_amount > 0 && i > 1)
                            break;
                        if(k) dainv = 7;
                        else dainv = 5;
                        goto CHECKINV1;
                    case 2:
                        if(p->steroids_amount > 0 && i > 1)
                            break;
                        if(k) dainv = 3;
                        else dainv = 1;
                        goto CHECKINV1;
                    case 3:
                        if(p->holoduke_amount > 0 && i > 1)
                            break;
                        if(k) dainv = 4;
                        else dainv = 2;
                        goto CHECKINV1;
                    case 0:
                    case 1:
                        if(p->firstaid_amount > 0 && i > 1)
                            break;
                        if(k) dainv = 2;
                        else dainv = 7;
                        goto CHECKINV1;
                    case 5:
                        if(p->heat_amount > 0 && i > 1)
                            break;
                        if(k) dainv = 6;
                        else dainv = 4;
                        goto CHECKINV1;
                    case 7:
                        if(p->boot_amount > 0 && i > 1)
                            break;
                        if(k) dainv = 1;
                        else dainv = 6;
                        goto CHECKINV1;
                }
            }
            else dainv = 0;
            p->inven_icon = dainv;

            switch(dainv)
            {
                case 1: FTA(3,p);break;
                case 2: FTA(90,p);break;
                case 3: FTA(91,p);break;
                case 4: FTA(88,p);break;
                case 5: FTA(88,p);break;
                case 6: FTA(89,p);break;
                case 7: FTA(6,p);break;
                case 8: FTA(6,p);break;
            }
        }

        j = ( (sb_snum&(15<<8))>>8 ) - 1;

        if( j > 0 && p->kickback_pic > 0)
            p->wantweaponfire = j;

        if(p->last_pissed_time <= (26*218) && p->show_empty_weapon == 0 && p->kickback_pic == 0 && p->quick_kick == 0 && sprite[p->i].xrepeat > 8 && p->access_incs == 0 && p->knee_incs == 0 )
        {
            if(  ( p->weapon_pos == 0 || ( p->holster_weapon && p->weapon_pos == -9 ) ) )
            {
                if(j == 10 || j == 11)
                {
                    k = p->curr_weapon;
#ifdef RRRA
                    if (k == RA16_WEAPON) k = RPG_WEAPON;
                    else if (k == GROW_WEAPON) k = SHRINKER_WEAPON;
                    else if (k == RA15_WEAPON) k = KNEE_WEAPON;
#endif
                    j = ( j == 10 ? -1 : 1 );
                    i = 0;

                    while( k >= 0 && k < 10 )
                    {
                        k += j;
                        if(k == -1) k = 9;
                        else if(k == 10) k = 0;

                        if( p->gotweapon[k] && p->ammo_amount[k] > 0 )
                        {
                            j = k;
                            break;
                        }

                        i++;
                        if(i == 10)
                        {
                            addweapon( p, KNEE_WEAPON );
                            break;
                        }
                    }
                }

                k = -1;


                if( j == HANDBOMB_WEAPON && p->ammo_amount[HANDBOMB_WEAPON] == 0 )
                {
                    k = headspritestat[1];
                    while(k >= 0)
                    {
                        if( sprite[k].picnum == HEAVYHBOMB && sprite[k].owner == p->i )
                        {
                            p->gotweapon[HANDBOMB_WEAPON] = 1;
                            j = HANDREMOTE_WEAPON;
                            break;
                        }
                        k = nextspritestat[k];
                    }
                }
#ifdef RRRA
                else if (j == KNEE_WEAPON)
                {
                    if(screenpeek == snum) pus = NUMPAGES;

                    if (p->curr_weapon == KNEE_WEAPON)
                    {
                        p->subweapon = 2;
                        j = RA15_WEAPON;
                    }
                    else if(p->subweapon&2)
                    {
                        p->subweapon = 0;
                        j = KNEE_WEAPON;
                    }
                }
                else if (j == RPG_WEAPON)
                {
                    if(screenpeek == snum) pus = NUMPAGES;

                    if (p->curr_weapon == RPG_WEAPON || p->ammo_amount[RPG_WEAPON] == 0)
                    {
                        if (p->ammo_amount[RA16_WEAPON] == 0)
                            return;
                        p->subweapon = 4;
                        j = RA16_WEAPON;
                    }
                    else if((p->subweapon&4) || p->ammo_amount[RA16_WEAPON] == 0)
                    {
                        p->subweapon = 0;
                        j = RPG_WEAPON;
                    }
                }
                else if(j == SHRINKER_WEAPON)
                {
                    if(screenpeek == snum) pus = NUMPAGES;

                    if (p->curr_weapon == SHRINKER_WEAPON || p->ammo_amount[SHRINKER_WEAPON] == 0)
                    {
                        p->subweapon = (1<<GROW_WEAPON);
                        j = GROW_WEAPON;
                    }
                    else if((p->subweapon&(1<<GROW_WEAPON)) || p->ammo_amount[GROW_WEAPON] == 0)
                    {
                        p->subweapon = 0;
                        j = SHRINKER_WEAPON;
                    }
                }
                else if(j == TRIPBOMB_WEAPON)
                {
                    if(screenpeek == snum) pus = NUMPAGES;

                    if (p->curr_weapon == TRIPBOMB_WEAPON || p->ammo_amount[TRIPBOMB_WEAPON] == 0)
                    {
                        p->subweapon = (1<<BOWLING_WEAPON);
                        j = BOWLING_WEAPON;
                    }
                    else if((p->subweapon&(1<<BOWLING_WEAPON)) || p->ammo_amount[BOWLING_WEAPON] == 0)
                    {
                        p->subweapon = 0;
                        j = TRIPBOMB_WEAPON;
                    }
                }
#else
                if(j == SHRINKER_WEAPON)
                {
                    if(screenpeek == snum) pus = NUMPAGES;

                    if (p->curr_weapon == SHRINKER_WEAPON || p->ammo_amount[SHRINKER_WEAPON] == 0)
                    {
                        p->subweapon = (1<<GROW_WEAPON);
                        j = GROW_WEAPON;
                    }
                    else if((p->subweapon&(1<<GROW_WEAPON)) || p->ammo_amount[GROW_WEAPON] == 0)
                    {
                        p->subweapon = 0;
                        j = SHRINKER_WEAPON;
                    }
                }

                if(j == TRIPBOMB_WEAPON)
                {
                    if(screenpeek == snum) pus = NUMPAGES;

                    if (p->curr_weapon == TRIPBOMB_WEAPON || p->ammo_amount[TRIPBOMB_WEAPON] == 0)
                    {
                        p->subweapon = (1<<BOWLING_WEAPON);
                        j = BOWLING_WEAPON;
                    }
                    else if((p->subweapon&(1<<BOWLING_WEAPON)) || p->ammo_amount[BOWLING_WEAPON] == 0)
                    {
                        p->subweapon = 0;
                        j = TRIPBOMB_WEAPON;
                    }
                }
#endif

                if(p->holster_weapon)
                {
                    sb_snum |= 1<<19;
                    p->weapon_pos = -9;
                }
                else if( p->gotweapon[j] && p->curr_weapon != j ) switch(j)
                {
#ifdef RRRA
                    case KNEE_WEAPON:
                        addweapon( p, j );
                        break;
                    case RA15_WEAPON:
                        spritesound(496,ps[screenpeek].i);
                        addweapon(p, j);
                        break;
#else
                    case KNEE_WEAPON:
                        addweapon( p, KNEE_WEAPON );
                        break;
#endif
                    case PISTOL_WEAPON:
                        if ( p->ammo_amount[PISTOL_WEAPON] == 0 )
                            if(p->show_empty_weapon == 0)
                        {
                            p->last_full_weapon = p->curr_weapon;
                            p->show_empty_weapon = 32;
                        }
                        addweapon( p, PISTOL_WEAPON );
                        break;
                    case SHOTGUN_WEAPON:
                        if( p->ammo_amount[SHOTGUN_WEAPON] == 0 && p->show_empty_weapon == 0)
                        {
                            p->last_full_weapon = p->curr_weapon;
                            p->show_empty_weapon = 32;
                        }
                        addweapon( p, SHOTGUN_WEAPON);
                        break;
                    case CHAINGUN_WEAPON:
                        if( p->ammo_amount[CHAINGUN_WEAPON] == 0 && p->show_empty_weapon == 0)
                        {
                            p->last_full_weapon = p->curr_weapon;
                            p->show_empty_weapon = 32;
                        }
                        addweapon( p, CHAINGUN_WEAPON);
                        break;
                    case RPG_WEAPON:
                        if( p->ammo_amount[RPG_WEAPON] == 0 )
                            if(p->show_empty_weapon == 0)
                        {
                            p->last_full_weapon = p->curr_weapon;
                            p->show_empty_weapon = 32;
                        }
                        addweapon( p, RPG_WEAPON );
                        break;
#ifdef RRRA
                    case RA16_WEAPON:
                        if( p->ammo_amount[RA16_WEAPON] == 0 )
                            if(p->show_empty_weapon == 0)
                        {
                            p->last_full_weapon = p->curr_weapon;
                            p->show_empty_weapon = 32;
                        }
                        addweapon( p, RA16_WEAPON );
                        break;
#endif
                    case DEVISTATOR_WEAPON:
                        if( p->ammo_amount[DEVISTATOR_WEAPON] == 0 && p->show_empty_weapon == 0 )
                        {
                            p->last_full_weapon = p->curr_weapon;
                            p->show_empty_weapon = 32;
                        }
                        addweapon( p, DEVISTATOR_WEAPON );
                        break;
#ifdef RRRA
                    case RA13_WEAPON:
                        if( p->ammo_amount[RA13_WEAPON] == 0 && p->show_empty_weapon == 0)
                        {
                            p->show_empty_weapon = 32;
                        }
                        addweapon( p, RA13_WEAPON );
                        break;
                    case RA14_WEAPON:
                        if( p->ammo_amount[RA14_WEAPON] == 0 && p->show_empty_weapon == 0)
                        {
                            p->show_empty_weapon = 32;
                        }
                        addweapon( p, RA14_WEAPON );
                        break;
#endif
                    case FREEZE_WEAPON:
                        if( p->ammo_amount[FREEZE_WEAPON] == 0 && p->show_empty_weapon == 0)
                        {
                            p->last_full_weapon = p->curr_weapon;
                            p->show_empty_weapon = 32;
                        }
                        addweapon( p, j );
                        break;
                    case SHRINKER_WEAPON:
                    case GROW_WEAPON:

                        if( p->ammo_amount[j] == 0 && p->show_empty_weapon == 0)
                        {
                            p->last_full_weapon = p->curr_weapon;
                            p->show_empty_weapon = 32;
                        }

                        addweapon(p, j);
                        break;
                    case HANDREMOTE_WEAPON:
                        if(k >= 0) // Found in list of [1]'s
                        {
                            p->curr_weapon = HANDREMOTE_WEAPON;
                            p->last_weapon = -1;
                            p->weapon_pos = 10;
                        }
                        break;
                    case HANDBOMB_WEAPON:
                        if( p->ammo_amount[HANDBOMB_WEAPON] > 0 && p->gotweapon[HANDBOMB_WEAPON] )
                            addweapon( p, HANDBOMB_WEAPON);
                        break;
                    case TRIPBOMB_WEAPON:
                    case BOWLING_WEAPON:
                        if( p->ammo_amount[j] == 0 && p->show_empty_weapon == 0)
                        {
                            p->last_full_weapon = p->curr_weapon;
                            p->show_empty_weapon = 32;
                        }

                        addweapon(p, j);
                        break;
                }
            }

            if( sb_snum&(1<<19) )
            {
                if( p->curr_weapon > KNEE_WEAPON )
                {
                    if(p->holster_weapon == 0 && p->weapon_pos == 0)
                    {
                        p->holster_weapon = 1;
                        p->weapon_pos = -1;
                        FTA(73,p);
                    }
                    else if(p->holster_weapon == 1 && p->weapon_pos == -9)
                    {
                        p->holster_weapon = 0;
                        p->weapon_pos = 10;
                        FTA(74,p);
                    }
                }
            }
        }

        if( sb_snum&(1<<24) )
        {
            if( p->holoduke_amount > 0 && sprite[p->i].extra < max_player_health)
            {
                p->holoduke_amount -= 400;
                sprite[p->i].extra += 5;
                if (sprite[p->i].extra > max_player_health)
                    sprite[p->i].extra = max_player_health;

                p->drink_amt += 5;
                p->at596 = 1;
                p->inven_icon = 3;
                if (p->holoduke_amount == 0)
                    checkavailinven(p);

                coolgaugetext(screenpeek);
                gutmeter(screenpeek);
                if (p->drink_amt < 99)
                    if (Sound[425].num == 0)
                    spritesound(425,p->i);
            }
        }

        if( (sb_snum&(1<<15)) && p->newowner == -1)
            if (p->at59d == 0)
        {
            p->at59d = 126;
            spritesound(390,p->i);
            p->at290 = 16384;
            madenoise(snum);
            if (sector[p->cursectnum].lotag == 857)
            {
                if (sprite[p->i].extra <= max_player_health)
                {
                    sprite[p->i].extra += 10;
                    if (sprite[p->i].extra >= max_player_health)
                        sprite[p->i].extra = max_player_health;
                    coolgaugetext(screenpeek);
                    gutmeter(screenpeek);
                }
            }
            else
            {
                if (sprite[p->i].extra+1 <= max_player_health)
                {
                    sprite[p->i].extra++;
                    coolgaugetext(screenpeek);
                    gutmeter(screenpeek);
                }
            }
        }

        if( sb_snum&(1<<16) )
        {
            if( p->firstaid_amount > 0 && sprite[p->i].extra < max_player_health )
            {
                j = 10;
                if(p->firstaid_amount > j)
                {
                    p->firstaid_amount -= j;
                    sprite[p->i].extra += j;
                    if (sprite[p->i].extra > max_player_health)
                        sprite[p->i].extra = max_player_health;
                    p->inven_icon = 1;
                }
                else
                {
                    sprite[p->i].extra += p->firstaid_amount;
                    p->firstaid_amount = 0;
                    checkavailinven(p);
                }
                if (sprite[p->i].extra > max_player_health)
                    sprite[p->i].extra = max_player_health;
                p->drink_amt += 10;
                if (p->drink_amt <= 100)
                    if (Sound[DUKE_USEMEDKIT].num == 0)
                    spritesound(DUKE_USEMEDKIT,p->i);
            }
        }

        if( sb_snum&(1<<25) )
        {
            if( p->jetpack_amount > 0 && sprite[p->i].extra < max_player_health)
            {
                if (Sound[429].num == 0)
                    spritesound(429,p->i);

                p->jetpack_amount -= 100;
                if (p->drink_amt > 0)
                {
                    p->drink_amt -= 5;
                    if (p->drink_amt < 0)
                        p->drink_amt = 0;
                }

                if (p->eat < 100)
                {
                    p->eat += 5;
                    if (p->eat > 100)
                        p->eat = 100;
                }

                sprite[p->i].extra += 5;

                p->inven_icon = 4;

                if (sprite[p->i].extra > max_player_health)
                    sprite[p->i].extra = max_player_health;

                if (p->jetpack_amount <= 0)
                    checkavailinven(p);
            }
        }

        if(sb_snum&(1<<28) && p->one_eighty_count == 0)
            p->one_eighty_count = -1024;
    }
    if (p->at596 || p->at597)
        gutmeter(screenpeek);
}

void checksectors(short snum)
{
    long i = -1,oldz;
    struct player_struct *p;
    short j,hitscanwall;

    p = &ps[snum];

    switch(sector[p->cursectnum].lotag)
    {

        case 32767:
            sector[p->cursectnum].lotag = 0;
#if defined(DESYNCCORRECT) && !defined(RRRA)
            fakejval = 0;
#endif
            FTA(9,p);
            p->secret_rooms++;
            return;
        case -1:
            for(i=connecthead;i>=0;i=connectpoint2[i])
                ps[i].gm = MODE_EOL;
            sector[p->cursectnum].lotag = 0;
#ifdef RRRA
            if (!word_119BE2)
            {
                if(ud.from_bonus)
                {
                    ud.level_number = ud.from_bonus;
                    ud.m_level_number = ud.level_number;
                    ud.from_bonus = 0;
                }
                else
                {
                    if (ud.level_number == 6 && ud.volume_number == 0)
                        word_119BE4 = 1;
                    ud.level_number++;
                    if( ud.level_number > 6 )
                        ud.level_number = 0;
                    ud.m_level_number = ud.level_number;
                }
                word_119BE2 = 1;
            }
#else
            if(ud.from_bonus)
            {
                ud.level_number = ud.from_bonus;
                ud.m_level_number = ud.level_number;
                ud.from_bonus = 0;
            }
            else
            {
                ud.level_number++;
                if( ud.level_number > 6 )
                    ud.level_number = 0;
                ud.m_level_number = ud.level_number;
            }
#endif
            return;
        case -2:
            sector[p->cursectnum].lotag = 0;
            p->timebeforeexit = 26*8;
            p->customexitsound = sector[p->cursectnum].hitag;
            return;
        default:
            if(sector[p->cursectnum].lotag >= 10000)
            {
#if defined(DESYNCCORRECT) && !defined(RRRA)
                if(snum == screenpeek || ud.coop == 1 )
                    fakejval = -1;
#endif
                if(snum == screenpeek || ud.coop == 1 )
                    spritesound(sector[p->cursectnum].lotag-10000,p->i);
                sector[p->cursectnum].lotag = 0;
            }
            break;

    }

    //After this point the the player effects the map with space

    if(p->gm&MODE_TYPE || sprite[p->i].extra <= 0) return;
    
#if defined(DESYNCCORRECT) && !defined(RRRA)
    if( ud.cashman && sync[snum].bits&(1<<29) )
        fakejval = -1;
#endif
    if( ud.cashman && sync[snum].bits&(1<<29) )
        lotsofmoney(&sprite[p->i],2);

    if( !(sync[snum].bits&(1<<29)) && !(sync[snum].bits&(1<<31)))
        p->toggle_key_flag = 0;

    else if(!p->toggle_key_flag)
    {
        neartagsprite = -1;
        p->toggle_key_flag = 1;
        hitscanwall = -1;
        
#if defined(DESYNCCORRECT) && !defined(RRRA)
        {
            long sx,sy,sz;
            short sect,hs,hw;

            hitscan(p->posx,p->posy,p->posz,p->cursectnum,
                sintable[(p->ang+512)&2047],
                sintable[p->ang&2047],
                0,&sect,hw,&hs,&sx,&sy,&sz,CLIPMASK0);
            fakejval &= ~0xffff;
            fakejval |= sect;
        }
#endif
        hitawall(p,&hitscanwall);

#ifdef RRRA
        if (hitscanwall >= 0 && wall[hitscanwall].overpicnum == MIRROR && snum == screenpeek)
            if (numplayers == 1)
            {
                if (Sound[27].num == 0 && Sound[28].num == 0 && Sound[29].num == 0 && Sound[257].num == 0 && Sound[258].num == 0)
                {
                    short snd = TRAND % 5;
                    if (snd == 0)
                        spritesound(27, p->i);
                    else if (snd == 1)
                        spritesound(28, p->i);
                    else if (snd == 2)
                        spritesound(29, p->i);
                    else if (snd == 3)
                        spritesound(257, p->i);
                    else if (snd == 4)
                        spritesound(258, p->i);
                }
                return;
            }
#else
        if(hitscanwall >= 0 && wall[hitscanwall].overpicnum == MIRROR)
            if( wall[hitscanwall].lotag > 0 && Sound[wall[hitscanwall].lotag].num == 0 && snum == screenpeek)
        {
#if defined(DESYNCCORRECT) && !defined(RRRA)
            fakejval = -1;
#endif
            spritesound(wall[hitscanwall].lotag,p->i);
            return;
        }
#endif

        if(hitscanwall >= 0 && (wall[hitscanwall].cstat&16) )
            switch(wall[hitscanwall].overpicnum)
        {
            default:
#if defined(DESYNCCORRECT) && defined(RRRA)
                fakejval = hitscanwall*32;
#endif
                if(wall[hitscanwall].lotag)
                    return;
        }

#ifdef RRRA
        if (p->OnMotorcycle)
        {
            if (p->MotoSpeed < 20)
            {
                OffMotorcycle(p);
                return;
            }
            return;
        }
        if (p->OnBoat)
        {
            if (p->MotoSpeed < 20)
            {
                OffBoat(p);
                return;
            }
            return;
        }
        neartag(p->posx,p->posy,p->posz,sprite[p->i].sectnum,p->oang,&neartagsector,&neartagwall,&neartagsprite,&neartaghitdist,1280L,3);
#endif
        
#if defined(DESYNCCORRECT) && !defined(RRRA)
        fakejval = -1;
#endif
        if(p->newowner >= 0)
            neartag(p->oposx,p->oposy,p->oposz,sprite[p->i].sectnum,p->oang,&neartagsector,&neartagwall,&neartagsprite,&neartaghitdist,1280L,1);
        else
        {
            neartag(p->posx,p->posy,p->posz,sprite[p->i].sectnum,p->oang,&neartagsector,&neartagwall,&neartagsprite,&neartaghitdist,1280L,1);
            if(neartagsprite == -1 && neartagwall == -1 && neartagsector == -1)
                neartag(p->posx,p->posy,p->posz+(8<<8),sprite[p->i].sectnum,p->oang,&neartagsector,&neartagwall,&neartagsprite,&neartaghitdist,1280L,1);
            if(neartagsprite == -1 && neartagwall == -1 && neartagsector == -1)
                neartag(p->posx,p->posy,p->posz+(16<<8),sprite[p->i].sectnum,p->oang,&neartagsector,&neartagwall,&neartagsprite,&neartaghitdist,1280L,1);
            if(neartagsprite == -1 && neartagwall == -1 && neartagsector == -1)
            {
                neartag(p->posx,p->posy,p->posz+(16<<8),sprite[p->i].sectnum,p->oang,&neartagsector,&neartagwall,&neartagsprite,&neartaghitdist,1280L,3);
                if(neartagsprite >= 0)
                {
                    switch(sprite[neartagsprite].picnum)
                    {
                        case FEM10:
                        case NAKED1:
                        case STATUE:
                        case TOUGHGAL:
                            return;
                        case COW:
                            sprite[neartagsprite].filler = 1;
                            return;
                    }
                }

                neartagsprite = -1;
                neartagwall = -1;
                neartagsector = -1;
            }
        }

        if(p->newowner == -1 && neartagsprite == -1 && neartagsector == -1 && neartagwall == -1 )
            if( isanunderoperator(sector[sprite[p->i].sectnum].lotag) )
                neartagsector = sprite[p->i].sectnum;

        if( neartagsector >= 0 && (sector[neartagsector].lotag&16384) )
            return;

        if( neartagsprite == -1 && neartagwall == -1)
            if(sector[p->cursectnum].lotag == 2 )
            {
                oldz = hitasprite(p->i,&neartagsprite);
                if(oldz > 1280) neartagsprite = -1;
            }

        if(neartagsprite >= 0)
        {
#if defined(DESYNCCORRECT) && !defined(RRRA)
            fakejval = snum;
#endif
            if( checkhitswitch(snum,neartagsprite,1) ) return;

            switch(sprite[neartagsprite].picnum)
            {
#ifdef RRRA
                case RRTILE8448:
                    if (Sound[340].num == 0)
                        spritesound(340, neartagsprite);
                    return;
                case RRTILE8704:
                    if (numplayers == 1)
                    {
                        if (Sound[445].num == 0 && dword_119C08 == 0)
                        {
                            spritesound(445, neartagsprite);
                            dword_119C08 = 1;
                        }
                        else if (Sound[445].num == 0 && Sound[446].num == 0 && Sound[447].num == 0 && dword_119C08 == 0)
                        {
                            short snd = TRAND%2;
                            if (snd == 1)
                                spritesound(446, neartagsprite);
                            else
                                spritesound(447, neartagsprite);
                        }
                    }
                    return;
                case EMPTYBIKE:
                    OnMotorcycle(p, neartagsprite);
                    return;
                case EMPTYBOAT:
                    OnBoat(p, neartagsprite);
                    return;
                case RRTILE8164:
                case RRTILE8165:
                case RRTILE8166:
                case RRTILE8167:
                case RRTILE8168:
                case RRTILE8591:
                case RRTILE8592:
                case RRTILE8593:
                case RRTILE8594:
                case RRTILE8595:
                    sprite[neartagsprite].extra = 60;
                    spritesound(235, neartagsprite);
                    return;
#endif
                case TOILET:
                case STALL:
                case RRTILE2121:
                case RRTILE2122:
                    if(p->last_pissed_time == 0)
                    {
                        if(ud.lockout == 0) spritesound(435,p->i);
                        
                        p->last_pissed_time = 26*220;
                        p->last_pissed_time = 26*220;
                        p->transporter_hold = 29*2;
                        if(p->holster_weapon == 0)
                        {
                            p->holster_weapon = 1;
                            p->weapon_pos = -1;
                        }
                        if(sprite[p->i].extra <= (max_player_health-(max_player_health/10) ) )
                        {
                            sprite[p->i].extra += max_player_health/10;
                            p->last_extra = sprite[p->i].extra;
                        }
                        else if(sprite[p->i].extra < max_player_health )
                             sprite[p->i].extra = max_player_health;
                    }
                    else if(Sound[DUKE_GRUNT].num == 0)
#if defined(DESYNCCORRECT) && !defined(RRRA)
                    {
                        fakejval = -1;
                        spritesound(DUKE_GRUNT, p->i);
                    }
#else
                        spritesound(DUKE_GRUNT,p->i);
#endif
                    return;
                case WATERFOUNTAIN:
                    if(hittype[neartagsprite].temp_data[0] != 1)
                    {
                        hittype[neartagsprite].temp_data[0] = 1;
                        sprite[neartagsprite].owner = p->i;

                        if(sprite[p->i].extra < max_player_health)
                        {
                            sprite[p->i].extra++;
#if defined(DESYNCCORRECT) && !defined(RRRA)
                            fakejval = -1;
#endif
                            spritesound(DUKE_DRINKING,p->i);
                        }
                    }
                    return;
                case PLUG:
#if defined(DESYNCCORRECT) && !defined(RRRA)
                    fakejval = -1;
#endif
                    spritesound(SHORT_CIRCUIT,p->i);
                    sprite[p->i].extra -= 2+(TRAND&3);
                    p->pals[0] = 48;
                    p->pals[1] = 48;
                    p->pals[2] = 64;
                    p->pals_time = 32;
                    break;
            }
        }

        if( (sync[snum].bits&(1<<29)) == 0 ) return;


#if defined(DESYNCCORRECT) && !defined(RRRA)
        if (neartagwall == -1 && neartagsector == -1 && neartagsprite == -1)
            fakejval = snum;
#endif

        if(neartagwall == -1 && neartagsector == -1 && neartagsprite == -1)
            if( klabs(hits(p->i)) < 512 )
        {
#if defined(DESYNCCORRECT) && !defined(RRRA)
            fakejval = -1;
#endif
            if( (TRAND&255) < 16 )
                spritesound(DUKE_SEARCH2,p->i);
            else spritesound(DUKE_SEARCH,p->i);
            return;
        }

        if( neartagwall >= 0 )
        {
            if( wall[neartagwall].lotag > 0 && isadoorwall(wall[neartagwall].picnum) )
            {
                if(hitscanwall == neartagwall || hitscanwall == -1)
#if defined(DESYNCCORRECT) && !defined(RRRA)
                {
                    fakejval = snum;
                    checkhitswitch(snum, neartagwall, 0);
                }
#else
                    checkhitswitch(snum,neartagwall,0);
#endif
                return;
            }
        }

        if( neartagsector >= 0 && (sector[neartagsector].lotag&16384) == 0 && isanearoperator(sector[neartagsector].lotag) )
        {
            short unk = 0;
            i = headspritesect[neartagsector];
            while(i >= 0)
            {
                if( PN == ACTIVATOR || PN == MASTERSWITCH )
                    return;
                i = nextspritesect[i];
            }
            if (haskey(neartagsector,snum))
#if defined(DESYNCCORRECT) && !defined(RRRA)
            {
                fakejval = -1;
                operatesectors(neartagsector,p->i);
            }
#else
                operatesectors(neartagsector,p->i);
#endif
            else
            {
                if (sector[neartagsector].filler > 3)
                    spritesound(99,p->i);
                else
                    spritesound(419,p->i);
#if defined(DESYNCCORRECT) && !defined(RRRA)
                fakejval = snum;
#endif
                FTA(41,p);
            }
        }
        else if( (sector[sprite[p->i].sectnum].lotag&16384) == 0 )
        {
            if( isanunderoperator(sector[sprite[p->i].sectnum].lotag) )
            {
                i = headspritesect[sprite[p->i].sectnum];
                while(i >= 0)
                {
                    if(PN == ACTIVATOR || PN == MASTERSWITCH) return;
                    i = nextspritesect[i];
                }
                if (haskey(neartagsector,snum))
#if defined(DESYNCCORRECT) && !defined(RRRA)
                {
                    fakejval = -1;
                    operatesectors(sprite[p->i].sectnum,p->i);
                }
#else
                    operatesectors(sprite[p->i].sectnum,p->i);
#endif
                else
                {
                    if (sector[neartagsector].filler > 3)
                        spritesound(99,p->i);
                    else
                        spritesound(419,p->i);
#if defined(DESYNCCORRECT) && !defined(RRRA)
                    fakejval = snum;
#endif
                    FTA(41,p);
                }
            }
#if defined(DESYNCCORRECT) && !defined(RRRA)
            else { fakejval = snum; checkhitswitch(snum,neartagwall,0); }
#else
            else checkhitswitch(snum,neartagwall,0);
#endif
        }
    }
}

void dofurniture(short wl, short sect, short snum)
{
    short startwall;
    short endwall;
    short i;
    short var_C;
    long x; 
    long y;
    long min_x;
    long min_y;
    long max_x;
    long max_y;
    long ins;
    long var_cx;

    startwall = sector[wall[wl].nextsector].wallptr;;
    endwall = startwall + sector[wall[wl].nextsector].wallnum;
    var_C = 1;
    max_x = max_y = -0x20000;
    min_x = min_y = 0x20000;
    var_cx = sector[sect].hitag;
    if (var_cx > 16)
        var_cx = 16;
    else if (var_cx == 0)
        var_cx = 4;
    for (i = startwall; i < endwall; i++)
    {
        x = wall[i].x;
        y = wall[i].y;
        if (x > max_x)
            max_x = x;
        if (y > max_y)
            max_y = y;
        if (x < min_x)
            min_x = x;
        if (y < min_y)
            min_y = y;
    }
    max_x += var_cx+1;
    max_y += var_cx+1;
    min_x -= var_cx+1;
    min_y -= var_cx+1;
    ins = inside(max_x, max_y, sect);
    if (!ins)
        var_C = 0;
    ins = inside(max_x, min_y, sect);
    if (!ins)
        var_C = 0;
    ins = inside(min_x, min_y, sect);
    if (!ins)
        var_C = 0;
    ins = inside(min_x, max_y, sect);
    if (!ins)
        var_C = 0;
    if (var_C)
    {
        if (Sound[389].num == 0)
            callsound2(389,snum);
        for (i = startwall; i < endwall; i++)
        {
            x = wall[i].x;
            y = wall[i].y;
            switch (wall[wl].lotag)
            {
                case 42:
                    y = wall[i].y + var_cx;
                    dragpoint(i,x,y);
                    break;
                case 41:
                    x = wall[i].x - var_cx;
                    dragpoint(i,x,y);
                    break;
                case 40:
                    y = wall[i].y - var_cx;
                    dragpoint(i,x,y);
                    break;
                case 43:
                    x = wall[i].x + var_cx;
                    dragpoint(i,x,y);
                    break;
            }
        }
    }
    else
    {
        for (i = startwall; i < endwall; i++)
        {
            x = wall[i].x;
            y = wall[i].y;
            switch (wall[wl].lotag)
            {
                case 42:
                    y = wall[i].y - (var_cx-2);
                    dragpoint(i,x,y);
                    break;
                case 41:
                    x = wall[i].x + (var_cx-2);
                    dragpoint(i,x,y);
                    break;
                case 40:
                    y = wall[i].y + (var_cx-2);
                    dragpoint(i,x,y);
                    break;
                case 43:
                    x = wall[i].x - (var_cx-2);
                    dragpoint(i,x,y);
                    break;
            }
        }
    }
}
