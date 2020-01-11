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

extern int32 turnheldtime; //MED
extern int32 lastcontroltime; //MED

#define TURBOTURNTIME (TICRATE/8) // 7
#define NORMALTURN   15
#define PREAMBLETURN 5
#define NORMALKEYMOVE 40
#define MAXVEL       ((NORMALKEYMOVE*2)+10)
#define MAXSVEL      ((NORMALKEYMOVE*2)+10)
#define MAXANGVEL    127
#define MAXHORIZ     127

static long myaimmode = 0, myaimstat = 0, omyaimstat = 0;

void getinput(short snum)
{

    short j, daang;
// MED
    ControlInfo info;
    int32 tics;
    boolean running;
    int32 turnamount;
    int32 keymove;
    int32 momx,momy;
    struct player_struct *p;

    p = &ps[snum];
    momx = momy = 0;

    CONTROL_GetInput( &info );

    if( (p->gm&MODE_MENU) || (p->gm&MODE_TYPE) || (ud.pause_on && !KB_KeyPressed(sc_Pause)) )
    {
         loc.fvel = vel = 0;
         loc.svel = svel = 0;
         loc.avel = angvel = 0;
         loc.horz = horiz = 0;
         loc.bits = (((long)gamequit)<<26);
         info.dz = info.dyaw = 0;
         return;
    }

    tics = totalclock-lastcontroltime;
    lastcontroltime = totalclock;

    if (MouseAiming)
          myaimmode = BUTTON(gamefunc_Mouse_Aiming);
     else
	 {
		  omyaimstat = myaimstat; myaimstat = BUTTON(gamefunc_Mouse_Aiming);
		  if (myaimstat > omyaimstat)
          {
				myaimmode ^= 1;
                FTA(44+myaimmode,p);
          }
	 }

    loc.bits =   BUTTON(gamefunc_Jump);
    loc.bits |=   BUTTON(gamefunc_Crouch)<<1;
    if (loc.bits & 2)
        loc.bits &= ~1;
    loc.bits |=   BUTTON(gamefunc_Fire)<<2;
    loc.bits |=   BUTTON(gamefunc_Aim_Up)<<3;
    loc.bits |=   BUTTON(gamefunc_Aim_Down)<<4;
    loc.bits |=   BUTTON(gamefunc_Run)<<5;
    if (ps[snum].drink_amt > 88)
        loc.bits |= 64;
    else
        loc.bits |=   BUTTON(gamefunc_Look_Left)<<6;
    loc.bits |=   BUTTON(gamefunc_Look_Right)<<7;

    j=0;
    if (BUTTON(gamefunc_Weapon_1))
       j = 1;
    if (BUTTON(gamefunc_Weapon_2))
       j = 2;
    if (BUTTON(gamefunc_Weapon_3))
       j = 3;
    if (BUTTON(gamefunc_Weapon_4))
       j = 4;
    if (BUTTON(gamefunc_Weapon_5))
       j = 5;
    if (BUTTON(gamefunc_Weapon_6))
       j = 6;

    if (BUTTON(gamefunc_Previous_Weapon))
        j = 11;
    if (BUTTON(gamefunc_Next_Weapon))
        j = 12;

    if (BUTTON(gamefunc_Weapon_7))
        j = 7;
    if (BUTTON(gamefunc_Weapon_8))
       j = 8;
    if (BUTTON(gamefunc_Weapon_9))
       j = 9;
    if (BUTTON(gamefunc_Weapon_10))
       j = 10;

    loc.bits |=   j<<8;
    loc.bits |=   BUTTON(gamefunc_Steroids)<<12;
    loc.bits |=   BUTTON(gamefunc_Look_Up)<<13;
    if (ps[snum].drink_amt > 99)
        loc.bits |= 16384;
    loc.bits |=   BUTTON(gamefunc_Look_Down)<<14;
    loc.bits |=   BUTTON(gamefunc_NightVision)<<15;
    loc.bits |=   BUTTON(gamefunc_MedKit)<<16;

    if(multiflag == 1)
    {
        loc.bits |=   1<<17;
        loc.bits |=   multiwhat<<18;
        loc.bits |=   multipos<<19;
        multiflag = 0;
        return;
    }
    loc.bits |=   BUTTON(gamefunc_Center_View)<<18;
    loc.bits |=   BUTTON(gamefunc_Holster_Weapon)<<19;
    loc.bits |=   BUTTON(gamefunc_Inventory_Left)<<20;
    loc.bits |=   KB_KeyPressed(sc_Pause)<<21;
    loc.bits |=   BUTTON(gamefunc_Quick_Kick)<<22;
    loc.bits |=   myaimmode<<23;
    loc.bits |=   BUTTON(gamefunc_Holo_Duke)<<24;
    loc.bits |=   BUTTON(gamefunc_Jetpack)<<25;
    loc.bits |=   (((long)gamequit)<<26);
    loc.bits |=   BUTTON(gamefunc_Inventory_Right)<<27;
    loc.bits |=   BUTTON(gamefunc_TurnAround)<<28;
    loc.bits |=   BUTTON(gamefunc_Open)<<29;
    loc.bits |=   BUTTON(gamefunc_Inventory)<<30;
#ifndef RRRA
    loc.bits |=   KB_KeyPressed(sc_Escape)<<31;
#endif

    running = BUTTON(gamefunc_Run)|ud.auto_run;
    svel = vel = angvel = horiz = 0;

    if( CONTROL_JoystickEnabled )
        if ( running ) info.dz *= 2;

    if( BUTTON(gamefunc_Strafe) )
       svel = -info.dyaw/8;
    else angvel = info.dyaw/64;

    if( myaimmode )
    {
        if(ud.mouseflip)
            horiz -= info.dz/(314-128);
        else horiz += info.dz/(314-128);

        info.dz = 0;
    }

    svel -= info.dx;
    vel = -info.dz>>6;

    if (running)
    {
        turnamount = NORMALTURN<<1;
        keymove = NORMALKEYMOVE<<1;
    }
    else
    {
        turnamount = NORMALTURN;
        keymove = NORMALKEYMOVE;
    }

    if (BUTTON(gamefunc_Strafe))
    {
        if ( BUTTON(gamefunc_Turn_Left))
           {
           svel -= -keymove;
           }
        if ( BUTTON(gamefunc_Turn_Right))
           {
           svel -= keymove;
           }
    }
    else
    {
        if ( BUTTON(gamefunc_Turn_Left))
           {
           turnheldtime += tics;
           if (turnheldtime>=TURBOTURNTIME)
              {
              angvel -= turnamount;
              }
           else
              {
              angvel -= PREAMBLETURN;
              }
           }
        else if ( BUTTON(gamefunc_Turn_Right))
           {
           turnheldtime += tics;
           if (turnheldtime>=TURBOTURNTIME)
              {
              angvel += turnamount;
              }
           else
              {
              angvel += PREAMBLETURN;
              }
           }
        else
           {
           turnheldtime=0;
           }
    }

    if ( BUTTON( gamefunc_Strafe_Left ) )
        svel += keymove;

    if ( BUTTON( gamefunc_Strafe_Right ) )
        svel += -keymove;

    if (BUTTON(gamefunc_Quick_Kick))
    {
        if (BUTTON(gamefunc_Move_Forward))
        {
            loc.bits |= 8;
        }
        if (BUTTON(gamefunc_Move_Backward))
        {
            loc.bits |= 16;
        }
    }
    else
    {
        if (ps[snum].drink_amt >= 66 && ps[snum].drink_amt <= 87)
        {
            if (BUTTON(gamefunc_Move_Forward))
            {
                vel += keymove;
                if (ps[snum].drink_amt & 1)
                    svel += keymove;
                else
                    svel -= keymove;
            }
            if (BUTTON(gamefunc_Move_Backward))
            {
                vel -= keymove;
                if (ps[snum].drink_amt & 1)
                    svel -= keymove;
                else
                    svel += keymove;
            }
        }
        else
        {
            if ( BUTTON(gamefunc_Move_Forward) )
                vel += keymove;

            if ( BUTTON(gamefunc_Move_Backward) )
                vel += -keymove;
        }
    }

    if(vel < -MAXVEL) vel = -MAXVEL;
    if(vel > MAXVEL) vel = MAXVEL;
    if(svel < -MAXSVEL) svel = -MAXSVEL;
    if(svel > MAXSVEL) svel = MAXSVEL;
    if(angvel < -MAXANGVEL) angvel = -MAXANGVEL;
    if(angvel > MAXANGVEL) angvel = MAXANGVEL;
    if(horiz < -MAXHORIZ) horiz = -MAXHORIZ;
    if(horiz > MAXHORIZ) horiz = MAXHORIZ;

    if(ud.scrollmode && ud.overhead_on)
    {
        ud.folfvel = vel;
        ud.folavel = angvel;
        loc.fvel = 0;
        loc.svel = 0;
        loc.avel = 0;
        loc.horz = 0;
        return;
    }

    if( numplayers > 1 )
        daang = myang;
    else daang = p->ang;

    momx = mulscale9(vel,sintable[(daang+2560)&2047]);
    momy = mulscale9(vel,sintable[(daang+2048)&2047]);

    momx += mulscale9(svel,sintable[(daang+2048)&2047]);
    momy += mulscale9(svel,sintable[(daang+1536)&2047]);

    momx += fricxv;
    momy += fricyv;

    loc.fvel = momx;
    loc.svel = momy;
    loc.avel = angvel;
    loc.horz = horiz;
}

#ifdef RRRA
long dword_119BFC = 0;
short bike_turn = 0;
void getinputmotorcycle(short snum)
{

    short j, daang;
// MED
    ControlInfo info;
    int32 tics;
    boolean turnl,turnr;
    int32 turnamount;
    int32 keymove;
    int32 momx,momy;
    struct player_struct *p;

    p = &ps[snum];

    CONTROL_GetInput( &info );

    if( (p->gm&MODE_MENU) || (p->gm&MODE_TYPE) || (ud.pause_on && !KB_KeyPressed(sc_Pause)) )
    {
         loc.fvel = vel = 0;
         loc.svel = svel = 0;
         loc.avel = angvel = 0;
         loc.horz = horiz = 0;
         loc.bits = (((long)gamequit)<<26);
         info.dz = info.dyaw = 0;
         return;
    }

    tics = totalclock-lastcontroltime;
    lastcontroltime = totalclock;
    loc.bits = 0;
    loc.bits |=   BUTTON(gamefunc_Fire)<<2;

    j=0;
    loc.bits |=   BUTTON(gamefunc_Steroids)<<12;
    loc.bits |=   BUTTON(gamefunc_NightVision)<<15;
    loc.bits |=   BUTTON(gamefunc_MedKit)<<16;

    if(multiflag == 1)
    {
        loc.bits |=   1<<17;
        loc.bits |=   multiwhat<<18;
        loc.bits |=   multipos<<19;
        multiflag = 0;
        return;
    }
    loc.bits |=   BUTTON(gamefunc_Inventory_Left)<<20;
    loc.bits |=   KB_KeyPressed(sc_Pause)<<21;
    loc.bits |=   BUTTON(gamefunc_Holo_Duke)<<24;
    loc.bits |=   BUTTON(gamefunc_Jetpack)<<25;
    loc.bits |=   (((long)gamequit)<<26);
    loc.bits |=   BUTTON(gamefunc_Inventory_Right)<<27;
    loc.bits |=   BUTTON(gamefunc_Open)<<29;
    loc.bits |=   BUTTON(gamefunc_Inventory)<<30;

    svel = vel = angvel = horiz = 0;

    turnl = BUTTON(gamefunc_Turn_Left);
    turnr = BUTTON(gamefunc_Turn_Right);

    bike_turn = info.dyaw/64;
    if (bike_turn > 0)
        turnr = 1;
    if (bike_turn < 0)
        turnl = 1;

    if (!p->raat5b9)
    {
        if (BUTTON(gamefunc_Move_Forward) || BUTTON(gamefunc_Strafe))
            loc.bits |= 1;
        if (BUTTON(gamefunc_Move_Backward))
            loc.bits |= 8;
        if (BUTTON(gamefunc_Run))
            loc.bits |= 2;
    }
    if (turnl)
        loc.bits |= 16;
    if (turnr)
        loc.bits |= 64;

    if (BUTTON(gamefunc_Move_Backward) && p->MotoSpeed <= 0)
        j = 1;

    if (p->MotoSpeed == 0 || !p->on_ground)
    {
        if (turnl)
        {
            p->TiltStatus--;
            if (p->TiltStatus < -10)
                p->TiltStatus = -10;
        }
        else if (turnr)
        {
            p->TiltStatus++;
            if (p->TiltStatus > 10)
                p->TiltStatus = 10;
        }
    }
    else
    {
        if (turnl || p->raat5c1 < 0)
        {
            turnheldtime += tics;
            p->TiltStatus--;
            if (p->TiltStatus < -10)
                p->TiltStatus = -10;
            if (turnheldtime >= TURBOTURNTIME && p->MotoSpeed > 0)
            {
                if (j)
                {
                    if (bike_turn)
                        angvel += 20;
                    else
                        angvel += 10;
                }
                else
                {
                    if (bike_turn)
                        angvel -= 20;
                    else
                        angvel -= 10;
                }
            }
            else
            {
                if (j)
                {
                    if (bike_turn)
                        angvel += 10;
                    else
                        angvel += 3;
                }
                else
                {
                    if (bike_turn)
                        angvel -= 10;
                    else
                        angvel -= 3;
                }
            }
        }
        else if (turnr || p->raat5c1 > 0)
        {
            turnheldtime += tics;
            p->TiltStatus++;
            if (p->TiltStatus > 10)
                p->TiltStatus = 10;
            if (turnheldtime >= TURBOTURNTIME && p->MotoSpeed > 0)
            {
                if (j)
                {
                    if (bike_turn)
                        angvel -= 20;
                    else
                        angvel -= 10;
                }
                else
                {
                    if (bike_turn)
                        angvel += 20;
                    else
                        angvel += 10;
                }
            }
            else
            {
                if (j)
                {
                    if (bike_turn)
                        angvel -= 10;
                    else
                        angvel -= 3;
                }
                else
                {
                    if (bike_turn)
                        angvel += 10;
                    else
                        angvel += 3;
                }
            }
        }
        else
        {
            turnheldtime = 0;

            if (p->TiltStatus > 0)
                p->TiltStatus--;
            else if (p->TiltStatus < 0)
                p->TiltStatus++;
        }
    }

    if (p->raat5b9)
        p->MotoSpeed = 0;

    vel += p->MotoSpeed;

    if(vel < -15) vel = -15;
    if(vel > 120) vel = 120;
    if(svel < -MAXSVEL) svel = -MAXSVEL;
    if(svel > MAXSVEL) svel = MAXSVEL;
    if(angvel < -MAXANGVEL) angvel = -MAXANGVEL;
    if(angvel > MAXANGVEL) angvel = MAXANGVEL;
    if(horiz < -MAXHORIZ) horiz = -MAXHORIZ;
    if(horiz > MAXHORIZ) horiz = MAXHORIZ;

    if(ud.scrollmode && ud.overhead_on)
    {
        ud.folfvel = vel;
        ud.folavel = angvel;
        loc.fvel = 0;
        loc.svel = 0;
        loc.avel = 0;
        loc.horz = 0;
        return;
    }

    if( numplayers > 1 )
        daang = myang;
    else daang = p->ang;

    momx = mulscale9(vel,sintable[(daang+2560)&2047]);
    momy = mulscale9(vel,sintable[(daang+2048)&2047]);

    momx += mulscale9(svel,sintable[(daang+2048)&2047]);
    momy += mulscale9(svel,sintable[(daang+1536)&2047]);

    momx += fricxv;
    momy += fricyv;

    loc.fvel = momx;
    loc.svel = momy;
    loc.avel = angvel;
    loc.horz = horiz;
}

void getinputboat(short snum)
{

    short j, daang;
// MED
    ControlInfo info;
    int32 tics;
    boolean turnl,turnr;
    int32 turnamount;
    int32 keymove;
    int32 momx,momy;
    struct player_struct *p;

    p = &ps[snum];

    CONTROL_GetInput( &info );

    if( (p->gm&MODE_MENU) || (p->gm&MODE_TYPE) || (ud.pause_on && !KB_KeyPressed(sc_Pause)) )
    {
         loc.fvel = vel = 0;
         loc.svel = svel = 0;
         loc.avel = angvel = 0;
         loc.horz = horiz = 0;
         loc.bits = (((long)gamequit)<<26);
         info.dz = info.dyaw = 0;
         return;
    }

    tics = totalclock-lastcontroltime;
    lastcontroltime = totalclock;
    loc.bits = 0;
    loc.bits |=   BUTTON(gamefunc_Fire)<<2;

    j=0;
    loc.bits |=   BUTTON(gamefunc_Steroids)<<12;
    loc.bits |=   BUTTON(gamefunc_NightVision)<<15;
    loc.bits |=   BUTTON(gamefunc_MedKit)<<16;

    if(multiflag == 1)
    {
        loc.bits |=   1<<17;
        loc.bits |=   multiwhat<<18;
        loc.bits |=   multipos<<19;
        multiflag = 0;
        return;
    }
    loc.bits |=   BUTTON(gamefunc_Inventory_Left)<<20;
    loc.bits |=   KB_KeyPressed(sc_Pause)<<21;
    loc.bits |=   BUTTON(gamefunc_Holo_Duke)<<24;
    loc.bits |=   BUTTON(gamefunc_Jetpack)<<25;
    loc.bits |=   (((long)gamequit)<<26);
    loc.bits |=   BUTTON(gamefunc_Inventory_Right)<<27;
    loc.bits |=   BUTTON(gamefunc_Open)<<29;
    loc.bits |=   BUTTON(gamefunc_Inventory)<<30;

    svel = vel = angvel = horiz = 0;

    turnl = BUTTON(gamefunc_Turn_Left);
    turnr = BUTTON(gamefunc_Turn_Right);

    bike_turn = info.dyaw/64;
    if (bike_turn > 0)
        turnr = 1;
    if (bike_turn < 0)
        turnl = 1;

    if (BUTTON(gamefunc_Move_Forward) || BUTTON(gamefunc_Strafe))
        loc.bits |= 1;
    if (BUTTON(gamefunc_Move_Backward))
        loc.bits |= 8;
    if (BUTTON(gamefunc_Run))
        loc.bits |= 2;

    if (turnl)
        loc.bits |= 16;
    if (turnr)
        loc.bits |= 64;

    if (BUTTON(gamefunc_Move_Backward) && p->MotoSpeed <= 0)
        j = 1;

    if (p->MotoSpeed)
    {
        if (turnl || p->raat5c1 < 0)
        {
            turnheldtime += tics;
            if (!p->NotOnWater)
            {
                p->TiltStatus--;
                if (p->TiltStatus < -10)
                    p->TiltStatus = -10;
            }
            if (turnheldtime >= TURBOTURNTIME && p->MotoSpeed != 0)
            {
                if (p->NotOnWater)
                {
                    if (bike_turn)
                        angvel -= 6;
                    else
                        angvel -= 3;
                }
                else
                {
                    if (bike_turn)
                        angvel -= 20;
                    else
                        angvel -= 10;
                }
            }
            else if (turnheldtime < TURBOTURNTIME && p->MotoSpeed != 0)
            {
                if (p->NotOnWater)
                {
                    if (bike_turn)
                        angvel -= 2;
                    else
                        angvel -= 1;
                }
                else
                {
                    if (bike_turn)
                        angvel -= 6;
                    else
                        angvel -= 3;
                }
            }
        }
        else if (turnr || p->raat5c1 > 0)
        {
            turnheldtime += tics;
            if (!p->NotOnWater)
            {
                p->TiltStatus++;
                if (p->TiltStatus > 10)
                    p->TiltStatus = 10;
            }
            if (turnheldtime >= TURBOTURNTIME && p->MotoSpeed != 0)
            {
                if (p->NotOnWater)
                {
                    if (bike_turn)
                        angvel += 6;
                    else
                        angvel += 3;
                }
                else
                {
                    if (bike_turn)
                        angvel += 20;
                    else
                        angvel += 10;
                }
            }
            else if (turnheldtime < TURBOTURNTIME && p->MotoSpeed != 0)
            {
                if (p->NotOnWater)
                {
                    if (bike_turn)
                        angvel += 2;
                    else
                        angvel += 1;
                }
                else
                {
                    if (bike_turn)
                        angvel += 6;
                    else
                        angvel += 3;
                }
            }
        }
        else if (!p->NotOnWater)
        {
            turnheldtime = 0;

            if (p->TiltStatus > 0)
                p->TiltStatus--;
            else if (p->TiltStatus < 0)
                p->TiltStatus++;
        }
    }
    else if (!p->NotOnWater)
    {
        turnheldtime = 0;

        if (p->TiltStatus > 0)
            p->TiltStatus--;
        else if (p->TiltStatus < 0)
            p->TiltStatus++;
    }

    vel += p->MotoSpeed;

    if(vel < -15) vel = -15;
    if(vel > 120) vel = 120;
    if(svel < -MAXSVEL) svel = -MAXSVEL;
    if(svel > MAXSVEL) svel = MAXSVEL;
    if(angvel < -MAXANGVEL) angvel = -MAXANGVEL;
    if(angvel > MAXANGVEL) angvel = MAXANGVEL;
    if(horiz < -MAXHORIZ) horiz = -MAXHORIZ;
    if(horiz > MAXHORIZ) horiz = MAXHORIZ;

    if(ud.scrollmode && ud.overhead_on)
    {
        ud.folfvel = vel;
        ud.folavel = angvel;
        loc.fvel = 0;
        loc.svel = 0;
        loc.avel = 0;
        loc.horz = 0;
        return;
    }

    if( numplayers > 1 )
        daang = myang;
    else daang = p->ang;

    momx = mulscale9(vel,sintable[(daang+2560)&2047]);
    momy = mulscale9(vel,sintable[(daang+2048)&2047]);

    momx += mulscale9(svel,sintable[(daang+2048)&2047]);
    momy += mulscale9(svel,sintable[(daang+1536)&2047]);

    momx += fricxv;
    momy += fricyv;

    loc.fvel = momx;
    loc.svel = momy;
    loc.avel = angvel;
    loc.horz = horiz;
}
#endif

//UPDATE THIS FILE OVER THE OLD GETSPRITESCORE/COMPUTERGETINPUT FUNCTIONS
getspritescore(long snum, long dapicnum)
{
    switch(dapicnum)
    {
        case FIRSTGUNSPRITE: return(5);
        case CHAINGUNSPRITE: return(50);
        case RPGSPRITE: return(200);
#ifdef RRRA
        case RPG2SPRITE: return(200);
#endif
        case FREEZESPRITE: return(25);
        case SHRINKERSPRITE: return(80);
        case HEAVYHBOMB: return(60);
        case TRIPBOMBSPRITE: return(50);
        case SHOTGUNSPRITE: return(120);
        case DEVISTATORSPRITE: return(120);
        case BOWLINGBALLSPRITE: return(25);

        case FREEZEAMMO: if (ps[snum].ammo_amount[FREEZE_WEAPON] < max_ammo_amount[FREEZE_WEAPON]) return(10); else return(0);
        case AMMO: if (ps[snum].ammo_amount[SHOTGUN_WEAPON] < max_ammo_amount[SHOTGUN_WEAPON]) return(10); else return(0);
        case BATTERYAMMO: if (ps[snum].ammo_amount[CHAINGUN_WEAPON] < max_ammo_amount[CHAINGUN_WEAPON]) return(20); else return(0);
        case DEVISTATORAMMO: if (ps[snum].ammo_amount[DEVISTATOR_WEAPON] < max_ammo_amount[DEVISTATOR_WEAPON]) return(25); else return(0);
        case RPGAMMO: if (ps[snum].ammo_amount[RPG_WEAPON] < max_ammo_amount[RPG_WEAPON]) return(50); else return(0);
        case CRYSTALAMMO: if (ps[snum].ammo_amount[SHRINKER_WEAPON] < max_ammo_amount[SHRINKER_WEAPON]) return(10); else return(0);
        case HBOMBAMMO: if (ps[snum].ammo_amount[HANDBOMB_WEAPON] < max_ammo_amount[HANDBOMB_WEAPON]) return(30); else return(0);
        case SHOTGUNAMMO: if (ps[snum].ammo_amount[SHOTGUN_WEAPON] < max_ammo_amount[SHOTGUN_WEAPON]) return(25); else return(0);

        case COLA: if (sprite[ps[snum].i].extra < 100) return(10); else return(0);
        case SIXPAK: if (sprite[ps[snum].i].extra < 100) return(30); else return(0);
        case FIRSTAID: if (ps[snum].firstaid_amount < 100) return(100); else return(0);
        case SHIELD: if (ps[snum].shield_amount < 100) return(50); else return(0);
        case STEROIDS: if (ps[snum].steroids_amount == 400) return(30); else return(0);
        case AIRTANK: if (ps[snum].scuba_amount < 6400) return(30); else return(0);
        case JETPACK: if (ps[snum].jetpack_amount < 600) return(100); else return(0);
        case HEATSENSOR: if (ps[snum].heat_amount < 1200) return(10); else return(0);
        case ACCESSCARD: return(1);
        case BOOTS: if (ps[snum].boot_amount < 2000) return(50); else return(0);
        case ATOMICHEALTH: if (sprite[ps[snum].i].extra < max_player_health) return(50); else return(0);
        case HOLODUKE: if (ps[snum].holoduke_amount < 2400) return(30); else return(0);
    }
    return(0);
}

long wupass = 0;

char doincrements(struct player_struct *p)
{
    long /*j,*/i,snum;

#ifdef RRRA
    if (WindTime > 0)
        WindTime--;
    else if ((TRAND & 127) == 8)
    {
        WindTime = 120+((TRAND&63)<<2);
        WindDir = TRAND&2047;
    }
    
    if (BellTime > 0)
    {
        BellTime--;
        if (BellTime == 0)
            sprite[word_119BE0].picnum++;
    }
    if (p->raat605 > 0)
        p->raat605--;
    if (p->SeaSick)
    {
        p->SeaSick--;
        if (p->SeaSick == 0)
            p->raat5dd = 0;
    }
#endif

    snum = sprite[p->i].yvel;
//    j = sync[snum].avel;
//    p->weapon_ang = -(j/5);

    p->player_par++;
    if (p->at59d)
        p->at59d--;


#ifdef RRRA
    if (numplayers < 2)
#endif
    if (p->gm == MODE_GAME)
        if (cdon)
    {
        cdromtime--;
        if (cdromtime <= 0)
        {
            whichtrack++;
            if (whichtrack > cdhitrack)
                whichtrack = cdlotrack+1;
            rbstop();
            rbPlayTrack(whichtrack);
        }
    }

    if (p->at57e > 0)
    {
        p->at57e++;
        p->at57c--;
    }
    p->at58e--;
    if (p->at58e <= 0)
    {
        p->at596 = 1;
        p->at58e = 1024;
        if (p->drink_amt)
        {
            p->drink_amt--;
            p->at598 = 0;
        }
    }
    p->at592--;
    if (p->at592 <= 0)
    {
        p->at597 = 1;
        p->at592 = 1024;
        if (p->eat)
            p->eat--;
    }
    if (p->drink_amt > 89 && p->drink_amt == 100)
    {
        p->at598 = 1;
        if (Sound[420].num == 0)
            spritesound(420, p->i);
        p->drink_amt -= 9;
        p->eat >>= 1;
        p->at596 = p->at597 = 1;
    }
    p->eatang = (1647+p->eat*8)&2047;

    if (p->eat >= 100)
        p->eat = 100;

    if (p->eat >= 31 && TRAND < p->eat)
    {
        switch (TRAND&3)
        {
            case 0:
                spritesound(404,p->i);
                break;
            case 1:
                spritesound(422,p->i);
                break;
            case 2:
                spritesound(423,p->i);
                break;
            case 3:
                spritesound(424,p->i);
                break;
        }
        if (numplayers < 2)
        {
            p->at290 = 16384;
            madenoise(screenpeek);
            p->posxv += sintable[(p->ang+512)&2047]<<4;
            p->posyv += sintable[p->ang&2047]<<4;
        }
        p->eat -= 4;
        if (p->eat < 0)
            p->eat = 0;
    }

    if(p->invdisptime > 0)
        p->invdisptime--;

    if(p->tipincs > 0) p->tipincs--;

    if(p->last_pissed_time > 0 )
    {
        p->last_pissed_time--;

        if (p->drink_amt > 66 && (p->last_pissed_time % 26) == 0)
            p->drink_amt--;

        if (ud.lockout == 0)
        {
            if (p->last_pissed_time == 5662)
                spritesound(434,p->i);
            else if (p->last_pissed_time == 5567)
                spritesound(434,p->i);
            else if (p->last_pissed_time == 5472)
                spritesound(433,p->i);
            else if (p->last_pissed_time == 5072)
                spritesound(435,p->i);
            else if (p->last_pissed_time == 5014)
                spritesound(434,p->i);
            else if (p->last_pissed_time == 4919)
                spritesound(433,p->i);
        }

        if( p->last_pissed_time == 5668 )
        {
            p->holster_weapon = 0;
            p->weapon_pos = 10;
        }
    }

    if(p->crack_time > 0)
    {
        p->crack_time--;
        if(p->crack_time == 0)
        {
            p->knuckle_incs = 1;
            p->crack_time = 777;
        }
    }

    if( p->steroids_amount > 0 && p->steroids_amount < 400)
    {
        p->steroids_amount--;
        if(p->steroids_amount == 0)
        {
            checkavailinven(p);
            p->eat = p->drink_amt = 0;
            p->eatang = p->drunkang = 1647;
            p->at597 = p->at596 = 1;
        }
        if( !(p->steroids_amount&14) )
            if(snum == screenpeek || ud.coop == 1)
                spritesound(DUKE_TAKEPILLS,p->i);
    }

    if(p->access_incs && sprite[p->i].pal != 1)
    {
        p->access_incs++;
        if(sprite[p->i].extra <= 0)
            p->access_incs = 12;
        if(p->access_incs == 12)
        {
            if(p->access_spritenum >= 0)
            {
                checkhitswitch(snum,p->access_spritenum,1);
                switch(sprite[p->access_spritenum].pal)
                {
                    case 0:p->keys[1] = 1;break;
                    case 21:p->keys[2] = 1;break;
                    case 23:p->keys[3] = 1;break;
                }
                p->access_spritenum = -1;
            }
            else
            {
                checkhitswitch(snum,p->access_wallnum,0);
                switch(wall[p->access_wallnum].pal)
                {
                    case 0:p->keys[1] = 1;break;
                    case 21:p->keys[2] = 1;break;
                    case 23:p->keys[3] = 1;break;
                }
            }
        }

        if(p->access_incs > 20)
        {
            p->access_incs = 0;
            p->weapon_pos = 10;
            p->kickback_pic = 0;
        }
    }

    if(p->scuba_on == 0 && sector[p->cursectnum].lotag == 2)
    {
        if(p->scuba_amount > 0)
        {
            p->scuba_on = 1;
            p->inven_icon = 6;
            FTA(76,p);
        }
        else
        {
            if(p->airleft > 0)
                p->airleft--;
            else
            {
                p->extra_extra8 += 32;
                if(p->last_extra < (max_player_health>>1) && (p->last_extra&3) == 0)
                    spritesound(DUKE_LONGTERM_PAIN,p->i);
            }
        }
    }
    else if(p->scuba_amount > 0 && p->scuba_on)
    {
        p->scuba_amount--;
        if(p->scuba_amount == 0)
        {
            p->scuba_on = 0;
            checkavailinven(p);
        }
    }

    if(p->knuckle_incs)
    {
        p->knuckle_incs ++;
        if(p->knuckle_incs==10)
        {
            if (!wupass)
            {
                short snd = -1;
                wupass = 1;
                if (lastlevel)
                {
                    snd = 391;
                }
                else switch (ud.volume_number)
                {
                    case 0:
                        switch (ud.level_number)
                        {
                            case 0:
#ifdef RRRA
                                snd = 63;
#else
                                snd = 391;
#endif
                                break;
                            case 1:
                                snd = 64;
                                break;
                            case 2:
                                snd = 77;
                                break;
                            case 3:
                                snd = 80;
                                break;
                            case 4:
                                snd = 102;
                                break;
                            case 5:
                                snd = 103;
                                break;
                            case 6:
                                snd = 104;
                                break;
                        }
                        break;
                    case 1:
                        switch (ud.level_number)
                        {
                            case 0:
                                snd = 105;
                                break;
                            case 1:
                                snd = 176;
                                break;
                            case 2:
                                snd = 177;
                                break;
                            case 3:
                                snd = 198;
                                break;
                            case 4:
                                snd = 230;
                                break;
                            case 5:
                                snd = 255;
                                break;
                            case 6:
                                snd = 283;
                                break;
                        }
                        break;
                }
                if (snd == -1)
                    snd = 391;
                spritesound(snd,p->i);
            }
            else if(totalclock > 1024)
                if(snum == screenpeek || ud.coop == 1)
            {
                if(rand()&1)
                    spritesound(DUKE_CRACK,p->i);
                else spritesound(DUKE_CRACK2,p->i);
            }
        }
        else if( p->knuckle_incs == 22 || (sync[snum].bits&(1<<2)))
            p->knuckle_incs=0;

        return 1;
    }
    return 0;
}


void checkweapons(struct player_struct *p)
{
    short weapon_sprites[MAX_WEAPONS] = { KNEE, FIRSTGUNSPRITE, SHOTGUNSPRITE,
            CHAINGUNSPRITE, RPGSPRITE, HEAVYHBOMB, SHRINKERSPRITE, DEVISTATORSPRITE,
            TRIPBOMBSPRITE, BOWLINGBALLSPRITE, FREEZEBLAST, HEAVYHBOMB};
    short i,j;

#ifdef RRRA
    if (p->OnMotorcycle && numplayers > 1)
    {
        j = spawn(p->i, 7220);
        sprite[j].ang = p->ang;
        sprite[j].owner = p->ammo_amount[RA13_WEAPON];
        p->OnMotorcycle = 0;
        p->gotweapon[RA13_WEAPON] = 0;
        p->horiz = 100;
        p->raat5b5 = 0;
        p->MotoSpeed = 0;
        p->TiltStatus = 0;
        p->raat5c1 = 0;
        p->VBumpTarget = 0;
        p->VBumpNow = 0;
        p->TurbCount = 0;
    }
    else if (p->OnBoat && numplayers > 1)
    {
        j = spawn(p->i, 7233);
        sprite[j].ang = p->ang;
        sprite[j].owner = p->ammo_amount[RA14_WEAPON];
        p->OnBoat = 0;
        p->gotweapon[RA14_WEAPON] = 0;
        p->horiz = 100;
        p->raat5b5 = 0;
        p->MotoSpeed = 0;
        p->TiltStatus = 0;
        p->raat5c1 = 0;
        p->VBumpTarget = 0;
        p->VBumpNow = 0;
        p->TurbCount = 0;
    }
#endif
    if (p->curr_weapon > 0)
    {
        if(TRAND&1)
            spawn(p->i,weapon_sprites[p->curr_weapon]);
        else switch(p->curr_weapon)
        {
            case HANDBOMB_WEAPON:
            case RPG_WEAPON:
#ifdef RRRA
            case RA16_WEAPON:
#endif
                spawn(p->i,EXPLOSION2);
                break;
        }
    }

    for (i = 0; i < 5; i++)
    {
        if (p->keys[i] == 1)
        {
            j = spawn(p->i, ACCESSCARD);
            switch (i)
            {
                case 1:
                    sprite[j].lotag = 100;
                    break;
                case 2:
                    sprite[j].lotag = 101;
                    break;
                case 3:
                    sprite[j].lotag = 102;
                    break;
                case 4:
                    sprite[j].lotag = 103;
                    break;
            }
        }
    }
}