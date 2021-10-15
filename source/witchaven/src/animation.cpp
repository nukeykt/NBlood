
/***************************************************************************
 *   WHANI.C  - object animation code for Witchaven game                   *
 *                                                                         *
 ***************************************************************************/
#include "witchaven.h"
#include "player.h"
#include "objects.h"
#include "sound.h"
#include "menu.h"
#include "effects.h"
#include "view.h"

#define TICSPERFRAME 3

extern int cartsnd;
extern int batsnd;

extern int mapon;
extern int spellbooktics;
extern int spellbook;
extern int spellbookframe;
extern int spellbookflip;

extern short torchpattern[];
extern char flashflag;
extern char tempbuf[];

extern int potiontilenum;
extern int debuginfo;
extern int scoretime;


void animateobjs(Player *plr)
{
    short osectnum = 0;
    short hitobject, hitdamage;
    int32_t i, nexti, dax, day, daz, j, k;
    short daang, movestat;
    int speed;
    int32_t ironbarmove;

    short startwall, endwall;

    int32_t dist, olddist;
    int found;

    int32_t dx, dy, dz, dh;
    int32_t nextj;

    // explosion variables:
    int32_t x, y, z;
    short belongs;

    int32_t mindist = 0x7fffffff;
    short target;

    if (plr->sector < 0 || plr->sector >= numsectors) {
        return;
    }

    i = headspritestat[PULLTHECHAIN];
    while (i != -1)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;
        if (sprite[i].lotag < 0)
        {
            sprite[i].picnum++;
            sprite[i].lotag = 24;
            if (sprite[i].picnum == PULLCHAIN3)
            {
                sprite[i].lotag = 0;
                changespritestat(i, 0);
            }
        }
        i = nexti;
    }

    i = headspritestat[ANIMLEVERDN];
    while (i != -1)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;
        if (sprite[i].lotag < 0)
        {
            sprite[i].picnum++;
            sprite[i].lotag = 24;
            if (sprite[i].picnum == LEVERDOWN)
            {
                sprite[i].lotag = 60;
                changespritestat(i, 0);
            }
        }
        i = nexti;
    }

    i = headspritestat[ANIMLEVERUP];
    while (i != -1)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;
        if (sprite[i].lotag < 0)
        {
            sprite[i].picnum--;
            sprite[i].lotag = 24;
            if (sprite[i].picnum == LEVERUP)
            {
                sprite[i].lotag = 1;
                changespritestat(i, 0);
            }
        }
        i = nexti;
    }

    i = headspritestat[WARPFX];
    while (i != -1)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;

        if (sprite[i].lotag < 0)
        {
            sprite[i].lotag = 12;
            sprite[i].picnum++;
            if (sprite[i].picnum == ANNIHILATE + 5)
                deletesprite(i);
        }
        i = nexti;
    }

    i = headspritestat[CHARCOAL];
    while (i != -1)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;
        if (sprite[i].lotag < 0)
        {
            sprite[i].picnum++;
            sprite[i].lotag = 24;
            switch (sprite[i].picnum)
            {
                case DEVILCHAR + 4:
                case GOBLINCHAR + 4:
                case JUDYCHAR + 4:
                case KOBOLDCHAR + 4:
                case MINOTAURCHAR + 4:
                case SKELETONCHAR + 4:
                case GRONCHAR + 4:
                case DRAGONCHAR + 4:
                case GUARDIANCHAR + 4:
                case FATWITCHCHAR + 4:
                case SKULLYCHAR + 4:
                trailingsmoke(i);
                deletesprite(i);
                break;
            }
        }
        i = nexti;
    }

    i = headspritestat[FROZEN];
    while (i != -1)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;
        if (sprite[i].lotag < 0)
        {
            sprite[i].pal = 0;
            newstatus(i, FACE);
            switch (sprite[i].picnum)
            {
                case GRONHALDIE:
                sprite[i].picnum = GRONHAL;
                break;
                case GRONMUDIE:
                sprite[i].picnum = GRONMU;
                break;
                case GRONSWDIE:
                sprite[i].picnum = GRONSW;
                break;
                case KOBOLDDIE:
                sprite[i].picnum = KOBOLD;
                break;
                case DRAGONDIE:
                sprite[i].picnum = DRAGON;
                break;
                case DEVILDIE:
                sprite[i].picnum = DEVIL;
                break;
                case FREDDIE:
                sprite[i].picnum = FRED;
                break;
                case SKELETONDIE:
                sprite[i].picnum = SKELETON;
                break;
                case GOBLINDIE:
                sprite[i].picnum = GOBLIN;
                break;
                case MINOTAURDIE:
                sprite[i].picnum = MINOTAUR;
                break;
                case SPIDERDIE:
                sprite[i].picnum = SPIDER;
                break;
            }
        }
        i = nexti;
    }

    // if cansee GOBLIN
    // WAR
    i = headspritestat[WAR];
    while (i != -1)
    {
        nexti = nextspritestat[i];
        if (sprite[i].lotag > 256)
        {
            sprite[i].lotag = 100;
            sprite[i].extra = 0;
        }
        switch (sprite[i].extra)
        {
            case 0: // find new target
            olddist = 1024 << 4;
            found = 0;
            for (k = 0; k < MAXSPRITES; k++)
            {
                if (sprite[k].picnum == GOBLIN
                    && sprite[i].pal != sprite[k].pal
                    && sprite[i].hitag == sprite[k].hitag)
                {
                    dist = labs(sprite[i].x - sprite[k].x) + labs(sprite[i].y - sprite[k].y);
                    if (dist < olddist)
                    {
                        found = 1;
                        olddist = dist;
                        sprite[i].owner = k;
                        sprite[i].ang = (getangle(sprite[k].x - sprite[i].x, sprite[k].y - sprite[i].y) & kAngleMask);
                        sprite[i].extra = 1;
                    }
                }
            }
            if (found == 0)
            {
                if (sprite[i].pal == 5)
                    sprite[i].hitag = adjusthp(35);
                else if (sprite[i].pal == 4)
                    sprite[i].hitag = adjusthp(25);
                else
                    sprite[i].hitag = adjusthp(15);
                if (plr->shadowtime > 0)
                {
                    sprite[i].ang = (((krand() & 512) - 256) + sprite[i].ang + 1024) & kAngleMask;  // NEW
                    newstatus(i, FLEE);
                }
                else
                    newstatus(i, FACE);
            }
            break;
            case 1: // chase
            k = sprite[i].owner;
            sprite[i].z = sector[sprite[i].sectnum].floorz;
            sprite[i].ang = (getangle(sprite[k].x - sprite[i].x, sprite[k].y - sprite[i].y) & kAngleMask);

            movestat = movesprite(i, (Cos(sprite[i].ang) * synctics) << 3, (Sin(sprite[i].ang) * synctics) << 3, 0, 4 << 8, 4 << 8, 2);
 
            if (sector[sprite[i].sectnum].floorpicnum == WATER
                || sector[sprite[i].sectnum].floorpicnum == LAVA
                || sector[sprite[i].sectnum].floorpicnum == SLIME)
            {
                if (sector[sprite[i].sectnum].floorpicnum == WATER || sector[sprite[i].sectnum].floorpicnum == SLIME)
                {
                    if (sprite[i].picnum == FISH)
                        sprite[i].z = sector[sprite[i].sectnum].floorz;
                    else
                    {
                        if (rand() % 100 > 60)
                            makemonstersplash(SPLASHAROO, i);
                    }
                }
                else
                {
                    trailingsmoke(i);
                    makemonstersplash(LAVASPLASH, i);
                }
                sprite[i].z += tilesiz[sprite[i].picnum].y << 5;
            }

            vec3_t pos;
            pos.x = sprite[i].x;
            pos.y = sprite[i].y;
            pos.z = sprite[i].z;

            setsprite(i, &pos);
            if (checkdist(i, sprite[k].x, sprite[k].y, sprite[k].z))
            {
                sprite[i].extra = 2;
            }
            else
                sprite[i].picnum = GOBLIN;

            if (sector[sprite[i].sectnum].floorz - (32 << 8) < sector[sprite[i].sectnum].ceilingz)
            {
                if (sector[sprite[i].sectnum].lotag == 6) {
                    newstatus(i, DIE);
                }
                else {
                    deletesprite(i);
                }
            }

            break;
            case 2: // attack
            k = sprite[i].owner;
            if (checkdist(i, sprite[k].x, sprite[k].y, sprite[k].z))
            {

                if ((krand() & 100) > 99)
                {
                    // goblins are fighting
                    //JSA_DEMO
                    if (rand() % 10 > 6)
                        playsound_loc(S_GENSWING, sprite[i].x, sprite[i].y);
                    if (rand() % 10 > 6)
                        playsound_loc(S_SWORD1 + (rand() % 6), sprite[i].x, sprite[i].y);

                    if (checkdist(i, plr->x, plr->y, plr->z))
                        sethealth(-(krand() & 5));

                    if (krand() % 100 > 90)
                    {   // if k is dead
                        sprite[i].extra = 0; // needs to chase
                        sprite[i].picnum = GOBLIN;
                        sprite[k].extra = 4;
                        sprite[k].picnum = GOBLINDIE;
                        sprite[k].lotag = 20;
                        sprite[k].hitag = 0;
                        newstatus(k, DIE);
                    }
                    else
                    {  // i attack k flee
                        sprite[i].extra = 0;
                        sprite[k].extra = 3;
                        sprite[k].ang = (sprite[i].ang + ((krand() & 256) - 128)) & kAngleMask;
                        sprite[k].lotag = 60;
                    }
                }
            }
            else
            {
                sprite[i].extra = 1;
            }
            if (sector[sprite[i].sectnum].floorpicnum == WATER
                || sector[sprite[i].sectnum].floorpicnum == LAVA
                || sector[sprite[i].sectnum].floorpicnum == SLIME)
            {
                if (sector[sprite[i].sectnum].floorpicnum == WATER || sector[sprite[i].sectnum].floorpicnum == SLIME)
                {
                    if (sprite[i].picnum == FISH)
                        sprite[i].z = sector[sprite[i].sectnum].floorz;
                    else
                    {
                        if (rand() % 100 > 60)
                            makemonstersplash(SPLASHAROO, i);
                    }
                }
                else
                {
                    trailingsmoke(i);
                    makemonstersplash(LAVASPLASH, i);
                }
                sprite[i].z += tilesiz[sprite[i].picnum].y << 5;
            }

            //vec3_t pos;
            pos.x = sprite[i].x;
            pos.y = sprite[i].y;
            pos.z = sprite[i].z;

            setsprite(i, &pos);

            if (sector[sprite[i].sectnum].floorz - (32 << 8) < sector[sprite[i].sectnum].ceilingz)
            {
                if (sector[sprite[i].sectnum].lotag == 6) {
                    newstatus(i, DIE);
                }
                else {
                    deletesprite(i);
                }
            }

            break;
            case 3: // flee
            sprite[i].lotag -= synctics;
            sprite[i].z = sector[sprite[i].sectnum].floorz;

            movestat = movesprite(i, (Cos(sprite[i].ang) * synctics) << 3, (Sin(sprite[i].ang) * synctics) << 3, 0, 4 << 8, 4 << 8, 2);

            if (sector[sprite[i].sectnum].floorpicnum == WATER
                || sector[sprite[i].sectnum].floorpicnum == LAVA
                || sector[sprite[i].sectnum].floorpicnum == SLIME)
            {
                if (sector[sprite[i].sectnum].floorpicnum == WATER || sector[sprite[i].sectnum].floorpicnum == SLIME)
                {
                    if (sprite[i].picnum == FISH)
                        sprite[i].z = sector[sprite[i].sectnum].floorz;
                    else
                    {
                        if (rand() % 100 > 60)
                            makemonstersplash(SPLASHAROO, i);
                    }
                }
                else
                {
                    trailingsmoke(i);
                    makemonstersplash(LAVASPLASH, i);
                }
                sprite[i].z += tilesiz[sprite[i].picnum].y << 5;
            }

            //vec3_t pos;
            pos.x = sprite[i].x;
            pos.y = sprite[i].y;
            pos.z = sprite[i].z;

            setsprite(i, &pos);
            if (movestat != 0)
                sprite[i].ang = krand() & kAngleMask;

            if (sprite[i].lotag < 0)
            {
                sprite[i].lotag = 0;
                sprite[i].extra = 0;
            }

            if (sector[sprite[i].sectnum].floorz - (32 << 8) < sector[sprite[i].sectnum].ceilingz)
            {
                if (sector[sprite[i].sectnum].lotag == 6) {
                    newstatus(i, DIE);
                }
                else {
                    deletesprite(i);
                }
            }

            break;
            case 4: // pain
            sprite[i].picnum = GOBLINDIE;
            break;
            case 5: // cast
            break;
        }

        j = headspritesect[sprite[i].sectnum];

        while (j != -1)
        {
            nextj = nextspritesect[j];
            dx = labs(sprite[i].x - sprite[j].x);            // x distance to sprite
            dy = labs(sprite[i].y - sprite[j].y);            // y distance to sprite
            dz = labs((sprite[i].z >> 8) - (sprite[j].z >> 8));  // z distance to sprite
            dh = tilesiz[sprite[j].picnum].y >> 1;       // height of sprite

            if (dx + dy < PICKDISTANCE && dz - dh <= PICKHEIGHT)
            {
                switch (sprite[i].picnum)
                {
                    case GOBLIN:
                    switch (sprite[j].picnum)
                    {
                        case EXPLO2:
                        case SMOKEFX:
                        case MONSTERBALL:
                        sprite[i].hitag -= synctics << 2;
                        if (sprite[i].hitag < 0)
                        {
                            newstatus(i, DIE);
                        }
                        break;
                    }
                }
            }
            j = nextj;
        }
        i = nexti;
    }

    // PAIN
    i = headspritestat[PAIN];
    while (i != -1)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;
        sprite[i].z = sector[sprite[i].sectnum].floorz;

        if (sprite[i].lotag < 0)
        {
            switch (sprite[i].picnum)
            {
                case GRONHALPAIN:
                sprite[i].picnum = GRONHAL;
                break;
                case GRONSWPAIN:
                sprite[i].picnum = GRONSW;
                break;
                case GRONMUPAIN:
                sprite[i].picnum = GRONMU;
                break;
                case  KOBOLDDIE:
                sprite[i].picnum = KOBOLD;
                break;
                case  DEVILPAIN:
                sprite[i].picnum = DEVIL;
                break;
                case  FREDPAIN:
                sprite[i].picnum = FRED;
                break;
                case  GOBLINPAIN:
                sprite[i].picnum = GOBLIN;
                break;
                case  MINOTAURPAIN:
                sprite[i].picnum = MINOTAUR;
                break;
                case  GUARDIANCHAR:
                sprite[i].picnum = GUARDIAN;
                break;
                case  FATWITCHDIE:
                sprite[i].picnum = FATWITCH;
                break;
                case  SKULLYDIE:
                sprite[i].picnum = SKULLY;
                break;
                case JUDY:
                sprite[i].picnum = JUDY;
                break;
            }
            sprite[i].ang = plr->ang;
            newstatus(i, FLEE);
        }

        movestat = movesprite(i, (Cos(sprite[i].ang) * synctics) << 3, (Sin(sprite[i].ang) * synctics) << 3, 0, 4 << 8, 4 << 8, 2);

        vec3_t pos;
        pos.x = sprite[i].x;
        pos.y = sprite[i].y;
        pos.z = sprite[i].z;

        setsprite(i, &pos);

        j = headspritesect[sprite[i].sectnum];
        while (j != -1)
        {
            nextj = nextspritesect[j];
            dx = labs(sprite[i].x - sprite[j].x);            // x distance to sprite
            dy = labs(sprite[i].y - sprite[j].y);            // y distance to sprite
            dz = labs((sprite[i].z >> 8) - (sprite[j].z >> 8));  // z distance to sprite
            dh = tilesiz[sprite[j].picnum].y >> 1;       // height of sprite

            if (dx + dy < PICKDISTANCE && dz - dh <= PICKHEIGHT)
            {
                switch (sprite[i].picnum)
                {
                    case SKELETON:
                    case KOBOLDDIE:
                    case FREDPAIN:
                    case GOBLINPAIN:
                    case SPIDER:
                    switch (sprite[j].picnum)
                    {
                        case EXPLO2:
                        case SMOKEFX:
                        case MONSTERBALL:
                        sprite[i].hitag -= synctics << 2;
                        if (sprite[i].hitag < 0)
                        {
                            newstatus(i, DIE);
                        }
                        break;
                    }
                    break;
                    case GRONHALPAIN:
                    case GRONSWPAIN:
                    case GRONMUPAIN:
                    switch (sprite[j].picnum)
                    {
                        case EXPLO2:
                        case MONSTERBALL:
                        sprite[i].hitag -= synctics << 2;
                        if (sprite[i].hitag < 0)
                        {
                            newstatus(i, DIE);
                        }
                        break;
                    }
                    break;
                }
            }
            j = nextj;
        }
        i = nexti;
    }

    // FLOCKSPAWN
    i = headspritestat[FLOCKSPAWN];
    while (i != -1)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;
        if (sprite[i].lotag < 0)
        {
            sprite[i].extra--;
            sprite[i].lotag = (krand() & 48) + 24;
            bats(i);
            if (sprite[i].extra == 0)
                changespritestat(i, 0);
        }
        i = nexti;
    }

    // FLOCK
    i = headspritestat[FLOCK];
    while (i != -1)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;
        switch (sprite[i].extra)
        {
            case 0: // going out of the cave
            if (sprite[i].lotag < 0)
            {
                sprite[i].extra = 1;
                sprite[i].lotag = 512;
            }
            else
            {
                movestat = movesprite(i, (Cos(sprite[i].ang) * synctics) << 3, (Sin(sprite[i].ang) * synctics) << 3, 0, 4 << 8, 4 << 8, 0);

                vec3_t pos;
                pos.x = sprite[i].x;
                pos.y = sprite[i].y;
                pos.z = sprite[i].z;

                setsprite(i, &pos);
                if (movestat != 0)
                    sprite[i].ang = rand() & kAngleMask;
            }
            break;
            case 1: // flying in circles
            if (sprite[i].lotag < 0)
            {
                sprite[i].extra = 2;
                sprite[i].lotag = 512;
                sprite[i].ang = ((getangle(sprite[sprite[i].hitag].x - sprite[i].x, sprite[sprite[i].hitag].y - sprite[i].y) & kAngleMask) - 1024) & kAngleMask;
            }
            else
            {
                sprite[i].z -= synctics << 4;
                sprite[i].ang = (sprite[i].ang + (synctics << 2)) & kAngleMask;

                movestat = movesprite(i, (Cos(sprite[i].ang) * synctics) << 3, (Sin(sprite[i].ang) * synctics) << 3, 0, 4 << 8, 4 << 8, 0);

                vec3_t pos;
                pos.x = sprite[i].x;
                pos.y = sprite[i].y;
                pos.z = sprite[i].z;

                setsprite(i, &pos);
                if (movestat != 0)
                    sprite[i].ang = rand() & kAngleMask;
            }
            break;
            case 2: // fly to roof and get deleted
            if (sprite[i].lotag < 0)
            {
                if (i == lastbat && batsnd != -1)
                {
                    SND_StopLoop(batsnd);
                    batsnd = -1;
                }
                deletesprite(i);
            }
            else
            {
                movestat = movesprite(i, (Cos(sprite[i].ang) * synctics) << 3, (Sin(sprite[i].ang) * synctics) << 3, 0, 4 << 8, 4 << 8, 0);

                vec3_t pos;
                pos.x = sprite[i].x;
                pos.y = sprite[i].y;
                pos.z = sprite[i].z;

                setsprite(i, &pos);
                if ((movestat & 0xc000) == 16384)
                {
                    //Hits a ceiling / floor
                    if (i == lastbat && batsnd != -1)
                    {
                        SND_StopLoop(batsnd);
                        batsnd = -1;
                    }
                    deletesprite(i);
                }
                if (movestat != 0)
                    sprite[i].ang = (rand() & kAngleMask);
            }
            break;
        }
        i = nexti;
    }

    // CHILL
    i = headspritestat[CHILL];
    while (i != -1)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;
        if (sprite[i].lotag < 0)
        {
            sprite[i].picnum++;
            sprite[i].lotag = 18;
            if (sprite[i].picnum == GOBLINSURPRISE + 5)
            {
                sprite[i].picnum = GOBLIN;
                newstatus(i, FACE);
            }
            if (sprite[i].picnum == HANGMAN + 10)
            {
                sprite[i].picnum = SKELETON;
                newstatus(i, FACE);
            }
        }
        i = nexti;
    }

    // SKIRMISH
    i = headspritestat[SKIRMISH];
    while (i != -1)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;

        if (sprite[i].lotag < 0)
        {
            newstatus(i, FACE);
        }

        sprite[i].z = sector[sprite[i].sectnum].floorz;

        movestat = movesprite(i, (Cos(sprite[i].ang) * synctics) << 3, (Sin(sprite[i].ang) * synctics) << 3, 0, 4 << 8, 4 << 8, 2);

        if (movestat != 0)
        {
            sprite[i].ang = (getangle(plr->x - sprite[i].x, plr->y - sprite[i].y) & kAngleMask);
            newstatus(i, FACE);
        }

        if ((sprite[i].sectnum != osectnum) && (sector[sprite[i].sectnum].lotag == 10))
            warpsprite(i);

        if (sector[sprite[i].sectnum].floorpicnum == WATER
            || sector[sprite[i].sectnum].floorpicnum == LAVA
            || sector[sprite[i].sectnum].floorpicnum == SLIME)
        {
            if (sector[sprite[i].sectnum].floorpicnum == WATER || sector[sprite[i].sectnum].floorpicnum == SLIME)
            {
                if (sprite[i].picnum == FISH)
                    sprite[i].z = sector[sprite[i].sectnum].floorz;
                else
                {
                    if (rand() % 100 > 60)
                        makemonstersplash(SPLASHAROO, i);
                }
            }
            else
            {
                trailingsmoke(i);
                makemonstersplash(LAVASPLASH, i);
            }
        }

        if (sector[osectnum].floorpicnum == SLIME
            || sector[osectnum].floorpicnum == WATER
            || sector[sprite[i].sectnum].floorpicnum == SLIME)
            sprite[i].z += tilesiz[sprite[i].picnum].y << 5;

        vec3_t pos;
        pos.x = sprite[i].x;
        pos.y = sprite[i].y;
        pos.z = sprite[i].z;

        setsprite(i, &pos);

        if (sector[sprite[i].sectnum].floorz - (32 << 8) < sector[sprite[i].sectnum].ceilingz)
        {
            if (sector[sprite[i].sectnum].lotag == 6)
                newstatus(i, DIE);
            else
            {
                deletesprite(i);
            }
        }
        j = headspritesect[sprite[i].sectnum];
        while (j != -1)
        {
            nextj = nextspritesect[j];
            dx = labs(sprite[i].x - sprite[j].x);            // x distance to sprite
            dy = labs(sprite[i].y - sprite[j].y);            // y distance to sprite
            dz = labs((sprite[i].z >> 8) - (sprite[j].z >> 8));  // z distance to sprite
            dh = tilesiz[sprite[j].picnum].y >> 1;       // height of sprite

            if (dx + dy < PICKDISTANCE && dz - dh <= PICKHEIGHT)
            {
                switch (sprite[i].picnum)
                {
                    case SKELETON:
                    case KOBOLD:
                    case FRED:
                    case GOBLIN:
                    case SPIDER:
                    switch (sprite[j].picnum)
                    {
                        case EXPLO2:
                        case SMOKEFX:
                        case MONSTERBALL:
                        sprite[i].hitag -= synctics << 2;
                        if (sprite[i].hitag < 0)
                        {
                            newstatus(i, DIE);
                        }
                        break;
                    }
                    break;
                    case GRONHAL:
                    case GRONMU:
                    case GRONSW:
                    switch (sprite[j].picnum)
                    {
                        case EXPLO2:
                        case MONSTERBALL:
                        sprite[i].hitag -= synctics << 2;
                        if (sprite[i].hitag < 0)
                        {
                            newstatus(i, DIE);
                        }
                        break;
                    }
                    break;
                }
            }
            j = nextj;
        }

        i = nexti;
    }

    // FINDME
    i = headspritestat[FINDME];
    while (i != -1)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;

        if (sprite[i].picnum == RAT)
        {
            sprite[i].ang = (((rand() & 512) - 256) + sprite[i].ang + 1024) & kAngleMask;  // NEW
            newstatus(i, FLEE);
            goto findout;
        }
        if (plr->invisibletime > 0)
        {
            newstatus(i, FACE);
            break;
        }

        monsternoise(i);

        sprite[i].z = sector[sprite[i].sectnum].floorz;

        movestat = movesprite(i, (Cos(sprite[i].ang) * synctics) << 3, (Sin(sprite[i].ang) * synctics) << 3, 0, 4 << 8, 4 << 8, 2);

        if (checkdist(i, plr->x, plr->y, plr->z))
        {
            if (plr->shadowtime > 0)
            {
                sprite[i].ang = ((rand() & 512 - 256) + sprite[i].ang + 1024) & kAngleMask;  // NEW
                newstatus(i, FLEE);
            }
            else
                newstatus(i, ATTACK);
            break;
        }

        if (movestat != 0)
        {
            if (cansee(plr->x, plr->y, plr->z, plr->sector, sprite[i].x, sprite[i].y, sprite[i].z - (tilesiz[sprite[i].picnum].y << 7), sprite[i].sectnum) == 1 && sprite[i].lotag < 0)
            {
                sprite[i].ang = (sprite[i].ang + 1024) & kAngleMask;
                newstatus(i, FLEE);
                goto findout;
            }
            if (sprite[i].lotag < 0)
            {
                if (rand() % 100 > 50)
                    sprite[i].ang = (sprite[i].ang + 512) & kAngleMask;
                else
                    sprite[i].ang = (sprite[i].ang + 1024) & kAngleMask;

                sprite[i].lotag = 30;
            }
            else
            {
                sprite[i].ang += (synctics << 4) & kAngleMask;
            }
        }

        if (cansee(plr->x, plr->y, plr->z, plr->sector, sprite[i].x, sprite[i].y, sprite[i].z - (tilesiz[sprite[i].picnum].y << 7), sprite[i].sectnum) == 1 && movestat == 0 && sprite[i].lotag < 0)
        {
            newstatus(i, FACE);
            break;
        }

        if ((sprite[i].sectnum != osectnum) && (sector[sprite[i].sectnum].lotag == 10))
            warpsprite(i);

        if (sector[sprite[i].sectnum].floorpicnum == WATER
            || sector[sprite[i].sectnum].floorpicnum == LAVA
            || sector[sprite[i].sectnum].floorpicnum == SLIME)
        {
            if (sector[sprite[i].sectnum].floorpicnum == WATER || sector[sprite[i].sectnum].floorpicnum == SLIME)
            {
                if (sprite[i].picnum == FISH)
                    sprite[i].z = sector[sprite[i].sectnum].floorz;
                else
                {
                    if (rand() % 100 > 60)
                        makemonstersplash(SPLASHAROO, i);
                }
            }
            else
            {
                trailingsmoke(i);
                makemonstersplash(LAVASPLASH, i);
            }
        }

        if (sector[osectnum].floorpicnum == SLIME
            || sector[osectnum].floorpicnum == WATER
            || sector[sprite[i].sectnum].floorpicnum == SLIME)
        {
            sprite[i].z += tilesiz[sprite[i].picnum].y << 5;
        }

        vec3_t pos;
        pos.x = sprite[i].x;
        pos.y = sprite[i].y;
        pos.z = sprite[i].z;

        setsprite(i, &pos);

        if (sector[sprite[i].sectnum].floorz - (32 << 8) < sector[sprite[i].sectnum].ceilingz)
        {
            if (sector[sprite[i].sectnum].lotag == 6)
                newstatus(i, DIE);
            else
            {
                deletesprite(i);
            }
        }

        j = headspritesect[sprite[i].sectnum];
        while (j != -1)
        {
            nextj = nextspritesect[j];
            dx = labs(sprite[i].x - sprite[j].x);            // x distance to sprite
            dy = labs(sprite[i].y - sprite[j].y);            // y distance to sprite
            dz = labs((sprite[i].z >> 8) - (sprite[j].z >> 8));  // z distance to sprite
            dh = tilesiz[sprite[j].picnum].y >> 1;       // height of sprite

            if (dx + dy < PICKDISTANCE && dz - dh <= PICKHEIGHT)
            {
                switch (sprite[i].picnum)
                {
                    case SKELETON:
                    case KOBOLD:
                    case FRED:
                    case GOBLIN:
                    case SPIDER:
                    switch (sprite[j].picnum)
                    {
                        case EXPLO2:
                        case SMOKEFX:
                        case MONSTERBALL:
                        sprite[i].hitag -= synctics << 2;
                        if (sprite[i].hitag < 0)
                        {
                            newstatus(i, DIE);
                        }
                        break;
                    }
                    break;
                    case GRONHAL:
                    case GRONMU:
                    case GRONSW:
                    switch (sprite[j].picnum)
                    {
                        case EXPLO2:
                        case MONSTERBALL:
                        sprite[i].hitag -= synctics << 2;
                        if (sprite[i].hitag < 0)
                        {
                            newstatus(i, DIE);
                        }
                        break;
                    }
                    break;
                }
            }
            j = nextj;
        }

    findout:
        i = nexti;
    }

    // TORCHLIGHT
    i = headspritestat[TORCHLIGHT];

    while (i != -1)
    {
        nexti = nextspritestat[i];
        osectnum = sprite[i].sectnum;
        j = (torchpattern[lockclock % 38]);
        sector[osectnum].ceilingshade = j;
        sector[osectnum].floorshade = j;
        startwall = sector[osectnum].wallptr;
        endwall = startwall + sector[osectnum].wallnum - 1;
        for (k = startwall; k <= endwall; k++)
            wall[k].shade = j;
        i = nexti;
    }

    // GLOWLIGHT
    i = headspritestat[GLOWLIGHT];
    while (i != -1)
    {
        nexti = nextspritestat[i];
        osectnum = sprite[i].sectnum;
        j = (torchpattern[lockclock % 38]);
        sector[osectnum].floorshade = j;
        startwall = sector[osectnum].wallptr;
        endwall = startwall + sector[osectnum].wallnum - 1;
        for (k = startwall; k <= endwall; k++)
            wall[k].shade = j;
        startredflash(j);
        i = nexti;
    }

    // BOB
    i = headspritestat[BOB];
    while (i != -1)
    {
        nexti = nextspritestat[i];
        sprite[i].z += Sin(lockclock << 4) >> 6;
        i = nexti;
    }

    // RESURECT
    i = headspritestat[RESURECT];

    while (i != -1)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;

        if (sprite[i].lotag <= 0)
        {
            newstatus(i, FACE);

            switch (sprite[i].picnum)
            {
                case GRONDEAD:
                j = krand() % 3;
                switch (j)
                {
                    case 0:
                    sprite[i].picnum = GRONHAL;
                    sprite[i].hitag = adjusthp(120);
                    sprite[i].extra = 3;
                    break;
                    case 1:
                    sprite[i].picnum = GRONSW;
                    sprite[i].hitag = adjusthp(120);
                    sprite[i].extra = 0;
                    break;
                    case 2:
                    sprite[i].picnum = GRONMU;
                    sprite[i].hitag = adjusthp(120);
                    sprite[i].extra = 2;
                    break;
                }
                break;
                case KOBOLDDEAD:
                sprite[i].picnum = KOBOLD;
                sprite[i].hitag = adjusthp(30);
                break;
                case DRAGONDEAD:
                sprite[i].picnum = DRAGON;
                sprite[i].hitag = adjusthp(900);
                break;
                case DEVILDEAD:
                sprite[i].picnum = DEVIL;
                sprite[i].hitag = adjusthp(60);
                break;
                case FREDDEAD:
                sprite[i].picnum = FRED;
                sprite[i].hitag = adjusthp(40);
                break;
                case SKELETONDEAD:
                sprite[i].picnum = SKELETON;
                sprite[i].hitag = adjusthp(10);
                break;
                case GOBLINDEAD:
                sprite[i].picnum = GOBLIN;
                sprite[i].hitag = adjusthp(35);
                break;
                case MINOTAURDEAD:
                sprite[i].picnum = MINOTAUR;
                sprite[i].hitag = adjusthp(100);
                break;
                case SPIDERDEAD:
                sprite[i].picnum = SPIDER;
                sprite[i].hitag = adjusthp(15);
                break;
                case SKULLYDEAD:
                sprite[i].picnum = SKULLY;
                sprite[i].hitag = adjusthp(100);
                break;
                case FATWITCHDEAD:
                sprite[i].picnum = FATWITCH;
                sprite[i].hitag = adjusthp(90);
                break;
                case JUDYDEAD:
                sprite[i].picnum = JUDY;
                sprite[i].hitag = adjusthp(200);
                break;
            }
            sprite[i].lotag = 100;
            sprite[i].cstat |= 1;
        }
        i = nexti;
    }

    // LIFT UP
    i = headspritestat[LIFTUP];

    while (i != -1)
    {
        nexti = nextspritestat[i];

        switch (sprite[i].lotag)
        {
            case 1821:
            sprite[i].z -= (synctics << 6);

            vec3_t pos;
            pos.x = sprite[i].x;
            pos.y = sprite[i].y;
            pos.z = sprite[i].z;

            setsprite(i, &pos);
            if (sprite[i].z <= sector[sprite[i].sectnum].ceilingz + 32768)
            {

                SND_StopLoop(cartsnd);
                cartsnd = -1;
                playsound_loc(S_CLUNK, sprite[i].x, sprite[i].y);

                changespritestat(i, 0);
                sprite[i].lotag = 1820;
                sprite[i].z = sector[sprite[i].sectnum].ceilingz + 32768;
            }
            break;
            case 1811:
            sprite[i].z -= (synctics << 6);

            //vec3_t pos;
            pos.x = sprite[i].x;
            pos.y = sprite[i].y;
            pos.z = sprite[i].z;

            setsprite(i, &pos);
            if (sprite[i].z <= sector[sprite[i].sectnum].ceilingz + 65536)
            {
                changespritestat(i, 0);
                sprite[i].lotag = 1810;
                sprite[i].z = sector[sprite[i].sectnum].ceilingz + 65536;
            }
            break;
            case 1801:
            sprite[i].z -= (synctics << 6);

            //vec3_t pos;
            pos.x = sprite[i].x;
            pos.y = sprite[i].y;
            pos.z = sprite[i].z;

            setsprite(i, &pos);
            if (sprite[i].z <= sector[sprite[i].sectnum].ceilingz + 65536)
            {
                changespritestat(i, 0);
                sprite[i].lotag = 1800;
                sprite[i].z = sector[sprite[i].sectnum].ceilingz + 65536;
            }
            break;
        }
        i = nexti;
    }

    // LIFT DN
    i = headspritestat[LIFTDN];

    while (i != -1)
    {
        nexti = nextspritestat[i];
        ironbarmove = 0;

        switch (sprite[i].lotag)
        {
            case 1820:

            sprite[i].z += ironbarmove = synctics << 6;

            vec3_t pos;
            pos.x = sprite[i].x;
            pos.y = sprite[i].y;
            pos.z = sprite[i].z;

            setsprite(i, &pos);
            if (sprite[i].z >= (sector[sprite[i].sectnum].floorz - 32768))
            {
                SND_StopLoop(cartsnd);
                cartsnd = -1;
                playsound_loc(S_CLUNK, sprite[i].x, sprite[i].y);
                changespritestat(i, 0);
                sprite[i].lotag = 1821;
                sprite[i].z = sector[sprite[i].sectnum].floorz - 32768;
            }
            break;
            case 1810:
            sprite[i].z += ironbarmove = synctics << 6;

            //vec3_t pos;
            pos.x = sprite[i].x;
            pos.y = sprite[i].y;
            pos.z = sprite[i].z;

            setsprite(i, &pos);
            if (sprite[i].z >= sector[sprite[i].sectnum].floorz)
            {
                changespritestat(i, 0);
                sprite[i].lotag = 1811;
                sprite[i].z = sector[sprite[i].sectnum].floorz;
            }
            break;
            case 1800:
            sprite[i].z += ironbarmove = synctics << 6;
            setsprite(i, &pos);
            if (sprite[i].z >= sector[sprite[i].sectnum].floorz)
            {
                changespritestat(i, 0);
                sprite[i].lotag = 1801;
                sprite[i].z = sector[sprite[i].sectnum].floorz;
            }
            break;
        }
        i = nexti;
    }

    // WITCHSIT
    i = headspritestat[WITCHSIT];
    while (i >= 0)
    {
        nexti = nextspritestat[i];
        sprite[i].ang = (getangle(plr->x - sprite[i].x, plr->y - sprite[i].y) & kAngleMask);
        if (cansee(plr->x, plr->y, plr->z, plr->sector, sprite[i].x, sprite[i].y, sprite[i].z - (tilesiz[sprite[i].picnum].y << 7), sprite[i].sectnum) == 1)
        {
            sprite[i].lotag -= synctics;
            if (sprite[i].lotag < 0)
            {
                sprite[i].picnum++;
                sprite[i].lotag = 12;
                if (sprite[i].picnum == JUDYSIT + 4)
                {
                    sprite[i].picnum = JUDY;
                    newstatus(i, FACE);
                }
            }
        }
        i = nexti;
    }

    // FACE
    i = headspritestat[FACE];

    while (i >= 0)
    {
        nexti = nextspritestat[i];

        sprite[i].ang = (getangle(plr->x - sprite[i].x, plr->y - sprite[i].y) & kAngleMask);

        if (cansee(plr->x, plr->y, plr->z, plr->sector,
            sprite[i].x, sprite[i].y, sprite[i].z - (tilesiz[sprite[i].picnum].y << 7), sprite[i].sectnum) == 1
            && plr->invisibletime < 0)
        {
            if (plr->shadowtime > 0)
            {
                sprite[i].ang = (((krand() & 512) - 256) + sprite[i].ang + 1024) & kAngleMask;  // NEW
                newstatus(i, FLEE);
            }
            else
            {
                sprite[i].owner = plr->spritenum;
                if (sprite[i].picnum == RAT)
                {
                    sprite[i].ang = (((krand() & 512) - 256) + sprite[i].ang + 1024) & kAngleMask;  // NEW
                    newstatus(i, FLEE);
                }
                else
                    newstatus(i, CHASE);
            }
        }
        else
        { // get off the wall
            if (sprite[i].owner == plr->spritenum)
            {
                sprite[i].ang = (((krand() & 512) - 256) + sprite[i].ang) & kAngleMask;  // NEW
                newstatus(i, FINDME);
            }
        }

        if (checkdist(i, plr->x, plr->y, plr->z))
        {
            if (sprite[i].picnum == RAT)
            {
                sprite[i].ang = (((krand() & 512) - 256) + sprite[i].ang + 1024) & kAngleMask;  // NEW
                newstatus(i, FLEE);
            }
            else
                newstatus(i, ATTACK);
        }

        j = headspritesect[sprite[i].sectnum];
        while (j != -1)
        {
            nextj = nextspritesect[j];
            dx = labs(sprite[i].x - sprite[j].x);            // x distance to sprite
            dy = labs(sprite[i].y - sprite[j].y);            // y distance to sprite
            dz = labs((sprite[i].z >> 8) - (sprite[j].z >> 8));  // z distance to sprite
            dh = tilesiz[sprite[j].picnum].y >> 1;       // height of sprite

            if (dx + dy < PICKDISTANCE && dz - dh <= PICKHEIGHT)
            {
                switch (sprite[i].picnum)
                {
                    case SKELETON:
                    case KOBOLD:
                    case FRED:
                    case GOBLIN:
                    case SPIDER:
                    switch (sprite[j].picnum)
                    {
                        case EXPLO2:
                        case SMOKEFX:
                        case MONSTERBALL:
                        sprite[i].hitag -= synctics << 2;
                        if (sprite[i].hitag < 0)
                        {
                            newstatus(i, DIE);
                        }
                        break;
                    }
                    break;
                    case GRONHAL:
                    case GRONMU:
                    case GRONSW:
                    switch (sprite[j].picnum)
                    {
                        case EXPLO2:
                        case MONSTERBALL:
                        sprite[i].hitag -= synctics << 2;
                        if (sprite[i].hitag < 0)
                        {
                            newstatus(i, DIE);
                        }
                        break;
                    }
                    break;
                }
            }
            j = nextj;
        }
        i = nexti;
    }

    // MASPLASH
    i = headspritestat[MASPLASH];
    while (i >= 0)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;
        sprite[i].z = sector[sprite[i].sectnum].floorz + (tilesiz[sprite[i].picnum].y << 8);

        vec3_t pos;
        pos.x = sprite[i].x;
        pos.y = sprite[i].y;
        pos.z = sprite[i].z;

        setsprite(i, &pos);

        if (sprite[i].lotag < 0)
        {
            if (sprite[i].picnum == LASTSPLASHAROO || sprite[i].picnum == LASTLAVASPLASH)
                deletesprite(i);

            sprite[i].picnum++;
            sprite[i].lotag = 8;
        }
        i = nexti;
    }

    // ATTACK2
    i = headspritestat[ATTACK2];
    while (i >= 0)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;
        if (sprite[i].lotag < 0)
        {
            if (cansee(plr->x, plr->y, plr->z, plr->sector,
                sprite[i].x, sprite[i].y, sprite[i].z - (tilesiz[sprite[i].picnum].y << 7), sprite[i].sectnum) == 1
                && plr->invisibletime < 0)
                newstatus(i, CAST);
            else
                newstatus(i, CHASE);
            break;
        }
        else
            sprite[i].ang = (getangle(plr->x - sprite[i].x, plr->y - sprite[i].y) & kAngleMask);

        if (sector[sprite[i].sectnum].floorz - (32 << 8) < sector[sprite[i].sectnum].ceilingz)
        {
            if (sector[sprite[i].sectnum].lotag == 6)
                newstatus(i, DIE);
            else
            {
                deletesprite(i);
            }
        }

        i = nexti;
    }

    // ATTACK SEQUENCE
    i = headspritestat[ATTACK];
    while (i >= 0)
    {
        nexti = nextspritestat[i];

        if (netgame)
        {
            i = nexti;
            continue;
        }

        switch (sprite[i].picnum)
        {
            case JUDYATTACK1:
            case JUDYATTACK2:
            sprite[i].extra -= synctics;
            fallthrough__;
            case GRONHALATTACK:
            case GRONMUATTACK:
            case DRAGONATTACK2:
            case DRAGONATTACK:
            case DEVILATTACK:
            case GUARDIANATTACK:
            case SKULLYATTACK:
            case FATWITCHATTACK:
            sprite[i].lotag -= synctics;
            if (sprite[i].lotag < 0)
            {
                if (cansee(plr->x, plr->y, plr->z, plr->sector,
                    sprite[i].x, sprite[i].y, sprite[i].z - (tilesiz[sprite[i].picnum].y << 7), sprite[i].sectnum) == 1)
                    //&& invisibletime < 0)
                    newstatus(i, CAST);
                else
                    newstatus(i, CHASE);
                break;
            }
            else
                sprite[i].ang = (getangle(plr->x - sprite[i].x, plr->y - sprite[i].y) & kAngleMask);
            break;
            case WILLOW:
            sprite[i].lotag -= synctics;
            if (sprite[i].lotag < 0)
            {
                if (cansee(plr->x, plr->y, plr->z, plr->sector,
                    sprite[i].x, sprite[i].y, sprite[i].z - (tilesiz[sprite[i].picnum].y << 7), sprite[i].sectnum) == 1)
                    if (checkdist(i, plr->x, plr->y, plr->z))
                    {
                        if (plr->shockme < 0)
                        {
                            if ((krand() & 100) > 95)
                            {
                                plr->shockme = 120;
                                plr->lvl--;
                                switch (plr->lvl)
                                {
                                    case 1:
                                    plr->score = 0;
                                    plr->maxhealth = 100;
                                    break;
                                    case 2:
                                    plr->score = 2350;
                                    plr->maxhealth = 120;
                                    break;
                                    case 3:
                                    plr->score = 4550;
                                    plr->maxhealth = 140;
                                    break;
                                    case 4:
                                    plr->score = 9300;
                                    plr->maxhealth = 160;
                                    break;
                                    case 5:
                                    plr->score = 18400;
                                    plr->maxhealth = 180;
                                    break;
                                    case 6:
                                    plr->score = 36700;
                                    plr->maxhealth = 200;
                                    break;
                                    case 7:
                                    plr->score = 75400;
                                    plr->maxhealth = 200;
                                    break;
                                }
                                if (plr->lvl < 1)
                                {
                                    plr->lvl = 1;
                                    plr->health = -1;
                                }
                                StatusMessage(360, "Level Drained");
                                //updatepics();
                            }
                        }
                    }
                    else
                        newstatus(i, DRAIN);
                else
                    newstatus(i, CHASE);
                break;
            }
            else
                sprite[i].ang = (getangle(plr->x - sprite[i].x, plr->y - sprite[i].y) & kAngleMask);
            break;
            case FISH:
            sprite[i].z = sector[sprite[i].sectnum].floorz;
            fallthrough__;
            case KOBOLDATTACK:
            case FREDATTACK:
            case SKELETONATTACK:
            case GOBLINATTACK:
            case MINOTAURATTACK:
            case SPIDER:
            case GRONSWATTACK:
            sprite[i].z = sector[sprite[i].sectnum].floorz;
            if (sector[sprite[i].sectnum].floorpicnum == WATER
                || sector[sprite[i].sectnum].floorpicnum == SLIME)
            {
                sprite[i].z += tilesiz[sprite[i].picnum].y << 3;

                vec3_t pos;
                pos.x = sprite[i].x;
                pos.y = sprite[i].y;
                pos.z = sprite[i].z;

                setsprite(i, &pos);
            }
            else
            {
                //sprite[i].z=sector[sprite[i].sectnum].floorz;

                vec3_t pos;
                pos.x = sprite[i].x;
                pos.y = sprite[i].y;
                pos.z = sprite[i].z;

                setsprite(i, &pos);
            }

            if (sprite[i].lotag >= 64)
            {
                if (checksight(i, &daang))
                {
                    if (checkdist(i, plr->x, plr->y, plr->z))
                    {
                        sprite[i].ang = daang;
                        dax = 0;
                        day = 0;
                        attack(i);

                        if (sprite[i].picnum == SPIDER)
                        {
                            if (krand() % 100 > ((plr->lvl * 7) + 20))
                            {
                                playsound_loc(S_SPIDERBITE, sprite[i].x, sprite[i].y);
                                plr->poisoned = 1;
                                plr->poisontime = 7200;
                                StatusMessage(360, "Poisoned");
                                newstatus(i, DIE);
                                goto outaattack;
                            }
                        }
                    }
                }
            }
            else if (sprite[i].lotag < 0)
            {
                if (plr->shadowtime > 0)
                {
                    sprite[i].ang = (((krand() & 512) - 256) + sprite[i].ang + 1024) & kAngleMask;  // NEW
                    newstatus(i, FLEE);
                }
                else
                    newstatus(i, CHASE);
            }
            sprite[i].lotag -= synctics;
            break;
        }

        if (sector[sprite[i].sectnum].floorz - (32 << 8) < sector[sprite[i].sectnum].ceilingz)
        {
            if (sector[sprite[i].sectnum].lotag == 6)
                newstatus(i, DIE);
            else
            {
                deletesprite(i);
            }
        }

    outaattack:
        i = nexti;
    }

    // SHATTER
    i = headspritestat[SHATTER];

    while (i != -1)
    {
        nexti = nextspritestat[i];

        sprite[i].lotag -= synctics;

        if (sprite[i].lotag < 0)
        {
            sprite[i].picnum++;
            sprite[i].lotag = 12;
        }
        switch (sprite[i].picnum)
        {
            case FSHATTERBARREL + 2:
            changespritestat(i, 0);
            break;
        }

        i = nexti;
    }

    // FIRE
    i = headspritestat[FIRE];

    while (i != -1)
    {

        nexti = nextspritestat[i];

        sprite[i].lotag -= synctics;

        if (sprite[i].z < sector[sprite[i].sectnum].floorz)
            sprite[i].z += (int)synctics << 8;
        if (sprite[i].z > sector[sprite[i].sectnum].floorz)
            sprite[i].z = sector[sprite[i].sectnum].floorz;


        if (sprite[i].lotag < 0)
        {
            switch (sprite[i].picnum)
            {
                case LFIRE:
                sprite[i].picnum = SFIRE;
                sprite[i].lotag = 2047;
                break;
                case SFIRE:
                deletesprite(i);

                break;
            }
        }

        if (checkdist(i, plr->x, plr->y, plr->z))
        {
            sethealth(-1);
            flashflag = 1;
            startredflash(10);
        }

        i = nexti;
    }

    // FALL
    i = headspritestat[FALL];

    while (i != -1)
    {
        nexti = nextspritestat[i];

        if (sprite[i].z < sector[sprite[i].sectnum].floorz)
            daz = sprite[i].zvel += (synctics << 9);

        hitobject = movesprite(i, (Cos(sprite[i].ang) * synctics) << 3, (Sin(sprite[i].ang) * synctics) << 3,
            daz, 4 << 8, 4 << 8, 0);

        vec3_t pos;
        pos.x = sprite[i].x;
        pos.y = sprite[i].y;
        pos.z = sprite[i].z;

        setsprite(i, &pos);

        if (sprite[i].picnum == FBARRELFALL || (sprite[i].picnum >= BOULDER && sprite[i].picnum <= BOULDER + 3) &&
            (checkdist(i, plr->x, plr->y, plr->z)))
        {
            sethealth(-50);
            startredflash(50);
        }

        if ((hitobject & 0xc0000) == 16384)
        {
            if (sector[sprite[i].sectnum].floorpicnum == WATER)
            {
                makemonstersplash(SPLASHAROO, i);
            }
            switch (sprite[i].picnum)
            {
                case FBARRELFALL:
                newstatus(i, SHATTER);
                sprite[i].lotag = 12;
                break;
                case TORCH:
                for (k = 0; k < 16; k++)
                    makeafire(i, 0);
                deletesprite(i);
                break;
                default:
                changespritestat(i, 0);
                break;
            }
            sprite[i].hitag = 0;
        }

        i = nexti;
    }

//  SHOVE
    i = headspritestat[SHOVE];

    while (i != -1)
    {
        nexti = nextspritestat[i];

        if (sprite[i].z < sector[sprite[i].sectnum].floorz)
            daz = sprite[i].zvel += (synctics << 5);

        hitobject = movesprite(i, (Cos(sprite[i].ang) * synctics) << 3, (Sin(sprite[i].ang) * synctics) << 3,
            daz, 4L << 8, 4L << 8, 0);

        vec3_t pos;
        pos.x = sprite[i].x;
        pos.y = sprite[i].y;
        pos.z = sprite[i].z;

        setsprite(i, &pos);

        if (sprite[i].z >= sector[sprite[i].sectnum].floorz)
        {
            if (sector[sprite[i].sectnum].floorpicnum == WATER)
            {
                makemonstersplash(SPLASHAROO, i);
            }
            newstatus(i, BROKENVASE);
            goto outashove;
        }

        if ((hitobject & 0xc000) == 16384)
        {
            newstatus(i, BROKENVASE);
            goto outashove;
        }

        if ((hitobject & 0xc000) == 49152) //Bullet hit a sprite
        {
            if (sprite[i].owner != hitobject)
            {
                hitdamage = damageactor(hitobject, i);
                if (hitdamage)
                {
                    newstatus(i, BROKENVASE);
                    goto outashove;
                }
            }

        }

    outashove:
        i = nexti;
    }

    // PUSH
    i = headspritestat[PUSH];

    while (i != -1)
    {
        nexti = nextspritestat[i];

        sprite[i].lotag -= synctics;

        osectnum = sprite[i].sectnum;

        switch (sprite[i].picnum)
        {
            case BOULDER:
            case BARREL:
            speed = 9;
            break;
        }

        if (sprite[i].z < sector[sprite[i].sectnum].floorz)
        {
            daz = sprite[i].zvel += (synctics << 1);
        }
        // clip type was 1
        hitobject = movesprite(i, (Cos(sprite[i].ang) * synctics) << 3, (Sin(sprite[i].ang) * synctics) << 3, daz, 4 << 8, 4 << 8, 0);

        vec3_t pos;
        pos.x = sprite[i].x;
        pos.y = sprite[i].y;
        pos.z = sprite[i].z;

        setsprite(i, &pos);

        if (sprite[i].lotag < 0 || (hitobject & 0xc000) == 32768)
        {
            sprite[i].lotag = 0;
            changespritestat(i, 0);
            if (sprite[i].z < sector[sprite[i].sectnum].floorz)
            {
                sprite[i].zvel += 256;
                changespritestat(i, FALL);
            }
        }

        i = nexti;
    }

    // DORMANT
    i = headspritestat[DORMANT];
    while (i != -1)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;
        sprite[i].xrepeat = sprite[i].yrepeat = 2;
        if (sprite[i].lotag < 0)
        {
            newstatus(i, ACTIVE);
        }

        i = nexti;
    }

    // ACTIVE
    i = headspritestat[ACTIVE];
    while (i != -1)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;
        sprite[i].xrepeat = 48;
        sprite[i].yrepeat = 32;
        if (sprite[i].lotag < 0)
        {
            newstatus(i, DORMANT);
        }
        i = nexti;
    }

    // FLEE
    i = headspritestat[FLEE];
    while (i != -1)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;
        osectnum = sprite[i].sectnum;

        switch (sprite[i].picnum)
        {
            case GRONHAL:
            case GRONHALATTACK:
            case GRONMU:
            case GRONMUATTACK:
            case GRONSW:
            case GRONSWATTACK:
            case DRAGON:
            case DRAGONATTACK:
            case DRAGONATTACK2:
            case DEVIL:
            case DEVILATTACK:
            case KOBOLD:
            case KOBOLDATTACK:
            case MINOTAUR:
            case MINOTAURATTACK:
            case SKELETON:
            case SKELETONATTACK:
            case GOBLIN:
            case GOBLINATTACK:
            case FRED:
            case FREDATTACK:
            case GUARDIAN:
            case GUARDIANATTACK:
            case SKULLY:
            case SKULLYATTACK:
            case FATWITCH:
            case FATWITCHATTACK:
            case JUDY:
            case JUDYATTACK1:
            case JUDYATTACK2:
            case SPIDER:
            case RAT:
            case WILLOW:
            if (sprite[i].picnum == WILLOW
                || sprite[i].picnum == GUARDIAN
                || (sprite[i].picnum >= GUARDIANATTACK
                    && sprite[i].picnum <= GUARDIANATTACK + 6))
                sprite[i].z = sector[sprite[i].sectnum].floorz - (32 << 8);
            else
                sprite[i].z = sector[sprite[i].sectnum].floorz;

            if (sprite[i].picnum == GUARDIAN)
            {
                if (totalclock % 100 > 70)
                    trailingsmoke(i);
            }

            movestat = movesprite(i, (Cos(sprite[i].ang) * synctics) << 3, (Sin(sprite[i].ang) * synctics) << 3, 0, 4 << 8, 4 << 8, 2);

            if (movestat != 0)
            {
                sprite[i].ang = (getangle(plr->x - sprite[i].x, plr->y - sprite[i].y) & kAngleMask);
                newstatus(i, FACE);
            }
            if (sprite[i].lotag < 0)
                newstatus(i, FACE);
            break;
            default:
            break;
        }
        if ((sprite[i].sectnum != osectnum)
            && (sector[sprite[i].sectnum].lotag == 10))
            warpsprite(i);

        if (sector[sprite[i].sectnum].floorz - (32 << 8) < sector[sprite[i].sectnum].ceilingz)
        {
            if (sector[sprite[i].sectnum].lotag == 6)
                newstatus(i, DIE);
            else
            {
                deletesprite(i);
            }
        }

        if (sector[sprite[i].sectnum].floorpicnum == WATER
            || sector[sprite[i].sectnum].floorpicnum == LAVA
            || sector[sprite[i].sectnum].floorpicnum == SLIME)
        {

            if (sector[sprite[i].sectnum].floorpicnum == WATER || sector[sprite[i].sectnum].floorpicnum == SLIME)
            {
                if (rand() % 100 > 60)
                    makemonstersplash(SPLASHAROO, i);
                if (sprite[i].picnum == RAT)
                {
                    sprite[i].hitag--;
                    if (sprite[i].hitag < 0)
                        newstatus(i, DIE);
                }
            }
            else
            {
                trailingsmoke(i);
                makemonstersplash(LAVASPLASH, i);

                if (sprite[i].picnum == RAT)
                {
                    sprite[i].hitag--;
                    if (sprite[i].hitag < 0)
                        newstatus(i, DIE);
                }

            }
        }

        if (sector[osectnum].floorpicnum == SLIME || sector[osectnum].floorpicnum == WATER)
        {

            if (sprite[i].picnum == WILLOW
                || sprite[i].picnum == GUARDIAN
                || (sprite[i].picnum >= GUARDIANATTACK
                    && sprite[i].picnum <= GUARDIANATTACK + 6))
                sprite[i].z = sector[sprite[i].sectnum].floorz - (32 << 8);
            else
                sprite[i].z += tilesiz[sprite[i].picnum].y << 5;
        }

        vec3_t pos;
        pos.x = sprite[i].x;
        pos.y = sprite[i].y;
        pos.z = sprite[i].z;

        setsprite(i, &pos);

        j = headspritesect[sprite[i].sectnum];
        while (j != -1)
        {
            nextj = nextspritesect[j];
            dx = labs(sprite[i].x - sprite[j].x);            // x distance to sprite
            dy = labs(sprite[i].y - sprite[j].y);            // y distance to sprite
            dz = labs((sprite[i].z >> 8) - (sprite[j].z >> 8));  // z distance to sprite
            dh = tilesiz[sprite[j].picnum].y >> 1;       // height of sprite

            if (dx + dy < PICKDISTANCE && dz - dh <= PICKHEIGHT)
            {
                switch (sprite[i].picnum)
                {
                    case SKELETON:
                    case KOBOLD:
                    case FRED:
                    case GOBLIN:
                    case SPIDER:
                    switch (sprite[j].picnum)
                    {
                        case EXPLO2:
                        case SMOKEFX:
                        case MONSTERBALL:
                        sprite[i].hitag -= synctics << 2;
                        if (sprite[i].hitag < 0)
                        {
                            newstatus(i, DIE);
                        }
                        break;
                    }
                    break;
                    case GRONHAL:
                    case GRONHALATTACK:
                    case GRONMU:
                    case GRONMUATTACK:
                    case GRONSW:
                    case GRONSWATTACK:
                    switch (sprite[j].picnum)
                    {
                        case EXPLO2:
                        case MONSTERBALL:
                        sprite[i].hitag -= synctics << 2;
                        if (sprite[i].hitag < 0)
                        {
                            newstatus(i, DIE);
                        }
                        break;
                    }
                    break;
                }
            }
            j = nextj;
        }

        i = nexti;
    }

    // CHASE
    i = headspritestat[CHASE];
    while (i != -1)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;

        if (sprite[i].picnum == RAT)
        {
            newstatus(i, FLEE);
            goto chaseout;
        }

        if (sprite[i].lotag < 0)
            sprite[i].lotag = 250;

        osectnum = sprite[i].sectnum;

        switch (sprite[i].picnum)
        {
            case DRAGON:
            speed = 10;
            if (krand() % 16 == 0)
            {
                if ((cansee(plr->x, plr->y, plr->z, plr->sector, sprite[i].x, sprite[i].y, sprite[i].z - (tilesiz[sprite[i].picnum].y << 7), sprite[i].sectnum) == 1) && plr->invisibletime < 0)
                    if (plr->z < sprite[i].z)
                        newstatus(i, ATTACK2);
                    else
                        newstatus(i, ATTACK);
                break;
            }
            else
            {
                checkspeed(i, &dax, &day, speed);
                checksight(i, &daang);
                if (!checkdist(i, plr->x, plr->y, plr->z))
                    checkmove(i, dax, day, &movestat);
                else
                {
                    if (krand() % 8 == 0)
                    {
                        if (plr->z < sprite[i].z)
                            newstatus(i, ATTACK2);
                        else
                            newstatus(i, ATTACK);
                    }
                    else
                    {
                        newstatus(i, FACE);
                    }
                }
            }
            break;
            case GUARDIAN:
            case WILLOW:
            speed = 8;
            if (krand() % 63 == 0)
            {
                if (cansee(plr->x, plr->y, plr->z, plr->sector, sprite[i].x, sprite[i].y, sprite[i].z - (tilesiz[sprite[i].picnum].y << 7), sprite[i].sectnum) == 1 && plr->invisibletime < 0)
                    newstatus(i, ATTACK);
                break;
            }
            else
            {
                sprite[i].z = sector[sprite[i].sectnum].floorz - (32 << 8);
                dax = (sintable[(sprite[i].ang + 512) & kAngleMask] >> speed);
                day = (sintable[sprite[i].ang] >> speed);
                checksight(i, &daang);

                if (sprite[i].picnum == GUARDIAN)
                {
                    if (totalclock % 100 > 70)
                        trailingsmoke(i);
                }

                if (!checkdist(i, plr->x, plr->y, plr->z))
                    checkmove(i, dax, day, &movestat);
                else
                {
                    if (krand() % 8 == 0)
                        newstatus(i, ATTACK);
                    else
                    {
                        sprite[i].ang = (((krand() & 512) - 256) + sprite[i].ang + 1024) & kAngleMask;
                        newstatus(i, CHASE);
                    }
                }
            }
            break;
            case JUDY:
            if (mapon < 24)
            {
                sprite[i].extra -= synctics;
                if (sprite[i].extra < 0)
                {
                    for (j = 0; j < 8; j++)
                        trailingsmoke(i);
                    deletesprite(i);
                    goto chaseout;
                }
            }
            case GRONHAL:
            case GRONMU:
            case DEVIL:
            case SKULLY:
            case FATWITCH:
            speed = 8;
            if (krand() % 63 == 0)
            {
                if (cansee(plr->x, plr->y, plr->z, plr->sector, sprite[i].x, sprite[i].y, sprite[i].z - (tilesiz[sprite[i].picnum].y << 7), sprite[i].sectnum) == 1)//&& invisibletime < 0)
                    newstatus(i, ATTACK);
                break;
            }
            else
            {
                checkspeed(i, &dax, &day, speed);
                checksight(i, &daang);
                if (!checkdist(i, plr->x, plr->y, plr->z))
                    checkmove(i, dax, day, &movestat);
                else
                {
                    if (krand() % 8 == 0)
                        newstatus(i, ATTACK);
                    else
                    {
                        sprite[i].ang = (((krand() & 512) - 256) + sprite[i].ang + 1024) & kAngleMask;
                        newstatus(i, FLEE);
                    }
                }
            }
            break;
            case FISH:
            case KOBOLD:
            case FRED:
            case GOBLIN:
            case MINOTAUR:
            case SPIDER:
            case SKELETON:
            case GRONSW:
            //JOE
            monsternoise(i);
            speed = 8;

            if (cansee(plr->x, plr->y, plr->z, plr->sector, sprite[i].x, sprite[i].y, sprite[i].z - (tilesiz[sprite[i].picnum].y << 7), sprite[i].sectnum) == 1 && plr->invisibletime < 0)
            {
                if (checkdist(i, plr->x, plr->y, plr->z))
                {
                    if (plr->shadowtime > 0)
                    {
                        sprite[i].ang = (((krand() & 512) - 256) + sprite[i].ang + 1024) & kAngleMask;
                        newstatus(i, FLEE);
                    }
                    else
                        newstatus(i, ATTACK);
                    break;
                }
                else if ((krand() & 32) == 0)
                {
                    sprite[i].ang = (((krand() & 128) - 256) + sprite[i].ang + 1024) & kAngleMask;
                    newstatus(i, FLEE);
                }
                if (krand() % 63 > 60)
                {
                    sprite[i].ang = (((krand() & 128) - 256) + sprite[i].ang + 1024) & kAngleMask;
                    newstatus(i, FLEE);
                    break;
                }

                // new AI
                sprite[i].z = sector[sprite[i].sectnum].floorz;

                movestat = movesprite(i, (Cos(sprite[i].ang) * synctics) << 3, (Sin(sprite[i].ang) * synctics) << 3, 0, 4 << 8, 4 << 8, 2);

                if (movestat != 0)
                {
                    if ((movestat & 4095) != plr->spritenum)
                    {
                        if ((krand() & 2) == 0)
                        {
                            daang = (sprite[i].ang + 256) & kAngleMask;
                            sprite[i].ang = daang;
                        }
                        else
                        {
                            daang = (sprite[i].ang - 256) & kAngleMask;
                            sprite[i].ang = daang;
                        }
                        if (plr->shadowtime > 0)
                        {
                            sprite[i].ang = (((krand() & 512) - 256) + sprite[i].ang + 1024) & kAngleMask;
                            newstatus(i, FLEE);
                        }
                        else
                            newstatus(i, SKIRMISH);
                    }
                    else
                    {
                        sprite[i].ang = (((krand() & 512) - 256) + sprite[i].ang + 1024) & kAngleMask;
                        newstatus(i, SKIRMISH);
                    }
                }
                break;
            }
            else
            {
                sprite[i].ang = (((krand() & 512) - 256) + sprite[i].ang + 1024) & kAngleMask;  // NEW
                newstatus(i, FLEE);
                break;
            }
        }

        if ((sprite[i].sectnum != osectnum)
            && (sector[sprite[i].sectnum].lotag == 10))
            warpsprite(i);

        if (sector[sprite[i].sectnum].floorz - (32 << 8) < sector[sprite[i].sectnum].ceilingz)
        {
            if (sector[sprite[i].sectnum].lotag == 6)
                newstatus(i, DIE);
            else
            {
                deletesprite(i);
            }
        }

        if (sector[sprite[i].sectnum].floorpicnum == WATER
            || sector[sprite[i].sectnum].floorpicnum == LAVA
            || sector[sprite[i].sectnum].floorpicnum == SLIME)
        {

            if (sector[sprite[i].sectnum].floorpicnum == WATER || sector[sprite[i].sectnum].floorpicnum == SLIME)
            {
                if (sprite[i].picnum == FISH)
                    sprite[i].z = sector[sprite[i].sectnum].floorz;
                else
                {
                    if (rand() % 100 > 60)
                        makemonstersplash(SPLASHAROO, i);
                }
            }
            else
            {
                trailingsmoke(i);
                makemonstersplash(LAVASPLASH, i);
            }
        }

        if (sector[osectnum].lotag == KILLSECTOR)
        {
            sprite[i].hitag--;
            if (sprite[i].hitag < 0)
                newstatus(i, DIE);
        }

        if (sector[osectnum].floorpicnum == SLIME || sector[osectnum].floorpicnum == WATER)
            sprite[i].z += tilesiz[sprite[i].picnum].y << 3;

        vec3_t pos;
        pos.x = sprite[i].x;
        pos.y = sprite[i].y;
        pos.z = sprite[i].z;

        setsprite(i, &pos);

        if (sector[sprite[i].sectnum].floorpicnum == LAVA ||
            sector[sprite[i].sectnum].floorpicnum == LAVA1 ||
            sector[sprite[i].sectnum].floorpicnum == ANILAVA)
        {
            switch (sprite[i].picnum)
            {
                case SKELETON:
                case KOBOLD:
                case FRED:
                case GOBLIN:
                case MINOTAUR:
                case SPIDER:
                sprite[i].hitag--;
                break;
            }
            if (sprite[i].hitag < 0)
                newstatus(i, DIE);
        }

        j = headspritesect[sprite[i].sectnum];
        while (j != -1)
        {
            nextj = nextspritesect[j];
            dx = labs(sprite[i].x - sprite[j].x);            // x distance to sprite
            dy = labs(sprite[i].y - sprite[j].y);            // y distance to sprite
            dz = labs((sprite[i].z >> 8) - (sprite[j].z >> 8));  // z distance to sprite
            dh = tilesiz[sprite[j].picnum].y >> 1;       // height of sprite
            if (dx + dy < PICKDISTANCE && dz - dh <= PICKHEIGHT)
            {
                switch (sprite[i].picnum)
                {
                    case SKELETON:
                    case KOBOLD:
                    case FRED:
                    case GOBLIN:
                    case SPIDER:
                    case GRONHAL:
                    case GRONMU:
                    case GRONSW:
                    switch (sprite[j].picnum)
                    {
                        case EXPLO2:
                        case SMOKEFX:
                        case MONSTERBALL:
                        sprite[i].hitag -= synctics << 2;
                        if (sprite[i].hitag < 0)
                        {
                            newstatus(i, DIE);
                        }
                        break;
                    }
                    break;
                }
            }
            j = nextj;
        }

    chaseout:
        i = nexti;
    }

    // DRAIN
    i = headspritestat[DRAIN];

    while (i >= 0)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;
        if (sprite[i].lotag < 0)
        {
            playsound_loc(S_FIREBALL, sprite[i].x, sprite[i].y);
            castspell(i);
            newstatus(i, CHASE);
        }
        i = nexti;
    }

    // CAST
    i = headspritestat[CAST];

    while (i >= 0)
    {
        nexti = nextspritestat[i];

        sprite[i].lotag -= synctics;

        if (sprite[i].lotag < 0)
        {
            if (sprite[i].picnum == GRONHALATTACK)
            {
                sprite[i].extra--;
                playsound_loc(S_THROWPIKE, sprite[i].x, sprite[i].y);
                throwhalberd(i);
                newstatus(i, CHASE);
                goto outchase;
            }

            if (sprite[i].picnum == GRONMUATTACK)
            {
                sprite[i].extra--;
                playsound_loc(S_SPELL2, sprite[i].x, sprite[i].y);
                castspell(i);
                newstatus(i, CHASE);
                goto outchase;
            }

            else
            {
                sprite[i].picnum++;
                sprite[i].lotag = 12;
            }
        }

        switch (sprite[i].picnum)
        {
            case DRAGONATTACK + 17:
            case DRAGONATTACK + 4:
            if (rand() % 2)
                playsound_loc(S_FLAME1, sprite[i].x, sprite[i].y);
            else
                playsound_loc(S_FIREBALL, sprite[i].x, sprite[i].y);

            firebreath(i, 1, 2, LOW);
            break;
            case DRAGONATTACK + 18:
            case DRAGONATTACK + 5:
            if (rand() % 2)
                playsound_loc(S_FLAME1, sprite[i].x, sprite[i].y);
            else
                playsound_loc(S_FIREBALL, sprite[i].x, sprite[i].y);

            firebreath(i, 2, 1, LOW);
            break;
            case DRAGONATTACK + 19:
            case DRAGONATTACK + 6:
            if (rand() % 2)
                playsound_loc(S_FLAME1, sprite[i].x, sprite[i].y);
            else
                playsound_loc(S_FIREBALL, sprite[i].x, sprite[i].y);

            firebreath(i, 4, 0, LOW);
            break;
            case DRAGONATTACK + 20:
            case DRAGONATTACK + 7:
            firebreath(i, 2, -1, LOW);
            break;
            case DRAGONATTACK + 21:
            case DRAGONATTACK + 8:
            firebreath(i, 1, -2, LOW);
            break;

            case DRAGONATTACK2 + 2:
            if (rand() % 2)
                playsound_loc(S_FLAME1, sprite[i].x, sprite[i].y);
            else
                playsound_loc(S_FIREBALL, sprite[i].x, sprite[i].y);

            firebreath(i, 1, -1, HIGH);
            break;
            case DRAGONATTACK2 + 3:
            firebreath(i, 2, 0, HIGH);
            break;

            case DRAGONATTACK2 + 5:
            sprite[i].picnum = DRAGON;
            newstatus(i, CHASE);
            break;
            case DRAGONATTACK + 22:
            sprite[i].picnum = DRAGONATTACK;
            newstatus(i, CHASE);
            break;

            case DEVILATTACK + 2:
            sprite[i].picnum = DEVIL;
            playsound_loc(S_FIREBALL, sprite[i].x, sprite[i].y);

            castspell(i);
            newstatus(i, CHASE);
            break;

            case GUARDIANATTACK + 6:
            sprite[i].picnum = GUARDIAN;
            playsound_loc(S_FIREBALL, sprite[i].x, sprite[i].y);
            castspell(i);
            newstatus(i, CHASE);
            break;

            case FATWITCHATTACK + 3:
            sprite[i].picnum = FATWITCH;
            throwspank(i);
            newstatus(i, CHASE);
            break;

            case SKULLYATTACK + 2:
            sprite[i].picnum = SKULLY;
            playsound_loc(S_SKULLWITCH1 + rand() % 3, sprite[i].x, sprite[i].y);
            skullycastspell(i);
            newstatus(i, CHASE);
            break;

            case JUDYATTACK1 + 3:
            sprite[i].picnum = JUDYATTACK1;
            playsound_loc(S_JUDY1 + rand() % 4, sprite[i].x, sprite[i].y);
            if (rand() % 100 > 70)
            {
                castspell(i);
            }
            else
            {
                if (rand() % 100 > 40)
                {
                    // raise the dead
                    j = headspritestat[DEAD];
                    while (j >= 0)
                    {
                        nextj = nextspritestat[j];
                        sprite[j].lotag = (rand() % 120) + 120;
                        newstatus(j, RESURECT);
                        j = nextj;
                    }
                }
                else
                {
                    if (rand() % 100 > 50)
                    {
                        //curse
                        for (j = 1; j < 9; j++)
                        {
                            plr->ammo[j] = 3;
                        }
                    }
                    else
                    {
                        j = rand() % 5;
                        switch (j)
                        {
                            case 0: // SPAWN WILLOW
                            spawnabaddy(i, WILLOW);
                            break;
                            case 1: // SPAWN 10 SPIDERS
                            for (j = 0; j < 4; j++)
                            {
                                spawnabaddy(i, SPIDER);
                            }
                            break;
                            case 2: // SPAWN 2 GRONSW
                            for (j = 0; j < 2; j++)
                            {
                                spawnabaddy(i, GRONSW);
                            }
                            break;
                            case 3: // SPAWN SKELETONS
                            for (j = 0; j < 4; j++)
                            {
                                spawnabaddy(i, SKELETON);
                            }
                            break;
                            case 4:
                            castspell(i);
                            break;
                        }
                    }
                }
            }
            newstatus(i, CHASE);
            break;
            case JUDYATTACK2 + 8:
            sprite[i].picnum = JUDYATTACK2;
            playsound_loc(S_JUDY1 + rand() % 4, sprite[i].x, sprite[i].y);
            if (rand() % 100 > 50)
                skullycastspell(i);
            else
            {
                if (rand() % 100 > 70)
                {
                    if (rand() % 100 > 50)
                    {
                        plr->health = 0;
                        sethealth(1);
                    }
                    else
                    {
                        addarmoramount(-(plr->armor));
                        plr->armortype = 0;
                    }
                }
                else
                {
                    j = rand() % 5;
                    switch (j)
                    {
                        case 0: // SPAWN WILLOW
                        spawnabaddy(i, WILLOW);
                        break;
                        case 1: // SPAWN 6 SPIDERS
                        for (j = 0; j < 4; j++)
                        {
                            spawnabaddy(i, SPIDER);
                        }
                        break;
                        case 2: // SPAWN 2 GRONSW
                        for (j = 0; j < 2; j++)
                        {
                            spawnabaddy(i, GRONSW);
                        }
                        break;
                        case 3: // SPAWN SKELETONS
                        for (j = 0; j < 4; j++)
                        {
                            spawnabaddy(i, SKELETON);
                        }
                        break;
                        case 4:
                        castspell(i);
                        break;
                    }
                }
            }
            newstatus(i, CHASE);
            break;
        }

        if (sector[sprite[i].sectnum].floorz - (32 << 8) < sector[sprite[i].sectnum].ceilingz)
        {
            if (sector[sprite[i].sectnum].lotag == 6)
                newstatus(i, DIE);
            else
            {
                deletesprite(i);
            }
        }
    outchase:
        i = nexti;
    }

    // DIE
    i = headspritestat[DIE];

    while (i >= 0)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;

        switch (sprite[i].picnum)
        {
            case RAT:
            case FISH:
            deletesprite(i);
            goto dieout;
            break;
            case GRONHALDIE:
            case GRONSWDIE:
            case GRONMUDIE:
            if (sprite[i].lotag < 0)
            {
                sprite[i].picnum = GRONDIE;
                sprite[i].lotag = 20;
            }
            else
            {
                goto dieout;
            }
            break;
        }

        if (sprite[i].lotag <= 0)
        {
            sprite[i].picnum++;
            sprite[i].lotag = 20;
            if (sprite[i].picnum == WILLOWEXPLO
                || sprite[i].picnum == WILLOWEXPLO + 1
                || sprite[i].picnum == WILLOWEXPLO + 2)
                sprite[i].xrepeat = sprite[i].yrepeat <<= 1;
            switch (sprite[i].picnum)
            {
                case WILLOWEXPLO + 2:
                case DRAGONDEAD:
                case KOBOLDDEAD:
                case DEVILDEAD:
                case FREDDEAD:
                case SKELETONDEAD:
                case GOBLINDEAD:
                case MINOTAURDEAD:
                case SPIDERDEAD:
                case SKULLYDEAD:
                case FATWITCHDEAD:
                case JUDYDEAD:
                case GRONDEAD:
                if (difficulty == 4)
                    newstatus(i, RESURECT);
                else
                    newstatus(i, DEAD);
                break;
            }
        }
    dieout:
        i = nexti;
    }

    // STAND
    i = headspritestat[STAND];

    while (i >= 0)
    {
        nexti = nextspritestat[i];

        sprite[i].ang = (getangle(plr->x - sprite[i].x, plr->y - sprite[i].y) & kAngleMask);

        if (Cos(sprite[i].ang + 2048) * (plr->x - sprite[i].x)
            + Sin(sprite[i].ang + 2048) * (plr->y - sprite[i].y) >= 0)
            if (cansee(plr->x, plr->y, plr->z, plr->sector, sprite[i].x, sprite[i].y, sprite[i].z - (tilesiz[sprite[i].picnum].y << 7), sprite[i].sectnum) == 1
                && plr->invisibletime < 0)
            {
                switch (sprite[i].picnum)
                {
                    case GOBLINCHILL:
                    sprite[i].picnum = GOBLINSURPRISE;
                    playsound_loc(S_GOBPAIN1 + (rand() % 2), sprite[i].x, sprite[i].y);
                    newstatus(i, CHILL);
                    break;
                    case HANGMAN:
                    newstatus(i, CHILL);
                    playsound_loc(S_SKELSEE, sprite[i].x, sprite[i].y);
                    break;
                    case SKELETON:
                    case KOBOLD:
                    case GOBLIN:
                    case FRED:
                    case DRAGON:
                    case SPIDER:
                    case MINOTAUR:
                    case FISH:
                    case RAT:
                    if (plr->shadowtime > 0)
                    {
                        sprite[i].ang = (((krand() & 512) - 256) + sprite[i].ang + 1024) & kAngleMask;  // NEW
                        newstatus(i, FLEE);
                    }
                    else
                    {
                        newstatus(i, CHASE);
                    }
                    break;
                    case SKULLY:
                    case FATWITCH:
                    case DEVIL:
                    case JUDY:
                    case GUARDIAN:
                    case WILLOW:
                    newstatus(i, FACE);
                    break;
                }
            }

        if (sector[sprite[i].sectnum].floorz - (32 << 8) < sector[sprite[i].sectnum].ceilingz)
        {
            if (sector[sprite[i].sectnum].lotag == 6)
                newstatus(i, DIE);
            else
            {
                deletesprite(i);
            }
        }

        i = nexti;
    }

#if 0
    // heatseeker
    i = headspritestat[HEATSEEKER];
    while (i != -1)
    {
        nexti = nextspritestat[i];
        spr = &sprite[i];

        sprite[i].lotag -= synctics;

        checkheat(i);

        movestat = movesprite((short)i, (((int)sintable[(sprite[i].ang + 512) & kAngleMask]) * synctics) << 3, (((int)sintable[sprite[i].ang]) * synctics) << 3, 0, 4 << 8, 4 << 8, 0);

        if (movestat != 0 || sprite[i].lotag < 0)
        {

            x = sprite[i].x;
            y = sprite[i].y;
            z = sprite[i].z;
            belongs = sprite[i].owner;

            for (j = 0; j < MAXSPRITES; j++)
            {

                if (j != plr->spritenum)
                    if (sprite[j].picnum == FRED
                        || sprite[j].picnum == KOBOLD
                        || sprite[j].picnum == KOBOLDATTACK
                        || sprite[j].picnum == GOBLIN
                        || sprite[j].picnum == GOBLINATTACK
                        || sprite[j].picnum == GOBLINCHILL
                        || sprite[j].picnum == MINOTAUR
                        || sprite[j].picnum == MINOTAURATTACK
                        || sprite[j].picnum == SPIDER
                        || sprite[j].picnum == SKELETON
                        || sprite[j].picnum == SKELETONATTACK
                        || sprite[j].picnum == GRONHAL
                        || sprite[j].picnum == GRONHALATTACK
                        || sprite[j].picnum == GRONMU
                        || sprite[j].picnum == GRONMUATTACK
                        || sprite[j].picnum == GRONSW
                        || sprite[j].picnum == GRONSWATTACK
                        || sprite[j].picnum == DRAGON
                        || sprite[j].picnum == FATWITCH
                        || sprite[j].picnum == SKULLY
                        || sprite[j].picnum == DEVIL
                        || sprite[j].picnum == DEVILATTACK)
                        //if( cansee(plr->x,plr->y,plr->z,plr->sector,sprite[j].x,sprite[j].y,sprite[j].z-(tilesizy[sprite[i].picnum]<<7),sprite[i].sectnum) == 1 )
                        //    if(checkmedusadist(j,plr->x,plr->y,plr->z,12) )
                        //       nukespell(j);
                        if (cansee(sprite[i].x, sprite[i].y, sprite[i].z, sprite[i].sectnum, sprite[j].x, sprite[j].y, sprite[j].z - (tilesizy[sprite[i].picnum] << 7), sprite[i].sectnum) == 1)
                            if (checkmedusadist(j, sprite[i].x, sprite[i].y, sprite[i].z, 12))
                                nukespell(j);
            }
            deletesprite(i);
        }

        i = nexti;
    }
#endif

    // New missile code
    i = headspritestat[MISSILE];

    while (i != -1)
    {
        nexti = nextspritestat[i];

        sprite[i].lotag -= synctics;

        switch (sprite[i].picnum)
        {
            case THROWPIKE:
            case FATSPANK:
            case MONSTERBALL:
            case FIREBALL:
            case PLASMA:
            sprite[i].z += sprite[i].zvel;

            if (sprite[i].z < sector[sprite[i].sectnum].ceilingz + (4 << 8))
            {
                sprite[i].z = sector[sprite[i].sectnum].ceilingz + (4 << 8);
                sprite[i].zvel = -(sprite[i].zvel >> 1);
            }

            if (sprite[i].z > sector[sprite[i].sectnum].floorz - (4 << 8))
            {
                sprite[i].z = sector[sprite[i].sectnum].floorz - (4 << 8);
                if (sector[sprite[i].sectnum].floorpicnum == WATER
                    || sector[sprite[i].sectnum].floorpicnum == SLIME)
                {
                    if (sprite[i].picnum == FISH)
                        sprite[i].z = sector[sprite[i].sectnum].floorz;
                    else
                    {
                        if (rand() % 100 > 60)
                            makemonstersplash(SPLASHAROO, i);
                    }
                }
                deletesprite(i);
                goto bulletisdeletedskip;
            }
            dax = sprite[i].xvel;
            day = sprite[i].yvel;
            daz = ((((int)sprite[i].zvel) * synctics) >> 3);
            break;
            case BULLET:
            dax = sprite[i].xvel;
            day = sprite[i].yvel;
            daz = sprite[i].zvel;
            break;
        } // switch

        osectnum = sprite[i].sectnum;

        if (sprite[i].picnum == THROWPIKE)
        {
            sprite[i].cstat = 0;

            hitobject = movesprite(i, (Cos(sprite[i].extra) * synctics) << 6, (Sin(sprite[i].extra) * synctics) << 6,
                daz, 4 << 8, 4 << 8, 1);

            sprite[i].cstat = 21;
        }
        else
        {
            hitobject = movesprite(i, (Cos(sprite[i].ang) * synctics) << 6, (Sin(sprite[i].ang) * synctics) << 6,
                daz, 4 << 8, 4 << 8, 1);
        }

        if (hitobject && sprite[i].picnum == MONSTERBALL)
        {
            if (sprite[i].owner == 4096)
            {
                for (j = 0; j < 8; j++)
                    explosion2(i, sprite[i].x, sprite[i].y, sprite[i].z, i);
            }
            else
            {
                for (j = 0; j < 4; j++)
                    explosion(i, sprite[i].x, sprite[i].y, sprite[i].z, i);
            }
        }

        if ((hitobject & 0xc000) == 16384) //Hits a ceiling / floor
        {  
            deletesprite(i);
            goto bulletisdeletedskip;
        }
        else if ((hitobject & 0xc000) == 32768) // hit a wall
        {
            if (sprite[i].picnum == THROWPIKE)
            {
                sprite[i].picnum++;
                changespritestat(i, 0);
                goto bulletisdeletedskip;
            }

            deletesprite(i);
            goto bulletisdeletedskip;
        }
        else if (sprite[i].lotag < 0 && sprite[i].picnum == PLASMA)
        {
            hitobject = 1;
        }
        if ((hitobject & 0xc000) == 49152) //Bullet hit a sprite
        {
            if (sprite[i].owner != hitobject)
            {
                hitdamage = damageactor(hitobject, i);
            }

            if (hitdamage)
            {
                deletesprite(i);
                goto bulletisdeletedskip;
            }
        }

        if (hitobject != 0 || sprite[i].lotag < 0)
        {
            x = sprite[i].x;
            y = sprite[i].y;
            z = sprite[i].z;
            belongs = sprite[i].owner;

            switch (sprite[i].picnum)
            {
                case THROWPIKE:
                case PLASMA:
                case FATSPANK:
                case MONSTERBALL:
                case FIREBALL:
                case BULLET:
                deletesprite(i);
                goto bulletisdeletedskip;
                break;
            }
        }
    bulletisdeletedskip:
        i = nexti;
    }

    i = headspritestat[JAVLIN];

    while (i != -1)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;

        switch (sprite[i].picnum)
        {
            case THROWPIKE:
            case WALLARROW:
            case DART:
            case HORIZSPIKEBLADE:
            case THROWHALBERD:
            sprite[i].z += sprite[i].zvel;

            if (sprite[i].z < sector[sprite[i].sectnum].ceilingz + (4 << 8))
            {
                sprite[i].z = sector[sprite[i].sectnum].ceilingz + (4 << 8);
                sprite[i].zvel = -(sprite[i].zvel >> 1);
            }

            if (sprite[i].z > sector[sprite[i].sectnum].floorz - (4 << 8))
            {
                sprite[i].z = sector[sprite[i].sectnum].floorz - (4 << 8);
                if (sector[sprite[i].sectnum].floorpicnum == WATER
                    || sector[sprite[i].sectnum].floorpicnum == SLIME)
                {
                    if (rand() % 100 > 60)
                        makemonstersplash(SPLASHAROO, i);
                }
                deletesprite(i);
                goto bulletisdeletedskip;
            }
            dax = sprite[i].xvel;
            day = sprite[i].yvel;
            daz = ((((int)sprite[i].zvel) * synctics) >> 3);
            break;
        }

        osectnum = sprite[i].sectnum;

        sprite[i].cstat = 0;

        hitobject = movesprite(i, (Cos(sprite[i].extra) * synctics) << 6, (Sin(sprite[i].extra) * synctics) << 6, daz, 4 << 8, 4 << 8, 0);

        if (sprite[i].picnum == WALLARROW || sprite[i].picnum == THROWHALBERD)
            sprite[i].cstat = 17;
        else
            sprite[i].cstat = 21;

        if ((hitobject & 0xc000) == 16384) // Hits a ceiling / floor
        {  
            if (sprite[i].picnum == THROWPIKE)
                sprite[i].picnum++;

            changespritestat(i, 0);
            goto javlinskip;
        }
        else if ((hitobject & 0xc000) == 32768) // hit a wall
        {  
            if (sprite[i].picnum == THROWPIKE)
                sprite[i].picnum++;

            changespritestat(i, 0);
            goto javlinskip;
        }

        if ((hitobject - 49152) >= 0 || (hitobject & 0xc000) == 49152) //Bullet hit a sprite
        { 
            j = (hitobject & 4095);     // j is the spritenum that the bullet (spritenum i) hit

            if (sprite[i].owner != hitobject)
            {
                hitdamage = damageactor(hitobject, i);
                goto javlinskip;
            }

            if (!hitdamage)
            {
                switch (sprite[j].picnum)
                {
                    case THROWPIKE:
                    case WALLARROW:
                    case DART:
                    case HORIZSPIKEBLADE:
                    case THROWHALBERD:
                    deletesprite(i);
                    goto javlinskip;
                    break;
                }
            }
        }

        if (hitobject != 0)
        {
            deletesprite(i);
            goto javlinskip;
        }
    javlinskip:
        i = nexti;
    }

    // CHUNK O WALL
    i = headspritestat[CHUNKOWALL];

    while (i >= 0)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;
        dax = sprite[i].xvel >> 3;
        day = sprite[i].yvel >> 3;
        daz = sprite[i].zvel -= synctics << 2;

        movestat = movesprite(i, (Cos(sprite[i].ang) * synctics) << 3, (Sin(sprite[i].ang) * synctics) << 3, 0, 4 << 8, 4 << 8, 1);

        vec3_t pos;
        pos.x = sprite[i].x;
        pos.y = sprite[i].y;
        pos.z = sprite[i].z;

        setsprite(i, &pos);
        if (sprite[i].extra == 0)
        {
            if (sprite[i].lotag < 0)
            {
                sprite[i].lotag = 8;
                sprite[i].picnum++;
                if (sprite[i].picnum == SMOKEFX + 3)
                    deletesprite(i);
            }
        }
        else
        {
            if (sprite[i].lotag < 0)
                deletesprite(i);
        }
        i = nexti;
    }

    // CHUNK O MEAT
    i = headspritestat[CHUNKOMEAT];

    while (i >= 0)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;

        sprite[i].z += sprite[i].zvel;

        daz = sprite[i].zvel += synctics << 4;

        if (sprite[i].picnum == BONECHUNK1 && sprite[i].picnum == BONECHUNKEND)
        {
            daz >>= 1;
            movestat = movesprite(i, (Cos(sprite[i].ang) * synctics) << 2, (Sin(sprite[i].ang) * synctics) << 2, 
                daz, 4 << 8, 4 << 8, 1);
        }
        else
        {
            movestat = movesprite(i, (Cos(sprite[i].ang) * synctics) << 3, (Sin(sprite[i].ang) * synctics) << 3,
                daz, 4 << 8, 4 << 8, 1);
        }

        if ((movestat & 0xc000) == 16384)
        {
            if (sector[sprite[i].sectnum].floorpicnum == WATER || sector[sprite[i].sectnum].floorpicnum == SLIME)
            {
                if (sprite[i].picnum == FISH)
                    sprite[i].z = sector[sprite[i].sectnum].floorz;
                else
                {
                    if (rand() % 100 > 60)
                        makemonstersplash(SPLASHAROO, i);
                }
                sprite[i].lotag = -1;
            }
            else
            {
                if (sprite[i].picnum >= BONECHUNK1 && sprite[i].picnum <= BONECHUNKEND)
                {
                    deletesprite(i);
                }
                else
                {
                    sprite[i].lotag = 1200;
                    newstatus(i, BLOOD);
                }
            }
        }
        else if ((movestat & 0xc000) == 32768)
        {
            if (sprite[i].picnum >= BONECHUNK1 && sprite[i].picnum <= BONECHUNKEND)
            {
                deletesprite(i);
            }
            else
            {
                sprite[i].lotag = 1200;
                newstatus(i, BLOOD);
            }
        }
        if (sprite[i].lotag < 0)
            deletesprite(i);
        i = nexti;
    }

    i = headspritestat[BLOOD];

    while (i != -1)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;

        if (sprite[i].lotag < 0)
        {
            if (sprite[i].z < sector[sprite[i].sectnum].floorz)
            {
                sprite[i].lotag = 600;
                sprite[i].zvel = 0;
                newstatus(i, DRIP);
            }
            else
                deletesprite(i);
        }
        i = nexti;
    }

    i = headspritestat[DEVILFIRE];

    while (i != -1)
    {
        nexti = nextspritestat[i];
        if (plr->invisibletime < 0)
        {
            sprite[i].lotag -= synctics;
            if (sprite[i].lotag < 0)
            {
                sprite[i].lotag = (krand() & 120) + 360;
                if (cansee(plr->x,
                    plr->y,
                    plr->z,
                    plr->sector,
                    sprite[i].x,
                    sprite[i].y,
                    sprite[i].z -
                    (tilesiz[sprite[i].picnum].y << 7),
                    sprite[i].sectnum) == 1)
                {
                    playsound_loc(S_FIREBALL, sprite[i].x, sprite[i].y);
                    castspell(i);
                }
            }
        }
        i = nexti;
    }

    i = headspritestat[DRIP];

    while (i != -1)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;
        sprite[i].z += sprite[i].zvel;
        dax = 0L;
        day = 0L;
        daz = sprite[i].zvel += synctics << 1;
        daz = ((((int)sprite[i].zvel) * synctics) << 1);
        movestat = movesprite(i, dax, day, daz, 4 << 8, 4 << 8, 1);

        if ((movestat & 0xc000) == 16384)
        {
            sprite[i].lotag = 1200;
            newstatus(i, BLOOD);
        }
        if (sprite[i].lotag < 0)
            deletesprite(i);
        i = nexti;
    }

    i = headspritestat[SMOKE];

    while (i >= 0)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;
        sprite[i].z -= (synctics << 6);

        if (sprite[i].xrepeat > 1)
            sprite[i].xrepeat = sprite[i].yrepeat -= synctics;

        vec3_t pos;
        pos.x = sprite[i].x;
        pos.y = sprite[i].y;
        pos.z = sprite[i].z;

        setsprite(i, &pos);
        if (sprite[i].lotag < 0)
            deletesprite(i);
        i = nexti;
    }

    i = headspritestat[EXPLO];

    while (i >= 0)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;

        sprite[i].x += ((sprite[i].xvel * synctics) >> 5);
        sprite[i].y += ((sprite[i].yvel * synctics) >> 5);
        sprite[i].z -= ((sprite[i].zvel * synctics) >> 6);

        sprite[i].zvel += (synctics << 4);

        if (sprite[i].z < sector[sprite[i].sectnum].ceilingz + (4 << 8))
        {
            sprite[i].z = sector[sprite[i].sectnum].ceilingz + (4 << 8);
            sprite[i].zvel = -(sprite[i].zvel >> 1);
        }
        if (sprite[i].z > sector[sprite[i].sectnum].floorz - (4 << 8))
        {
            sprite[i].z = sector[sprite[i].sectnum].floorz - (4 << 8);
            sprite[i].zvel = -(sprite[i].zvel >> 1);
        }

        sprite[i].xrepeat += synctics;
        sprite[i].yrepeat += synctics;

        sprite[i].lotag -= synctics;

        if (rand() % 100 > 90)
        {
            j = insertsprite(sprite[i].sectnum, SMOKE);

            sprite[j].x = sprite[i].x;
            sprite[j].y = sprite[i].y;
            sprite[j].z = sprite[i].z;
            sprite[j].cstat = 0x03;
            sprite[j].cstat &= ~3;
            sprite[j].picnum = SMOKEFX;
            sprite[j].shade = 0;
            sprite[j].pal = 0;
            sprite[j].xrepeat = sprite[i].xrepeat;
            sprite[j].yrepeat = sprite[i].yrepeat;

            sprite[j].owner = sprite[i].owner;
            sprite[j].lotag = 256;
            sprite[j].hitag = 0;
        }

        if (sprite[i].lotag < 0)
            deletesprite(i);

        i = nexti;
    }

    i = headspritestat[BROKENVASE];

    while (i >= 0)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;

        if (sprite[i].lotag < 0)
        {
            sprite[i].picnum++;
            sprite[i].lotag = 18;

            switch (sprite[i].picnum)
            {
                case FSHATTERBARREL + 2:
                randompotion(i);
                case SHATTERVASE + 6:
                case SHATTERVASE2 + 6:
                case SHATTERVASE3 + 6:
                changespritestat(i, 0);
                break;
            }
        }
        i = nexti;
    }

    i = headspritestat[FX]; //Go through explosion sprites

    while (i >= 0)
    {
        nexti = nextspritestat[i];
        sprite[i].lotag -= synctics;

        if (sprite[i].picnum == BULLET
            || sprite[i].picnum == EXPLOSION
            || sprite[i].picnum == FIREBALL
            || sprite[i].picnum == MONSTERBALL
            || sprite[i].picnum == FATSPANK)
        {
            sprite[i].zvel += (synctics << 5);
            if (sprite[i].z < sector[sprite[i].sectnum].ceilingz + (4 << 8))
            {
                sprite[i].z = sector[sprite[i].sectnum].ceilingz + (4 << 8);
                sprite[i].zvel = -(sprite[i].zvel >> 1);
            }
            if (sprite[i].z > sector[sprite[i].sectnum].floorz - (4 << 8)
                && sprite[i].picnum != EXPLOSION)
            {
                sprite[i].z = sector[sprite[i].sectnum].floorz - (4 << 8);
                sprite[i].zvel = 0;
                sprite[i].lotag = 4;
            }
            dax = ((((int)sprite[i].xvel) * synctics) >> 3);
            day = ((((int)sprite[i].yvel) * synctics) >> 3);
            daz = (((int)sprite[i].zvel) * synctics);
            movestat = movesprite((short)i, dax, day, daz, 4 << 8, 4 << 8, 1);

            vec3_t pos;
            pos.x = sprite[i].x;
            pos.y = sprite[i].y;
            pos.z = sprite[i].z;

            setsprite(i, &pos);
        }

        if (sprite[i].picnum == ICECUBE)
        {
            sprite[i].z += sprite[i].zvel;

            daz = sprite[i].zvel += synctics << 4;

            movestat = movesprite(i, (Cos(sprite[i].ang) * synctics) << 3, (Sin(sprite[i].ang) * synctics) << 3,
                daz, 4 << 8, 4 << 8, 1);
        }

        if (sprite[i].lotag < 0 || movestat != 0)
        {
            if (sprite[i].picnum == PLASMA
                || sprite[i].picnum == EXPLOSION
                || sprite[i].picnum == FIREBALL
                || sprite[i].picnum == MONSTERBALL
                || sprite[i].picnum == FATSPANK
                || sprite[i].picnum == ICECUBE)
            {
                deletesprite(i);
                goto outathere;
            }
        }

        if (((sprite[i].z + (8 << 8)) >= sector[sprite[i].sectnum].floorz) && sprite[i].picnum == ICECUBE || movestat != 0)
        {
            sprite[i].z = sector[sprite[i].sectnum].floorz, changespritestat(i, 0);
            if (sector[sprite[i].sectnum].floorpicnum == WATER || sector[sprite[i].sectnum].floorpicnum == SLIME)
            {
                if (sprite[i].picnum == FISH)
                    sprite[i].z = sector[sprite[i].sectnum].floorz;
                else
                {
                    if (rand() % 100 > 60)
                        makemonstersplash(SPLASHAROO, i);
                }
            }
        }
    outathere:
        i = nexti;
    }
}

// TEMP - remove me when proper defines are in
#define PATROLPOINT 999

int findapatrolpoint(short i)
{
    short p, target;
    int32_t mindist = 0x7fffffff;
    int32_t dist;
    short oldang;
    spritetype* spr, * tspr;

    target = 0;

    spr = &sprite[i];

    oldang = spr->ang;

    for (p = 0; p < MAXSPRITES; p++)
    {
        if (sprite[p].picnum == PATROLPOINT && spr->owner != p)
        {
            dist = klabs(sprite[p].x - spr->x) + klabs(sprite[p].y - spr->y);
            if (dist < mindist)
            {
                mindist = dist;
                target = p;
            }
        }
    }

    spr->owner = target;
    tspr = &sprite[target];

    if (tspr->cstat != -1)
    {
        spr->ang = (getangle(spr->x - tspr->x, spr->y - tspr->y) & kAngleMask);

        if (cansee(tspr->x,
            tspr->y,
            tspr->z,
            tspr->sectnum,
            spr->x,
            spr->y,
            spr->z - (tilesiz[spr->picnum].y << 7),
            spr->sectnum) == 1)
        {

            spr->owner = target;
            return (1);
        }
        else
        {
            spr->ang = oldang;
            return 0;
        }
    }

    return 0;
}

void checkhit(short i, short j)
{
    short daang2, daang;
    int32_t daz2;
    int32_t x, y, z, dasectnum;

    daang = sprite[j].ang;
    x = sprite[j].x;
    y = sprite[j].y;
    z = sprite[j].z;
    dasectnum = sprite[j].sectnum;
    daang2 = ((daang + 2048) & kAngleMask);
    daz2 = 0;

    vec3_t pos;
    pos.x = x;
    pos.y = y;
    pos.z = z;

    hitdata_t hitinfo;

    hitscan(&pos, dasectnum, Cos(daang2 + 2048), Sin(daang2 + 2048), daz2, &hitinfo, CLIPMASK1); // CHECKME - is CLIPMASK1 correct for version of this function that hadn't got cliptype param?

    sprite[i].ang = (getangle(hitinfo.x - sprite[i].x, hitinfo.y - sprite[i].y) & kAngleMask);
}
