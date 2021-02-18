/***************************************************************************
 *   WHINP.C  - main game code for Apogee engine                           *
 *                                                                         *
 ***************************************************************************/

#include "input.h"
#include "player.h"
#include "sound.h"
#include "objects.h"
#include "menu.h"
#include "witchaven.h"
#include "network.h"
#include "effects.h"
#include "keyboard.h"
#include "control.h"
#include "scancodes.h"
#include "view.h"
#include "config.h"
#include "control.h"

#define WEAPONONE    0x02

#define MAX_ANGULAR_VELOCITY 127
#define TURN_ACCELERATION_RATE 16
#define TURN_DECELERATION_RATE 12
#define MAX_STRAFE_VELOCITY 127
#define MAX_MOVEMENT_VELOCITY 201
#define MOVEMENT_ACCELERATION_RATE 8
#define MOVEMENT_DECELERATION_RATE 2

void dosoundthing();
void nettypeletter();
void typeletter();

int soundcontrol;
int musiclevel;
int digilevel;
int soundtoggle;

extern int mapon;

bool followmode = false;
bool justplayed = false;
bool lopoint = false;
bool walktoggle = false;
int runningtime = 0;

int charsperline = 0;
char nettemp[80];
extern char typemessage[];
extern char typemessageleng, typemode;
extern char scantoasc[];
extern char scantoascwithshift[];
int nettypemode = 0;

extern char tempbuf[];

int mousecalibrate = 0;
int mousespeed = 3;
int mousxspeed = 3;
int mousyspeed = 3;

int32_t angvel, svel, vel;

extern int escapetomenu;

int32_t lockclock;

int oldhoriz;

#if 1
char joyb;
char oldjoyb;
char oldbstatus;
char butbit[] = {0x10, 0x20, 0x40, 0x80};
char mbutbit[] = {0x01, 0x02};

short jcalibration = 0;
short jctrx;
short jctry;
short jlowx, jhighx;
short jlowy, jhighy;
short jmovespeed = 16;
int joyx, joyy;
int joykeys[4];
int jstickenabled = 0;
int turnspeed = 16;
#endif

short mousekeys[2];

char option2[7];

extern char flashflag;

short oldmousestatus;
extern short compass;

short pitch, roll, yaw;

void initjstick()
{
    #if 0 // TODO
    jcalibration = 1;
    jstickenabled = 0;

    if (option2[3] == 0)
    {
        option2[3] = KEYFIRE;
    }
    if (option2[4] == 0)
    {
        option2[4] = KEYUSE;
    }
    if (option2[5] == 0)
    {
        option2[5] = KEYUSEP;
    }
    if (option2[6] == 0)
    {
        option2[6] = KEYSTRAFE;
    }

    joykeys[0] = option2[3];
    joykeys[1] = option2[4];
    joykeys[2] = option2[5];
    joykeys[3] = option2[6];
    #endif
}

void dophysics(Player* plr, int goalz, short flyupdn, int v)
{
    if (plr->orbactive[5] > 0)
    {
        if (v > 0)
        {
            if (plr->horiz > 125)
                plr->hvel -= (synctics << 6);
            else if (plr->horiz < 75)
                plr->hvel += (synctics << 6);
        }
        else
        {
            if (flyupdn > 0)
            {
                plr->hvel -= (synctics << 7);
            }
            if (flyupdn < 0)
            {
                plr->hvel += (synctics << 7);
            }
        }
 
        plr->hvel += Sin(lockclock << 4) >> 6;
        plr->fallz = 0;
    }
    else if (plr->z < goalz)
    {
        plr->hvel += GRAVITYCONSTANT;
        plr->onsomething &= ~(GROUNDBIT | PLATFORMBIT);
        plr->fallz += plr->hvel;
    }
    else if (plr->z > goalz)
    {
        plr->hvel -= ((plr->z - goalz) >> 6);
        plr->onsomething |= GROUNDBIT;
        plr->fallz = 0;
    }
    else
    {
        plr->fallz = 0;
    }

    plr->z += plr->hvel;

    if (plr->hvel > 0 && plr->z > goalz)
    {
        plr->hvel >>= 2;
    }
    else if (plr->onsomething != 0)
    {
        if (plr->hvel < 0 && plr->z < goalz)
        {
            plr->hvel = 0;
            plr->z = goalz;
        }
    }
    if (plr->z < sector[plr->sector].ceilingz)
    {
        plr->z = sector[plr->sector].ceilingz + (plr->height >> 2);
        plr->hvel = 0;
    }
    else if (plr->z > sector[plr->sector].floorz)
    {
        plr->z = sector[plr->sector].floorz - (plr->height >> 4);
        plr->hvel = 0;
    }
    #if 0
    if (plr->forcev != 0)
    {
        *xvect = (int)((plr->forcev * (int)sintable[(plr->forcea + 512) & kAngleMask]) >> 3);
        *yvect = (int)((plr->forcev * (int)sintable[plr->forcea]) >> 3);
        plr->forcev >>= 1;
    }
    #endif
}

void keytimerstuff()
{
    static int counter = 0;
    static int32_t turn = 0;

    if (BUTTON(gamefunc_Strafe))
    {
        if (BUTTON(gamefunc_Turn_Left))
        {
            svel += MOVEMENT_ACCELERATION_RATE;
            if (svel > MAX_STRAFE_VELOCITY) {
                svel = MAX_STRAFE_VELOCITY;
            }
        }
        if (BUTTON(gamefunc_Turn_Right))
        {
            svel -= MOVEMENT_ACCELERATION_RATE;
            if (svel < -MAX_STRAFE_VELOCITY - 1) {//Original says -128
                svel = -MAX_STRAFE_VELOCITY - 1;
            }
        }
    }
    else
    {
        if (BUTTON(gamefunc_Turn_Left))
        {
            angvel -= TURN_ACCELERATION_RATE;
            if (angvel < -MAX_ANGULAR_VELOCITY - 1) { //Original says -128
                angvel = -MAX_ANGULAR_VELOCITY - 1;
            }
        }
        if (BUTTON(gamefunc_Turn_Right))
        {
            angvel += TURN_ACCELERATION_RATE;
            if (angvel > MAX_ANGULAR_VELOCITY) {
                angvel = MAX_ANGULAR_VELOCITY;
            }
        }
    }

    if (BUTTON(gamefunc_Strafe_Left))
    {
        svel += MOVEMENT_ACCELERATION_RATE;
        if (svel > MAX_STRAFE_VELOCITY) {
            svel = MAX_STRAFE_VELOCITY;
        }
    }
    else if (BUTTON(gamefunc_Strafe_Right))
    {
        svel -= MOVEMENT_ACCELERATION_RATE;
        if (svel < -MAX_STRAFE_VELOCITY - 1) {//Original says -128
            svel = -MAX_STRAFE_VELOCITY - 1;
        }
    }

    if (BUTTON(gamefunc_Move_Forward))
    {
        vel += MOVEMENT_ACCELERATION_RATE;
        if (vel > MAX_MOVEMENT_VELOCITY) {
            vel = MAX_MOVEMENT_VELOCITY;
        }
    }

    if (BUTTON(gamefunc_Move_Backward))
    {
        vel -= MOVEMENT_ACCELERATION_RATE;
        if (vel < -MAX_MOVEMENT_VELOCITY) {
            vel = -MAX_MOVEMENT_VELOCITY;
        }
    }

    //Deceleration
    if (angvel < 0)
    {
        angvel += TURN_DECELERATION_RATE;
        if (angvel > 0) {
            angvel = 0;
        }
    }

    if (angvel > 0)
    {
        angvel -= TURN_DECELERATION_RATE;
        if (angvel < 0) {
            angvel = 0;
        }
    }

    if (svel < 0)
    {
        svel += MOVEMENT_DECELERATION_RATE;
        if (svel > 0) {
            svel = 0;
        }
    }

    if (svel > 0)
    {
        svel -= MOVEMENT_DECELERATION_RATE;
        if (svel < 0) {
            svel = 0;
        }
    }

    if (vel < 0)
    {
        vel += MOVEMENT_DECELERATION_RATE;
        if (vel > 0) {
            vel = 0;
        }
    }

    if (vel > 0)
    {
        vel -= MOVEMENT_DECELERATION_RATE;
        if (vel < 0) {
            vel = 0;
        }
    }
}

void processinput(Player* plr)
{
    short bstatus = 0;
    short mousx = 0;
    short mousy = 0;

    int32_t goalz, xvect, yvect;
    int32_t hihit, hiz, loz, lohit;
    int32_t dax, dax2, day, day2, odax, odax2, oday, oday2;
    int a, s, v;
    static int  mv;
    int32_t oldposx, oldposy;
    int32_t dist;
    int32_t feetoffground;
    short onsprite = -1;
    static short tempsectornum;
    short onground;

    a = angvel;
    s = svel;
    v = vel;

    ControlInfo info;
    CONTROL_ProcessBinds();
    CONTROL_GetInput(&info);

    //if backspace > 0 then type mode = 1
    //backspace = 0

    if (keystatus[sc_BackSpace] > 0)
    {
        if (netgame == 0)
        {
            if (typemode == 1)
            {
                typemode = 0;
                charsperline = 0;
                typemessageleng = 0;
            }
            else
            {
                typemode = 1;
            }
            keystatus[sc_BackSpace] = 0;
        }
        else
        {
            if (nettypemode == 1)
            {
                nettypemode = 0;
                charsperline = 0;
                typemessageleng = 0;
                strcpy(nettemp, "");
            }
            else
            {
                nettypemode = 1;
                typemessageleng = 0;
            }
            keystatus[sc_BackSpace] = 0;
        }
    }

    if (typemode == 1)
    {
        typeletter();
    }

    if (nettypemode == 1)
    {
        nettypeletter();
    }

    if (keystatus[sc_Escape] > 0)
    {
        keystatus[sc_Escape] = 0;
        if (plr->z < sector[plr->sector].floorz - ((PLAYERHEIGHT + 8) << 8)
            //     || sector[plr->sector].floorpicnum == LAVA
            //     || sector[plr->sector].floorpicnum == SLIME
            //     || sector[plr->sector].floorpicnum == WATER
            //     || sector[plr->sector].floorpicnum == HEALTHWATER
            //     || sector[plr->sector].floorpicnum == ANILAVA
            //     || sector[plr->sector].floorpicnum == LAVA1
            //     || sector[plr->sector].floorpicnum == LAVA2 )
            )
        {

            StatusMessage(360, "must be on the ground");
        }
        else
        {
            escapetomenu = 1;
            plr->z = sector[plr->sector].floorz - (PLAYERHEIGHT << 8);

            vec3_t pos;
            pos.x = plr->x;
            pos.y = plr->y;
            pos.z = plr->z + (plr->height << 8);

            setsprite(plr->spritenum, &pos);
            sprite[plr->spritenum].ang = plr->ang;

            visibility = 1024;
        }
    }

    if (followmode)
    {
        if (BUTTON(gamefunc_Turn_Left))
        {
            followx -= synctics << 5;
        }
        if (BUTTON(gamefunc_Turn_Right))
        {
            followx += synctics << 5;
        }
        if (BUTTON(gamefunc_Move_Forward))
        {
            followy -= synctics << 5;
        }
        if (BUTTON(gamefunc_Move_Backward))
        {
            followy += synctics << 5;
        }

        if (labs(followx) >= 65536 - 1024)
            followx = 65536 - 1024;

        if (labs(followy) >= 65536 - 1024)
            followy = 65536 - 1024;

        if (keystatus[sc_F] > 0)
        {
            keystatus[sc_F] = 0;
            followmode = false;
        }
        return;
    }

    if (keystatus[sc_F10] > 0) // F10 - brightness
    {
        keystatus[sc_F10] = 0;
        gbrightness = brightness++;

        if (brightness > 8)
        {
            brightness = 0;
            gbrightness = 0;
        }

        setbrightness(brightness);
    }

    if (v < -MAX_MOVEMENT_VELOCITY) v = -MAX_MOVEMENT_VELOCITY;
    else if (v > MAX_MOVEMENT_VELOCITY) v = MAX_MOVEMENT_VELOCITY;

    v += v >> 1;// SUPER MARIO BROTHERS

    if (s < -MAX_MOVEMENT_VELOCITY) s = -MAX_MOVEMENT_VELOCITY; // Yes, this is strafing, however, original code compares it against 201
    else if (s > MAX_MOVEMENT_VELOCITY) s = MAX_MOVEMENT_VELOCITY;

    if (a < -112) a = -112; //fhomolka 18/02/2021: I have no clue why it's 112 here, but 127 elsewhere
    else if (a > 112) a = 112;

    int i = 0;

    if (option[3] != 0)
    {
        // TODO		getmousevalues(&mousx, &mousy, &bstatus);
        if (BUTTON(gamefunc_Strafe))
        {
            i = s;
            i -= (mousx * mousxspeed);
        }
        else
        {
            i = a;
            i += (mousx * mousxspeed);
        }

        if (i < -MAX_STRAFE_VELOCITY - 1) i = -MAX_STRAFE_VELOCITY - 1; // original says -128
        if (i > MAX_STRAFE_VELOCITY) i = MAX_STRAFE_VELOCITY;

        if (BUTTON(gamefunc_Strafe))
            s = i;
        else
            a = i;

        // Les START - 07/24/95  - if key KEYLOOKING is held, mouse fwd/back looks up/down
        if (BUTTON(gamefunc_Mouse_Aiming)) // TODO - change all this to work correctly
//		if (keystatus[keys[KEYLOOKING]]) 
        {
            i = plr->horiz;
            i += (mousy >> 4);
            if (i < 100 - (YDIM >> 1)) i = 100 - (YDIM >> 1);
            else if (i > 100 + (YDIM >> 1)) i = 100 + (YDIM >> 1);
            plr->horiz = i;
        }
        //                         else mouse fwd/back moves fwd/back
        else
        {
            i = v;

            i -= (mousy * mousyspeed);
            if (i < -128) //fhomolka 18/02/2021: I have no clue why it's 128 here, but 201 elsewhere
            {
                i = -128;
            }
            else if (i > 127)
            {
                i = 127;
            }
            v = i;
        }

        #if 0 // TODO
        for (i = 0; i < 2; i++)
        {
            if (((bstatus & mbutbit[i]) == mbutbit[i])     // button is down
                && ((oldbstatus & mbutbit[i]) != mbutbit[i]))
            {
                keystatus[keys[mousekeys[i]]] = 1;
                if (mousekeys[i] == KEYRUN)
                {
                    keystatus[keys[KEYFWD]] = 1;
                }
            }
            else if (((bstatus & mbutbit[i]) == mbutbit[i])// button still down
                && ((oldbstatus & mbutbit[i]) == mbutbit[i]))
            {
                if (mousekeys[i] == KEYUSE              // ..one-time actions
                    || mousekeys[i] == KEYJUMP
                    || mousekeys[i] == KEYMAP
                    || mousekeys[i] == KEYUSEP
                    || mousekeys[i] == KEYCAST)
                {
                    keystatus[keys[mousekeys[i]]] = 0;
                }
            }
            else if (((bstatus & mbutbit[i]) != mbutbit[i])// button released
                && ((oldbstatus & mbutbit[i]) == mbutbit[i]))
            {
                keystatus[keys[mousekeys[i]]] = 0;
                if (mousekeys[i] == KEYRUN)
                {
                    keystatus[keys[KEYFWD]] = 0;
                }
            }
        }
        #endif

        oldbstatus = bstatus;
    }

 //   i = (int)totalclock - lockclock;
/*
    if (i > 255)
        i = 255;
*/
    //synctics = tics = i;

    int32_t tics = kTicksPerFrame;

    lockclock += kTicksPerFrame;//synctics;

    // TEMP
    synctics = 4;

    sprite[plr->spritenum].cstat ^= 1;

    vec3_t pos;
    pos.x = plr->x;
    pos.y = plr->y;
    pos.z = plr->z;

    getzrange(&pos, plr->sector, &hiz, &hihit, &loz, &lohit, 128L, CLIPMASK0);

    sprite[plr->spritenum].cstat ^= 1;

    if ((lohit & 0xc000) == 49152)
    {
        if ((sprite[lohit & 4095].z - plr->z) <= (PLAYERHEIGHT << 8))
            onsprite = (lohit & 4095);
    }
    else {
        onsprite = -1;
    }

    feetoffground = (sector[plr->sector].floorz - plr->z);

    if (BUTTON(gamefunc_Run) || v > 201)
    {
        if (v > 201)
            v = 201;

        v += v >> 1; // SUPER MARIO BROTHERS

        if (feetoffground > (32 << 8))
            tics += tics >> 1;
    }

    if (BUTTON(gamefunc_Look_Down))
    {
        if (plr->horiz > 100 - (YDIM >> 1))
        {
            plr->horiz -= (synctics << 1);
            autohoriz = 0;
        }
    }
    else if (BUTTON(gamefunc_Look_Up))
    {
        if (plr->horiz < 100 + (YDIM >> 1))
            plr->horiz += (synctics << 1);
        autohoriz = 0;
    }

    if (keystatus[sc_End] != 0)
        plr->orbactive[5] = -1;

    //     oldmousestatus=bstatus&0x01;

    //      if(keystatus[keys[KEYFIRE]] != 0 || keystatus[sc_RightControl] != 0 || (oldmousestatus) != 0 && hasshot == 0) {
    //      if (keystatus[keys[KEYFIRE]] != 0 || (oldmousestatus) != 0 && hasshot == 0) {

    if (BUTTON(gamefunc_Fire) && !plr->hasshot)
    {
        if (plr->currweaponfired == 0)
            plrfireweapon(plr);
    }

    // cast
    if (BUTTON(gamefunc_Cast_Spell) && !plr->orbshot && !plr->currweaponflip)
    {
        if (plr->orb[plr->currentorb] == 1 && plr->selectedgun == 0)
        {
            if (lvlspellcheck(plr))
            {
                plr->orbshot = true;
                activatedaorb(plr);
            }
        }
        if (plr->selectedgun != 0)
        {
            keystatus[WEAPONONE] = 1; // TODO FIXME
        }

        CONTROL_ClearButton(gamefunc_Cast_Spell);
    }

    if (BUTTON(gamefunc_Use_Potion))
    {
        CONTROL_ClearButton(gamefunc_Use_Potion);

        if (plr->potion[plr->currentpotion] > 0) {
            usapotion(plr);
        }
    }

    if (BUTTON(gamefunc_Open))
    {
        CONTROL_ClearButton(gamefunc_Open);

        if (netgame) {
            netdropflag();
        }
        else {
            plruse(plr);
        }
    }

    if ((sector[plr->sector].floorpicnum != LAVA
        || sector[plr->sector].floorpicnum != SLIME
        || sector[plr->sector].floorpicnum != WATER
        || sector[plr->sector].floorpicnum != HEALTHWATER
        || sector[plr->sector].floorpicnum != ANILAVA
        || sector[plr->sector].floorpicnum != LAVA1
        || sector[plr->sector].floorpicnum != LAVA2)
        && feetoffground <= (32 << 8))
    {
        v >>= 1;
    }

    if ((sector[plr->sector].floorpicnum == LAVA
        || sector[plr->sector].floorpicnum == SLIME
        || sector[plr->sector].floorpicnum == WATER
        || sector[plr->sector].floorpicnum == HEALTHWATER
        || sector[plr->sector].floorpicnum == ANILAVA
        || sector[plr->sector].floorpicnum == LAVA1
        || sector[plr->sector].floorpicnum == LAVA2)
        && plr->orbactive[5] < 0                                 //loz
        && plr->z >= sector[plr->sector].floorz - (plr->height << 8) - (8 << 8))
    {
        goalz = loz - (32 << 8);
        switch (sector[plr->sector].floorpicnum)
        {
            case ANILAVA:
            case LAVA:
            case LAVA1:
            case LAVA2:
            {
                if (plr->treasure[5] == 1)
                {
                    goalz = loz - (PLAYERHEIGHT << 8);
                    break;
                }
                else
                    v -= v >> 3;

                if (plr->invincibletime > 0 || plr->manatime > 0)
                    break;
                else
                {
                    if (lavasnd == -1)
                    {
                        lavasnd = SND_PlaySound(S_FIRELOOP1, 0, 0, 0, -1);
                    }
                    sethealth(-1);
                    startredflash(10);
                }
            }
            break;
            case WATER:
            {
                if (plr->treasure[4] == 1)
                {
                    goalz = loz - (PLAYERHEIGHT << 8);
                }
                else
                {
                    v -= v >> 3;
                }
            }
            break;
            case HEALTHWATER:
            if (plr->health < plr->maxhealth)
            {
                sethealth(1);
                startblueflash(5);
            }
            break;
        }
    }
    else if (plr->orbactive[5] > 0)
    {
        goalz = plr->z - (plr->height << 8);
        plr->hvel = 0;
    }
    else
        goalz = loz - (plr->height << 8);

    if (BUTTON(gamefunc_Jump))
    {
        if (plr->onsomething)
        {
            plr->hvel -= JUMPVEL;
            plr->onsomething = 0;
        }

        CONTROL_ClearButton(gamefunc_Jump);
    }

    if (BUTTON(gamefunc_Crouch))
    {
        if (goalz < ((sector[plr->sector].floorz) - (plr->height >> 3)))
        {
            goalz += (24 << 8);
        }
    }

    if (BUTTON(gamefunc_Map))
    {
        if (plr->dimension == 3)
        {
            plr->dimension = 2;

            StatusMessage(720, "map %d", mapon);
        }
        else
        {
            plr->dimension = 3;
            followmode = false;
        }

        CONTROL_ClearButton(gamefunc_Map);
    }

    // Map mode
    if (plr->dimension == 2)
    {
        if (BUTTON(gamefunc_Zoom_Out))
        {
            if (plr->zoom > 48)
                plr->zoom -= (plr->zoom >> 4);
        }
        if (BUTTON(gamefunc_Zoom_In))
        {
            if (plr->zoom < 4096)
                plr->zoom += (plr->zoom >> 4);
        }

        if (keystatus[sc_F] > 0)
        {
            if (!followmode)
            {
                StatusMessage(360, "Map %d", mapon);
                followmode = true;
                followx = 0;
                followy = 0;
            }
            else
            {
                followmode = false;
            }

            keystatus[sc_F] = 0;
        }
    }

    else if (plr->dimension == 3 && svga == 0)
    {
        if (BUTTON(gamefunc_Shrink_Screen) && plr->screensize > 64)
        {
            if (plr->screensize <= 320)
            {
                //updatepics();
            }
            plr->screensize -= 8;
            dax = (XDIM >> 1) - (plr->screensize >> 1);
            dax2 = dax + plr->screensize - 1;
            day = (STATUSSCREEN >> 1) - (((plr->screensize * STATUSSCREEN) / XDIM) >> 1);
            day2 = day + ((plr->screensize * STATUSSCREEN) / XDIM) - 1;

            videoSetViewableArea(dax, day, dax2, day2);

            odax = (YDIM >> 1) - ((plr->screensize + 8) >> 1);
            odax2 = dax + (plr->screensize + 8) - 1;
            oday = (STATUSSCREEN >> 1) - ((((plr->screensize + 8) * STATUSSCREEN) / XDIM) >> 1);
            oday2 = day + (((plr->screensize + 8) * STATUSSCREEN) / XDIM) - 1;
            permanentwritesprite(0, 0, BACKGROUND, 0, odax, oday, dax - 1, oday2, 0);
            permanentwritesprite(0, 0, BACKGROUND, 0, dax2 + 1, oday, odax2, oday2, 0);
            permanentwritesprite(0, 0, BACKGROUND, 0, dax, oday, dax2, day - 1, 0);
            permanentwritesprite(0, 0, BACKGROUND, 0, dax, day2 + 1, dax2, oday2, 0);

            if (plr->screensize == XDIM)
            {
                permanentwritesprite(0, 200 - 46, NEWSTATUSBAR, 0, 0, 0, XDIM - 1, YDIM - 1, 0);
                //updatepics();
            }
        }

        if (BUTTON(gamefunc_Enlarge_Screen) && plr->screensize <= XDIM)
        {
            plr->screensize += 8;
            if (plr->screensize > XDIM)
            {
                dax = day = 0;
                dax2 = XDIM - 1;
                day2 = YDIM - 1;
            }
            else
            {
                dax = (XDIM >> 1) - (plr->screensize >> 1);
                dax2 = dax + plr->screensize - 1;
                day = (STATUSSCREEN >> 1) - (((plr->screensize * STATUSSCREEN) / XDIM) >> 1);
                day2 = day + ((plr->screensize * STATUSSCREEN) / XDIM) - 1;
            }

            videoSetViewableArea(dax, day, dax2, day2);
        }
    }

    if (plr->dimension == 3 && svga == 1)
    {
        if (BUTTON(gamefunc_Shrink_Screen))
        {
            plr->screensize = 320;
            videoSetViewableArea(0, 0, 640 - 1, 372 - 1);
            overwritesprite(0, 372, SSTATUSBAR, 0, 0, 0);
            //updatepics();
        }

        if (BUTTON(gamefunc_Enlarge_Screen))
        {
            plr->screensize = 328;
            videoSetViewableArea(0, 0, 640 - 1, 480 - 1);
        }
    }

    onground = plr->onsomething;

    if (BUTTON(gamefunc_Fly_Up)) {
        dophysics(plr, goalz, 1, v);
    }
    else if (BUTTON(gamefunc_Fly_Down)) {
        dophysics(plr, goalz, -1, v);
    }
    else {
        dophysics(plr, goalz, 0, v);
    }

    if (!onground && plr->onsomething)
    {
        if (plr->fallz > 32768)
        {
            if (rand() % 2)
                playsound_loc(S_PLRPAIN1 + (rand() % 2), plr->x, plr->y);
            else
                playsound_loc(S_PUSH1 + (rand() % 2), plr->x, plr->y);

            sethealth(-(plr->fallz >> 13));
            plr->fallz = 0;
        }
        else if (plr->fallz > 8192)
        {
            playsound_loc(S_BREATH1 + (rand() % 2), plr->x, plr->y);
        }
    }

    if (ihaveflag > 0)
        v -= v >> 2;

    if (v != 0 || s != 0)
    {
        xvect = yvect = 0;
        if (v != 0)
        {
            xvect = (v * tics * Cos(plr->ang + 2048)) >> 3;
            yvect = (v * tics * Sin(plr->ang + 2048)) >> 3;
        }
        if (s != 0)
        {
            xvect += (s * tics * Cos(plr->ang + 1536)) >> 3;
            yvect += (s * tics * Sin(plr->ang + 1536)) >> 3;
        }

        oldposx = plr->x; oldposy = plr->y;

        clipmove_old(&plr->x, &plr->y, &plr->z, &plr->sector, xvect, yvect, 128, 4 << 8, 4 << 8, CLIPMASK0);

        if (plr->sector != tempsectornum)
        {
            if (lavasnd != -1)
            {
                switch (sector[plr->sector].floorpicnum)
                {
                    case ANILAVA:
                    case LAVA:
                    case LAVA1:
                    case LAVA2:
                    break;
                    default:
                    SND_StopLoop(lavasnd);
                    lavasnd = -1;
                    break;
                }
            }
            sectorsounds();
        }

        tempsectornum = plr->sector;

        // walking on sprite
        plr->horiz -= oldhoriz;

        dist = ksqrt((plr->x - oldposx) * (plr->x - oldposx) + (plr->y - oldposy) * (plr->y - oldposy));

        if (BUTTON(gamefunc_Run))
        {
            dist >>= 2;
        }

        if (dist > 0 && feetoffground <= (plr->height << 8) || onsprite != -1)
        {
            if (svga == 1)
                oldhoriz = (dist * Sin((int)totalclock << 5) >> 19) >> 2;
            else
                oldhoriz = (dist * Sin((int)totalclock << 5) >> 19) >> 1;

            plr->horiz += oldhoriz;
        }
        else
            oldhoriz = 0;

        if (plr->horiz > 200) plr->horiz = 200;
        if (plr->horiz < 0) plr->horiz = 0;

        if (onsprite != -1 && dist > 50 && lopoint && !justplayed)
        {
            switch (sprite[onsprite].picnum)
            {
                case WALLARROW:
                case OPENCHEST:
                case GIFTBOX:
                if (walktoggle)
                    playsound_loc(S_WOOD1, (plr->x + 3000), plr->y);
                else
                    playsound_loc(S_WOOD1, plr->x, (plr->y + 3000));
                walktoggle = !walktoggle;
                justplayed = true;
                break;
                case WOODPLANK:
                if (walktoggle)
                    playsound_loc(S_SOFTCHAINWALK, (plr->x + 3000), plr->y);
                else
                    playsound_loc(S_SOFTCHAINWALK, plr->x, (plr->y + 3000));
                walktoggle = !walktoggle;
                justplayed = true;

                break;
                case SQUAREGRATE:
                case SQUAREGRATE + 1:
                if (walktoggle)
                    playsound_loc(S_LOUDCHAINWALK, (plr->x + 3000), plr->y);
                else
                    playsound_loc(S_LOUDCHAINWALK, plr->x, (plr->y + 3000));
                walktoggle = !walktoggle;
                justplayed = true;
                break;
                case SPACEPLANK:
                if (walktoggle)
                    playsound_loc(S_SOFTCREAKWALK, (plr->x + 3000), plr->y);
                else
                    playsound_loc(S_SOFTCREAKWALK, plr->x, (plr->y + 3000));
                walktoggle = !walktoggle;
                justplayed = true;
                break;
                case RAT:
                playsound_loc(S_RATS1 + (rand() % 2), sprite[i].x, sprite[i].y);
                justplayed = true;
                deletesprite(i);
                break;
                case SPIDER:
                //STOMP
                playsound_loc(S_DEADSTEP, sprite[i].x, sprite[i].y);
                justplayed = true;
                newstatus(i, DIE);
                break;

                case FREDDEAD:
                case 1980:
                case 1981:
                case 1984:
                case 1979:
                case 1957:
                case 1955:
                case 1953:
                case 1952:
                case 1941:
                case 1940:
                playsound_loc(S_DEADSTEP, plr->x, plr->y);
                justplayed = true;
                break;

                default:
                break;
            }
        }

        if (!lopoint && oldhoriz == -2 && !justplayed)
            lopoint = true;

        if (lopoint && oldhoriz != -2 && justplayed)
        {
            lopoint = false;
            justplayed = false;
        }

        if (vel > 199 || vel < -199 && dist > 10)
            runningtime += kTimerTicks; // synctics;
        else
            runningtime -= kTimerTicks; //synctics;

        if (runningtime < -360)
            runningtime = 0;

        if (runningtime > 360)
        {
            SND_PlaySound(S_PLRPAIN1, 0, 0, 0, 0);
            runningtime = 0;
        }
    }
    if (a != 0)
    {
        plr->ang += ((a * /*synctics*/kTimerTicks) >> 4);
        plr->ang = (plr->ang + 2048) & kAngleMask;
    }

    //vec3_t pos;
    pos.x = plr->x;
    pos.y = plr->y;
    pos.z = plr->z + (plr->height << 8);

    setsprite(plr->spritenum, &pos);
    sprite[plr->spritenum].ang = plr->ang;

    if (sector[plr->sector].ceilingz > sector[plr->sector].floorz - (8 << 8))
        sethealth(-10);

    if (plr->health <= 0)
    {
        SND_CheckLoops();
        playerdead(plr);
    }

    if (BUTTON(gamefunc_Look_Straight))
    {
        autohoriz = 1;
        CONTROL_ClearButton(gamefunc_Look_Straight);
    }

    if (autohoriz == 1)
        autothehoriz(plr);

    singleshot(bstatus);

    weaponchange();
}

void autothehoriz(Player* plr)
{
    if (plr->horiz < 100)
        plr->horiz += synctics << 1;
    if (plr->horiz > 100)
        plr->horiz -= synctics << 1;
    if (plr->horiz >= 90 && plr->horiz <= 110)
        autohoriz = 0;
}

#if 0
int gimmer=0;
int gimmes=0;
int gimmev=0;
int gimmep=0;

void cheatkeys(Player* plr)
{

    int i;

    /*
        the cheat code is R S V P
    */

    if (keystatus[sc_R] > 0)
    {
        gimmer = 1;
        //          keystatus[sc_R]=0;
    }
    if (gimmer == 1 && keystatus[sc_S] > 0)
    {
        gimmes = 1;
        //          keystatus[sc_S]=0;
    }
    if (gimmes == 1 && keystatus[sc_V] > 0)
    {
        gimmev = 1;
        //          keystatus[sc_V]=0;
    }
    if (gimmev == 1 && keystatus[sc_P] > 0)
    {
        gimmep = 1;
        //          keystatus[sc_P]=0;
    }
    if (gimmep == 1)
    {
        gimmer = gimmes = gimmev = gimmep = 0;

        //        spiked=1;

        for (i = 0; i < MAXPOTIONS; i++)
        {
            plr->potion[i] = 9;
        }

        plr->weapon[1] = 1; plr->ammo[1] = 45; //DAGGER
        plr->weapon[2] = 1; plr->ammo[2] = 55; //MORNINGSTAR
        plr->weapon[3] = 1; plr->ammo[3] = 50; //SHORT SWORD
        plr->weapon[4] = 1; plr->ammo[4] = 80; //BROAD SWORD
        plr->weapon[5] = 1; plr->ammo[5] = 100; //BATTLE AXE
        plr->weapon[6] = 1; plr->ammo[6] = 50; // BOW
        plr->weapon[7] = 2; plr->ammo[7] = 40; //PIKE
        plr->weapon[8] = 1; plr->ammo[8] = 250; //TWO HANDED
        plr->weapon[9] = 1; plr->ammo[9] = 50;

        for (i = 0; i < 8; i++)
        {
            plr->orb[i] = 1;
            plr->orbammo[i] = 9;
        }

        currweapon = selectedgun = 4;
        plr->health = 0;
        sethealth(200);
        plr->armor = 150;
        plr->armortype = 3;
        plr->lvl = 7;
        plr->maxhealth = 200;
        plr->treasure[14] = 1;
        plr->treasure[15] = 1;
        plr->treasure[16] = 1;
        plr->treasure[17] = 1;
        updatepics();
    }

    //if(keystatus[sc_D] > 0 && keystatus[sc_O] > 0 && keystatus[sc_N] > 0)
    //if( keystatus[sc_F12] > 0 )
    //    invincibletime=65000;

    return;
    // OHM
    if (keystatus[sc_O] > 0 && keystatus[sc_H] > 0 && keystatus[sc_M] > 0)
        for (i = 0; i < MAXWEAPONS; i++)
        {
            plr->weapon[i] = 1;
            plr->ammo[i] = 999;
        }

    // BINGO
    if (keystatus[sc_B] > 0 && keystatus[sc_I] > 0 && keystatus[sc_N] > 0 && keystatus[sc_G] > 0 && keystatus[sc_O] > 0)
    {
        plr->health = 1;
        sethealth(99);
    }
}
#endif


extern char fancy[];

void nettypeletter()
{
    strcpy(nettemp, "");

    if (typemessageleng <= 40)
    {
        for (int i = 0; i < 128; i++)
        {
            if (keystatus[i] > 0)
            {
                nettemp[typemessageleng] = scantoasc[i];
                typemessageleng++;
                nettemp[typemessageleng] = '\0';
                keystatus[i] = 0;
            }
        }
        printext256(0, 0, 31, -1, Bstrupr(nettemp), 1);
    }
}

char typingbuffer[40];

void typeletter()
{
    int exit = 0;
    //	char temp[20];

    keystatus[sc_Return] = keystatus[sc_kpad_Enter] = 0;

    for (int i = 0; i < 128; i++)
        keystatus[i] = 0;

    typingbuffer[0] = '\0';
    typemessageleng = 0;
    //strcpy(temp, "");

    while (!exit)
    {
        handleevents();

        if (keystatus[sc_BackSpace])
        {
            if (typemessageleng)
            {
                typemessageleng--;
                typingbuffer[typemessageleng] = '\0';
            }
            keystatus[sc_BackSpace] = 0;
        }

        if (typemessageleng <= 10)
        {
            for (int i = 0; i < 128; i++)
            {
                if (keystatus[i] > 0
                    && keystatus[sc_BackSpace] == 0
                    && keystatus[sc_Escape] == 0
                    && keystatus[sc_Return] == 0
                    && keystatus[sc_kpad_Enter] == 0)
                {
                    for (int j = 0; j < 41; j++)
                    {
                        if (scantoasc[i] == ' ')
                            continue;

                        else if (scantoasc[i] == fancy[j])
                        {
                            typingbuffer[typemessageleng] = fancy[j];
                            typemessageleng++;
                            typingbuffer[typemessageleng] = '\0';
                            keystatus[i] = 0;
                        }
                        else
                        {
                            keystatus[i] = 0;
                        }
                    }
                }
            }
        }

        if (keystatus[sc_Escape] > 0)
        {
            exit = 1;
            keystatus[sc_Escape] = 0;
        }

        if (keystatus[sc_Return] > 0 || keystatus[sc_kpad_Enter] > 0)
        {
            exit = 2;
            keystatus[sc_Return] = keystatus[sc_kpad_Enter] = 0;
        }

        fancyfontscreen(18, 24, THEFONT, typingbuffer);
        videoNextPage();
    }

    if (exit == 2)
    {
        keystatus[sc_Return] = keystatus[sc_kpad_Enter] = 0;
    }

    checkcheat();
    lockclock = (int)totalclock;
    typemode = 0;
    typemessageleng = 0;
}

void checkcheat()
{
    Player* plr = &player[pyrn];

    Bstrupr(typingbuffer);

    if (strcmp(typingbuffer, "RSVP") == 0)
    {
        // TODO - this behaves differently in the demo (max health, ammo, magic and 9 of each potions etc)
        sethealth(-plr->health);
        plr->horiz = 200;
        //updatepics();
    }
    else if (strcmp(typingbuffer, "RAMBO") == 0)
    {
        sethealth(-plr->health);
        plr->horiz = 200;
        //updatepics();
    }
    else if (strcmp(typingbuffer, "IDKFA") == 0)
    {
        sethealth(-plr->health);
        plr->horiz = 200;
        //updatepics();
    }
    else if (strcmp(typingbuffer, "SCOOTER") == 0)
    {
        plr->weapon[1] = 1; plr->ammo[1] = 45;  // DAGGER
        plr->weapon[2] = 1; plr->ammo[2] = 55;  // MORNINGSTAR
        plr->weapon[3] = 1; plr->ammo[3] = 50;  // SHORT SWORD
        plr->weapon[4] = 1; plr->ammo[4] = 80;  // BROAD SWORD
        plr->weapon[5] = 1; plr->ammo[5] = 100; // BATTLE AXE
        plr->weapon[6] = 1; plr->ammo[6] = 50;  // BOW
        plr->weapon[7] = 2; plr->ammo[7] = 40;  // PIKE
        plr->weapon[8] = 1; plr->ammo[8] = 250; // TWO HANDED
        plr->weapon[9] = 1; plr->ammo[9] = 50;
        plr->currweapon = plr->selectedgun = 4;
        //updatepics();
    }
    else if (strcmp(typingbuffer, "MOMMY") == 0)
    {
        for (int i = 0; i < MAXPOTIONS; i++)
        {
            plr->potion[i] = 9;
        }

        //updatepics();
    }
    else if (strcmp(typingbuffer, "WANGO") == 0)
    {
        for (int i = 0; i < 8; i++)
        {
            plr->orb[i] = 1;
            plr->orbammo[i] = 9;
        }

        plr->health = 0;
        sethealth(200);
        plr->armor = 150;
        plr->armortype = 3;
        plr->lvl = 7;
        plr->maxhealth = 200;
        plr->treasure[14] = 1;
        plr->treasure[15] = 1;
        plr->treasure[16] = 1;
        plr->treasure[17] = 1;
        //updatepics();
    }
    else if (strcmp(typingbuffer, "DARKNESS") == 0)
    {
        // TODO - god mode
    }
    else if (strcmp(typingbuffer, "GOTHMOG") == 0)
    {
        // TODO - teleport
    }
    else if (strcmp(typingbuffer, "SPINACH") == 0)
    {
        plr->health = 0;
        sethealth(200);
    }

    strcpy(typingbuffer, "");
}

void typecheat(char ch)
{
    char tempbuf[40];
    charsperline = 40;

    for (int i = 0; i <= typemessageleng; i += charsperline)
    {
        for (int j = 0; j < charsperline; j++)
        {
            tempbuf[j] = typemessage[i + j];
        }

        if (typemessageleng < i + charsperline)
        {
            tempbuf[(typemessageleng - i)] = '-';
            tempbuf[(typemessageleng - i) + 1] = 0;
        }
        else
        {
            tempbuf[charsperline - 1] = 0;
        }
    }

    StatusMessage(360, tempbuf);
}

void dosoundthing()
{
    musiclevel = (wMIDIVol >> 3);
    digilevel = (wDIGIVol >> 11);

    if (BUTTON(gamefunc_Move_Forward))
        soundcontrol++;

    if (BUTTON(gamefunc_Move_Backward))
        soundcontrol--;

    if (soundcontrol > 1)
        soundcontrol = 1;
    if (soundcontrol < 0)
        soundcontrol = 0;

    switch (soundcontrol)
    {
        case 0:
        if (BUTTON(gamefunc_Turn_Left))
        {
            musiclevel--;
            if (musiclevel < 0)
            {
                musiclevel = 0;

                StatusMessage(10, "Music %d", musiclevel);
            }
            else
            {
                StatusMessage(10, "Music %d", musiclevel);
                SND_Mixer(1, musiclevel);
            }
            break;
        }
        else if (BUTTON(gamefunc_Turn_Right))
        {
            musiclevel++;
            if (musiclevel > 16)
            {
                musiclevel = 16;
                StatusMessage(10, "Music %d", musiclevel);
            }
            else
            {
                StatusMessage(10, "Music %d", musiclevel);
                SND_Mixer(1, musiclevel);
            }
            break;
        }

        else
        {
            StatusMessage(10, "Music %d", musiclevel);
        }
        break;

        case 1:
        if (BUTTON(gamefunc_Turn_Left))
        {
            digilevel--;
            if (digilevel < 0)
            {
                digilevel = 0;
                StatusMessage(10, "Sounds %d", mousyspeed);
            }
            else
            {
                StatusMessage(10, "Sounds %d", mousyspeed);
                SND_Mixer(0, digilevel);
                SND_Sound(S_LOUDCHAINWALK);
            }
            break;
        }
        else if (BUTTON(gamefunc_Turn_Right))
        {
            digilevel++;
            if (digilevel > 16)
            {
                digilevel = 16;
                StatusMessage(10, "Sounds %d", mousyspeed);
            }
            else
            {
                StatusMessage(10, "Sounds %d", mousyspeed);
                SND_Mixer(0, digilevel);
                SND_Sound(S_LOUDCHAINWALK);
            }
            break;
        }

        else
        {
            StatusMessage(10, "Sounds %d", mousyspeed);
        }
        break;
    }
}
