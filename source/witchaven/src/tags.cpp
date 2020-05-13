/***************************************************************************
 *   WHTAG.C  - tag'd sector code for Witchaven                            *
 *                                                                         *
 *                                                     12/15/94 Les Bird   *
 ***************************************************************************/

#include "objects.h"
#include "sound.h"
#include "witchaven.h"
#include "input.h"
#include "tags.h"
#include "view.h"

#define MAXDOORKEYS   4

struct swingdoor_t swingdoor[MAXSWINGDOORS];

extern short crushsectorlist[32], crushsectorcnt;
extern short crushsectoranim[32], crushsectordone[32];

extern short dragsectorlist[16], dragxdir[16], dragydir[16], dragsectorcnt;
extern int dragx1[16], dragy1[16], dragx2[16], dragy2[16], dragfloorz[16];

// WITCHAVEN 2 variables
int32_t speed;
int d_soundplayed = 0;

void operatesprite(int s)
{
    Player* plr = &player[pyrn];

    if (sprite[s].picnum == SPAWNFIREBALL)
        newstatus(s, DEVILFIRE);

    if (sprite[s].picnum == SPAWNJAVLIN)
        trowajavlin(s);

    if (WH2)
    {
        #if 0
        switch (sprite[s].picnum)
        {
            case STONEGONZOCHM:
            case STONEGONZOGSH:
            case STONEGRONDOVAL:
            case STONEGONZOBSW:
            //
            //
            //
            //
            //
            //
            sprite[s].lotag *= 120;
            changespritestat((short)s, STONETOFLESH);
            break;
            case GONZOHMJUMP:
            case GONZOSHJUMP:
            newstatus((short)s, AMBUSH);
            break;
            case STAINGLASS1:
            case STAINGLASS2:
            case STAINGLASS3:
            case STAINSKULL:
            // case STAINGARGO:
            // case STAINMU:
            case STAINHEAD:
            case STAINSNAKE:
            // case STAINFLOWER:
            case STAINCIRCLE:
            // case STAINTRIANGLE:
            // case STAINTHIN:
            case STAINQ:
            case STAINSCENE:
            switch (sprite[s].lotag)
            {
                case 2:
                playsound_loc(S_GLASSBREAK1 + (rand() % 3), sprite[s].x, sprite[s].y);
                for (j = 0; j < 20; j++)
                {
                    shards(s, 2);
                }
                deletesprite(s);
                break;
            }
            break;
        }
        #endif
    }

    if ((sprite[s].lotag == 1800 || sprite[s].lotag == 1810 || sprite[s].lotag == 1820)
        && sprite[s].sectnum == plr->sector)
    {
        for (int j = 0; j < MAXSPRITES; j++)
        {
            if (sprite[s].sectnum == sprite[j].sectnum
                && (sprite[j].lotag >= 1800 && sprite[j].lotag <= 1899))
                newstatus(j, LIFTDN);
        }
    }
    if ((sprite[s].lotag == 1801 || sprite[s].lotag == 1811 || sprite[s].lotag == 1821)
        && sprite[s].sectnum == plr->sector)
    {
        for (int j = 0; j < MAXSPRITES; j++)
        {
            if (sprite[s].sectnum == sprite[j].sectnum
                && (sprite[j].lotag >= 1800 && sprite[j].lotag <= 1899))
            {
                newstatus(j, LIFTUP);
            }
        }
    }
}

void operatesector(int s)
{
    short endwall, startwall, wallfind[2];
    int  botz, dax2, day2, datag, goalz, i, j, size, topz;
    int32_t centx, centy;
    int32_t daz;
    int doorantic, doorkey, doortype, checkforkey;
    int  temp1, temp2, temp3;
    short daceiling = 0;
    int32_t k;
    int keysok = 0;
    int32_t speed;

    Player* plr = &player[pyrn];

    datag = sector[s].lotag;

    startwall = sector[s].wallptr;
    endwall = startwall + sector[s].wallnum - 1;
    centx = 0, centy = 0;
    for (i = startwall; i <= endwall; i++)
    {
        centx += wall[i].x;
        centy += wall[i].y;
    }
    centx /= (endwall - startwall + 1);
    centy /= (endwall - startwall + 1);

    // WITCHAVEN 2
    if (WH2)
    {
        switch (datag)
        {
            case 131:
            case 61:
            case 71:
            // check for proper key
            if (plr->treasure[14] == 0)
            {
                keysok = 0;
                StatusMessage(360, "BRASS KEY NEEDED");
            }
            else
                keysok = 1;
            break;
            case 132:
            case 62:
            // check for proper key
            if (plr->treasure[15] == 0)
            {
                keysok = 0;
                StatusMessage(360, "Black KEY NEEDED");
            }
            else
                keysok = 1;
            break;
            case 133:
            case 63:
            case 73:
            // check for proper key
            if (plr->treasure[16] == 0)
            {
                keysok = 0;
                StatusMessage(360, "glass KEY NEEDED");
            }
            else
                keysok = 1;
            break;
            case 134:
            case 64:
            if (plr->treasure[17] == 0)
            {
                keysok = 0;
                StatusMessage(360, "ivory KEY NEEDED");
            }
            else
                keysok = 1;
            break;
            case 72:
            // check for proper key
            if (plr->treasure[15] == 0)
            {
                keysok = 0;
                StatusMessage(360, "black KEY NEEDED");
            }
            else
                keysok = 1;
            break;
            case 74:
            if (plr->treasure[17] == 0)
            {
                keysok = 0;
                StatusMessage(360, "Ivory KEY NEEDED");
            }
            else
                keysok = 1;
            break;
            #if 0 // unreachable - this is original code
            i = getanimationgoal(&sector[s].floorz);
            if (i >= 0)
            {
                goalz = sector[nextsectorneighborz(s, sector[s].ceilingz, 1, 1)].floorz;
                if (animategoal[i] == goalz)
                {
                    animategoal[i] = sector[s].ceilingz;
                }
                else
                {
                    animategoal[i] = goalz;
                }
            }
            else
            {
                if (sector[s].ceilingz == sector[s].floorz)
                {
                    goalz = sector[nextsectorneighborz(s, sector[s].ceilingz, 1, 1)].floorz;
                }
                else
                {
                    goalz = sector[s].ceilingz;
                }
                setanimation(&sector[s].floorz, goalz, DOORSPEED);
            }
            //JSA_DEMO
            SND_PlaySound(S_DOOR1, 0, 0, 0, 0);
            break;
            #endif
        }
    }

    switch (datag)
    {
        case DOORBOX:

        wallfind[0] = -1;
        wallfind[1] = -1;
        for (i = startwall; i <= endwall; i++)
            if (wall[i].lotag == 6)
            {
                if (wallfind[0] == -1)
                    wallfind[0] = i;
                else
                    wallfind[1] = i;
            }

        for (j = 0; j < 2; j++)
        {
            if ((((wall[wallfind[j]].x + wall[wall[wallfind[j]].point2].x) >> 1) == centx) && (((wall[wallfind[j]].y + wall[wall[wallfind[j]].point2].y) >> 1) == centy))
            {
                i = wallfind[j] - 1; if (i < startwall) i = endwall;
                dax2 = wall[i].x - wall[wallfind[j]].x;
                day2 = wall[i].y - wall[wallfind[j]].y;
                if (dax2 != 0)
                {
                    dax2 = wall[wall[wall[wall[wallfind[j]].point2].point2].point2].x;
                    dax2 -= wall[wall[wall[wallfind[j]].point2].point2].x;
                    setanimation(&wall[wallfind[j]].x, wall[wallfind[j]].x + dax2, 4);
                    setanimation(&wall[i].x, wall[i].x + dax2, 4);
                    setanimation(&wall[wall[wallfind[j]].point2].x, wall[wall[wallfind[j]].point2].x + dax2, 4);
                    setanimation(&wall[wall[wall[wallfind[j]].point2].point2].x, wall[wall[wall[wallfind[j]].point2].point2].x + dax2, 4);
                }
                else if (day2 != 0)
                {
                    day2 = wall[wall[wall[wall[wallfind[j]].point2].point2].point2].y;
                    day2 -= wall[wall[wall[wallfind[j]].point2].point2].y;
                    setanimation(&wall[wallfind[j]].y, wall[wallfind[j]].y + day2, 4);
                    setanimation(&wall[i].y, wall[i].y + day2, 4);
                    setanimation(&wall[wall[wallfind[j]].point2].y, wall[wall[wallfind[j]].point2].y + day2, 4);
                    setanimation(&wall[wall[wall[wallfind[j]].point2].point2].y, wall[wall[wall[wallfind[j]].point2].point2].y + day2, 4);
                }
            }
            else
            {
                i = wallfind[j] - 1; if (i < startwall) i = endwall;
                dax2 = wall[i].x - wall[wallfind[j]].x;
                day2 = wall[i].y - wall[wallfind[j]].y;
                if (dax2 != 0)
                {
                    setanimation(&wall[wallfind[j]].x, centx, 4);
                    setanimation(&wall[i].x, centx + dax2, 4);
                    setanimation(&wall[wall[wallfind[j]].point2].x, centx, 4);
                    setanimation(&wall[wall[wall[wallfind[j]].point2].point2].x, centx + dax2, 4);
                }
                else if (day2 != 0)
                {
                    setanimation(&wall[wallfind[j]].y, centy, 4);
                    setanimation(&wall[i].y, centy + day2, 4);
                    setanimation(&wall[wall[wallfind[j]].point2].y, centy, 4);
                    setanimation(&wall[wall[wall[wallfind[j]].point2].point2].y, centy + day2, 4);
                }
            }
        }

        //playsound_loc(S_DOOR2,centx,centy);
        SND_PlaySound(S_DOOR2, 0, 0, 0, 0);

        break;
        case DOORUPTAG:          // a door that opens up
        i = getanimationgoal(&sector[s].ceilingz);
        if (i >= 0)
        {
            goalz = sector[nextsectorneighborz(s, sector[s].floorz, -1, -1)].ceilingz;
            if (animategoal[i] == goalz)
            {
                animategoal[i] = sector[s].floorz;
                //setdelayfunc(operatesector,s,0);
            }
            else
            {
                animategoal[i] = goalz;
                //setdelayfunc(operatesector,s,DOORDELAY);
            }
        }
        else
        {
            if (sector[s].ceilingz == sector[s].floorz)
            {
                goalz = sector[nextsectorneighborz(s, sector[s].floorz, -1, -1)].ceilingz;
                //setdelayfunc(operatesector,s,DOORDELAY);
            }
            else
            {
                goalz = sector[s].floorz;
                //setdelayfunc(operatesector,s,0);
            }
            setanimation(&sector[s].ceilingz, goalz, DOORSPEED);
        }
        SND_PlaySound(S_DOOR2, 0, 0, 0, 0);
        break;

        case DOORDOWNTAG:        // a door that opens down
        i = getanimationgoal(&sector[s].floorz);
        if (i >= 0)
        {
            goalz = sector[nextsectorneighborz(s, sector[s].ceilingz, 1, 1)].floorz;
            if (animategoal[i] == goalz)
            {
                animategoal[i] = sector[s].ceilingz;
                //setdelayfunc(operatesector,s,0);
            }
            else
            {
                animategoal[i] = goalz;
                //setdelayfunc(operatesector,s,DOORDELAY);
            }
        }
        else
        {
            if (sector[s].ceilingz == sector[s].floorz)
            {
                goalz = sector[nextsectorneighborz(s, sector[s].ceilingz, 1, 1)].floorz;
                //setdelayfunc(operatesector,s,DOORDELAY);
            }
            else
            {
                goalz = sector[s].ceilingz;
                //setdelayfunc(operatesector,s,0);
            }
            setanimation(&sector[s].floorz, goalz, DOORSPEED);
        }
        //JSA_DEMO
                  //playsound_loc(S_DOOR1,centx,centy);
        SND_PlaySound(S_DOOR1, 0, 0, 0, 0);
        break;

        case PLATFORMELEVTAG:
        i = getanimationgoal(&sector[s].floorz);
        goalz = sector[plr->sector].floorz;
        if (i >= 0)
        {
            animategoal[i] = goalz;
        }
        else
        {
            setanimation(&sector[s].floorz, goalz, ELEVSPEED);
        }
        break;
        case BOXELEVTAG:
        i = getanimationgoal(&sector[s].floorz);
        j = getanimationgoal(&sector[s].ceilingz);
        size = sector[s].ceilingz - sector[s].floorz;
        goalz = sector[plr->sector].floorz;
        if (i >= 0)
        {
            animategoal[i] = goalz;
        }
        else
        {
            setanimation(&sector[s].floorz, goalz, ELEVSPEED);
        }
        goalz = goalz + size;
        if (j >= 0)
        {
            animategoal[j] = goalz;
        }
        else
        {
            setanimation(&sector[s].ceilingz, goalz, ELEVSPEED);
        }
        break;
        case DOORSPLITHOR:
        i = getanimationgoal(&sector[s].floorz);
        j = getanimationgoal(&sector[s].ceilingz);
        if (i >= 0)
        {
            botz = sector[nextsectorneighborz(s, sector[s].ceilingz, 1, 1)].floorz;
            if (animategoal[i] == botz)
            {
                animategoal[i] = (sector[s].ceilingz + sector[s].floorz) >> 1;
                //setdelayfunc(operatesector,s,0);
            }
            else
            {
                animategoal[i] = botz;
                //setdelayfunc(operatesector,s,DOORDELAY);
            }
        }
        else
        {
            if (sector[s].ceilingz == sector[s].floorz)
            {
                botz = sector[nextsectorneighborz(s, sector[s].ceilingz, 1, 1)].floorz;
                //setdelayfunc(operatesector,s,DOORDELAY);
            }
            else
            {
                botz = (sector[s].ceilingz + sector[s].floorz) >> 1;
                //setdelayfunc(operatesector,s,0);
            }
            setanimation(&sector[s].floorz, botz, ELEVSPEED);
        }
        if (j >= 0)
        {
            topz = sector[nextsectorneighborz(s, sector[s].floorz, -1, -1)].ceilingz;
            if (animategoal[j] == topz)
            {
                animategoal[j] = (sector[s].ceilingz + sector[s].floorz) >> 1;
            }
            else
            {
                animategoal[j] = topz;
            }
        }
        else
        {
            if (sector[s].ceilingz == sector[s].floorz)
            {
                topz = sector[nextsectorneighborz(s, sector[s].floorz, -1, -1)].ceilingz;
            }
            else
            {
                topz = (sector[s].ceilingz + sector[s].floorz) >> 1;
            }
            setanimation(&sector[s].ceilingz, topz, ELEVSPEED);
        }
        //playsound_loc(S_DOOR1+(rand()%3),centx,centy);
        SND_PlaySound(S_DOOR1 + (rand() % 3), 0, 0, 0, 0);
        break;
        case DOORSPLITVER:

        wallfind[0] = -1;
        wallfind[1] = -1;
        for (i = startwall; i <= endwall; i++)
            if ((wall[i].x == centx) || (wall[i].y == centy))
            {
                if (wallfind[0] == -1)
                    wallfind[0] = i;
                else
                    wallfind[1] = i;
            }

        for (j = 0; j < 2; j++)
        {
            if ((wall[wallfind[j]].x == centx) && (wall[wallfind[j]].y == centy))
            {
                i = wallfind[j] - 1; if (i < startwall) i = endwall;
                dax2 = ((wall[i].x + wall[wall[wallfind[j]].point2].x) >> 1) - wall[wallfind[j]].x;
                day2 = ((wall[i].y + wall[wall[wallfind[j]].point2].y) >> 1) - wall[wallfind[j]].y;
                if (dax2 != 0)
                {
                    dax2 = wall[wall[wall[wallfind[j]].point2].point2].x;
                    dax2 -= wall[wall[wallfind[j]].point2].x;
                    setanimation(&wall[wallfind[j]].x, wall[wallfind[j]].x + dax2, 4);
                    setanimation(&wall[i].x, wall[i].x + dax2, 4);
                    setanimation(&wall[wall[wallfind[j]].point2].x, wall[wall[wallfind[j]].point2].x + dax2, 4);
                }
                else if (day2 != 0)
                {
                    day2 = wall[wall[wall[wallfind[j]].point2].point2].y;
                    day2 -= wall[wall[wallfind[j]].point2].y;
                    setanimation(&wall[wallfind[j]].y, wall[wallfind[j]].y + day2, 4);
                    setanimation(&wall[i].y, wall[i].y + day2, 4);
                    setanimation(&wall[wall[wallfind[j]].point2].y, wall[wall[wallfind[j]].point2].y + day2, 4);
                }
            }
            else
            {
                i = wallfind[j] - 1; if (i < startwall) i = endwall;
                dax2 = ((wall[i].x + wall[wall[wallfind[j]].point2].x) >> 1) - wall[wallfind[j]].x;
                day2 = ((wall[i].y + wall[wall[wallfind[j]].point2].y) >> 1) - wall[wallfind[j]].y;
                if (dax2 != 0)
                {
                    setanimation(&wall[wallfind[j]].x, centx, 4);
                    setanimation(&wall[i].x, centx + dax2, 4);
                    setanimation(&wall[wall[wallfind[j]].point2].x, centx + dax2, 4);
                }
                else if (day2 != 0)
                {
                    setanimation(&wall[wallfind[j]].y, centy, 4);
                    setanimation(&wall[i].y, centy + day2, 4);
                    setanimation(&wall[wall[wallfind[j]].point2].y, centy + day2, 4);
                }
            }
        }
        break;
        // case 131 to 134 inclusive are Witchaven 2
        case 131:
        if (keysok == 0)
            break;
        else
        {
            SND_PlaySound(S_CREAKDOOR2, 0, 0, 0, 0);
            d_soundplayed = 1;
        }
        fallthrough__; // CHECKME - why did the WH2 code not include a break?
        case 132:
        if (keysok == 0)
            break;
        else
        {
            SND_PlaySound(S_CREAKDOOR2, 0, 0, 0, 0);
            d_soundplayed = 1;
        }
        fallthrough__;
        case 133:
        if (keysok == 0)
            break;
        else
        {
            SND_PlaySound(S_CREAKDOOR2, 0, 0, 0, 0);
            d_soundplayed = 1;
        }
        fallthrough__;
        case 134:
        if (keysok == 0)
            break;
        else
        {
            SND_PlaySound(S_CREAKDOOR2, 0, 0, 0, 0);
            d_soundplayed = 1;
        }
        fallthrough__;
        case DOORSWINGTAG:
        {
            if (WH2)
            {
                #if 0
                if (!d_soundplayed)
                    SND_PlaySound(S_SWINGDOOR, 0, 0, 0, 0);
                else
                    d_soundplayed = 1;
                #endif
            }

            for (i = 0; i < swingcnt; i++)
            {
                if (swingdoor[i].sector == s)
                {
                    if (swingdoor[i].anginc == 0)
                    {
                        if (swingdoor[i].ang == swingdoor[i].angclosed)
                        {
                            swingdoor[i].anginc = swingdoor[i].angopendir;
                        }
                        else
                        {
                            swingdoor[i].anginc = -swingdoor[i].angopendir;
                        }
                    }
                    else
                    {
                        swingdoor[i].anginc = -swingdoor[i].anginc;
                    }

                    if (WH2)
                    {
                        sector[s].hitag = 0;
                        sector[s].lotag = 0;
                    }
                }
            }
        }
        break;
    } // switch
//
//   LOWER FLOOR ANY AMOUNT
//
    if (datag >= 1100 && datag <= 1199)
    {
        // WITCHAVEN 2 code
        if (sector[s].hitag > 100)
        { // CHECKME / TODO - always default to 32 for Witchaven 1?
            speed = 64;
        }
        else
        {
            speed = 32;
        }

        sector[s].hitag = 0;

        daz = sector[s].floorz + (1024 * (sector[s].lotag - 1100));

        if ((j = setanimation(&sector[s].floorz, daz, speed)) >= 0)
        {
            SND_PlaySound(S_STONELOOP1, 0, 0, 0, (sector[s].lotag - 1100) / 10);
        }
        sector[s].lotag = 0;
    }

    //
    //   RAISE FLOOR 1-99
    //
    if (datag >= 1200 && datag <= 1299)
    {
        // WITCHAVEN 2 code
        if (sector[s].hitag > 100)
        { // CHECKME / TODO - always default to 32 for Witchaven 1?
            speed = 64;
        }
        else
        {
            speed = 32;
        }

        sector[s].hitag = 0;

        switch (sector[s].floorpicnum)
        {
            case LAVA:
            case ANILAVA:
            case LAVA1:
            sector[s].floorpicnum = COOLLAVA;
            break;
            case SLIME:
            sector[s].floorpicnum = DRYSLIME;
            break;
            case WATER:
            case HEALTHWATER:
            sector[s].floorpicnum = DRYWATER;
            break;
            case LAVA2:
            sector[s].floorpicnum = COOLLAVA2;
            break;
        }

        daz = sector[s].floorz - (1024 * (sector[s].lotag - 1200));

        if ((j = setanimation(&sector[s].floorz, daz, speed)) >= 0)
        {
            SND_PlaySound(S_STONELOOP1, 0, 0, 0, (sector[s].lotag - 1200) / 10);
        }
        sector[s].lotag = 0;
    }

    if (datag >= 1300 && datag <= 1399)
    {
        // WITCHAVEN 2 code
        if (sector[s].hitag > 100)
        { // CHECKME / TODO - always default to 32 for Witchaven 1?
            speed = 64;
        }
        else
        {
            speed = 32;
        }

        sector[s].hitag = 0;

        daz = sector[s].ceilingz + (1024 * (sector[s].lotag - 1300));

        if ((j = setanimation(&sector[s].ceilingz, daz, speed)) >= 0)
        {
            SND_PlaySound(S_STONELOOP1, 0, 0, 0, (sector[s].lotag - 1300) / 10);
        }
        sector[s].lotag = 0;
    }

    // RAISE CEILING ANY AMOUNT   
    if (datag >= 1400 && datag <= 1499)
    {
        // WITCHAVEN 2 code
        if (sector[s].hitag > 100)
        { // CHECKME / TODO - always default to 32 for Witchaven 1?
            speed = 64;
        }
        else
        {
            speed = 32;
        }

        sector[s].hitag = 0;

        daz = sector[s].ceilingz - (1024 * (sector[s].lotag - 1400));

        if ((j = setanimation(&sector[s].ceilingz, daz, speed)) >= 0)
        {
            //JSA_DEMO
            SND_PlaySound(S_STONELOOP1, 0, 0, 0, (sector[s].lotag - 1400) / 10);
        }
        sector[s].lotag = 0;
    }

    /*********
      LOWER FLOOR AND CEILING ANY AMOUNT
    *********/

    if (datag >= 1500 && datag <= 1599)
    {
        // WITCHAVEN 2 code
        if (sector[s].hitag > 100)
        { // CHECKME / TODO - always default to 32 for Witchaven 1?
            speed = 64;
        }
        else
        {
            speed = 32;
        }

        sector[s].hitag = 0;

        daz = sector[s].floorz + (1024 * (sector[s].lotag - 1500));

        setanimation(&sector[s].floorz, daz, speed);

        daz = sector[s].ceilingz + (1024 * (sector[s].lotag - 1500));

        if ((j = setanimation(&sector[s].ceilingz, daz, speed)) >= 0)
        {
            SND_PlaySound(S_STONELOOP1, 0, 0, 0, (sector[s].lotag - 1500) / 10);
        }
        sector[s].lotag = 0;
    }

    //   
    //  RAISE FLOOR AND CEILING ANY AMOUNT
    //
    if (datag >= 1600 && datag <= 1699)
    {
        // WITCHAVEN 2 code
        if (sector[s].hitag > 100)
        { // CHECKME / TODO - always default to 32 for Witchaven 1?
            speed = 64;
        }
        else
        {
            speed = 32;
        }

        sector[s].hitag = 0;

        daz = sector[s].floorz - (1024 * (sector[s].lotag - 1600));

        setanimation(&sector[s].floorz, daz, speed);

        daz = sector[s].ceilingz - (1024 * (sector[s].lotag - 1600));

        if ((j = setanimation(&sector[s].ceilingz, daz, speed)) >= 0)
        {
            SND_PlaySound(S_STONELOOP1, 0, 0, 0, (sector[s].lotag - 1600) / 10);
        }
        sector[s].lotag = 0;
    }

    if (datag >= 1800 && datag <= 1899)
    {
        i = getanimationgoal(&sector[s].floorz);
        if (i >= 0)
        {
            daz = sector[s].ceilingz + (1024 * 16);

            if (animategoal[i] == daz)
                animategoal[i] = sector[nextsectorneighborz(s, sector[s].ceilingz - (1024 * 16), -1, -1)].floorz;
            else
                animategoal[i] = daz;
        }
        else
        {
            if (sector[s].floorz == sector[s].ceilingz + (1024 * 16))
                daz = sector[nextsectorneighborz(s, sector[s].ceilingz - (1024 * 16), -1, -1)].floorz;
            else
            {
                daz = sector[s].ceilingz + (1024 * 16);
            }
            if ((j = setanimation(&sector[s].floorz, daz, 32)) >= 0)
            {
                SND_PlaySound(S_STONELOOP1, 0, 0, 0, (sector[s].lotag - 1800) / 10);
            }
        }
    }

    if (datag >= 1900 && datag <= 1999)
    {
        sector[s].hitag = 0;
        temp1 = sector[s].lotag - 1900;
        temp2 = temp1 / 10;
        temp3 = temp1 - temp2;

        SND_PlaySound(S_STONELOOP1, 0, 0, 0, 0);

        switch (temp3)
        {  // type of crush
            case 0:
            sector[s].lotag = DOORDOWNTAG;
            setanimation(&sector[s].floorz, sector[s].ceilingz, 64);
            break;
            case 1:
            daz = sector[s].ceilingz;
            setanimation(&sector[s].floorz, daz, 64);
            sector[s].lotag = 0;
            crushsectoranim[s] = 0;
            crushsectordone[s] = 0;
            break;
            case 2:
            daz = sector[s].floorz;
            setanimation(&sector[s].ceilingz, daz, 64);
            sector[s].lotag = 0;
            crushsectoranim[s] = 0;
            crushsectordone[s] = 0;
            break;
            case 3:
            sector[s].lotag = 0;
            crushsectoranim[s] = 1;
            crushsectordone[s] = 1;
            break;
            case 4:
            sector[s].lotag = 0;
            crushsectoranim[s] = 2;
            crushsectordone[s] = 1;
            break;
            case 5:
            daz = (sector[s].ceilingz + sector[s].floorz) >> 1;
            setanimation(&sector[s].floorz, daz, 64);
            setanimation(&sector[s].ceilingz, daz, 64);
            break;
            case 6:
            sector[s].lotag = 0;
            crushsectoranim[s] = 3;
            crushsectordone[s] = 1;
            break;
        }
    }

    // BRASS KEY
    // BLACK KEY
    // GLASS KEY
    // IVORY KEY

    if (datag >= 2000 && datag <= 2999)
    {
        doorkey = (sector[s].lotag - 2000) / 100;
        doorantic = (sector[s].lotag - (2000 + (doorkey * 100))) / 10;
        doortype = sector[s].lotag - (2000 + (doorkey * 100) + (doorantic * 10));
        checkforkey = 0;
        for (i = 0; i < MAXDOORKEYS; i++)
        {
            if (plr->treasure[i] == doorkey)
                checkforkey = 1;
        }
        if (!checkforkey)
            goto getoutahere;
        if (doorantic == 0)
            sector[s].lotag = 0;
        switch (doortype)
        {
            case 0: // up
            i = getanimationgoal(&sector[s].ceilingz);
            if (i >= 0)
            {
                goalz = sector[nextsectorneighborz(s, sector[s].floorz, -1, -1)].ceilingz;
                if (animategoal[i] == goalz)
                {
                    animategoal[i] = sector[s].floorz;
                    if (doorantic == 2 || doorantic == 3)
                        setdelayfunc(operatesector, s, 0);
                }
                else
                {
                    animategoal[i] = goalz;
                    if (doorantic == 2)
                        setdelayfunc(operatesector, s, DOORDELAY);
                    else if (doorantic == 3)
                        setdelayfunc(operatesector, s, DOORDELAY << 3);
                }
            }
            else
            {
                if (sector[s].ceilingz == sector[s].floorz)
                {
                    goalz = sector[nextsectorneighborz(s, sector[s].floorz, -1, -1)].ceilingz;
                    if (doorantic == 2)
                        setdelayfunc(operatesector, s, DOORDELAY);
                    else if (doorantic == 3)
                        setdelayfunc(operatesector, s, DOORDELAY << 3);
                }
                else
                {
                    goalz = sector[s].floorz;
                    if (doorantic == 2 || doorantic == 3)
                        setdelayfunc(operatesector, s, 0);
                }
                setanimation(&sector[s].ceilingz, goalz, DOORSPEED);
            }
            break;
            case 1: //dn
            i = getanimationgoal(&sector[s].floorz);
            if (i >= 0)
            {
                goalz = sector[nextsectorneighborz(s, sector[s].ceilingz, 1, 1)].floorz;
                if (animategoal[i] == goalz)
                {
                    animategoal[i] = sector[s].ceilingz;
                    if (doorantic == 2 || doorantic == 3)
                        setdelayfunc(operatesector, s, 0);
                }
                else
                {
                    animategoal[i] = goalz;
                    if (doorantic == 2)
                        setdelayfunc(operatesector, s, DOORDELAY);
                    else if (doorantic == 3)
                        setdelayfunc(operatesector, s, DOORDELAY << 3);
                }
            }
            else
            {
                if (sector[s].ceilingz == sector[s].floorz)
                {
                    goalz = sector[nextsectorneighborz(s, sector[s].ceilingz, 1, 1)].floorz;
                    if (doorantic == 2)
                        setdelayfunc(operatesector, s, DOORDELAY);
                    else if (doorantic == 3)
                        setdelayfunc(operatesector, s, DOORDELAY << 3);
                }
                else
                {
                    goalz = sector[s].ceilingz;
                    if (doorantic == 2 || doorantic == 3)
                        setdelayfunc(operatesector, s, 0);
                }
                setanimation(&sector[s].floorz, goalz, DOORSPEED);
            }
            break;
            case 2: //middle
            i = getanimationgoal(&sector[s].floorz);
            j = getanimationgoal(&sector[s].ceilingz);
            if (i >= 0)
            {
                botz = sector[nextsectorneighborz(s, sector[s].ceilingz, 1, 1)].floorz;
                if (animategoal[i] == botz)
                {
                    animategoal[i] = (sector[s].ceilingz + sector[s].floorz) >> 1;
                    if (doorantic == 2 || doorantic == 3)
                        setdelayfunc(operatesector, s, 0);
                }
                else
                {
                    animategoal[i] = botz;
                    if (doorantic == 2)
                        setdelayfunc(operatesector, s, DOORDELAY);
                    else if (doorantic == 3)
                        setdelayfunc(operatesector, s, DOORDELAY << 3);
                }
            }
            else
            {
                if (sector[s].ceilingz == sector[s].floorz)
                {
                    botz = sector[nextsectorneighborz(s, sector[s].ceilingz, 1, 1)].floorz;
                    if (doorantic == 2)
                        setdelayfunc(operatesector, s, DOORDELAY);
                    else if (doorantic == 3)
                        setdelayfunc(operatesector, s, DOORDELAY << 3);
                }
                else
                {
                    botz = (sector[s].ceilingz + sector[s].floorz) >> 1;
                    if (doorantic == 2 || doorantic == 3)
                        setdelayfunc(operatesector, s, 0);
                }
                setanimation(&sector[s].floorz, botz, DOORSPEED);
            }
            if (j >= 0)
            {
                topz = sector[nextsectorneighborz(s, sector[s].floorz, -1, -1)].ceilingz;
                if (animategoal[j] == topz)
                {
                    animategoal[j] = (sector[s].ceilingz + sector[s].floorz) >> 1;
                }
                else
                {
                    animategoal[j] = topz;
                }
            }
            else
            {
                if (sector[s].ceilingz == sector[s].floorz)
                {
                    topz = sector[nextsectorneighborz(s, sector[s].floorz, -1, -1)].ceilingz;
                }
                else
                {
                    topz = (sector[s].ceilingz + sector[s].floorz) >> 1;
                }
                setanimation(&sector[s].ceilingz, topz, DOORSPEED);
            }
        getoutahere:
            break;
            case 3: //vert
            case 4: //swing
            break;
        }
        //} // if key
    } // end of complexdoors

    if (datag == 3000)
    {
        for (k = 0; k < ironbarscnt; k++)
        {
            if (ironbarsector[k] == s)
            {
                ironbarsdone[k] = 1;

                switch (sprite[ironbarsanim[k]].picnum)
                {

                    case SWINGDOOR:
                    case SWINGDOOR2:
                    case SWINGDOOR3:
                    case TALLSWING:
                    case TALLSWING2:
                    SND_PlaySound(S_CREAKDOOR2, 0, 0, 0, 0);
                    break;

                    case SWINGGATE:
                    SND_PlaySound(S_CREAKDOOR3, 0, 0, 0, 0);
                    break;

                    case SWINGHOLE:
                    case SWINGGATE2:
                    case SWINGGATE3:
                    case SWINGGATE4:
                    case SWINGGATE5:
                    case ROPEDOOR:
                    case GEARSSTART:
                    case WOODGEARSSTART:
                    case GEARS2START:
                    SND_PlaySound(S_CREAKDOOR1, 0, 0, 0, 0);
                    break;
                }
            }
        }
    }

    if (datag == 4000)
    {
        sector[datag].lotag = 0;
        //here
        for (k = 0; k < MAXSPRITES; k++)
        {
            if (sector[s].hitag == sprite[k].hitag && sprite[k].extra < 1)
            {
                newstatus(k, FLOCKSPAWN);
                if (batsnd == -1)
                    batsnd = SND_PlaySound(S_BATSLOOP, sprite[k].x, sprite[k].y, 0, -1);
                sector[datag].lotag = sector[datag].hitag = 0;
            }
        }
    }
    if (datag == 4001)
    {
        sector[datag].lotag = 0;
        for (k = 0; k < MAXSPRITES; k++)
        {
            if (sector[s].hitag == sprite[k].hitag)
            {
                //sprite[k].hitag=15;
                newstatus(k, WAR);
            }
        }
    }

    // keysok=0;
    // BRASS KEY
    // BLACK KEY
    // GLASS KEY
    // IVORY KEY

    switch (datag)
    {
        case 131:
        case 61:
        case 71:
        // check for proper key
        if (plr->treasure[14] == 0)
        {
            keysok = 0;
            StatusMessage(360, "BRASS KEY NEEDED");
        }
        else keysok = 1;
        break;
        case 132:
        case 62:
        // check for proper key
        if (plr->treasure[15] == 0)
        {
            keysok = 0;
            StatusMessage(360, "Black KEY NEEDED");
        }
        else keysok = 1;
        break;
        case 133:
        case 63:
        case 73:
        // check for proper key
        if (plr->treasure[16] == 0)
        {
            keysok = 0;
            StatusMessage(360, "glass KEY NEEDED");
        }
        else keysok = 1;
        break;
        case 134:
        case 64:
        if (plr->treasure[17] == 0)
        {
            keysok = 0;
            StatusMessage(360, "ivory KEY NEEDED");
        }
        else keysok = 1;
        break;
        case 72:
        // check for proper key
        if (plr->treasure[15] == 0)
        {
            keysok = 0;
            StatusMessage(360, "black KEY NEEDED");
        }
        else keysok = 1;
        break;
        case 74:
        if (plr->treasure[17] == 0)
        {
            keysok = 0;
            StatusMessage(360, "Ivory KEY NEEDED");
        }
        else keysok = 1;
        break;

        i = getanimationgoal(&sector[s].floorz);

        if (i >= 0)
        {
            goalz = sector[nextsectorneighborz(s, sector[s].ceilingz, 1, 1)].floorz;
            if (animategoal[i] == goalz)
            {
                animategoal[i] = sector[s].ceilingz;
            }
            else
            {
                animategoal[i] = goalz;
            }
        }
        else
        {
            if (sector[s].ceilingz == sector[s].floorz)
            {
                goalz = sector[nextsectorneighborz(s, sector[s].ceilingz, 1, 1)].floorz;
            }
            else
            {
                goalz = sector[s].ceilingz;
            }
            setanimation(&sector[s].floorz, goalz, DOORSPEED);
        }
        //JSA_DEMO
        SND_PlaySound(S_DOOR1, 0, 0, 0, 0);
        break;
    }

    switch (datag)
    {
        case 61:
        case 62:
        case 63:
        case 64:
        if (keysok == 0)
            break;
        else
        {
            i = getanimationgoal(&sector[s].ceilingz);
            if (i >= 0)
            {
                goalz = sector[nextsectorneighborz(s, sector[s].floorz, -1, -1)].ceilingz;
                if (animategoal[i] == goalz)
                {
                    animategoal[i] = sector[s].floorz;
                }
                else
                {
                    animategoal[i] = goalz;
                }
            }
            else
            {
                if (sector[s].ceilingz == sector[s].floorz)
                {
                    goalz = sector[nextsectorneighborz(s, sector[s].floorz, -1, -1)].ceilingz;
                }
                else
                {
                    goalz = sector[s].floorz;
                }
                setanimation(&sector[s].ceilingz, goalz, DOORSPEED);
            }
            SND_PlaySound(S_DOOR2, 0, 0, 0, 0);
        }
    }
}

void animatetags(Player* plr)
{
    int  endwall, good, i, j, k, oldang, startwall;
    int  nexti;
    int32_t dax, dax2, day, day2;
    int32_t dasector;

    if (sector[plr->sector].lotag == 2)
    {
        for (i = 0; i < numsectors; i++)
            if (sector[i].hitag == sector[plr->sector].hitag)
                if (sector[i].lotag != 2)
                    operatesector(i);
        i = headspritestat[0];
        while (i != -1)
        {
            nexti = nextspritestat[i];
            if (sprite[i].hitag == sector[plr->sector].hitag)
                operatesprite(i);
            i = nexti;
        }

        sector[plr->sector].lotag = 0;
        sector[plr->sector].hitag = 0;
    }
    if ((sector[plr->sector].lotag == 1) && (plr->sector != plr->oldsector))
    {
        for (i = 0; i < numsectors; i++)
            if (sector[i].hitag == sector[plr->sector].hitag)
                if (sector[i].lotag != 2)
                    operatesector(i);
        i = headspritestat[0];
        while (i != -1)
        {
            nexti = nextspritestat[i];
            if (sprite[i].hitag == sector[plr->sector].hitag)
                operatesprite(i);
            i = nexti;
        }
    }

    /*
         if (plr->sector != plr->oldsector) {
              if (sector[plr->sector].lotag == ACTIVATESECTOR
                   || sector[plr->sector].lotag == ACTIVATESECTORONCE) {
                   for (i=0 ; i < numsectors ; i++) {
                        if (sector[i].hitag == sector[plr->sector].hitag) {
                             if (sector[i].lotag != ACTIVATESECTOR
                                  || sector[i].lotag != ACTIVATESECTORONCE) {
                                  operatesector(i);
                                  operatesprite(i);
                            }
                        }
                   }
                   if(sector[plr->sector].lotag == ACTIVATESECTORONCE) {
                      sector[plr->sector].lotag=0;
                      sector[plr->sector].hitag=0;
                   }
              }
         }

    */

    for (i = 0; i < dragsectorcnt; i++)
    {
        dasector = dragsectorlist[i];

        startwall = sector[dasector].wallptr;
        endwall = startwall + sector[dasector].wallnum - 1;

        if (wall[startwall].x + dragxdir[i] < dragx1[i]) dragxdir[i] = 16;
        if (wall[startwall].y + dragydir[i] < dragy1[i]) dragydir[i] = 16;
        if (wall[startwall].x + dragxdir[i] > dragx2[i]) dragxdir[i] = -16;
        if (wall[startwall].y + dragydir[i] > dragy2[i]) dragydir[i] = -16;

        for (j = startwall; j <= endwall; j++)
        {
            dragpoint(j, wall[j].x + dragxdir[i], wall[j].y + dragydir[i], 0);
        }

        j = sector[dasector].floorz;
        sector[dasector].floorz = dragfloorz[i] + (Sin(lockclock << 4) >> 3);

        if (plr->sector == dasector)
        {

            plr->x += dragxdir[i];
            plr->y += dragydir[i];
            plr->z += (sector[dasector].floorz - j);

            //Update sprite representation of player

            vec3_t pos;
            pos.x = plr->x;
            pos.y = plr->y;
            pos.z = plr->z + (plr->height);

            setsprite(plr->spritenum, &pos);
            sprite[plr->spritenum].ang = plr->ang;
        }
    }

    for (i = 0; i < swingcnt; i++)
    {
        if (swingdoor[i].anginc != 0)
        {
            oldang = swingdoor[i].ang;
            for (j = 0; j < (synctics << 2); j++)
            {
                swingdoor[i].ang = ((swingdoor[i].ang + 2048 + swingdoor[i].anginc) & kAngleMask);
                if (swingdoor[i].ang == swingdoor[i].angclosed)
                {
                    swingdoor[i].anginc = 0;
                }
                if (swingdoor[i].ang == swingdoor[i].angopen)
                {
                    swingdoor[i].anginc = 0;
                }
            }
            for (k = 1; k <= 3; k++)
            {
                vec2_t pivot;
                pivot.x = swingdoor[i].x[0];
                pivot.y = swingdoor[i].y[0];

                vec2_t p;
                p.x = swingdoor[i].x[k];
                p.y = swingdoor[i].y[k];

                vec2_t p2;
                p2.x = wall[swingdoor[i].wall[k]].x;
                p2.y = wall[swingdoor[i].wall[k]].y;

                rotatepoint(pivot, p, swingdoor[i].ang, &p2);

                wall[swingdoor[i].wall[k]].x = p2.x;
                wall[swingdoor[i].wall[k]].y = p2.y;
            }

            if (swingdoor[i].anginc != 0)
            {
                if (plr->sector == swingdoor[i].sector)
                {
                    good = 1;
                    for (k = 1; k <= 3; k++)
                    {
                        dax = wall[swingdoor[i].wall[k]].x - plr->x + 64;
                        day = wall[swingdoor[i].wall[k]].y - plr->y + 64;
                        dax2 = wall[wall[swingdoor[i].wall[k]].point2].x - plr->x + 64;
                        day2 = wall[wall[swingdoor[i].wall[k]].point2].y - plr->y + 64;
                        #if 0 // TODO
                        if (clipinsidebox(dax, day, dax2, day2, 128) == 1)
                        {
                            good = 0;
                            break;
                        }
                        #endif
                    }
                    if (good == 0)
                    {
                        swingdoor[i].ang = oldang;

                        for (k = 1; k <= 3; k++)
                        {
                            vec2_t pivot;
                            pivot.x = swingdoor[i].x[0];
                            pivot.y = swingdoor[i].y[0];

                            vec2_t p;
                            p.x = swingdoor[i].x[k];
                            p.y = swingdoor[i].y[k];

                            vec2_t p2;
                            p2.x = wall[swingdoor[i].wall[k]].x;
                            p2.y = wall[swingdoor[i].wall[k]].y;

                            rotatepoint(pivot, p, swingdoor[i].ang, &p2);

                            wall[swingdoor[i].wall[k]].x = p2.x;
                            wall[swingdoor[i].wall[k]].y = p2.y;
                        }
                        swingdoor[i].anginc = -swingdoor[i].anginc;
                        break;
                    }
                }
            }
        }
    }
}
